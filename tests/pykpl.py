import random
import donna25519
import hashlib
import struct
from ledgerwallet.client import LedgerClient, CommException
from ledgerwallet.transport.tcp import TcpDevice

GET_APP_CONFIGURATION = 0
STORE_KEY = 1
GET_KEY = 2
GET_KEY_FROM_NAME = 3
GET_VALID_KEY_SLOTS = 4

class PyKPL:
    exception = CommException

    @classmethod
    def get_key_from_name(cls, dongle, name):
        privkey, pubkey = cls.gen_x25519_keypair()
        result = dongle.apdu_exchange(GET_KEY_FROM_NAME, pubkey.public+name, 0, 0)
        return cls.decrypt_kpkey(privkey, pubkey, result)

    @classmethod
    def store_key_slot(cls, dongle, slot, key):
        dongle.apdu_exchange(STORE_KEY, key, slot, 0)

    @classmethod
    def get_key_slot(cls, dongle, slot):
        privkey, pubkey = cls.gen_x25519_keypair()
        result = dongle.apdu_exchange(GET_KEY, pubkey.public, slot, 0)
        return cls.decrypt_kpkey(privkey, pubkey, result)

    @classmethod
    def get_valid_slots(cls, dongle):
        slots = dongle.apdu_exchange(GET_VALID_KEY_SLOTS, b"", 0, 0)
        slots = struct.unpack("<B", slots)[0]
        ret = []
        s = 0
        while slots > 0:
            if slots & 1:
                ret.append(s)
            slots >>= 1
            s += 1
        return ret

    # X25519 exchange helpers
    @staticmethod
    def gen_x25519_keypair():
        # WARNING: we are not using a cryptographically secure random generator
        # here. Do not do this for real-life applications!
        privkey = bytes(random.getrandbits(8) for _ in range(32))
        privkey = donna25519.PrivateKey(privkey)
        pubkey = privkey.get_public()
        return (privkey,pubkey)

    @staticmethod
    def decrypt_kpkey(own_privkey, own_pubkey, apdu_result):
        apdu_result = bytes(apdu_result)
        app_pubkey,key = apdu_result[:32],apdu_result[32:]
        app_pubkey = donna25519.PublicKey(app_pubkey)
        ecdhe = own_privkey.do_exchange(app_pubkey)
        h = hashlib.blake2b(ecdhe, digest_size=32)
        h.update(own_pubkey.public)
        h.update(app_pubkey.public)
        keystream = h.digest()
        return bytes(a^b for a,b in zip(key,keystream))

    @staticmethod
    def connect(port):
        return LedgerClient(device=TcpDevice("127.0.0.1:%d" % port))

if __name__ == "__main__":
    clt = LedgerClient()
    print(PyKPL.get_key_from_name(clt, b"test").hex())
