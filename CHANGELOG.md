## Changelog

- Added static linking for OpenSSL (Patrix)
- Removed ``g2ows.json`` config file
- Removed ``websocket_send``, ``websocket_sendBinary``, ``websocket_sendToAll``, ``websocket_close`` functions
- Added ``WebsocketServer`` and ``WebsocketClient`` classes
- Added support for client side
- Added support for launching multiple servers / clients
- Added validation for UTF-8 messages
- Changed arguments of all events: now they also pass ``WebsocketServer`` or ``WebsocketClient`` object