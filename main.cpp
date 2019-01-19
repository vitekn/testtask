#include "server.h"
#include "connectionmanager.h"
#include "config.h"
#include "proxy.h"
#include "proxymanager.h"
#include "controller.h"
#include "jsonparser.h"
#include "controlchannel.h"

#include <event2/event.h>

#include <iostream>
#include <event2/event.h>
#include <functional>
#include <fstream>
#include <signal.h>
#include <unistd.h>

#include <log4cpp/SyslogAppender.hh>
#include <log4cpp/OstreamAppender.hh>
#include <log4cpp/FileAppender.hh>
#include <log4cpp/Category.hh>

using namespace log4cpp;

event_base *base = 0;
bool run = true;

void sig_pipe(int signum)
{
    //ignore
}

void sig_handler(int signum)
{
    run = false;
    if (base)
    {
        event_base_loopbreak(base);
    }
}

log4cpp::Priority::Value toPriority(int l)
{
    static log4cpp::Priority::Value map[] = {
        log4cpp::Priority::ERROR,
        log4cpp::Priority::WARN,
        log4cpp::Priority::INFO,
        log4cpp::Priority::DEBUG};
        
    if (l > 3) l=3;
    
    return map[l];
}

int main(int argc, char **argv)
{
 
    signal(SIGINT, sig_handler);    
    signal(SIGQUIT, sig_handler);    
    signal(SIGABRT, sig_handler);    
    signal(SIGTERM, sig_handler);    
    
    signal(SIGPIPE, sig_pipe);
    
    base = event_base_new();
    if (!base){
        return -1;
    }
    std::shared_ptr<Config> cfg(new Config());
    if (!cfg->parseCmdLine(argc, argv)){
        return -1;
    }
    
    log4cpp::Category& root = log4cpp::Category::getRoot();
	root.setPriority(toPriority(cfg->verbosity()));

    if (cfg->logToConsole()) {
        log4cpp::Appender *appender = new log4cpp::OstreamAppender("console", &std::cout);
        appender->setLayout(new log4cpp::BasicLayout());
        root.addAppender(appender);
    }
    
    if (cfg->logToSyslog()) {
        log4cpp::Appender *appender = new log4cpp::SyslogAppender("proxy","proxy");
        appender->setLayout(new log4cpp::BasicLayout());
        root.addAppender(appender);
    }

    if (!cfg->logFilename().empty()) {
        log4cpp::Appender *appender = new log4cpp::FileAppender("default", cfg->logFilename());
        appender->setLayout(new log4cpp::BasicLayout());
        root.addAppender(appender);
    }

    std::ifstream cfgFl(cfg->cfgFilename());
    if (!cfg->read(cfgFl)) {
        root << Priority::ERROR << "can't read cfg file: " << cfg->cfgFilename();
        return -3;
    }
    
    Server            s(cfg->port(), base);
    s.setOnError([](){ event_base_loopbreak(base);});
    if (!s.init()) {
        root << Priority::ERROR << "can't init server";
        return -4;
    }

    
    std::shared_ptr<ConnectionManager> cm = std::make_shared<ConnectionManager>(base);
    std::shared_ptr<ProxyManager> pm = std::make_shared<ProxyManager>(cm);

    Controller ctl(pm);

    ControlChannel cc(base,"amqp://localhost/","proxyCtl", std::make_shared<JsonParser>());
    cc.setCtlMsgCallback(std::bind(&Controller::onControlMessage,&ctl, std::placeholders::_1));
    cc.start();
    
    s.setOnClientConnect([&](const ClientConnection& connection) {
        pm->addProxy(std::make_shared<Proxy>(connection, cm->getEvents(), cfg));
    });

    if (run) {
        root << Priority::DEBUG << "dispatching events.";
        event_base_dispatch(base);
        root << Priority::DEBUG << "exiting.";
    }
    
    root << Priority::DEBUG << "closing connections.";
    ctl.onControlMessage({ControlMessage::CLOSE_ALL,0});
    cc.stop();
    root << Priority::DEBUG << "cleanup.";
    event_base_free(base);
    return 0;

}
