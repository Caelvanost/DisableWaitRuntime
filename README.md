# Disable Wait Runtime

A lightweight SKSE plugin for **Skyrim Special Edition / Anniversary Edition** that disables Skyrim's native **Wait** action at runtime and frees its key binding for use by other mods.

The plugin does **not** replace `controlmap.txt`, does not require an ESP, and does not store persistent data in your save.

## Features

- Disables the native `Wait` input action.
- Frees the default `T` key so it can be used by another mod or MCM hotkey.
- Reapplies the Wait unbind after in-game control remapping.
- Does not replace or patch `controlmap.txt`.
- Does not affect sleeping in beds.
- No ESP or ESL.
- No Papyrus scripts.
- No persistent save-game data.
- Normal Skyrim bindings return after uninstalling the plugin and restarting the game.

## How it works

Skyrim loads its input bindings into `RE::ControlMap`.

After the game's control data has loaded, Disable Wait Runtime searches the gameplay mappings for the event named `Wait` and sets that event to Skyrim's invalid/unmapped input value.

The operation is reapplied after:

- `DataLoaded`
- `NewGame`
- `PostLoadGame`
- menu closure, queued through the SKSE task interface so in-game control remapping has finished before `Wait` is unbound again

This means assigning the Wait action to another key in Skyrim's Controls menu does not permanently restore the Wait function. Once the menu closes, Disable Wait Runtime removes the new Wait binding again.

This avoids distributing a replacement `controlmap.txt` and reduces compatibility problems with UI, controller, hotkey, and control-remapping mods.

## Requirements

- Skyrim Special Edition / Anniversary Edition
- SKSE64
- Address Library for SKSE Plugins

The project is built with **CommonLibSSE-NG**.

## Installation

Install the compiled release archive with Vortex.

The release package contains:

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
5. Open Skyrim's Controls menu and assign the Wait action to `T`.
6. Close the menu and confirm that pressing `T` still does not open Wait.
7. Activate a bed and confirm that sleeping still works.

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

The build script reads the project version from `VERSION`, configures CMake, compiles the plugin, stages the Vortex package under `package/`, and creates the versioned release archive under `dist/`.

For the current version, the output is:

```text
package/
└── SKSE/
    └── Plugins/
        └── DisableWaitRuntime.dll

dist/
└── Disable-Wait-Runtime-v0.2.2.zip
```

The ZIP itself contains only the deployable `SKSE/` tree.

## Versioning

`VERSION` is the source of truth for the project version.

Current source version: **v0.2.2**

Versioning policy:

- Small fixes and minor maintenance changes increment the third number: `0.2.1` → `0.2.2`.
- Larger feature or behavior changes increment the second number and reset the third to zero: `0.2.2` → `0.3.0`.
- `CMakeLists.txt`, the plugin log version, and the deployment ZIP name derive their version from `VERSION`.
- `README.md` and package metadata must be updated whenever `VERSION` changes.

## Project structure

```text
DisableWaitRuntime/
├── .gitignore
├── LICENSE
├── README.md
├── VERSION
├── CMakeLists.txt
├── vcpkg.json
├── vcpkg-configuration.json
├── build_release.bat
└── src/
    ├── PCH.h
    └── plugin.cpp
```

Generated build artifacts are kept out of source control:

```text
build/
package/
dist/
```

## Compatibility

The plugin is intended for Skyrim SE/AE runtimes supported by CommonLibSSE-NG.

Because it modifies the in-memory control map instead of distributing a complete `controlmap.txt`, it is less invasive than traditional control-map replacement mods.

Mods that intentionally modify the `Wait` mapping at runtime may conflict temporarily, but Disable Wait Runtime reapplies its unbind after menu closures and game load events.

## Uninstallation

Disable or remove the mod and restart Skyrim.

The plugin modifies only the in-memory control map, so Skyrim reloads its normal bindings on the next launch. No save cleaning is required.

## License

This project is licensed under the **MIT License**. See [LICENSE](LICENSE).
