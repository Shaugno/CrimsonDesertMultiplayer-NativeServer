# GPT end-to-end plan: port the working 2976 co-op to 1.0.0.3663

Written by Claude, 2026-11-14, at Shaugno's request. GPT (Codex) owns all
implementation, research, testing and fixes from Phase 0 through Phase 9
without intermediate review. Claude reviews once, at the end (Phase 9).
This document supersedes the review cadence in CURSOR-END-TO-END-HANDOFF.md,
CLAUDE-HANDOFF.md and COOP-3663-PROTOTYPE-PLAN.md. It does not relax the
engineering rules in CLAUDE.md and AGENTS.md: guarded memory reads, no
invented offsets, version isolation, save backups, GPL/attribution.

## Goal

Two legitimate Crimson Desert installations on build 1.0.0.3663 (timestamp
0x6AF5D198, SizeOfImage 0x144E9000) play together using the mod: the guest
joins the host's world, both see each other's character walk and turn, and
disconnect/reconnect and removal are clean. This is the same core behavior
ByJunter validated on 2976 (README "Upstream co-op behavior"). Mounts, rope,
shops, combat sync, NPC sync and other session2 extras are out of scope for
this milestone. Keep them compiled but disabled on 3663.

## Strategy change (read this first)

Until now the 3663 work has rebuilt the actor system from scratch: decoding
spawn packets and component readers one by one. That is slow and still has
no runtime result. The production mod already works on 2976. Its "option D"
design lets the game's own internal server spawn and move the guest, so the
mod does not need to build spawn packets itself.

New approach: **port the existing 2976 implementation to 3663 by matching
functions between the two executables**, instead of continuing to
reverse-engineer from scratch.

- Pause expanding `src/coop3663/` (decoders/feed). Keep it and its tests;
  it is the fallback if a specific piece cannot be ported (Phase 3 blocker).
- Do not delete, rewrite, or "clean up" the 2976 code path. 3663 gets its own
  address table and its own guard branch next to it.

## Operating rules for the whole run

- **Branch:** create `gpt/3663-port` from `origin/grok/compat-guard`
  (a8656e8 or newer). Commit at the end of every phase with a message that
  names the phase. Push the branch to origin after every phase as a backup
  (the earlier OneDrive checkout was damaged). Never push to main, never
  force-push, never publish a release or binaries.
- **Do not stop for review.** Self-review, test, fix and continue. When a
  piece is blocked, record it in `docs/probe-3663/HANDOFF-CURRENT.md` (what
  was tried, the exact missing fact) and keep going with every phase or
  sub-item that does not depend on it. Stop entirely only when nothing
  independent remains, and then write the blocker handoff (Phase 9 format).
- **User touchpoints are batched.** Only these require Shaugn, and each is
  requested once, with exact instructions, when the phase is ready:
  1. Phase 1: run the Steam depot download if GPT cannot (needs his login).
  2. Phase 6/7: be present to load a save and walk a character in game.
  3. Phase 8: a second PC with a second Steam account that owns the game.
- **Game build drift:** at the start of Phases 3, 6, 7 and 8, re-hash the
  installed exe. If Steam updated it past 3663, stop live work, record the
  new identity, and redo Phase 3 matching for the new build before any
  further live step. Never broaden a guard to accept an unverified build.
- **Live testing is authorized by Shaugn for Phases 6-8 only**, under the
  safety rules in those phases. Phases 0-5 keep the game closed.
- Write in English. Existing research notes are Spanish; read them, cite
  them, do not translate them.

## Phase 0 — Baseline and review fixes (game closed)

1. Check out the branch, confirm `git log -1`, run `scripts/build.ps1` (or the
   documented sibling build) and record the full CTest result as the baseline.
2. Fix the five findings from Claude's review of a8656e8, with regression tests:
   - `Feed`: replace the shared 32-entry retirement ring with a per-peer
     "highest generation seen" rule; `begin_session` accepts only a strictly
     higher generation than any previously seen for that peer. Test: an old
     generation is refused after 40 other peers churn; an older generation
     is refused after a newer one ends.
   - `accepted_pose` and `decode_move_ack_transform_fields`: one shared
     quaternion-normalize helper; delete the dead `kMinHeadingNorm`; make the
     header comment state the real accepted range (norm 0.5-2).
   - `Feed`: document "single-threaded; caller serializes" in the header.
   - `disconnect_all()`: return the number of sessions it failed to retire.
   - Movement decoder: use `opcode_is`; fix the two -Wsign-conversion
     warnings in `tests/coop3663_tests.cpp` (lines ~251, ~287).
