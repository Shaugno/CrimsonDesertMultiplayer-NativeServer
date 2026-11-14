# Codex review of the uncommitted 3663 controller

Date: 2026-11-13. Base checkpoint: `0b03641`. Disposition: changes requested.
The controller is not approved for live capture or publication in this revision.
The game stayed closed. This review changed no implementation source and did
not commit or publish Grok's changes.

## Findings

### P1: the report snapshot overwrites a failed stop command

Location: `tools/pkt3663/live_control.cpp:975-989`, specifically the
`apply_report` call at line 981. `apply_report` replaces `out.stop` with
`report.stop_code` at line 727.

The runner intentionally distinguishes removal of hooks from durable logging.
In `tools/pkt3663/live_runner.cpp:746-753` it stores Removed as its cleanup
state, then returns BadLog if the final session flush fails. The controller
initially records that BadLog return, but then replaces it with the report's
Removed state and returns success. It therefore accepts a failed logging
operation as a clean stop. The same overwrite can also discard a conservative
Retained return if an export's exception boundary reports it while the report
still contains older state.

Reproduced using the actual fixture runner and owned dummy process:

1. Start a fixture capture through `cdmp_pkt3663_live_ctl_fixture`.
2. In that dummy only, invoke the existing fixture export
   `pkt3663_live_fixture_fail_log_flush_after(0)`.
3. Run controller `stop`, then inspect the session log.

Observed:

```text
ctl_action=stop
ctl_stop=4
ctl_retained=0
ctl_note=removed
ctl_exit=0
session_write_failed_marker=1
```

Expected: preserve the stop invocation's BadLog result, return a nonzero exit,
and retain the fact that hooks were removed as separate diagnostic state. A
report snapshot must never downgrade the immediate Retained or BadLog return.
Add a controller-level regression using the existing flush-failure seam.

### P2: report failure can still produce a successful command

Locations: `tools/pkt3663/live_control.cpp:1107-1116` for start,
`:976-989` for stop, and `:1035-1043` for mark.

The command branches reject report timeout, but not all other report failures.
If report allocation, export execution, or the remote report read fails after
Started/Removed/Idle, start and stop can return exit 0. Mark can additionally
replace the report failure note with `marked`. The result claims completion
without the required report, and its default counters and `retained=0` can look
like observations even though no report was collected.

Reproduced using an isolated test DLL with the allowed fixture file name in
an owned dummy host. Its arm export returns Started, its stop export returns
Idle, and its report export returns Refused. No game code or packet function
is involved. The unchanged controller printed:

```text
ctl_action=start
ctl_start=0
ctl_raw_mode=-1
ctl_note=report-unreadable
ctl_exit=0

ctl_action=stop
ctl_stop=5
ctl_raw_mode=-1
ctl_note=report-unreadable
ctl_exit=0
```

Expected: every failed required report returns nonzero, preserves its failure
note and uncertainty, and leaves module ownership conservative. Do not unload
on report loss. Preserve a known Retained return as exit 3. Add controller
regressions for report returning an error, an unreadable report, and timeout;
verify start, stop, and mark each propagate the failure.

## Independent validation

- Rebuilt `cdmp_tests_pkt3663_live_ctl`, `cdmp_tests_pkt3663_live`, and
  `cdmp_tests_pkt3663_live_host` with Visual Studio 18 2026 / MSVC
  19.52.37029.0, x64 RelWithDebInfo, in sibling `NativeServer-build`.
- Focused CTest selection `^pkt3663_(live_ctl|live|live_host)$`: 3/3 passed
  in 7.59 seconds.
- Full CTest suite: 100/100 passed in 64.40 seconds. The passing suite does
  not cover the two reproduced failures above.
- Reviewer reproductions were built separately in sibling `Review3663-build`
  from `Review3663`; no production source was patched for the reproductions.
- The diff leaves `tools/pkt3576`, production offsets, and ASI guards unchanged.
  New production load/start require the strict 3663 identity; fixture tests
  remain distinct. No payload layout is established by this review.
- `scripts/build.ps1` was not rerun because it would configure another build
  tree. This review used the existing sibling MSVC build and CTest instead.

## Task for Cursor / Grok

Read `AGENTS.md`, `CLAUDE.md`, the controller contract, and this review.
Fix both findings without weakening identity gates, adding production fixture
overrides, changing old-build behavior, or altering hook ownership. Preserve
command return values separately from report state and fail on missing required
reports. Add focused regressions and rerun the full suite. Keep the game closed.
Leave implementation changes local and provide the diff and test results for
Codex review before publication or any live test.
