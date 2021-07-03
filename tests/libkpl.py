import os
import subprocess

KPL_BUILD_DIR = os.getenv("KPL_BUILD_DIR")
if KPL_BUILD_DIR is None:
    raise RuntimeError("Please set the KPL_BUILD_DIR to a build directory of libkpl to test")

class LibKPL:
    exception = subprocess.CalledProcessError
    @classmethod
    def get_key_from_name(cls, port, name):
        key = cls.run(port, "kpl_derive", name).strip()
        return bytes.fromhex(key.strip().decode("ascii"))

    @classmethod
    def store_key_slot(cls, port, slot, key):
        cls.run(port, "kpl_set", str(slot), key.hex())

    @classmethod
    def get_key_slot(cls, port, slot):
        key = cls.run(port, "kpl_get", str(slot)).strip()
        return bytes.fromhex(key.strip().decode("ascii"))

    @classmethod
    def get_valid_slots(cls, port):
        slots = cls.run(port, "kpl_get_slots").strip().decode("ascii")
        return [int(v.strip()) for v in slots.split(" ")]

    @classmethod
    def erase_all_slots(cls, port):
        cls.run(port, "kpl_erase_all_slots")

    @staticmethod
    def connect(host,port):
        return (host,port)

    # Helpers
    @staticmethod
    def run(host_port, tool, *args):
        host,port = host_port
        env = {'LEDGER_PROXY_ADDRESS': host, 'LEDGER_PROXY_PORT': str(port)}
        pargs = [os.path.join(KPL_BUILD_DIR, "tools", tool)] + list(args)
        p = subprocess.run(pargs, stdout=subprocess.PIPE, check=True, env=env)
        return p.stdout
