## Changelog

- Fixed calling Squirrel functions inside the docker container
- Fixed property ``WebsocketServer.port`` not actually changed the port
- Fixed memory usage for ``WebsocketServer``
- Added explicit clients disconnection on ``WebsocketServer`` shutdown
- Default port of ``WebsocketServer`` changed to ``8675``