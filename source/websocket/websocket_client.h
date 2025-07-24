#pragma once

class WebsocketClient : public WebsocketBase
{
private:

    ix::WebSocket* _client;
    std::string _url{""};
    
    void _MessageHandler(ix::WebSocketMessageType, std::string);
    ix::SocketTLSOptions _getTLSOptions();
    void _log(std::string);
public:
    
    WebsocketClient();
    ~WebsocketClient();
    
    void Start();
    void Stop();
    void Send(std::string);
    
    void SetUrl(std::string);
    std::string GetUrl() { return _url; }
    bool GetRunning() { return _running; };
    
    bool silent{false};
    bool disableHostnameValidation{false};
    Sqrat::Table headers;
    std::string certificateFilePath{""};
    std::string keyFilePath{""};
    std::string caFilePath{"NONE"};
    
    Sqrat::Function onOpenHandler;
    Sqrat::Function onCloseHandler;
    Sqrat::Function onMessageHandler;
};
