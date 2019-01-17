#ifndef INCLUDED_CLIENTCONNECTION_H
#define INCLUDED_CLIENTCONNECTION_H
#include <netinet/in.h>

struct evconnlistener;

struct ClientConnection {
    evconnlistener* listener;
    int fd;
    sockaddr_in sa;
};

#endif
