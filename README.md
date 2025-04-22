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

**WebsocketServer**
```cpp
class WebsocketServer
{
    int port;                                           // Port which server will be running on
    bool silent;                                        // Disable information in the console (new connection, disconnect, etc.)
    (read-only) bool running;                           // Current state of the server
    (read-only) array whitelist                         // Array of whitelisted hosts
    
    function(string: url) onOpen                        // Reference to a function, that will be called on new connection to the server
    function(string: url, string: message) onClose      // Reference to a function, that will be called when a client disconnects
    function(string: url, string: message) onMessage    // Reference to a function, that will be called on new message from a client
    
    bool disableHostnameValidation;
    string certFile;                                    // Path to certificate file
    string keyFile;                                     // Path to key file
    string caFile;                                      // Path to CA file
    
    void start();                                       // Starts server with given settings
    void stop();                                        // Stops server
    void send(string topic, string message);            // Send message to given host
    void close(string url, string reason);              // Disconnect client with given host
    void setWhitelist(array IPv4)                       // Overwrite whitelist with given array
    void addWhitelist(string IPv4)                      // Add host to existing whitelist
    void removeWhitelist(string IPv4)                   // Remove host from whitelist
    void subscribe(string url, string topic)            // Subscribe client to a topic
    void unsubscribe(string url, string topic)          // Unsubscribe cleant from a certain topic
    void unsubscribe(string url)                        // Unsubscribe client from all topics
    array getTopics(string url)                         // Get list of all subscribed by client topics
}
```
**WebsocketClient**
```cpp
class WebsocketClient
{
    (read-only) string url;                             // Client url which it will be connect
    bool silent;                                        // Disable information in the console (new connection, disconnect, etc.)
    (read-only) bool running;                           // Current state of the client
    
    function(string: url) onOpen                        // Reference to a function, that will be called on connection to the server
    function(string: url, string: message) onClose      // Reference to a function, that will be called when connection will be closed
    function(string: url, string: message) onMessage    // Reference to a function, that will be called on new message from the server
    
    bool disableHostnameValidation;
    string certFile;                                    // Path to certificate file
    string keyFile;                                     // Path to key file
    string caFile;                                      // Path to CA file
    
    void start();                                       // Starts client with given settings
    void stop();                                        // Stops client
    void send(string message);                          // Send message to given host
}
```

**URL** is a string with following format: ``ws://ip:port``. It is a way to identify clients from each other.

## Usage example

**Squirrel (G2O server side)**
```cpp
local server = -1;
local client = -1;

function onServerConnect(url)
{
    server.send(url, "Greetings");
    server.subscribe(url, "TIME");
}

function onServerMessage(url, message)
{
    print("Got message from " + url + ": " + message);
}

function onServerDisconnect(url, message)
{
    print("Client " + url + " has been disconnected");
}

function onClientMessage(url, message)
{
    print("Got message from server: " + message);
    client.send("Echo: Greetings");
}

addEventHandler("onInit", function()
{
    server = WebsocketServer();
    server.port = 8080;
    server.setWhitelist(["127.0.0.1", "localhost"]);
    server.onOpen = onServerConnect;
    server.onMessage = onServerMessage;
    server.onClose = onServerDisconnect;
    server.start();
    
    print("Current whitelist:");
    foreach (val in server.whitelist)
      print(val);

    client = WebsocketClient();
    client.setUrl("ws://localhost:8080");
    client.onMessage = onClientMessage;
    client.start();
});

addEventHandler("onTime", function(day, hour, min)
{
    if (server != -1 && server.running)
        server.send("TIME", "Current time: " + hour + ":" + min);
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