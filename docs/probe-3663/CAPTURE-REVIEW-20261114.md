# Codex evidence review: instrumented 3663 windows

Date: 2026-11-14. Source checkpoint `b89a733`. Reviewed
`CAPTURE-20261114.md`, prep/verifier output, commands, all three raw record
files, and the three archived session files. No game process was opened,
launched, stopped, or modified by this review. No runtime source was changed.

## Result

The startup/cleanup diagnostic milestone is supported. The movement-evidence
milestone is not: all 768 stored records are before their window's readiness
cutoff, and none is a send record. These files cannot establish a movement
layout. The next implementation should admit records only after durable
readiness and preserve complete bounded evidence for each attempt.

## Verified observations

- All three starts returned Started inside the existing 20-second remote
  wait: stand 18437 ms, walk 18486 ms, turn 19053 ms. No lost start return or
  timeout occurred in these windows.
- Every stop returned Removed, retained=0, process alive, and module mapped.
  Session stop blocks agree. The pin remaining mapped is expected; the
  installed identity label alone is not proof of ownership.
- Stand and walk archived sessions preserve the identity, readiness and
  stage lines without history truncation or session-write failure. Each
  records a matching non-fixture 3663 identity and raw mode, no allow-list,
  and adopted=0.
- The final/turn session has `session_history_truncated=1`. It preserves
  the final stop block but has lost turn startup stage lines. It is not a
  complete accepted history for that window. The earlier immutable stand
  and walk copies remain separately inspectable.
- The records have consistent copied byte lengths, little-endian raw first
  words, phases, and complete/truncated status. There were no parse or
  byte-count errors. Truncated rows were explicitly capped at 4096 bytes.

| Window | Stored rows | Post-ready rows | Send rows | Stored truncated rows | Last stored row before ready |
| --- | ---: | ---: | ---: | ---: | ---: |
| stand | 256 | 0 | 0 | 0 | 4.4266688 s |
| walk | 256 | 0 | 0 | 10 | 5.0256790 s |
| turn | 256 | 0 | 0 | 14 | 3.6255769 s |

These comparisons use each archived session's last `capture_ready_qpc` and
frequency 10000000. Stored spans are 1.6465473, 1.1986828, and 3.0761263
seconds. Stand and walk stored rows also end before their send-hook stage.
The trailers agree with the report: dropped 341/1336/365, truncated
3/61/27, stored 256 each, and omitted 276/531/502. Trailer counters include
observations that were not stored; they are not counts of just file rows.

The source explains the capacity loss: `capture.cpp:491-503` accepts entry
observations during installation, `:506-515` spends the fixed storage budget
on the first popped rows without a readiness filter, and the writer starts
before the runner establishes durable readiness. Receive is installed
roughly six seconds before Send, giving startup receive traffic time to fill
the queue. Filtering only during later analysis cannot recover omitted data.

## Timing conclusion

Stand and walk durable-ready marks occur 18.417461 and 18.466823 seconds
after their export-entry marks. The initial identity interval is about 6.064
seconds, followed by approximately 6.16-6.24 and 6.08-6.22 seconds around the
Receive and Send admission checks. Hook-install stage gaps are under 4 ms.

This locates the dominant startup cost in the repeated-verification intervals
in these two runs, not the hook-install intervals. Those gaps also include
intervening timing I/O and small surrounding work; they do not separately
measure VirtualQuery, page faults, or individual scanner operations. This
does not reconstruct the earlier timeout. Starts are still close to the
20-second boundary; do not remove verification or assume timeout cannot recur.

## Integrity and provenance

All seven hashes printed in the capture report match the files inspected.
Each record file matches its archived copy. The final session matches the
turn session copy; earlier session/command archives are legitimate earlier
snapshots, not identical final files.

The present DLL, controller, and scanner hashes exactly match prep.txt and
the report. The save-backup directory contains the recorded 23 files and
12251985 bytes. `save_hash_mismatches=0` is recorded by prep; no separate
per-file pre-copy manifest was supplied to independently replay that check.
Raw captures and saves remain under Git-ignored backups.

## Next task for Grok in Cursor

Keep the game closed for this focused implementation; leave the current
archives intact.

1. Design and implement an explicit post-ready capture admission boundary.
   Until the runner has completed identity/ownership checks and successfully
   persisted readiness, callbacks must not consume the accepted queue or
   the 256-record output budget. Keep pre-ready counters distinct if needed.
   Use a synchronized publication boundary and a defensive writer-side
   check so racing callbacks/queued rows cannot enter the accepted window.
2. Failed/timeout startup must remain uncertain or refused under the existing
   cleanup rules. A failed ready write must not enable record admission.
   Stop must close admission safely and preserve callback lifetime. Preserve
   the recording cutoff, explicit phases, raw semantics, 4096-byte copy cap,
   empty allow-list, exact identity checks, and Retained behavior.
3. Preserve complete session evidence for each window within a bounded
   storage design. Prefer separately named, exclusive-create attempt/session
   files or another explicit bounded archival policy. Three attempts must
   not silently erase earlier timing/readiness evidence. Keep invalid-history
   and write-failure handling; do not simply grow an unbounded lifetime log.
4. Add game-free regressions that flood Receive before readiness, submit
   mixed post-ready Send/Receive, and verify zero pre-ready rows consume
   accepted capacity. Cover records queued across the publication boundary,
   exact cutoff behavior, concurrent stop, failed ready logging, repeated
   starts, and enough sequential attempts to reproduce the former history
   truncation. Prove all per-attempt files remain intact and bounded.
5. Run the affected tests and full suite. Supply a Codex handoff with the
   admission/state ordering, output accounting, log retention behavior, and
   test results. Do not rerun the game until this change is reviewed.

After review, collect a fresh stand/walk/turn set whose retained rows actually
meet the readiness rule. Only then compare packet shapes and test candidate
movement fields offline. A second visible actor remains a later experiment;
production offsets and gameplay guards stay unchanged.
