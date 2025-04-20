#pragma once

struct WSClient
{
    std::string ipv4;
    int port;
    std::string url;
    std::vector<std::string> topics;
    ix::WebSocket* socket;
    
    bool operator==(const WSClient& other) const
    {
        return socket == other.socket;
    }
};

class WebsocketServer : public WebsocketBase
{
private:

    ix::WebSocketServer* _server;
    std::thread _serverThread;
    std::vector<std::string> _whitelist;
    
    std::vector<WSClient> _connectedClients;
    std::unordered_map<std::string, std::vector<WSClient>> _topics;
    
    bool _IsHostWhitelisted(std::string);
    
    std::vector<WSClient>::iterator _findClient(ix::WebSocket*);
    std::vector<WSClient>::iterator _findClient(std::string);
    
    void _MessageHandler(std::shared_ptr<ix::ConnectionState>, ix::WebSocket&, const ix::WebSocketMessagePtr&);
    ix::SocketTLSOptions _getTLSOptions();
    void _log(std::string);
    
public:
    
    WebsocketServer() {};
    ~WebsocketServer();
    
    void Start();
    void Stop();
    
    Sqrat::Array GetTopics(std::string);
    void Subscribe(std::string, std::string);
    void Unsubscribe(std::string, std::string);
    void Unsubscribe(std::string);
    
    void Send(std::string, std::string);
    void Close(std::string, std::string);
    
    Sqrat::Array GetWhitelist();
    void SetWhitelist(Sqrat::Array&);
    void AddWhitelist(std::string);
    void RemoveWhitelist(std::string);
    
    bool GetRunning() { return _running; }
    
    bool silent{false};
    int port{8080};
    
    bool disableHostnameValidation{false};
    std::string certificateFilePath{""};
    std::string keyFilePath{""};
    std::string caFilePath{"NONE"};
};