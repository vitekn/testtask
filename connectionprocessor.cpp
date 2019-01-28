#include "connectionprocessor.h"
#include <event2/listener.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/dns.h>

#include <arpa/inet.h>
#include <ostream>
#include <log4cpp/Category.hh>

using namespace log4cpp;
namespace {
    Category& logger = Category::getInstance("ConnectionProcessor");
}

std::ostream& operator<<(std::ostream& os, const ConnectionProcessor& cp)
{
    return os << "Connection processor of " << (cp._incoming ? "incoming" : "outgoing") << " connection host: " << cp._host << " port: " << cp._port;
}


void ConnectionProcessor::readProxy(bufferevent* bev, void* ctx)
{
    logger << Priority::DEBUG << "readProxy got read event for ptr" << ctx;
    evbuffer *input = bufferevent_get_input(bev);
    ConnectionProcessor* cp = reinterpret_cast<ConnectionProcessor*>(ctx);
    cp->dataRecieved(input);
}

void ConnectionProcessor::eventProxy(bufferevent* bev, short events, void* ctx)
{
    logger << Priority::DEBUG << "eventProxy got connection event for ptr" << ctx;
    reinterpret_cast<ConnectionProcessor*>(ctx)->onEvent(events);
}

void ConnectionProcessor::onEvent(short events)
{
    logger << Priority::DEBUG << "onEvent " << events;
    if (events & BEV_EVENT_CONNECTED)
    {
        logger << Priority::DEBUG << "onEvent connected";
        _connecting = false;
        _connectCb(true);
    }
    else
    if (events & (BEV_EVENT_EOF | BEV_EVENT_ERROR)) {
        logger << Priority::DEBUG << "onEvent disconnected or error";
        if (_connecting) {
            _connectCb(false);
        } else {
            close();
        }
    }
}

ConnectionProcessor::ConnectionProcessor(const ClientConnection& connection, event_base* events): _eventbase(events), _connecting(false), _incoming(true)
{
    _dns_base = evdns_base_new(_eventbase, 1);
    logger << Priority::DEBUG << "creating - existing connection " << this;
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &connection.sa.sin_addr.s_addr, ip, INET_ADDRSTRLEN);
    _host = ip;
    _port = ntohs(connection.sa.sin_port);
    
    _bev = bufferevent_socket_new(events, connection.fd, BEV_OPT_CLOSE_ON_FREE);
    logger << Priority::DEBUG << "creating, bufferevent = " << _bev;

    bufferevent_setcb(_bev, &ConnectionProcessor::readProxy, NULL, &ConnectionProcessor::eventProxy, this);

    bufferevent_enable(_bev, EV_READ|EV_WRITE);
}

ConnectionProcessor::ConnectionProcessor(event_base* events): _eventbase(events), _connecting(false), _incoming(false)
{
    _dns_base = evdns_base_new(_eventbase, 1);
    logger << Priority::DEBUG << "creating - new connection " << this;
    _bev = bufferevent_socket_new(events, -1, BEV_OPT_CLOSE_ON_FREE);
    logger << Priority::DEBUG << "creating, bufferevent = " << _bev;

    bufferevent_setcb(_bev, &ConnectionProcessor::readProxy, NULL, &ConnectionProcessor::eventProxy, this);

    bufferevent_enable(_bev, EV_READ|EV_WRITE);
}

bool ConnectionProcessor::connect(const std::string& host, uint16_t port, ConnectionProcessor::OnConnectCb cb)
{
    logger << Priority::DEBUG << "connecting to " << host << ':' << port;
    _host = host;
    _port = port;
    _connecting = true;
    _connectCb = cb;
    
    
    const int rc = bufferevent_socket_connect_hostname(_bev, _dns_base, AF_UNSPEC, host.c_str(), port);
    
    if (0 != rc) {
        logger << Priority::ERROR << "can't connect to " << host << ':' << port;
    }
    
    return 0 == rc;
        
}


bool ConnectionProcessor::sendData(evbuffer* data)
{
    logger << Priority::DEBUG << "sendData " << *this;
    evbuffer *output = bufferevent_get_output(_bev);
    const int rc = evbuffer_add_buffer(output, data);
    if (0 != rc) {
        logger << Priority::ERROR << "can't send data to ";
    }
    
    return 0 == rc;
}


void ConnectionProcessor::close()
{
    logger << Priority::DEBUG << "close " << *this;
    if (_bev) {
        bufferevent_free(_bev);
        _bev = 0;
    }
    if (_closeCb) {
        logger << Priority::DEBUG << "invoking onclose" << *this;
        _closeCb();
    }
}

ConnectionProcessor::~ConnectionProcessor()
{
    logger << Priority::DEBUG << "~ConnectionProcessor " << *this << " buf = " << _bev;
    if (_bev){
        bufferevent_free(_bev);
        _bev = 0;
    }
    evdns_base_free(_dns_base,0);
}




