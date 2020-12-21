import unittest
from app_base import BaseTestCase

from pykpl import PyKPL

class AppPyTestCase(BaseTestCase, unittest.TestCase):
    API = PyKPL

if __name__ == "__main__":
    unittest.main()
