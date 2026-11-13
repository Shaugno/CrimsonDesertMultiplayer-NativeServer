# Live packet capture on Crimson Desert 1.0.0.3576

Controlled passive capture with the production controller from commit
`03ce15ecca76c7fc54b17bc04f77d9f3cb8c5828`. This file records what was
observed. It does not adopt offsets, open the ASI guard, or claim co-op works.
No packets were sent, modified, or replayed.

Both captures were armed and stopped cleanly in PID 39068. The production DLL
remains mapped. The process was still alive after the second stop. No co-op
session was attempted.

## Save backup

Crimson Desert was not running when the copy started (2026-11-12 21:53:21
-05:00). The source was not restored, deleted, or edited.

Verified copy:

`C:\Users\shaug\OneDrive\Documents\ChatGPT\COOP MOD\backups\NativeServer\20261112-215320-cd-save-before-pkt3576-live\save`

Manifest, 23 files, SHA-256 of every file matched the source, result PASS:

`C:\Users\shaug\OneDrive\Documents\ChatGPT\COOP MOD\backups\NativeServer\20261112-215320-cd-save-before-pkt3576-live\MANIFEST.sha256.txt`

An earlier copy at `20261112-215240-cd-save-before-pkt3576-live` nested an
extra `save` directory. Its manifest result is FAIL. It is not the backup.

## Binaries

Production controller and DLL only. Sources for the controller, runner,
identity, capture, and `CMakeLists.txt` match commit
`03ce15ecca76c7fc54b17bc04f77d9f3cb8c5828`. Fixture binaries were not used.

| File | SHA-256 | Bytes | Local mtime |
| --- | --- | --- | --- |
| `build\bin\RelWithDebInfo\cdmp_pkt3576_live_ctl.exe` | `D86E83D960CD4DE6B57551BEA3727D583F33B9F2B5427DFAA96895B0B92689AB` | 775680 | 2026-11-12 20:32:28 |
| `build\bin\RelWithDebInfo\cdmp_pkt3576_live.dll` | `F93B7089C5B0A4722E183F5D69D8F83512299D49DC631A91101A205B9E82C959` | 1054208 | 2026-11-12 20:03:15 |

Production exports: `pkt3576_live_report`, `pkt3576_live_start`,
`pkt3576_live_status`, `pkt3576_live_stop`. No fixture export.

`bin64\cdmp.asi` and `bin64\winmm.dll` were absent. The on-disk game file is
`D:\SteamLibrary\steamapps\common\Crimson Desert\bin64\CrimsonDesert.exe`,
326,872,984 bytes, SHA-256
`C94AD6C3EA54970A6D2DA7A2719753210A7728A91698ED3405DD9125470ED98F`,
`e_lfanew` `0x80`, PE32+ magic `0x20B`, timestamp `0x6AED2E85`,
`SizeOfImage` `0x13CC4000`.

## Launch and identity

Steam launch requested 2026-11-12 21:55:31 -05:00 with
`steam://rungameid/3321460`. One process:

- PID `39068`
- Path `D:\SteamLibrary\steamapps\common\Crimson Desert\bin64\CrimsonDesert.exe`
- Created 2026-11-12 21:55:33 -05:00

Read-only `status` at 2026-11-12 21:56:04 -05:00, before any load. Raw stdout
is in
`C:\Users\shaug\OneDrive\Documents\ChatGPT\COOP MOD\backups\NativeServer\20261112-215320-pkt3576-live-evidence\00-status-before-load.txt`.
Process exit code 0.

```
ctl_action=status
ctl_pid=39068
ctl_identity=match
ctl_rejected_2976=0
ctl_gate=match
ctl_modified=0
ctl_loaded=0
ctl_start=-1
ctl_stop=-1
ctl_retained=0
ctl_stored=0
ctl_omitted=0
ctl_capture_attempts=0
ctl_unloaded=0
ctl_target_alive=1
ctl_note=not-loaded
ctl_exit=0
```

