#include <vector>
#include <string>
#include <fstream>
#include <nlohmann/json.hpp>

#include "json_config.h"

using json = nlohmann::json;

JSONConfig::JSONConfig()
{
    _port = 8080;
    _sendBinary = false;
    
    std::ifstream f("g2ows.json");
    if (!f.is_open())
        return;
    
    json data = json::parse(f);
    
    if (data.contains("port") && data["port"].is_number())
        _port = data["port"];
        
    if (data.contains("sendBinary") && data["sendBinary"].is_boolean())
        _sendBinary = data["sendBinary"];
        
    if (data.contains("whitelist") && data["whitelist"].is_array())
        _whitelist = data["whitelist"];
}