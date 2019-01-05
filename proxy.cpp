#include "proxy.h"

#include <event2/buffer.h>

namespace {
    const int TAG_INT = 1;
    const int SIZE_INT = 4;
    
    enum State {
        BEGIN = 0,
        CONNECTING,
        FWD
    };
}

Proxy::Proxy(const ClientConnection& connection, event_base* events,const std::shared_ptr<Config>& config) : ConnectionProcessor(connection, events),_state(BEGIN), _config(config)
{

    
}

void Proxy::onConnected(bool res)
{
    if (!res) {
        //error!
    }
    
    _state = FWD;
    dataRecieved(_lastInput);
    _lastInput = 0;
}

void Proxy::onFwdClose()
{
    close();
}


void Proxy::dataRecieved(evbuffer* input)
{
    if (FWD == _state) {
        _fwd->sendData(input);
    } else if (BEGIN == _state) {
        if (evbuffer_get_length(input) < MSG_SIZE) {
            return;
        }
            
        int res = evbuffer_copyout(input, _data, MSG_SIZE);
        if (res != MSG_SIZE) {
            close();
            return;
        }

        int *iar = (int*)_data;
        if (TAG_INT == ntohl(*iar) && SIZE_INT == ntohl(*(iar+1))) {
            int id = ntohl(*(iar+2));
            Config::Host host = _config->getHostById(id);
            if (host.first.empty()) {
                close();
                return;
                //error!
            }
            _fwd.reset(new Forwarder(_events));
            _fwd->setOnCloseCb(std::bind(&Proxy::onFwdClose, this));
            _fwd->setDestination(this);
            _fwd->connect(host.first, host.second,std::bind(&Proxy::onConnected, this, std::placeholders::_1));
            _state = CONNECTING;
                
        } else {
            close();
            return;
            //error!
        }
    }
}

