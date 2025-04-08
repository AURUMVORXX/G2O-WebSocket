# G2O Websocket

## Introduction

This module implements websocket server as the G2O server-side module. This way you can connect and execute real-time commands from any external application.

## Installation

1. Download **ZIP** archive and unpack it to your server root folder
2. Configure ``g2ows.json`` and add your host to whitelist
3. Connect ``G2OWS.dll`` as server-side module:
```xml
<module src="G2OWS.dll" type="server" />
```

## Documentation

**New Events:**
```cpp
onWebsocketConnect(string: url) // Triggers on new connection
onWebsocketMessage(string: url, string: message) // Triggers on new message
onWebsocketClose(string: url)   // Triggers on disconnect
```

**New Functions:**
```cpp
websocket_send(string: url, std: message)   // Send a message to client with given url
websocket_sendtoall(string: message)    // Send a message to all clients
websocket_close(string: url, string: reason)    // Disconnect client with given url
```

**URL** is a string with following format: ``ws://ip:port``. It is a way to identify clients from each other.

## Config

* **port**: port which will be used for websocket server.
* **sendBinary**: if you want to convert all of your messages to binary before sending.
* **whitelist**: list of allowed to connect IPs.
* **tls**: settings for connection with TLS.
    * **enabled**: enabling TLS connection.
    * **certFile**: path to certificate file (required if enabled: true)
    * **keyFile**: path to private key file (required if enabled: true)
    * **caFile**: specifying caFile implies that: 1. You require clients to present a certificate; 2. It must be signed by one of the trusted roots in the file
    * **disableHostnameValidation**: by default, a destination's hostname is always validated against the certificate that it presents. To accept certificates with any hostname, set this to **true**.

## Usage example

**Squirrel (G2O server side)**
```cpp
addEventHandler("onWebsocketConnect", function(client)
{
    websocket_send(client, "Hello!");
});

addEventHandler("onTime", function(day, hour, min)
{
    websocket_sendtoall("Current time: " + day + " / " + hour + ":" + min);
});
```

**Any external app (Python i.e.)**
```python
import websockets
import asyncio

async def listen_websocket(uri):
    async with websockets.connect(uri) as websocket:
        while True:
            message = await websocket.recv()
            print(message)
            
if __name__ == "__main__":
    WS_URI = "ws://localhost:8080"
    asyncio.run_until_complete(listen_websocket(WS_URI))
```