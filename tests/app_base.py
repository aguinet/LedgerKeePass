import unittest
import random

from run_speculos import RunSpeculos

TXT_KEEPASS_OPEN_NAME = "Keepass open name"
TXT_KEEPASS_OPEN_SLOT = "Keepass open slot"
TXT_ERASE = "Erase?"
SLOT_COUNT = 8

def automation_txt(txts, btn):
    # See https://github.com/LedgerHQ/speculos/blob/master/doc/automation.md
    # for more information
    if isinstance(txts, str):
        txts = [txts]
    rules = [{
        "text": txt,
        "actions": [ ["button", btn, True], [ "button", btn, False ] ]
    } for txt in txts]
    return {"version": 1, "rules": rules}

def automation_accept_txt(txts):
    return automation_txt(txts, 2)
def automation_refuse_txt(txts):
    return automation_txt(txts, 1)

class BaseTestCase:
    def run_speculos(self, automation=None):
        return RunSpeculos(self.API, automation)

    def test_get_key_by_name_accepts(self):
        # If TXT_KEEPASS_OPEN_NAME appears, accepts it.
        automation_accept = automation_accept_txt(TXT_KEEPASS_OPEN_NAME)

        # Send the request twice, verify that it's the same key twice
        with self.run_speculos(automation_accept) as dongle:
            key0 = self.API.get_key_from_name(dongle, b"perso")
            key1 = self.API.get_key_from_name(dongle, b"perso")
            self.assertEqual(key0, key1)
            # TODO: reproductible tests with test vectors for a given seed

    def test_get_key_by_name_refuses(self):
        # If TXT_KEEPASS_OPEN_NAME appears, refuses it.
        automation_refuse = automation_refuse_txt(TXT_KEEPASS_OPEN_NAME)
        with self.run_speculos(automation_refuse) as dongle:
            with self.assertRaises(self.API.exception):
                self.API.get_key_from_name(dongle, b"perso")

    def test_store_key_erase_accepts(self):
        automation_accept = automation_accept_txt([TXT_ERASE,TXT_KEEPASS_OPEN_SLOT])
        key0 = bytes(random.getrandbits(8) for _ in range(32))
        key1 = bytes(random.getrandbits(8) for _ in range(32))

        with self.run_speculos(automation_accept) as dongle:
            self.API.store_key_slot(dongle, 1, key0)
            key = self.API.get_key_slot(dongle, 1)
            self.assertEqual(key, key0)

            self.API.store_key_slot(dongle, 1, key1)
            key = self.API.get_key_slot(dongle, 1)
            self.assertEqual(key, key1)

    def test_store_key_erase_refuses(self):
        automation_refuse = automation_refuse_txt(TXT_ERASE)
        key0 = bytes(random.getrandbits(8) for _ in range(32))

        with self.run_speculos(automation_refuse) as dongle:
            self.API.store_key_slot(dongle, 1, key0)
            with self.assertRaises(self.API.exception):
                self.API.store_key_slot(dongle, 1, key0)

    def test_store_get_key_slot(self):
        automation_accept = automation_accept_txt(TXT_KEEPASS_OPEN_SLOT)
        with self.run_speculos(automation_accept) as dongle:
            for s in range(SLOT_COUNT):
                keyref = bytes(random.getrandbits(8) for _ in range(32))
                self.API.store_key_slot(dongle, s, keyref)
                key = self.API.get_key_slot(dongle, s)
                self.assertEqual(keyref, key)

    def test_get_invalid_key_slot(self):
        with self.run_speculos() as dongle:
            for s in range(SLOT_COUNT):
                with self.assertRaises(self.API.exception):
                    self.API.get_key_slot(dongle, s)

    def test_get_valid_slots(self):
        key = bytes(random.getrandbits(8) for _ in range(32))
        slots = [0,2,4,6]
        with self.run_speculos() as dongle:
            for s in slots:
                self.API.store_key_slot(dongle, s, key)
            recvslots = self.API.get_valid_slots(dongle)
            self.assertEqual(slots, recvslots)

    def test_invalid_name(self):
        with self.run_speculos() as dongle:
            with self.assertRaises(self.API.exception):
                self.API.get_key_from_name(dongle, "tést".encode("utf8"))
