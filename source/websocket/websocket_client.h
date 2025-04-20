#pragma once

class WebsocketClient : public WebsocketBase
{
private:

    ix::WebSocket* _client;
    
    void _MessageHandler(const ix::WebSocketMessagePtr& msg);
public:
    
    WebsocketClient() {};
    ~WebsocketClient();
    
    void Start();
    void Stop();
    void Send(std::string);
};