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
    if(_initialized)
    {
        _server.stop();
        if (_serverThread.joinable())
            _serverThread.join();
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
    for (auto s : whitelist)
    {
        std::cout << s << std::endl;
    }
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