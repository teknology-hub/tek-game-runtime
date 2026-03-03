# TEK Game Runtime
[![Discord](https://img.shields.io/discord/937821572285206659?style=flat-square&label=Discord&logo=discord&logoColor=white&color=7289DA)](https://discord.gg/JBUgcwvpfc)

tek-game-runtime is a dynamic library that can be injected into game processes early at startup to modify some of the functions to e.g. override license ownership checks.

## Features

- Does not require modifying any game files, all changes are performed in running process
- Does not violate process function flow, transparently integrates into it
- All functionality can be configured via a settings file
- Has a modular structure allowing it to support multiple game stores (currently, Steam is the only supported one), and game-specific modifications on top of store ones

[Steam features](https://github.com/teknology-hub/tek-game-runtime/blob/main/features/steam.md)

Note that currently it has no "support" for any DRM, so it'll likely not work in DRM-protected games

## Using

Get the DLL from [releases](https://github.com/teknology-hub/tek-game-runtime/releases) and use [tek-injector](https://github.com/teknology-hub/tek-injector) to inject it into a game process. A settings JSON file *must* be provided, with `store` option being mandatory. Currently the only valid value for it is `"steam"`. See store-specific features page for other required and available options. In the minimal Steam game setup with default tek-injector options, the settings file must be named `tek-gr-setting.json`, located next to game's executable along with libtek-game-runtime.dll, and have the following contents, with your game's Steam app ID instead of 0:
```json
{
  "store": "steam",
  "app_id": 0
}
```

libtek-game-runtime.dll release is built in MSYS2 [TEK-X86_64 environment](https://github.com/teknology-hub/MinGW-env) and signed by Nuclearist's code signing certificate, which in turn is signed by [TEK CA](https://teknology-hub.com/public-keys/ca.crt), so it will be trusted by OS if TEK CA certificate is.

## Project structure

- `features` - Documentation for store- and game-specific features and settings
- `res` - Windows resource files
- `src` - Source code:
  + `steam` - Steam-specific code
    - `api` - Steam API interface wrapper headers
    - `games` - Game-specific code for Steam games
- `subprojects` - Meson subproject directory. The repository includes wrap files and package files for dependencies that do not have their own MSYS2 package. Currently the only such is [ValveFileVDF](https://github.com/TinyTinni/ValveFileVDF)
