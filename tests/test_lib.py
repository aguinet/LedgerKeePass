import unittest
from app_base import BaseTestCase

from libkpl import LibKPL

class AppLibTestCase(BaseTestCase, unittest.TestCase):
    API = LibKPL

if __name__ == "__main__":
    unittest.main()
