# GPT plan, round 9: finish the guest's loading instead of guarding each crash

Written by Claude, 2026-11-18, after reviewing R8 (branch `gpt/3663-port`,
final commit 265705b). GPT owns all work; Claude reviews at the end. All
earlier rules still apply unless changed below.

## Where R8 ended

- The two-stage join works every time: native staging Enter, then the
  shared Enter into the host's field, both returning zero. The host's
  2364 owner alias works.
- Post-join, both processes crash while the guest is still loading:
  - Attempt 1: guest, ~63 s after Enter, `CrimsonDesert.exe+0x258C8FD`
    (null playback pointer at layer+0x18, CharacterControl update).
  - Attempt 2: host, ~11 s after Enter, access violation, no site.
  - Attempt 3: guest, ~14 s after Enter, the **same** null-playback read,
    but for actor `0xB01002C7`, **class B**, identity `other`. Not the host
    character (class A `0xA0100001`). The host-only guard never fired.
- Neither player saw the other character.
- In attempt 1, the 3663 loading-completion and preload acknowledgments
  (**2891 and 2941**) never reached the guest's native receive after the
  shared Enter, although the guest got 78 ActorLoadDataAck (2005) frames.
  The 2976 capture shows a full second-load sequence ending in
  LoadingComplete. Samuel's 3576 guest adds a preloader for a client left
  waiting for a sequencer marker.

## Claude's read of it

The guest is being handed actors (class B = not player characters, likely
NPCs, animals or objects) while its own loading never completes, so their
animation playback is never set up and the per-frame character update reads
a null pointer. Guarding one actor at a time treats the symptom. Round 9
looks for why the guest's loading does not finish.

## Rule changes for round 9 (Claude's decision)

1. **Fresh budgets:** startup 5, join 3.
2. **No new per-actor crash guards** until the loading pipeline is
   understood. The existing guards stay as they are.
3. **One diagnostic run is allowed to be a "watch only" L3:** the same
   setup, but the goal is the post-Enter message trace on both sides, not
   acceptance.

## Steps

### Step 1 — Loading pipeline analysis (offline)

1. From the R8 logs, build a side-by-side timeline after shared Enter:
   every request the guest sent (opcode, name) and every response the host
   sent or did not send, compared with the 2976 second-load sequence.
   Identify exactly which expected exchange is missing (for example
   PreLoadSequencerReq → 2941, LoadingCompleteReq → 2891, or a sequencer
   marker).
2. For each missing exchange, find where it stops: the guest never sends
   it, the v1 request allow-list or Lifecycle refuses it, the tunnel drops
   it, the host server doesn't answer, or the host answers and the guest
   tunnel doesn't deliver it.
3. Compare with Samuel's 3576 guest preloader and opcode translation, and
   decide whether 3663 needs the equivalent.
4. Fix only what that shows. Add unit tests for any allow-list, Lifecycle
   or translation change.

### Step 2 — Host log noise (offline, small)

The host's first-chance log fills with a handled read of 0x18 at
`server.cpp:319` (`peek<uint64_t>` via `server_host_user` / `host_context` /
`host_ready` / `Bridge::tick`). Check the pointer for null before reading, so
real faults are not buried. No behaviour change otherwise.

### Step 3 — Containment review (offline)

If loading cannot be completed in this round, prepare (but do not enable
without a note in the handoff) a guard modelled on Samuel's remote-update
guard that covers **every actor delivered by the host** on the guest, not
only the host primary, still only for the exact null-playback case. Record
why it is or isn't needed after Step 1.

### Step 4 — L3

Full suite and smoke, fresh backups, then L3 with crash capture on both
sides. Shaugn records with Win+Alt+R and reports what each window shows.
Accept as in R7. If it fails, capture the post-Enter message trace on
both sides and stop.

### Step 5 — Package

New top section in `docs/probe-3663/HANDOFF-CURRENT.md`: the timeline, the
missing exchange and its cause, the fix, attempt results, what each window
showed, and remaining blockers. Push and tell Shaugn.

## Prompt to give GPT

> Save the attached file as docs/GPT-3663-PORT-PLAN-R9.md on branch
> gpt/3663-port, commit and push it. Then read AGENTS.md, CLAUDE.md and that
> file and execute it from Step 1 through Step 5 without stopping for
> review. The rule changes are approved by me, including fresh budgets
> (startup 5, join 3). Commit and push after each step. Ask me only to be
> present for live sessions and to record. Finish with the Step 5 package.
