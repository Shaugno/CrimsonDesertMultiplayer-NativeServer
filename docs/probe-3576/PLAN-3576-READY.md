# Plan: ready NativeServer for Crimson Desert 1.0.0.3576

Date: 2026-11-13. Codex reviewed this plan and the Phase 4 identity gate on
2026-11-13. No blocking findings. A 3576 identity cannot reach a 2976 RVA,
2976 startup behavior is unchanged, and unproved packet fields stay unresolved.
Codex's complete game-free suite passed 93/93. The earlier implementation run
of the same suite passed 93/93 in 71.50 seconds. Collector AddressSanitizer
was not re-run for this gate.

Published collector commit `26a0e1a` stays as it is. Codex has not re-reviewed
that collector commit. This approval covers the plan and the identity gate.
It does not authorize a live capture, packet replay, offset adoption, or
installing `cdmp.asi`. Phase 1 is prepared and not started. It runs only in a
session the user starts from Steam, after Codex verifies and publishes the
local commit.

"Ready" means a separate 3576 path that can show two player actors moving from
verified packets. It does not mean combat, saves, reconnects, NPCs, or two-PC
play. Co-op is not claimed until those later checks exist.

## Stay closed

- Do not write 3576 addresses into `offsets.h` or `offsets_s2.h`.
- Do not call `sniffer_install` against 3576. Its gate is still the 2976
  timestamp and it has no uninstall.
- Do not install `cdmp.asi` or `winmm.dll` into the game `bin64`.
- Do not replay or inject packets. Prefix-only archives, including the PID
  39068 capture, are not replay input.
- Do not raise the 4096-byte copy cap. `kActorLoadDataAckMax` stays 0 until a
  complete sample exists. A truncated actor-load record is not a spawn packet.
- Keep the 2976 path working. A 3576 identity must not fall through into 2976
  reads.
- Do not edit the sibling `CrimsonDesertMultiplayer` tree.
- Do not rewrite the historical limits in `LIVE-CAPTURE-3576.md` or the numbered
  proposal items in `PACKET-MAP-3576.md`.

## Already known

- Installed executable identity, recorded in `docs/COMPATIBILITY-3576.md`:
  timestamp `0x6AED2E85`, base `0x140000000`, `SizeOfImage` `0x13CC4000`.
- Nine message names and opcodes in `docs/probe-3576/PACKET-MAP-3576.md`:
  MoveActorReq 2833, RotateActorReq 2852, StartActionReq 2202, MoveActorAck
  3094, RotateActorAck 3078, StartActionAck 2552, ActorLoadDataAck 2005,
  UpdateVisiblePlayerAck 2555, RemoveActorAck 3067.
- Send candidate `exe+0x3A6310` and receive candidate `exe+0x3A66A0`. They are
  not adopted. They stay in `tools/pkt3576/candidates.h`.
- The collector at `26a0e1a` copies those nine messages, checks the copied
  header, and creates labeled archives exclusively. Game-free result then:
  93/93 in 53.29 seconds, collector AddressSanitizer exit 0. It has not been
  run against the game.

## Still unknown

No message has a proven payload layout. Coordinates, actor identity, and action
timing are not assigned. The six acknowledgement opcodes were not in the stored
capture. Spawn, visibility, and removal need a later load or actor-creation
observation. Chunk origin and a world position are not adopted.

## Phase 1 — Capture complete packets

Prepared and not started. Operator session. The user launches the game from
Steam, not from `scripts/launch-2inst.ps1`. This approval does not run the
capture.

1. Back up the saves with the existing save-backup procedure. Do not restore
   or edit saves.
2. Confirm `cdmp.asi` and `winmm.dll` are absent from `bin64`. Confirm the
   running process is `CrimsonDesert.exe` with timestamp `0x6AED2E85`, base
   `0x140000000`, and `SizeOfImage` `0x13CC4000`.
3. Load `cdmp_pkt3576_live.dll` with `cdmp_pkt3576_live_ctl.exe` using an
   explicit PID and a log directory outside `bin64`. Commands are in
   `docs/probe-3576/LIVE-CAPTURE-CONTROL.md`.
4. Mark and arm three windows: stand, walk, turn. Each labeled file is
   exclusive. `Retained` means stop testing: no force-unload, no
   `TerminateProcess`, no retry.
5. Archive the records. Report opcode, claimed length, copied length, and
   complete versus truncated versus malformed.

Exit: complete copies of MoveActorReq, RotateActorReq, and StartActionReq for
each action, with QPC timestamps. A dropped or truncated row is reported and is
not the layout.

## Phase 2 — Name the movement fields

Not started. Offline. No game process.

