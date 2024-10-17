# LR2ArenaEx

## Build

Requirements:
- Microsoft DirectX SDK **(August 2006 version)**: https://archive.org/details/dxsdk_aug2006 (might work with older versions but it has been untested)
- Visual Studio 2019 or newer

Paths for the DirectX SDK have been hardcoded in the project settings; if you chose the default settings it should work fine but you might need to edit them depending on your setup.

⚠️ **Make sure to set the platform to x86; setting it to anything else will not work**

## Libraries

LR2ArenaEx uses the following libraries:
- [Dear ImGui](https://github.com/ocornut/imgui)
- [readerwriterqueue](https://github.com/cameron314/readerwriterqueue)
- [Garnet](https://github.com/jopo86/garnet)
- [SQLite](https://www.sqlite.org)
- [sqlite_modern_cpp](https://github.com/SqliteModernCpp/sqlite_modern_cpp)
- [cppack](https://github.com/dacap/cppack)
- [implot](https://github.com/epezent/implot)

## Special thanks

- All people credited in the original [LR2Arena](https://github.com/SayakaIsBaka/LR2Arena) project
- [MatVeiQaaa](https://github.com/MatVeiQaaa) for providing the base for the DirectX 9 hook
- [tenaibms](https://github.com/tenaibms) for their [LR2OOL](https://github.com/tenaibms/LR2OOL) project which gave me a few leads on how to fix some issues (especially regarding cursor / mouse input management)

## TODO

- Better graph (different colors)
- Add database check on chart receive
- Lobby management (change host, etc)
- Do cleanup on exit (or at least do something *slightly* cleaner than right now)