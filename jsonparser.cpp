#include "jsonparser.h"
#include <jsoncpp/json/reader.h>

bool JsonParser::parse(ControlMessage *out, std::istream& data)
{
    Json::CharReaderBuilder rbuilder;
    Json::Value root;
    std::string errs;
    if (!Json::parseFromStream(rbuilder, data, &root, &errs)) {
        return false;
    }
    
    if (root["client_id"].empty() || root["job"].empty() || !root["client_id"].isConvertibleTo(Json::intValue)) {
        return false;
    }
    if (root["job"].asString() != "close_sockets") {
        return false;
    }
    
    out->id = root["client_id"].asInt();
    out->type = out->id >= 0 ? ControlMessage::CLOSE : ControlMessage::CLOSE_ALL;
    return true;
}

