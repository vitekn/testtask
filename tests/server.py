import asyncio
from itertools import count
import threading


class EchoServerClientProtocol(asyncio.Protocol):

    def __init__(self, server):
        self.client_ct = len(server.get_clients_output())
        self._server = server

    def connection_made(self, transport):
        peername = transport.get_extra_info('peername')
        print('{} Connection from {}'.format(self.client_ct, peername))
        self.transport = transport
        self._server.add_client(self.client_ct)

    def data_received(self, data):
        self._server.append_client_data(self.client_ct, data)
        message = data.hex()
        print('{} data received: {!r}'.format(self.client_ct, message))

        self.transport.write(data)


class Server(threading.Thread):
    def __init__(self, port):
        threading.Thread.__init__(self)
        self._port = port
        self._proc = None
        self._loop = None
        self._clients = {}
        self.status = 0;

    def add_client(self, client_id):
        self._clients[client_id] = bytearray()

    def append_client_data(self, client_id, data):
        self._clients[client_id].extend(data)

    def get_clients_output(self):
        return self._clients

    def run(self):
        self._loop = asyncio.new_event_loop()
        loop = self._loop
        asyncio.set_event_loop(loop)
        print("creating server on port {}".format(self._port))
        coro = loop.create_server(lambda: EchoServerClientProtocol(self), '0.0.0.0', self._port)
        server = loop.run_until_complete(coro)
        if not server.sockets:
            print("Cannot listen")
            self.status = 1
            return

        print("running server")
        loop.run_forever()

        print("closing server")
        server.close()
        loop.run_until_complete(server.wait_closed())
        loop.close()
        print("exiting")

    def stop(self):
        self._loop.call_soon_threadsafe(self._loop.stop)

