#include "config.h"
#include <unistd.h>
#include <log4cpp/Category.hh>
 
using namespace log4cpp;
namespace {
    Category& logger = Category::getInstance("Config");
}

Config::Config()
: _verbosity(0)
, _cfgFile("config.cfg")
, _logFile()
, _useSyslog(false)
, _useConsole(false)
, _port(12345)
, _rmqStr("amqp://localhost/")
, _rmqQueueName("proxyCtl")
{
}


bool Config::read(std::istream& in)
{
    logger << Priority::DEBUG << "reading config";
    if (!in) {
        return false;
    }
    while (in) {
        std::string line;
        std::getline(in, line);
        if (!line.empty()) {
            std::istringstream is(line);
        
            int id;
            std::string host;
            uint16_t port;
            
            is >> id;
            if (!is) {
                logger << Priority::ERROR << "can't read id";
                return false; }
            is >> host;
            if (!is) {
                logger << Priority::ERROR << "can't read host";
                return false;
            }
            is >> port;
            if (!is && !is.eof()) {
                logger << Priority::ERROR << "can't read port";
                return false;
            }
            _hosts[id]=std::make_pair(host, port);
            logger << Priority::DEBUG <<"id: " << id << " host: " << host << " port: " << port;
        }
    }
    
    return true;
}

bool Config::parseCmdLine(int argc, char ** argv)
{
    int c ;
    while( ( c = getopt (argc, argv, "v:c:l:p:r:q:soh") ) != -1 ) 
    {
        switch(c)
        {
            case 'v':
                if(!optarg) return false;
                _verbosity = std::atoi(optarg);
                break;
            case 'c':
                if(!optarg) return false;
                _cfgFile = std::string(optarg) ;
                break;
            case 'l':
                if(!optarg) return false;
                _logFile = std::string(optarg) ;
                break;
            case 'p':
                if(!optarg) return false;
                _port = std::atoi(optarg) ;
                break;
            case 'r':
                if(!optarg) return false;
                _rmqStr = std::string(optarg) ;
                break;
            case 'q':
                if(!optarg) return false;
                _rmqQueueName = std::string(optarg) ;
                break;
            case 's':
                _useSyslog = true;
                break;
            case 'o':
                _useConsole = true;
                break;
            case 'h':
                std::cout << "Usage:\n"
                             "    testtask [-h] [-p port] [-v N] [-c config] [-l logfile] [-s] [-o] [-r RMQString] [-q RMQQueueName]\n"
                             "\n"
                             "    Defaults:\n"
                             "        -p (port)          = 12345\n"
                             "        -v (verbosity)     = 0\n"
                             "        -c (config file)   = config.cfg\n"
                             "        -l (log file)      = [no file]\n"
                             "        -s (log to syslog)\n"
                             "        -o (log to console)\n"
                             "        -r (rmq conn str)  = \"amqp://localhost/\"\n"
                             "        -q (rmq queue name)= \"proxyCtl\"\n\n";
                             return false;
            default:
                return false;
        }
    }   
    return true;
}


Config::Host Config::getHostById(int id) const
{
    Hosts::const_iterator it = _hosts.find(id);
    if (it != _hosts.end())
    {
        logger << Priority::DEBUG << "getHostById return "
            << it->second.first << ":" << it->second.second
            << " for id: " << id;
        return it->second;
    }
    logger << Priority::WARN << "no host found for id: " << id;
    return Host();
}

