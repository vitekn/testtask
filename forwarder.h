#ifndef INCLUDED_FORWARDER_H
#define INCLUDED_FORWARDER_H

#include "connectionprocessor.h"

struct evbuffer;

class Forwarder : public ConnectionProcessor
{
public:
    Forwarder(event_base *events);
    Forwarder(const ClientConnection& connection, event_base *events);
    
    void dataRecieved(evbuffer *input) override;
    
    void setDestination (ConnectionProcessor* dest);
    
private:
    ConnectionProcessor *_destination;
};


inline void Forwarder::setDestination(ConnectionProcessor* dest)
{
    _destination = dest;
}

#endif
