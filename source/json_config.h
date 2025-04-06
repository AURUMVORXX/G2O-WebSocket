#pragma once

class JSONConfig
{
private:

    int _port;
    bool _sendBinary;
    std::vector<std::string> _whitelist;
    
    JSONConfig();
    
public:
    
    static JSONConfig& Get()
    {
        static JSONConfig instance;
        return instance;
    }
    
    JSONConfig(const JSONConfig&) = delete;
    JSONConfig operator=(JSONConfig&) = delete;
    
    int GetPort() { return _port; }
    bool GetBinary() { return _sendBinary; }
    std::vector<std::string> GetWhitelist() { return _whitelist; }
};