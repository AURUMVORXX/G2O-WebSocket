#pragma once

class WebsocketServer
{
private:
    ix::WebSocketServer _server;
    std::thread _serverThread;
    bool _initialized = false;
    
public:
    
    WebsocketServer(){};
    WebsocketServer(int port);
    ~WebsocketServer();
    
    void Shutdown();
    
    void RegisterEvents();
    std::optional<const std::shared_ptr<ix::WebSocket>> GetClient(std::string);
    std::set<std::shared_ptr<ix::WebSocket>> GetClients() { return _server.getClients(); }
    
    bool ValidateWhitelist(std::string);
    
    void MessageHandler(std::shared_ptr<ix::ConnectionState>, ix::WebSocket&, const ix::WebSocketMessagePtr&);
};