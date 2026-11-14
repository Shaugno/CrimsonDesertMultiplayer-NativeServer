# Claude implementation handoff — 2026-11-14

The user requested this handoff to Claude after Codex's offline component-reader
work. Continue implementation from the local checkout. Codex can review the
result when the user returns; no intermediate review is required for every step.

## Use this checkout

Repository:
`C:\Users\shaug\.codex\visualizations\2026\11\12\01a1cf3e-5728-7633-99ba-7c7ac1473b17\NativeServer-work2`

Branch: `grok/compat-guard`. Base before this progress checkpoint: `502577a`.
Origin: `https://github.com/Shaugno/CrimsonDesertMultiplayer-NativeServer.git`.
The user subsequently requested a commit and push of the offline progress and
this handoff. This checkpoint includes the source, tests, reports and raw
file-analysis traces previously described as local/uncommitted. Use
`git log -1` and `git status --short` to verify the actual checkout and preserve
any newer changes. Do not reset, clean, or replace this working checkout.
The older OneDrive checkout was damaged/dehydrated; do not use it as the source.
Build outputs, save backups and ignored live capture archives remain local;
they are not included in this source checkpoint.

## Goal and current boundary

First deliverable: two legitimate installations on an explicitly supported
build see each other's characters walking/turning, with correct creation,
identity, disconnect/reconnect, and removal. Combat, quests, progression and
save synchronization are later work. Full co-op has NOT been demonstrated.

Current authorization is OFFLINE ONLY. Keep the game closed. Do not launch or
attach to it, inject/load a DLL, replay packets, invoke native actor functions,
deploy cdmp.asi/winmm.dll, change installed files, or modify saves. File-only
inspection of the installed executable and synthetic tests are allowed.
Keep production version guards closed to unsupported builds. Do not adopt
candidate offsets, borrow 2976/3576 layouts for 3663, or bypass checks.

Target file: `D:\SteamLibrary\steamapps\common\Crimson Desert\bin64\CrimsonDesert.exe`.
Assessed identity: 1.0.0.3663, timestamp 0x6AF5D198, preferred base
0x140000000, SizeOfImage 0x144E9000. Recheck identity before file analysis;
Steam may update it. These are executable identity values, not a claim about
the public marketing patch number. Earlier 2976/3576 notes are historical.

## Read first

1. Repository `AGENTS.md` and `CLAUDE.md` (memory, versioning, license rules).
   Earlier Grok/Codex ownership language is historical; this handoff records
   the user's latest request to transfer the work to Claude.
2. `docs/probe-3663/HANDOFF-CURRENT.md` (newest section first).
3. `docs/probe-3663/COMPONENT-READERS-20261114.md`.
4. `docs/probe-3663/COMPONENT-LIFECYCLE-20261114.md`.
5. `docs/probe-3663/SPAWN-HEADING-REMOVAL-20261114.md` and
   `docs/probe-3663/MOVEMENT-CONSUMERS-20261114.md`.
6. `docs/COOP-3663-PROTOTYPE-PLAN.md` for broader milestones.

Preserve ByJunter attribution and GPL v3. Do not copy unlicensed reference code.

## Implemented locally

- `src/coop3663/feed.*`: synthetic-only actor lifecycle abstraction and fake
  backend. Explicit session generations, bounded peer state, initial pose,
  normalized rotation, failed-operation handling and stale-sample refusal.
- `src/coop3663/ack_decode.*`: bounded offline decoders for movement fields,
  rotation-like data, spawn envelopes, removal fields, isolated body relation
  tails, and the first concrete character-control component reader.
- `tests/coop3663_tests.cpp`: malformed/truncated data, boundaries, conditional
  fields, variable records and synthetic lifecycle coverage.
- `tools/pkt3663/ack_trace.cpp`: file-only disassembly and RTTI inspection;
  maps the executable read-only and never executes its entry point.
- CMake links this offline library only to tests, not the production ASI.
- `tests/pkt3663_live_ctl_tests.cpp`: test log directories include the test
  process creation identity to avoid recycled dummy-PID collisions. Production
  evidence overwrite protection is unchanged.

This is NOT a native character backend. Decoder success means structural
consumption only, not valid resources, readiness, ownership, or a usable spawn.
Borrowed spans require source-buffer lifetime. The character decoder returns
consumed length and leaves subsequent components unread.

## Key evidence for the next work

