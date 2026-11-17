# R5 Step 2 — single-process startup acceptance

S1 and S2 pass on the Step 1 binary, SHA-256
`1C112AEFC06E2B158E0DAB7DD76ED94984CD00CC5C685837C062DA2F7D60438D`.
The embedded banner is `31748D3C + local changes`; the tested source was
committed as 5129358 and sealed by the Step 1 manifest. No newer embedded
commit label is claimed.

| Attempt (2026-11-16 Eastern) | Result |
| --- | --- |
| R5-S1, 21:01–21:11 | Reached title, loaded the save, native world readiness and over one minute in-world; watchdog stopped the process at 600 seconds. Normal quit was not established. Incomplete, counted. |
| R5-S2, 21:22–21:27 | Guest-only title reached; user selected Quit to Desktop; normal exit 0. Native profile isolation verified, file_hooks=0, networking and remote guest installers off. Passed. |
| R5-S1B, 21:31–21:35 | Host save loaded, native descriptors/host route/world readiness verified, over one minute in-world; user selected Quit to Desktop; normal exit 0. No host file sandbox. Passed. |

The first trial's exit timing remains unconfirmed. No native crash was
observed and no code defect was inferred from its deadline stop. Before the
retry the unchanged binary passed the full **116/116** suite again,
**135.49 seconds**, including owned normal/deadline/identity watchdog
reproductions. The corrected procedure gave the complete sequence before
launch and removed the second title-message wait. The retry exited well
before its fixed deadline.

Every session used fresh verified backups outside OneDrive, an exact 3663
executable hash, a written question, present user, PID/birth-pinned ten-minute
watchdog, and no assistant UI control. All three real-save manifests remain
unchanged and both frozen backups verify. The host is allowed normal
autosave; it was not required to remain immutable and no progress was
restored over. All six controlled installation files were removed after each
session; no game process remains after S1B cleanup.

`port-evidence/r5/step2-results.json` records each outcome, exit code, raw log
hash, launch/deadline receipts, save manifests and cleanup proof. S2's copied
stale host-log alias is not treated as a guest-session observation; its proof
uses the guest PID and guest log. Startup usage is **3/5**, join usage **0/3**.
No L3 or L4 acceptance follows from startup checks.

Independent upstream research continued while waiting for live touchpoints.
`R5-UPSTREAM-REFERENCE.md` records the verified 3576 download, three-argument
EnterField comparison, paired guard vtables/bodies and character-call-chain
checks. No upstream runtime code was copied or merged in this step.
