import unittest
from client import Client
from variable_lib import *

TEST_SERVERADDR = '127.0.0.1'
TEST_PORT = 1024

class TestClientClass(unittest.TestCase):
    def test_client_init(self):
        Client(TEST_SERVERADDR, TEST_PORT)