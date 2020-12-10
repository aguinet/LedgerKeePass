import os
import subprocess

KPL_BUILD_DIR = os.getenv("KPL_BUILD_DIR")
if KPL_BUILD_DIR is None:
    raise RuntimeError("Please set the KPL_BUILD_DIR to a build directory of libkpl to test")

class LibKPL:
    exception = subprocess.CalledProcessError
    @classmethod
    def get_key_from_name(cls, port, name):
        key = cls.run(port, "derive", name).strip()
        return bytes.fromhex(key.strip().decode("ascii"))

    @classmethod
    def store_key_slot(cls, port, slot, key):
        cls.run(port, "set", str(slot), key.hex())

    @classmethod
    def get_key_slot(cls, port, slot):
        key = cls.run(port, "get", str(slot)).strip()
        return bytes.fromhex(key.strip().decode("ascii"))

    @classmethod
    def get_valid_slots(cls, port):
        slots = cls.run(port, "get_slots").strip().decode("ascii")
        return [int(v.strip()) for v in slots.split(" ")]

    @staticmethod
    def connect(port):
        return port

    # Helpers
    @staticmethod
    def run(port, tool, *args):
        env = {'LEDGER_PROXY_ADDRESS': '127.0.0.1', 'LEDGER_PROXY_PORT': str(port)}
        pargs = [os.path.join(KPL_BUILD_DIR, "tools", tool)] + list(args)
        p = subprocess.run(pargs, stdout=subprocess.PIPE, check=True, env=env)
        return p.stdout
