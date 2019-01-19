#ifndef INCLUDED_PROXY_MANAGER_H
#define INCLUDED_PROXY_MANAGER_H
#include "connectionmanager.h"
#include "proxy.h"
#include <memory>
#include <unordered_map>


class ProxyManager {
public:
    
  
    ProxyManager(const std::shared_ptr<ConnectionManager>& cm);
    ~ProxyManager();
    
    
    void addProxy(const std::shared_ptr<Proxy>& pr);
    
    void dropById(int id);
    void dropAll();
    
private:
    typedef std::unordered_multimap<int, std::weak_ptr<ConnectionProcessor> > IdToConn;
    
    void onIdRecv(int id, const std::weak_ptr<ConnectionProcessor>& proxy);
    
    
    std::shared_ptr<ConnectionManager> _cm;
    IdToConn _idToConn;
    std::mutex _lock;
    
};

#endif

