import unittest
from client import Message
from variable_lib import *

##################### SMALL BINARY TEST #####################
test_small_start_message = bytearray([
    0xAA, 0x01, 0x3F, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0xBB  
])
test_small_data_message = bytearray([
    0xAA, 0x02,
    0x54, 0x65, 0x73, 0x74, 0x20, 0x62, 0x69, 0x6E, 0x61, 0x72, 0x79, 0x20, 
    0x66, 0x69, 0x6C, 0x65, 0x0D, 0x0A, 0x54, 0x68, 0x69, 0x73, 0x20, 0x69,
    0x73, 0x20, 0x61, 0x20, 0x73, 0x6D, 0x61, 0x6C, 0x6C, 0x20, 0x62, 0x69,
    0x6E, 0x61, 0x72, 0x79, 0x20, 0x66, 0x69, 0x6C, 0x65, 0x20, 0x75, 0x73,
    0x65, 0x64, 0x20, 0x66, 0x6F, 0x72, 0x20, 0x74, 0x65, 0x73, 0x74, 0x69,
    0x6E, 0x67, 0x2E, 
    0xBB
])
# 63 bytes
test_small_binary_path = "tests/binary/test_small_binary"

##################### LARGE BINARY TEST #####################

test_medium_binary_path = "tests/binary/test_medium_binary"

test_large_binary_path = "tests/binary/centre_console"

def view_bytearray(message, byte_array):
    hex_string = ' '.join(['{:02X}'.format(byte) for byte in byte_array])
    print(message, hex_string, "\n")

class TestMessageClass(unittest.TestCase):
    def test_start_message(self):
        message_small = Message(binary_path=test_small_binary_path)
        start_packet_small = message_small.start()
        view_bytearray("START SMALL_BINARY PACKET:", start_packet_small)
        self.assertEqual(message_small._binary_size, 63)
        self.assertEqual(test_small_start_message, start_packet_small)

        message_medium = Message(binary_path=test_medium_binary_path)
        start_packet_medium = message_medium.start()
        view_bytearray("START MEDIUM_BINARY PACKET:", start_packet_medium)
        self.assertEqual(message_medium._binary_size, 1048)

        message_large = Message(binary_path=test_large_binary_path)
        start_pack_large = message_large.start()
        view_bytearray("START LARGE_BINARY PACKET:", start_pack_large)
        self.assertEqual(message_large._binary_size, 1162536)


    def test_data_message(self):
        message_small = Message(binary_path=test_small_binary_path)
        data_packet_small = message_small.data()
        view_bytearray("DATA SMALL_BINARY PACKET:", data_packet_small)

        binary_test_size = 0
        for byte in data_packet_small[2:-1]:
            binary_test_size += 1

        self.assertEqual(message_small._binary_size, binary_test_size)
        self.assertEqual(data_packet_small, test_small_data_message)

class TestSenderClass(unittest.TestCase):
    def test_send_chunkify(self):
        message = Message(binary_path=test_small_binary_path)


if __name__ == '__main__':
    unittest.main()