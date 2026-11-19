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

## Steps

### Step 0 — Housekeeping (offline)

- Clear the 145 line-ending-only status entries (local `core.autocrlf=input`;
  no content change).
- Build, full suite and synthetic smoke to confirm the baseline.
- Note: `docs/CODE-REVIEW-3663-CLEANUP.md` named in the takeover prompt is
  not in the repository.

### Step 1 — Confirm the owner-only 2941 path in 3663 (offline)

Locate the 3663 equivalents of MoveFieldCompleteCore, its owner/non-owner
branch, the 2941 emitter and the `ToTargetSendTargetFinder` constructor (RTTI).
Verify that 2941 is emitted only on the owner path. Record the evidence with
build, hash and addresses.

### Step 2 — Port the observer milestone to the 3663 host (offline)

Host-side, native emitter, once per load trip, at the non-owner point. Keep
every 2976 guard: live session 2 of this epoch, this trip's 2268 and 2792
queued on the same thread, verified guest primary key, host-owned current
field, no native 2941 already queued, no open bundler, and disable after any
fault. No hand-built packet. Port `optd_observer_milestone_tests` to 3663.
The guest loading sampler stays to show `0x11` → `0x12` live.
Fallback if the host call cannot be verified: Samuel's guest-side preloader,
adapted to a host-owned field.

### Step 3 — Class-B containment (offline, prepared)

Guard only the 2976 condition: actor `+0x90` is a non-local user and the
selected layer has a graph with null playback. Log the actor's owner and type
so the class-B actor is identified. Whether it is enabled for the first L3 is
Shaugn's decision (recommended: enabled, since Step 1 settles the loading
cause that R9's rule was waiting for).

### Step 4 — Review the R9 A1 guest startup crash (offline)

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
