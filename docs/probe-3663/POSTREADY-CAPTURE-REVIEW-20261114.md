# Review of the available post-ready capture

Reviewed 2026-11-14 against checkout `grok/compat-guard`, HEAD `3e950a7`.
This review only read files. No game process was queried, launched, attached,
stopped, or modified. No binaries were executed and no tests were rerun.
There are no source changes in this review.

## Disposition

Package A remains incomplete and Package B movement analysis is blocked.
`docs/probe-3663/HANDOFF-CURRENT.md` is absent from this checkout. A filename
search under both workspace roots and the user profile found no copy.

The new directory `backups/pkt3663-live-20261114-postready` now contains
preparation, verifier output, commands, a standing records file, a numbered
session file, and its latest-session mirror. No walk or turn records, later
attempt sessions, or analysis output are present in that directory.

## Findings

1. **No retained post-ready evidence.**
   `pkt3663-live-records-stand.txt:12` reports stored=0, omitted=0,
   dropped=0, unread=0, truncated=0, pre_ready=88. The file contains a header
   and counters only. `pkt3663-live-session-1.txt:77` repeats these final
   counts. Pre-ready traffic was excluded, but there are no retained bytes
   from which to infer a movement field or layout. Missing walk/turn files
   also prevent a controlled comparison.

2. **No deliberate observation interval after the start command returned.**
   `commands.txt:135` records start returning at 11:29:34.655; line 168
   records stop beginning at 11:29:34.679, approximately 24 ms later.
   This is the gap between controller commands, not the exact period during
   which admission was open: readiness occurred inside start before its
   return. The archive does not provide an exact ready-to-stop duration.
   A further experiment should explicitly log a bounded post-success dwell
   with the game in the foreground and the requested character action.
   Empty output alone does not establish why no post-ready traffic arrived.

3. **The promised milestone handoff is missing.**
   Write `HANDOFF-CURRENT.md` with this attempt marked insufficient and the
   exact remaining experiment. Do not describe Packages A/B as complete.

## Positive evidence and limits

The numbered session identifies non-fixture build 3663, timestamp
`6AF5D198`, base `140000000`, size `144E9000`, and successful structural
checks (lines 11-27). Durable readiness is recorded at 18.384489 seconds
(line 55); final shutdown is removed (line 76), with no history truncation
or session-write failure (lines 99-100). Both session copies hash equally.
The controller records target_alive=1 and retained=0 after stop
(`commands.txt:148`, line 160). These are archived observations, not a
claim about the process's current state.

The stop's anchor-mismatch with gate=installed is documented behavior for
the already loaded diagnostic's hook-modified entry bytes; see
`LIVE-CONTROLLER-3663.md:64`. It is not evidence that a wrong game identity
was accepted at load/start. No field adoption is justified by this archive.

## Next work for Grok

Preserve these files and write the missing handoff honestly. Plan one
bounded coverage experiment using the unchanged reviewed diagnostic:
log start success, then a clearly timed observation period (for example
15 seconds after success) with the game foreground and the specified
action, followed by the reviewed stop and complete archival. Retain all
existing version, ownership, logging, and timeout rules. No automatic retry
of a failed or uncertain session is authorized by this recommendation.

Audit each window before claiming it usable. Continue stand/walk/body-turn
comparison and offline analysis only when the required evidence exists.
If the planned interval remains empty, report that blocker and assess
offline whether these call sites can observe local movement; propose a
focused diagnostic change or a version-isolated read-only player-state
investigation for review rather than repeating blind captures. Keep
layouts, production offsets/guards, and actor spawning unchanged.

## SHA-256 of reviewed local evidence

| File in the postready directory | SHA-256 |
| --- | --- |
| commands.txt | 646F165AAA2852810E757E80AD112A2AD53D90CFEC9FB01B275A454B8E307B45 |
| pkt3663-live-records-stand.txt | 1F0FD3ECA497E4BDAE06BD31B5C998E4C6D1661D905B7C94CE4BFEF5D923B799 |
| pkt3663-live-session-1.txt | 7CA03FBC49EBFC42E373482C457DB49BDB9C60985D95D349E8D839239F6D7B6C |
| pkt3663-live-session.txt | 7CA03FBC49EBFC42E373482C457DB49BDB9C60985D95D349E8D839239F6D7B6C |
| prep.txt | 64F06B1A0A60C5A10BCB7558472DADE1CF6605E88D1CEC84894CACA77E55F42B |
| verifier.txt | 3C2CC3F401F87DB1DF4590F402DC4ED91E7243C66794B0C7B6846B1B203906EC |
