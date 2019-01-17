#ifndef INCLUDED_CONFIG_H
#define INCLUDED_CONFIG_H

#include <istream>
#include <unordered_map>

class Config
{
public:
    typedef std::pair<std::string, std::uint16_t> Host;

    Config();
    
    bool parseCmdLine(int argc, char** argv);
    bool read(std::istream& in);
    Host getHostById(int id) const;
    
    bool logToConsole() const;
    bool logToSyslog() const;
    const std::string& logFilename() const;
    int verbosity() const;
    const std::string& cfgFilename() const;
    uint16_t port() const;
    
private:
    typedef std::unordered_map<int, Host> Hosts;
    
    Hosts _hosts;
    int _verbosity;
    std::string _cfgFile;
    std::string _logFile;
    bool _useSyslog;
    bool _useConsole;
    uint16_t _port;
    
};

inline bool Config::logToConsole() const
{
    return _useConsole;
}

inline bool Config::logToSyslog() const
{
    return _useSyslog;
}

inline const std::string& Config::logFilename() const
{
    return _logFile;
}

inline int Config::verbosity() const
{
    return _verbosity;
}

inline const std::string & Config::cfgFilename() const
{
    return _cfgFile;
}

inline uint16_t Config::port() const
{
    return _port;
}

#endif
