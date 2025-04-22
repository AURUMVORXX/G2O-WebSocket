## Changelog

- Event processing moved from its own thread into Squirrel thread
- Fixed lifetime control for ``WebsocketClient`` and ``WebsocketServer``
- Optimized memory usage for multiple manual relaunching ``WebsocketClient`` and ``WebsocketServer``
- New properties (both sides):
    - ``onOpen``
    - ``onClose``
    - ``onMessage``
- Deleted events:
    - ``onWebsocketConnect``
    - ``onWebsocketClose``
    - ``onWebsocketMessage``