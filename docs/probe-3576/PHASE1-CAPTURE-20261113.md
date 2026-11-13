# Phase 1 capture, 2026-11-13

## Result

Aborted identity mismatch. Codex verified this evidence. The stand, walk, and
turn windows were not captured. Steam had updated the installed executable to
Crimson Desert 1.0.0.3663. The 3576 controller rejected PID 35896 before
`load`. The 3576 gate was not broadened, and its constants were not changed.
No labeled record file exists. Phase 2 does not start from this session.

The stand, walk, and turn capture did not start. The running game is not
Crimson Desert 1.0.0.3576. The controller's read-only status rejected it.
Nothing was loaded, hooked, or written into the process. No packet records
were stored. This file does not name coordinates, rotations, or field layouts.

Commit under test: `58eae2476c65c6f312567e816fc9564b5e87176c`.

## Save backup

Crimson Desert was not running when the copy started. The source was not
restored, deleted, or edited. Result PASS, 23 files, `mismatches=0`.

`C:\Users\shaug\OneDrive\Documents\ChatGPT\COOP MOD\backups\NativeServer\20261113-104102-cd-save-before-pkt3576-phase1`

Manifest:

`C:\Users\shaug\OneDrive\Documents\ChatGPT\COOP MOD\backups\NativeServer\20261113-104102-cd-save-before-pkt3576-phase1\MANIFEST.sha256.txt`

## Installed executable

Path: `D:\SteamLibrary\steamapps\common\Crimson Desert\bin64\CrimsonDesert.exe`.

The file version resource reads 1.0.0.3663. The collector accepts only the
1.0.0.3576 image. These 3663 values are not adopted.

| Field | Running process and file | Required for this capture |
| --- | --- | --- |
| File version resource | 1.0.0.3663 | 1.0.0.3576 |
| Mapped base | `0x140000000` | `0x140000000` |
| PE timestamp | `0x6AF5D198` | `0x6AED2E85` |
| SizeOfImage | `0x144E9000` | `0x13CC4000` |
| File bytes | 335440280 | 326872984 |
| SHA-256 | `8BE67178D9B9A149C361A731051665E0BA4D441E250AE1DFE645BD6E524345C8` | `C94AD6C3EA54970A6D2DA7A2719753210A7728A91698ED3405DD9125470ED98F` |

File mtime: 2026-11-13 05:48:42 -05:00. The mapped header matched the file.
`bin64\cdmp.asi` and `bin64\winmm.dll` were absent. PID 35896, started
2026-11-13 10:57:06 -05:00, was still alive after status. The process was not
terminated.

## Tools

Built from the existing source. No collector or controller source was edited.

| File | Bytes | SHA-256 |
| --- | ---: | --- |
| `build\bin\RelWithDebInfo\cdmp_pkt3576_live_ctl.exe` | 775680 | `02D9F686B997021BAF140EF0323B68DAC60C637F4D894DB0E68B25A94AF05185` |
| `build\bin\RelWithDebInfo\cdmp_pkt3576_live.dll` | 1059840 | `E5694C6D99FB49A2B1ACFB000520E9D68B223855D62F2046705CEDD160839BD8` |

## Capture directory

`C:\Users\shaug\OneDrive\Documents\ChatGPT\COOP MOD\backups\NativeServer\20261113-104102-pkt3576-phase1-records`

It is outside the game directory. It holds the prep note, the mapped-header
read, and the status transcript. It has no `pkt3576-live-records-stand.txt`,
`pkt3576-live-records-walk.txt`, or `pkt3576-live-records-turn.txt`.

## Commands

`scripts/launch-2inst.ps1` was not used. The user launched the game from Steam.

Read-only status, after the mapped header was read:

```
cdmp_pkt3576_live_ctl.exe status --pid 35896 --dll C:\Users\shaug\OneDrive\Documents\ChatGPT\COOP MOD\CrimsonDesertMultiplayer-NativeServer\build\bin\RelWithDebInfo\cdmp_pkt3576_live.dll
```

```
ctl_action=status
ctl_pid=35896
ctl_identity=wrong-timestamp
ctl_rejected_2976=0
ctl_gate=rejected
ctl_modified=0
ctl_loaded=0
ctl_start=-1
ctl_stop=-1
ctl_retained=0
ctl_stored=0
ctl_omitted=0
ctl_filtered=0
ctl_malformed=0
ctl_truncated=0
ctl_phase=0
ctl_capture_attempts=0
ctl_unloaded=0
ctl_target_alive=1
ctl_note=wrong-timestamp
ctl_exit=4
```

`load`, `mark`, and `start` were not run. No capture was armed. There is no
Retained result.

## Packet counts

No phase was captured. Counts for stand, walk, and turn are zero for every
opcode and for complete, truncated, and malformed. The missing targets are
MoveActorReq, RotateActorReq, and StartActionReq in each action window.
MoveActorAck, RotateActorAck, StartActionAck, ActorLoadDataAck,
UpdateVisiblePlayerAck, and RemoveActorAck were not observed either.

Phase 2 field naming does not start from this session.
