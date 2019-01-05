#include "server.h"

#include <iostream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>


void Server::acceptCbProxy(struct evconnlistener *listener,
                           evutil_socket_t fd,
                           struct sockaddr *address,
                           int socklen,
                           void *ctx)
{
    reinterpret_cast<Server*>(ctx)->onAcceptConnection(listener, fd, address, socklen);
}

void Server::errorCbProxy(evconnlistener* listener, void* ctx)
{
    Server *s = reinterpret_cast<Server*>(ctx);
    if (s->_onError) {
        s->_onError();
    }
}

void Server::onAcceptConnection(evconnlistener* listener, int fd, sockaddr* , int)
{
    
        if (_onClient) {
            _onClient({listener, fd});
        }
        
/*        struct event_base *base = evconnlistener_get_base(listener);
        struct bufferevent *bev = bufferevent_socket_new(
                base, fd, BEV_OPT_CLOSE_ON_FREE);

        bufferevent_setcb(bev, echo_read_cb, NULL, echo_event_cb, NULL);

        bufferevent_enable(bev, EV_READ|EV_WRITE);*/
}

void Server::init()
{
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(_port);
    addr.sin_addr.s_addr = INADDR_ANY;
    
    _listener = evconnlistener_new_bind(_base,
                                        &Server::acceptCbProxy,
                                        this,
                                        LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE,
                                        -1,
                                        (struct sockaddr*)&addr,
                                        sizeof(addr));
    evconnlistener_set_error_cb(_listener, &Server::errorCbProxy);
}

Server::Server(std::uint16_t port): _internalEvents(true), _base(event_base_new()), _port(port)
{
    init();
}


Server::Server( std::uint16_t port, event_base *base): _internalEvents(false), _base(base), _port(port)
{
    init();
}

Server::~Server()
{
    if (_listener) {
        evconnlistener_free(_listener);
    }
    
}


bool Server::run()
{
    if (!_internalEvents || !_base || !_listener) {
        return false;
    }
        
    event_base_dispatch(_base);
    return true;
}

void Server::stop()
{
    if (_internalEvents) {
        event_base_loopbreak(_base);
    }
}   
