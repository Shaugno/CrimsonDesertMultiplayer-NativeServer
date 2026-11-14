# Codex review: 3663 live-capture controller

Date: 2026-11-13. Branch `grok/compat-guard`, saved checkpoint `0b03641`
(`0b03641d5c91f8cbde4b404b22ce7167cea38183`). The implementation below is
local and uncommitted. Do not publish it from this note.

The contract is `docs/probe-3663/LIVE-CONTROLLER-3663.md`. It was written
before the controller code. Crimson Desert stayed closed. No `cdmp.asi` or
`winmm.dll` was installed, and `launch-2inst.ps1` was not run. Fixture output
is not evidence from the game. Working co-op has not been demonstrated.

## What changed

New files:

- `tools/pkt3663/live_control.h`
- `tools/pkt3663/live_control.cpp`
- `tools/pkt3663/live_ctl.cpp`
- `tests/pkt3663_live_dummy.cpp`
- `tests/pkt3663_live_ctl_tests.cpp`
- `docs/probe-3663/LIVE-CONTROLLER-3663.md`

Edited:

- `CMakeLists.txt` adds `cdmp_pkt3663_live_control`, `cdmp_pkt3663_live_ctl`,
  `cdmp_pkt3663_live_ctl_fixture`, `cdmp_pkt3663_live_dummy`, and
  `cdmp_tests_pkt3663_live_ctl` (`pkt3663_live_ctl`, timeout 180).
- `tools/pkt3663/live_identity.h` publishes `kExeName`, `kMappedReadCap`, and
  `log_directory_allowed`.
- `tools/pkt3663/live_identity.cpp` shares that log-directory rule and rejects
  a mapped read above `kMappedReadCap`.
- `tools/pkt3663/live_runner.cpp` calls the shared log-directory function.
  Hook ownership, candidate addresses, and the recording-ready cutoff are
  unchanged.
- `docs/ROADMAP.md`, `docs/PROGRESS.txt`,
  `docs/probe-3663/PLAN-3663-READY.md`, and
  `docs/probe-3663/LIVE-RUNNER-3663.md` point at this local controller.

`tools/pkt3576` is untouched. Production offsets and the ASI guard are
untouched. The production DLL still exports only `pkt3663_live_start`,
`pkt3663_live_stop`, `pkt3663_live_report`, and `pkt3663_live_mark`.
`pkt3663_live_start` still takes the log directory alone.

## Behavior to review

The operator passes an explicit PID and absolute DLL path. The tool does not
search for a process. `status` before load is read-only. Load and start
require `IdentityKind::Match` on the production path. The fixture controller
requires `--fixture`, the fixture DLL name, and a target whose file name is
not `CrimsonDesert.exe`.

After the runner's entry hooks, `AnchorMismatch` with the requested module
loaded allows status, report, stop, mark, and the unload check. Load and
start stay refused. Wrong name, timestamp, base, or size, and
`Unreadable`, `MalformedImage`, `MissingFunctionTable`, `NotExecutable`,
`FunctionRangeMismatch`, `AmbiguousAnchor`, and `CallTargetMismatch`, refuse
every command even when a DLL is mapped.

The process image must match the first module snapshot. The loaded module
must match the requested DLL file name and normalized path. Two modules with
that file name are `module-ambiguous`. Remote identity copies are capped per
read at `kMappedReadCap` and in total at twice the 1.0.0.3663 `SizeOfImage`
plus 1 MiB.

An existing labeled record file returns `BadLog` before `pin_module` and
before `capture_attempts` increments. The controller leaves a module that was
already loaded in place, so a later idle mark and start can use another
phase. A module loaded by that same command is unloaded only when the report
shows it cannot have been pinned. `stop` never unloads. Explicit unload
requires `retained=0`, `capture_attempts=0`, and a start code other than
Started, RolledBack, or Retained.

A remote wait is 20 seconds. Timeout keeps the remote allocation, does not
call report again, and does not unload. If the target has exited, the tool
does not write, free remote memory, or unload. `ctl_modified` stays 0 until
a remote allocation, write, thread, or `FreeLibrary`.

Fixture arm addresses come from `pkt3663_dummy_send` and
`pkt3663_dummy_receive` in the dummy host. Queue and record capacities are 8.
There is no caller RVA and no force flag.

## Commands and results

Sibling build, not `scripts/build.ps1`:

