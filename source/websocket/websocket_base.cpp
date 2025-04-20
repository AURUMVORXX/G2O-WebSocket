#include <ixwebsocket/IXWebSocketServer.h>
#include <queue>
#include <iostream>

#include "websocket_base.h"

void WebsocketBase::_insertEvent(std::function<void()> function)
{
    {
        std::lock_guard<std::mutex> lock(_eventMutex);
        _eventQueue.push(function);
    }
    _eventCondition.notify_one();
}

void WebsocketBase::_startEventThread() {
    _eventThreadRunning = true;
    _eventThread = std::thread([this]() {
        while (_eventThreadRunning) {
            _processEvents();
            std::unique_lock<std::mutex> lock(_eventMutex);
            _eventCondition.wait(lock, [this]() {
                return !_eventQueue.empty() || !_eventThreadRunning;
            });
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