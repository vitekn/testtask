#include "connectionmanager.h"
#include <algorithm>
#include <event2/event.h>


ConnectionManager::ConnectionManager(event_base* events) : _events(events), _running(true), _internalEvents(false)
{
}

ConnectionManager::ConnectionManager() : _events(event_base_new()), _running(true), _internalEvents(true)
{
}

void ConnectionManager::addConnection(const std::shared_ptr<ConnectionProcessor>& processor)
{
    std::lock_guard<std::mutex> lock(_mutex);
    {
        if (_running) {
            processor->setOnCloseCb(std::bind(&ConnectionManager::onProcessorExit, this, processor));
            _processors.insert(processor);
        }
    }
}

void ConnectionManager::onProcessorExit(const std::weak_ptr<ConnectionProcessor>& processor)
{
    std::lock_guard<std::mutex> lock(_mutex);
    {
        if (_running) {
            _processors.erase(processor.lock());
        }
    }
}

void ConnectionManager::dropAll()
{
    std::lock_guard<std::mutex> lock(_mutex);
    {
        _running = true;
        _processors.clear();
    }
}

bool ConnectionManager::run()
{
    if (!_internalEvents) {
        return false;
    }
        
    event_base_dispatch(_events);
    return true;
    
}

bool ConnectionManager::stop()
{
    if (!_internalEvents) {
        return false;
    }
    event_base_loopbreak(_events);
    return true;
}   

