#include <ixwebsocket/IXWebSocketServer.h>
#include <sqapi.h>
#include <vector>
#include <queue>

#include <websocket_util.h>
#include "websocket_base.h"
#include "websocket_server.h"

bool WebsocketServer::_IsHostWhitelisted(std::string host)
{
    return std::find(_whitelist.begin(), _whitelist.end(), host) != _whitelist.end();
}

void WebsocketServer::Start()
{
    std::lock_guard<std::mutex> lock(_operationMutex);
    
    if (_running)
    {
        _log("[Websocket][Start] Server is already running");
        return;
    }
    
    ix::initNetSystem();
    _server = new ix::WebSocketServer(port, "0.0.0.0");
    _server->setTLSOptions(_getTLSOptions());
    
    _server->setOnClientMessageCallback(
        [this](std::shared_ptr<ix::ConnectionState> state, ix::WebSocket& ws, const ix::WebSocketMessagePtr& msg)
        {
            _MessageHandler(state, ws, msg);
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
    
    _running = true;
    WebsocketBase::Start();
}

WebsocketServer::~WebsocketServer()
{
    Stop();
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
    ix::uninitNetSystem();
    
    std::stringstream ss;
    ss << "[Websocket][Start] Listening on " << port << " has been stopped";
    _log(ss.str());
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

void WebsocketServer::_MessageHandler(std::shared_ptr<ix::ConnectionState> state, ix::WebSocket& ws, const ix::WebSocketMessagePtr& msg)
{   
    if (!_running)
        return;
        
    if (msg->type == ix::WebSocketMessageType::Open)
    {
        if (!_IsHostWhitelisted(state->getRemoteIp()))
        {
            std::stringstream ss;
            ss << "[Websocket][Handler] Host " << state->getRemoteIp() << " tried to connect, but it's not whitelisted";
            _log(ss.str());
            ws.close(ix::WebSocketCloseConstants::kNormalClosureCode, "Host is not in the whitelist.");
            return;
        }
        
        std::stringstream ss_url;
        ss_url << "ws://" << state->getRemoteIp() << ":" << state->getRemotePort();
        
        std::string url = ss_url.str();
        ws.setUrl(url);
        
        WSClient newClient;
        newClient.ipv4 = state->getRemoteIp();
        newClient.port = state->getRemotePort();
        newClient.url = url;
        newClient.socket = &ws;
        _connectedClients.push_back(newClient);
        
        Subscribe(url, state->getRemoteIp());
        Subscribe(url, url);
        Subscribe(url, "ALL");
        
        std::stringstream ss;
        ss << "[Websocket][Handler] " << state->getRemoteIp() << " connected to the websocket server";
        _log(ss.str());

        _insertEvent([this, url](){
            Sqrat::Function callEvent(Sqrat::RootTable(), "callEvent");
            callEvent("onWebsocketConnect", this, url);
        });
    }
    
    else if (msg->type == ix::WebSocketMessageType::Close)
    {
        std::string url = ws.getUrl();
        std::string message = msg->closeInfo.reason;
        
        Unsubscribe(url);
        auto client = _findClient(&ws);
        if (client != _connectedClients.end())
            _connectedClients.erase(client);
        
        std::stringstream ss;
        ss << "[Websocket][Handler] " << url << " disconnected from the websocket server. Reason: " << message;
        _log(ss.str());
        
        _insertEvent([this, url, message](){
            Sqrat::Function callEvent(Sqrat::RootTable(), "callEvent");
            callEvent("onWebsocketClose", this, url, message);
        });
    }
    
    else if (msg->type == ix::WebSocketMessageType::Message)
    {
        std::string url = ws.getUrl();
        std::string message = msg->str;
        _insertEvent([this, url, message](){
            Sqrat::Function callEvent(Sqrat::RootTable(), "callEvent");
            callEvent("onWebsocketMessage", this, url, message);
        });
    }
}