#pragma once

class JSONConfig
{
private:

    int _port;
    bool _sendBinary;
    std::vector<std::string> _whitelist;
    
    std::string _certFile;
    std::string _keyFile;
    std::string _caFile;
    bool _useTls = false;
    bool _disableHostnameValidation = false;
    
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
    
    bool GetTlsEnabled() { return _useTls; }
    bool GetDisableHostnameValidation() { return _disableHostnameValidation; }
    std::string GetCertFile() { return _certFile; }
    std::string GetKeyFile() { return _keyFile; }
    std::string GetCaFile() { return _caFile; }
};