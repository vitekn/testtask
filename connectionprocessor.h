#ifndef INCLUDED_CONNECTIONPROCESSOR_H
#define INCLUDED_CONNECTIONPROCESSOR_H
#include "clientconnection.h"
#include <functional>

struct event_base;
struct evbuffer;
struct bufferevent;

class ConnectionProcessor
{
public:
    typedef std::function<void(evbuffer *input)> OnDataCb;
    typedef std::function<void()> OnCloseCb;
    typedef std::function<void(bool)> OnConnectCb;
    
    ConnectionProcessor(event_base *events);
    ConnectionProcessor(const ClientConnection& connection, event_base *events);
    virtual ~ConnectionProcessor();
    
//    void setOnDataCb(const OnDataCb& cb);
    void setOnCloseCb(const OnCloseCb& cb);

    void connect (const std::string& host, uint16_t port, OnConnectCb);
    virtual void close();
    
    void sendData(evbuffer *data);
    virtual void dataRecieved(evbuffer *input) = 0;
//    virtual void connectionClosed();
    
private:
    static void readProxy(bufferevent *bev, void *ctx);
    static void eventProxy(bufferevent *bev, short events, void *ctx);
  
    void onEvent(short events);
    
    event_base *_eventbase;
    bufferevent *_bev;
    OnConnectCb _connectCb;
    bool _connecting;
//    OnDataCb  _dataCb;
    OnCloseCb _closeCb;
};

inline void ConnectionProcessor::setOnCloseCb(const ConnectionProcessor::OnCloseCb& cb)
{
    _closeCb = cb;
}

/*
inline void ConnectionProcessor::setOnDataCb(const ConnectionProcessor::OnDataCb& cb)
{
    _dataCb = cb;
}
*/

#endif
