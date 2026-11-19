# Plan, round 10: give the guest its own preload acknowledgement

Written by Claude, 2026-11-19, on taking over from GPT (branch `gpt/3663-port`,
from d6b947f). Claude now owns implementation. All earlier safety rules still
apply: never launch the game without Shaugn's go-ahead; verified, hashed save
backups outside OneDrive before and after every live session; never touch
anti-tamper/activation files or patch the exe; minimise launches; guest keeps
its save-folder-only redirect; stop on any crash, hang or activation error.
Credit ByJunter and Samuel Cardillo; GPL v3.

Budgets carried over from R9: startup 3/5 left, join 3/3 left.

## Diagnosis (to be confirmed in the 3663 image, Step 1)

After shared Enter, all three R8 guest packet rings show the host's
MoveFieldCompleteAck 2268 and MoveFieldCompletePostAck 2792 (plus 2005 actor
bursts), but never PlayAllPreLoadSequencerAck 2941. The guest then never sends
LoadingCompleteReq 2789, so no LoadingCompleteAck 2891 follows.

This is the failure already diagnosed and fixed on 2976
([p8-2322-observador.md](research/pendientes/p8-2322-observador.md) §2.2):

- The native server emits 2322 (2976's number for 2941) from
  MoveFieldCompleteCore **only for the field owner**.
- The client's loading state `0x11` waits until a 2322 naming its own
  character sets the sequencer marker to 2
  ([guest_reload_wait_logic.h](../src/mod/session2/guest_reload_wait_logic.h)).
- The 2976 capture of the guest loading into the host's field (GC trip 2) shows
  2144 → 3066 → no 2322, no 2445, no 2762: the same as R8 on 3663.
- The 2976 fix, `observer_milestone`, has the host call the native 2322
  emitter once per load with a ToTarget finder for the guest's session, at the
  non-owner point of MoveFieldCompleteCore. Every later 2976 two-instance host
  configuration enables it (`observer_milestone_send=1`). Samuel Cardillo's
  3576 guest preloader covers the same wait from the guest side.

R9's analysis compared against a single-player capture, where the client owns
the field and receives its 2322 unasked, so it concluded 2941 was not a
prerequisite.

The class-B null-playback crash (`CrimsonDesert.exe+0x258C8FD`,
`[layer+0x18] = 0`) also has a 2976 counterpart: an action layer with a graph
and no playback, seen on remote actors in mode 2 with no local AI
(`+0x90` ≠ local user), such as the other player's character or horse
([c1-actor-rechazado.md](research/data/c1-actor-rechazado.md) §3–4). The
upstream containment covered that whole condition.

## Game update to 1.0.0.3732 (2026-11-19)

Steam updated the game to 1.0.0.3732 at 5:08 AM Eastern on 2026-11-19
(timestamp `0x6AFDC0CF`, SHA-256 `DFCF5C50…`). Every 3663 address is stale;
the build guard keeps the mod unarmed on 3732. Shaugn chose to port to 3732.
A 3663 reference image was downloaded (depot 3321461, manifest
532620525636668883, hash verified exact) so the port compares 3663 with 3732
directly. Tools: [tools/port3732](../tools/port3732/README.md).

The builds are close: the main code section grew only `0x5000` bytes and
functions moved by small, locally constant deltas. First pass over the 158
unique RVA-like literals in the 3663 sources (112 code, 46 data):
86 code addresses resolved (73 unique signature, 12 agreed local delta,
1 thunk); about 12 literals are sizes or flags, not addresses; about 14 real
code addresses need manual review. MoveFieldCompleteCore is 3663 `0x2ED3360`
→ 3732 `0x2ED78B0`. [Matches](probe-3663/port-evidence/r10/code-matches-3663-to-3732.tsv).

Port steps, all offline, before Step 1 below:

- P1. Resolve the remaining code addresses by hand (disassembly of both images).
- P2. Data: vtables by RTTI name, globals through the RIP references of matched
  code; verify struct offsets used by each matched function.
- P3. Update every expected-bytes check at hook and call sites.
- P4. Add 3732 identity to the build guard; retarget the 3663 tables to 3732
  (names stay until the cleanup plan); full suite and smoke.
- P5. Re-verify each adoption against the 3732 image (same checks R2–R9 used).

Steps 1–6 below then apply to 3732 addresses.

**Port status (2026-11-19).** P1–P4 are done offline and pushed.

- P1: all 95 code addresses mapped.
- P2: 45 of 46 data addresses mapped (one 2976 default). Every vtable is
  confirmed by RTTI.
- P3: every expected-bytes check refreshed. 82 are identical, 13 changed only
  in branch or RIP fields, and one thunk differs only in its post-jmp filler.
  Two tests that pinned 3663 addresses were updated.
- Struct audit: 169 of 177 R2 offset rows match their 3663/3732 evidence
  functions; the rest and the R9 sampler fields were checked by hand. 3732
  changed some layouts, none that the mod reads.
- Packet opcodes are unchanged: the descriptor table is identical except for
  pointers.
- P4: the build guard now recognizes 1.0.0.3732 (timestamp `0x6AFDC0CF`,
  SizeOfImage `0x13C7E000`) and refuses the retired 3663 identity. The live
  runner, launcher, watchdog and window report pin the 3732 hash.
- 120/120 tests pass; smoke shows 78 remote states and zero rejects.
- Evidence: [r10](probe-3663/port-evidence/r10/); see
  [struct-audit-manual.md](probe-3663/port-evidence/r10/struct-audit-manual.md).

Not yet verified: any 3732 runtime behaviour. The first live launch should
be a startup check before L3, under the usual rules.

## Steps

### Step 0 — Housekeeping (offline)

- Clear the 145 line-ending-only status entries (local `core.autocrlf=input`;
  no content change).
- Build, full suite and synthetic smoke to confirm the baseline.
- Note: `docs/CODE-REVIEW-3663-CLEANUP.md` named in the takeover prompt is
  not in the repository.

### Step 1 — Confirm the owner-only 2941 path (offline) — DONE on 3732

Confirmed in the 3732 image: MoveFieldCompleteCore `0x2ED78B0` reaches the
2941 sender `0x2C7F430` only through its owner branch (`jne` at `0x2ED87DE`).
All 3732 equivalents for the 2976 milestone are in
[step1-preload-owner-path.md](probe-3663/port-evidence/r10/step1-preload-owner-path.md).
Hook point `0x2ED8852` (field in r15, traveller in r14); realm flag TLS `+0x1EB`.

Original text:

Locate the 3663 equivalents of MoveFieldCompleteCore, its owner/non-owner
branch, the 2941 emitter and the `ToTargetSendTargetFinder` constructor (RTTI).
Verify that 2941 is emitted only on the owner path. Record the evidence with
build, hash and addresses.

### Step 2 — Port the observer milestone to the host (offline) — DONE on 3732

Implemented and tested offline; not yet run in the game.

- `src/coop3663/milestone_policy.h`: the trip ledger and decision, with no game
  access. `tests/milestone3663_policy_tests.cpp` covers ordering, threads, lost
  pushes, a native 2941, owner/host/session/field checks, once per trip, and a
  new session resetting the ledger.
- `src/mod/session2/milestone3663.cpp`: before hooking, checks the 3732 owner
  branch, the rejoin and hook point, the ToTarget constructor, the 2941 sender
  and both finder vtables. It then installs a mid hook at `0x2ED8852`.
- On an eligible guest entry, the hook calls the game's constructor
  `0x2D70BE0(finder, traveller)`, confirms the finder targets the live guest
  session, and calls the sender `0x2C7F430(traveller, finder)`. Faults are
  caught and switch the module off. Decision notes are written to a fixed
  buffer in the hook and logged from the host bridge's tick.
- `take_frame` (core route) feeds the ledger after each real push, with the
  server's 2364 owner read before the guest copy is aliased.
- `[session2] core_milestone` = 0 off (default), 1 dry run, 2 send.
  `launch-2inst-3663.ps1` writes 2 for the host and 0 for the guest. If a
  requested mode cannot be installed exactly, the host v1 route refuses to
  start, so no join attempt is spent without the fix.
- `tools/port3732/check_milestone.py` checks every byte and vtable constant
  against the 3732 image (all match). 121/121 tests pass.

Expected in the next L3 host log: one `3663 milestone trip N: eligible |
returned` line after shared Enter. The guest ring should then show 2941, then
2789, then 2891.

Original text:

Host-side, native emitter, once per load trip, at the non-owner point. Keep
every 2976 guard: live session 2 of this epoch, this trip's 2268 and 2792
queued on the same thread, verified guest primary key, host-owned current
field, no native 2941 already queued, no open bundler, and disable after any
fault. No hand-built packet. Port `optd_observer_milestone_tests` to 3663.
The guest loading sampler stays to show `0x11` → `0x12` live.
Fallback if the host call cannot be verified: Samuel's guest-side preloader,
adapted to a host-owned field.

### Step 3 — Class-B containment (offline, prepared) — DONE, off by default

- `coop3663::character_guard::skip_remote_controlled`, unit-tested in
  `v1_3663_policy_tests`, applies the 2976 rule. An actor whose `+0x90`
  controlling-user key is a user key other than the local user's (the
  local controlled actor's `+0x90`) is skipped, but never the local primary.
  It applies only at the exact null-playback read `0x258ED8D` (otherwise an
  access violation), with the existing native frame, CharacterControl type,
  backlink and layer checks and a live link. It resumes at the verified helper
  epilogue `0x258EE2A`.
- The meaning of `+0x90` comes from the 2976 research (written from the
  ActorLoadDataAck record; observed in game). It is not re-derived in 3732
  because the unpack is protected code. So every sampled null-playback actor
  now also writes `null_playback_controller actor=… controller=… local_user=…
  exact_frame=… remote_controlled=… remote_actor_guard=…` to the crash log,
  whether or not the guard is on.
- Switch: `[session2] remote_actor_guard` (0 default). In
  `launch-2inst-3663.ps1` it is one variable, `$remoteActorGuard=0`, which
  must be set before the build is armed (the launcher is in the armed source
  manifest).
- The R9 rule ("no new per-actor guards until loading is understood") is met:
  Step 1 explains the loading. **Shaugn decided on 2026-11-19 to enable it
  for the next L3**, so the launcher now writes `remote_actor_guard=1` for
  both roles.

Original text:

Guard only the 2976 condition: actor `+0x90` is a non-local user and the
selected layer has a graph with null playback. Log the actor's owner and type
so the class-B actor is identified. Whether it is enabled for the first L3 is
Shaugn's decision (recommended: enabled, since Step 1 settles the loading
cause that R9's rule was waiting for).

### Step 4 — Review the R9 A1 guest startup crash (offline) — DONE

No mod cause found. R9 added nothing that runs in the guest before login.
The sandbox's HID refusals match R8's surviving guests. There was no WER
event and no armed capture. Capture now arms 25 s into startup (`33dae67`).
See [step4-r9-startup-review.md](probe-3663/port-evidence/r10/step4-r9-startup-review.md).

Original text:

R8's paired launches had no startup crash. Check that no R9 guest change
(`guest3663.cpp`, `v1_3663.cpp`, `crash3663.cpp`) runs before login, before
any launch.

### Step 5 — L3 (live, Shaugn present and approving the launch)

As in the R9 handoff: host Loads immediately, guest launches with more than six
minutes of watchdog left, Shaugn records with Win+Alt+R. Success: the guest
ring shows 2941 → 2789 → 2891 and the guest reaches control; then mutual
walking/turning, removal, reconnect and host reload. Also explain R8 A1's 52 s
host delay between 2723 and 2268 (A3: under a second) from the host ring.

### Step 6 — L4 and cleanup

L4 on the second PC only after an accepted L3; then the cleanup plan.
