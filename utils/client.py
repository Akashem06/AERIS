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

server_msg, server_addr = client_socket.recvfrom(BUFFER_SIZE)
print(f"Message from Server: {server_msg.decode()}.")
print(f"Server address/port: {server_addr}.")
print("Connection established!")

################################################################################
# PACKAGING INTO CUSTOM SCHEMA FOR SERVER
def send_binary_ota():
    path = input("Enter the relative path of the new firmware: ")
    if os.path.isfile(path):
        binary_size = os.path.getsize(path)
        prv_send_start(binary_size, 15)

        server_msg, server_addr = client_socket.recvfrom(BUFFER_SIZE)

        if packet_check(server_msg) and server_msg[1] == AERIS_TYPE_ACK and server_msg[2] == PACKET_ACK:
            binary_byte_counter = 0
            binary = open(path, 'rb')
            # CHUNKIFY
            while (binary_byte_counter < binary_size):
                chunk = binary.read(512)
                binary_byte_counter += len(chunk)
                prv_send_data(chunk)
        else:
            print("ERROR: Server did not recieve start.")
    else:
        print("ERROR: Project path does not exist.")

def prv_send_data(chunk):
    packet = bytearray()
    packet.append(AERIS_SOF)
    packet.append(AERIS_TYPE_DATA)
    packet.extend(chunk)
    packet.append(AERIS_EOF)
                
    client_socket.sendto(packet, serverAddrPort)
    
    print(len(packet))
    # print(f"Sending: {' '.join([hex(byte) for byte in packet])}")

def prv_send_start(binary_size, crc32):
    crc16 = (14).to_bytes(2, byteorder='little') # TODO: add custom crc16
    packet = bytearray()
    packet.append(AERIS_SOF)
    packet.append(AERIS_TYPE_START)
    packet.extend(binary_size.to_bytes(3, byteorder='little'))
    packet.extend(crc32.to_bytes(4, byteorder='little'))
    packet.extend(crc16)
    packet.append(AERIS_EOF)

    client_socket.sendto(packet, serverAddrPort)
    print(f"SENT START PACKAGE: {' '.join([hex(byte) for byte in packet])}")
    
################################################################################
# RECEIVING ACK/NACK MESSAGES + READING IT OUT TO USER

def prv_view_message(server_msg):
    print(f"RECEIVED MESSAGE: {' '.join([hex(byte) for byte in server_msg])}")
