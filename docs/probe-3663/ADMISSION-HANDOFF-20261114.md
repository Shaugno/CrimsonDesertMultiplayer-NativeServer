# Handoff: post-ready admission and per-attempt session files

Date: 2026-11-14. Local changes on `grok/compat-guard` after `b89a733`.
The 2026-11-14 capture archives were not modified. No new game session was
started. The leftover capture process, PID 33020, was closed so the suite
could link the diagnostic DLL and so the controller tests would not refuse
to run.

## Admission order

1. `Capture::start` installs both hooks and starts the writer with
   `ready_qpc == 0`. Callbacks that arrive in that state increment
   `pre_ready` and return before `Observer::observe`, so they do not take a
   ring slot, a copy, or an output row.
2. The runner writes the pending ready block and the `ready-durable` stage
   into that attempt's session file. Only after both writes succeed does
   `publish_ready` release-store the cutoff. A zero QPC, a second publish,
   or a failed ready write does not publish. The existing BadLog/Retained
   cleanup still removes the hooks or reports Retained.
3. A callback may enter the queue only after it observes a nonzero cutoff.
   The writer drops a popped row whose QPC is below that cutoff and counts
   it as `pre_ready`, not as `omitted`. A row with `qpc == cutoff` is
   accepted. `omitted` still means the 256-record budget was already full.
4. `stop` stores `admission_closed` before it disables hooks. Queued rows
   already past the cutoff are drained. The writer join and the existing
   admission gate still bound callback lifetime. `stop` does not publish
   readiness.

Identity checks, hook-ownership checks, the 4096-byte copy cap, the empty
allow-list, raw mode, phase files, and Retained behavior are unchanged.

## Session retention

Each start exclusive-creates `pkt3663-live-session-<n>.txt` and resets the
8192-byte buffer. The file is sealed when that start does not remain active,
or when `stop` writes its final block. Later attempts and idle marks do not
reopen it. `pkt3663-live-session.txt` mirrors the latest flush.
`pkt3663-live-session-pending.txt` receives idle marks after a seal. Those
marks still truncate explicitly at the buffer cap. The newest mark line is
copied into the next attempt so the phase selection remains with that
attempt. A failed ready write still leaves `session_write_failed=1` and does
not publish admission.

## Tests

Game-free coverage:

- 40 receive calls before publication leave `stored=0`, `omitted=0`, and
  `dropped=0`, with `pre_ready=40`.
- A queued row below the cutoff is not stored. A row at the cutoff and later
  sends share the 4-record budget; `omitted` counts only the overflow.
- A receive flood concurrent with `stop` before publication stores nothing
  and still removes the hooks.
- The host test keeps session 1 byte-identical across a duplicate start,
  300 truncating marks, a walk attempt, an unlabeled attempt, and a failed
  ready write. Sessions 3 and 4 stay intact as well.
- The ready-flush fault and the ready short write still return BadLog with
  hooks removed.

`ctest --test-dir NativeServer-build -C RelWithDebInfo --output-on-failure`
passed 100/100 in 92.26 seconds. `pkt3663_capture` passed in 0.60 seconds,
`pkt3663_live_host` in 4.31 seconds, and `pkt3663_live_ctl` in 48.08 seconds.
Crimson Desert was not running. This does not collect a new stand, walk, or
turn window.
