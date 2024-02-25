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

def prv_forward_start(client_msg):
    stm_serial.write(client_msg)
    try:
        data = stm_serial.read(12) # Wait for ACK
        if packet_check(data) and data[1] == AERIS_TYPE_ACK and data[2] == PACKET_ACK:
            print ("Yay")
            prv_send_ack(data[2], data[3])

        elif packet_check(data) and data[1] == AERIS_TYPE_ACK and data[2] == PACKET_NACK:
            print ("Nay, forward the error package")
            # configure data[3] so it is actually the entire buffer
            prv_send_ack(data[2], data[3])
        else:
            #Define critical error. Most likely 0xffffffff
            prv_send_ack(PACKET_NACK, 0)
        
    except SerialTimeoutException:
        print("ACK reception timed out")
        prv_send_ack(PACKET_NACK, 0) # Same as above NACK

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

def prv_process_client_message(client_msg):
    if packet_check(client_msg) and client_msg[1] == AERIS_TYPE_START:
        prv_send_ack(AERIS_TYPE_ACK, 0)  # TODO: add error buffer
        # UNPACK/REPACK START MESSAGE + SEND ACK
    elif packet_check(client_msg) and client_msg[1] == AERIS_TYPE_DATA:
        prv_send_ack(AERIS_TYPE_ACK, 0)

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