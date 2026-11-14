# Handoff: Package A incomplete

Current direction after this evidence review:
[Co-op prototype plan](../COOP-3663-PROTOTYPE-PLAN.md). Repeated captures are
paused while the independent state/transport/actor-path work proceeds.
The experiment below was proposed but not performed; it is not the immediate
next assignment. The archived observations and hashes remain unchanged.

Date: 2026-11-14. Checkout `grok/compat-guard` at published commit `3e950a7`
(`Gate capture on durable readiness and preserve attempt logs`). Newer local
files kept: `docs/PROGRESS.txt`, `docs/ROADMAP.md`, and
`docs/GROK-3663-EXECUTION-PLAN.md`. This handoff adds no runtime change.
The reviewed diagnostic, compatibility guards, production offsets, and the
4096-byte copy cap are unchanged. No actor was spawned and no packet was
replayed.

Packages A and B are not complete. There is no usable post-ready movement
evidence, so offline field analysis was not started.

## This attempt

Directory, left unchanged:

`C:\Users\shaug\.codex\visualizations\2026\11\12\01a1cf3e-5728-7633-99ba-7c7ac1473b17\NativeServer-work2\backups\pkt3663-live-20261114-postready`

PID 51032 was
`D:\SteamLibrary\steamapps\common\Crimson Desert\bin64\CrimsonDesert.exe`,
started 2026-11-14 11:25:31 -05:00 by `steam.exe` with
`PlatformServiceType=Steam`. Status before load was `identity=match`,
`loaded=0`, `modified=0`. Load used
`cdmp_pkt3663_live.dll`
`D44705E057693B20B505F69335DE8C8980243AAF1EA899484D0A3E65D90FD18F`
at `0x7ff940090000`. The operator was standing before the arm.

| Command | Result |
| --- | --- |
| `start` stand | exit 0, `ctl_start=0`, `note=started`, remote wait 18404 ms, `stored=0` |
| `stop` stand | exit 0, `ctl_stop=4`, `note=removed`, `retained=0`, `target_alive=1`, `stored=0` |

Start returned at 11:29:34.655. Stop was issued at 11:29:34.679, about 24 ms
later. That is the gap between the two controller commands. Readiness was
already durable inside `start` before that return, so the archive does not
give a separate ready-to-stop duration. `ctl_identity=anchor-mismatch` and
`ctl_gate=installed` on stop match the documented state after this
diagnostic's own hooks are installed. They are not a wrong-build acceptance.

`pkt3663-live-session-1.txt` is a complete non-fixture 3663 session:
timestamp `6AF5D198`, base `140000000`, size `144E9000`, `kind=match`,
`recording_ready=1`, `capture_ready_qpc=1785584313182`,
`qpc_frequency=10000000`, `session_history_truncated=0`,
`session_write_failed=0`, `startup=started`, `shutdown=removed`. Durable
readiness is 18.384489 seconds after export entry. Final counters are
`stored=0`, `omitted=0`, `dropped=0`, `unread=0`, `truncated=0`,
`pre_ready=88`. The records file is 12 lines: the 11-line header and that
same counter trailer. It has no data row. Walk and turn files were not
created. There is no attempt 2.

The 88 pre-ready counts happened before admission. They have no direction,
length, or bytes. None of them is a post-ready sample.

SHA-256, matching the review and recomputed without editing the files:

```text
commands.txt
646F165AAA2852810E757E80AD112A2AD53D90CFEC9FB01B275A454B8E307B45
pkt3663-live-records-stand.txt
1F0FD3ECA497E4BDAE06BD31B5C998E4C6D1661D905B7C94CE4BFEF5D923B799
pkt3663-live-session-1.txt
7CA03FBC49EBFC42E373482C457DB49BDB9C60985D95D349E8D839239F6D7B6C
pkt3663-live-session.txt
7CA03FBC49EBFC42E373482C457DB49BDB9C60985D95D349E8D839239F6D7B6C
prep.txt
64F06B1A0A60C5A10BCB7558472DADE1CF6605E88D1CEC84894CACA77E55F42B
verifier.txt
3C2CC3F401F87DB1DF4590F402DC4ED91E7243C66794B0C7B6846B1B203906EC
```

## Why this is not a movement sample

The accepted window is empty. A phase label and a ready session do not
identify a coordinate, heading, actor, or opcode. Package B does not start
on this archive.

The empty window does not show that the hooked entries ignore local
movement. Two existing observations point the other way, without proving a
layout:

- This process called the hooked entries 88 times before the cutoff while
  the operator was standing. Those calls were counted and discarded.
- The 2026-11-13 stand archive, under the older diagnostic, retained 249
  post-ready send rows of 181 bytes (`raw_word0=08F8`) during a stand. That
  file is a different build of the diagnostic and is not reused as a field
  table.

The static verifier finds request descriptors for `MoveActorReq` and
`RotateActorReq` constructed before calls to the send candidate. That is a
file correlation. It is not a live message ID, and it is not evidence from
the empty post-ready file.

No repeated capture was run.

## Next experiment, not started

Use the same reviewed DLL and controller. Do not change guards or offsets.
Use a new process and a new directory so this archive stays intact. The
loaded module from PID 51032 must not be reused for another attempt.

After an observed `ctl_start=0` and a durable ready block:

1. Leave the game in the foreground.
2. Log the wall-clock time of the successful start return.
3. Keep the requested character action going for 15 seconds. Stand means
   still, with no character turn and no camera turn. Walk means the
   character walks in a straight line. Turn means the character's body
   rotates in place.
4. Log the stop time, then use the reviewed stop. Do not unload if
   ownership is uncertain. Do not retry a timeout, Retained, or failed log.

Audit that window before any later phase: nonempty rows at or after
`capture_ready_qpc`, copied length consistent with the hex, `raw_word0`
matching the first little-endian word, the phase label matching the file,
and a numbered session with `session_history_truncated=0` and
`session_write_failed=0`. If that 15-second window is still empty, stop.
The next step is a reviewed diagnostic or read-only player-state
investigation, not another blind capture.

## Tests

No suite was rerun for this handoff. The last full suite cited by the
admission review is historical and is not a new result.
