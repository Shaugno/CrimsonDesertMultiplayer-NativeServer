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

`cdmp_pkt3576_live.dll` exports `pkt3576_live_start`, `pkt3576_live_stop`,
`pkt3576_live_status`, `pkt3576_live_mark`, and the one-pointer
`pkt3576_live_report`. `DllMain`
only calls `DisableThreadLibraryCalls`. It
does not install hooks, open a log, or start a thread. Nothing launches the
game or picks a process. The caller passes an absolute log directory.

The test-only build `cdmp_pkt3576_live_fixture.dll` also exports
`pkt3576_live_fixture_start`. The production DLL does not. Fixture limits are
capped (queue 2..256, records 1..256). Production uses a fixed queue of 256
and 256 output records. Selected messages are copied up to 4096 bytes, and
`claimed` stays separate from `copied`. The earlier PID 39068 capture used 64
records and a 16-byte prefix. That archive is `LIVE-CAPTURE-3576.md` and is
not replay input.

A labeled records file is created with exclusive mode `wbx`. The runner still
returns BadLog without incrementing attempts when that file already exists
before startup. If the file appears after that check, the open fails, the
existing bytes stay unchanged, and the attempt counts. The unlabeled records
file still uses truncating `wb`. `start`, `stop`, `mark`, `status`, and
`report` share one control lock. Packet callbacks do not take it. `mark`
holds that lock across the phase update, so stop cannot release the capture
while the update is in progress. A retained capture stays allocated.

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

## Controller

`cdmp_pkt3576_live_ctl.exe` loads this DLL, calls `pkt3576_live_start`,
`pkt3576_live_report`, and `pkt3576_live_stop`, and prints the result. It
checks the documented identity before any remote allocation, write, or thread.
`load` and `start` require the pristine entry signatures. `status` and `stop`
of an already loaded production module still run after that module's hooks
change those bytes. Name, timestamp, base, and size are still required. The
DLL repeats the full check inside `pkt3576_live_start`. Commands, Retained
behavior, and limits are in `docs/probe-3576/LIVE-CAPTURE-CONTROL.md`. An
earlier load, start, and stop is recorded in `LIVE-CAPTURE-3576.md`. The
`mark` command and the 256-record collector have not been run against the
game.

`pkt3576_live_report` is the one-pointer form of status. It does not skip the
identity check inside `pkt3576_live_start`. The production DLL does not export
a fixture entry.

## Future authorized live test (not run)

Do not attach `cdmp_pkt3576_live.dll` to `CrimsonDesert.exe` until that test
is requested. A crash during capture can be sent, so copy the save first:

`%LOCALAPPDATA%\Pearl Abyss\CD\save`

Proposed sequence, not executed here:

1. Copy the save folder.
2. Start Crimson Desert 1.0.0.3576 from the Steam client. This runner does not
   launch it.
3. Run the four production commands in `LIVE-CAPTURE-CONTROL.md` against that
   PID. Those commands were not run for this change.
4. If stop reports Retained (`ctl_exit=3`), leave the DLL loaded. Do not
   terminate the game.

No `steam://` launch is part of this procedure. The control flow exercised by
the current tests is `cdmp_pkt3576_live_dummy.exe` plus in-process `LoadLibrary`
of the production DLL in the identity test and of the fixture DLL in
`cdmp_tests_pkt3576_live_host`. Fixture success is not 3576 compatibility.
