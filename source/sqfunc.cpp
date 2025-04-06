#include <sqapi.h>
#include <vector>
#include <ixwebsocket/IXWebSocketServer.h>

#include "sqfunc.h"
#include "json_config.h"
#include "websocket_server.h"

extern std::unique_ptr<WebsocketServer> g_server;

SQInteger sqwebsocket_close(HSQUIRRELVM vm)
{
    const SQChar* url;
    const SQChar* reason;
    
    sq_getstring(vm, 2, &url);
    sq_getstring(vm, 3, &reason);
    
    if (auto client = g_server->GetClient(url))
        client.value()->close(ix::WebSocketCloseConstants::kNormalClosureCode, reason);
    else
        return sq_throwerror(vm, "Not a valid connected websocket url");
        
    return 0;
}

SQInteger sqwebsocket_send(HSQUIRRELVM vm)
{
    const SQChar* url;
    const SQChar* message;
    
    sq_getstring(vm, 2, &url);
    sq_getstring(vm, 3, &message);
    
    if (auto client = g_server->GetClient(url))
    {
        if (JSONConfig::Get().GetBinary())
            client.value()->sendBinary(message);
        else
            client.value()->send(message);
    }
    else
        return sq_throwerror(vm, "Not a valid connected websocket url");
        
    return 0;
}

SQInteger sqwebsocket_sendtoall(HSQUIRRELVM vm)
{
    const SQChar* message;
    
    sq_getstring(vm, 2, &message);
    
    for (auto && client : g_server->GetClients())
    {
        if (JSONConfig::Get().GetBinary())
            client->sendBinary(message);
        else
            client->send(message);
    }
        
    return 0;
}