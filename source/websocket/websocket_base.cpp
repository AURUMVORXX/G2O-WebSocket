#include <ixwebsocket/IXWebSocketServer.h>
#include <queue>
#include <iostream>
#include <sqapi.h>

#include "websocket_base.h"

std::vector<WebsocketBase*> WebsocketBase::_objectList;

void WebsocketBase::Init()
{
    HSQUIRRELVM vm = Sqrat::DefaultVM().Get();
    Sqrat::Object serverSide = Sqrat::ConstTable(vm).GetSlot("SERVER_SIDE");
    if (serverSide.Cast<bool>())
        _addEventHandler("onTick", &WebsocketBase::_sqCallback);
    else
        _addEventHandler("onRender", &WebsocketBase::_sqCallback);
}

WebsocketBase::WebsocketBase()
{
    WebsocketBase::_objectList.push_back(this);
}

WebsocketBase::~WebsocketBase()
{
    auto it = std::find(WebsocketBase::_objectList.begin(), WebsocketBase::_objectList.end(), this);
    if (it != WebsocketBase::_objectList.end())
        WebsocketBase::_objectList.erase(it);
}

void WebsocketBase::_insertEvent(std::function<void()> function)
{
    std::lock_guard<std::mutex> lock(_eventMutex);
    _eventQueue.push(function);
}

void WebsocketBase::_processEvents() {
    
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

void WebsocketBase::_addEventHandler(const char* eventName, SQFUNCTION closure, unsigned int priority)
{
    using namespace SqModule;

    Sqrat::Function sq_addEventHandler = Sqrat::RootTable().GetFunction("addEventHandler");

    if (sq_addEventHandler.IsNull())
        return;

    HSQOBJECT closureHandle;

    sq_newclosure(vm, closure, 0);
    sq_getstackobj(vm, -1, &closureHandle); 

    Sqrat::Function func(vm, Sqrat::RootTable().GetObject(), closureHandle);
    sq_addEventHandler(eventName, func, priority);

    sq_pop(vm, 1);
}

SQInteger WebsocketBase::_sqCallback(HSQUIRRELVM vm)
{
    for(auto& object : WebsocketBase::_objectList)
        object->_processEvents();
        
    return 0;
}

void WebsocketBase::Start()
{
    _running = true;
}

void WebsocketBase::Stop()
{
    _running = false;
}