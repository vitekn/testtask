from subprocess import Popen, TimeoutExpired
from server import Server
from client import Client
import time
import unittest
import pika


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

    def test_rmq_close_clients(self):
        servers = [Server(8888), Server(8889), Server(8890)]
        clients = [
            Client(1, "127.0.0.1", 12345),
            Client(3, "127.0.0.1", 12345),
            Client(1, "127.0.0.1", 12345),
            Client(4, "127.0.0.1", 12345)]
        for server in servers:
            server.start()
        time.sleep(1)

        for client in clients:
            client.start()
            time.sleep(0.1)

        time.sleep(1);

        for client in clients:
            self.assertEqual(0, client.status)

        connection = pika.BlockingConnection(pika.ConnectionParameters('localhost'))
        channel = connection.channel()
        channel.queue_declare(queue='proxyCtl')
        channel.basic_publish(exchange='',
                              routing_key='proxyCtl',
                              body='{ "client_id":1, "job":"close_sockets"}')
        time.sleep(1)

        clients[0].join(1)
        self.assertFalse(clients[0].isAlive())
        clients[2].join(1)
        self.assertFalse(clients[2].isAlive())

        time.sleep(2);

        self.assertNotEqual(0, clients[0].status)
        self.assertNotEqual(0, clients[2].status)
        self.assertEqual(0, clients[1].status)
        self.assertEqual(0, clients[3].status)

        clients[1].stop()
        clients[1].join(5)
        clients[3].stop()
        clients[3].join(5)

        for server in servers:
            server.stop()
            server.join(5)

        self.assertEqual(clients[1].send_data, servers[1].get_clients_output()[0])
        self.assertEqual(clients[3].send_data, servers[2].get_clients_output()[0])


if __name__ == '__main__':
    unittest.main()

