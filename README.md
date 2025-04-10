# G2O Websocket

## Introduction

This module implements websocket server as the G2O server-side module. This way you can connect and execute real-time commands from any external application.

## Installation

1. Download module and place it to your server folder (root folder, i.e.)
3. Connect ``G2OWS.dll`` as server-side module or client-side module:
```xml
<module src="G2OWS.dll" type="server" />
```

## Documentation

**New Events:**
```cpp
onWebsocketConnect(object: socket, string: url) // Triggers on new connection
onWebsocketMessage(object: socket, string: url, string: message) // Triggers on new message
onWebsocketClose(object: socket, string: url)   // Triggers on disconnects
```

**WebsocketServer**
```cpp
class WebsocketServer
{
    int port;                               // Port which server will be running on
    bool silent;                            // Disable information in the console (new connection, disconnect, etc.)
    (read-only) bool running;               // Current state of the server
    (read-only) array whitelist             // Array of whitelisted hosts
    
    bool useTls;
    bool disableHostnameValidation;
    string certFile;                        // Path to certificate file
    string keyFile;                         // Path to key file
    string caFile;                          // Path to CA file
    
    void start();                           // Starts server with given settings
    void stop();                            // Stops server
    void send(string host, string message); // Send message to given host
    void sendBinary(string host, string message); // Send binary message to given host
    void sendToAll(string message);         // Send message to all connected clients
    void sendBinaryToAll(string message);
    void disconnect(string host, string reason); // Disconnect client with given host
    void setWhitelist(array hosts)              // Overwrite whitelist with given array
    void addWhitelist(string host)              // Add host to existing whitelist
}
```
**WebsocketClient**
```cpp
class WebsocketClient
{
    (read-only) string url;                 // Client url which it will be connect
    bool silent;                            // Disable information in the console (new connection, disconnect, etc.)
    (read-only) bool running;               // Current state of the client
    
    bool useTls;
    bool disableHostnameValidation;
    string certFile;                        // Path to certificate file
    string keyFile;                         // Path to key file
    string caFile;                          // Path to CA file
    
    void start();                           // Starts client with given settings
    void stop();                            // Stops client
    void send(string message); // Send message to given host
    void sendBinary(string message); // Send binary message to given host
}
```

**URL** is a string with following format: ``ws://ip:port``. It is a way to identify clients from each other.

## Usage example

**Squirrel (G2O server side)**
```cpp
local server = -1;
local client = -1;

addEventHandler("onInit", function()
{
    server = WebsocketServer();
    server.port = 8080;
    server.setWhitelist(["127.0.0.1", "localhost"]);
    server.start();
    
    print("Current whitelist:");
    foreach (val in server.whitelist)
      print(val);

    client = WebsocketClient();
    client.setUrl("ws://localhost:8080");
    client.start();
});

addEventHandler("onWebsocketConnect", function(socket, url)
{
    if (socket == server)
    {
      server.send(url, "Greetings");
    }
});

addEventHandler("onWebsocketMessage", function(socket, url, message)
{
   if (socket == client)
   {
      print("Got message from server: " + message);
      client.send("Echo: Greetings");
   }

   if (socket == server)
   {
      print("Got message from " + url + ": " + message);
   }
});

addEventHandler("onWebsocketDisconnect", function(socket, url)
{
   if (socket == server)
   {
      print("Client " + url + " has been disconnected");
   }
});

// Just simple example how to dynamically update whitelist to connect client-side and server-side
addEventHandler("onPlayerConnect", function(pid)
{
    server.addWhitelist(getPlayerIp(pid));
});
addEventHandler("onPlayerDisconnect", function(pid, reason)
{
    server.removeWhitelist(getPlayerIp(pid));
});

```