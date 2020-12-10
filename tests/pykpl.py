import random
import donna25519
import hashlib
import struct
from ledgerblue.comm import getDongleTCP
from ledgerblue.commException import CommException

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
        apdu = cls.apdu(GET_KEY_FROM_NAME, 0, 0, pubkey.public+name, 32)
        result = dongle.exchange(apdu)
        return cls.decrypt_kpkey(privkey, pubkey, result)

    @classmethod
    def store_key_slot(cls, dongle, slot, key):
        apdu = cls.apdu(STORE_KEY, slot, 0, key, 0)
        dongle.exchange(apdu)

    @classmethod
    def get_key_slot(cls, dongle, slot):
        privkey, pubkey = cls.gen_x25519_keypair()
        apdu = cls.apdu(GET_KEY, slot, 0, pubkey.public, 32*2)
        result = dongle.exchange(apdu)
        return cls.decrypt_kpkey(privkey, pubkey, result)

    @classmethod
    def get_valid_slots(cls, dongle):
        apdu = cls.apdu(GET_VALID_KEY_SLOTS, 0, 0, b"", 10)
        slots = dongle.exchange(apdu)
        slots = struct.unpack("<B", slots)[0]
        ret = []
        s = 0
        while slots > 0:
            if slots & 1:
                ret.append(s)
            slots >>= 1
            s += 1
        return ret

    # Helpers
    @staticmethod
    def apdu(ins, p1, p2, data, le):
        msg = bytes((0xE0,ins,p1,p2,len(data)))
        msg += data
        msg += bytes((le,))
        return msg

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
        return getDongleTCP(server="127.0.0.1", port=port, debug=True)
