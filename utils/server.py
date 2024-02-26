import socket
import serial
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

stm_serial = serial.Serial(COM_PORT, baudrate=BAUDRATE, timeout=5)


################################################################################
# REPACKAGING RECIEVED BINARY INTO CUSTOM DFU SCHEMA

def prv_forward_msg(client_msg):
    stm_serial.write(client_msg)
    try:
        data = stm_serial.read(12) # Wait for ACK
        if packet_check(data) and data[1] == AERIS_TYPE_ACK:
            prv_send_ack(data[2], data[3:7])
        else:
            prv_send_ack(PACKET_NACK, b'\xff\xff\xff\xff')
        
    except SerialTimeoutException:
        print("ACK reception timed out")
        prv_send_ack(PACKET_NACK, b'\xff\xff\xff\xff')

################################################################################
# SENDING MESSAGE FUNCTIONS WITH SERIAL

################################################################################
# FORWARDING ACK/NACK MESSAGES TO THE HOST

def prv_send_ack(packet_status, error_buffer):
    crc16 = (14).to_bytes(2, byteorder='little') # TODO: add custom crc16
    packet = bytearray()
    packet.append(AERIS_SOF)
    packet.append(packet_status)
    if isinstance(error_buffer, int):
        packet.extend(error_buffer.to_bytes(4, byteorder='little', signed=False))
    elif isinstance(error_buffer, bytes):
        packet.extend(error_buffer)
    else:
        server_socket.sendto(str.encode("SERVER: Failed to send error buffer"))
        packet.extend(b'\xff\xff\xff\xff')
    packet.extend(crc16)
    packet.append(AERIS_EOF)
    server_socket.sendto(packet, client_addr)

def prv_process_client_message(client_msg):
    if packet_check(client_msg) and client_msg[1] == AERIS_TYPE_START:
        prv_forward_msg(client_msg)
    elif packet_check(client_msg) and client_msg[1] == AERIS_TYPE_DATA:
        prv_forward_msg(client_msg)

def main():
    try:
        while True:
            client_msg, client_addr = server_socket.recvfrom(BUFFER_SIZE)

            if client_msg:
                prv_process_client_message(client_msg)
    except KeyboardInterrupt:
        print("Killing server.")
    finally:
        stm_serial.close()
        server_socket.close()

if __name__ == "__main__":
    main()