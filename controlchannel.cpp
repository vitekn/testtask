#include "controlchannel.h"
#include <amqpcpp/libevent.h>
#include "amqpcpp.h"
#include "amqpcpp/linux_tcp.h"
#include "amqpcpp/libevent.h"
#include "amqpcpp/channel.h"

#include "event2/event.h"

#include <log4cpp/Category.hh>

#include <sstream>

using namespace log4cpp;
namespace {
    Category& logger = Category::getInstance("ControlChannel");
}

ControlChannel::ControlChannel(event_base *events, const std::string& host, const std::string& queue, const std::shared_ptr<JsonParser>& jp)
: _queue(queue)
, _parser(jp)
, _handler(events)
, _connection(&_handler, AMQP::Address(host))
, _channel(&_connection)
{

}

bool ControlChannel::start()
{
    logger << Priority::DEBUG << "starting";

/*    if (!_connection.ready()) {
        logger << Priority::ERROR << "connection failed";
        return false;
    }*/
    
    
    auto onRcv = [this](const AMQP::Message &message, uint64_t, bool redelivered)
                 {
                     std::string msg(message.body(), message.bodySize());
                     logger << Priority::DEBUG << "ctl message received: \"" << msg << "\"";
                     
                     std::istringstream iss(msg);
                     ControlMessage cmsg;
                     if (!_parser->parse(&cmsg, iss)) {
                         logger << Priority::ERROR << "can't parse message";
                         return;
                     }
                     if (_ctlCb) {
                         _ctlCb(cmsg);
                     }
                  };
    _channel.onError([this](const char* msg){
            logger << Priority::ERROR << "channel error " << msg;
            _connection.close();
        });

    _channel.onReady([](){logger << Priority::DEBUG << "channel ready" ;});
    
    ;
    
    _channel.declareQueue(_queue)
        .onSuccess([this](){
            logger << Priority::DEBUG << "queue opened";
            _channel.consume(_queue).onReceived([this](const AMQP::Message &message, uint64_t dt, bool redelivered)
                 {
                     _channel.ack(dt);
                     std::string msg(message.body(), message.bodySize());
                     logger << Priority::DEBUG << "ctl message received: \"" << msg << "\"";
                     
                     std::istringstream iss(msg);
                     ControlMessage cmsg;
                     if (!_parser->parse(&cmsg, iss)) {
                         logger << Priority::ERROR << "can't parse message";
                         return;
                     }
                     if (_ctlCb) {
                         _ctlCb(cmsg);
                     }
                  });
        });
    
        
        
    return true;
}

bool ControlChannel::stop()
{
    logger << Priority::DEBUG << "stopping";
    _connection.close();
    return true;
}

ControlChannel::~ControlChannel()
{
    stop();
}



