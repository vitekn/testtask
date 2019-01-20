#include "proxymanager.h"
#include <vector>
#include <algorithm>

#include <log4cpp/Category.hh>

using namespace log4cpp;
namespace {
    Category& logger = Category::getInstance("ProxyManager");
}

ProxyManager::ProxyManager(const std::shared_ptr<ConnectionManager>& cm): _cm(cm)
{
}

ProxyManager::~ProxyManager()
{
    dropAll();
}

void ProxyManager::addProxy(const std::shared_ptr<Proxy>& pr)
{
    logger << Priority::DEBUG << "adding proxy " << pr;

    pr->setOnCloseCb(std::bind(&ProxyManager::dropConnection, this, std::weak_ptr<Proxy>(pr)));
    pr->setIdRcvClb(std::bind(&ProxyManager::onIdRecv, this, std::placeholders::_1, std::weak_ptr<ConnectionProcessor>(pr)));
    _cm->addConnection(pr);
}

void ProxyManager::dropConnection(const std::weak_ptr<Proxy>& proxy)
{
    {
        const std::shared_ptr<Proxy>& pp = proxy.lock();
        if (pp) {
            if (pp->id() >= 0) {
                _idToConn.erase(pp->id());
            }
        }
    }
    
    _cm->dropConnection(proxy);
}


void ProxyManager::dropAll()
{
    logger << Priority::DEBUG << "dropping all";
    _cm->dropAll();
    {
        std::lock_guard<std::mutex> lock(_lock);
        _idToConn.clear();
    }
}

void ProxyManager::dropById(int id)
{
    logger << Priority::DEBUG << "dropping with id = " << id;
    std::vector<std::weak_ptr<ConnectionProcessor> > allProxies;
    {
        std::lock_guard<std::mutex> lock(_lock);
        auto allProxiesR = _idToConn.equal_range(id);
        if (allProxiesR.first == allProxiesR.second) {
            logger << Priority::INFO << "no clients with id = " << id;
            return;
        }
        std::for_each(allProxiesR.first, allProxiesR.second,[&](const IdToConn::value_type& v){
            allProxies.push_back(v.second);
        });
        logger << Priority::DEBUG << "dropping " << allProxies.size() << " client(s).";
        _idToConn.erase(id);
    }
    
    for(auto pr : allProxies){
        _cm->dropConnection(pr);
    }
}

void ProxyManager::onIdRecv(int id, const std::weak_ptr<ConnectionProcessor>& proxy)
{
    logger << Priority::DEBUG << "adding client received id = " << id;
    std::lock_guard<std::mutex> lock(_lock);
    _idToConn.insert({id, proxy});
}
