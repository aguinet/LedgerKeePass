import unittest
import random
import itertools

from setup_speculos import SetupSpeculos, speculos_model

TXT_KEEPASS_OPEN_NAME = "Database open name"
TXT_KEEPASS_OPEN_SLOT = "Database open slot"
TXT_ERASE = "Erase?"
TXT_ERASE_ALL_SLOTS = "Erase all slots?"
SLOT_COUNT = 8

class Btn:
    def __init__(self, which):
        self.which = which
    def __call__(self):
        if self.which == 0:
            return [["button", 1, True], ["button", 2, True], ["button", 1, False], ["button", 2, False]]
        return [["button", self.which, True], ["button", self.which, False]]

BTN_LEFT = Btn(1)
BTN_RIGHT = Btn(2)
BTN_BOTH = Btn(0)

def automation_txt(txts, btns):
    # See https://github.com/LedgerHQ/speculos/blob/master/doc/automation.md
    # for more information
    if isinstance(txts, str):
        txts = [txts]
    actions = itertools.chain(*(btn() for btn in btns))
    actions = list(actions)
    rules = [{
        "text": txt,
        "actions": actions
    } for txt in txts]
    return {"version": 1, "rules": rules}

def automation_accept_txt(txts):
    btns = [BTN_RIGHT] if speculos_model == "nanos" else [BTN_BOTH]
    return automation_txt(txts, btns)
def automation_refuse_txt(txts):
    btns = [BTN_LEFT] if speculos_model == "nanos" else [BTN_RIGHT, BTN_BOTH]
    return automation_txt(txts, btns)


class BaseTestCase:
    def setup_speculos(self, automation=None):
        return SetupSpeculos(self.API, automation)

    def erase_all_slots(self):
        with self.setup_speculos(automation_accept_txt(TXT_ERASE_ALL_SLOTS)) as dongle:
            self.API.erase_all_slots(dongle)

    def test_get_key_by_name_accepts(self):
        # If TXT_KEEPASS_OPEN_NAME appears, accepts it.
        automation_accept = automation_accept_txt(TXT_KEEPASS_OPEN_NAME)

        # Send the request twice, verify that it's the same key twice
        with self.setup_speculos(automation_accept) as dongle:
            key0 = self.API.get_key_from_name(dongle, b"perso")
            key1 = self.API.get_key_from_name(dongle, b"perso")
            self.assertEqual(key0, key1)
            # TODO: reproductible tests with test vectors for a given seed

    def test_get_key_by_name_refuses(self):
        # If TXT_KEEPASS_OPEN_NAME appears, refuses it.
        automation_refuse = automation_refuse_txt(TXT_KEEPASS_OPEN_NAME)
        with self.setup_speculos(automation_refuse) as dongle:
            with self.assertRaises(self.API.exception):
                self.API.get_key_from_name(dongle, b"perso")

    def test_store_key_erase_accepts(self):
        self.erase_all_slots()
        automation_accept = automation_accept_txt([TXT_ERASE,TXT_KEEPASS_OPEN_SLOT])
        key0 = bytes(random.getrandbits(8) for _ in range(32))
        key1 = bytes(random.getrandbits(8) for _ in range(32))

        with self.setup_speculos(automation_accept) as dongle:
            self.API.store_key_slot(dongle, 1, key0)
            key = self.API.get_key_slot(dongle, 1)
            self.assertEqual(key, key0)

            self.API.store_key_slot(dongle, 1, key1)
            key = self.API.get_key_slot(dongle, 1)
            self.assertEqual(key, key1)

    def test_store_key_erase_refuses(self):
        self.erase_all_slots()
        automation_refuse = automation_refuse_txt(TXT_ERASE)
        key0 = bytes(random.getrandbits(8) for _ in range(32))

        with self.setup_speculos(automation_refuse) as dongle:
            self.API.store_key_slot(dongle, 1, key0)
            with self.assertRaises(self.API.exception):
                self.API.store_key_slot(dongle, 1, key0)

    def test_store_get_key_slot(self):
        self.erase_all_slots()
        automation_accept = automation_accept_txt(TXT_KEEPASS_OPEN_SLOT)
        with self.setup_speculos(automation_accept) as dongle:
            for s in range(SLOT_COUNT):
                keyref = bytes(random.getrandbits(8) for _ in range(32))
                self.API.store_key_slot(dongle, s, keyref)
                key = self.API.get_key_slot(dongle, s)
                self.assertEqual(keyref, key)

    def test_get_invalid_key_slot(self):
        self.erase_all_slots()
        with self.setup_speculos() as dongle:
            for s in range(SLOT_COUNT):
                with self.assertRaises(self.API.exception):
                    self.API.get_key_slot(dongle, s)

    def test_get_valid_slots(self):
        self.erase_all_slots()
        key = bytes(random.getrandbits(8) for _ in range(32))
        slots = [0,2,4,6]
        with self.setup_speculos() as dongle:
            for s in slots:
                self.API.store_key_slot(dongle, s, key)
            recvslots = self.API.get_valid_slots(dongle)
            self.assertEqual(slots, recvslots)

    def test_invalid_name(self):
        with self.setup_speculos() as dongle:
            with self.assertRaises(self.API.exception):
                self.API.get_key_from_name(dongle, "tést".encode("utf8"))
