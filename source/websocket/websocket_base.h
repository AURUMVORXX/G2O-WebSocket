#pragma once

class WebsocketBase
{
private:
    
    static std::vector<WebsocketBase*> _objectList;
    
    std::mutex _eventMutex;
    std::queue<std::function<void()>> _eventQueue;
    
    static void _addEventHandler(const char*, SQFUNCTION, unsigned int = 9999);
    void _processEvents();
    
    static SQInteger _sqCallback(HSQUIRRELVM vm);
    
protected:
    
    std::mutex _operationMutex;
    bool _running{false};
    
    void _insertEvent(std::function<void()>);
    
    void _log(std::string);
    
public:
    
    WebsocketBase();
    ~WebsocketBase();
    
    void Start();
    void Stop();
    static void Init();
};