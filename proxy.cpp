#include "proxy.h"

#include <event2/buffer.h>
#include <log4cpp/Category.hh>

using namespace log4cpp;
namespace {
    Category& logger = Category::getInstance("Proxy");

    const int TAG_INT = 1;
    const int SIZE_INT = 4;
    
    enum State {
        BEGIN = 0,
        CONNECTING,
        FWD
    };
}

Proxy::Proxy(const ClientConnection& connection, event_base* events,const std::shared_ptr<Config>& config)
: ConnectionProcessor(connection, events)
, _state(BEGIN)
, _config(config)
, _id(-1)
{
}

void Proxy::onConnected(bool res)
{
    if (!res) {
        logger << Priority::ERROR << "can't connect to server for " << *this;
        return;
    }
    
    _state = FWD;
    dataRecieved(_lastInput);
    _lastInput = 0;
}

Proxy::~Proxy()
{
    _fwd.reset();
}


void Proxy::onFwdClose()
{
    logger << Priority::DEBUG << "server " << *_fwd << " closed connection, closing " << *this;
    close();
}


void Proxy::dataRecieved(evbuffer* input)
{
    if (FWD == _state) {
        logger << Priority::DEBUG << "transmitting data to server " << *_fwd << " from " << *this;
        _fwd->sendData(input);
    } else if (BEGIN == _state) {
        _lastInput = input;
        logger << Priority::DEBUG << "parsing 1st message " << *this;
        if (evbuffer_get_length(input) < MSG_SIZE) {
            logger << Priority::DEBUG << "not enough data " << *this;
            return;
        }
            
        int res = evbuffer_copyout(input, _data, MSG_SIZE);
        if (res != MSG_SIZE) {
            logger << Priority::ERROR << "can't read message, closing " << *this;
            close();
            return;
        }

        int *iar = (int*)_data;
        if (TAG_INT == ntohl(*iar) && SIZE_INT == ntohl(*(iar+1))) {
            int id = ntohl(*(iar+2));
            Config::Host host = _config->getHostById(id);
            if (host.first.empty()) {
                logger << Priority::ERROR << "id is not configured, closing " << *this;
                close();
                return;
            }
            
            _id = id;
            if (_idClb) {
                _idClb(_id);
            }
            
            _fwd.reset(new Forwarder(getEvents()));
            _fwd->setOnCloseCb(std::bind(&Proxy::onFwdClose, this));
            _fwd->setDestination(this);
             if (!_fwd->connect(host.first, host.second,std::bind(&Proxy::onConnected, this, std::placeholders::_1))){
                logger << Priority::ERROR << "can't connect to server, closing " << *this;
                close();
                return;
            }
            _state = CONNECTING;
                
        } else {
            logger << Priority::ERROR << "can't parse message, closing " << *this;
            close();
            return;
            //error!
        }
    }
}

