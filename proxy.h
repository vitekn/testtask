#ifndef INCLUDED_PROXY_H
#define INCLUDED_PROXY_H

#include "connectionprocessor.h"
#include "forwarder.h"
#include <memory>
#include "config.h"

class Proxy : public ConnectionProcessor
{
public:
    Proxy(const ClientConnection& connection, event_base *events, const std::shared_ptr<Config>& config);
    
    void dataRecieved(evbuffer * input) override;
    
private:
    static const int MSG_SIZE = 12;
    
    void onConnected(bool res);
    void onFwdClose();
    
    std::unique_ptr<Forwarder> _fwd;
    int _state;
    size_t _bytesRead;
    char _data[MSG_SIZE];
    std::shared_ptr<Config> _config;
    event_base *_events;
    evbuffer *_lastInput;
    
};

#endif
