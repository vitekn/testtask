#!/bin/bash

echo "$SERVER_1 n1.example.com" >> /etc/hosts
echo "$SERVER_2 n2.example.com" >> /etc/hosts
echo "$SERVER_3 n3.example.com" >> /etc/hosts

./testtask "$@"