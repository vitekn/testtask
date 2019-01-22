#!/bin/bash

cmake .
make
make install
cp config.cfg /output/config.cfg
mkdir /output/docker-proxy
mkdir /output/docker-tests

cp docker/Dockerfile.proxy /output/docker-proxy/Dockerfile
cp /output/testtask /output/docker-proxy
cp /output/config.cfg /output/docker-proxy
cp -R scripts/* /output/docker-proxy

cp docker/Dockerfile.tests /output/docker-tests/Dockerfile
cp -R tests/* /output/docker-tests
