#include <ixwebsocket/IXWebSocketServer.h>
#include <sqapi.h>
#include <vector>
#include <iostream>
#include <codecvt>
#include <locale>

#include <utf8_check.h>
#include "websocket_server.h"

bool WebsocketServer::_IsHostWhitelisted(std::string host)
{
    return std::find(_whitelist.begin(), _whitelist.end(), host) != _whitelist.end();
}

void WebsocketServer::Start()
{
    if (running)
    {
        std::cout << "[WebSocket][Start] Server is already running" << std::endl;
        return;
    }
    
    ix::initNetSystem(); // According to docs, should be called for correct work on Windows
    
    _server = new ix::WebSocketServer(port, "0.0.0.0");
    
    _server->setOnClientMessageCallback(
        [this](std::shared_ptr<ix::ConnectionState> state, ix::WebSocket& ws, const ix::WebSocketMessagePtr& msg)
        {
            this->MessageHandler(state, ws, msg);
        }
    );
    
    ix::SocketTLSOptions tlsOptions;
    tlsOptions.tls = useTls;
    tlsOptions.disable_hostname_validation = disableHostnameValidation;
    tlsOptions.certFile = certificateFilePath;
    tlsOptions.keyFile = keyFilePath;
    tlsOptions.caFile = caFilePath;
    _server->setTLSOptions(tlsOptions);
    
    _serverThread = std::thread(
        [this]
        {
            if (this->_server->listen().first)
            {
                this->_server->start();
                if (!this->silent)
                {
                    if (this->useTls)
                        std::cout << "[WebSocket][Start] Server is running on wss://0.0.0.0:" << port << std::endl;
                    else
                        std::cout << "[WebSocket][Start] Server is running on ws://0.0.0.0:" << port << std::endl;
                }
            }
        }
    );
    
    running = true;
}

WebsocketServer::~WebsocketServer()
{
    Stop();
}

void WebsocketServer::Stop()
{
    if(running)
    {
        _server->stop();
        if (_serverThread.joinable())
            _serverThread.join();
            
        delete _server;
        running = false;
        if (!silent)
            std::cout << "[Websocket][Stop] Server at 0.0.0.0:" << port << " has been stopped" << std::endl;
    }
}

void WebsocketServer::SetWhitelist(Sqrat::Array& whitelist)
{
    _whitelist.clear();
    
    Sqrat::Object::iterator it;
    while(whitelist.Next(it))
    {
        HSQOBJECT obj_host = it.getValue();
        Sqrat::string host = sq_objtostring(&obj_host);
        _whitelist.push_back(host);
    }
}

void WebsocketServer::AddWhitelist(std::string host)
{
    if(!_IsHostWhitelisted(host))
        _whitelist.push_back(host);
}

void WebsocketServer::RemoveWhitelist(std::string host)
{
    auto it = std::find(_whitelist.begin(), _whitelist.end(), host);
    if (it != _whitelist.end())
        _whitelist.erase(it);
}

Sqrat::Array WebsocketServer::GetWhitelist()
{
    Sqrat::Array sqHostArray(SqModule::vm);
    
    for (auto& host : _whitelist)
        sqHostArray.Append(host);
        
    return sqHostArray;
}

void WebsocketServer::Send(std::string host, std::string message)
{
    if (!running)
    {
        std::cout << "[WebSocket][Send] Server is stopped" << std::endl;
        return;
    }
    
    if (!is_utf8(message))
    {
        std::cout << "[WebSocket][Send] Message is not a valid UTF-8 string" << std::endl;
        return;
    }
    
    for(auto&& client : _server->getClients())
    {
        if (client->getUrl() == host)
        {
            client->send(message);
            break;
        }
    }
}

void WebsocketServer::SendBinary(std::string host, std::string message)
{
    if (!running)
    {
        std::cout << "[WebSocket][Send] Server is stopped" << std::endl;
        return;
    }
    
    for(auto&& client : _server->getClients())
    {
        if (client->getUrl() == host)
        {
            client->sendBinary(message);
            break;
        }
    }
}

void WebsocketServer::SendToAll(std::string message)
{
    if (!running)
    {
        std::cout << "[WebSocket][Send] Server is stopped" << std::endl;
        return;
    }
    
    if (!is_utf8(message))
    {
        std::cout << "[WebSocket][Send] Message is not a valid UTF-8 string" << std::endl;
        return;
    }
    
    for(auto&& client : _server->getClients())
    {
        client->send(message);
    }
}

void WebsocketServer::SendBinaryToAll(std::string message)
{
    if (!running)
    {
        std::cout << "[WebSocket][Send] Server is stopped" << std::endl;
        return;
    }
    
    for(auto&& client : _server->getClients())
    {
        client->sendBinary(message);
    }
}

void WebsocketServer::Disconnect(std::string host, std::string reason)
{
    if (!running)
    {
        std::cout << "[WebSocket][Send] Server is stopped" << std::endl;
        return;
    }
    
    for(auto&& client : _server->getClients())
    {
        if (client->getUrl() == host)
        {
            client->close(ix::WebSocketCloseConstants::kNormalClosureCode, reason);
            break;
        }
    }
}

void WebsocketServer::MessageHandler(std::shared_ptr<ix::ConnectionState> state, ix::WebSocket& ws, const ix::WebSocketMessagePtr& msg)
{   
    Sqrat::Function callEvent(Sqrat::RootTable(), "callEvent");
    
    if (msg->type == ix::WebSocketMessageType::Open)
    {
        if (!_IsHostWhitelisted(state->getRemoteIp()))
        {
            if (!silent)
                std::cout << "[Websocket][Message] Host " << state->getRemoteIp() << " tried to connect, but it's not whitelisted" << std::endl;
                
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
        
        if (!silent)
            std::cout << "[Websocket][Message] " << state->getRemoteIp() << " connected to the websocket server" << std::endl;
            
        callEvent("onWebsocketConnect", this, url);
    }
    
    if (!state->isTerminated())
    {
        if (msg->type == ix::WebSocketMessageType::Message)
            callEvent("onWebsocketMessage", this, ws.getUrl(), msg->str);
        else if (msg->type == ix::WebSocketMessageType::Close)
        {
            callEvent("onWebsocketDisconnect", this, ws.getUrl());
            if (!silent)
                std::cout << "[Websocket][Message] " << state->getRemoteIp() << " disconnected from the websocket server" << std::endl;
        }
    }
}