# Disable Wait Runtime

A lightweight SKSE plugin for **Skyrim Special Edition / Anniversary Edition** that prevents Skyrim's native **Wait** menu from remaining open during normal gameplay while preserving the vanilla `Wait` key mapping for UI mods that reuse it.

The plugin does **not** replace `controlmap.txt`, does not require an ESP, and does not store persistent data in your save.

## Features

- Suppresses the native **Wait** menu triggered by the gameplay Wait input.
- Preserves Skyrim's `Wait` control mapping instead of setting it to unmapped.
- Keeps the default `T` mapping available to UI mods such as RaceMenu that query the vanilla Wait binding.
- Allows other SKSE mods and hotkey systems to react to the same physical key.
- Uses CommonLib/SKSE event sinks only; no executable code hook or trampoline is installed.
- Does not replace or patch `controlmap.txt`.
- Does not affect sleeping in beds.
- No ESP or ESL.
- No Papyrus scripts.
- No persistent save-game data.
- Safe to uninstall.

## How it works

Version 0.3.x changes the implementation from earlier releases.

Older versions removed the `Wait` mapping from `RE::ControlMap`. That successfully stopped the Wait menu, but it also removed the mapping used contextually by some UI mods. RaceMenu, for example, uses Skyrim's vanilla `Wait` binding for its **Choose Texture** action.

Version 0.3.x leaves `RE::ControlMap` untouched.

Version 0.3.2 also removes the low-level `UIMessageQueue` hook introduced during development of the 0.3.x implementation. The plugin now relies only on event sinks exposed through CommonLibSSE-NG/SKSE.

Disable Wait Runtime now:

1. Watches gameplay input events for the vanilla `Wait` user event.
2. When a gameplay Wait press is detected, briefly arms suppression for the next `Sleep/Wait Menu` opening.
3. Watches `MenuOpenCloseEvent` notifications.
4. If `Sleep/Wait Menu` opens during the suppression window, queues a `Hide` message on the next SKSE task.
5. Leaves other `Sleep/Wait Menu` openings alone, including openings caused by activating beds.

The suppression window is intentionally short so an unrelated later Sleep/Wait menu opening is not affected.

Because the Wait mapping itself remains intact, UI mods can continue to query and reuse that binding.

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
3. Confirm that the native Wait menu is immediately suppressed.
4. Confirm that another mod using `T` can still react to the key.
5. Open RaceMenu and verify that **Choose Texture** still uses the vanilla Wait-bound key.
6. Activate a bed and confirm that the Sleep/Wait menu still opens normally for sleeping.

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
└── Disable-Wait-Runtime-v0.3.2.zip
```

The ZIP itself contains only the deployable `SKSE/` tree.

## Versioning

`VERSION` is the source of truth for the project version.

Current source version: **v0.3.2**

Versioning policy:

- Small fixes and minor maintenance changes increment the third number: `0.3.1` → `0.3.2`.
- Larger feature or behavior changes increment the second number and reset the third to zero: `0.3.2` → `0.4.0`.
- `CMakeLists.txt`, the plugin log version, and the deployment ZIP name derive their version from `VERSION`.
- `README.md` and package metadata must be updated whenever `VERSION` changes.

## Changelog

### 0.3.2

- Removed the low-level `UIMessageQueue::AddMessage` branch hook that could crash during SKSE plugin startup.
- Replaced the hook with safe CommonLib/SKSE input and menu event sinks.
- Preserves the vanilla `Wait` mapping for RaceMenu compatibility.
- Keeps bed sleeping unaffected.

### 0.3.1

- Fixed the CommonLibSSE-NG 3.5.3 hook build by using the SKSE trampoline API for the branch hook.

### 0.3.0

- Reworked Wait suppression so the vanilla `Wait` mapping is preserved.
- Added RaceMenu compatibility for actions that query Skyrim's Wait binding, including **Choose Texture**.
- Suppresses only the gameplay-triggered Sleep/Wait menu request.

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

Because it preserves Skyrim's control mapping and uses only CommonLib/SKSE event APIs, it is less invasive than replacing `controlmap.txt`, removing the Wait binding entirely, or patching executable code.

### RaceMenu

Version 0.3.x is specifically designed to preserve RaceMenu's **Choose Texture** shortcut. RaceMenu can continue to query the vanilla `Wait` mapping while the native gameplay Wait menu is suppressed.

## Uninstallation

Disable or remove the mod and restart Skyrim.

The plugin does not persistently modify Skyrim's controls or save data. No save cleaning is required.

## License

This project is licensed under the **MIT License**. See [LICENSE](LICENSE).
