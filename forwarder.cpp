#include "forwarder.h"
#include <event2/buffer.h>

Forwarder::Forwarder(const ClientConnection& connection, event_base* events) : ConnectionProcessor(connection, events), _destination(0)
{
}

Forwarder::Forwarder(event_base* events): ConnectionProcessor(events), _destination(0)
{
}


void Forwarder::dataRecieved(evbuffer* input)
{
    if (_destination) {
        _destination->sendData(input);
    }
    
}


