# Grok handoff: reviewed 3663 checkpoint

Date: 2026-11-13. Project: Shaugno/CrimsonDesertMultiplayer-NativeServer.
Branch: `grok/compat-guard`. Codex hands implementation back to Grok; Codex
reviews the resulting diff before another implementation checkpoint is published.

## Saved state

- `afbbd9d`: aborted 3576 capture and offline 3663 assessment.
- `bdf30f9`: verified 3663 packet entry candidates offline.
- `4334e43`: separate fail-closed 3663 passive capture runner and tests.
- This handoff commit records the next task; it changes no runtime code.

The working copy was recovered outside OneDrive after the original checkout
became incomplete. Work in the repository selected by the dedicated
`Start-Grok-NativeServer.ps1` launcher. Do not resume an old session against the
incomplete OneDrive checkout, copy its files over this checkout, or restore an
old session snapshot. The build already validated here is the sibling
`../NativeServer-build`, configured with Visual Studio 18 2026, x64,
RelWithDebInfo. A fresh checkout can instead use `scripts/build.ps1`.

Read `AGENTS.md`, `CLAUDE.md`, `docs/ROADMAP.md`,
`docs/probe-3663/PLAN-3663-READY.md`,
`docs/probe-3663/ENTRY-VERIFIER-3663.md`, and
`docs/probe-3663/LIVE-RUNNER-3663.md` before implementation.

## What is verified

The installed build assessed was 1.0.0.3663: timestamp `0x6AF5D198`,
mapped base `0x140000000`, SizeOfImage `0x144E9000`. The isolated diagnostic
also checks sections, exception ranges, entry signatures, and pump calls.
Old identities are refused before using candidate addresses. File identity
alone is insufficient to admit the loaded image.

The runner uses three installation admission stages, complete patch-span and
route ownership checks, conservative retained teardown, bounded raw records,
and a durable recording-ready cutoff. Production and fixture DLLs are separate.
`raw_word0` has no proved opcode meaning; packet roles, layouts, coordinates,
actor identity, and live reachability remain unresolved. Addresses remain
experimental, and the ASI gameplay guard remains closed for this build.

Validation: the full RelWithDebInfo build succeeded with MSVC 19.52.37029.0.
The final full CTest suite passed 99/99 in 59.65 seconds. Three independent
reviews checked hook ownership, runner/log lifetime, and version/fixture
isolation. Their correctness findings were fixed and re-reviewed. One
non-blocking coverage limitation remains: Capture tests accept whichever E9
or FF25 hook form SafetyHook produces instead of forcing direct FF25 routing;
synthetic FF25 identity normalization is covered. No corresponding defect
was found. The game stayed closed; no live capture was performed.

## Next task: a separate 3663 controller, tested without the game

The runner DLL exists, but `tools/pkt3663` has no external controller yet.
Build a version-checked controller for this diagnostic before proposing a live
session. Inspect `tools/pkt3576/live_ctl.cpp` and `live_control.*` for lessons,
but preserve that controller and its 3576 acceptance unchanged.

1. Document the interface, state transitions, failure/exit codes, and expected
   behavior before coding. Support explicit-PID status and the minimum
   load/mark/start/report/stop/unload workflow required by
   `tools/pkt3663/live_api.h`. Reconcile the workflow with the existing API;
   do not invent a packet-call ABI or require a production API override.
2. Prove target executable and loaded 3663 identity before loading the
   production DLL or invoking its exports. Wrong builds, malformed images,
   changed signatures/topology, and mismatched process/module identity must
   refuse before mutation. Keep read-only status available before load.
   Retain the runner's independent in-process verification.
3. Require explicit paths and PID; do not pick a process arbitrarily. Keep
   production and dummy-host fixture paths distinct. Do not add force flags,
   caller-supplied RVAs, broader version gates, or fixture seams to production.
4. Bound remote operations and buffers. Handle process exit, timeout,
   partial failure, and report errors without freeing memory still used by a
   remote operation or unloading a retained module. Preserve diagnostics
   needed to explain refusal and uncertain cleanup.
5. Add game-free regressions using owned dummy hosts and fixtures. Cover
   old/wrong identities refusing before mutation, positive fixture lifecycle,
   production/fixture separation, phase handling, process exit, and uncertain
   cleanup. Capture the observed results; do not describe fixtures as live
   game evidence. Run relevant tests and the complete CTest suite after changes.
6. Finish with a Codex review handoff: changed files, commands/results,
   remaining limitations, and a proposed manual capture checklist. Leave
   implementation changes local for review.

Keep Crimson Desert closed for this task. Do not attach to it, install
`cdmp.asi`/`winmm.dll`, deploy a diagnostic into it, or run `launch-2inst.ps1`.
Do not modify production offsets/guards, adopt candidate addresses, replay
packets, inject game messages, spawn actors, or infer payload layouts.

After controller review, the following milestone is a separately requested
live session: back up saves, recheck the current executable identity, have the
user launch through Steam, record stand/walk/turn separately, and preserve raw
records before interpretation. `Retained`, failed log durability, history
truncation, identity mismatch, or missing readiness evidence stops acceptance
of that session. Working co-op has not yet been demonstrated.
