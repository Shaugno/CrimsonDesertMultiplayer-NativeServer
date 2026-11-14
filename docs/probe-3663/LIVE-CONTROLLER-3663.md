# 3663 live-capture controller

Date: 2026-11-13. This is the contract for `cdmp_pkt3663_live_ctl.exe`.
It was written before the controller implementation. A fixture run is not a
live capture and does not prove that a candidate is reached in Crimson Desert.

The tool loads and calls the existing exports in `tools/pkt3663/live_api.h`.
It does not add a production export, a packet-call ABI, a force flag, or a
caller-supplied RVA. The 3576 controller and its acceptance stay unchanged.

## Commands

The operator supplies the PID and every path. The tool does not search for a
process, launch Crimson Desert, or accept `steam://`.

```
cdmp_pkt3663_live_ctl.exe status --pid <pid> --dll <abs>\cdmp_pkt3663_live.dll
cdmp_pkt3663_live_ctl.exe load   --pid <pid> --dll <abs>\cdmp_pkt3663_live.dll
cdmp_pkt3663_live_ctl.exe mark   --pid <pid> --dll <abs>\cdmp_pkt3663_live.dll --phase stand
cdmp_pkt3663_live_ctl.exe start  --pid <pid> --dll <abs>\cdmp_pkt3663_live.dll --log-dir <abs outside bin64>
cdmp_pkt3663_live_ctl.exe report --pid <pid> --dll <abs>\cdmp_pkt3663_live.dll
cdmp_pkt3663_live_ctl.exe stop   --pid <pid> --dll <abs>\cdmp_pkt3663_live.dll
cdmp_pkt3663_live_ctl.exe unload --pid <pid> --dll <abs>\cdmp_pkt3663_live.dll
```

`mark` accepts `stand`, `walk`, `turn`, or `unlabeled`. It does not take
`--log-dir` and does not load a missing DLL. The runner stores a phase only
while it is idle, so mark comes before start. A mark during an active capture
returns the runner's Refused code and leaves the open record file on its
original phase. `start` calls `pkt3663_live_start` with the log directory
only. The DLL derives both entries after its own mapped-image check.

`status` is read-only before the DLL is loaded: it reports `not-loaded` and
does not allocate, write, or create a remote thread. `report` calls
`pkt3663_live_report` and requires the DLL to already be loaded. `stop` calls
`pkt3663_live_stop` and leaves a pinned module mapped. `unload` calls
`FreeLibrary` only when the report shows the module cannot have been pinned.

There is no `--force`, `--rva`, `--opcode`, or other passthrough. `--fixture`
on the production executable is usage and does not open the PID.

## Identity before any mutation

The tool opens the PID with query and read rights. It accepts only a native
64-bit process. The first module must be the process image returned by
`QueryFullProcessImageNameW`. The tool then runs `pkt3663::identify_image` on
that mapped image. Remote copies are capped at the identity reader's maximum
single read. The total copied bytes are capped at twice the 1.0.0.3663
`SizeOfImage` plus 1 MiB, which covers one code scan with the anchor overlap
and one pass over the exception directory.

`load` and `start` require `IdentityKind::Match`: file name
`CrimsonDesert.exe`, timestamp `0x6AF5D198`, mapped and preferred base
`0x140000000`, `SizeOfImage` `0x144E9000`, valid sections, exact pdata ranges,
unique anchors, and the two decoded pump calls. A 2976 or 3576 timestamp,
malformed image, wrong base or size, missing function table, non-executable
range, anchor or call mismatch, or ambiguous anchor refuses before
`VirtualAllocEx`, `WriteProcessMemory`, `CreateRemoteThread`, or `FreeLibrary`.
`ctl_modified` stays 0. The runner repeats this check in process. The
controller does not weaken that check and does not pass an address into the
production export.

After this module's own hooks replace an entry, a pristine match becomes
`anchor-mismatch`. `load` and `start` stay refused. `status`, `report`,
`stop`, and `mark` may call the already loaded module so the operator can
query and stop the capture that replaced those bytes. `CallTargetMismatch`,
`AmbiguousAnchor`, a function-range or section failure, and every wrong-build
kind stay refused even if a DLL is mapped. That installed exception is not a
force flag and does not authorize another install.

A loaded module is accepted only when its file name and normalized full path
match `--dll`. Two modules with that file name, or the same name at another
path, refuse before an export call. The production tool accepts only
`cdmp_pkt3663_live.dll`. The DLL path must be absolute.

