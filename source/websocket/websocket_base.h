#pragma once

class WebsocketBase
{
private:
    
    std::mutex _eventMutex;
    std::thread _eventThread;
    std::atomic<bool> _eventThreadRunning{false};
    std::queue<std::function<void()>> _eventQueue;
    
protected:
    
    std::mutex _operationMutex;
    
    bool _running{false};
    std::string _url{""};
    
    void _insertEvent(std::function<void()>);
    void _startEventThread();
    void _stopEventThread();
    void _processEvents();
    
    ix::SocketTLSOptions _getTLSOptions();
    void _log(std::string);
    
public:
    
    bool silent{false};
    
    bool disableHostnameValidation{false};
    std::string certificateFilePath{""};
    std::string keyFilePath{""};
    std::string caFilePath{"NONE"};
    
    void Start();
    void Stop();
    
    void SetUrl(std::string);
    std::string GetUrl() { return _url; }
    
    bool GetRunning() { return _running; }
};