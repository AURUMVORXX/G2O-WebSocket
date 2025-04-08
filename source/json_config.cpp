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
        
    if (data.contains("tls") && data["tls"].is_object())
    {
        json tls = data["tls"];
        
        if (tls.contains("certFile") && data["certFile"].is_string())
            _certFile = tls["certFile"];
            
        if (tls.contains("keyFile") && data["keyFile"].is_string())
            _keyFile = tls["keyFile"];
            
        if (tls.contains("caFile") && data["caFile"].is_string())
            _caFile = tls["caFile"];
            
        if (tls.contains("enabled") && data["enabled"].is_boolean())
            _useTls = tls["enabled"];
            
        if (tls.contains("disableHostnameValidation") && data["disableHostnameValidation"].is_boolean())
            _useTls = tls["disableHostnameValidation"];
    }
}