#ifndef INCLUDED_JSON_PARSER_H
#define INCLUDED_JSON_PARSER_H
#include "controlmessage.h"
#include <istream>

class JsonParser{
public:
    bool parse(ControlMessage *out, std::istream& data);
};

#endif

