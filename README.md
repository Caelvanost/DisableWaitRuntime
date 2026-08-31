# Disable Wait Runtime

A lightweight SKSE plugin for **Skyrim Special Edition / Anniversary Edition** that disables Skyrim's native **Wait** action during normal gameplay while preserving the vanilla `Wait` key mapping for UI mods that reuse it.

The plugin does **not** replace `controlmap.txt`, does not require an ESP, and does not store persistent data in your save.

## Features

- Prevents the native **Wait** action from reaching Skyrim's gameplay/menu handlers.
- Preserves Skyrim's `Wait` control mapping instead of setting it to unmapped.
- Keeps the default `T` mapping available to UI mods such as RaceMenu that query the vanilla Wait binding.
- Leaves the physical key code intact for other SKSE hotkey mods.
- Uses CommonLib/SKSE event APIs only; no executable code hook or trampoline is installed.
- Does not replace or patch `controlmap.txt`.
- Does not affect sleeping in beds.
- No ESP or ESL.
- No Papyrus scripts.
- No persistent save-game data.
- Safe to uninstall.

## How it works

Older versions removed the `Wait` mapping from `RE::ControlMap`. That stopped the Wait menu, but it also removed the mapping used contextually by some UI mods. RaceMenu, for example, uses Skyrim's vanilla `Wait` binding for its **Choose Texture** action.

Version 0.3.x leaves `RE::ControlMap` untouched.

Starting with **0.3.3**, Disable Wait Runtime no longer waits for the Sleep/Wait menu to open and then closes it. Instead, it filters the transient input event itself:

1. The plugin registers an input sink when SKSE reports `kInputLoaded`.
2. It moves that sink to the front of `BSInputDeviceManager`'s sink list while preserving the relative order of all existing sinks.
3. During normal unpaused gameplay, if an input event carries the vanilla user event `Wait`, only that transient event is renamed before Skyrim's native handlers receive it.
4. The physical key code and the `ControlMap` binding are not modified.
5. While a pausing UI menu is active, the plugin leaves the `Wait` user event untouched so UI mods such as RaceMenu can continue to use it.

This avoids both the compatibility problem of unbinding `Wait` and the instability of low-level UI hooks.

## Requirements

- Skyrim Special Edition / Anniversary Edition
- SKSE64
- Address Library for SKSE Plugins

The project is built with **CommonLibSSE-NG**.

## Installation

Install the compiled release archive with Vortex or another mod manager.

The release package contains:

```text
SKSE/
└── Plugins/
    └── DisableWaitRuntime.dll
```

Enable the mod and launch Skyrim through SKSE or your normal Skyrim Together setup.

## Testing

After loading a save:

1. Make sure Skyrim's **Wait** action is assigned to `T` in the Controls menu.
2. Press `T` during normal gameplay.
3. Confirm that the native Wait menu does not open.
4. Confirm that another mod using the physical `T` key can still react to it.
5. Open RaceMenu and verify that **Choose Texture** still uses the vanilla Wait-bound key.
6. Activate a bed and confirm that sleeping still works normally.

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
└── Disable-Wait-Runtime-v0.3.3.zip
```

The ZIP itself contains only the deployable `SKSE/` tree.

## Versioning

`VERSION` is the source of truth for the project version.

Current source version: **v0.3.3**

Versioning policy:

- Small fixes and minor maintenance changes increment the third number: `0.3.2` → `0.3.3`.
- Larger feature or behavior changes increment the second number and reset the third to zero: `0.3.3` → `0.4.0`.
- `CMakeLists.txt`, the plugin log version, and the deployment ZIP name derive their version from `VERSION`.
- `README.md` and package metadata must be updated whenever `VERSION` changes.

## Changelog

### 0.3.3

- Replaced the delayed Sleep/Wait menu-closing approach with input-event filtering.
- Moves the Disable Wait Runtime input sink ahead of Skyrim's native input handlers.
- Neutralizes only the transient gameplay `Wait` user event.
- Preserves the physical key code and the `ControlMap` mapping for RaceMenu and other hotkey mods.
- No low-level executable hook or trampoline.

### 0.3.2

- Removed the low-level `UIMessageQueue::AddMessage` branch hook that could crash during SKSE plugin startup.
- Replaced the hook with CommonLib/SKSE input and menu event sinks.
- Preserved the vanilla `Wait` mapping for RaceMenu compatibility.

### 0.3.1

- Fixed the CommonLibSSE-NG 3.5.3 hook build by using the SKSE trampoline API for the branch hook.

### 0.3.0

- Reworked Wait suppression so the vanilla `Wait` mapping is preserved.
- Added RaceMenu compatibility for actions that query Skyrim's Wait binding, including **Choose Texture**.

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

Because it preserves Skyrim's control mapping and filters only the transient gameplay input event, it is less invasive than replacing `controlmap.txt`, removing the Wait binding entirely, or patching executable code.

### RaceMenu

Version 0.3.x is designed to preserve RaceMenu's **Choose Texture** shortcut. RaceMenu can continue to query and receive the vanilla `Wait` mapping while its pausing menu is active.

## Uninstallation

Disable or remove the mod and restart Skyrim.

The plugin does not persistently modify Skyrim's controls or save data. No save cleaning is required.

## License

This project is licensed under the **MIT License**. See [LICENSE](LICENSE).
