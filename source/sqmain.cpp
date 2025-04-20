#include <sqapi.h>
#include <ixwebsocket/IXWebSocketServer.h>
#include <vector>
#include <queue>

#include "websocket/websocket_base.h"
#include "websocket/websocket_server.h"
#include "websocket/websocket_client.h"

extern "C" SQRESULT SQRAT_API sqmodule_load(HSQUIRRELVM vm, HSQAPI api)
{
	SqModule::Initialize(vm, api);
	
	Sqrat::Function addEvent(Sqrat::RootTable(), "addEvent");
	addEvent("onWebsocketMessage");
    addEvent("onWebsocketConnect");
    addEvent("onWebsocketClose");
	
	Sqrat::Object serverSide = Sqrat::ConstTable(vm).GetSlot("SERVER_SIDE");
	
	// WebSocket Server Squirrel binding (server-side only)
	
	if (serverSide.Cast<bool>())
	{
		Sqrat::Class<WebsocketServer, Sqrat::NoCopy<WebsocketServer>> websocketServer(vm, "WebsocketServer");
		
		websocketServer
		.Var("port", &WebsocketServer::port)
		.Var("silent", static_cast<bool WebsocketBase::*>(&WebsocketServer::silent))
		.Prop("running", static_cast<bool (WebsocketBase::*)()>(&WebsocketServer::GetRunning))
		.Prop("whitelist", &WebsocketServer::GetWhitelist)
		
		.Var("disableHostnameValidation", static_cast<bool WebsocketBase::*>(&WebsocketServer::disableHostnameValidation))
		.Var("certFile", static_cast<std::string WebsocketBase::*>(&WebsocketServer::certificateFilePath))
		.Var("keyFile", static_cast<std::string WebsocketBase::*>(&WebsocketServer::keyFilePath))
		.Var("caFile", static_cast<std::string WebsocketBase::*>(&WebsocketServer::caFilePath))
		
		.Func("start", &WebsocketServer::Start)
		.Func("stop", &WebsocketServer::Stop)
		.Func("send", &WebsocketServer::Send)
		.Func("close", &WebsocketServer::Close)
		.Func("subscribe", &WebsocketServer::Subscribe)
		.Overload("unsubscribe", static_cast<void (WebsocketServer::*)(std::string, std::string)>(&WebsocketServer::Unsubscribe))
		.Overload("unsubscribe", static_cast<void (WebsocketServer::*)(std::string)>(&WebsocketServer::Unsubscribe))
		.Func("getTopics", &WebsocketServer::GetTopics)
		.Func("setWhitelist", &WebsocketServer::SetWhitelist)
		.Func("addWhitelist", &WebsocketServer::AddWhitelist)
		.Func("removeWhitelist", &WebsocketServer::RemoveWhitelist);
		
		Sqrat::RootTable().Bind("WebsocketServer", websocketServer);
	}
	
	// WebSocket Client Squirrel binding
	
	Sqrat::Class<WebsocketClient, Sqrat::NoCopy<WebsocketClient>> websocketClient(vm, "WebsocketClient");
		
	websocketClient
	.Var("silent", static_cast<bool WebsocketBase::*>(&WebsocketClient::silent))
	.Prop("running", static_cast<bool (WebsocketBase::*)()>(&WebsocketClient::GetRunning))
	.Prop("url", static_cast<std::string (WebsocketBase::*)()>(&WebsocketClient::GetUrl))
	
	.Var("disableHostnameValidation", static_cast<bool WebsocketBase::*>(&WebsocketClient::disableHostnameValidation))
	.Var("certFile", static_cast<std::string WebsocketBase::*>(&WebsocketClient::certificateFilePath))
	.Var("keyFile", static_cast<std::string WebsocketBase::*>(&WebsocketClient::keyFilePath))
	.Var("caFile", static_cast<std::string WebsocketBase::*>(&WebsocketClient::caFilePath))
	
	.Func("start", &WebsocketClient::Start)
	.Func("stop", &WebsocketClient::Stop)
	.Func("send", &WebsocketClient::Send)
	.Func("setUrl", static_cast<void (WebsocketBase::*)(std::string)>(&WebsocketClient::SetUrl));
	
	Sqrat::RootTable().Bind("WebsocketClient", websocketClient);
	
	return SQ_OK;
}