Character-control RTTI name RVA 0x6F858B0 -> locator 0x621C170 -> primary
vtable 0x59C1660 -> reader slot +0x58 at 0x2582580. Transform-sync name RVA
0x6F8B890 -> locator 0x6226BE0 -> vtable 0x59CBA60 -> reader 0x23DCEA0.
All remain experimental and unadopted.

Character-control conditional entries are 4 or 31 WIRE bytes, while their
native object stride is 48. Resource identifiers consume 8 wire bytes each
before native registry lookup yields a 4-byte result. Do not infer wire
widths from destination sizes. The minimal 62-byte structural fixture is
NOT a valid character template.

Initialization follows actor+0x88 to a descriptor, then calls virtual +0x38
on descriptor+8 with actor and descriptor selector byte +1. The concrete
factory and component inventory remain unresolved. Initialized non-null
slots become a compact ordered list at actor+0x78/count+0x80. Component virtual
+0x58 consumes the shared reader; there is no proved generic wire framing.

Component cleanup runs in reverse order via actor virtual +0x118 and component
virtual +0x38. Detach has a deferred callback path. Return from removal is not
proof of final destruction. Failed-initialization rollback and thread/ownership
contracts remain unresolved. Never substitute raw free/delete for that lifecycle.

Latest raw evidence: `docs/probe-3663/component-readers-trace-20261114.txt`,
SHA-256 B7F620147F8C48FF31FD3BE02AC343770CC6E61C4B08B0645FEFEC25F5EFF524.
Earlier traces and reports are retained; do not overwrite historical evidence.

## Next work package

Work through these dependencies autonomously, keeping evidence and tests together:

1. Resolve the concrete descriptor factory and required ordered components
   for a remote visible character. Trace constructor defaults and distinguish
   user actor, normal body, and child-only body. Do not guess an inventory.
2. Finish transform-sync nested readers, including 0x1797250, 0x1796F80 and
   0x179D530. Account for every nested/conditional byte; keep unknown semantics
   raw. Trace 0x14D0870's computed constants before adopting field boundaries.
3. Determine resource lookup requirements and omitted-field defaults. Separate
   structural parsability from semantically valid spawn data.
4. Compose an offline whole-spawn decoder only when component order and each
   boundary are evidenced. Test truncation, count limits, conditional branches,
   invalid resources where modelable, and exact component/tail consumption.
5. Model creation failure, rollback and deferred cleanup completion using fake
   backends. Do not claim those tests prove native execution or thread safety.

Stop at a concrete evidence gap rather than inventing fields. Document the
precise unresolved target and what would resolve it. A later live phase needs
separate user authorization and preparation; do not quietly launch the game.

## Build and verification

Configured sibling build:
`C:\Users\shaug\.codex\visualizations\2026\11\12\01a1cf3e-5728-7633-99ba-7c7ac1473b17\NativeServer-build`

MSVC toolset 14.52.37025, Visual Studio 18 Community, x64 RelWithDebInfo.
Run these PowerShell commands from the repository:

```powershell
$cmakeBin = 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin'
& "$cmakeBin\cmake.exe" --build '..\NativeServer-build' --config RelWithDebInfo --target cdmp_tests_coop3663 pkt3663_ack_trace
if ($LASTEXITCODE -ne 0) { throw 'Build failed' }
& '..\NativeServer-build\bin\RelWithDebInfo\cdmp_tests_coop3663.exe'
if ($LASTEXITCODE -ne 0) { throw 'Focused tests failed' }
& "$cmakeBin\ctest.exe" --test-dir '..\NativeServer-build' -C RelWithDebInfo --output-on-failure
if ($LASTEXITCODE -ne 0) { throw 'Suite failed' }
```

Repo scripts/build.ps1 creates a different build directory; prior work reused
the configured sibling tree and recorded this deviation to avoid duplicating
dependencies. Do not silently switch toolchains or reuse obsolete review harnesses.
Some tests use dummy fixture processes; that is not game execution. Verify game
absence with an actual host process listing before the full suite; sandboxed
process views previously gave misleading absence results.

Latest completed validation: 101/101 passed, 90.26 seconds. Focused parser tests
passed; git diff --check passed. Log is sibling
`Coop3663-review/component-readers-suite.log`. No live runtime validation.

## Return handoff

Update HANDOFF-CURRENT.md with changes, evidence paths/hashes, tests and exact
results, unresolved dependencies, and the next useful milestone. Clearly label
file evidence, synthetic fixtures, and any unverified inference. Keep changes
local unless the user requests a commit/push. Give Codex the checkout, branch,
changed files and handoff path for final review.
