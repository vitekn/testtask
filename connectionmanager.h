#ifndef INCLUDED_CONNECTIONMANAGER_H
#define INCLUDED_CONNECTIONMANAGER_H

#include "connectionprocessor.h"
#include "clientconnection.h"

#include <memory>
#include <mutex>
#include <unordered_set>


struct event_base;

class ConnectionManager {
    
public:
    ConnectionManager();
    ConnectionManager(event_base *events);
    
    void addConnection(const std::shared_ptr<ConnectionProcessor>& processor);
    
    bool run();
    bool stop();
    void dropAll();
    void dropConnection(const std::weak_ptr<ConnectionProcessor>& processor);
    
    event_base* getEvents();
    
    
private:
    
   // void onProcessorExit(const std::weak_ptr<ConnectionProcessor>& processor);
    
    event_base   *_events;
    
    std::unordered_set<std::shared_ptr<ConnectionProcessor> >  _processors;
    std::mutex _mutex;
    bool _running; 
    bool _internalEvents;
};

inline event_base * ConnectionManager::getEvents()
{
    return _events;
    
}


#endif
