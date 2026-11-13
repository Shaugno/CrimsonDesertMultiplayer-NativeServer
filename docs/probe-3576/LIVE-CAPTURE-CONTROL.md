# Live-capture controller

This loads, arms, queries, and stops `cdmp_pkt3576_live.dll` in a process the
operator names by PID. It does not search for a process, launch Crimson Desert,
or accept `steam://`. The commands below were not run against the game.

The DLL repeats its own identity check after it is loaded. This tool's gate is
additional. There is no production force flag. The ASI guard still accepts only
1.0.0.2976. Packet candidates stay out of `offsets.h` and `offsets_s2.h`.

## Future operator commands (not run)

Replace `<pid>` with the running 1.0.0.3576 process. Replace the DLL path with
the absolute path of the built production module. The log directory must be
absolute and must sit outside `Crimson Desert\bin64` and outside the directory
of `CrimsonDesert.exe`.

```
cdmp_pkt3576_live_ctl.exe load --pid <pid> --dll <abs>\cdmp_pkt3576_live.dll
cdmp_pkt3576_live_ctl.exe start --pid <pid> --log-dir <abs outside bin64> --dll <abs>\cdmp_pkt3576_live.dll
cdmp_pkt3576_live_ctl.exe status --pid <pid> --dll <abs>\cdmp_pkt3576_live.dll
cdmp_pkt3576_live_ctl.exe stop --pid <pid> --dll <abs>\cdmp_pkt3576_live.dll
```

Before any later in-game use, copy `%LOCALAPPDATA%\Pearl Abyss\CD\save`. A
crash during capture can be sent. Start the game from the Steam client. Do not
use `scripts/launch-2inst.ps1`.

## What happens before a process is modified

The tool opens the PID with query and read rights only. It reads the first
module in that process (the executable) and runs the same documented identity
check as the DLL: file name `CrimsonDesert.exe`, mapped base `0x140000000`,
PE timestamp `0x6AED2E85`, `SizeOfImage` `0x13CC4000`, then both 16-byte
candidate signatures. Build 1.0.0.2976 (`0x6AB28F00`) is rejected before those
signature reads. The PID text must be a decimal value from 1 through
4294967295. Zero, non-digits, and any larger number are usage and do not open
a process.

`load` and `start` require that full match, including the pristine entry
bytes. Only then may they allocate, write, or create a thread. Opening the
later write handle is not reported as a modification. `ctl_modified` stays 0
until `VirtualAllocEx`, `WriteProcessMemory`, `CreateRemoteThread`, or
`FreeLibrary` runs. A rejected name, timestamp, base, or size does not load
the DLL. `status` and `stop` on that rejected process exit 4 and do not report
`not-loaded`.

`status` and `stop` of an already loaded `cdmp_pkt3576_live.dll` are also
allowed when the name, timestamp, base, and size still match and only the
entry bytes differ. That difference is what this module's own hooks write.
`ctl_identity` stays `signature-mismatch` and `ctl_gate` is `installed`. This
is not a match, not a force flag, and it does not authorize `load` or `start`.
A signature mismatch with that module absent still exits 4 with
`ctl_modified=0`.

The tool is 64-bit and accepts only a native 64-bit target. The DLL path must
be absolute. The production tool accepts only the file name
`cdmp_pkt3576_live.dll`. `--fixture` on the production tool is a usage error
and does not open the PID.

`start` checks the log directory before injection. A relative path, the game
executable's directory, or any `Crimson Desert\bin64` path sets `ctl_start=6`,
`ctl_note=bad-log`, `ctl_modified=0`, and does not load the DLL.

## Results

Every run prints `ctl_action`, `ctl_pid`, `ctl_identity`, `ctl_rejected_2976`,
`ctl_gate`, `ctl_modified`, `ctl_loaded`, `ctl_start`, `ctl_stop`,
`ctl_retained`, `ctl_stored`, `ctl_omitted`, `ctl_capture_attempts`,
`ctl_unloaded`, `ctl_target_alive`, `ctl_note`, and `ctl_exit`. A loaded module
also prints `ctl_dll_base`.

`ctl_exit` is the controller result:

- 0: the request completed. `stop` that returns Removed is 0 and leaves the DLL mapped.
- 2: usage. The process was not opened.
- 3: Retained. The DLL, capture owner, writer, queue, callback code, and module pin stay. Nothing is force-unloaded and the target is not terminated.
- 4: identity or architecture rejected. `ctl_modified=0`.
- 5: the request failed. A refused load that never pinned may `FreeLibrary` once. A timeout or an unreadable report does not unload and does not kill the process.

`ctl_start` and `ctl_stop` use the module codes: Started 0, Refused 1,
RolledBack 2, Retained 3, Removed 4, Idle 5, BadLog 6. `-1` means that call
was not collected.

`ctl_target_alive=1` means the PID was still running when the tool exited.
This tool never calls `TerminateProcess` or `TerminateThread`. On a remote-thread
timeout the scratch allocation stays, because the thread may still read it.

## Retained and the pin

A successful arm pins the DLL until the target process exits, including after
a clean stop (`ctl_stop=4`, `ctl_unloaded=0`). `stop` never unloads the module.
`Retained` (`ctl_exit=3`) also leaves it mapped. Do not loop `FreeLibrary`.
The next `start` after Retained does not replace the capture, and the next
`stop` does not retry removal.

Identity and path refusals do not pin. `FreeLibrary` is used only when the
module cannot have been pinned and the runner did not ask to keep ownership.

## Test controller

`cdmp_pkt3576_live_ctl_fixture.exe` is a test build. It is not an operator tool
for the game. `--fixture` exists only there, and only to call
`pkt3576_live_fixture_arm` in a process whose executable is not
`CrimsonDesert.exe`. It refuses the game even if the identity matches. The
production DLL does not export `pkt3576_live_fixture_start` or
`pkt3576_live_fixture_arm`.

The game-free host is `cdmp_pkt3576_live_dummy.exe`. Fixture logs from that
host record `fixture=1` and `game_match=0`. That is not 3576 compatibility.

## Remaining limitations

- No process picker and no launch. The operator supplies the PID. A value
  above 4294967295 is usage and does not open a process.
- Same-user native 64-bit targets only. The tool does not raise privileges.
- The production tool loads only `cdmp_pkt3576_live.dll`.
- The first successful log directory stays bound for the life of the module.
  The file names are `pkt3576-live-session.txt` and `pkt3576-live-records.txt`.
  The session text cap is 4096 bytes.
- Remote calls wait 20 seconds. A timed-out call is left in place.
- `status` of a module that is not loaded is success with `ctl_note=not-loaded`.
  `stop` of a module that is not loaded fails with that note and does not write.
- A pin or a Retained result lasts until the target process exits on its own.
- These commands have not been run on Crimson Desert 1.0.0.3576.
- Gameplay compatibility, adopted offsets, and an open ASI guard are unchanged.
