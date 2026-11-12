# Local to world on 1.0.0.3576

Read-only file scan plus standing reads in pid 16920. Nothing here is adopted. `offsets.h` and `build_guard` were not changed. No hook was installed.

The scanner is `tools/probe-3576/scan_local_to_world.cpp`. `tools/probe-3576/run_scan.ps1` compiles it under `%TEMP%` and writes `docs/probe-3576/local-to-world-scan.txt`. It refuses any executable whose PE timestamp is not `0x6AED2E85`. The self-test checks the RIP formula `next_ip + disp32` on a known `vsubps` / `vmovups [rcx+0x90]` buffer.

## What 2976 does

`PlayerReader::read` in `src/mod/game/player_reader.cpp` adds two vec3 values. Local is physics `+0x90`. The addend is the vec3 at `exe+0x6D56500` (`kChunkOrigin`). The cross-check is the vec3 at `exe+0x6D77858` (`kStaticPlayerPos`). A mismatch is kept as `static_pos_error` because that global can lag by a frame. `docs/research/hallazgos-en-vivo.md` records one running sample: local `(-805.6, 728.2, -598.8)` plus origin `(-10000, 0, -2000)` against global `(-10805.4, 727.7, -2599.1)`.

The file shape of that subtract is recorded in `docs/research/cheat-tables-versions.md`: load physics from `[rbx+0x2C0]`, subtract a RIP-relative vec, store `[rcx+0x90]`. The static world store is `vmovsd [rip+XY], xmm7` then `mov [rip+Z], r15d`. Those 2976 RVAs are a comparison. This note does not treat them as 3576 slots.

## What the 3576 file does

The same byte strings are present once each. The RIP targets are different globals.

| 2976 shape | 3576 matches | Site | Resolved operand |
|---|---|---|---|
| `mov rcx, [rbx+??]` / `vsubps [rip]` / `vmovups [rcx+0x90]` | 1 | `exe+0x397DCB1` | `exe+0x71AC2C0` |
| Short neighbour of that store | 1 | `exe+0x397DCC6` | Same site as the row above. The `vsubps` starts here. |
| `vmovsd [rip+XY]` / `mov [rip+Z], r15d` | 1 | `exe+0xA310EA` | XY `exe+0x71CF140`, Z `exe+0x71CF148` |
| SSE `addps [r13]` / `movups [r13]` | 1 | `exe+0x458711F` | Local step. No origin operand. |
| `lea r13, [rbx+0x90]` | 1 | `exe+0x4586895` | 0x88A bytes before the SSE step, the same distance as 2976. |

Bytes at `exe+0x397DCB1`:

```text
48 8B 8B C0 02 00 00          mov rcx, [rbx+0x2C0]
C5 FB 10 44 24 60             vmovsd xmm0, [rsp+0x60]
C4 E3 79 21 44 24 68 20       vinsertps xmm0, xmm0, [rsp+0x68], 0x20
C5 F8 5C 05 F2 E5 82 03       vsubps xmm0, xmm0, [rip+0x0382E5F2]
C5 F8 11 81 90 00 00 00       vmovups [rcx+0x90], xmm0
```

`vsubps` ends at `exe+0x397DCCE`. `0x397DCCE + 0x0382E5F2 = 0x71AC2C0`. The displacement on `rbx` is `0x2C0`, the same hop the live chain uses to reach the physics object. The stack vec3 is the minuend. The store writes physics `+0x90`. That is local = stack world − the global, so world = local + the global. `rbx` itself is not identified by this file pass.

A forward site at `exe+0x3982FB0` loads `[rax+0x90]`, adds the same global, and stores the sum to `[rbp-0x80]`:

```text
C5 F8 10 88 90 00 00 00       vmovups xmm1, [rax+0x90]
C5 70 58 35 00 93 82 03       vaddps xmm14, xmm1, [rip+0x03829300]
C5 78 11 75 80                vmovups [rbp-0x80], xmm14
```

`0x3982FC0 + 0x03829300 = 0x71AC2C0`. `rax` is not identified beyond the encoding.

The static writer at `exe+0xA310EA`:

```text
C5 FB 11 3D 4E E0 79 06       vmovsd [rip+0x0679E04E], xmm7
44 89 3D 4F E0 79 06          mov [rip+0x0679E04F], r15d
```