3. Full suite passes. Commit "Phase 0".

## Phase 1 — Obtain the reference executables (analysis only)

1. Download the 2976 executable from Steam for the owned game: app 3321460,
   depot 3321461, manifest 8201393788450828800 (recorded in
   `docs/research/cheat-tables-versions.md`). Use DepotDownloader or the Steam
   console `download_depot 3321460 3321461 8201393788450828800`. If it needs
   Shaugn's Steam login, give him the single command and wait.
2. Verify SHA-256 `57DA440D72F4DB974F25FEF047CF84C4DADD999A88CB2A3C5AF4C9BD67FDE1E7`.
   Also fetch 3576 if its manifest can be found (useful as a midpoint).
3. Store them outside the repo (for example `..\reference-bins\2976\`).
   **Never commit game binaries. Never run them. Never point Steam's install
   at them.** They are inputs to static matching only. Saves are shared in
   `%LOCALAPPDATA%\Pearl Abyss\CD\save`; an old build must not touch them.
4. Record paths and hashes in HANDOFF-CURRENT.md. Commit "Phase 1" (docs only).

## Phase 2 — Inventory what the core co-op path needs (game closed)

1. Determine which code path delivered the validated 2976 walk-around:
   `src/mod/session2/` (option D, guest joins host's internal server) and/or
   `src/mod/net/remote_puppet.*`. Use `docs/OPCION-D-EMULADOR.md`,
   `docs/ROADMAP.md`, `kit/cdmp.ini` defaults and the code itself.
2. Define the **minimal feature set**: startup/guard, player reader, transport
   (Steam + UDP), session handshake, guest login into the host's server,
   movement both directions, remote character creation/removal, disconnect/
   reconnect, guest save protection. List every other module (mounts, rope,
   shops, docked gimmicks, NPC/enemy relay, combat probes, overlay extras)
   as "disabled on 3663".
3. Extract every 2976-specific value the minimal set touches: RVAs, vtables,
   struct offsets, byte signatures, Troc message IDs, payload sizes, and
   Denuvo-related notes. Output `docs/probe-3663/PORT-INVENTORY.md`: one row per
   value with file:line, symbol name, kind, what it is used for, and how it was
   verified on 2976.
4. Make sure each disabled module is actually unreachable on 3663 (config
   default off plus guard check), so a missing address cannot be dereferenced.
5. Commit "Phase 2".

## Phase 3 — Match 2976 to 3663 (file-only, game closed)

1. Use Ghidra headless (Version Tracking or BSim) and/or Diaphora/BinDiff to
   match functions between the 2976 and 3663 executables. Supplement with:
   RTTI names and vtable layouts (the repo already has RTTI tooling), string
   cross-references, unique byte signatures, call-graph neighbours, and the
   existing `tools/pkt3663` scanners and entry verifier.
2. For every inventory row, record: 3663 value, match method, confidence
   (exact / strong / weak), and an independent second check. Struct offsets
   must be confirmed from code that uses them in 3663, not assumed unchanged.
   Troc IDs and payload sizes must be confirmed from 3663 handler code.
3. Rows in Denuvo-mutated code (`.xtls`) will not diff cleanly. Trace those
   by hand from their callers/callees; if still unresolved, mark blocked.
4. Only exact or strong matches with a second check are adopted. Weak or
   unmatched rows stay out of the build and are listed as blockers with the
   specific evidence missing.
5. Output `docs/probe-3663/PORT-MATCHES.md` plus the tool scripts/projects
   needed to reproduce it (scripts in `tools/`, no game bytes committed beyond
   the short signatures the repo already permits). Commit "Phase 3".
6. If a core row cannot be matched at all (for example the option-D server
   accept path changed), evaluate whether the `src/coop3663` decoder route
   covers that piece, and document the decision.

## Phase 4 — Implement the 3663 path (game closed)

1. Add `src/mod/game/build_3663.h` with only adopted values, each commented
   with its 2976 counterpart and the PORT-MATCHES row.
2. Extend `build_guard` with a separate `Build3663` identity (timestamp,
   SizeOfImage, base) that arms only the minimal feature set. 2976 behavior
   must be byte-for-byte unchanged; 3576 stays recognized-but-unarmed.
3. Route the minimal modules through a per-build address table (or a
   templated/indirected accessor) rather than copying modules. Keep every read
   guarded (`VirtualQuery`/SEH) and every hook a function-entry or
   `safetyhook::create_mid` hook per CLAUDE.md.
4. If the wire protocol changes, bump `kVersion`. Keep the Steam-friends
   restriction and the executable filter.
5. Unit tests for the guard (2976, 3576, 3663, unknown, and a 3663 file with a
   wrong SizeOfImage) and for any new table/indirection. Commit "Phase 4".

## Phase 5 — Game-free validation (game closed)

1. Full CTest suite passes, plus the new tests.
2. `cdmp_host` + `cdmp_peer` loopback smoke test passes with `CDMP_TEST_HOST`
   (use `build\e2e\` to avoid the long-path LoadLibrary failure).
3. `cdmp_dx12_test` still passes if the overlay is touched.
4. Self-review the whole diff against AGENTS.md "Review priorities". Fix
   what you find and rerun. Commit and push "Phase 5".

## Phase 6 — Live check, one game instance (Shaugn present)

Safety rules for Phases 6-8:
- Back up `%LOCALAPPDATA%\Pearl Abyss\CD\save` to `backups\<timestamp>\` with
  a hash manifest before every session. Verify the manifest after.
- Re-hash the installed exe first (build drift rule).
- Deploy with `scripts/deploy.ps1`; remove with `-Remove` after each session.
- A crash is reported to Pearl Abyss automatically: if the game crashes,
  stop, collect `cdmp.log` and the crash context, fix the cause offline,
  and rerun Phase 5 before another live attempt. Never retry blindly.
- Each session has one stated question, an expected observation, a time
  limit, and a stop condition, written in HANDOFF-CURRENT.md beforehand.

Steps:
1. Read-only first: start the game with the mod, load a save, and confirm the
   3663 player reader reports position/rotation/HP that track the character
   (walk, turn, fast travel). No hooks that write memory yet.
2. Then enable the hooks of the minimal set against `cdmp_peer` over loopback
   and confirm the session handshake and movement stream in `cdmp.log`.
3. Commit evidence (logs, hashes; no saves) and push "Phase 6".

## Phase 7 — Two instances, one PC (Shaugn present)

1. Use `scripts/launch-2inst.ps1` per CLAUDE.md (SteamAppId/SteamGameId env,
   `PlatformServiceType=Steam`). Give each instance its own role, ini and
   log. Separate the guest's save first and back up both.
2. Host loads a save; guest joins via the option-D path. Acceptance: each
   window shows the other character walking and turning; guest disconnect
   removes its character on the host; guest reconnect works; host save is
   untouched (compare manifests).
3. Record a short screen capture of both windows. Commit evidence, push
   "Phase 7".

## Phase 8 — Two PCs over Steam (Shaugn + second PC)

1. Follow `docs/PRUEBA-DOS-PC.md`. Package the guest kit; Shaugn installs it on
   the second PC and returns the guest logs via the shared folder.
2. Same acceptance as Phase 7, over the real Steam tunnel, plus one load
   screen / fast travel on each side.
3. Commit evidence, push "Phase 8".

## Phase 9 — Final package for Claude's review

Update `docs/probe-3663/HANDOFF-CURRENT.md` with a new top section:

- branch, final commit, the full commit list since a8656e8;
- build configuration, toolset, and binary hashes;
- PORT-INVENTORY and PORT-MATCHES summary: adopted / blocked counts and every
  blocked row with its missing evidence;
- exact test commands and results for Phase 0, 5, and each live phase,
  including failures, fixes and reruns;
- live session records (question, identity, result, log/capture hashes);
- what works, what is disabled on 3663, known limitations.

If the goal was not reached, write the same package as a blocker handoff: the
furthest phase completed, the exact missing fact or action, and the smallest
experiment that would resolve it. Do not describe a loopback or simulated
result as two-player evidence. Push, then tell Shaugn the branch is ready
for Claude's review.

## Prompt to give GPT

> Read AGENTS.md, CLAUDE.md, and docs/GPT-3663-PORT-PLAN.md. Execute that plan
> from Phase 0 through Phase 9 on branch gpt/3663-port without stopping for
> review. Commit and push the branch after each phase. Ask me only for the
> batched touchpoints the plan lists (Steam depot login, in-game presence for
> Phases 6-7, the second PC for Phase 8). If something is blocked, document it
> and continue with independent work. Finish with the Phase 9 package.
