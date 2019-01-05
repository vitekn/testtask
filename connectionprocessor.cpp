#include "connectionprocessor.h"
#include <event2/listener.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/dns.h>

void ConnectionProcessor::readProxy(bufferevent* bev, void* ctx)
{
    evbuffer *input = bufferevent_get_input(bev);
    ConnectionProcessor* cp = reinterpret_cast<ConnectionProcessor*>(ctx);
    cp->dataRecieved(input);
/*    if (cp->_dataCb) {
        cp->_dataCb(input);
    }*/
}

void ConnectionProcessor::eventProxy(bufferevent* bev, short events, void* ctx)
{
    reinterpret_cast<ConnectionProcessor*>(ctx)->onEvent(events);
}

void ConnectionProcessor::onEvent(short events)
{
    if (events & BEV_EVENT_CONNECTED)
    {
        _connectCb(true);
    }
    else
    if (events & (BEV_EVENT_EOF | BEV_EVENT_ERROR)) {
        if (_connecting) {
            _connectCb(false);
        } else {
            close();
        }
            
/*        if (cp->_closeCb) {
            cp->_closeCb();
        }*/
    }
}

ConnectionProcessor::ConnectionProcessor(const ClientConnection& connection, event_base* events): _eventbase(events), _connecting(false)
{
    _bev = bufferevent_socket_new(events, connection.fd, BEV_OPT_CLOSE_ON_FREE);

    bufferevent_setcb(_bev, &ConnectionProcessor::readProxy, NULL, &ConnectionProcessor::eventProxy, this);

    bufferevent_enable(_bev, EV_READ|EV_WRITE);
}

ConnectionProcessor::ConnectionProcessor(event_base* events): _eventbase(events), _connecting(false)
{
    _bev = bufferevent_socket_new(events, -1, BEV_OPT_CLOSE_ON_FREE);

    bufferevent_setcb(_bev, &ConnectionProcessor::readProxy, NULL, &ConnectionProcessor::eventProxy, this);

    bufferevent_enable(_bev, EV_READ|EV_WRITE);
}

void ConnectionProcessor::connect(const std::string& host, uint16_t port, ConnectionProcessor::OnConnectCb)
{
    _connecting = true;
    struct evdns_base *dns_base;
    dns_base = evdns_base_new(_eventbase, 1);
    
    bufferevent_socket_connect_hostname(_bev, dns_base, AF_UNSPEC, host.c_str(), port);
}


void ConnectionProcessor::sendData(evbuffer* data)
{
    evbuffer *output = bufferevent_get_output(_bev);
    evbuffer_add_buffer(output, data);
}


void ConnectionProcessor::close()
{
    if (_bev) {
        bufferevent_free(_bev);
        _bev = 0;
    }
    if (_closeCb) {
        _closeCb();
    }
}

ConnectionProcessor::~ConnectionProcessor()
{
    if (_bev){
        bufferevent_free(_bev);
        _bev = 0;
    }
}




