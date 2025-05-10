#include <sqapi.h>
#include <ixwebsocket/IXWebSocketServer.h>
#include <vector>
#include <queue>
#include <iostream>

#include "websocket/websocket_base.h"
#include "websocket/websocket_server.h"
#include "websocket/websocket_client.h"

extern "C" SQRESULT SQRAT_API sqmodule_load(HSQUIRRELVM vm, HSQAPI api)
{
	SqModule::Initialize(vm, api);
	WebsocketBase::Init();
	
#ifdef WIN32
	ix::initNetSystem();
#endif
	
	Sqrat::Object serverSide = Sqrat::ConstTable(vm).GetSlot("SERVER_SIDE");
	
	// WebSocket Server Squirrel binding (server-side only)
	
	if (serverSide.Cast<bool>())
	{
		Sqrat::Class<WebsocketServer, Sqrat::NoCopy<WebsocketServer>> websocketServer(vm, "WebsocketServer");
		
		websocketServer
		.Var("port", &WebsocketServer::port)
		.Var("silent", &WebsocketServer::silent)
		.Func("running", &WebsocketServer::GetRunning)
		.Prop("whitelist", &WebsocketServer::GetWhitelist)
		
		.Var("disableHostnameValidation", &WebsocketServer::disableHostnameValidation)
		.Var("certFile", &WebsocketServer::certificateFilePath)
		.Var("keyFile", &WebsocketServer::keyFilePath)
		.Var("caFile", &WebsocketServer::caFilePath)
		
		.Var("onOpen", &WebsocketServer::onOpenHandler)
		.Var("onClose", &WebsocketServer::onCloseHandler)
		.Var("onMessage", &WebsocketServer::onMessageHandler)
		
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
	.Var("silent", &WebsocketClient::silent)
	.Func("running", &WebsocketClient::GetRunning)
	.Prop("url", static_cast<std::string (WebsocketBase::*)()>(&WebsocketClient::GetUrl))
	
	.Var("disableHostnameValidation", &WebsocketClient::disableHostnameValidation)
	.Var("certFile", &WebsocketClient::certificateFilePath)
	.Var("keyFile", &WebsocketClient::keyFilePath)
	.Var("caFile", &WebsocketClient::caFilePath)
	
	.Var("onOpen", &WebsocketClient::onOpenHandler)
	.Var("onClose", &WebsocketClient::onCloseHandler)
	.Var("onMessage", &WebsocketClient::onMessageHandler)
	
	.Func("start", &WebsocketClient::Start)
	.Func("stop", &WebsocketClient::Stop)
	.Func("send", &WebsocketClient::Send)
	.Func("setUrl", &WebsocketClient::SetUrl);

	Sqrat::RootTable().Bind("WebsocketClient", websocketClient);
	
	return SQ_OK;
}