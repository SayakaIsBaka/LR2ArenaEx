# LR2ArenaEx

## Quick start

- Download the latest release [here](https://github.com/SayakaIsBaka/LR2ArenaEx/releases) and extract it somewhere
- Launch LR2
- Run Launcher.exe
- Press `Ins` to toggle the overlay and `PgUp` to toggle the graph
- If hosting the server, make sure port 2222 is reachable from the Internet on your computer (alternatively, all players may use a VPN or a service such as [ZeroTier](https://www.zerotier.com/) or [Tailscale](https://tailscale.com) to remove the need to expose the port to the Internet)

> [!WARNING]
> **Host must always launch the chart first before other players do so, this is by design;** the chart won't start if this is not done, even if the host ends up selecting the same chart as everyone else afterwards

## Build

Requirements:
- Microsoft DirectX SDK **(August 2007 version)**: https://archive.org/details/dxsdk_aug2007 (might work with older versions but it has been untested)
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
- [ImGuiNotify](https://github.com/TyomaVader/ImGuiNotify)
- [mINI](https://github.com/metayeti/mINI)

## Special thanks

- All people credited in the original [LR2Arena](https://github.com/SayakaIsBaka/LR2Arena) project
- [MatVeiQaaa](https://github.com/MatVeiQaaa) for providing the base for the DirectX 9 hook
- [tenaibms](https://github.com/tenaibms) for their [LR2OOL](https://github.com/tenaibms/LR2OOL) project which gave me a few leads on how to fix some issues (especially regarding cursor / mouse input management)
- AYhaz and Shalink for helping with testing

## Known issues

- IME is not supported on the overlay
- LR2FHD is not supported
- Cleanup is not properly performed when LR2 exits (atexit is not called)