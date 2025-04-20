## Changelog

- Fixed application crash due to incorrect memory management (resource deadlock)
- Changed message delivery system for server side. Now, server is **broadcasting** to certain topics, and clients which are subscribed to it - will recieve the message.
By default, all clients is subscribed to three topics: **"\<IPv4\>"**, **"\<URL\>"** and **"ALL"**. For example, if client connected from 127.0.0.1:49724, it will be subscribed to
"127.0.0.1", "ws://127.0.0.1:49724", and "ALL"
- New methods:
    - ``WebsocketServer.subscribe(string: url, string: topic)``
    - ``WebsocketServer.unsubscribe(string: url, string: topic)``
    - ``WebsocketServer.unsubscribe(string: url)``
    - ``WebsocketServer.getTopics(string: url)``
- Removed properties:
    - ``WebsocketClient.useTls`` - TLS now will now be enabled automatically if you pass certificate file
- Removed methods:
    - ``WebsocketClient.sendBinary``
    - ``WebsocketServer.sendBinary``
    - ``WebsocketServer.sendToAll``
    - ``WebsocketServer.sendBinaryToAll``
- Renamed methods:
    - ``WebsocketServer.disconnect`` -> ``WebsocketServer.close``
- Changed methods:
    - ``WebsocketServer.send(string: host, string: message)`` -> ``WebsocketServer.send(string: topic, string: message)``
- Changed events:
    - ``onWebsocketClose(instance: socket, string: host)`` -> ``onWebsocketClose(instance: socket, string: host, string: message)``