# Phase 7 — blocked; no two-instance test

Re-hashed installed image at this checkpoint: SHA-256
8BE67178D9B9A149C361A731051665E0BA4D441E250AE1DFE645BD6E524345C8,
timestamp 0x6AF5D198, SizeOfImage 0x144E9000. No build drift.
File evidence: `port-evidence/phase7-preflight-identity.json`.

Depends on Phase 6 and the unmatched field-sharing/identity/cleanup contracts
in PORT-MATCHES.md. The current 3663 guest deliberately ends before the 2976
save redirect. Launching it would not test the target behavior. No launcher,
game instance, native hook, deployment or save operation was attempted.
There is no screen capture or live-log evidence for this phase.

Prepared session after prerequisites pass:

- Question: can the 3663 guest join the host's native field, walk/turn on both
  screens, disappear on guest disconnect and return correctly on reconnect?
- Expected observation: both native characters appear with distinct identities;
  removal completes before reuse, reconnect creates the new session, and the
  host save manifest proves no guest writes to the host's save data.
- Time limit: 15 minutes after both characters are ready.
- Stop condition: any crash, shared-save access, stale actor/key reuse, native
  field/lifetime inconsistency, build drift or backup verification failure.
- Before launch: separate guest save storage, back up/hash both sources,
  allocate role-specific ini/log paths, and use scripts/launch-2inst.ps1 with
  SteamAppId/SteamGameId and PlatformServiceType=Steam as documented in CLAUDE.md.
  Record both windows only after actual acceptance; remove the deployment and
  verify backup manifests afterward. Crash recovery requires Phase 5 again.

The combined Phase 6–7 presence touchpoint was not requested because the
offline prerequisites are unresolved. Synthetic loopback is not a substitute
for this acceptance test. Status: blocked, not passed.
