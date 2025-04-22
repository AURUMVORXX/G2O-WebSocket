#include <ixwebsocket/IXWebSocket.h>
#include <sqapi.h>
#include <vector>
#include <queue>
#include <iostream>

#include <websocket_util.h>
#include "websocket_base.h"
#include "websocket_client.h"

WebsocketClient::WebsocketClient()
{
    _client = new ix::WebSocket();
    _client->setPingInterval(30);
    _client->setOnMessageCallback(
        [this](const ix::WebSocketMessagePtr& msg)
        {
            ix::WebSocketMessageType msgType = msg->type;
            std::string message;
            if (msgType == ix::WebSocketMessageType::Close)
                message = msg->closeInfo.reason;
            else
                message = msg->str;
            
            _insertEvent([this, msgType, message]()
            {
                _MessageHandler(msgType, message);
            });
        }
    );
}

void WebsocketClient::Start()
{   
    std::lock_guard<std::mutex> lock(_operationMutex);
    if (_running)
    {
        _log("[Websocket][Start] Client is already running");
        return;
    }
    
    _client->setUrl(_url);
    _client->setTLSOptions(_getTLSOptions());
    
    WebsocketBase::Start();
    _client->start();
    
    std::stringstream ss;
    ss << "[WebSocket][Start] Client started connection to "<< _url;
    _log(ss.str());
}

WebsocketClient::~WebsocketClient()
{
    Stop();
    delete _client;
}

void WebsocketClient::Stop()
{
    std::lock_guard<std::mutex> lock(_operationMutex);
    if (!_running)
        return;
    
    _client->stop(1000, "Normal closure");
    WebsocketBase::Stop();
    
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

void WebsocketClient::_MessageHandler(ix::WebSocketMessageType msgType, std::string message)
{
    if (!_running)
        return;
        
    if (msgType == ix::WebSocketMessageType::Open)
    {
        std::stringstream ss;
        ss << "[WebSocket][Start] Client connected to " << _url;
        _log(ss.str());

        if (!onOpenHandler.IsNull())
            onOpenHandler(_url);
    }
    
    else if (msgType == ix::WebSocketMessageType::Close)
    {
        std::stringstream ss;
        ss << "[WebSocket][Start] Client disconnected from " << _url << ". Reason: " << message;
        _log(ss.str());
        
        if (!onCloseHandler.IsNull())
            onCloseHandler(_url, message);
    }
    
    if (msgType == ix::WebSocketMessageType::Message)
    {
        if (!onMessageHandler.IsNull())
            onMessageHandler(_url, message);
    }
}