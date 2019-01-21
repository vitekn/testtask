# testtask

Proxy:

SERVER_1,2,3 go to /etc/hosts in the container as n1,2,3.example.com 

12345 - listening proxy port

-r "amqp://test:test@172.17.0.1/" - RabbitMQ URL

docker run -i -e SERVER_1='172.17.0.1' -e SERVER_2='172.17.0.1' -e SERVER_3="172.17.0.1" -p 172.17.0.1:12345:12345 -t proxy ./start.sh -v 5 -o -r "amqp://test:test@172.17.0.1/"

Tests:

TEST_PROXYADDR - proxy address, 12345 port is hardcoded in the tests.

TEST_RMQADDR, TEST_RMQUSER, TEST_RMQPASS RabbitMQ creds.

ports 8888-8890 - test servers.

docker run -i -e TEST_PROXYADDR=172.17.0.1 -e TEST_RMQADDR=172.17.0.1 -e TEST_RMQUSER=test -e TEST_RMQPASS=test -p 172.17.0.1:8888:8888 -p 172.17.0.1:8889:8889 -p 172.17.0.1:8890:8890 -t tests
