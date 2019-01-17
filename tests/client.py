import socket
import struct
import os
import time
import threading


class Client(threading.Thread):
    def __init__(self, client_id, host, port):
        threading.Thread.__init__(self)
        self._id = client_id
        self._host = host
        self._port = port
        self.status = 0
        self._run = True
        self.send_data = bytearray()
        self.recv_data = bytearray()
        self.connected = False

    def run(self):
        msg1 = struct.pack("!III", 1, 4, self._id)

        sock = socket.socket()
        sock.settimeout(1)
        sock.connect((self._host, self._port))
        if sock.send(msg1) <= 0:
            print("send error")
            sock.close()
            self.status = 1
            return
        self.connected = True
        self.send_data.extend(msg1)

        data = os.urandom(100)
        ct = 12
        rct = 0
        while self._run:
            if sock.send(data) <= 0:
                print("send error")
                self.status = 2
                break

            self.send_data.extend(data)

            ct += 100
            print("send:{}".format(data.hex()))
            time.sleep(1)

            rcv = sock.recv(1024)
            if rcv is None:
                print("recv error")
                self.status = 3
                break

            self.recv_data.extend(rcv)
            rct += len(rcv)
            rstr = rcv.hex()
            print("recv:{}".format(rstr))

        sock.close()
        print("exit {}/{}", ct, rct)
        if self.status == 0:
            self.status = ct - rct

    def stop(self):
        self._run = False