`ctl_gate=match` means the pristine entry signatures were present. Nothing
was allocated, written, or threaded in the game (`ctl_modified=0`).

## Standing capture

The operator confirmed the character was standing still. The same PID was
still `CrimsonDesert.exe`, created 2026-11-12 21:55:33 -05:00. Log directory,
outside the game install:

`C:\Users\shaug\OneDrive\Documents\ChatGPT\COOP MOD\backups\NativeServer\20261112-215320-pkt3576-live-evidence\bound-log`

The armed window was 2026-11-12 21:58:45.416 through 21:58:50.428 -05:00.
Raw controller stdout is in the evidence directory. Process exit codes were 0.

Load at 21:58:45.095:

```
ctl_action=load
ctl_pid=39068
ctl_identity=match
ctl_rejected_2976=0
ctl_gate=match
ctl_modified=1
ctl_loaded=1
ctl_start=-1
ctl_stop=-1
ctl_retained=0
ctl_stored=0
ctl_omitted=0
ctl_capture_attempts=0
ctl_unloaded=0
ctl_target_alive=1
ctl_note=loaded
ctl_dll_base=0x7ff8fd2e0000
ctl_exit=0
```

Start at 21:58:45.271, while the entries were still pristine:

```
ctl_action=start
ctl_pid=39068
ctl_identity=match
ctl_rejected_2976=0
ctl_gate=match
ctl_modified=1
ctl_loaded=1
ctl_start=0
ctl_stop=-1
ctl_retained=0
ctl_stored=0
ctl_omitted=0
ctl_capture_attempts=1
ctl_unloaded=0
ctl_target_alive=1
ctl_note=started
ctl_dll_base=0x7ff8fd2e0000
ctl_exit=0
```

Status at 21:58:50.430, after the hooks were installed:

```
ctl_action=status
ctl_pid=39068
ctl_identity=signature-mismatch
ctl_rejected_2976=0
ctl_gate=installed
ctl_modified=1
ctl_loaded=1
ctl_start=0
ctl_stop=-1
ctl_retained=0
ctl_stored=0
ctl_omitted=0
ctl_capture_attempts=1
ctl_unloaded=0
ctl_target_alive=1
ctl_note=ok
ctl_dll_base=0x7ff8fd2e0000
ctl_exit=0
```

Stop at 21:58:50.530. One stop. It was not repeated.

```
ctl_action=stop
ctl_pid=39068
ctl_identity=signature-mismatch
ctl_rejected_2976=0
ctl_gate=installed
ctl_modified=1
ctl_loaded=1
ctl_start=0
ctl_stop=4
ctl_retained=0
ctl_stored=6
ctl_omitted=0
ctl_capture_attempts=1
ctl_unloaded=0
ctl_target_alive=1
ctl_note=removed
ctl_dll_base=0x7ff8fd2e0000
ctl_exit=0
```

`ctl_stop=4` and `ctl_exit=0`. The process was still `CrimsonDesert` after
the controller returned. `ctl_unloaded=0` and `ctl_loaded=1`: the DLL stayed
mapped at `0x7ff8fd2e0000`. `ctl_retained=0`. No second stop was issued.

The session log from this phase was copied before any later start. Archive:

`C:\Users\shaug\OneDrive\Documents\ChatGPT\COOP MOD\backups\NativeServer\20261112-215320-pkt3576-live-evidence\phase1-standing`

- `pkt3576-live-session.txt`, 585 bytes, SHA-256 `AC261CF94E04BC194707C7481F2E1366C837CB2D2B089A820283AA0BD3601CF9`
- `pkt3576-live-records.txt`, 659 bytes, SHA-256 `5677EE6B66D58377FD47A18D5E149AE15BEFC8BEED358A71891DCFC338843028`

