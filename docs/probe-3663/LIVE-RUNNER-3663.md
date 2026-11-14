# 3663 passive raw-capture runner

Date: 2026-11-13. This document describes the isolated diagnostic runner and
its game-free test boundary. It is not a live-game result and does not
authorize loading the DLL into Crimson Desert.

## Purpose and isolation

`cdmp_pkt3663_live.dll` is a separate, passive diagnostic for exactly Crimson
Desert 1.0.0.3663. It is not `cdmp.asi`, is not linked into the production mod,
and does not open any production compatibility guard. The 2976 and 3576 paths
are unchanged.

The production export accepts only a log directory. It accepts no PID, RVA,
force flag, identity override, fixture address, replay input, or packet data.
The send and receive candidates are derived internally only after the loaded
main image passes the strict 3663 verifier:

- mapped base `0x140000000`;
- timestamp `0x6AF5D198`;
- `SizeOfImage` `0x144E9000`;
- valid executable sections and exception-function ranges;
- the exact send, receive, and pump anchors; and
- the two decoded pump calls to the expected roles.

Old 2976 and 3576 identities fail before a candidate address is used. The
fixture DLL is a different binary and is always reported as `fixture=1` and
`game_match=0`; only that DLL exposes dummy-host entry points.

## Three-stage install gate

Hook admission is deliberately split into three synchronous checks:

1. `Receive` proves the pristine mapped image immediately before the receive
   hook factory runs.
2. `Send` runs after Receive is installed. It accepts a mapped-image read only
   when the bytes overlapping Receive still equal the exact patch owned by
   this capture, then substitutes SafetyHook's saved original bytes for that
   span before applying the full verifier again.
3. `Complete` runs after Send is installed and compares SafetyHook's saved
   instruction-aligned Send bytes with the verified 3663 entry.

Receive ownership is re-proved after the `Send` callback even when that
callback returns false. After `Complete`, both installed slots are re-proved.
A failed check rolls back only while exact ownership can still be shown.
Otherwise the result is `Retained`.

The generic capture backend does not contain the 3663 entry signatures. The
production runner supplies those comparisons; the dummy-host fixture can use
synthetic functions without claiming they are game entries.

## Exact hook ownership

SafetyHook v0.7.0 overwrites a whole, instruction-aligned span. The backend
therefore records and compares the complete span returned by
`MidHook::original_bytes()` rather than a five- or six-byte prefix. It also
records the exact installed route:

- target patch form (`E9` or `FF 25`);
- relay and indirect-slot addresses;
- MidHook stub address;
- callback address;
- continuation/trampoline address; and
- the allocation regions used only for thread-quiescence checks.

The live target bytes and every route component must still match before a
normal stop disables either hook. If another owner changes a patch or route,
the backend does not guess, restore foreign bytes, or unload callback code. It
keeps the module pinned, returns `Retained`, and refuses another start.

The MidHook tail offsets used to recover the callback and continuation depend
on the repository's pinned SafetyHook v0.7.0. Updating SafetyHook requires a
fresh ownership audit; it is not a routine dependency bump.

## What is recorded

The capture is `raw-all`: `allowlist_count=0`, `classification=unclassified`,
`layout=unresolved`, and `adopted=0`. It never calls either packet function,
injects or replays a packet, or writes through a captured packet pointer.

Each record contains the entry registers, measured stack values, QPC time,
claimed and copied lengths, a bounded byte copy, direction, phase, and raw
status. The first copied word is `raw_word0`, not an opcode. Receive records
remain marked `receive_pre_transform_possible=1`. The copy cap is 4096 bytes;
larger claimed frames are explicitly `truncated` rather than silently treated
as complete. Queue overflow is counted as `dropped`.

Production uses a 256-record queue and writes at most 256 records. Stand,
walk, and turn use separate exclusive-create files so an earlier labeled
capture is not overwritten:

- `pkt3663-live-records-stand.txt`;
- `pkt3663-live-records-walk.txt`;
- `pkt3663-live-records-turn.txt`.

Each start exclusive-creates `pkt3663-live-session-<n>.txt`. The legacy
`pkt3663-live-session.txt` mirrors the latest flush and is not the only copy.
Stored, omitted, pre-ready, dropped, unread, and truncated counters are reset
for every start attempt;
`capture_attempts` is intentionally cumulative for the module lifetime. An
active report refreshes the live counters. A successful start writes
`recording_ready=1` and a `capture_ready_qpc`; only records whose QPC is at or
after that cutoff are evidence. Pre-start callback records are not evidence.

The ready block must reach the session file before start returns success. If
that flush fails, the runner removes the newly installed hooks while ownership
is still proved and returns `BadLog`; an uncertain removal returns `Retained`.
An idle phase-mark flush failure also returns `BadLog` and restores the prior
phase. `session_write_failed=1` or `mark_log_write_failed=1` invalidates the
session even if a retry later makes the marker durable.

Session logging is bounded. If its history cap is reached, the runner does
not silently leave a plausible partial history: it resets the bounded history
with `session_history_truncated=1`. Treat that marker as a failed evidence
session and preserve the record file separately before retrying.

## Game-free validation boundary

The dedicated offline tests cover:

- raw observer bounds, concurrency, unreadable input, truncation, and empty
  allow-list behavior;
- receive-first installation, all three admission stages, rollback, retained
  ownership, full-span patch forms, and ownership loss during start or stop;
- strict mapped-image identity, old-build rejection, mutated evidence,
  malformed PE metadata, and exact patch normalization; and
- production/fixture export separation, dummy-host start/stop/report, phase
  files, attempt-counter reset, recording-ready cutoff, and bounded-log marker.

These tests use owned dummy functions and synthetic mapped images. They do not
open, launch, attach to, or modify the game, its loader, or its saves.

Independent result on 2026-11-13: the complete RelWithDebInfo build succeeded
with MSVC 19.52.37029.0. All 99 CTest tests passed in 59.65 seconds. The four
dedicated `pkt3663_observer`, `pkt3663_capture`, `pkt3663_live`, and
`pkt3663_live_host` tests passed again after the final ownership and log-fault
fixes. The game stayed closed. The ASI was not installed or run.

External control of that runner is a separate local tool, pending review:
`docs/probe-3663/LIVE-CONTROLLER-3663.md`. The controller reuses this runner's
identity check and log-directory rule. It does not change hook ownership,
candidate addresses, or the recording-ready cutoff.

## Remaining safety boundary

Passing the offline suite makes the runner ready for a separately approved,
controlled passive test once the controller review accepts the operator path.
It does not prove that the candidates are reached in the live 3663 process. It also does not prove an opcode, packet role at the
wire boundary, position, rotation, actor identity, or any payload layout.

A future live test must keep `cdmp.asi` and `winmm.dll` absent, preserve saves
first, use separate stand/walk/turn windows, stop immediately on `Retained`,
and archive the raw files before interpretation. Injection, replay, spawning,
offset adoption, and opening the production guard remain out of scope.
