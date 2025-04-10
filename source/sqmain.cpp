#include <sqapi.h>
#include <ixwebsocket/IXWebSocketServer.h>
#include <vector>
#include <iostream>

#include "server/websocket_server.h"
#include "client/websocket_client.h"

extern "C" SQRESULT SQRAT_API sqmodule_load(HSQUIRRELVM vm, HSQAPI api)
{
	SqModule::Initialize(vm, api);
	
	Sqrat::Function addEvent(Sqrat::RootTable(), "addEvent");
	addEvent("onWebsocketMessage");
    addEvent("onWebsocketConnect");
    addEvent("onWebsocketDisconnect");
	
	Sqrat::Object serverSide = Sqrat::ConstTable(vm).GetSlot("SERVER_SIDE");
	
	// WebSocket Server Squirrel binding (server-side only)
	
	if (serverSide.Cast<bool>())
	{
		Sqrat::Class<WebsocketServer, Sqrat::NoCopy<WebsocketServer>> websocketServer(vm, "WebsocketServer");
		
		websocketServer
		.Var("port", &WebsocketServer::port)
		.Var("silent", &WebsocketServer::silent)
		.Prop("running", &WebsocketServer::GetRunning)
		.Prop("whitelist", &WebsocketServer::GetWhitelist)
		
		.Var("useTls", &WebsocketServer::useTls)
		.Var("disableHostnameValidation", &WebsocketServer::disableHostnameValidation)
		.Var("certFile", &WebsocketServer::certificateFilePath)
		.Var("keyFile", &WebsocketServer::keyFilePath)
		.Var("caFile", &WebsocketServer::caFilePath)
		
		.Func("start", &WebsocketServer::Start)
		.Func("stop", &WebsocketServer::Stop)
		.Func("send", &WebsocketServer::Send)
		.Func("sendBinary", &WebsocketServer::SendBinary)
		.Func("sendToAll", &WebsocketServer::SendToAll)
		.Func("sendBinaryToAll", &WebsocketServer::SendBinaryToAll)
		.Func("disconnect", &WebsocketServer::Disconnect)
		.Func("setWhitelist", &WebsocketServer::SetWhitelist)
		.Func("addWhitelist", &WebsocketServer::AddWhitelist)
		.Func("removeWhitelist", &WebsocketServer::RemoveWhitelist);
		
		Sqrat::RootTable().Bind("WebsocketServer", websocketServer);
	}
	
	// WebSocket Client Squirrel binding
	
	Sqrat::Class<WebsocketClient, Sqrat::NoCopy<WebsocketClient>> websocketClient(vm, "WebsocketClient");
		
	websocketClient
	.Var("silent", &WebsocketClient::silent)
	.Prop("running", &WebsocketClient::GetRunning)
	.Prop("url", &WebsocketClient::GetUrl)
	
	.Var("useTls", &WebsocketClient::useTls)
	.Var("disableHostnameValidation", &WebsocketClient::disableHostnameValidation)
	.Var("certFile", &WebsocketClient::certificateFilePath)
	.Var("keyFile", &WebsocketClient::keyFilePath)
	.Var("caFile", &WebsocketClient::caFilePath)
	
	.Func("start", &WebsocketClient::Start)
	.Func("stop", &WebsocketClient::Stop)
	.Func("send", &WebsocketClient::Send)
	.Func("sendBinary", &WebsocketClient::SendBinary)
	.Func("setUrl", &WebsocketClient::SetUrl);
	
	Sqrat::RootTable().Bind("WebsocketClient", websocketClient);
	
	return SQ_OK;
}