`0xA310F2 + 0x0679E04E = 0x71CF140`. The Z instruction ends at `0xA310F9`; `0xA310F9 + 0x0679E04F = 0x71CF148`. Z is a 32-bit store, so this triple is float32, not one double. `vmovsd` stores the low 8 bytes of the register, which on 2976 were the two float32 values X and Y.

A tight search for a VEX load or store of `[reg+0x90]` within 48 bytes of a RIP-relative `vaddps` or `vsubps` found 20 sites. Nineteen use `exe+0x71AC2C0`. Fifteen of those also have a `+0x2C0` memory displacement within the preceding 64 bytes. The twentieth site is a scalar `vaddss` in `.tls$` at `exe+0x10832635` against `exe+0x6141B20`. That target is non-writable `.arch` and the file floats there are about `0.001, 0.0013, 0.00139, 0.0017`. It is a constant, not a chunk origin.

`.text1` contains 440 VEX loads and 662 VEX stores of a `[reg+0x90]` operand. `.tls$` adds 42 loads and 34 stores. The raw log prints cumulative totals, so the `.tls$` line shows 482 and 696. Almost all of those sites are not next to this global.

`exe+0x6D56500`, `exe+0x6D77858`, and `exe+0x6D77860` are not operands of any tight pair. In the file they are small floats in writable `.rodata` (`1.0f` is the third float at `0x6D56500`). A live read at 10:41:26 matched those file floats. They are not the 3576 origin or the 3576 static position.

The candidate globals sit in the writable tail of `.rodata`, past the file-backed span that ends at VA `0x0703EA00`. The scanner reports their file bytes as unavailable. Their values come from the live reads below.

Both candidate globals are float32 vectors. `vsubps` / `vaddps` / `vmovups` are packed float32. A double read of the first 8 bytes at `exe+0x71AC2C0` printed `0.0000`, not `-8000`. The i32 reads of the static triple are the float bits (`-973228827`, `1141535672`, `-992333643`), not milli-units of the coordinates.

ASCII `WorldOffset` occurs twice in the file. `chunkOrigin` occurs zero times. RTTI for `hknpWorld` and several broadphase classes is present. Those names do not assign the global to a field.

## Standing sample, pid 16920

Identity was read again at 10:31:32: base `0x140000000`, timestamp `0x6AED2E85`, unknown build. The chain from temporary sampler `exe+0x71C09A8` still ended at physics `0x582E9CB2F80`, local field `0x582E9CB3010`. That slot is not adopted.

Precise float32 reads at 10:43:04, one burst, character still at the section 15 pose:

| Value | X | Y | Z | W |
|---|---|---|---|---|
| Physics `+0x90` | -118.6118 | 553.8584 | -745.5221 | 0 |
| Candidate origin `exe+0x71AC2C0` | -8000.0000 | 0.0000 | -1000.0000 | 0.0000 |
| Local + origin | -8118.6118 | 553.8584 | -1745.5221 | 0 |
| Candidate static `exe+0x71CF140` | -8118.6118 | 553.8081 | -1745.5221 | 0.0000 |
| Static − (local + origin) | 0 | -0.0503 | 0 | 0 |

X and Z match as printed. Y is 0.0503 lower on the static global. The same relationship was already visible at 10:41:26 from the vec4 prints. The character had not moved since 10:13, so this residual is not a walking-frame lag. 2976 research measured a world-minus-physics Y near `-0.05` in the same kind of comparison. That is context for the residual. It is not a constant to subtract in the mod.

This is one still pose. It is not a walk, and the origin was not observed to change. The RVAs stay candidates.

## The two heap copies

Section 15 found three heap hits for the stopped local bytes in pid 16920. `0x582E9CB3010` is physics `+0x90`. The other two are `0x583072A6470` and `0x583ADF71470`.

At 10:31:32 both still held `(-118.612, 553.858, -745.522, 0.000)`. The same 16 bytes as float64 are about `6.10e19` and `0`. As i32 they are `-1024640704`, `1141536496`, `-1002806934`. Those are the float bits, not a second coordinate encoding. Image `findptr` of either address, and of either address minus `0x90`, found 0 slots. Heap `findptr` of either translation found 0 pointers. Nothing points at the vec itself.

A `0x120` dump ending at each translation is identical, including one pointer to `.?AVhknpCompoundShape@@` `0x5830797A800` at translation minus `0x78` (`0x583072A63F8` and `0x583ADF713F8`). `rtti` on the translation and on translation minus `0x90` is `(no RTTI)`. Heap `findptr` of the shape object found exactly four stores:

