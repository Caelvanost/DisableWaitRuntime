# Disable Wait Runtime

A lightweight SKSE plugin for **Skyrim Special Edition / Anniversary Edition** that disables Skyrim's native **Wait** action at runtime and frees its key binding for use by other mods.

The plugin does **not** replace `controlmap.txt`, does not require an ESP, and does not store persistent data in your save.

## Features

- Disables the native `Wait` input action.
- Frees the default `T` key so it can be used by another mod or MCM hotkey.
- Does not replace or patch `controlmap.txt`.
- Does not affect sleeping in beds.
- No ESP or ESL.
- No Papyrus scripts.
- No persistent save-game data.
- Normal Skyrim bindings return after uninstalling the plugin and restarting the game.

## How it works

Skyrim loads its input bindings into `RE::ControlMap`.

After the game's control data has loaded, Disable Wait Runtime searches the gameplay mappings for the event named `Wait` and sets only that event's `inputKey` to Skyrim's unmapped value, `0xFF`.

The operation is reapplied after:

- `DataLoaded`
- `NewGame`
- `PostLoadGame`

This avoids distributing a replacement `controlmap.txt` and reduces compatibility problems with UI, controller, hotkey, and control-remapping mods.

## Requirements

- Skyrim Special Edition / Anniversary Edition
- SKSE64
- Address Library for SKSE Plugins

The project is built with **CommonLibSSE-NG**.

## Installation

Install the compiled release archive with Vortex.

The release package should contain:

```text
SKSE/
└── Plugins/
    └── DisableWaitRuntime.dll
```

Enable the mod and launch Skyrim through SKSE or your normal Skyrim Together setup.

## Testing

After loading a save:

1. Press `T` during normal gameplay.
2. The Wait menu should no longer open.
3. Assign `T` to another mod or MCM hotkey.
4. Confirm that the new hotkey works.
5. Activate a bed and confirm that sleeping still works.

The plugin log is written to:

```text
Documents/My Games/Skyrim Special Edition/SKSE/DisableWaitRuntime.log
```

## Building from source

### Requirements

- Visual Studio with the C++ desktop workload
- CMake
- vcpkg
- CommonLibSSE-NG

Set the `VCPKG_ROOT` environment variable to your vcpkg installation, then run:

```bat
build_release.bat
```

The build script configures CMake, compiles the plugin, copies the DLL into the distribution folder, and creates:

```text
Disable-Wait-Runtime-v0.2.0.zip
```

## Project structure

```text
DisableWaitRuntime/
├── .gitignore
├── LICENSE
├── README.md
├── CMakeLists.txt
├── vcpkg.json
├── build_release.bat
└── src/
    └── plugin.cpp
```

## Compatibility

The plugin is intended for Skyrim SE/AE runtimes supported by CommonLibSSE-NG.

Because it modifies the in-memory control map instead of distributing a complete `controlmap.txt`, it is less invasive than traditional control-map replacement mods.

Mods that intentionally modify the `Wait` mapping at runtime may conflict depending on when each mod applies its changes.

## Uninstallation

Disable or remove the mod and restart Skyrim.

The plugin modifies only the in-memory control map, so Skyrim reloads its normal bindings on the next launch. No save cleaning is required.

## Version

Current source version: **v0.2.0**

## License

This project is licensed under the **MIT License**. See [LICENSE](LICENSE).
