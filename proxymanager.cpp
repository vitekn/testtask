#include "proxymanager.h"
#include <vector>
#include <algorithm>

ProxyManager::ProxyManager(const std::shared_ptr<ConnectionManager>& cm): _cm(cm)
{
}

ProxyManager::~ProxyManager()
{
    dropAll();
}

void ProxyManager::addProxy(const std::shared_ptr<Proxy>& pr)
{
    pr->setIdRcvClb(std::bind(&ProxyManager::onIdRecv, this, std::placeholders::_1, pr));
    _cm->addConnection(pr);
}

void ProxyManager::dropAll()
{
    _cm->dropAll();
    {
        std::lock_guard<std::mutex> lock(_lock);
        _idToConn.clear();
    }
}

void ProxyManager::dropById(int id)
{
    std::vector<std::weak_ptr<ConnectionProcessor> > allProxies;
    {
        std::lock_guard<std::mutex> lock(_lock);
        auto allProxiesR = _idToConn.equal_range(id);
        if (allProxiesR.first == allProxiesR.second) {
            return;
        }
        allProxies.reserve(std::distance(allProxiesR.second, allProxiesR.first));
        std::for_each(allProxiesR.first, allProxiesR.second,[&](const IdToConn::value_type& v){
            allProxies.push_back(v.second);
        });
        _idToConn.erase(id);
    }
    
    for(auto pr : allProxies){
        _cm->dropConnection(pr);
    }
}

void ProxyManager::onIdRecv(int id, const std::weak_ptr<ConnectionProcessor>& proxy)
{
    std::lock_guard<std::mutex> lock(_lock);
    _idToConn.insert({id, proxy});
}
