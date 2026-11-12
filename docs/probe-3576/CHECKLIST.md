# Capture checklist: 3576 read-only player state

Session rules:

- Game start is the Steam library entry. `scripts/launch-2inst.ps1` stays unused.
- `bin64\cdmp.asi` stays absent. No ini from this mod is copied into `bin64`.
- Probe binary: `build\bin\RelWithDebInfo\cdmp_probe.exe`.
- Paste raw stdout into `docs/probe-3576/CAPTURE.md` under the same heading. Do not summarize over the raw lines.
- A result stays **unverified** until its pass line is true. Do not add the RVA to `offsets.h`.
- The compatibility guard stays closed for timestamp `0x6AED2E85`.

## Before the process exists

- [ ] `cdmp.asi` is not in `D:\SteamLibrary\steamapps\common\Crimson Desert\bin64`.
- [ ] Operator has started unmodified 1.0.0.3576 and is on the title screen, save not loaded.
- [ ] Operator replies that the window is up. The probe is not attached before that.

## 1. Identity and mapped base

Command: `info`.

- [ ] Raw stdout saved.
- [ ] Timestamp line is `0x6AED2E85` and says `UNKNOWN BUILD`. Any other timestamp: stop.
- [ ] `Base` and `size` copied into the conclusion. This is the mapped base. Preferred base `0x140000000` is not a substitute if `Base` differs.
- [ ] Pass: those three fields are in the capture. Otherwise identity is unverified and later `exe+` commands wait.

## 2. Vtables

Commands: four `vtables` lines and `classes WorldSystem`.

- [ ] Raw stdout saved.
- [ ] Each class has one `subobject offset 0x0` vtable.
- [ ] Manager vtable is `exe+0x5946C40`, or the line is marked candidate-only.
- [ ] Controlled-actor vtable is `exe+0x5914D58`, or candidate-only.
- [ ] Character-control vtable is `exe+0x59459F0`, or candidate-only.
- [ ] Status vtable is `exe+0x591C7C0`, or candidate-only.
- [ ] `classes WorldSystem` output saved. A miss does not invent a WorldSystem slot.

## 3. Manager and controlled actor

Commands: `findptr --heap`, `rtti 0xHIT`, `findptr` of the manager object, `chain exe+SLOT` twice, then the classified actor chain.

- [ ] Root recorded as **manager** or **WorldSystem**, from the no-offset `chain` rule in `COMMANDS.md`.
- [ ] Manager-rooted walks do not include `0x30`.
- [ ] WorldSystem-rooted walks include `0x30` only as the first hop, and that hop's `=>` line names `ClientActorManager`.
- [ ] `0x6D69190` was not used as the slot.
- [ ] Manager pass: `rtti` and both slot chains name `.?AVClientActorManager@pa@@`.
- [ ] Actor pass: in-world chain names `.?AVClientChildOnlyInGameActor@pa@@`, and `rtti 0xACTOR` repeats it.
- [ ] Title-screen null at the actor hop is recorded as unverified actor, not as a failed manager.

## 4. Position and rotation

Commands: the classified position `chain ... 0x90 --type vec3` standing and after a walk. Axes `0x60`, `0x70`, `0x80` before and after a turn. Physics `dump 0xPHYSICS --size 0x100` once.

- [ ] No command contains `<physics>+`, `object+`, or a bare `+90`.
- [ ] Operator states that the character walked, and the two position samples differ.
- [ ] Operator states that the character turned, and one axis changed while the three stay orthonormal.
- [ ] Position pass requires the walk. Rotation pass requires the turn. Otherwise mark that part unverified.
- [ ] Chunk origin and static world position stay unverified if they were not found. Do not fill them from `exe+0x6D56500` or `exe+0x6D77858`.
- [ ] A standing match does not verify the candidate RVAs. Sections 17 and 18 record the repeated stationary bursts and the short-walk burst. Section 20 records the settled burst after the origin change. The residual is the full vector, with Y left as measured. The jump after the rebase was not required to stay continuous. The candidates remain unadopted.

## 5. Health

Commands: status chain, count `u32`, entries pointer, `dump` of the entries. Operator reads the on-screen number.

- [ ] Status `=>` line names `.?AVClientStatusActorComponent@pa@@` before the container hops are trusted.
- [ ] Displayed health number is written beside the dump.
- [ ] The matching entry's current/1000 and max/1000 equal that number.
- [ ] The same id is still the one that moves if a second displayed value is captured.
- [ ] Otherwise health is unverified. Id 0 is not assumed.

## 6. Loading

Commands: `watch ... 0x90 --type vec3 --seconds 90` plus two or three verbose position chains during that window.

- [ ] Watch stdout saved, including `<chain broken>` lines.
- [ ] Verbose chains saved so a broken hop is named.
- [ ] A held coordinate and the later in-world position are both in the log before any park is called verified.
- [ ] `(0, 1000, 0)` is labeled a 2976 hypothesis unless those samples show it.

## Conclusions block

Write this at the end of `CAPTURE.md` after the session. Use only these words for each row: `verified`, `unverified`, or `verified non-match`.

| Item | Status | Evidence heading |
|---|---|---|
| PE timestamp and mapped base | | 1 |
| Four vtable RVAs | | 2 |
| Manager object and slot root | | 3 |
| Controlled actor | | 3 |
| Position | | 4 |
| Rotation | | 4 |
| Health versus displayed value | | 5 |
| Loading pointer behavior | | 6 |
| Offsets adopted into the mod | not adopted | guard remains closed |
