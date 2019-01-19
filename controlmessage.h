#ifndef INCLUDED_CONTROL_MESSAGE_H
#define INCLUDED_CONTROL_MESSAGE_H

struct ControlMessage {
    enum MessageType{
        CLOSE,
        CLOSE_ALL
    };
    int type;
    int id;
};

#endif

