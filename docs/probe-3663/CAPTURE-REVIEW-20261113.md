# Codex review: first 3663 passive capture

Date: 2026-11-13. Reviewed `CAPTURE-20261113.md`, raw commands, session
text, stand records, archived copies, and the saved offline verifier output.
The game was closed during this review. No game process was opened, no code
was changed, and no capture or save file was overwritten.

## Disposition

Useful diagnostic evidence from an interrupted attempt; the controlled
stand/walk/turn milestone is incomplete. The start command timed out and
returned exit 5. Walk and turn files are absent. Later readiness and clean
stop do not retroactively make that failed start an accepted session.
The stored records support live reachability of the diagnostic's send and
receive hooks and recorded cleanup, not movement layouts or working co-op.

## Independently checked evidence

- Initial status recorded `identity=match`, `loaded=0`, `modified=0`.
  Load and mark succeeded. Start recorded `remote-timeout`, `ctl_start=-1`,
  `ctl_exit=5`; no start return value was collected.
- The final session contains matching 3663 identity, `fixture=0`,
  `game_match=1`, full identity checks, `recording_ready=1`, raw mode,
  empty allow-list, and `adopted=0`. It records `shutdown=removed`,
  `retained=0` in the controller result, zero dropped/unread/truncated,
  no session-history truncation, and no session-write failure.
- Stop recorded Removed, process alive, and module still mapped. The module
  staying mapped is expected after a pin. The recorded cleanup supports
  hook removal; it does not prove anything about later gameplay.
- Parsed all 256 data rows. Every copied byte string has the declared copied
  length, every copied length equals claimed length, and each `raw_word0`
  matches the little-endian first two copied bytes. All rows are stand-phase
  complete records. There were no malformed rows.
- Four receive records precede `capture_ready_qpc=1318049551595` and are
  excluded from evidence by the existing cutoff rule.
- The remaining 252 records are eligible by that cutoff alone: 249 send
  records of 181 bytes (`raw_word0=08F8`) and three receive records:
  10 bytes (`0AAA`), 25 bytes (`09AA`), and 3532 bytes (`09FE`).
- The send-record QPC span is 3.3148392 seconds at frequency 10000000.
  Stored=256 and omitted=5590 mean this is bounded initial output, not a
  complete record of the arm's traffic or a guaranteed observation interval.
- Current and archived commands, session, and stand files match pairwise.
  SHA-256 values:

```text
commands.txt
3F40FEAF631B5724D2538070995C61310E9719E56131F5031B27C5E3C2698509
pkt3663-live-session.txt
23BCD6DACD757741731B54BE54C98A6A119D2261299CFFD5BE20C21ADD1F2B9B
pkt3663-live-records-stand.txt
214F21A644733E3405A61956652826A6E512D59081077EB05DF33E43D7873F84
```

The saved verifier output agrees on timestamp `0x6AF5D198`, base
`0x140000000`, size `0x144E9000`, verified file candidates, and `adopted=0`.
The save-backup directory contains 23 files totaling 12263762 bytes. No
pre-copy hash manifest or capture-time DLL hash manifest is present in the
provided evidence directories; their original verification and exact binary
provenance cannot be independently reconstructed from this archive alone.
The backups are Git-ignored and are not automatically saved by a source push.

## Findings and limits

1. **Incomplete controlled capture:** `CAPTURE-20261113.md:22` records the
   start timeout; `:10` and `:77` confirm missing walk and turn windows.
   Do not proceed to a movement decoder or actor spawning from this attempt.
2. **P2 report correction:** `CAPTURE-20261113.md:49` and `:57` say the
   receive lengths are only 10 or 3532. Both sets also contain a 25-byte
   `09AA` record. Correct the summary without editing the raw records.
3. The final log cannot establish the exact wall-clock moment at which its
   ready block became durable relative to the controller timeout. Late
   completion is plausible, not a measured cause. `:30-32` should avoid
   asserting that write ordering as an independently proved observation.
4. `anchor-mismatch` is compatible with the installed entry hooks; the
   label alone is not proof of ownership. The runner's stop checks and the
   recorded Removed result are the cleanup evidence in this archive.

No opcode, coordinate, rotation, actor ID, or packet layout is established.
A match with an on-disk descriptor number remains a correlation until the
live entry format and controlled changes support the interpretation.

## Next task for Grok in Cursor: startup timing and capture-window plan

Keep the game closed for this implementation task.

1. Correct the report's receive lengths and distinguish recorded facts from
   the late-completion hypothesis. Preserve raw evidence and its hashes.
2. Add bounded, build-specific timing diagnostics for the controller's
   thread creation/wait and the runner's export entry, initial identity
   verification, Receive/Send/Complete admission, hook creation, writer
   startup, and durable ready block. Record elapsed/QPC values and clear
   stages without claiming a timed-out call was cancelled.
3. Investigate startup cost using owned dummy hosts, synthetic mapped images,
   and offline file-backed benchmarks. Current code performs full image
   verification before arm and again during Receive/Send admission
   (`live_runner.cpp:677`, `:488`, `:499`); these are measurement targets,
   not an established cause. Preserve their checks and ownership guarantees.
4. Add a delayed-start regression, beyond the existing delayed-report tests.
   Check that timeout retains arguments needed by the unfinished call,
   reports uncertainty, does not unload or start another capture, and permits
   eventual safe cleanup. Do not simply raise the 20-second wait without
   timing evidence and an explicit bounded design.
5. Propose a controlled window procedure compatible with the observed output
   cap. The current 256 stored rows filled in roughly 3.3 seconds of send
   observations. Ensure the requested action actually overlaps retained
   post-ready data. Keep the 4096-byte copy cap; any sampling, pre-ready quota,
   duration, or storage-policy change needs its own bounded implementation
   and tests. Do not silently discard or overwrite earlier evidence.
6. Run relevant game-free tests, provide timing results and a focused diff,
   and leave changes local for Codex review before another live attempt.

After review, use a fresh process and new archive for a separately requested
stand/walk/turn run, with a verified save backup and recorded binary hashes.
Require a successful observed start and durable readiness for each accepted
window. Stop on timeout, Retained, or failed evidence/log checks.

Only after successful contrasting captures should an offline analyzer test
candidate frame meanings and movement fields against those observations.
Verified decoding comes before a separate experiment synchronizing a second
visible actor. Production gameplay guards and offsets remain unchanged.
