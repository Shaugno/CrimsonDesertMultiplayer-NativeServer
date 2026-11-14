# Phase 6 — blocked; no live session started

Preflight file identity: 1.0.0.3663, timestamp 0x6AF5D198, image size
0x144E9000, SHA-256 8BE67178D9B9A149C361A731051665E0BA4D441E250AE1DFE645BD6E524345C8.
No build drift. Raw file facts: `port-evidence/phase6-preflight-identity.json`.
An actual host process listing found zero CrimsonDesert processes.

Blocking prerequisite: a complete, independently verified 3663 player chain
and native minimal-path table do not exist. The startup decision intentionally
grants zero gameplay reads/hooks/networking for this recognized build. The
Phase 5 passing suite and synthetic loopback do not resolve that prerequisite.

No live test was requested from the user, launched, attached, deployed or run.
No live logs/capture or save-backup manifest exists for this phase, because no
session began. Saves were not accessed. Deployment/removal commands were not
run. This is a blocked-phase record, not a successful Phase 6 result.

Prepared first session, only after the offline prerequisites are satisfied:

- Question: does the guarded 3663 read-only player source follow the loaded
  character's position, rotation and HP through a walk, turn and fast travel?
- Expected observation: valid typed hops and finite samples track those actions;
  loading produces an explicit invalid/unready sample rather than a fabricated
  pose. Native writes/hooks stay disabled for this read-only question.
- Time limit: 10 minutes after the save is loaded.
- Stop condition: any identity mismatch, unreadable/ambiguous chain, crash,
  nonfinite data, unexpected native write/hook, or backup verification failure.
- Before launch: re-hash the installed exe again; hash-manifest the timestamped
  save backup, record question/time/identity, then deploy with scripts/deploy.ps1.
  Remove with -Remove after the session; verify the backup manifest afterward.
  A crash requires evidence collection, offline correction and Phase 5 before
  another attempt. No blind retry.

The later hook/loopback live question remains dependent on passing that first
read-only session and completing the native table. The single Phase 6–7 user
presence touchpoint remains unused until the combined work is ready.
