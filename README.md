# Crimson Desert Multiplayer — Shaugno Continuation

An independent continuation of the experimental **Crimson Desert** co-op mod, maintained by
**Shaugno**. Our goal is reliable two-player co-op on newer game builds, starting with **1.0.0.3576**.
The C++ project keeps the `cdmp` name and `cdmp.asi` binary used by the original implementation.

Project home: [Shaugno/CrimsonDesertMultiplayer-NativeServer](https://github.com/Shaugno/CrimsonDesertMultiplayer-NativeServer).
Built on [ByJunter's original native-server implementation](https://github.com/ByJunter/CrimsonDesertMultiplayer-NativeServer),
which succeeded [CrimsonDesertMultiplayer](https://github.com/ByJunter/CrimsonDesertMultiplayer).
The native-server approach connects the guest to the host's world through the game's internal server.

> **Status: active continuation, experimental.** Build **1.0.0.3576 compatibility is in development**;
> working co-op on that build has not been demonstrated. The production mod still accepts only
> **1.0.0.2976** (PE timestamp `0x6AB28F00`). The 3576 diagnostics and capture backend are development
> tools, not a compatible gameplay release.

## Community

[Join our Discord](https://discord.gg/fE5GYxrv9V) to follow development, share ideas, and discuss the co-op project.

[Join the Heaven server](https://haven.rmsportalhosting.com/?invite=70f53638).

## Current progress

- A read-only 3576 position diagnostic has been live tested.
- A passive packet observer and real SafetyHook capture backend have passed game-free tests.
- A version-checked diagnostic module now exposes explicit in-process start/stop/status control.
- The latest approved runner passed **92 tests**, four AddressSanitizer test programs, and the
  original stop/log-recovery reproductions. Real hooks were tested on dummy functions only.
- Next: select and review the game-loading/controller route, then gather controlled live evidence
  after a separate test instruction. The module does not include a remote injector.

See [progress](docs/PROGRESS.txt), the [latest runner review](docs/probe-3576/LIVE-RUNNER-APPROVAL-20261112.txt),
and the [continuation roadmap](docs/ROADMAP.md). Experimental candidate addresses stay isolated until
their intended use is verified. The production compatibility guard remains in place.

## Requirements

- A legitimately purchased copy of **Crimson Desert** on Steam. The original gameplay implementation
  targets **1.0.0.2976**; our current compatibility research targets **1.0.0.3576**. The mod does not work
  without the original game, and this repository contains **no game code, assets or data**.
- Windows 10/11 x64.
- Visual Studio 2022 or 2026, or the corresponding Build Tools, with the "Desktop development with C++"
  workload (MSVC x64, CMake). The build downloads Dear ImGui and safetyhook with CMake `FetchContent`.
- [Ultimate ASI Loader](https://github.com/ThirteenAG/Ultimate-ASI-Loader) v9.7.4 to install the mod
  (see [third_party/ultimate-asi-loader/README.md](third_party/ultimate-asi-loader/README.md)).

## Upstream co-op behavior on 1.0.0.2976

The game keeps the client/server architecture of an MMO internally: the client talks to an in-process
server through ~1,100 internal "Troc" messages. The current approach ("option D") lets the **host's
internal server accept a second player**: the guest's game client connects to the host's server through a
Steam (or UDP) tunnel.

ByJunter reported live validation with two game copies on one PC, and partly between two PCs over
Steam, on the original supported build. These results have not been reproduced on 3576:

- the guest logs in to the host's world and plays alongside the host's character;
- movement, actions, climbing, gliding, mounts (mount, ride and dismount in both directions), pets;
- NPCs, schedules and travellers in the shared field; fast travel inside the host's field;
- doors, and the host's scenes and cutscenes kept from taking over the guest's client;
- separate save handling for the guest (the host's saves are protected from the guest's writes).

Known gaps: combat latency, time-of-day and weather sync, the world far from the host, shopkeepers (work in
progress, off by default) and many smaller issues. See [docs/ROADMAP.md](docs/ROADMAP.md) and
[docs/OPCION-D-EMULADOR.md](docs/OPCION-D-EMULADOR.md). Most design and research notes are in Spanish, under
[docs/research/](docs/research/).

## Building

```powershell
.\scripts\build.ps1              # configure, build (RelWithDebInfo) and run the unit tests
.\scripts\build.ps1 -SkipTests
```

The output goes to `build\bin\<Config>\` (`cdmp.asi`, `cdmp_peer.exe`, `cdmp_host.exe`, `cdmp_probe.exe`
and the tests).

## Installing

The commands below are the original mod installation workflow for its supported build. This
continuation has no 3576 gameplay release yet. Keep experimental live-capture deployment separate
from ordinary build and review work.

```powershell
.\scripts\deploy.ps1                                  # copies cdmp.asi and the loader (as winmm.dll) to bin64
.\scripts\deploy.ps1 -GameDir "<Steam>\steamapps\common\Crimson Desert\bin64"
.\scripts\deploy.ps1 -Remove                          # uninstall
```

The mod is configured with `bin64\cdmp.ini` (examples in `kit/` and `docs/research/ini/`) and logs to
`bin64\cdmp.log`. Two copies of the game on one PC can be started with `scripts\launch-2inst.ps1`.

## Repository layout

| Folder | Contents |
|---|---|
| `src/common/` | Protocol, reliable channel, session, transports, save guards (with tests) |
| `src/mod/` | The mod: startup, config, overlay, memory reading, network and the option-D session code |
| `tools/` | `cdmp_peer` (relay/test peer), `cdmp_host` (loads the mod without the game), `cdmp_probe` (read-only external reader), helper scripts |
| `tests/` | Unit and integration tests (run by `ctest`) |
| `scripts/` | Build, install, packaging and two-instance launch scripts |
| `kit/` | Example configurations and instructions for testers |
| `docs/` | Roadmap, test plans and research notes |
| `third_party/` | safetyhook patch (Boost Software License 1.0) and the ASI loader instructions |

## Legal

This is an unofficial fan project, not affiliated with or endorsed by Pearl Abyss. Crimson Desert is a
trademark of Pearl Abyss Corp. The repository does not include decompiled game code, game binaries,
extracted game files or save data. The source and research notes reference addresses (RVAs), function
names and short byte signatures of the game executable, as needed to hook it.

## License

GPL v3: see [LICENSE](LICENSE). `third_party/safetyhook-patch` keeps its own license (BSL-1.0).

Original implementation and research: **ByJunter**.

Continuation maintainer: **Shaugno**. Original attribution, source history, and GPL v3 licensing are
preserved. Third-party components retain their existing licenses.
