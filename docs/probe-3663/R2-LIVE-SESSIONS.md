# R2 Step 5 — actual passive sessions and gated live tests

No production tier is armed. None of L1–L4 has passed; no synthetic result
is counted as native two-player evidence. Three user-present passive sessions
ran on exact 3663 (SHA-256 8BE67178D9B9A149C361A731051665E0BA4D441E250AE1DFE645BD6E524345C8).
Each has a prewritten question, backup/installation receipts, logs and hashes
in port-evidence/r2/live/<session>/result.json and accompanying files.

- R2-P1-20261116-072000, HP: both definition registries select key 0; the
  unique native 900000/900000 stat agrees with visible inventory 900/900.
  HP did not vary, and no literal-ID stability is claimed. The session exceeded
  the fifteen-minute sitting limit during delayed exit. Its verified process
  was stopped; normal save progress was preserved and backup verified. This
  deviation led to the independent exact-process fourteen-minute watchdog.
- R2-P1-20261116-075900, loading: movement/rotation captured, but collection
  began after loading because PowerShell JSON DateTime coercion rejected the
  valid watchdog receipt. The user reported successful fast travel, absent
  from the captured pose interval. Timing could not distinguish stale chain
  from out-of-interval travel. The fourteen-minute watchdog closed the test
  process and cleanup restored the installation. The timestamp parser was
  fixed without weakening exact deadline comparison, and regression passed.
- R2-P1-20261116-100800, loading rerun after that fix: collection was verified
  active before Continue. 438 samples include chain_not_ready at start, park
  at 71.344 s, placement at 108.797 s and chain_not_ready at 212.641 s before
  normal exit at 213.657 s. A 182.35 m world jump occurs at 142.438–142.688 s
  in the user-confirmed travel interval. The chain remained coherent across
  it, with no separate park/not-ready interval. This supports the initial park
  observation but does not establish travel loading readiness. All 23 live
  save files are unchanged; backup verified, installation restored, matcher
  resumed, no game process remains. No UI control or screen capture was used.

No game crash occurred in these sessions. Deadline termination in the earlier
loading session is recorded as such, not as a crash. The HP overrun remains
a deviation even though its cleanup succeeded. Physical Escape was the user
operating the game menu; subsequent continuation used no UI control.

Step 5 continuation decision: L1 still requires a complete actual Tier A
reader and a non-mutating loading readiness rule that survives fast travel.
The native reference-acquisition vslot cannot be called as a read-only predicate.
L2 needs A–B; L3 needs A–D; L4 additionally needs the second PC. Their
prerequisites are absent, so no second-PC session was requested. The smallest
next experiment first identifies the native client loading field and its
lifetime/update path offline, then samples it passively across one travel.
Repeating the same pointer-only trace would not settle that missing fact.
