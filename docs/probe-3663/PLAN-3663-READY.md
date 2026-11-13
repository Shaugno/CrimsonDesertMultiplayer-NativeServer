# Plan: assess Crimson Desert 1.0.0.3663

Date: 2026-11-13. This plan is for Codex to review. It does not authorize a
live capture, a load of `cdmp_pkt3576_live.dll`, or an ASI install.

The installed game is no longer 1.0.0.3576. Steam updated
`CrimsonDesert.exe` to 1.0.0.3663. The 3576 controller rejected that process.
The abort is `docs/probe-3576/PHASE1-CAPTURE-20261113.md`. The file facts are
`docs/COMPATIBILITY-3663.md`.

"Ready" still means a separate path that can show two player actors from
verified packets on the installed build. This document does not reach that
result. Co-op is not claimed.

## Stay closed

- Do not change the 3576 timestamp, image size, opcode list, or entry samples.
- Do not broaden `tools/pkt3576` so that 3663 passes its gate.
- Do not load `cdmp_pkt3576_live.dll` into the 3663 process.
- Do not reuse a 3576 RVA, opcode, payload size, or entry sample as a 3663 fact.
- Do not write 3663 addresses into `offsets.h`, `offsets_s2.h`, or
  `build_3576.h`.
- Do not change the normal ASI guard or the live 3576 controller's acceptance.
- Do not install `cdmp.asi` or `winmm.dll`.
- Do not replay or inject packets, and do not raise the 4096-byte copy cap.
- Do not attach to the game for the assessment in this commit.

## What this commit does

`tools/pkt3663` reads an executable only as a data file. A 2976 or 3576
timestamp is classified from the PE header and is not scanned. A 3663 file is
accepted only when the timestamp is `0x6AF5D198`, the base is `0x140000000`,
and `SizeOfImage` is `0x144E9000`. The scanner then looks up the nine message
names through MSVC RTTI and reports the instance fields it reads. `mov ecx,
8030h` and the other `8030h` encodings are reported separately from the old
16-byte samples. A sample hit is not an entry.

Game-free coverage is `tests/pkt3663_tests.cpp`. The fixture opcodes are not
the numbers read from the installed file. The installed-file result is
`docs/probe-3663/PACKET-SCAN-3663.md`. `adopted=0` in that run.

## Later, only after review

1. Codex reviews the scan. A partial type name is not the message it contains.
   `TrocTrManagementRemoveActorAck` is not `TrocTrRemoveActorAck`.
2. If the review accepts the exact names, a new collector can be written under
   `tools/pkt3663`. It must not be the 3576 DLL with a wider timestamp check.
   Its allow-list is the opcodes re-read from this file, and only after a live
   capture shows those values on the wire. The file read is not that capture.
3. The user launches from Steam. Saves are backed up first. `cdmp.asi` and
   `winmm.dll` stay absent. Identity is checked before `load`. `Retained`
   stops the session.
4. Stand, walk, and turn are separate labeled windows. Field names come from
   those records. No field is a coordinate until the records show it.
5. Spawn and a second visible actor wait on that evidence. Do not invent a
   spawn call.

Loading, reconnects, combat, saves, and two-PC play stay later.
