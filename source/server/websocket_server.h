#pragma once

class WebsocketServer
{
private:

    ix::WebSocketServer* _server;
    std::thread _serverThread;
    std::vector<std::string> _whitelist;
    
    bool _IsHostWhitelisted(std::string);
    
public:
    
    WebsocketServer() {};
    ~WebsocketServer();
    
    void Start();
    void Stop();
    
    bool GetRunning() { return running; }
    
    void Send(std::string, std::string);
    void SendBinary(std::string, std::string);
    void SendToAll(std::string);
    void SendBinaryToAll(std::string);
    void Disconnect(std::string, std::string);
    Sqrat::Array GetWhitelist();
    void SetWhitelist(Sqrat::Array&);
    void AddWhitelist(std::string);
    void RemoveWhitelist(std::string);
    
    void MessageHandler(std::shared_ptr<ix::ConnectionState>, ix::WebSocket&, const ix::WebSocketMessagePtr&);
    
public:
    
    int port{8080};
    bool running{false};
    bool silent{false};
    
    // TLS options
    bool useTls{false};
    bool disableHostnameValidation{false};
    std::string certificateFilePath{""};
    std::string keyFilePath{""};
    std::string caFilePath{"NONE"};
    
};