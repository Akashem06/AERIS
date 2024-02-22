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

host_msg, host_addr = server_socket.recvfrom(BUFFER_SIZE)

print(f"Message from Client: {host_msg.decode()}, responding..." )

server_socket.sendto(str.encode("SERVER STARTED"), host_addr)

# TODO: Fix this while loop, if user interupts program it will
# continue to run until killed.
while(True):
    host_msg, host_addr = server_socket.recvfrom(BUFFER_SIZE)
    print(f"Recieved message: { ' '.join([hex(byte) for byte in host_msg]) }")


################################################################################
# REPACKAGING RECIEVED BINARY INTO CUSTOM DFU SCHEMA

################################################################################
# SENDING MESSAGE FUNCTIONS WITH SERIAL

################################################################################
# FORWARDING ACK/NACK MESSAGES TO THE HOST