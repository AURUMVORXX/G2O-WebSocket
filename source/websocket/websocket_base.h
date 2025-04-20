#pragma once

class WebsocketBase
{
private:
    
    std::mutex _eventMutex;
    std::thread _eventThread;
    std::atomic<bool> _eventThreadRunning{false};
    std::queue<std::function<void()>> _eventQueue;
    std::condition_variable _eventCondition;
    
protected:
    
    std::mutex _operationMutex;
    
    bool _running{false};
    
    void _insertEvent(std::function<void()>);
    void _startEventThread();
    void _stopEventThread();
    void _processEvents();
    
    void _log(std::string);
    
public:
    
    void Start();
    void Stop();
};