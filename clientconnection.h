#ifndef INCLUDED_CLIENTCONNECTION_H
#define INCLUDED_CLIENTCONNECTION_H

struct evconnlistener;

struct ClientConnection {
    evconnlistener* listener;
    int fd;
};

#endif
