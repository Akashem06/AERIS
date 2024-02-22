# import serial
import socket
import os
import time

from variable_lib import *

################################################################################
# USER DEFINES
COM_PORT = r'COM4'
BAUDRATE = 115200

################################################################################
# UDP SERVER THAT RECIEVES BINARY CHUNKS OTA
server_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
server_socket.bind((SERVERADDR, PORT))

client_msg, client_addr = server_socket.recvfrom(BUFFER_SIZE)

print(f"Message from Client: {client_msg.decode()}, responding..." )

server_socket.sendto(str.encode("SERVER STARTED"), client_addr)

################################################################################
# REPACKAGING RECIEVED BINARY INTO CUSTOM DFU SCHEMA

################################################################################
# SENDING MESSAGE FUNCTIONS WITH SERIAL

################################################################################
# FORWARDING ACK/NACK MESSAGES TO THE HOST

def prv_send_ack(packet_status, error_buffer):
    crc16 = (14).to_bytes(2, byteorder='little') # TODO: add custom crc16
    packet = bytearray()
    packet.append(AERIS_SOF)
    packet.append(packet_status)
    packet.extend(error_buffer.to_bytes(4, byteorder='little'))
    packet.extend(crc16)
    packet.append(AERIS_EOF)
    print(f"DATA: {' '.join([hex(byte) for byte in packet])}")
    server_socket.sendto(packet, client_addr)

# TODO: Fix this while loop, if user interupts program it will
# continue to run until killed.

while(True):
    client_msg, client_addr = server_socket.recvfrom(BUFFER_SIZE)

    if packet_check(client_msg) and client_msg[1] == AERIS_TYPE_START:
        prv_send_ack(AERIS_TYPE_ACK, 0) # TODO: add error buffer
        # UNPACK/REPACK START MESSAGE + SEND ACK
    elif  packet_check(client_msg) and client_msg[1] == AERIS_TYPE_DATA:
        prv_send_ack(AERIS_TYPE_ACK, 0)