#ifndef INCLUDED_SERVER_H
#define INCLUDED_SERVER_H

#include "clientconnection.h"

#include <string>
#include <cstdint>
#include <functional>

#include <event2/util.h>

struct evconnlistener;
struct sockaddr;
struct event_base;

class Server
{
public:
    typedef std::function<void()> OnErrorCb;
    typedef std::function<void(const ClientConnection&)> OnClientConnectCb;
    
    Server(std::uint16_t port);
    Server(std::uint16_t port, event_base* base = 0);
    ~Server();
    
    void setOnError(const OnErrorCb& onError);
    void setOnClientConnect(const OnClientConnectCb& onClient);
    
    bool init();
    bool run();
    void stop();
    
    bool isListening() const;
private:

    void onAcceptConnection(evconnlistener* listener, int fd, sockaddr*, int);
  
    static void acceptCbProxy(evconnlistener *listener,
                        evutil_socket_t fd,
                        sockaddr *address,
                        int socklen,
                        void *ctx);
    
    static void errorCbProxy(evconnlistener *listener, void *ctx);
    
    
    bool _internalEvents;
    struct evconnlistener *_listener;
    event_base         *_base;    
    std::uint16_t     _port;
    OnErrorCb         _onError;
    OnClientConnectCb _onClient;
    
    
};


inline void Server::setOnClientConnect(const Server::OnClientConnectCb& onClient)
{
    _onClient = onClient;
}

inline void Server::setOnError(const Server::OnErrorCb& onError)
{
    _onError = onError;
}

inline bool Server::isListening() const
{
    return _listener;
}


#endif

