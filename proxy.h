#ifndef INCLUDED_PROXY_H
#define INCLUDED_PROXY_H

#include "connectionprocessor.h"
#include "forwarder.h"
#include <memory>
#include "config.h"

class Proxy : public ConnectionProcessor
{
public:
    typedef std::function<void(int)> IdRcvCallback;
    
    Proxy(const ClientConnection& connection, event_base *events, const std::shared_ptr<Config>& config);
    ~Proxy();
    
    void dataRecieved(evbuffer * input) override;
    void setIdRcvClb(const IdRcvCallback& idClb);
    int id();
    
    
    
private:
    static const int MSG_SIZE = 12;
    
    void onConnected(bool res);
    void onFwdClose();
    
    std::unique_ptr<Forwarder> _fwd;
    int _state;
    size_t _bytesRead;
    char _data[MSG_SIZE];
    std::shared_ptr<Config> _config;
    evbuffer *_lastInput;
    int _id;
    IdRcvCallback _idClb;
    
};

inline int Proxy::id()
{
    return _id;
}

inline void Proxy::setIdRcvClb(const Proxy::IdRcvCallback& idClb)
{
    _idClb = idClb;
}



#endif
