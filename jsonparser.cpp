#include "jsonparser.h"
#include <jsoncpp/json/reader.h>

#include <log4cpp/Category.hh>

using namespace log4cpp;
namespace {
    Category& logger = Category::getInstance("JsonParser");
}

bool JsonParser::parse(ControlMessage *out, std::istream& data)
{
    Json::CharReaderBuilder rbuilder;
    Json::Value root;
    std::string errs;
    if (!Json::parseFromStream(rbuilder, data, &root, &errs)) {
        logger << Priority::ERROR << "can't parse json data";
        return false;
    }
    
    if (root["client_id"].empty() || root["job"].empty() || !root["client_id"].isConvertibleTo(Json::intValue)) {
        logger << Priority::ERROR << "wrong message data";
        return false;
    }
    if (root["job"].asString() != "close_sockets") {
        logger << Priority::ERROR << "wrong job type";
        return false;
    }
    
    out->id = root["client_id"].asInt();
    out->type = out->id >= 0 ? ControlMessage::CLOSE : ControlMessage::CLOSE_ALL;
    return true;
}

