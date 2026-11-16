# R4 Step 4 — guarded v1 route and stable disconnect

Completed offline, 2026-11-16. All 114 tests pass in 101.74 seconds. The
eight-second synthetic host/peer smoke receives 78 remote states, rejects
zero packets, reports compiled tier mask 0xF, and unloads cleanly. These
checks execute no native C/D gameplay and do not establish L3 or L4.
`port-evidence/r4/step4-result.json` seals the local evidence hashes.

Reproduction: Step 3 implemented target join and identity adapters without
a target guest transport, lifecycle route or C/D permission. Expected behavior
is an explicit, exact-build v1 route with verified file redirection, bounded
native transport, ordinary-character recipients and native stable logout.

The default 3663 installation remains the HP reader. Explicit
`[session2] core_v1=1` requires all four compiled tiers, verified file sandbox,
consistent roles, session/network configuration and Steam or strict local
loopback. A controlled marker rejects unsandboxed relaunches. Guest startup
refuses before gameplay if any prerequisite fails. The 2976 branch and
explicit synthetic test-host paths remain intact.

The new target guest intercepts only the verified Send entry and native
NetworkThread pump. It copies bounded frames, returns each native send buffer
once, and delivers host bytes through the adopted native receive function.
Initial title refresh uses the native save-slot parser under the verified
WindowsLock3/busy protocol. It adds no custom eligibility predicate or legacy
title patch. Host channel discovery permits a narrow native RTTI request
allowlist; travel/time controls, repeated loading requests after admission,
combat, mounts and optional gameplay stay refused or unreachable.

Native ExceptControl/ExceptSelf entries use the separately adopted ToNear
implementation for the two ordinary primary characters, removing the owning
session from the bounded result. They do not guess a legacy actor-session
virtual slot. Identity readiness includes generation-bearing user/actor keys,
so address reuse alone cannot authorize admission.

Initial joining reaches two world-clock paths. Native Travel's owner check
skips publication for a guest in the host-owned field. LoadingComplete has
six arguments and a separate fifth-argument publication flag; its new target
entry hook forces that flag to zero only for the verified guest. All other
arguments and native work are preserved. This is why travel-request filtering
alone was insufficient. Source evidence is retained in the R2 clock-gate and
travel-clock-owner exports and the R4 v1 binding catalog.

Stable/title disconnect requests native LogoutSession first. The lifecycle
refuses logout during loading and quarantines on loading loss instead of
inventing completion payloads. Native removal supplies normal actor cleanup;
the older manual SAM bucket/reconnect barrier is unreachable in v1. D030's
bounded bucket geometry remains a documented fact, not a claim of live
removal. Target reporter guards use adopted target bytes, never legacy RVAs.

Join field locks follow native order: acquire field+18 before reading and
acquiring the second lock. Failure before original EnterField writes the
verified native refusal result. A native fault seals admission and new native
injection, retains uncertain owning pins, and continues exception search.
It does not attempt damaged-lock recovery or manufacture native success.

The controlled launcher backs up and copies both roles' saves outside
OneDrive, binds deployed files to offline evidence, and verifies the real
manifest after exit. It records installation originals before changing any
file and restores them even if save verification fails. A watchdog pins each
test process by PID, creation time, path and executable hash; guest and extra
process deadlines cannot exceed the host's fourteen-minute deadline. Tests
cover normal exit, deadline termination, identity mismatch, save drift and
installation restoration. A local-only ten-second capture helper accepts
only the two controlled foreground windows; its fixture uses owned images.

Reconnect is a known limitation: a changed guest native conversation exits
with guard code 0xCD67 before another conversation starts. Host field changes,
loading loss and uncertain rollback require process restart. Only the measured
single-primary roster is supported. The file sandbox is API interception,
not a kernel security boundary. The existing optional S3 publication race is
unfixed and outside this route.

Manual acceptance remains Step 5: two real sandboxed game processes, each
seeing the other walk/turn, guest removal after disconnect, unchanged real
saves, reconnect outcome and local capture hash. Only after L3 passes may the
second-PC Steam sitting establish L4. Current join-attempt count: zero.