The module log says `fixture=0`, `game_match=1`, `kind=match`, base
`0x140000000`, timestamp `6AED2E85`, size `13CC4000`, `limits=64/64`,
`capture_attempts=1`. The startup block has `stored=0`. The shutdown block
has `shutdown=removed`, `stored=6`, `omitted=0`, `dropped=0`, `unread=0`.
The status sample immediately before stop still reported `ctl_stored=0`.
The stop report and the records file agree on 6 stored and 0 omitted.

The records file has six `R` lines and no `S` lines, then
`counters dropped=0 unread=0 stored=6 omitted=0`. These fields are raw
register and stack values plus a copied prefix. They are not decoded.

| Line | rcx | rdx | r8 | r9 | u16 at [rsp+28h] | prefix bytes |
| --- | --- | --- | --- | --- | --- | --- |
| 1 | `00000552CA0DE600` | `00000552F7F77600` | `7FFFFFFFFFFFFFFC` | `00000553BF8FC033` | `000A` | `E008FF050004010010A0` |
| 2 | `00000552CA0DE600` | `00000552F7F77600` | `7FFFFFFFFFFFFFFC` | `00000553BF924033` | `0DCC` | `3809FFC70D04010010A000003B460F00` |
| 3 | `00000552CA0DE600` | `00000552F7F77600` | `7FFFFFFFFFFFFFFC` | `00000553BF924033` | `000A` | `E008FF050004010010A0` |
| 4 | `00000552CA0DE600` | `00000552F7F77600` | `7FFFFFFFFFFFFFFC` | `00000553BF91A033` | `0DCC` | `3809FFC70D04010010A000003B460F00` |
| 5 | `00000552CA0DE600` | `00000552F7F77600` | `7FFFFFFFFFFFFFFC` | `00000553BF910033` | `0019` | `DB0AFF14000000000090120200000000` |
| 6 | `00000552CA0DE600` | `00000552F7F77600` | `7FFFFFFFFFFFFFFC` | `00000553BF8E8033` | `000A` | `3E09FF050004010010A0` |

`rcx`, `rdx`, and `r8` are the same on every standing line. `r9` and the
copied prefix differ. No send-direction line was stored. Nothing was omitted.
The 64-record cap was not reached.

## Walking capture

The operator confirmed the character was walking in the same process. Before
arming, status showed the entries were pristine again: `ctl_identity=match`,
`ctl_gate=match`, `ctl_loaded=1`, `ctl_exit=0`. That status was at
2026-11-12 22:00:58.394 -05:00. The previous stop's report was still visible
(`ctl_stop=4`, `ctl_stored=6`).

Start reused the bound directory and did not load a second DLL. It returned
at 22:00:58.539:

```
ctl_action=start
ctl_pid=39068
ctl_identity=match
ctl_rejected_2976=0
ctl_gate=match
ctl_modified=1
ctl_loaded=1
ctl_start=0
ctl_stop=-1
ctl_retained=0
ctl_stored=0
ctl_omitted=0
ctl_capture_attempts=2
ctl_unloaded=0
ctl_target_alive=1
ctl_note=started
ctl_dll_base=0x7ff8fd2e0000
ctl_exit=0
```

The armed window was 22:00:58.668 through 22:01:03.675 -05:00. Status at
22:01:03.677:

```
ctl_action=status
ctl_pid=39068
ctl_identity=signature-mismatch
ctl_rejected_2976=0
ctl_gate=installed
ctl_modified=1
ctl_loaded=1
ctl_start=0
ctl_stop=-1
ctl_retained=0
ctl_stored=0
ctl_omitted=0
ctl_capture_attempts=2
ctl_unloaded=0
ctl_target_alive=1
ctl_note=ok
ctl_dll_base=0x7ff8fd2e0000
ctl_exit=0
```

One stop at 22:01:03.788:

