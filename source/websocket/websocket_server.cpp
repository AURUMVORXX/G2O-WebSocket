#include <ixwebsocket/IXWebSocketServer.h>
#include <sqapi.h>
#include <vector>
#include <queue>
#include <iostream>

#include <websocket_util.h>
#include "websocket_base.h"
#include "websocket_server.h"

WebsocketServer::WebsocketServer()
{
}

WebsocketServer::~WebsocketServer()
{
    Stop();
}

std::vector<WSClient>::iterator WebsocketServer::_findClient(ix::WebSocket* ws)
{
    for (auto it = _connectedClients.begin(); it != _connectedClients.end(); it++)
    {
        if (it->socket == ws)
        {
            return it;
        }
    }
    
    return _connectedClients.end();
}

std::vector<WSClient>::iterator WebsocketServer::_findClient(std::string url)
{
    for (auto it = _connectedClients.begin(); it != _connectedClients.end(); it++)
    {
        if (it->url == url)
        {
            return it;
        }
    }
    
    return _connectedClients.end();
}

bool WebsocketServer::_validateWhitelisted(ix::WebSocket* ws, std::string ipv4)
{
    bool isHostValid = std::find(_whitelist.begin(), _whitelist.end(), ipv4) != _whitelist.end();
    if (isHostValid)
        return true;
        
    std::stringstream ss;
    ss << "[Websocket][Handler] Host " << ipv4 << " tried to connect, but it's not whitelisted";
    _log(ss.str());
    
    ws->close(ix::WebSocketCloseConstants::kNormalClosureCode, "Host is not in the whitelist.");
    return false;
}

ix::SocketTLSOptions WebsocketServer::_getTLSOptions()
{
    ix::SocketTLSOptions tlsOptions;
    tlsOptions.tls  = !certificateFilePath.empty();
    tlsOptions.disable_hostname_validation = disableHostnameValidation;
    tlsOptions.certFile = certificateFilePath;
    tlsOptions.keyFile = keyFilePath;
    tlsOptions.caFile = caFilePath;
    
    return tlsOptions;
}

void WebsocketServer::_log(std::string message)
{
    if (silent)
        return;
        
    std::cout << message << std::endl;
}

void WebsocketServer::_initializeClient(ix::WebSocket* ws, std::string ipv4, int port)
{
    std::stringstream ss_url;
    ss_url << "ws://" << ipv4 << ":" << port;
    std::string url = ss_url.str();
    
    WSClient newClient;
    newClient.ipv4 = ipv4;
    newClient.port = port;
    newClient.url = url;
    newClient.socket = ws;
    _connectedClients.push_back(newClient);
    ws->setUrl(url);
    
    Subscribe(url, ipv4);
    Subscribe(url, url);
    Subscribe(url, "ALL");
}

void WebsocketServer::_deinitializeClient(ix::WebSocket* ws)
{
    Unsubscribe(ws->getUrl());
    auto client = _findClient(ws);
    if (client != _connectedClients.end())
        _connectedClients.erase(client);
}

void WebsocketServer::_MessageHandler(std::shared_ptr<ix::ConnectionState> state, ix::WebSocket* ws, ix::WebSocketMessageType msgType, std::string message)
{   
    if (!_running)
        return;
        
    if (msgType == ix::WebSocketMessageType::Open)
    {
        if (!_validateWhitelisted(ws, state->getRemoteIp()))
        {
            if (silent)
                return;
                
            std::stringstream ss;
            ss << "[Websocket][Handler] " << state->getRemoteIp() << " tried to connect, but it's not in whitelist";
            _log(ss.str());
            return;
        }

        _initializeClient(ws, state->getRemoteIp(), state->getRemotePort());
            
        std::stringstream ss;
        ss << "[Websocket][Handler] " << state->getRemoteIp() << " connected to the websocket server";
        _log(ss.str());
        
        if (!onOpenHandler.IsNull())
            onOpenHandler(ws->getUrl());
    }
    
    else if (msgType == ix::WebSocketMessageType::Close)
    {
        std::string url = ws->getUrl();
        _deinitializeClient(ws);
        
        std::stringstream ss;
        ss << "[Websocket][Handler] " << url << " disconnected from the websocket server. Reason: " << message;
        _log(ss.str());
        
        if (!onCloseHandler.IsNull())
            onCloseHandler(ws->getUrl(), message);
    }
    
    else if (msgType == ix::WebSocketMessageType::Message)
    {
        std::string url = ws->getUrl();
        if (!onMessageHandler.IsNull())
            onMessageHandler(ws->getUrl(), message);
    }
}

