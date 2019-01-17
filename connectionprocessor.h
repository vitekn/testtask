#ifndef INCLUDED_CONNECTIONPROCESSOR_H
#define INCLUDED_CONNECTIONPROCESSOR_H
#include "clientconnection.h"

#include <functional>

struct event_base;
struct evbuffer;
struct bufferevent;

class ConnectionProcessor
{
friend std::ostream& operator<<(std::ostream&, const ConnectionProcessor&);
    
public:
    typedef std::function<void(evbuffer *input)> OnDataCb;
    typedef std::function<void()> OnCloseCb;
    typedef std::function<void(bool)> OnConnectCb;
    
    ConnectionProcessor(event_base *events);
    ConnectionProcessor(const ClientConnection& connection, event_base *events);
    virtual ~ConnectionProcessor();
    
    void setOnCloseCb(const OnCloseCb& cb);

    bool connect (const std::string& host, uint16_t port, OnConnectCb);
    virtual void close();
    
    bool sendData(evbuffer *data);
    virtual void dataRecieved(evbuffer *input) = 0;
    
    event_base* getEvents();
    
private:
    static void readProxy(bufferevent *bev, void *ctx);
    static void eventProxy(bufferevent *bev, short events, void *ctx);
  
    void onEvent(short events);
    
    event_base *_eventbase;
    bufferevent *_bev;
    OnConnectCb _connectCb;
    bool _connecting;
    OnCloseCb _closeCb;
    bool _incoming;
    std::string _host;
    uint16_t _port;
};

inline void ConnectionProcessor::setOnCloseCb(const ConnectionProcessor::OnCloseCb& cb)
{
    _closeCb = cb;
}

inline event_base* ConnectionProcessor::getEvents(){
    return _eventbase;
}


std::ostream& operator<<(std::ostream& os, const ConnectionProcessor& cp);


#endif