```
ctl_action=stop
ctl_pid=39068
ctl_identity=signature-mismatch
ctl_rejected_2976=0
ctl_gate=installed
ctl_modified=1
ctl_loaded=1
ctl_start=0
ctl_stop=4
ctl_retained=0
ctl_stored=64
ctl_omitted=3422
ctl_capture_attempts=2
ctl_unloaded=0
ctl_target_alive=1
ctl_note=removed
ctl_dll_base=0x7ff8fd2e0000
ctl_exit=0
```

`ctl_capture_attempts=2`. `ctl_stop=4` and `ctl_exit=0`. The process was still
`CrimsonDesert` afterward. The DLL stayed at `0x7ff8fd2e0000`. Stop was not
repeated. Nothing was force-unloaded.

The walking files were copied to
`C:\Users\shaug\OneDrive\Documents\ChatGPT\COOP MOD\backups\NativeServer\20261112-215320-pkt3576-live-evidence\phase2-walking`.
The standing archive hashes were unchanged after that copy.

- `pkt3576-live-session.txt`, 1161 bytes, SHA-256 `1AB6AA35EB91271B6EE9EC717662BD505A31230FFE84A2A2ED22358AE39AA55F`
- `pkt3576-live-records.txt`, 7279 bytes, SHA-256 `532A32A9EF4AE0D178D54BBE9ACE5FBC46B59563BAB26ECD2C4B1982097EA7D0`

The phase-2 session file contains both the standing blocks and the walking
blocks, because the module keeps one session buffer for the bound directory
and rewrites that file. The phase-1 archive remains the standing-only copy.
The walking records file contains only the second capture: 64 stored lines
and `counters dropped=0 unread=0 stored=64 omitted=3422`. The module log for
the second shutdown says `stored=64`, `omitted=3422`, `dropped=0`,
`unread=0`, `capture_attempts=2`, `limits=64/64`. As in the standing capture,
status immediately before stop still reported `ctl_stored=0` and
`ctl_omitted=0`. The stop report matches the records file.

Of the 64 stored walking lines, 50 begin with `S` and 14 begin with `R`.
Every stored line in both captures uses `rcx=00000552CA0DE600`. Every stored
`R` line in both captures uses `rdx=00000552F7F77600` and
`r8=7FFFFFFFFFFFFFFC`. Every stored `S` line uses `r8=0000000000000004`.
The 50 send lines use 18 distinct `rdx` values. Send lines also carry the
two extra raw stack fields; receive lines do not.

Stored receive `u16` values at `[rsp+28h]`:

- Standing: `000A` three times, `0DCC` twice, `0019` once.
- Walking, among the 14 stored receive lines: `007B` fourteen times.

Stored send `u16` values at `[rsp+28h]`, walking only: `007D` fifteen,
`020B` fifteen, `0009` six, `02DE` six, `00FB` three, and one each of
`02A5`, `003C`, `00F9`, `00B5`, and `00A6`.

## Comparison and limits

The standing window stored 6 lines and omitted none. The walking window, of
about the same length, stored the 64-line cap and omitted 3422. The omitted
lines were not written, so the walking file is the first 64 stored
observations, not the whole interval. Both files report `dropped=0` and
`unread=0`.

Standing stored no `S` lines. Walking stored both `S` and `R` lines. The
receive `u16` values stored while standing do not appear among the 14 receive
lines stored while walking. That does not say those values were absent from
the 3422 omitted observations.

No field here is decoded. Repeated prefixes and repeated stack values are
raw observations only. `game_match=1` in the session log means the production
module accepted this process identity. It does not mean co-op works.

`bin64\cdmp.asi` and `bin64\winmm.dll` were still absent after both stops.
The ASI guard was not opened. Offsets were not changed. No packet was sent,
modified, or replayed. The DLL pin remains until PID 39068 exits on its own.
Raw controller stdout is under
`C:\Users\shaug\OneDrive\Documents\ChatGPT\COOP MOD\backups\NativeServer\20261112-215320-pkt3576-live-evidence`.

