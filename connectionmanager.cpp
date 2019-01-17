#include "connectionmanager.h"
#include <algorithm>
#include <event2/event.h>
#include <log4cpp/Category.hh>

using namespace log4cpp;
namespace {
    Category& logger = Category::getInstance("ConnectionManager");
}

ConnectionManager::ConnectionManager(event_base* events) : _events(events), _running(true), _internalEvents(false)
{
}

ConnectionManager::ConnectionManager() : _events(event_base_new()), _running(true), _internalEvents(true)
{
}

void ConnectionManager::addConnection(const std::shared_ptr<ConnectionProcessor>& processor)
{
    logger << Priority::DEBUG << "addConnection " << processor;
    std::lock_guard<std::mutex> lock(_mutex);
    {
        if (_running) {
            processor->setOnCloseCb(std::bind(&ConnectionManager::onProcessorExit, this, processor));
            _processors.insert(processor);
            logger << Priority::DEBUG << "addConnection added " << processor;
        }
    }
}

void ConnectionManager::onProcessorExit(const std::weak_ptr<ConnectionProcessor>& processor)
{
    logger << Priority::DEBUG << "onProcessorExit";
    std::lock_guard<std::mutex> lock(_mutex);
    {
        if (_running) {
            const std::shared_ptr<ConnectionProcessor> scp = processor.lock();
            if (scp) {
                logger << Priority::DEBUG << "onProcessorExit closing " << *scp;
                _processors.erase(processor.lock());
            }
        }
    }
}

void ConnectionManager::dropAll()
{
    logger << Priority::DEBUG << "dropAll";
    std::lock_guard<std::mutex> lock(_mutex);
    {
        _running = true;
        _processors.clear();
    }
}

bool ConnectionManager::run()
{
    logger << Priority::DEBUG << "run";
    if (!_internalEvents) {
        logger << Priority::ERROR << "can't run extarnally driven instance";
        return false;
    }
        
    event_base_dispatch(_events);
    return true;
    
}

bool ConnectionManager::stop()
{
    logger << Priority::DEBUG << "stop";
    if (!_internalEvents) {
        logger << Priority::ERROR << "can't stop extarnally driven instance";
        return false;
    }
    event_base_loopbreak(_events);
    return true;
}   

