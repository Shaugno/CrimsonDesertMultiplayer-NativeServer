# cdmp_probe commands for the 1.0.0.3576 player-state session

Read-only. The probe is `build\bin\RelWithDebInfo\cdmp_probe.exe` in this checkout. It attaches with `PROCESS_VM_READ | PROCESS_QUERY_INFORMATION` only. Do not copy `cdmp.asi` into the game, do not run `scripts/launch-2inst.ps1`, and do not change `build_guard.h` or `offsets.h`.

Run from PowerShell at the repository root:

```powershell
$probe = Join-Path (Get-Location) 'build\bin\RelWithDebInfo\cdmp_probe.exe'
```

Append every stdout block to `docs/probe-3576/CAPTURE.md` under the heading in `CHECKLIST.md`. A command that prints `Bad address` was not accepted by the probe. Fix the spelling and run it again. Do not treat that line as a game result.

## Address syntax the probe accepts

| Form | Example | Meaning |
|---|---|---|
| `exe+RVA` | `exe+0x5946C40` | Mapped base plus the hex RVA |
| Full hex address | `0x14ABCDEF0` | Absolute address, already summed |
| Chain or watch offset | `0x50` | Hex displacement applied after a pointer read |

`read <physics>+90`, `read 0xPHYS+90`, and `rtti exe+SLOT` are not valid. `rtti` must receive the object address (`0x...`). `chain exe+SLOT` with no offsets is valid: it reads the pointer stored at the slot.

Offsets below are written `0xNN` so they are not read as decimal. `50` would also be hex `0x50`. `--seconds`, `--max`, `--size`, and `--count` are decimal unless written with `0x`.

## Which chain to type

Run `chain exe+SLOT` with no offsets first.

- The `=>` line names `.?AVClientActorManager@pa@@`: the slot is manager-rooted. Later commands use the **Manager** line. Do not pass `0x30`.
- The `=>` line does not name that class, and `chain exe+SLOT 0x30` does: the slot is WorldSystem-rooted. Later commands use the **WorldSystem** line.
- Neither names the manager: the slot is unverified. Do not walk position, rotation, health, or loading from it.

`SLOT` below is the RVA printed by `findptr` (`exe+0x........` of the image hit). Substitute that RVA. Do not substitute `0x6D69190`; that 2976 slot is a verified non-match in this file.

## 1. Identity and mapped base

Title screen is enough. No save loaded.

```powershell
& $probe info
```

Record `Base`, `size`, and the timestamp line. Stop if the timestamp is not `0x6AED2E85` or the text does not say `UNKNOWN BUILD`.

## 2. Vtable candidates

```powershell
& $probe vtables '.?AVClientActorManager@pa@@'
& $probe vtables '.?AVClientChildOnlyInGameActor@pa@@'
& $probe vtables '.?AVClientCharacterControlActorComponent@pa@@'
& $probe vtables '.?AVClientStatusActorComponent@pa@@'
& $probe classes WorldSystem
```

File candidates, still unverified until these lines match them: manager `exe+0x5946C40`, controlled actor `exe+0x5914D58`, character control `exe+0x59459F0`, status `exe+0x591C7C0`. One subobject-0 vtable at that RVA verifies that vtable only. A second vtable stays a candidate.

## 3. Manager object and static slot

Use the vtable RVA from step 2 when it matches. The command below uses the file candidate.

```powershell
& $probe findptr exe+0x5946C40 --heap --max 20
```

For each aligned hit address `0xHIT`:

```powershell
& $probe rtti 0xHIT
```

Pick the hit whose class is `.?AVClientActorManager@pa@@`. Call that address `0xMANAGER` and search the image for slots that store it:

```powershell
& $probe findptr 0xMANAGER --max 20
```

For each image hit, `describe` prints `exe+0xSLOT`. Confirm the root:

```powershell
& $probe chain exe+0xSLOT
& $probe chain exe+0xSLOT
```

The second command is the repeat. Both must name the same class. If the class is the manager, continue with Manager commands. If it is not, try the WorldSystem test once:

```powershell
& $probe chain exe+0xSLOT 0x30
```

Only a `=>` line that names `ClientActorManager` makes this a WorldSystem root.

Controlled actor, after the root is classified. On the title screen this pointer may be null. Null does not verify the actor. Run it again with a character in the world.

```powershell
# Manager root
& $probe chain exe+0xSLOT 0x50

# WorldSystem root
& $probe chain exe+0xSLOT 0x30 0x50
```

Verified only when the `=>` line names `.?AVClientChildOnlyInGameActor@pa@@`. Then `rtti` that absolute object address from the `=>` line:

```powershell
& $probe rtti 0xACTOR
```

## 4. Position while walking, and rotation while turning

Character in the world. Run the position command once while standing still. Walk a few metres, stand still, and run it again. The local vec3 is unverified if the two samples do not move.

```powershell
# Manager root, local position at physics +0x90
& $probe chain exe+0xSLOT 0x50 0x68 0x40 0x140 0x2C0 0x90 --type vec3

# WorldSystem root, local position at physics +0x90
& $probe chain exe+0xSLOT 0x30 0x50 0x68 0x40 0x140 0x2C0 0x90 --type vec3
```

Character control and physics class, once, before trusting the axes. Copy `0xPHYSICS` from the `=>` pointer.

```powershell
# Manager root
& $probe chain exe+0xSLOT 0x50 0x68 0x40
& $probe chain exe+0xSLOT 0x50 0x68 0x40 0x140 0x2C0
& $probe dump 0xPHYSICS --size 0x100

# WorldSystem root
& $probe chain exe+0xSLOT 0x30 0x50 0x68 0x40
& $probe chain exe+0xSLOT 0x30 0x50 0x68 0x40 0x140 0x2C0
& $probe dump 0xPHYSICS --size 0x100
```

