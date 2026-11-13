# Compatibility assessment: Crimson Desert 1.0.0.3663

Date of this file: 2026-11-13.
Checkout: `CrimsonDesertMultiplayer-NativeServer` on `grok/compat-guard`.

This note records the installed executable and the rejected 3576 controller
status. It does not adopt addresses, open the 3576 gate, install hooks, or
deploy the mod. The 3576 collector still accepts only timestamp `0x6AED2E85`
and `SizeOfImage` `0x13CC4000`.

## Result

Steam updated the installation from 1.0.0.3576 to 1.0.0.3663 before the Phase 1
stand, walk, and turn capture. The capture was aborted. Codex verified that
abort. No packet archive was written.

| Field | Installed file and PID 35896 |
| --- | --- |
| Path | `D:\SteamLibrary\steamapps\common\Crimson Desert\bin64\CrimsonDesert.exe` |
| File version resource | 1.0.0.3663 |
| Product version resource | 1.0.0.3663 |
| Mapped base | `0x140000000` |
| PE timestamp | `0x6AF5D198` |
| SizeOfImage | `0x144E9000` |
| File bytes | 335440280 |
| SHA-256 | `8BE67178D9B9A149C361A731051665E0BA4D441E250AE1DFE645BD6E524345C8` |
| File mtime | 2026-11-13 05:48:42 -05:00 |

The mapped header matched the file. `bin64\cdmp.asi` and `bin64\winmm.dll` were
absent. PID 35896 was started 2026-11-13 10:57:06 -05:00 and was still alive
after status. The process was not terminated.

Read-only controller status, with no `load`:

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

The offline file scan of this same executable is
`docs/probe-3663/PACKET-SCAN-3663.md`. Nothing in that scan is an adopted
offset.
