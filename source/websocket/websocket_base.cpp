#include <ixwebsocket/IXWebSocketServer.h>
#include <queue>
#include <iostream>
#include <format>

#include "websocket_base.h"

void WebsocketBase::_insertEvent(std::function<void()> function)
{
    std::lock_guard<std::mutex> lock(_eventMutex);
    _eventQueue.push(function);
}

void WebsocketBase::_startEventThread() {
    _eventThreadRunning = true;
    _eventThread = std::thread([this]() {
        while (_eventThreadRunning) {
            _processEvents();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        _processEvents();
    });
}

void WebsocketBase::_stopEventThread() {
    _eventThreadRunning = false;
    if (_eventThread.joinable()) {
        _eventThread.join();
    }
}

void WebsocketBase::_processEvents() {
    try
    {
        std::queue<std::function<void()>> events;
    
        {
            std::lock_guard<std::mutex> lock(_eventMutex);
            events.swap(_eventQueue);
        }
        
        while (!events.empty()) {
            auto event = events.front();
            events.pop();
            event();
        }
    }
    catch(const std::exception& e)
    {
        std::cout << e.what() << '\n';
    }
}

void WebsocketBase::Start()
{
    _running = true;
    _startEventThread();
}

void WebsocketBase::Stop()
{
    _running = false;
    _startEventThread();
}

void WebsocketBase::SetUrl(std::string url)
{
    if (_running)
    {
        std::cout << "[WebSocket][SetUrl] Already running" << std::endl;
        return;
    }
    
    _url = url;
}

ix::SocketTLSOptions WebsocketBase::_getTLSOptions()
{
    ix::SocketTLSOptions tlsOptions;
    tlsOptions.tls  = !certificateFilePath.empty();
    tlsOptions.disable_hostname_validation = disableHostnameValidation;
    tlsOptions.certFile = certificateFilePath;
    tlsOptions.keyFile = keyFilePath;
    tlsOptions.caFile = caFilePath;
    
    return tlsOptions;
}

void WebsocketBase::_log(std::string message)
{
    if (silent)
        return;
        
    std::cout << message << std::endl;
}