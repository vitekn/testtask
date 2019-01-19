#include "controlchannel.h"
#include <amqpcpp/libevent.h>
#include "amqpcpp.h"
#include "amqpcpp/linux_tcp.h"
#include "amqpcpp/libevent.h"
#include "amqpcpp/channel.h"

#include "event2/event.h"
#include <sstream>


ControlChannel::ControlChannel(event_base *events, const std::string& host, const std::string& queue, const std::shared_ptr<JsonParser>& jp)
: _queue(queue)
, _parser(jp)
, _handler(events)
, _connection(&_handler, AMQP::Address(host))
{

}

bool ControlChannel::start()
{
    
    AMQP::TcpChannel channelt(&_connection);
    AMQP::Channel& channel = static_cast<AMQP::Channel&>(channelt);
    
    auto onRcv = [this](const AMQP::Message &message, uint64_t, bool redelivered)
                 {
                     std::istringstream iss(std::string(message.body(), message.bodySize()));
                     ControlMessage cmsg;
                    if (!_parser->parse(&cmsg, iss)) {
                        //logerror
                        return;
                    }
                    if (_ctlCb) {
                        _ctlCb(cmsg);
                    }
                 };
    
    channel.declareQueue(_queue)
        .onSuccess([&](){
            channel.consume(_queue, AMQP::noack).onReceived(onRcv);
        }).onError([this](const char* msg){
            //log error
            //stop
            _connection.close();
            
        }).onFinalize([](){
            
        });
    
    
    return true;
    
}

bool ControlChannel::stop()
{
    _connection.close();
    return true;
}

ControlChannel::~ControlChannel()
{
    stop();
}