```
cmake -S NativeServer-work2 -B NativeServer-build
cmake --build NativeServer-build --config RelWithDebInfo --target cdmp_tests_pkt3663_live_ctl --parallel
bin\RelWithDebInfo\cdmp_tests_pkt3663_live_ctl.exe
ctest --test-dir NativeServer-build -C RelWithDebInfo --output-on-failure
```

Visual Studio 18 2026, RelWithDebInfo, Windows SDK 10.0.26100.0, MSVC
19.52.37029, toolset 14.52.37025. The direct test exited 0 and printed
`pkt3663 live ctl tests ok`. Observed lines:

```
observed timestamp-2976 load=wrong-timestamp start=wrong-timestamp status=wrong-timestamp rejected_2976=1 rejected_3576=0
observed timestamp-3576 load=wrong-timestamp start=wrong-timestamp status=wrong-timestamp rejected_2976=0 rejected_3576=1
observed timestamp-3663-forged load=wrong-base start=wrong-base status=wrong-base rejected_2976=0 rejected_3576=0
observed malformed-image identity=malformed-image note=malformed-image
observed crowded-start exit=0 start=0 note=started
observed crowded-stop exit=3 stop=3 retained=1 note=retained
observed process-exit note=process-exited modified=0 unloaded=0 alive=0
```

The forged `0x6AF5D198` timestamp on the dummy image stopped at `wrong-base`.
The crowded start itself returned Started. Stop then returned Retained, unload
stayed refused, and the dummy stayed alive. The process-exit case left the
dummy's exit code at 77.

CTest passed 100/100 in 60.24 seconds. `pkt3663_live_ctl` took 3.74 seconds.
`pkt3663_live` and `pkt3663_live_host` passed. No `CrimsonDesert.exe` was
running afterward.

## Limitations

- The 20-second remote-thread timeout is covered by `keep_remote_allocation`
  and `unload_after_observed` checks. There is no stall export, so that wait
  is not integration-tested. A timed-out thread keeps its allocation.
- The crowded path observed here was a successful start followed by a retained
  stop. A start that itself returns Retained remains an accepted uncertain
  result in the test, and this run did not hit it.
- `AnchorMismatch` control of an already hooked module is checked in the
  policy table. No test installs the production hooks in a remote process.
- The remote-read budget was not measured against a real 1.0.0.3663 image. If
  a legitimate scan exceeds it, identity returns `Unreadable` and the
  controller refuses.
- Renamed dummy copies and an in-memory `e_lfanew` patch are fixture evidence.
  They do not prove the installed executable.
- A live capture, opcode, payload layout, and co-op session are all still
  absent.

## Proposed manual capture checklist

Do this only after Codex accepts this diff, and only when the user asks for
the live session.

1. Re-read the installed `CrimsonDesert.exe`. Require version 1.0.0.3663,
   timestamp `0x6AF5D198`, preferred and mapped base `0x140000000`, and
   `SizeOfImage` `0x144E9000`. Stop unless the offline verifier still reports
   the current candidates with `adopted=0`.
2. Copy `%LOCALAPPDATA%\Pearl Abyss\CD\save` into `backups/` before any load.
3. Confirm `cdmp.asi` and `winmm.dll` are absent from `bin64`. Leave them
   absent.
4. Have the user launch through Steam. Record that process's PID. Do not
   pick another `CrimsonDesert.exe`.
5. `status --pid <that pid> --dll <absolute>\cdmp_pkt3663_live.dll`. Continue
   only when identity is `match`, the module is not loaded, and
   `ctl_modified=0`.
6. `load` the same PID and DLL. Continue only when the note is `loaded` and
   the report path is still the requested DLL.
7. `mark --phase stand`, then `start` with an absolute log directory outside
   `Crimson Desert\bin64` and outside the game directory. Require
   `recording_ready=1`, `game_match=1`, `raw=1`, `allowlist_count=0`,
   `adopted=0`, `session_history_truncated=0`, and `retained=0`.
8. Have the user stand, then `stop`. Archive that stand record before the
   next window. Repeat as separate mark/start/stop windows for walk and turn.
   An existing labeled file must stay BadLog and must not be overwritten.
9. Stop the session on `Retained`, a failed log flush, history truncation,
   an identity other than `match`, or missing `recording_ready=1`. Do not
   unload a retained module and do not free a timed-out remote allocation.
10. Archive the raw session and record files before interpreting any field.
    Leave candidate addresses unadopted.
