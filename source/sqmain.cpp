#include <sqapi.h>
#include <ixwebsocket/IXWebSocketServer.h>
#include <vector>

#ifdef _WIN32
#include <Windows.h>
#endif

#include "websocket_server.h"
#include "json_config.h"
#include "sqfunc.h"

std::unique_ptr<WebsocketServer> g_server;

#ifdef _WIN32
BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
	if (reason == DLL_PROCESS_DETACH)
		if (g_server)
            g_server->Shutdown();

    return TRUE;
}
#else
__attribute__((destructor)) 
void LibraryUnload() {
    if (g_server)
        g_server->Shutdown();
}
#endif

extern "C" SQRESULT SQRAT_API sqmodule_load(HSQUIRRELVM vm, HSQAPI api)
{
	SqModule::Initialize(vm, api);
	g_server = std::make_unique<WebsocketServer>(JSONConfig::Get().GetPort());
	
	Sqrat::RootTable().SquirrelFunc("websocket_send", sqwebsocket_send, -2, ".ss");
	Sqrat::RootTable().SquirrelFunc("websocket_sendtoall", sqwebsocket_sendtoall, -1, ".s");
	Sqrat::RootTable().SquirrelFunc("websocket_close", sqwebsocket_close, -2, ".ss");
	
	return SQ_OK;
}