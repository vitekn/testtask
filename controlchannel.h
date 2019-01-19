#ifndef INCLUDED_CONTROL_CHANNEL_H
#define INCLUDED_CONTROL_CHANNEL_H
#include "controlmessage.h"
#include "jsonparser.h"
#include "amqpcpp.h"
#include "amqpcpp/linux_tcp.h"
#include "amqpcpp/libevent.h"
#include "event2/event.h"

#include <memory>

class ControlChannel{
public:
    
    ControlChannel (event_base *events, const std::string& host, const std::string& queue, const std::shared_ptr<JsonParser>& jp);
    ~ControlChannel();
    
    
    bool start();
    bool stop();
    
    void setCtlMsgCallback(const std::function<void (const ControlMessage&)>& clb);
    
private:
    std::string _login;
    std::string _pass;
    std::string _queue;
    std::shared_ptr<JsonParser> _parser;
    std::function<void (const ControlMessage&)> _ctlCb;
    bool _running;
    AMQP::LibEventHandler _handler;
    AMQP::TcpConnection _connection;

    
};

inline void ControlChannel::setCtlMsgCallback(const std::function<void (const ControlMessage&)>& clb)
{
    _ctlCb = clb;
}


#endif
