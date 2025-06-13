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