- `0x583072A63F8` and `0x583ADF713F8`, the two records above
- `0x583079872E8`
- `0x5832E82A5F8`

`rtti` on those extra stores, and on the qwords beside them (`0x582F744C6F0`, `0x582F737BF38`, `0x583077AE240`, `0x58309D2B5E0`), is `(no RTTI)`. The shape object's first qword is `exe+0x56B8B08`. The first `0x100` bytes of that object do not contain either translation. That vtable qword is not adopted.

The physics object is a different allocation. Its first qword is `exe+0x60CD660`, and `+0x28` is `.?AVhknpCapsuleShape@@` at `0x58352D621C0`. Neither value is adopted. The capsule is not the compound shape.

Neighboring records in the dump use other compound-shape pointers and other translations, including values such as `-112`, `-712`, `48`, `712`, and `808`. The matched vec3 is one record in that run.

The 3x3 immediately before each translation matches the physics axes read at 10:34:28 to three decimals: `(0.409, 0, -0.912)`, `(0, 1, 0)`, `(0.912, 0, 0.409)`. The physics Y vec4 w-lane was `0.912`. The record's corresponding w-lane, re-read at 10:41:26, was `0.944`. The records are not a byte copy of physics `+0x60` through `+0x9F`.

On this pose the origin is nonzero and the static candidate differs from local by about `(-8000, -0.050, -1000)`. Both records still equal the local vec4. They are not the static world position. Their parent object is still unnamed, and they were not watched across an origin change, so this does not close their classification.

## Burst procedure

Three corrections to the earlier command list:

1. `cdmp_probe` accepts `exe+RVA` or one full hex address. `0xLOCAL+4` is rejected. A burst reads four float32 values with `--count 4` from the absolute field the chain just printed, and from `exe+0x71AC2C0` and `exe+0x71CF140`. The chain output is the pointer-identity record for that burst.
2. Take at least two stationary bursts, then a short walk, then another stationary burst. Each burst records local, the origin candidate, the static candidate, the pointer identities, the timestamps, and the full residual vector. The bookend covers X, Y, and Z. A change on any of those three means the burst was not stationary.
3. An origin step is judged after the character is still. Prefer a continuous walk across a boundary. Fast travel or a load may jump the world position. After that kind of move, compare local + origin with the static candidate once the character has settled. Do not require the static candidate to stay continuous through the jump.

The residual is `static - (local + origin)` on X, Y, Z, and W. It is measured. No component is subtracted out or replaced. `exe+0x71AC2C0` and `exe+0x71CF140` stay candidates. `exe+0x6D56500` and `exe+0x6D77858` are not read as the origin. The slot passed to the script is a temporary sampler.

`tools/probe-3576/burst.ps1` prints the probe lines under `OBSERVATION` and the arithmetic under `COMPUTED`.

```powershell
powershell -NoProfile -File tools\probe-3576\burst.ps1 -Slot exe+0xSLOT -ProcessId PID
```

Run that command twice while the character stands still. If both bookends match and the two residual vectors agree at the printed resolution, the stationary pair is usable. Walk a short distance, stop, stand still, and run it once more. Keep every raw line.

After that pair and the short-walk burst are saved, watch the origin candidate for a bounded window with `tools/probe-3576/origin-watch.ps1`. This session uses 180 seconds. The script samples local, the origin candidate, the static candidate, and the residual, and it exits when the origin changes or when the window ends. It does not ask for more travel after `DONE`. On a change it keeps two extra samples so a continuous walk can be compared around that sample. World position may jump on fast travel or a load; those settled samples are compared without requiring continuity. Stand still only after the watch says to stop, or after it reports no change, then run `burst.ps1`. The comparison on a still character is the settled residual. Y is measured and not adjusted.

Heap addresses belong to the pid in that burst's `info` line. A new process needs a new chain. The 10:43:04 numbers are an earlier single pose, not this repeated series.

The stationary pair and the short-walk burst for pid 16920 are in `CAPTURE.md` sections 17 and 18. Section 19 records the bounded watch: the origin candidate changed from `(-8000, 0, -1000)` to `(-10000, 0, -4000)`, the rebase sample kept the world point, and the next sample jumped. Section 20 records the settled residual `(0.0000, -0.0500, 0.0000, 0.0000)`. Y was not adjusted. The candidates are not adopted.
