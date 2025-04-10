#include <ixwebsocket/IXWebSocket.h>
#include <sqapi.h>
#include <vector>
#include <iostream>
#include <codecvt>
#include <locale>

#include <utf8_check.h>
#include "websocket_client.h"

void WebsocketClient::Start()
{
    if (running)
    {
        std::cout << "[WebSocket][Start] Client is already running" << std::endl;
        return;
    }
    
    _client.setUrl(_url);
    _client.setPingInterval(pingInterval);
    
    _client.setOnMessageCallback(
        [this](const ix::WebSocketMessagePtr& msg)
        {
            this->MessageHandler(msg);
        }
    );
    
    ix::SocketTLSOptions tlsOptions;
    tlsOptions.tls = useTls;
    tlsOptions.disable_hostname_validation = disableHostnameValidation;
    tlsOptions.certFile = certificateFilePath;
    tlsOptions.keyFile = keyFilePath;
    tlsOptions.caFile = caFilePath;
    _client.setTLSOptions(tlsOptions);
    
    _client.start();
    running = true;
    
    if (!silent)
    {
        if (useTls)
            std::cout << "[WebSocket][Start] Client started connection to " << _url << " (TLS)" << std::endl;
        else
            std::cout << "[WebSocket][Start] Client started connection to " << _url << std::endl;
    }
}

WebsocketClient::~WebsocketClient()
{
    Stop();
}

void WebsocketClient::Stop()
{
    if (running)
    {
        _client.stop();
        running = false;
        
        if (!silent)
            std::cout << "[Websocket][Stop] Client at " << _url << " has been stopped" << std::endl;
    }
}

void WebsocketClient::SetUrl(std::string url)
{
    if (running)
    {
        std::cout << "[WebSocket][Start] Client is already running" << std::endl;
        return;
    }
    
    _url = url;
}

void WebsocketClient::Send(std::string message)
{
    if (!running)
    {
        std::cout << "[WebSocket][Send] Client is stopped" << std::endl;
        return;
    }
    
    if (!is_utf8(message))
    {
        std::cout << "[WebSocket][Send] Message is not valid UTF-8 string" << std::endl;
        return;
    }
    _client.send(message);
}

void WebsocketClient::SendBinary(std::string message)
{
    _client.send(message);
}

void WebsocketClient::MessageHandler(const ix::WebSocketMessagePtr& msg)
{
    Sqrat::Function callEvent(Sqrat::RootTable(), "callEvent");
    
    if (msg->type == ix::WebSocketMessageType::Open)
    {
        if (!silent)
            std::cout << "[WebSocket][Start] Client connected to " << _url << std::endl;
            
        callEvent("onWebsocketConnect", this, _url);
    }
    
    if (running)
    {
        if (msg->type == ix::WebSocketMessageType::Message)
            callEvent("onWebsocketMessage", this, _url, msg->str);
        else if (msg->type == ix::WebSocketMessageType::Close)
        {
            callEvent("onWebsocketDisconnect", this, _url);
            if (!silent)
                std::cout << "[WebSocket][Start] Client disconnected from " << _url << ". Reason: " << msg->closeInfo.reason << std::endl;
        }
    }
}