`status` and `stop` of a rejected process exit 4. They do not report
`not-loaded`. `status` of an accepted process whose DLL is absent exits 0
with `ctl_note=not-loaded`.

## Log directory and phases

`start` checks the log directory before injection, using the same rule as the
runner. A relative path, any `Crimson Desert\bin64` path, or the directory of
a loaded `CrimsonDesert.exe` sets `ctl_start=6`, `ctl_note=bad-log`, and
`ctl_modified=0`. The directory string must be shorter than 1024 wchar units.

Phase files are exclusive creates:

- `pkt3663-live-records-stand.txt`
- `pkt3663-live-records-walk.txt`
- `pkt3663-live-records-turn.txt`

`unlabeled` uses `pkt3663-live-records.txt`. Each start exclusive-creates
`pkt3663-live-session-<n>.txt` and does not rewrite an earlier attempt.
`pkt3663-live-session.txt` mirrors the latest flush. Idle marks that exceed
the bounded buffer truncate only `pkt3663-live-session-pending.txt` and that
mirror. An existing labeled file makes that start return
BadLog. A start that fails before the runner pins the module unloads the DLL
only when this command was the load. A DLL that was already mapped stays
mapped so a later phase can be marked and started. `capture_attempts` stays
cumulative. A successful arm pins the module until the target process exits.

## Results

Every run prints `ctl_action`, `ctl_pid`, `ctl_identity`,
`ctl_rejected_2976`, `ctl_rejected_3576`, `ctl_gate`, `ctl_modified`,
`ctl_loaded`, `ctl_start`, `ctl_stop`, `ctl_retained`, `ctl_stored`,
`ctl_omitted`, `ctl_unread`, `ctl_truncated`, `ctl_raw_mode`, `ctl_allowlist`,
`ctl_adopted`, `ctl_phase`, `ctl_capture_attempts`, `ctl_qpc_frequency`,
`ctl_unloaded`, `ctl_target_alive`, `ctl_note`, and `ctl_exit`. A loaded
module also prints `ctl_dll_base`. `ctl_phase` is 0 unlabeled, 1 stand,
2 walk, 3 turn. `ctl_raw_mode`, `ctl_allowlist`, and `ctl_adopted` stay -1
until a report is read. A report from this runner uses raw mode 1, an empty
allow-list, and `adopted=0`.

`ctl_exit`:

- 0: the request completed. `stop` that returns Removed is 0 and leaves the DLL mapped. A successful `unload` is 0. BadLog is not completion.
- 2: usage. The process was not opened.
- 3: Retained. The DLL and callback ownership stay. Nothing is force-unloaded and the target is not terminated. A Retained return stays exit 3 when its required report times out, returns an error, or cannot be read.
- 4: identity, architecture, or module path rejected. `ctl_modified=0`.
- 5: the request failed. This includes BadLog, and start, stop, or mark when the required report does not arrive.

`ctl_start` and `ctl_stop` use the runner codes: Started 0, Refused 1,
RolledBack 2, Retained 3, Removed 4, Idle 5, BadLog 6. Each command keeps
the code that call returned, including Refused, RolledBack, and an unknown
error. A report may still show older state, such as an earlier Started
capture or hooks already removed, but that snapshot does not replace the
command result. If either the command or the report is Retained, the
selected result stays Retained. `-1` means that call was not collected.
The PID text must be a decimal value from 1 through 4294967295. Zero,
non-digits, a value that would wrap, and the controller's own PID are usage.

`ctl_modified` becomes 1 only after a remote allocation, write, remote thread,
or `FreeLibrary`. Opening the later write handle is not a modification.
`ctl_target_alive=1` means the PID was still running when the tool exited.
The tool never calls `TerminateProcess` or `TerminateThread`. A remote wait
that reaches 20 seconds is `ctl_remote_wait=timeout`. Closing the waited
thread handle does not cancel that thread, and its argument allocation stays.
`ctl_thread_created=1` means the thread was created. `ctl_remote_wait_ms` is
the measured wait for the command thread, or `-1` when that command created
none. The session log records `stage=` lines with a QPC and `elapsed_us`
from the export entry. A missing stage does not mean an earlier stage was
cancelled.

## Failure and cleanup

Remote calls wait 20 seconds. On timeout the scratch allocation stays,
because the remote thread may still read it, and the tool does not call
`FreeLibrary`. Start, stop, and mark each require a report. If that report
times out, returns an error, or cannot be read, the command fails, keeps
the failure note, and leaves `ctl_raw_mode` at -1. The module stays mapped.
A known Retained return uses exit 3 and still keeps that failure note.

