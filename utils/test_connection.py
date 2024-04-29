import unittest
from client import Client
from variable_lib import *

class TestClientClass(unittest.TestCase):
    def test_client_init(self):
        Client.start(SERVERADDR, PORT)