################################################################################
# CAN BE USER MODIFIED

SERVERADDR = '127.0.0.1'
PORT = 1024
BUFFER_SIZE = 1024

################################################################################
# PACKET STRUCTURE. DO NOT MODIFY

AERIS_SOF = 0xAA
AERIS_EOF = 0XBB

AERIS_TYPE_START = 0X01
AERIS_TYPE_DATA = 0X02
AERIS_TYPE_ACK = 0x03

PACKET_ACK = 0x00
PACKET_NACK = 0x01

def packet_check(message):
    return message[0] == AERIS_SOF and message[-1] == AERIS_EOF
