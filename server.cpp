#include "server.h"

#include <iostream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <log4cpp/Category.hh>

using namespace log4cpp;
namespace {
    Category& logger = Category::getInstance("Server");
}

void Server::acceptCbProxy(struct evconnlistener *listener,
                           evutil_socket_t fd,
                           struct sockaddr *address,
                           int socklen,
                           void *ctx)
{
    logger << Priority::DEBUG << "accept for ptr " << ctx;
    reinterpret_cast<Server*>(ctx)->onAcceptConnection(listener, fd, address, socklen);
}

void Server::errorCbProxy(evconnlistener* listener, void* ctx)
{
    logger << Priority::DEBUG << "error for ptr " << ctx;
    Server *s = reinterpret_cast<Server*>(ctx);
    if (s->_onError) {
        s->_onError();
    }
}

void Server::onAcceptConnection(evconnlistener* listener, int fd, sockaddr* sa, int)
{
    if (_onClient) {
        logger << Priority::DEBUG << "onAcceptConnection";
        _onClient({listener, fd, *((sockaddr_in*)sa)});
    }
}

bool Server::init()
{
    logger << Priority::DEBUG << "init on port: " << _port;
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
    if (!_listener) {
        logger << Priority::ERROR << "can't bind to port: " << _port;
        return false;
    }
    
    evconnlistener_set_error_cb(_listener, &Server::errorCbProxy);
    return true;
}

Server::Server(std::uint16_t port): _internalEvents(true), _base(event_base_new()), _port(port)
{
}


Server::Server( std::uint16_t port, event_base *base): _internalEvents(false), _base(base), _port(port)
{
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
        logger << Priority::ERROR << "can't run mulformed instance on port: " << _port;
        return false;
    }
 
    logger << Priority::DEBUG << "running on port: " << _port;
        
    event_base_dispatch(_base);
    return true;
}

void Server::stop()
{
    logger << Priority::DEBUG << "stopping on port: " << _port;
 
    if (_internalEvents) {
        event_base_loopbreak(_base);
    }
}   
