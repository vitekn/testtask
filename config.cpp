#include "config.h"
#include <unistd.h>

Config::Config()
:_verbosity(0)
,_cfgFile("config.cfg")
,_logFile()
,_useSyslog(false)
,_useConsole(false)
{
}


bool Config::read(std::istream& in)
{
    while (in){
        int id;
        std::string host;
        uint16_t port;
        
        in >> id;
        if (!in) return false;
        in >> host;
        if (!in) return false;
        in >> port;
        _hosts[id]=std::make_pair(host, port);
    }
    
    return true;
}

bool Config::parseCmdLine(int argc, char ** argv)
{
    int c ;
    while( ( c = getopt (argc, argv, "v:c:l:so") ) != -1 ) 
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
            case 's':
                _useSyslog = true;
                break;
            case 'o':
                _useConsole = true;
                break;
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
        return it->second;
    }
    return Host();
}

