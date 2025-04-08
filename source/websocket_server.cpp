#include <ixwebsocket/IXWebSocketServer.h>
#include <sqapi.h>
#include <vector>
#include <iostream>

#include "json_config.h"
#include "websocket_server.h"

WebsocketServer::WebsocketServer(int port):
    _server(ix::WebSocketServer(port, "0.0.0.0"))
{
    ix::initNetSystem();
    
    _server.setOnClientMessageCallback(
        [this](std::shared_ptr<ix::ConnectionState> state, ix::WebSocket& ws, const ix::WebSocketMessagePtr& msg)
        {
            this->MessageHandler(state, ws, msg);
        }
    );
    
    ix::SocketTLSOptions tlsOptions;
    
    std::string caFile = JSONConfig::Get().GetCaFile();
    std::string certFile = JSONConfig::Get().GetCertFile();
    std::string keyFile = JSONConfig::Get().GetKeyFile();
    bool useTls = JSONConfig::Get().GetTlsEnabled();
    bool disableHostnameValidation = JSONConfig::Get().GetDisableHostnameValidation();
    
    if (useTls && !certFile.empty() && !keyFile.empty())
    {
        tlsOptions.tls = useTls;
        tlsOptions.certFile = certFile;
        tlsOptions.keyFile = keyFile;
    
        tlsOptions.disable_hostname_validation = disableHostnameValidation;
        
        if (!caFile.empty())
            tlsOptions.caFile = caFile;
        
        _server.setTLSOptions(tlsOptions);
        std::cout << "WebSocket server started at wss://0.0.0.0:" << port << std::endl;
    }
    else
        std::cout << "WebSocket server started at ws://0.0.0.0:" << port << std::endl;
    
    _serverThread = std::thread(
        [this]
        {
            if (this->_server.listen().first)
                this->_server.start();
        }
    );
    
    RegisterEvents();
    _initialized = true;
}

void WebsocketServer::RegisterEvents()
{
    Sqrat::Function addEvent(Sqrat::RootTable(), "addEvent");
    addEvent("onWebsocketMessage");
    addEvent("onWebsocketConnect");
    addEvent("onWebsocketDisconnect");
}

WebsocketServer::~WebsocketServer()
{
    Shutdown();
}

void WebsocketServer::Shutdown()
{
    if(_initialized)
    {
        _server.stop();
        if (_serverThread.joinable())
            _serverThread.join();
            
        _initialized = false;
    }
}

std::optional<const std::shared_ptr<ix::WebSocket>> WebsocketServer::GetClient(std::string url)
{
    for (auto&& client : _server.getClients())
    {
        if(client->getUrl() == url)
            return client;
    }
    
    return std::nullopt;
}

bool WebsocketServer::ValidateWhitelist(std::string ip)
{
    std::vector<std::string> whitelist = JSONConfig::Get().GetWhitelist();
    return std::find(whitelist.begin(), whitelist.end(), ip) != whitelist.end();
}

void WebsocketServer::MessageHandler(std::shared_ptr<ix::ConnectionState> state, ix::WebSocket& ws, const ix::WebSocketMessagePtr& msg)
{   
    Sqrat::Function callEvent(Sqrat::RootTable(), "callEvent");
    
    if (msg->type == ix::WebSocketMessageType::Open)
    {
        if (!ValidateWhitelist(state->getRemoteIp()))
        {
            ws.close(ix::WebSocketCloseConstants::kInternalErrorCode, "Host is not in the whitelist.");
            return;
        }
        
        std::string url;
        {
            std::stringstream ss;
            ss << "ws://" << state->getRemoteIp() << ":" << state->getRemotePort();
            url = ss.str();
        }
        ws.setUrl(url);
        callEvent("onWebsocketConnect", url);
    }
    
    if (!state->isTerminated())
    {
        if (msg->type == ix::WebSocketMessageType::Message)
            callEvent("onWebsocketMessage", ws.getUrl(), msg->str);
        else if (msg->type == ix::WebSocketMessageType::Close)
            callEvent("onWebsocketDisconnect", ws.getUrl());
    }
}