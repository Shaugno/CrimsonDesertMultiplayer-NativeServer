# R4 runtime blockers and remaining limits

Steps 1–4 are implemented. The additive ledger has no remaining blocked
address/layout rows: A 37/37, B 78/78, C 150/150, D 130/130. Closed rows
include approved policies and scope retirements; they are not all native
addresses or claims of executed runtime behavior.

| Obligation | Current evidence | Remaining requirement |
| --- | --- | --- |
| L3 host startup | Third guarded host activated, then crashed before title; small-stack mapping overflow reproduced and corrected offline | A later authorized live round must establish that the corrected game reaches title/world |
| L3 guest join/mutual movement | Zero guest launches/native joins | Two real game processes must show each other walking/turning |
| Guest disconnect/removal | Native logout path and guards implemented/tested on owned data | Real guest disconnect must remove its character from the host |
| Live saves/capture | All three controlled startups verified unchanged real-save manifests and restoration; no two-window capture reached | Full L3 real-save checks and a local capture hash |
| Reconnect | Guarded unsupported path exits guest on native conversation change | Report actual live reconnect outcome when a later round reaches it |
| L4 Steam/two PCs | Not attempted; Steam friends restriction preserved | L3 must pass before second-PC kit deployment/test |

R4 ends live work after three failed L3 startup experiments. This counts
the first stalled Exit and both startup crashes against the plan's cap;
zero native joins does not reset that limit. See `R4-LIVE.md` and
`port-evidence/r4/startup-trials.json`. No review approval is requested at
an intermediate step; the Step 6 package records the incomplete milestone.

The last game crash is consistent with the reproduced 128 KB-stack file
callback overflow. The correction passes the expanded fixture/full suite,
but has not been retried in the real game. Earlier Steam shared-memory,
named-pipe/reopen and hook-teardown bugs have offline reproductions and
corrections; they do not establish complete runtime compatibility.

The v1 route supports only the measured single-primary roster. Guest
travel/time, mounts, combat and optional modules remain out of scope.
Loading loss, uncertain insertion rollback or host field replacement seals
the route and requires process restart. Reconnect is unsupported. API-level
file redirection is not a kernel security boundary. The earlier optional
S3 publication race is unmodified and unreachable through this route.
