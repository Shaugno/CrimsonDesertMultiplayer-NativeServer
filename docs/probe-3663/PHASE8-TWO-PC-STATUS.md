# Phase 8 — blocked; no Steam two-PC test

Host image re-hashed: SHA-256
8BE67178D9B9A149C361A731051665E0BA4D441E250AE1DFE645BD6E524345C8,
timestamp 0x6AF5D198, SizeOfImage 0x144E9000. No host build drift.
Raw facts: `port-evidence/phase8-preflight-identity.json`.
No guest image identity is available because no guest session was requested.

Prerequisites remain the blocked native Phase 6–7 behavior and PORT-MATCHES.md
core rows. No second-PC touchpoint was requested, no guest deployment kit was
issued, no Steam tunnel was exercised, and no guest logs were returned. Local
build outputs are development artifacts only, not a functioning 3663 kit.
The shared-folder delivery/return step therefore did not occur.

Prepared session after native validation succeeds:

- Question: does the same join/walk/turn/disconnect/removal/reconnect acceptance
  pass over the real Steam tunnel, with one load/fast-travel on each side?
- Expected observation: distinct legitimate accounts, matching game/mod build
  hashes, friend-restricted connection, both native characters moving, clean
  removal/rejoin, and separately protected host/guest saves.
- Time limit: 20 minutes after both sides are ready.
- Stop condition: any identity/mod mismatch, crash, loss of save isolation,
  inconsistent actor cleanup, or backup-manifest verification failure.
- Before the single second-PC touchpoint: complete a local guest kit with exact
  manifest/ini hashes and minimal features only; use docs/PRUEBA-DOS-PC.md for
  transport/log delivery while excluding its historical combat/mount extras.
  Re-hash both installations, back up/hash both saves, state the question,
  deploy through the documented scripts, and remove after the session.
  Preserve logs/capture hashes without committing saves or game binaries.

Status: blocked, not passed. There is no two-player Steam evidence in this run.
