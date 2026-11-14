# Cursor implements; Codex performs final review

User direction, 2026-11-14: Cursor/Grok owns implementation from start to
finish; Codex is the final reviewer. Stop requiring the user to shuttle each
small fix or research step between assistants. This changes review cadence;
it does not establish compatibility or remove the need for evidence.

## Target

Complete the current milestone: two legitimate installations on the verified
target build see each other walking and turning, with explicit player
identity, handling of loads/reconnects, and clean remote-actor removal.
Recorded target: internal executable 1.0.0.3663. Recheck the installed file
before runtime work. A Steam update is a real compatibility blocker.

This milestone is a co-op prototype. Shared combat, quests, mounts, and
progress/save synchronization are later scoped work; do not call the whole
mod finished when only the walk-around is complete.

## Ownership and progress

- Work in the recovered NativeServer-work2 checkout opened in Cursor,
  outside OneDrive, on the existing task branch. Inspect HEAD, status,
  remotes, and local work first. Published base is `502577a`; newer local
  implementation and review files must be preserved.
- Cursor/Grok performs research, implementation, self-review, regression
  fixes, integration, and validation. Continue through normal test/fix loops
  without asking for a Codex review after every step.
- Treat existing review findings as implementation input. Self-review the
  fixes and continue to the next dependency when checks pass. A failing test
  or unknown layout is not resolved by weakening its checks or renaming it.
- Codex reviews the completed milestone once, including source and raw test
  evidence. The user may still request an earlier review voluntarily.
- This document supersedes older procedural requirements to obtain a Codex
  milestone review before each intermediate implementation or diagnostic
  change. Actual version, ownership, backup, and evidence requirements remain.
- Keep local progress notes current. Ask the user only for required in-game
  actions, unavailable credentials/access, a missing second legitimate game
  installation, a scope decision, or a concrete blocker that independent work
  cannot resolve. Do not claim there is an automatic connection to Codex.

## Immediate work already identified

Read `docs/probe-3663/ACK-HANDLER-REVIEW-20261114.md`. Resolve its three
findings in one package:

1. The movement decoder's fixed offsets skip an intervening reader helper.
   Trace that helper and all relevant cursor consumption before defining a
   layout. Top-level read sizes alone do not prove adjacent fields.
2. ActorLoad slot 2 has a runtime-derived start and nested dispatch; a flat
   sequence of u16s is unproved. Trace the calculation and callee.
3. Correct the inclusive header boundary and its tests according to the
   comparison/branch. Keep handler-specific differences explicit.

Then continue through demonstrated movement/actor semantics, the chosen
state and transport path, a real version-isolated actor lifecycle, and the
two-PC prototype. Use the existing code and evidence where applicable.
Avoid adding disconnected scaffolds as a substitute for resolving the next
actual dependency. An unresolved result is acceptable when the exact blocker
and attempted evidence are documented.

## Validation and runtime work

Start with file-only analysis and meaningful game-free tests. Read AGENTS.md,
CLAUDE.md, and relevant research. Preserve GPL and original attribution.
Run focused regressions during work and the required full suite at a code
milestone with the game closed. If it is running, do independent work and
coordinate closure with the user; do not terminate it to make tests pass.

For a live phase, prepare the concrete experiment, tested binaries, exact
identity checks, output directory, and acceptance/stop criteria first.
Coordinate Steam launch and character actions with the user. Back up saves
and verify their manifest before memory-writing tests. Preserve raw evidence
and hashes, including failures. Do not repeat an empty experiment unchanged.
No game launch or runtime modification is being performed by the creation
of this handoff.

Progress runtime implementation only from demonstrated facts. Add a separate
3663 gameplay path when evidence supports it; do not force old code to run
by broadening a timestamp check. Keep old-build paths isolated. Never invent
offsets, infer readiness from coherence alone, or assume an old payload
layout. Use stable peer/actor identities rather than cross-PC pointers.
Keep Steam-friends restrictions and executable filters intact.

Use guarded memory operations and bounded payloads, queues, session state,
and lifetimes. Test loading, actor replacement, stale traffic, disconnect,
and cleanup. Preserve the controller's timeout and uncertain-ownership rules:
a timeout is not cancellation; do not force unload or retry an uncertain
session. Stop and investigate a concrete failure before another live attempt.
Protocol wire changes require the appropriate version change.

Implementation remains local pending final review and the user's publish
request. Do not publish an experimental gameplay release as part of this
workflow change. Code/docs publication and raw saves/capture backups are
different: do not publish save data or credentials.

## Final review handoff

Maintain `docs/probe-3663/HANDOFF-CURRENT.md`. At completion include:

- exact source commit and remaining diff, build configuration and binary hashes;
- evidence provenance for every adopted layout, address, and lifecycle operation;
- commands and actual test results, including failures, fixes, and reruns;
- a reproducible demonstration of both players seeing movement and turning;
- loading, reconnect/disconnect, actor-removal results and raw archive hashes;
- known limitations and the exact scope achieved.

If a real blocker prevents completion, write a blocker handoff with the
specific missing fact/access/action, independent work completed, and the
smallest experiment that would resolve it. Do not declare success to trigger
the final review. A simulated feed or local echo alone is not two-player
runtime evidence.

## Prompt for Cursor/Grok

> Follow docs/CURSOR-END-TO-END-HANDOFF.md. Own implementation, research,
> self-review, testing, and fixes through the current two-player walk-around
> milestone. Resolve the existing review findings and continue without
> sending every small change back to Codex. Ask me only for required game
> actions or concrete blockers. Keep evidence, version isolation, save
> backups, and cleanup rules intact. Maintain HANDOFF-CURRENT.md and prepare
> one complete final review package for Codex when the milestone is actually
> demonstrated, or an honest blocker handoff if independent progress stops.