Rotation, standing, then after a clear turn. Last offset is the axis. Do not write `physics+0x60`.

```powershell
# Manager root
& $probe chain exe+0xSLOT 0x50 0x68 0x40 0x140 0x2C0 0x60 --type vec3
& $probe chain exe+0xSLOT 0x50 0x68 0x40 0x140 0x2C0 0x70 --type vec3
& $probe chain exe+0xSLOT 0x50 0x68 0x40 0x140 0x2C0 0x80 --type vec3

# WorldSystem root
& $probe chain exe+0xSLOT 0x30 0x50 0x68 0x40 0x140 0x2C0 0x60 --type vec3
& $probe chain exe+0xSLOT 0x30 0x50 0x68 0x40 0x140 0x2C0 0x70 --type vec3
& $probe chain exe+0xSLOT 0x30 0x50 0x68 0x40 0x140 0x2C0 0x80 --type vec3
```

The 2976 chunk-origin and static-position RVAs are verified non-matches. Do not `read exe+0x6D56500` or `read exe+0x6D77858` as those values. If a live world-space vec3 is identified some other way, read it with `read 0xABSOLUTE --type vec3` and mark the cross-check unverified until both samples agree.

## 5. Health against the displayed value

The operator reads the on-screen health number and that number is written next to the probe output. Without that number, health stays unverified.

```powershell
# Manager root: status component, then count, then entries pointer
& $probe chain exe+0xSLOT 0x50 0x68 0x20
& $probe chain exe+0xSLOT 0x50 0x68 0x20 0x18 0x60 --type u32
& $probe chain exe+0xSLOT 0x50 0x68 0x20 0x18 0x58

# WorldSystem root
& $probe chain exe+0xSLOT 0x30 0x50 0x68 0x20
& $probe chain exe+0xSLOT 0x30 0x50 0x68 0x20 0x18 0x60 --type u32
& $probe chain exe+0xSLOT 0x30 0x50 0x68 0x20 0x18 0x58
```

`0x20` is accepted only when the status `=>` line names `.?AVClientStatusActorComponent@pa@@`. Copy `0xENTRIES` from the entries `=>` line. `--size` is decimal bytes: count times 144 (`0x90`). Cap the dump at 9216 bytes (64 entries).

```powershell
& $probe dump 0xENTRIES --size 1440
```

Id, current, and max are int64 at `+0x00`, `+0x08`, and `+0x18` of each `0x90`-byte entry. Divide current and max by 1000 only when matching the displayed number. Do not assume id 0. Repeat the dump after the displayed health changes. An id that does not move with the bar stays unverified.

## 6. Pointers during loading

Start the watch, then load a save within that 90 second window. Use the same root classified above. This watch lands on the position vec3.

```powershell
# Manager root
& $probe watch exe+0xSLOT 0x50 0x68 0x40 0x140 0x2C0 0x90 --type vec3 --seconds 90

# WorldSystem root
& $probe watch exe+0xSLOT 0x30 0x50 0x68 0x40 0x140 0x2C0 0x90 --type vec3 --seconds 90
```

While the watch is running, run the matching verbose position `chain` from section 4 two or three times so the log shows which hop is unreadable. `<chain broken>` alone does not identify the pointer. A coordinate is a verified load park only when those chain samples hold it and a later in-world position command shows a different place. `(0, 1000, 0)` within 5 is a 2976 hypothesis until the log shows it.

If the watch ends while the load is still on screen, run one more watch for 90 seconds. Keep both logs.

## 7. Local-versus-world burst

Not an adopted offset. The pass rules are in `docs/probe-3576/LOCAL-TO-WORLD.md`. `exe+0x71AC2C0` and `exe+0x71CF140` are candidates. Do not read `exe+0x6D56500` or `exe+0x6D77858` as the origin.

The script prints probe output, then the residual `static - (local + origin)` with no component adjusted. It uses `--count 4` on the absolute local field from that burst's chain. Run it twice while standing still, once after a short walk, and again after the character has stopped. Prefer a continuous walk across a boundary. After fast travel or a load, run the burst only once the character has settled, and do not require the static candidate to have stayed continuous.

```powershell
powershell -NoProfile -File tools\probe-3576\burst.ps1 -Slot exe+0xSLOT -ProcessId PID
```

While the character is traveling, a bounded watch samples the same candidates and exits by itself:

```powershell
powershell -NoProfile -File tools\probe-3576\origin-watch.ps1 -Slot exe+0xSLOT -ProcessId PID -Seconds 180
```

The watch writes each sample to `docs/probe-3576/origin-watch-PID-HHMMSS.log` and prints one line. An origin change keeps two extra samples, then prints `ACTION_REQUIRED` so the operator can stop. If the origin candidate is unchanged when the window ends, it prints `DONE` and the walk is not extended. The settled burst is still `burst.ps1` after the character is standing still. Y is recorded and not adjusted.

## 8. Diagnostic position reader

`cdmp_diag3576` is a separate read-only executable. Usage, status values, and the live checklist are in `docs/probe-3576/DIAG3576.md`. It does not adopt a slot or an origin, and it does not open the mod compatibility guard. Pass the PID and the experimental slot explicitly:

```powershell
build\bin\RelWithDebInfo\cdmp_diag3576.exe --pid PID --slot exe+0xSLOT --count 1
```
