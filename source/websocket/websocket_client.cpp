#include <ixwebsocket/IXWebSocket.h>
#include <sqapi.h>
#include <vector>
#include <queue>

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
    
    _log(std::format("[WebSocket][Start] Client started connection to {}", _url));
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
    
    _log(std::format("[Websocket][Stop] Client at {} has been stopped", _url));
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
        _log(std::format("[WebSocket][Start] Client connected to {}", _url));
        _insertEvent([this]() {
            Sqrat::Function callEvent(Sqrat::RootTable(), "callEvent");
            callEvent("onWebsocketConnect", this, _url);
        });
    }
    
    else if (msg->type == ix::WebSocketMessageType::Close)
    {
        std::string message = msg->closeInfo.reason;
        _log(std::format("[WebSocket][Start] Client disconnected from {}. Reason: {}", _url, message));
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