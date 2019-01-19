#include "controller.h"

Controller::Controller(const std::shared_ptr<ProxyManager>& pm): _proxyManager(pm)
{
}

void Controller::onControlMessage(const ControlMessage& msg)
{
    switch (msg.type) {
        case ControlMessage::CLOSE:
            _proxyManager->dropById(msg.id);
            break;
        case ControlMessage::CLOSE_ALL:
            _proxyManager->dropAll();
            break;
        default:
            return;
    }
}
