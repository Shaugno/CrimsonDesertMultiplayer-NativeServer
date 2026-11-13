# Experimental 3576 live-capture runner

This is a game-free diagnostic. It is not 3576 compatibility, it does not adopt
packet offsets, and it does not open the ASI guard. The guard still accepts
only Crimson Desert 1.0.0.2976. `cdmp.asi` does not link this module.

Fixture runs arm dummy functions in a test executable. A fixture log records
`fixture=1` and `game_match=0`. That is not evidence the host is build 3576.

## What gets checked before hooks

`pkt3576_live_start` reads the loaded process executable. Hooks run only when
all of the following match the documented 1.0.0.3576 identity in
`tools/pkt3576/candidates.h` and `docs/COMPATIBILITY-3576.md`:

- file name `CrimsonDesert.exe`
- mapped base `0x140000000`
- PE timestamp `0x6AED2E85`
- `SizeOfImage` `0x13CC4000`
- 16 bytes at `exe+0x3A6310` and `exe+0x3A66A0`

Those RVAs stay candidates. They are not copied into `offsets.h` or
`offsets_s2.h`. Build 1.0.0.2976 (`0x6AB28F00`) is rejected before either
entry is read. Unreadable headers and signature mismatches install nothing.
There is no production force-unsupported switch. `capture_attempts` stays 0
when identity or the log path is rejected.

The approved backend is `Capture::start`. It runs inside the calling process.
The runner passes that process's module and `base + RVA`. It does not take a
process id or another process's addresses.

## Diagnostic module

`cdmp_pkt3576_live.dll` exports `pkt3576_live_start`, `pkt3576_live_stop`, and
`pkt3576_live_status`. `DllMain` only calls `DisableThreadLibraryCalls`. It
does not install hooks, open a log, or start a thread. Nothing launches the
game or picks a process. The caller passes an absolute log directory.

The test-only build `cdmp_pkt3576_live_fixture.dll` also exports
`pkt3576_live_fixture_start`. The production DLL does not. Fixture limits are
capped (queue 2..256, records 1..256). Production uses a fixed queue of 64 and
64 output records. Packet bytes stay on the existing raw capture path, including
the 16-byte prefix bound.

Before `Capture::start`, the runner pins itself with
`GET_MODULE_HANDLE_EX_FLAG_PIN`. That pin lasts until process exit, so a
successful arm also keeps the DLL mapped. Identity and path refusals do not
pin, and `FreeLibrary` can unload the DLL. If start or stop returns `3`
(`Retained`), the runner leaves the module, the callback code, the queue, and
the writer in place. It does not call `release_owner` and it must not be
unloaded.

Logs are `pkt3576-live-session.txt` and `pkt3576-live-records.txt` in the
caller directory. The session text is capped at 4096 bytes. Relative paths,
the directory of a loaded `CrimsonDesert.exe`, and any `Crimson Desert\bin64`
directory are refused and not created. An existing destination is accepted
only when it is a directory. The first binding is committed only after the
session file is opened and written. A failed first bind is rolled back, so a
later absolute directory can still start. A directory that already has a
committed session stays bound.

`pkt3576_live_stop` contains exceptions from backend stop and from release.
If cleanup is not proved complete, the phase stays Held, the capture owner
and module pin stay in place, and status reports retained. A later start does
not replace that capture, and a later stop does not retry removal. The C
export returns 3 if an exception still reaches it.

## Future authorized live test (not run)

This tree does not contain a remote injector. Do not attach
`cdmp_pkt3576_live.dll` to `CrimsonDesert.exe` until that test is requested.
A crash during capture can be sent, so copy the save first:

`%LOCALAPPDATA%\Pearl Abyss\CD\save`

Proposed sequence, not executed here:

1. Copy the save folder.
2. Start Crimson Desert 1.0.0.3576 from the Steam client. This runner does not
   launch it.
3. From a controller that is already inside that process, load
   `cdmp_pkt3576_live.dll` and call:
   `pkt3576_live_start(<absolute directory outside bin64>)`
   `pkt3576_live_stop()`
4. If `pkt3576_live_stop` returns `3`, leave the DLL loaded.

No `steam://` launch is part of this procedure. The only load exercised by
the current tests is `LoadLibrary` of the production DLL into the identity
test and of the fixture DLL into `cdmp_tests_pkt3576_live_host`.
