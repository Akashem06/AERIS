import socket
import os
from variable_lib import *

class Message:
    _binary_path = None
    _binary_size = 0

    def __init__(self, binary_path):
        Message._binary_path = binary_path
        Message._binary_size = os.path.getsize(self._binary_path)

    @classmethod
    def data(cls):
        if os.path.isfile(cls._binary_path):
            with open(cls._binary_path, 'rb') as binary:
                packet = bytearray()
                packet.append(AERIS_SOF)
                packet.append(AERIS_TYPE_DATA)
                packet.extend(binary.read())
                packet.append(AERIS_EOF)
                return packet
        else:
            print("ERROR: Project path does not exist.")
        # print(f"Sending: {' '.join([hex(byte) for byte in packet])}")
    
    @classmethod
    def start(cls):
        if os.path.isfile(cls._binary_path):
            app_crc32 = (15).to_bytes(4, byteorder='little')

            packet = bytearray()
            packet.append(AERIS_SOF)
            packet.append(AERIS_TYPE_START)
            packet.extend(cls._binary_size.to_bytes(3, byteorder='little'))
            packet.extend(app_crc32)
            packet.append(AERIS_EOF)
            return packet
        else:
            print("ERROR: Project path does not exist.")

class Sender:
    client_socket = None
    serverAddrPort = None

    def __init__(self, serverAddrPort, client_socket):
        Sender._serverAddrPort = serverAddrPort
        Sender._client_socket = client_socket

    @classmethod
    def send(cls, packet):
        chunks_list = cls._chunkify(packet, AERIS_MAX_PACKET_SIZE)
        for chunk in chunks_list:
            cls._client_socket.sendto(packet, cls._serverAddrPort)
            print(f"SENT PACKAGE: {' '.join([hex(byte) for byte in packet])}")

    def _chunkify(self, data, size):
        return (data[pos:pos + size] for pos in range(0, len(data), size))

class Client:
    sender = None

    def __init__(self, server_addr, port):
        client_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        serverAddrPort = (server_addr, port)
        client_socket.sendto(str.encode("CLIENT STARTED"), serverAddrPort)
        server_msg, server_addr = client_socket.recvfrom(BUFFER_SIZE)
        print(f"Message from Server: {server_msg.decode()}.")
        print(f"Server address/port: {server_addr}.")
        print("Connection established!")

        Client._sender = Sender(serverAddrPort, client_socket)

    @classmethod
    def send_binary_ota(cls):
        path = input("Enter the relative path of the new firmware: ")
        message = Message(binary_path=path)
        start_packet = message.start()
        data_packet = message.data()
        cls._sender.send(start_packet)

        server_msg, server_addr = cls._client_socket.recvfrom(BUFFER_SIZE)

        if packet_check(server_msg) and server_msg[1] == AERIS_TYPE_ACK and server_msg[2] == PACKET_ACK:
            cls._sender.send(data_packet)
        else:
            print("ERROR: Server did not receive start.")

    def view_message(self, server_msg):
        print(f"RECEIVED MESSAGE: {' '.join([hex(byte) for byte in server_msg])}")

# Usage example:
client = Client(SERVERADDR, PORT)
# Client.send_binary_ota(client_socket, serverAddrPort)
