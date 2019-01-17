from subprocess import Popen, TimeoutExpired
from server import Server
from client import Client
import time
import unittest


class Proxy:
    def __init__(self, config):
        self._app = "testtask"
        self._config = config
        self._proc = None

    def run(self):
        file = open("proxy.cfg", "w")
        file.write(self._config)
        file.flush()
        file.close()
        self._proc = Popen([self._app, "-c", "proxy.cfg", "-v", "5"])
        return self._proc.poll() is None

    def close(self):
        if self._proc.poll is not None:
            return False
        self._proc.terminate()
        try:
            self._proc.wait(timeout=5)
        except TimeoutExpired:
            self._proc.kill()
            return False
        return True


class ProxyTest(unittest.TestCase):
    def test_one_client_one_server(self):
        params = [[1, 8888], [3, 8889]]
        for param in params:
            s1 = Server(param[1])
            s1.start()

            time.sleep(1)

            c1 = Client(param[0], "127.0.0.1", 12345)
            c1.start()

            time.sleep(5)

            c1.stop()
            c1.join(5)

            s1.stop()
            s1.join(5)
            self.assertEqual(0, c1.status)
            self.assertEqual(c1.send_data, c1.recv_data)
            self.assertEqual(c1.send_data, s1.get_clients_output()[0])
        time.sleep(1)

    def test_two_clients_one_server(self):
        s1 = Server(8888)
        s1.start()

        time.sleep(1)

        c1 = Client(1, "127.0.0.1", 12345)
        c2 = Client(2, "127.0.0.1", 12345)
        c1.start()
        i = 0
        while not c1.connected and i < 10:
            time.sleep(0.1)
            ++i

        c2.start()

        time.sleep(5)

        c1.stop()
        c1.join(5)
        c2.stop()
        c2.join(5)

        s1.stop()
        s1.join(5)
        self.assertEqual(0, c1.status)
        self.assertEqual(c1.send_data, c1.recv_data)
        self.assertEqual(c1.send_data, s1.get_clients_output()[0])
        self.assertEqual(0, c2.status)
        self.assertEqual(c2.send_data, c2.recv_data)
        self.assertEqual(c2.send_data, s1.get_clients_output()[1])

    def test_many_clients_many_servers(self):
        servers = [Server(8888), Server(8889), Server(8890)]
        clients = [Client(1, "127.0.0.1", 12345), Client(3, "127.0.0.1", 12345), Client(4, "127.0.0.1", 12345)]
        for server in servers:
            server.start()
        time.sleep(1)

        for client in clients:
            client.start()

        time.sleep(5)

        for client in clients:
            client.stop()
            client.join(5)

        for server in servers:
            server.stop()
            server.join(5)

        for client in clients:
            self.assertEqual(0, client.status)
            self.assertEqual(client.send_data, client.recv_data)

        for i in range(3):
            self.assertEqual(clients[i].send_data, servers[i].get_clients_output()[0])


if __name__ == '__main__':
    unittest.main()

