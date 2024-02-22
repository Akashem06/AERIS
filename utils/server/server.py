# import serial
import socket
import os
import time

################################################################################
# USER DEFINES
port = 1024
host = '127.0.0.1'
buffer_size = 1024

################################################################################
# UDP SERVER THAT RECIEVES BINARY CHUNKS OTA
def start_server():
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    server_socket.bind((host, port))

    host_msg, host_addr = server_socket.recvfrom(buffer_size)

    msg = "Message from Host {}".format(host_msg.decode('utf-8'))  
    print(msg)

    server_socket.sendto(str.encode("Received"), host_addr)

################################################################################
# REPACKAGING RECIEVED BINARY INTO CUSTOM DFU SCHEMA

################################################################################
# SENDING MESSAGE FUNCTIONS WITH SERIAL

################################################################################
# FORWARDING ACK/NACK MESSAGES TO THE HOST