Compare the Phase 1 stand, walk, and turn records. A field that changes only
while walking is a movement candidate. A field that changes only while turning
is a rotation candidate. Constant fields are not coordinates. Record each
accepted field as offset, width, and endianness in a new
`docs/probe-3576/PACKET-FIELDS-3576.md`. Leave a field unassigned when two
records disagree. Do not copy the 2976 sizes from
`docs/research/mirror-ack-formats.md`.

Exit: a written field table for the three request messages, each row tied to a
captured byte range. Messages that stayed truncated stay unresolved.

## Phase 3 — Capture spawn, visibility, and removal

Not started. A separate live session after Phase 2. Same controller, identity,
and backup rules. The allow-list stays the same nine opcodes. One window while
a remote or second actor becomes visible, and one window while that actor
leaves. If ActorLoadDataAck claims more than 4096 bytes, keep the truncated
record and do not raise the cap. Note the claimed length.

Exit: at least one complete UpdateVisiblePlayerAck and one complete
RemoveActorAck, plus either a complete ActorLoadDataAck or a written statement
that the claim exceeds 4096 and the layout is still unknown.

## Phase 4 — 3576 identity beside 2976

Codex approved this gate on 2026-11-13. No blocking findings. It is in the
local commit with this plan and is not installed. Game-free only. Gameplay
reads, hooks, networking, and the player feed stay off. Packet RVAs are
absent from `src/mod/game/build_3576.h` until Phase 1 shows the 16-byte
entries still match.

The approved wording said `decide_startup` would enable game reads, hooks, and
networking for 3576 through a new path. The working tree does not do that.
There is no 3576 hook callback. Turning on the existing callbacks would call
2976 code, including `build_2976::kWorldSystemSlot` in `late_init` and the
guest save hooks. The new path is the identity tag. Reads, hooks, networking,
and the player feed stay off. A 3576 guest still sets `end_guest_process` so
the 2976 save redirect is not installed.

Files in this commit, on top of `26a0e1a`:

- `src/mod/game/build_3576.h` — timestamp `0x6AED2E85`, base `0x140000000`,
  `SizeOfImage` `0x13CC4000`. No gameplay address.
- `src/mod/game/build_guard.h` — `IdentityKind::Build3576` requires base,
  timestamp, and `SizeOfImage`. A 3576 timestamp with another image size stays
  `Unsupported`. Timestamp `0x6AB28F00` stays the 2976 path. Identity
  classification does not read packet entry bytes.
- `src/mod/mod.cpp` — `late_init` returns before any 2976 address when the
  identity is `Build3576`.
- `tests/build_guard_tests.cpp` — `test_build_3576_stays_off_the_2976_path`.

`scripts/build.ps1` after the change passed 93/93 in 71.50 seconds. Codex then
ran the complete game-free suite and it passed 93/93. Collector
AddressSanitizer was not re-run for this guard change. The rebuilt
`build\bin\RelWithDebInfo\cdmp.asi` was not copied into the game.

Exit met for the identity gate: a 3576 identity cannot reach a 2976 RVA, and
2976 startup is unchanged. Publishing the local commit is Codex's step.
Gameplay permissions stay disabled.

## Phase 5 — Spawn and move a second actor

Not started. First gameplay milestone. It waits for written field tables from
Phases 2 and 3, and for the Phase 4 path. The user starts the session. Saves
are backed up first.

On the host, read the local player's verified movement and rotation fields.
Send them through the existing NativeServer session with new 3576 encode and
decode functions. Do not reuse the 2976 packet sizes. On the guest, apply an
update only through the game path Phase 3 showed creates or updates a visible
actor. If that path is still unknown, stop and record the gap. Do not invent a
spawn call.

Success is two visible player actors whose positions follow the captured walk
and turn. Combat, inventory, saves, and NPC simulation stay out.

## Phase 6 — Later

Loading, reconnects, combat, save handling, and two-PC play. Each one needs
its own evidence pass. The 2976 path remains available for the old build.

## Order and validation

Phase 1, then 2, then 3. The identity half of Phase 4 is already written
because it stores file facts that were already verified. It does not install
the mod. Phase 5 waits for Phases 2, 3, and 4.

Phases 1 and 3 are live and use the controller identity gate. A mismatch does
not load the DLL. Phases 2 and 4 are game-free. Phase 5 is a manual in-game
check. AddressSanitizer of the collector stays limited to
`cdmp_tests_pkt3576`, `cdmp_tests_pkt3576_capture`, `cdmp_tests_pkt3576_live`,
and `cdmp_tests_pkt3576_live_host`. Do not sanitize the fault-injection
executables.

## Review result

Codex reviewed this plan and the Phase 4 files on 2026-11-13. No blocking
findings. A 3576 identity cannot reach a 2976 RVA. 2976 startup behavior is
unchanged. Unproved packet fields stay unresolved. The complete game-free
suite passed 93/93. The game stayed closed. Phase 1 remains the next session,
and only when the user has launched the game from Steam.
