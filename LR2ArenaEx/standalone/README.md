# LR2ArenaEx-server

A cross-platform standalone server for LR2ArenaEx; it's effectively just a simple wrapper around the server included in LR2ArenaEx.

## Build

This project uses CMake (3.14 or newer required).
```bash
mkdir build
cd build
cmake ..
make
```
Alternatively, use the CMake extension in VS Code which does most of the work for you.

## Libraries

The standalone server for LR2ArenaEx uses the following libraries:
- [IXWebSocket](https://github.com/machinezone/IXWebSocket)
- [msgpack](https://github.com/msgpack/msgpack-c)
- [argparse](https://github.com/p-ranav/argparse)

## Hosting the standalone server

> [!WARNING]
> If running via a reverse proxy ensure that you do not redirect via SSL or upgrade from `ws://` to `wss://`. Similarly, be aware that a lot of clients hardcode `:2222` as a port for the server, and other ports will likely fail!

### Example Traefik config
```yml
# config.yml
providers:
  file:
    directory: ./dynamic-config
    watch: true

entrypoints:
  websocket:
    address: ":2222"
```
```yml
# dynamic-config/lr2arenaex.yml
http:
  routers:
    lr2arenaex:
      rule: Host(`lr2arenaex.pfy.ch`)
      service: lr2arenaex
      entrypoints: websocket

  services:
    lr2arenaex:
      loadBalancer:
        passHostHeader: true
        servers:
          - url: "http://<service-ip>:<service-port>"
```
