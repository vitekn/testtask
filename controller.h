#ifndef INCLUDED_CONTROLLER_H
#define INCLUDED_CONTROLLER_H
#include "proxymanager.h"
#include "controlmessage.h"

class Controller {
public:
    Controller(const std::shared_ptr<ProxyManager>& pm);
    
    void onControlMessage(const ControlMessage& msg);
    
private:
    std::shared_ptr<ProxyManager> _proxyManager;
    
};

#endif


