import socket
import os
import time

################################################################################
# USER DEFINES
port = 1024
serverAddrPort = ("127.0.0.1", 1024)
buffer_size = 1024

################################################################################
# UDP SERVER THAT SENDS BINARY CHUNKS OTA
def start_host():
    host_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    host_socket.sendto( str.encode("AryanKashem"), serverAddrPort)
    time.sleep(2)
    server_msg = host_socket.recvfrom(buffer_size)  
    msg = "Message from Server {}".format(server_msg[0].decode())  
    print(msg)


################################################################################
# PACKAGING INTO CUSTOM SCHEMA FOR SERVER

################################################################################
# SENDING MESSAGE FUNCTIONS WITH SERIAL

################################################################################
# RECEIVING ACK/NACK MESSAGES + READING IT OUT TO USER