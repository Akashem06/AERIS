import socket
import os
import time

from variable_lib import *

################################################################################
# USER DEFINES
serverAddrPort = (SERVERADDR, PORT)

################################################################################
# UDP SERVER THAT SENDS BINARY CHUNKS OTA
client_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
client_socket.sendto(str.encode("CLIENT STARTED"), serverAddrPort)

server_msg = client_socket.recvfrom(BUFFER_SIZE)  
print(f"Message from Server: {server_msg[0].decode()}.")
print(f"Server address/port: {server_msg[1]}.")
print("Connection established!")  

################################################################################
# PACKAGING INTO CUSTOM SCHEMA FOR SERVER
def send_binary_ota():
    path = input("Enter the relative path of the new firmware: ")
    if os.path.isfile(path):
        binary_byte_counter = 0
        binary_size = os.path.getsize(path) 

        binary = open(path, 'rb')

        while (binary_byte_counter < binary_size):
            chunk = binary.read(1024)
            binary_byte_counter += len(chunk)
            prv_data_send(chunk)
    else:
        print("ERROR: Project path does not exist.")

def prv_data_send(chunk):
    packet = bytearray()
    packet.append(AERIS_SOF)
    packet.append(AERIS_TYPE_DATA)
    packet.extend(chunk)
    packet.append(AERIS_EOF)
                
    client_socket.sendto(packet, serverAddrPort)
    print(f"Sending: {' '.join([hex(byte) for byte in packet])}")

################################################################################
# SENDING MESSAGE FUNCTIONS WITH SERIAL

################################################################################
# RECEIVING ACK/NACK MESSAGES + READING IT OUT TO USER