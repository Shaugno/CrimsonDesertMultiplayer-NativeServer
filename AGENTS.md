# Development instructions

Read CLAUDE.md before changing code. It defines the memory, hook, networking,
game-build, and licensing rules for this NativeServer checkout. Read the relevant
docs/research notes before changing a reverse-engineered behavior.

This checkout is Shaugno's independent continuation and the development target.
Project home: https://github.com/Shaugno/CrimsonDesertMultiplayer-NativeServer.
`origin` is this continuation; `upstream` preserves ByJunter's original source.
Preserve original attribution, history, and the GPL v3 license. Do not describe
the original implementation as newly authored by this continuation.
The sibling folder
`CrimsonDesertMultiplayer` is the original mod and stays untouched, including
its local setup files. The parent `Start-Grok.ps1` still opens that original
checkout; do not retarget it unless the user asks.

## Working agreement

Latest handoff (2026-11-14): the user requested transfer to Claude. Read
docs/CLAUDE-HANDOFF.md for the current checkout, offline-only implementation
scope, evidence and next package. Preserve all uncommitted work. The earlier
ownership arrangements below describe previous phases.

Current reassignment (2026-11-14): the user asked Codex to take over active
implementation and investigation from Grok. Continue offline movement and
actor creation/removal work without launching or attaching to the game.
Preserve the existing local work. The earlier ownership arrangement below
describes the previous workflow.

- Current user direction (2026-11-14): Cursor/Grok owns end-to-end
  implementation, self-review, testing, and fixes through the two-player
  prototype. Codex performs the final review, rather than a mandatory review
  after each intermediate fix. Read docs/CURSOR-END-TO-END-HANDOFF.md for
  the current workflow. The user can assign either assistant additional work.
- Work on a task branch. Keep each change focused on a reproducible issue and
  describe expected behavior and reproduction steps before implementing a fix.
- Preserve unrelated changes. Report the files changed, commands run, results,
  remaining uncertainties, and a short manual test plan at handoff.
- Do not claim a runtime issue is fixed based only on compilation or unit tests.
- Keep patches local for review; publish changes to Shaugno when the user requests
  it. The working agreement is Cursor/Grok implementation and validation,
  followed by Codex final review.

## Validation

- Run scripts/build.ps1 for a baseline and after relevant code changes. It configures
  an x64 MSVC build, compiles, and runs CTest. Report missing prerequisites honestly.
- Add targeted regression coverage for substantive behavior fixes where practical.
- Game-specific behavior requires the supported build and a separate runtime test.
  The supported game build is Crimson Desert 1.0.0.2976 / public patch 2.03.02,
  PE timestamp 0x6AB28F00, image base 0x140000000.
- Start with game-free tests. Deployment and in-game testing are separate tasks;
  follow CLAUDE.md, including save backups, when those tasks are requested.
- `cdmp_host` is the game-free host smoke test. It sets `CDMP_TEST_HOST` and must
  keep working without a Crimson Desert process. Do not launch the game from a
  validation step unless the user asks.

## Review priorities

Check thread/lifetime correctness, bounded and validated network payloads, guarded
game memory access, actor identity across peers, protocol compatibility, and
reconnect/cleanup behavior. Preserve the Steam-friends restriction and executable
filter. Never invent offsets or disable game-build compatibility checks to force
the mod to load. Preserve explicit test-host paths (`CDMP_TEST_HOST`,
`CDMP_TEST_OVERLAY`).

See docs/DEV-WORKFLOW.md for the local setup and handoff prompts.
