# Current direction: first two-player walk-around on 3663

Updated 2026-11-14 after the user requested an end to the repeated capture
and review loop. Grok implements; Codex reviews completed milestones.

This direction supersedes the requirement to keep repeating Packages A/B
in `GROK-3663-EXECUTION-PLAN.md` before any independent work can proceed.
Preserve the capture tools and archives. Captures remain an available
experiment when a specific missing observation justifies one.

## Deliverable and current limits

First deliverable: two legitimate game installations on the verified target
build can each see one remote character walking and turning, with clean
disconnect and removal. This is a prototype. Shared combat, quests, mounts,
progression, and release readiness are later milestones.

Checkpoint `3e950a7` provides reviewed diagnostics and game-free tests.
The current handoff reports no usable post-ready movement evidence.
Production gameplay integration remains restricted to its older build.
The installed file must be rechecked before eventual runtime work; a
future Steam update may invalidate the recorded 3663 identity.

Existing code provides a starting point, not proof of 3663 compatibility:

| Existing component | What it establishes | Work still needed |
| --- | --- | --- |
| `src/common/protocol.*` and networking | Player-state serialization and transport mechanisms exist | Reuse only applicable mechanisms; validate any new adapter and wire changes |
| `tools/diag3576/reader.*` | Pattern for bounded, coherent external read-only sampling | Establish 3663 addresses, field meanings, and validity independently |
| `src/mod/game/player_reader.cpp` | Older-build state reader using `build_2976` | No 3663 state-reader compatibility is established |
| `src/mod/net/remote_puppet.*` | Older remote-actor implementation using native load and movement packets | Verify a 3663 actor lifecycle and packet layouts or an evidenced alternative |

Reading position directly could reduce dependence on decoding outgoing
movement packets. It cannot by itself create or control a remote actor.
Whether this route saves work is an engineering question to resolve from
the existing implementation and executable evidence.

## Next work package: finish the independent work first

Keep the game untouched during this package. Preserve unrelated local edits.

1. Trace the smallest existing path from player sample to peer transport to
   remote actor creation, movement, and removal. Produce a dependency table
   separating reusable code, older-build assumptions, and missing 3663 facts.
   Include loading, actor replacement, and reconnect dependencies.
2. Compare two practical routes: native packet relay with verified layouts;
   or verified read-only state plus a separately verified actor-control
   adapter. Select a route only when the evidence supports the choice.
   Name the concrete blocker if neither is ready. Do not promise that a
   position reader eliminates the native actor work.
3. Use file-only analysis to establish 3663 candidate state and lifecycle
   locations where possible. Trace provenance; distinguish file evidence
   from live field meaning. Do not port old RVAs by an assumed displacement
   or treat a matching class name as proof of its fields.
4. Implement the bounded, game-independent pieces justified by that design:
   state validity model, reader abstraction and synthetic memory fixtures,
   transport adapter if needed, and actor-lifecycle interface with a fake
   backend. Keep unresolved fields absent or explicitly invalid. An identity
   quaternion, zero health, or a stable pointer is not proof of real state.
   A coherent read does not establish player readiness or loading status.
5. Exercise the proposed feed/adapter using owned test processes or loopback.
   Clearly label generated state. Cover relevant malformed input, stale
   samples, identity changes, disconnect, and cleanup. Reuse existing tests
   where they already cover the behavior. Bump the protocol version if the
   wire format changes. Keep Steam-friends restrictions intact.
6. Self-review and fix failures within this package. Run meaningful focused
   tests and the required full suite at the completed code milestone with
   the game closed. Report failures and runtime limits honestly.

Do not implement a speculative native actor call, replay a packet, open the
production guard, or deploy a changed diagnostic as part of this offline
package. Keep historical 2976 behavior isolated and old diagnostics unchanged.

## One consolidated handoff

Update `docs/probe-3663/HANDOFF-CURRENT.md` only with work actually completed.
Include the source commit, changed files, route/dependency table, evidence,
tests performed, and the smallest remaining runtime experiment. Resolve
ordinary implementation/test problems independently before requesting review.
Do not ask the user to relay every small fix.

Every proposed live experiment must state the single question, exact
identity, expected observation, bounded duration, acceptance condition,
and stop condition. Request the user's in-game actions only when ready to
perform that experiment. Before a new runtime implementation is deployed,
obtain one milestone review. Preserve backups and ownership/timeout rules.
An empty capture is a measured result, not a reason to repeat it unchanged.

## Subsequent acceptance milestones

1. **State feed:** live-verified 3663 position and heading, explicit readiness
   validity, and bounded state exchange. Unknown readiness suppresses the
   gameplay feed. A synthetic loopback demo alone is not this milestone.
2. **One remote actor:** separately reviewed 3663 lifecycle, movement, and
   cleanup; one actor appears, follows verified state, and is removed cleanly.
3. **Two-PC prototype:** both sides observe walking/turning, reconnect and
   loading behave as documented, and cleanup succeeds. Then scope combat,
   world interaction, and saves as separate deliverables.

There is no defensible completion date until the 3663 actor path is resolved.

## Prompt for Grok in Cursor

> Follow docs/COOP-3663-PROTOTYPE-PLAN.md. Pause repeated packet captures.
> Complete the offline work package toward a two-player walk-around prototype:
> trace the existing state/transport/remote-actor path, identify the 3663
> dependencies, select an evidence-supported route, and implement/test the
> independent pieces. Keep unknown fields explicit and preserve version guards.
> Handle your own self-review and test-fix loops. Deliver one consolidated
> HANDOFF-CURRENT.md with results and the exact next live experiment or blocker.
> Keep the game untouched and changed runtime code undeployed during this package.