// ###################################################################

void WebsocketServer::Start()
{
    std::lock_guard<std::mutex> lock(_operationMutex);
    
    if (_running)
    {
        _log("[Websocket][Start] Server is already running");
        return;
    }
    
    _server = new ix::WebSocketServer(port, "0.0.0.0");
    _server->setTLSOptions(_getTLSOptions());
    
    _server->setOnClientMessageCallback(
        [this](std::shared_ptr<ix::ConnectionState> state, ix::WebSocket& ws, const ix::WebSocketMessagePtr& msg)
        {
            ix::WebSocketMessageType msgType = msg->type;
            ix::WebSocket* websocket = &ws;
            std::string message;
            if (msgType == ix::WebSocketMessageType::Close)
                message = msg->closeInfo.reason;
            else
                message = msg->str;
            
            _insertEvent([this, state, websocket, msgType, message]()
            {
                _MessageHandler(state, websocket, msgType, message);
            });
        }
    );
    
    _serverThread = std::thread(
        [this]
        {
            if (!_server->listen().first)
                return;
                
            _server->start();
            std::stringstream ss;
            ss << "[Websocket][Start] Listening on " << port;
            _log(ss.str());
        }
    );
    
    WebsocketBase::Start();
}

void WebsocketServer::Stop()
{
    std::lock_guard<std::mutex> lock(_operationMutex);
    if (!_running)
        return;

    _server->stop();
    WebsocketBase::Stop();
    if (_serverThread.joinable())
        _serverThread.join();

    delete _server;
    _server = nullptr;
    
    std::stringstream ss;
    ss << "[Websocket][Start] Listening on " << port << " has been stopped";
    _log(ss.str());
}

void WebsocketServer::Close(std::string url, std::string reason)
{
    if (!_running)
    {
        _log("[WebSocket][Send] Server is stopped");
        return;
    }
    
    auto client = _findClient(url);
    if (client == _connectedClients.end())
        return;
        
    client->socket->close(ix::WebSocketCloseConstants::kNormalClosureCode, reason);
}

void WebsocketServer::Send(std::string topic, std::string message)
{
    if (!_running)
    {
        _log("[WebSocket][Send] Server is stopped");
        return;
    }
    
    if (!is_utf8(message))
    {
        _log("[WebSocket][Send] Message is not a valid UTF-8 string");
        return;
    }
    
    if (_topics.find(topic) == _topics.end())
        return;
        
    for(auto client : _topics[topic])
        client.socket->send(message);
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
    bool isHostValid = std::find(_whitelist.begin(), _whitelist.end(), host) != _whitelist.end();
    if(!isHostValid)
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

void WebsocketServer::Subscribe(std::string url, std::string topic)
{
    auto client = _findClient(url);
    if (client == _connectedClients.end())
        return;
        
    if (_topics.find(topic) == _topics.end())
        _topics[topic] = std::vector<WSClient>();
    
    client->topics.push_back(topic);    
    _topics[topic].push_back(*client);
}

void WebsocketServer::Unsubscribe(std::string url, std::string topic)
{
    auto client = _findClient(url);
    if (client == _connectedClients.end())
        return;
        
    if (_topics.find(topic) == _topics.end() || 
        std::find(client->topics.begin(), client->topics.end(), topic) == client->topics.end())
        return;
    
    client->topics.erase(std::remove(client->topics.begin(), client->topics.end(), topic), client->topics.end()); 
    _topics[topic].erase(std::remove(_topics[topic].begin(), _topics[topic].end(), *client), _topics[topic].end());
    
    if (_topics[topic].size() == 0)
        _topics.erase(topic);
}

void WebsocketServer::Unsubscribe(std::string url)
{
    auto client = _findClient(url);
    if (client == _connectedClients.end())
        return;
    
    for (std::string topic : client->topics)
    {
        _topics[topic].erase(std::remove(_topics[topic].begin(), _topics[topic].end(), *client), _topics[topic].end());
        if (_topics[topic].size() == 0)
            _topics.erase(topic);
    }

    client->topics.clear();
}

Sqrat::Array WebsocketServer::GetTopics(std::string url)
{
    Sqrat::Array sqTopicsArray(SqModule::vm);
    
    auto client = _findClient(url);
    if (client == _connectedClients.end())
        return sqTopicsArray;
        
    for (auto& topic : client->topics)
        sqTopicsArray.Append(topic);
        
    return sqTopicsArray;
}