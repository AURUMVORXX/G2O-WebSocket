#pragma once

class WebsocketClient
{
private:

    ix::WebSocket _client;
    std::string _url;
    
public:
    
    WebsocketClient() {};
    ~WebsocketClient();
    
    void Start();
    void Stop();
    
    bool GetRunning() { return running; }
    
    void SetUrl(std::string);
    std::string GetUrl() { return _url; }
    
    void Send(std::string);
    void SendBinary(std::string);
    
    void MessageHandler(const ix::WebSocketMessagePtr& msg);
    
public:
    
    int pingInterval{30};
    bool running{false};
    bool silent{false};
    
    // TLS options
    bool useTls{false};
    bool disableHostnameValidation{false};
    std::string certificateFilePath{""};
    std::string keyFilePath{""};
    std::string caFilePath{"NONE"};
};