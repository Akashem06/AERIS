import socket
import serial
import sys

from variable_lib import *

class Server:
    def __init__(self, com_port, baudrate, server_addr, port):
        self.com_port = com_port
        self.baudrate = baudrate
        self.server_addr = server_addr
        self.port = port

        try:
            self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            self.server_socket.bind((server_addr, port))

            client_msg, client_addr = self.server_socket.recvfrom(AERIS_MAX_PACKET_SIZE)
            print(f"Message from Client: {client_msg.decode()}, responding...")
            self.server_socket.sendto(str.encode("SERVER STARTED"), client_addr)

            self.stm_serial = serial.Serial(com_port, baudrate=baudrate, timeout=5)

            while True:
                client_msg, client_addr = self.server_socket.recvfrom(AERIS_MAX_PACKET_SIZE)
                if client_msg:
                    self.prv_process_client_message(client_msg, client_addr)

        except Exception as e:
            print(f"An error occurred: {str(e)}")
            self.stop()

    def prv_forward_msg(self, client_msg, client_addr):
        self.stm_serial.write(client_msg)
        try:
            data = self.stm_serial.read(12)  # Wait for ACK
            if packet_check(data) and data[1] == AERIS_TYPE_ACK:
                self.prv_send_ack(data[2], data[3:7], client_addr)
            else:
                self.prv_send_ack(PACKET_NACK, b'\xff\xff\xff\xff', client_addr)

        except serial.SerialTimeoutException:
            print("ACK reception timed out")
            self.prv_send_ack(PACKET_NACK, b'\xff\xff\xff\xff', client_addr)

    def prv_send_ack(self, packet_status, error_buffer, client_addr):
        crc16 = (14).to_bytes(2, byteorder='little')  # TODO: add custom crc16
        packet = bytearray()
        packet.append(AERIS_SOF)
        packet.append(packet_status)
        if isinstance(error_buffer, int):
            packet.extend(error_buffer.to_bytes(4, byteorder='little', signed=False))
        elif isinstance(error_buffer, bytes):
            packet.extend(error_buffer)
        else:
            self.server_socket.sendto(str.encode("SERVER: Failed to send error buffer"), client_addr)
            packet.extend(b'\xff\xff\xff\xff')
        packet.extend(crc16)
        packet.append(AERIS_EOF)
        self.server_socket.sendto(packet, client_addr)

    def prv_process_client_message(self, client_msg, client_addr):
        if packet_check(client_msg) and client_msg[1] == AERIS_TYPE_START:
            self.prv_forward_msg(client_msg, client_addr)
        elif packet_check(client_msg) and client_msg[1] == AERIS_TYPE_DATA:
            self.prv_forward_msg(client_msg, client_addr)

    def stop(self):
        try:
            self.stm_serial.close()
            self.server_socket.close()
            sys.exit(0)
        except AttributeError:
            pass

if __name__ == "__main__":
    COM_PORT = r'COM4'
    BAUDRATE = 115200
    SERVER_ADDR = "your_server_address"
    PORT = 12345  # Example port number

    server = Server(COM_PORT, BAUDRATE, SERVER_ADDR, PORT)
