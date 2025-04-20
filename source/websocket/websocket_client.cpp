#include <ixwebsocket/IXWebSocket.h>
#include <sqapi.h>
#include <vector>
#include <queue>
#include <iostream>

#include <websocket_util.h>
#include "websocket_base.h"
#include "websocket_client.h"

void WebsocketClient::Start()
{
    std::lock_guard<std::mutex> lock(_operationMutex);
    if (_running)
    {
        _log("[Websocket][Start] Client is already running");
        return;
    }
    
    _client = new ix::WebSocket();
    _client->setUrl(_url);
    _client->setPingInterval(30);
    _client->setTLSOptions(_getTLSOptions());
    
    _client->setOnMessageCallback(
        [this](const ix::WebSocketMessagePtr& msg)
        {
            this->_MessageHandler(msg);
        }
    );
    
    _client->start();
    WebsocketBase::Start();
    
    std::stringstream ss;
    ss << "[WebSocket][Start] Client started connection to "<< _url;
    _log(ss.str());
}

WebsocketClient::~WebsocketClient()
{
    Stop();
}

void WebsocketClient::Stop()
{
    std::lock_guard<std::mutex> lock(_operationMutex);
    if (!_running)
        return;
    
    _client->stop(1000, "Normal closure");
    WebsocketBase::Stop();
    
    delete _client;
    
    std::stringstream ss;
    ss << "[WebSocket][Start] Client at " << _url << " has been stopped";
    _log(ss.str());
}

void WebsocketClient::SetUrl(std::string url)
{
    if (_running)
    {
        _log("[WebSocket][SetUrl] Already running");
        return;
    }
    
    _url = url;
}

ix::SocketTLSOptions WebsocketClient::_getTLSOptions()
{
    ix::SocketTLSOptions tlsOptions;
    tlsOptions.tls  = !certificateFilePath.empty();
    tlsOptions.disable_hostname_validation = disableHostnameValidation;
    tlsOptions.certFile = certificateFilePath;
    tlsOptions.keyFile = keyFilePath;
    tlsOptions.caFile = caFilePath;
    
    return tlsOptions;
}

void WebsocketClient::_log(std::string message)
{
    if (silent)
        return;
        
    std::cout << message << std::endl;
}

void WebsocketClient::Send(std::string message)
{
    if (!_running)
    {
        _log("[WebSocket][Send] Client is stopped");
        return;
    }
    
    if (!is_utf8(message))
    {
        _log("[WebSocket][Send] Message is not valid UTF-8 string");
        return;
    }
    
    _client->send(message);
}

void WebsocketClient::_MessageHandler(const ix::WebSocketMessagePtr& msg)
{
    if (!_running)
        return;
        
    if (msg->type == ix::WebSocketMessageType::Open)
    {
        std::stringstream ss;
        ss << "[WebSocket][Start] Client connected to " << _url;
        _log(ss.str());

        _insertEvent([this]() {
            Sqrat::Function callEvent(Sqrat::RootTable(), "callEvent");
            callEvent("onWebsocketConnect", this, _url);
        });
    }
    
    else if (msg->type == ix::WebSocketMessageType::Close)
    {
        std::string message = msg->closeInfo.reason;
        
        std::stringstream ss;
        ss << "[WebSocket][Start] Client disconnected from " << _url << ". Reason: " << message;
        _log(ss.str());
        
        _insertEvent([this, message]() {
            Sqrat::Function callEvent(Sqrat::RootTable(), "callEvent");
            callEvent("onWebsocketClose", this, _url, message);
        });
    }
    
    else if (msg->type == ix::WebSocketMessageType::Message)
    {
        std::string message = msg->str;
        _insertEvent([this, message]() {
            Sqrat::Function callEvent(Sqrat::RootTable(), "callEvent");
            callEvent("onWebsocketMessage", this, _url, message);
        });
    }
}