A stop that removes the hooks and then fails the session flush keeps BadLog
in `ctl_stop` and returns exit 5. The session file keeps `shutdown=removed`
and `session_write_failed=1`. That file, and a later `report`, are the
record that the hooks came out. The snapshot's Removed code is not the stop
command's result. A second start while a capture is active returns Refused.
Its counters stay on the original attempt, the module stays mapped, and a
later stop can still remove that capture. `Retained`, a nonzero
`capture_attempts`, or a start code of Started, RolledBack, or Retained
keeps the module mapped. `unload` of a
pinned module returns `ctl_note=pinned` or `retained` and `ctl_unloaded=0`.

If the target has already exited, the tool does not write, free remote
memory, or unload. The note is `process-exited` when the process can still
be opened and observed, or `open-failed` when it cannot. The target's exit
code is left as it was.

A refused load that never pinned may `FreeLibrary` once. An uncertain
removal returns `ctl_note=unload-failed` or `remote-timeout` and leaves the
module mapped.

## Test controller

`cdmp_pkt3663_live_ctl_fixture.exe` is a test build. `--fixture` exists only
there. It calls `pkt3663_live_fixture_arm` and refuses every process whose
file name is `CrimsonDesert.exe`, including one whose timestamp was edited.
The arm addresses come from the dummy host exports `pkt3663_dummy_send` and
`pkt3663_dummy_receive`. The production DLL does not export fixture functions,
and the production controller cannot name the fixture DLL.

The game-free host is `cdmp_pkt3663_live_dummy.exe`. Fixture logs record
`fixture=1` and `game_match=0`. Those logs are not 3663 compatibility and
are not evidence from Crimson Desert.

## Boundaries

- Same-user native 64-bit targets only. The tool does not raise privileges.
- The first successful log directory stays bound for the life of the loaded module.
- Production queue and output storage are 256 records. `raw_word0` is not an opcode.
- The copy cap stays 4096. The controller does not replay packets or inject messages.
- `Retained`, a failed session flush, `session_history_truncated=1`, an identity mismatch, or a missing `recording_ready=1` stops acceptance of a future session.
- Gameplay offsets, the ASI guard, and `tools/pkt3576` stay unchanged.
- This contract does not authorize attaching to Crimson Desert.

## Validation

Recorded 2026-11-13 on `grok/compat-guard` at `0b03641`, with the controller
changes still local. Build tree:
`NativeServer-build`, Visual Studio 18 2026, RelWithDebInfo, Windows SDK
10.0.26100.0, MSVC 19.52.37029 (toolset 14.52.37025). Crimson Desert stayed
closed. No process named `CrimsonDesert.exe` was running after the suite.

Direct run of `bin\RelWithDebInfo\cdmp_tests_pkt3663_live_ctl.exe` exited 0
and printed `pkt3663 live ctl tests ok`. Observed identity lines:

- timestamp `0x6AB28F00`: `wrong-timestamp`, `rejected_2976=1`, `rejected_3576=0`
- timestamp `0x6AED2E85`: `wrong-timestamp`, `rejected_2976=0`, `rejected_3576=1`
- forged timestamp `0x6AF5D198` on the dummy image: `wrong-base`, both rejection flags 0
- in-memory `e_lfanew` of `0x10`: `malformed-image`, `modified=0`
- crowded start with the dummy's sleeper threads: `exit=0`, `start=0`, `note=started`
- crowded stop: `exit=3`, `stop=3`, `retained=1`, `note=retained`
- process exit: `note=process-exited`, `modified=0`, `unloaded=0`, `alive=0`

The forged timestamp stopped at `wrong-base` before any anchor check. Load,
start, and status of that image exited 4 with `modified=0` and `loaded=0`.
The renamed dummy is not the installed game image, so this does not prove the
installed file's mapped base.

The same direct run also covered parser refusal, production and fixture DLL
separation, a positive fixture load/mark/start/report/stop, an existing stand
file returning `BadLog` with `capture_attempts=0`, a later walk file, mark
during capture returning Refused, module-path mismatch, and an explicit unload
before any start. Fixture session text contained `fixture=1`, `game_match=0`,
`raw=1`, `allowlist_count=0`, `adopted=0`, and `recording_ready=1`.

`ctest --test-dir NativeServer-build -C RelWithDebInfo --output-on-failure`
passed 100/100 in 60.24 seconds. `pkt3663_live_ctl` passed in 3.74 seconds.
`pkt3663_live` and `pkt3663_live_host` passed after the shared log-directory
move. These results are game-free. They are not a live 3663 capture.
