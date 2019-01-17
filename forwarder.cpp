#include "forwarder.h"
#include <event2/buffer.h>

#include <log4cpp/Category.hh>

using namespace log4cpp;
namespace {
    Category& logger = Category::getInstance("Forwarder");
}

Forwarder::Forwarder(const ClientConnection& connection, event_base* events) : ConnectionProcessor(connection, events), _destination(0)
{
}

Forwarder::Forwarder(event_base* events): ConnectionProcessor(events), _destination(0)
{
}

void Forwarder::dataRecieved(evbuffer* input)
{
    logger << Priority::DEBUG << "forwarding data from " << *this << " to " << *_destination;
    if (_destination) {
        if (!_destination->sendData(input)) {
            logger << Priority::ERROR << "can't transmit data";
        }
    }
}


