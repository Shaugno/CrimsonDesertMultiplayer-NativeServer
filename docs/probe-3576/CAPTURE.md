# Capture: 3576 title-screen identity and vtables

Process pid 48460, path `D:\SteamLibrary\steamapps\common\Crimson Desert\bin64\CrimsonDesert.exe`.
`cdmp.asi` was not in that directory. The process module list had no `cdmp` module. System `WINMM.dll` and `VERSION.dll` were loaded from `C:\WINDOWS\SYSTEM32`.
Probe: `build\bin\RelWithDebInfo\cdmp_probe.exe`. Read-only attach. No offset header was edited.

## 1. Identity and mapped base

Command: `cdmp_probe info`

```
PID      48460
Module   D:\SteamLibrary\steamapps\common\Crimson Desert\bin64\CrimsonDesert.exe
Base     0x140000000  size 0x13CC4000
PE timestamp 0x6AED2E85 (UNKNOWN BUILD - offsets/sigs may be invalid)
```

Exit code 0.

## 2. Vtables

Command: `cdmp_probe vtables .?AVClientActorManager@pa@@`

```
vtable exe+0x5946C40  (COL exe+0x61A0C38, subobject offset 0x0)
```

Exit code 0.

Command: `cdmp_probe vtables .?AVClientChildOnlyInGameActor@pa@@`

```
vtable exe+0x5914D58  (COL exe+0x6185438, subobject offset 0x0)
```

Exit code 0.

Command: `cdmp_probe vtables .?AVClientCharacterControlActorComponent@pa@@`

```
vtable exe+0x59459F0  (COL exe+0x619F8C0, subobject offset 0x0)
```

Exit code 0.

Command: `cdmp_probe vtables .?AVClientStatusActorComponent@pa@@`

```
vtable exe+0x591C7C0  (COL exe+0x618A040, subobject offset 0x0)
```

Exit code 0.

Command: `cdmp_probe classes WorldSystem`

```
0 shown
```

Exit code 0.

## 3. Manager object and static slots

Title screen, same process pid 48460. Vtable used: `exe+0x5946C40`, verified in section 2. No `0x30` hop was applied. `0x6D69190` was not read.

Command: `cdmp_probe findptr exe+0x5946C40 --heap --max 20`

```
  0x3C9F8757800
1 found in heap
```

Exit code 0.

Command: `cdmp_probe rtti 0x3C9F8757800`

```
.?AVClientActorManager@pa@@
```

Exit code 0.

Command: `cdmp_probe findptr 0x3C9F8757800 --max 20`

```
  exe+0x71C09A8
  exe+0x7233960
  exe+0x7233AA0
3 found in exe image
```

Exit code 0.

Command: `cdmp_probe chain exe+0x71C09A8` (first)

```
=> exe+0x71C09A8 : 0x3C9F8757800  -> object .?AVClientActorManager@pa@@
```

Exit code 0.

Command: `cdmp_probe chain exe+0x71C09A8` (repeat)

```
=> exe+0x71C09A8 : 0x3C9F8757800  -> object .?AVClientActorManager@pa@@
```

Exit code 0.

Command: `cdmp_probe chain exe+0x7233960` (first)

```
=> exe+0x7233960 : 0x3C9F8757800  -> object .?AVClientActorManager@pa@@
```

Exit code 0.

Command: `cdmp_probe chain exe+0x7233960` (repeat)

```
=> exe+0x7233960 : 0x3C9F8757800  -> object .?AVClientActorManager@pa@@
```

Exit code 0.

Command: `cdmp_probe chain exe+0x7233AA0` (first)

```
=> exe+0x7233AA0 : 0x3C9F8757800  -> object .?AVClientActorManager@pa@@
```

Exit code 0.

Command: `cdmp_probe chain exe+0x7233AA0` (repeat)

```
=> exe+0x7233AA0 : 0x3C9F8757800  -> object .?AVClientActorManager@pa@@
```

Exit code 0.

## 3b. Title-screen actor hop +0x50

Same process pid 48460, title screen, save not loaded. All three slots are the manager-rooted slots from section 3. The hop is hex `0x50`. The probe prints that displacement as decimal `+80`. No permanent slot was chosen. The in-world repeat has not been run.

Command: `cdmp_probe chain exe+0x71C09A8 0x50`

```
  [exe+0x71C09A8] = 0x3C9F8757800  (.?AVClientActorManager@pa@@)  +80 -> 0x3C9F8757850
=> 0x3C9F8757850 : 0x3CA8B8A7080  -> object .?AVClientChildOnlyInGameActor@pa@@
```

Exit code 0.

Command: `cdmp_probe chain exe+0x7233960 0x50`

```
  [exe+0x7233960] = 0x3C9F8757800  (.?AVClientActorManager@pa@@)  +80 -> 0x3C9F8757850
=> 0x3C9F8757850 : 0x3CA8B8A7080  -> object .?AVClientChildOnlyInGameActor@pa@@
```

Exit code 0.

Command: `cdmp_probe chain exe+0x7233AA0 0x50`

```
  [exe+0x7233AA0] = 0x3C9F8757800  (.?AVClientActorManager@pa@@)  +80 -> 0x3C9F8757850
=> 0x3C9F8757850 : 0x3CA8B8A7080  -> object .?AVClientChildOnlyInGameActor@pa@@
```

Exit code 0.

Command: `cdmp_probe rtti 0x3CA8B8A7080`

```
.?AVClientChildOnlyInGameActor@pa@@
```

Exit code 0.

The pointer at manager `+0x50` was not null on this title screen. All three slots named the same actor object. That object address belongs to this process only.

## 4. In-world actor, character control, and physics

Same process pid 48460, after the operator confirmed the character was standing in the world. The three manager-rooted slots were walked one hop at a time. No permanent slot was chosen. The probe prints hex offsets as decimal: `0x50` is `+80`, `0x68` is `+104`, `0x40` is `+64`, `0x140` is `+320`, and `0x2C0` is `+704`. No hop was unreadable, and no pointer on this walk was null.

Command: `cdmp_probe chain exe+0x71C09A8 0x50`

```
  [exe+0x71C09A8] = 0x3C9F8757800  (.?AVClientActorManager@pa@@)  +80 -> 0x3C9F8757850
=> 0x3C9F8757850 : 0x3CA8B8A7080  -> object .?AVClientChildOnlyInGameActor@pa@@
```

Exit code 0.

Command: `cdmp_probe chain exe+0x7233960 0x50`

```
  [exe+0x7233960] = 0x3C9F8757800  (.?AVClientActorManager@pa@@)  +80 -> 0x3C9F8757850
=> 0x3C9F8757850 : 0x3CA8B8A7080  -> object .?AVClientChildOnlyInGameActor@pa@@
```

Exit code 0.

Command: `cdmp_probe chain exe+0x7233AA0 0x50`

```
  [exe+0x7233AA0] = 0x3C9F8757800  (.?AVClientActorManager@pa@@)  +80 -> 0x3C9F8757850
=> 0x3C9F8757850 : 0x3CA8B8A7080  -> object .?AVClientChildOnlyInGameActor@pa@@
```

Exit code 0.

Command: `cdmp_probe rtti 0x3CA8B8A7080`

```
.?AVClientChildOnlyInGameActor@pa@@
```

Exit code 0.

Command: `cdmp_probe chain exe+0x71C09A8 0x50 0x68`

```
  [exe+0x71C09A8] = 0x3C9F8757800  (.?AVClientActorManager@pa@@)  +80 -> 0x3C9F8757850
  [0x3C9F8757850] = 0x3CA8B8A7080  (.?AVClientChildOnlyInGameActor@pa@@)  +104 -> 0x3CA8B8A70E8
=> 0x3CA8B8A70E8 : 0x3C9FCB8C000
```

Exit code 0.

Command: `cdmp_probe chain exe+0x7233960 0x50 0x68`

```
  [exe+0x7233960] = 0x3C9F8757800  (.?AVClientActorManager@pa@@)  +80 -> 0x3C9F8757850
  [0x3C9F8757850] = 0x3CA8B8A7080  (.?AVClientChildOnlyInGameActor@pa@@)  +104 -> 0x3CA8B8A70E8
=> 0x3CA8B8A70E8 : 0x3C9FCB8C000
```

Exit code 0.

Command: `cdmp_probe chain exe+0x7233AA0 0x50 0x68`

```
  [exe+0x7233AA0] = 0x3C9F8757800  (.?AVClientActorManager@pa@@)  +80 -> 0x3C9F8757850
  [0x3C9F8757850] = 0x3CA8B8A7080  (.?AVClientChildOnlyInGameActor@pa@@)  +104 -> 0x3CA8B8A70E8
=> 0x3CA8B8A70E8 : 0x3C9FCB8C000
```

Exit code 0.

Command: `cdmp_probe rtti 0x3C9FCB8C000`

```
(no RTTI)
```

Exit code 0.

Command: `cdmp_probe chain exe+0x71C09A8 0x50 0x68 0x40`

```
  [exe+0x71C09A8] = 0x3C9F8757800  (.?AVClientActorManager@pa@@)  +80 -> 0x3C9F8757850
  [0x3C9F8757850] = 0x3CA8B8A7080  (.?AVClientChildOnlyInGameActor@pa@@)  +104 -> 0x3CA8B8A70E8
  [0x3CA8B8A70E8] = 0x3C9FCB8C000  +64 -> 0x3C9FCB8C040
=> 0x3C9FCB8C040 : 0x3CA8B7A6180  -> object .?AVClientCharacterControlActorComponent@pa@@
```

Exit code 0.

Command: `cdmp_probe chain exe+0x7233960 0x50 0x68 0x40`

```
  [exe+0x7233960] = 0x3C9F8757800  (.?AVClientActorManager@pa@@)  +80 -> 0x3C9F8757850
  [0x3C9F8757850] = 0x3CA8B8A7080  (.?AVClientChildOnlyInGameActor@pa@@)  +104 -> 0x3CA8B8A70E8
  [0x3CA8B8A70E8] = 0x3C9FCB8C000  +64 -> 0x3C9FCB8C040
=> 0x3C9FCB8C040 : 0x3CA8B7A6180  -> object .?AVClientCharacterControlActorComponent@pa@@
```

Exit code 0.

Command: `cdmp_probe chain exe+0x7233AA0 0x50 0x68 0x40`

```
  [exe+0x7233AA0] = 0x3C9F8757800  (.?AVClientActorManager@pa@@)  +80 -> 0x3C9F8757850
  [0x3C9F8757850] = 0x3CA8B8A7080  (.?AVClientChildOnlyInGameActor@pa@@)  +104 -> 0x3CA8B8A70E8
  [0x3CA8B8A70E8] = 0x3C9FCB8C000  +64 -> 0x3C9FCB8C040
=> 0x3C9FCB8C040 : 0x3CA8B7A6180  -> object .?AVClientCharacterControlActorComponent@pa@@
```

Exit code 0.

Command: `cdmp_probe rtti 0x3CA8B7A6180`

```
.?AVClientCharacterControlActorComponent@pa@@
```

Exit code 0.

Command: `cdmp_probe chain exe+0x71C09A8 0x50 0x68 0x40 0x140`

```
  [exe+0x71C09A8] = 0x3C9F8757800  (.?AVClientActorManager@pa@@)  +80 -> 0x3C9F8757850
  [0x3C9F8757850] = 0x3CA8B8A7080  (.?AVClientChildOnlyInGameActor@pa@@)  +104 -> 0x3CA8B8A70E8
  [0x3CA8B8A70E8] = 0x3C9FCB8C000  +64 -> 0x3C9FCB8C040
  [0x3C9FCB8C040] = 0x3CA8B7A6180  (.?AVClientCharacterControlActorComponent@pa@@)  +320 -> 0x3CA8B7A62C0
=> 0x3CA8B7A62C0 : 0x3C9E61C3F00
```

Exit code 0.

Command: `cdmp_probe chain exe+0x7233960 0x50 0x68 0x40 0x140`

```
  [exe+0x7233960] = 0x3C9F8757800  (.?AVClientActorManager@pa@@)  +80 -> 0x3C9F8757850
  [0x3C9F8757850] = 0x3CA8B8A7080  (.?AVClientChildOnlyInGameActor@pa@@)  +104 -> 0x3CA8B8A70E8
  [0x3CA8B8A70E8] = 0x3C9FCB8C000  +64 -> 0x3C9FCB8C040
  [0x3C9FCB8C040] = 0x3CA8B7A6180  (.?AVClientCharacterControlActorComponent@pa@@)  +320 -> 0x3CA8B7A62C0
=> 0x3CA8B7A62C0 : 0x3C9E61C3F00
```

Exit code 0.

Command: `cdmp_probe chain exe+0x7233AA0 0x50 0x68 0x40 0x140`

```
  [exe+0x7233AA0] = 0x3C9F8757800  (.?AVClientActorManager@pa@@)  +80 -> 0x3C9F8757850
  [0x3C9F8757850] = 0x3CA8B8A7080  (.?AVClientChildOnlyInGameActor@pa@@)  +104 -> 0x3CA8B8A70E8
  [0x3CA8B8A70E8] = 0x3C9FCB8C000  +64 -> 0x3C9FCB8C040
  [0x3C9FCB8C040] = 0x3CA8B7A6180  (.?AVClientCharacterControlActorComponent@pa@@)  +320 -> 0x3CA8B7A62C0
=> 0x3CA8B7A62C0 : 0x3C9E61C3F00
```

Exit code 0.

Command: `cdmp_probe rtti 0x3C9E61C3F00`

```
(no RTTI)
```

Exit code 0.

Command: `cdmp_probe chain exe+0x71C09A8 0x50 0x68 0x40 0x140 0x2C0`

```
  [exe+0x71C09A8] = 0x3C9F8757800  (.?AVClientActorManager@pa@@)  +80 -> 0x3C9F8757850
  [0x3C9F8757850] = 0x3CA8B8A7080  (.?AVClientChildOnlyInGameActor@pa@@)  +104 -> 0x3CA8B8A70E8
  [0x3CA8B8A70E8] = 0x3C9FCB8C000  +64 -> 0x3C9FCB8C040
  [0x3C9FCB8C040] = 0x3CA8B7A6180  (.?AVClientCharacterControlActorComponent@pa@@)  +320 -> 0x3CA8B7A62C0
  [0x3CA8B7A62C0] = 0x3C9E61C3F00  +704 -> 0x3C9E61C41C0
=> 0x3C9E61C41C0 : 0x3C9E7AE0A00
```

Exit code 0.

Command: `cdmp_probe chain exe+0x7233960 0x50 0x68 0x40 0x140 0x2C0`

```
  [exe+0x7233960] = 0x3C9F8757800  (.?AVClientActorManager@pa@@)  +80 -> 0x3C9F8757850
  [0x3C9F8757850] = 0x3CA8B8A7080  (.?AVClientChildOnlyInGameActor@pa@@)  +104 -> 0x3CA8B8A70E8
  [0x3CA8B8A70E8] = 0x3C9FCB8C000  +64 -> 0x3C9FCB8C040
  [0x3C9FCB8C040] = 0x3CA8B7A6180  (.?AVClientCharacterControlActorComponent@pa@@)  +320 -> 0x3CA8B7A62C0
  [0x3CA8B7A62C0] = 0x3C9E61C3F00  +704 -> 0x3C9E61C41C0
=> 0x3C9E61C41C0 : 0x3C9E7AE0A00
```

Exit code 0.

Command: `cdmp_probe chain exe+0x7233AA0 0x50 0x68 0x40 0x140 0x2C0`

```
  [exe+0x7233AA0] = 0x3C9F8757800  (.?AVClientActorManager@pa@@)  +80 -> 0x3C9F8757850
  [0x3C9F8757850] = 0x3CA8B8A7080  (.?AVClientChildOnlyInGameActor@pa@@)  +104 -> 0x3CA8B8A70E8
  [0x3CA8B8A70E8] = 0x3C9FCB8C000  +64 -> 0x3C9FCB8C040
  [0x3C9FCB8C040] = 0x3CA8B7A6180  (.?AVClientCharacterControlActorComponent@pa@@)  +320 -> 0x3CA8B7A62C0
  [0x3CA8B7A62C0] = 0x3C9E61C3F00  +704 -> 0x3C9E61C41C0
=> 0x3C9E61C41C0 : 0x3C9E7AE0A00
```

Exit code 0.

Command: `cdmp_probe rtti 0x3C9E7AE0A00`

```
(no RTTI)
```

Exit code 0.

Command: `cdmp_probe dump 0x3C9E7AE0A00 --size 0x100`

```
+0000  00000001460CD660     9013.5938               exe+0x60CD660
+0008  00000000FFFFFFFF                     0.0000  ptr
+0010  0000000000000001                     0.0000  
+0018  000003CB23D2AC40                             ptr
+0020  0000000100000001                             ptr
+0028  000003CAA543BF40                             ptr -> .?AVhknpCapsuleShape@@
+0030  0000000000000000        0.0000       0.0000  
+0038  800000000100001B                    -0.0000  
+0040  000000000000401B                     0.0000  
+0048  0000000146E90188    29824.7656               exe+0x6E90188
+0050  0000000146E90190    29824.7812               exe+0x6E90190
+0058  0000000000000000        0.0000       0.0000  
+0060  00000000BCE1F9C0       -0.0276       0.0000  
+0068  800000003F7FE710        0.9996      -0.0000  
+0070  3F80000080000000       -0.0000       1.0000  
+0078  BF7FE71000000000        0.0000      -0.9996  
+0080  80000000BF7FE710       -0.9996      -0.0000  
+0088  00000000BCE1F9C0       -0.0276       0.0000  
+0090  440A1B90C2D9D75C     -108.9206     552.4307  
+0098  00000000C4396EFB     -741.7341       0.0000  ptr
+00A0  00000000BEEC52D6       -0.4616       0.0000  
+00A8  3F09D695BEEC52D6       -0.4616       0.5384  
+00B0  3FF193423EEC52D6        0.4616       1.8873  
+00B8  3FBB14B63EEC52D6        0.4616       1.4616  
+00C0  0000000000000000        0.0000       0.0000  
+00C8  0000000000000000        0.0000       0.0000  
+00D0  350C9D0DB0FE4A22                             
+00D8  00000000B255DE27                     0.0000  
+00E0  0000000000000000        0.0000       0.0000  
+00E8  0000000000000000        0.0000       0.0000  
+00F0  00000000447454EB      977.3268       0.0000  
+00F8  000003C9E6868500                             ptr
```

Exit code 0.

The `+0x2C0` pointer was readable from all three slots. `rtti` on `0x3C9E7AE0A00` returned `(no RTTI)`, so the physics class is unverified. The qword at offset 0 is `exe+0x60CD660`. That RVA is not adopted. The dump also shows a pointer at `+0x28` to `.?AVhknpCapsuleShape@@`. Floats at `+0x60`, `+0x70`, `+0x80`, and `+0x90` are present in this dump and are not a verified position or rotation.

## 5. Standing sample

Temporary sampling slot: `exe+0x71C09A8`. This does not adopt that slot. The other two manager slots were not read in this section. Each command re-walked the chain from the slot. Operator state: standing in the world, as confirmed before this sample. No walk or turn was requested yet.

`+144` is decimal for hex `0x90`, `+96` for `0x60`, `+112` for `0x70`, and `+128` for `0x80`.

Command: `cdmp_probe chain exe+0x71C09A8 0x50 0x68 0x40 0x140 0x2C0 0x90 --type vec3`

Timestamp: 2026-11-12 08:50:33.766

```
  [exe+0x71C09A8] = 0x3C9F8757800  (.?AVClientActorManager@pa@@)  +80 -> 0x3C9F8757850
  [0x3C9F8757850] = 0x3CA8B8A7080  (.?AVClientChildOnlyInGameActor@pa@@)  +104 -> 0x3CA8B8A70E8
  [0x3CA8B8A70E8] = 0x3C9FCB8C000  +64 -> 0x3C9FCB8C040
  [0x3C9FCB8C040] = 0x3CA8B7A6180  (.?AVClientCharacterControlActorComponent@pa@@)  +320 -> 0x3CA8B7A62C0
  [0x3CA8B7A62C0] = 0x3C9E61C3F00  +704 -> 0x3C9E61C41C0
  [0x3C9E61C41C0] = 0x3C9E7AE0A00  +144 -> 0x3C9E7AE0A90
=> 0x3C9E7AE0A90 : (-112.655, 552.443, -724.742)
```

Exit code 0.

Command: `cdmp_probe chain exe+0x71C09A8 0x50 0x68 0x40 0x140 0x2C0 0x60 --type vec3`

Timestamp: 2026-11-12 08:50:33.837

```
  [exe+0x71C09A8] = 0x3C9F8757800  (.?AVClientActorManager@pa@@)  +80 -> 0x3C9F8757850
  [0x3C9F8757850] = 0x3CA8B8A7080  (.?AVClientChildOnlyInGameActor@pa@@)  +104 -> 0x3CA8B8A70E8
  [0x3CA8B8A70E8] = 0x3C9FCB8C000  +64 -> 0x3C9FCB8C040
  [0x3C9FCB8C040] = 0x3CA8B7A6180  (.?AVClientCharacterControlActorComponent@pa@@)  +320 -> 0x3CA8B7A62C0
  [0x3CA8B7A62C0] = 0x3C9E61C3F00  +704 -> 0x3C9E61C41C0
  [0x3C9E61C41C0] = 0x3C9E7AE0A00  +96 -> 0x3C9E7AE0A60
=> 0x3C9E7AE0A60 : (-0.998, 0.000, 0.057)
```

Exit code 0.

Command: `cdmp_probe chain exe+0x71C09A8 0x50 0x68 0x40 0x140 0x2C0 0x70 --type vec3`

Timestamp: 2026-11-12 08:50:33.883

```
  [exe+0x71C09A8] = 0x3C9F8757800  (.?AVClientActorManager@pa@@)  +80 -> 0x3C9F8757850
  [0x3C9F8757850] = 0x3CA8B8A7080  (.?AVClientChildOnlyInGameActor@pa@@)  +104 -> 0x3CA8B8A70E8
  [0x3CA8B8A70E8] = 0x3C9FCB8C000  +64 -> 0x3C9FCB8C040
  [0x3C9FCB8C040] = 0x3CA8B7A6180  (.?AVClientCharacterControlActorComponent@pa@@)  +320 -> 0x3CA8B7A62C0
  [0x3CA8B7A62C0] = 0x3C9E61C3F00  +704 -> 0x3C9E61C41C0
  [0x3C9E61C41C0] = 0x3C9E7AE0A00  +112 -> 0x3C9E7AE0A70
=> 0x3C9E7AE0A70 : (0.000, 1.000, 0.000)
```

Exit code 0.

Command: `cdmp_probe chain exe+0x71C09A8 0x50 0x68 0x40 0x140 0x2C0 0x80 --type vec3`

Timestamp: 2026-11-12 08:50:33.921

```
  [exe+0x71C09A8] = 0x3C9F8757800  (.?AVClientActorManager@pa@@)  +80 -> 0x3C9F8757850
  [0x3C9F8757850] = 0x3CA8B8A7080  (.?AVClientChildOnlyInGameActor@pa@@)  +104 -> 0x3CA8B8A70E8
  [0x3CA8B8A70E8] = 0x3C9FCB8C000  +64 -> 0x3C9FCB8C040
  [0x3C9FCB8C040] = 0x3CA8B7A6180  (.?AVClientCharacterControlActorComponent@pa@@)  +320 -> 0x3CA8B7A62C0
  [0x3CA8B7A62C0] = 0x3C9E61C3F00  +704 -> 0x3C9E61C41C0
  [0x3C9E61C41C0] = 0x3C9E7AE0A00  +128 -> 0x3C9E7AE0A80
=> 0x3C9E7AE0A80 : (-0.057, 0.000, -0.998)
```

Exit code 0.

Candidate local position at physics `+0x90`: `(-112.655, 552.443, -724.742)`. This is not a verified world position. Chunk origin and the independent world-position copy were not read.

Numerical check on the three-decimal printed axes. Magnitude of a unit axis should be near 1, and each pair's dot product near 0. The probe rounds each component to three decimal places, so this check uses those printed numbers.

| Axis | Printed vec3 | Magnitude |
|---|---|---|
| `+0x60` | `(-0.998, 0.000, 0.057)` | 0.9996 |
| `+0x70` | `(0.000, 1.000, 0.000)` | 1.0000 |
| `+0x80` | `(-0.057, 0.000, -0.998)` | 0.9996 |

Dot products: `+0x60` with `+0x70` is 0, `+0x70` with `+0x80` is 0, and `+0x60` with `+0x80` is 0. The standing axes meet a 0.01 tolerance. Whether one axis tracks facing is still unverified until the turn sample.

## 6. After walking and stopping

Operator action: walked several metres and replied `stopped`. Temporary slot `exe+0x71C09A8` again. Each command re-walked the chain. The return sample and the in-place turn have not been taken.

Command: `cdmp_probe chain exe+0x71C09A8 0x50 0x68 0x40 0x140 0x2C0 0x90 --type vec3`

Timestamp: 2026-11-12 08:51:58.633

```
  [exe+0x71C09A8] = 0x3C9F8757800  (.?AVClientActorManager@pa@@)  +80 -> 0x3C9F8757850
  [0x3C9F8757850] = 0x3CA8B8A7080  (.?AVClientChildOnlyInGameActor@pa@@)  +104 -> 0x3CA8B8A70E8
  [0x3CA8B8A70E8] = 0x3C9FCB8C000  +64 -> 0x3C9FCB8C040
  [0x3C9FCB8C040] = 0x3CA8B7A6180  (.?AVClientCharacterControlActorComponent@pa@@)  +320 -> 0x3CA8B7A62C0
  [0x3CA8B7A62C0] = 0x3C9E61C3F00  +704 -> 0x3C9E61C41C0
  [0x3C9E61C41C0] = 0x3C9E7AE0A00  +144 -> 0x3C9E7AE0A90
=> 0x3C9E7AE0A90 : (-141.741, 550.489, -679.240)
```

Exit code 0.

Command: `cdmp_probe chain exe+0x71C09A8 0x50 0x68 0x40 0x140 0x2C0 0x60 --type vec3`

Timestamp: 2026-11-12 08:51:58.687

```
  [exe+0x71C09A8] = 0x3C9F8757800  (.?AVClientActorManager@pa@@)  +80 -> 0x3C9F8757850
  [0x3C9F8757850] = 0x3CA8B8A7080  (.?AVClientChildOnlyInGameActor@pa@@)  +104 -> 0x3CA8B8A70E8
  [0x3CA8B8A70E8] = 0x3C9FCB8C000  +64 -> 0x3C9FCB8C040
  [0x3C9FCB8C040] = 0x3CA8B7A6180  (.?AVClientCharacterControlActorComponent@pa@@)  +320 -> 0x3CA8B7A62C0
  [0x3CA8B7A62C0] = 0x3C9E61C3F00  +704 -> 0x3C9E61C41C0
  [0x3C9E61C41C0] = 0x3C9E7AE0A00  +96 -> 0x3C9E7AE0A60
=> 0x3C9E7AE0A60 : (-0.828, 0.000, -0.561)
```

Exit code 0.

Command: `cdmp_probe chain exe+0x71C09A8 0x50 0x68 0x40 0x140 0x2C0 0x70 --type vec3`

Timestamp: 2026-11-12 08:51:58.716

```
  [exe+0x71C09A8] = 0x3C9F8757800  (.?AVClientActorManager@pa@@)  +80 -> 0x3C9F8757850
  [0x3C9F8757850] = 0x3CA8B8A7080  (.?AVClientChildOnlyInGameActor@pa@@)  +104 -> 0x3CA8B8A70E8
  [0x3CA8B8A70E8] = 0x3C9FCB8C000  +64 -> 0x3C9FCB8C040
  [0x3C9FCB8C040] = 0x3CA8B7A6180  (.?AVClientCharacterControlActorComponent@pa@@)  +320 -> 0x3CA8B7A62C0
  [0x3CA8B7A62C0] = 0x3C9E61C3F00  +704 -> 0x3C9E61C41C0
  [0x3C9E61C41C0] = 0x3C9E7AE0A00  +112 -> 0x3C9E7AE0A70
=> 0x3C9E7AE0A70 : (0.000, 1.000, 0.000)
```

Exit code 0.

Command: `cdmp_probe chain exe+0x71C09A8 0x50 0x68 0x40 0x140 0x2C0 0x80 --type vec3`

Timestamp: 2026-11-12 08:51:58.761

```
  [exe+0x71C09A8] = 0x3C9F8757800  (.?AVClientActorManager@pa@@)  +80 -> 0x3C9F8757850
  [0x3C9F8757850] = 0x3CA8B8A7080  (.?AVClientChildOnlyInGameActor@pa@@)  +104 -> 0x3CA8B8A70E8
  [0x3CA8B8A70E8] = 0x3C9FCB8C000  +64 -> 0x3C9FCB8C040
  [0x3C9FCB8C040] = 0x3CA8B7A6180  (.?AVClientCharacterControlActorComponent@pa@@)  +320 -> 0x3CA8B7A62C0
  [0x3CA8B7A62C0] = 0x3C9E61C3F00  +704 -> 0x3C9E61C41C0
  [0x3C9E61C41C0] = 0x3C9E7AE0A00  +128 -> 0x3C9E7AE0A80
=> 0x3C9E7AE0A80 : (0.561, 0.000, -0.828)
```

Exit code 0.

Comparison of the printed local candidates, section 5 minus this sample is the reverse of the displacement below. Displacement is this sample minus section 5:

| | X | Y | Z |
|---|---:|---:|---:|
| Standing | -112.655 | 552.443 | -724.742 |
| Stopped after walk | -141.741 | 550.489 | -679.240 |
| Delta | -29.086 | -1.954 | 45.502 |

Distance from the printed components is 54.039. The `+0x90` value changed with the walk. It remains a candidate local position. Chunk origin and the independent world copy are still unread.

The stopped axes, from the printed three-decimal values: magnitudes 1.0002, 1.0000, and 1.0002. Dot products are 0. They still meet a 0.01 tolerance. `+0x70` stayed `(0.000, 1.000, 0.000)`. `+0x60` and `+0x80` changed. The horizontal unit of the displacement is about `(-0.539, 0.843)`. Its dot product with the horizontal part of `+0x80` is -1.000, and with `+0x60` is -0.027. That alignment is a walk observation. The in-place turn is the check that still has to show an axis changing with facing.

## 7. Returned near the standing area

Operator action: walked back and replied that they were close to the starting area. Temporary slot `exe+0x71C09A8`. Each command re-walked the chain. The in-place turn has not been taken.

Command: `cdmp_probe chain exe+0x71C09A8 0x50 0x68 0x40 0x140 0x2C0 0x90 --type vec3`

Timestamp: 2026-11-12 08:53:34.212

```
  [exe+0x71C09A8] = 0x3C9F8757800  (.?AVClientActorManager@pa@@)  +80 -> 0x3C9F8757850
  [0x3C9F8757850] = 0x3CA8B8A7080  (.?AVClientChildOnlyInGameActor@pa@@)  +104 -> 0x3CA8B8A70E8
  [0x3CA8B8A70E8] = 0x3C9FCB8C000  +64 -> 0x3C9FCB8C040
  [0x3C9FCB8C040] = 0x3CA8B7A6180  (.?AVClientCharacterControlActorComponent@pa@@)  +320 -> 0x3CA8B7A62C0
  [0x3CA8B7A62C0] = 0x3C9E61C3F00  +704 -> 0x3C9E61C41C0
  [0x3C9E61C41C0] = 0x3C9E7AE0A00  +144 -> 0x3C9E7AE0A90
=> 0x3C9E7AE0A90 : (-115.798, 552.792, -718.831)
```

Exit code 0.

Command: `cdmp_probe chain exe+0x71C09A8 0x50 0x68 0x40 0x140 0x2C0 0x60 --type vec3`

Timestamp: 2026-11-12 08:53:34.260

```
  [exe+0x71C09A8] = 0x3C9F8757800  (.?AVClientActorManager@pa@@)  +80 -> 0x3C9F8757850
  [0x3C9F8757850] = 0x3CA8B8A7080  (.?AVClientChildOnlyInGameActor@pa@@)  +104 -> 0x3CA8B8A70E8
  [0x3CA8B8A70E8] = 0x3C9FCB8C000  +64 -> 0x3C9FCB8C040
  [0x3C9FCB8C040] = 0x3CA8B7A6180  (.?AVClientCharacterControlActorComponent@pa@@)  +320 -> 0x3CA8B7A62C0
  [0x3CA8B7A62C0] = 0x3C9E61C3F00  +704 -> 0x3C9E61C41C0
  [0x3C9E61C41C0] = 0x3C9E7AE0A00  +96 -> 0x3C9E7AE0A60
=> 0x3C9E7AE0A60 : (0.777, -0.000, 0.630)
```

Exit code 0.

Command: `cdmp_probe chain exe+0x71C09A8 0x50 0x68 0x40 0x140 0x2C0 0x70 --type vec3`

Timestamp: 2026-11-12 08:53:34.297

```
  [exe+0x71C09A8] = 0x3C9F8757800  (.?AVClientActorManager@pa@@)  +80 -> 0x3C9F8757850
  [0x3C9F8757850] = 0x3CA8B8A7080  (.?AVClientChildOnlyInGameActor@pa@@)  +104 -> 0x3CA8B8A70E8
  [0x3CA8B8A70E8] = 0x3C9FCB8C000  +64 -> 0x3C9FCB8C040
  [0x3C9FCB8C040] = 0x3CA8B7A6180  (.?AVClientCharacterControlActorComponent@pa@@)  +320 -> 0x3CA8B7A62C0
  [0x3CA8B7A62C0] = 0x3C9E61C3F00  +704 -> 0x3C9E61C41C0
  [0x3C9E61C41C0] = 0x3C9E7AE0A00  +112 -> 0x3C9E7AE0A70
=> 0x3C9E7AE0A70 : (0.000, 1.000, 0.000)
```

Exit code 0.

Command: `cdmp_probe chain exe+0x71C09A8 0x50 0x68 0x40 0x140 0x2C0 0x80 --type vec3`

Timestamp: 2026-11-12 08:53:34.331

```
  [exe+0x71C09A8] = 0x3C9F8757800  (.?AVClientActorManager@pa@@)  +80 -> 0x3C9F8757850
  [0x3C9F8757850] = 0x3CA8B8A7080  (.?AVClientChildOnlyInGameActor@pa@@)  +104 -> 0x3CA8B8A70E8
  [0x3CA8B8A70E8] = 0x3C9FCB8C000  +64 -> 0x3C9FCB8C040
  [0x3C9FCB8C040] = 0x3CA8B7A6180  (.?AVClientCharacterControlActorComponent@pa@@)  +320 -> 0x3CA8B7A62C0
  [0x3CA8B7A62C0] = 0x3C9E61C3F00  +704 -> 0x3C9E61C41C0
  [0x3C9E61C41C0] = 0x3C9E7AE0A00  +128 -> 0x3C9E7AE0A80
=> 0x3C9E7AE0A80 : (-0.630, 0.000, 0.777)
```

Exit code 0.

Printed local candidates:

| Sample | X | Y | Z | Distance from standing |
|---|---:|---:|---:|---:|
| Standing, section 5 | -112.655 | 552.443 | -724.742 | 0 |
| Stopped after the walk, section 6 | -141.741 | 550.489 | -679.240 | 54.039 |
| Returned, this section | -115.798 | 552.792 | -718.831 | 6.703 |

Distance from the away sample to this sample is 47.389. The `+0x90` value went out and came back near the start. It is a local position that tracks this round trip. It is not a verified world position. Chunk origin and the independent world copy remain unread.

Returned axes, from the printed three-decimal values: magnitudes 1.0003, 1.0000, and 1.0003. Dot products are 0. They meet a 0.01 tolerance. `+0x70` is still `(0.000, 1.000, 0.000)`. `+0x60` and `+0x80` differ from both earlier samples. The in-place turn is still required before an axis is called the facing axis.

## 8. In-place turn and three-slot position check

Operator action: turned in place and replied `turned`. No slot was adopted. `exe+0x71C09A8` remains the temporary sampling slot for the axes. The other two slots were read only for the local-position cross-check. Each command re-walked the chain.

Command: `cdmp_probe chain exe+0x71C09A8 0x50 0x68 0x40 0x140 0x2C0 0x90 --type vec3`

Timestamp: 2026-11-12 08:55:04.037

```
  [exe+0x71C09A8] = 0x3C9F8757800  (.?AVClientActorManager@pa@@)  +80 -> 0x3C9F8757850
  [0x3C9F8757850] = 0x3CA8B8A7080  (.?AVClientChildOnlyInGameActor@pa@@)  +104 -> 0x3CA8B8A70E8
  [0x3CA8B8A70E8] = 0x3C9FCB8C000  +64 -> 0x3C9FCB8C040
  [0x3C9FCB8C040] = 0x3CA8B7A6180  (.?AVClientCharacterControlActorComponent@pa@@)  +320 -> 0x3CA8B7A62C0
  [0x3CA8B7A62C0] = 0x3C9E61C3F00  +704 -> 0x3C9E61C41C0
  [0x3C9E61C41C0] = 0x3C9E7AE0A00  +144 -> 0x3C9E7AE0A90
=> 0x3C9E7AE0A90 : (-115.566, 552.821, -718.548)
```

Exit code 0.

Command: `cdmp_probe chain exe+0x71C09A8 0x50 0x68 0x40 0x140 0x2C0 0x60 --type vec3`

Timestamp: 2026-11-12 08:55:04.102

```
  [exe+0x71C09A8] = 0x3C9F8757800  (.?AVClientActorManager@pa@@)  +80 -> 0x3C9F8757850
  [0x3C9F8757850] = 0x3CA8B8A7080  (.?AVClientChildOnlyInGameActor@pa@@)  +104 -> 0x3CA8B8A70E8
  [0x3CA8B8A70E8] = 0x3C9FCB8C000  +64 -> 0x3C9FCB8C040
  [0x3C9FCB8C040] = 0x3CA8B7A6180  (.?AVClientCharacterControlActorComponent@pa@@)  +320 -> 0x3CA8B7A62C0
  [0x3CA8B7A62C0] = 0x3C9E61C3F00  +704 -> 0x3C9E61C41C0
  [0x3C9E61C41C0] = 0x3C9E7AE0A00  +96 -> 0x3C9E7AE0A60
=> 0x3C9E7AE0A60 : (-0.987, 0.000, 0.161)
```

Exit code 0.

Command: `cdmp_probe chain exe+0x71C09A8 0x50 0x68 0x40 0x140 0x2C0 0x70 --type vec3`

Timestamp: 2026-11-12 08:55:04.143

```
  [exe+0x71C09A8] = 0x3C9F8757800  (.?AVClientActorManager@pa@@)  +80 -> 0x3C9F8757850
  [0x3C9F8757850] = 0x3CA8B8A7080  (.?AVClientChildOnlyInGameActor@pa@@)  +104 -> 0x3CA8B8A70E8
  [0x3CA8B8A70E8] = 0x3C9FCB8C000  +64 -> 0x3C9FCB8C040
  [0x3C9FCB8C040] = 0x3CA8B7A6180  (.?AVClientCharacterControlActorComponent@pa@@)  +320 -> 0x3CA8B7A62C0
  [0x3CA8B7A62C0] = 0x3C9E61C3F00  +704 -> 0x3C9E61C41C0
  [0x3C9E61C41C0] = 0x3C9E7AE0A00  +112 -> 0x3C9E7AE0A70
=> 0x3C9E7AE0A70 : (0.000, 1.000, 0.000)
```

Exit code 0.

Command: `cdmp_probe chain exe+0x71C09A8 0x50 0x68 0x40 0x140 0x2C0 0x80 --type vec3`

Timestamp: 2026-11-12 08:55:04.183

```
  [exe+0x71C09A8] = 0x3C9F8757800  (.?AVClientActorManager@pa@@)  +80 -> 0x3C9F8757850
  [0x3C9F8757850] = 0x3CA8B8A7080  (.?AVClientChildOnlyInGameActor@pa@@)  +104 -> 0x3CA8B8A70E8
  [0x3CA8B8A70E8] = 0x3C9FCB8C000  +64 -> 0x3C9FCB8C040
  [0x3C9FCB8C040] = 0x3CA8B7A6180  (.?AVClientCharacterControlActorComponent@pa@@)  +320 -> 0x3CA8B7A62C0
  [0x3CA8B7A62C0] = 0x3C9E61C3F00  +704 -> 0x3C9E61C41C0
  [0x3C9E61C41C0] = 0x3C9E7AE0A00  +128 -> 0x3C9E7AE0A80
=> 0x3C9E7AE0A80 : (-0.161, 0.000, -0.987)
```

Exit code 0.

Command: `cdmp_probe chain exe+0x7233960 0x50 0x68 0x40 0x140 0x2C0 0x90 --type vec3`

Timestamp: 2026-11-12 08:55:04.224

```
  [exe+0x7233960] = 0x3C9F8757800  (.?AVClientActorManager@pa@@)  +80 -> 0x3C9F8757850
  [0x3C9F8757850] = 0x3CA8B8A7080  (.?AVClientChildOnlyInGameActor@pa@@)  +104 -> 0x3CA8B8A70E8
  [0x3CA8B8A70E8] = 0x3C9FCB8C000  +64 -> 0x3C9FCB8C040
  [0x3C9FCB8C040] = 0x3CA8B7A6180  (.?AVClientCharacterControlActorComponent@pa@@)  +320 -> 0x3CA8B7A62C0
  [0x3CA8B7A62C0] = 0x3C9E61C3F00  +704 -> 0x3C9E61C41C0
  [0x3C9E61C41C0] = 0x3C9E7AE0A00  +144 -> 0x3C9E7AE0A90
=> 0x3C9E7AE0A90 : (-115.566, 552.821, -718.548)
```

Exit code 0.

Command: `cdmp_probe chain exe+0x7233AA0 0x50 0x68 0x40 0x140 0x2C0 0x90 --type vec3`

Timestamp: 2026-11-12 08:55:04.266

```
  [exe+0x7233AA0] = 0x3C9F8757800  (.?AVClientActorManager@pa@@)  +80 -> 0x3C9F8757850
  [0x3C9F8757850] = 0x3CA8B8A7080  (.?AVClientChildOnlyInGameActor@pa@@)  +104 -> 0x3CA8B8A70E8
  [0x3CA8B8A70E8] = 0x3C9FCB8C000  +64 -> 0x3C9FCB8C040
  [0x3C9FCB8C040] = 0x3CA8B7A6180  (.?AVClientCharacterControlActorComponent@pa@@)  +320 -> 0x3CA8B7A62C0
  [0x3CA8B7A62C0] = 0x3C9E61C3F00  +704 -> 0x3C9E61C41C0
  [0x3C9E61C41C0] = 0x3C9E7AE0A00  +144 -> 0x3C9E7AE0A90
=> 0x3C9E7AE0A90 : (-115.566, 552.821, -718.548)
```

Exit code 0.

Position stayed nearly still. The section 7 return sample was `(-115.798, 552.792, -718.831)`. This sample is `(-115.566, 552.821, -718.548)`, a distance of 0.367. All three manager slots printed that same vec3 and the same object addresses. The slots agree on the candidate local position. Chunk origin and the independent world copy are still unread, so this is not a verified world position. No slot is chosen as permanent.

Axes after the turn, from the printed three-decimal values:

| Axis | Before the turn, section 7 | After the turn | Magnitude |
|---|---|---|---:|
| `+0x60` | `(0.777, -0.000, 0.630)` | `(-0.987, 0.000, 0.161)` | 1.0000 |
| `+0x70` | `(0.000, 1.000, 0.000)` | `(0.000, 1.000, 0.000)` | 1.0000 |
| `+0x80` | `(-0.630, 0.000, 0.777)` | `(-0.161, 0.000, -0.987)` | 1.0000 |

Dot products of the after-turn axes are 0. They meet a 0.01 tolerance. `+0x70` did not change, so it stayed the vertical axis. The horizontal parts of `+0x60` and `+0x80` both rotated by the same angle: dot product with the previous horizontal vector is -0.665, and the 2D cross product is 0.747, which is about 132 degrees. They remain a pair. Which of those two is forward is not assigned. During the earlier walk, travel was opposite the then-current `+0x80`; that makes `-0x80` a candidate forward direction only.

## 9. Skill-screen health 600

Operator report at this sample: the skill screen shows health 600. A live in-game health bar was not visible. Temporary slot `exe+0x71C09A8`. Each command re-walked the chain. No id was assumed.

Command: `cdmp_probe chain exe+0x71C09A8 0x50 0x68 0x20`

Timestamp: 2026-11-12 08:57:34.247

```
  [exe+0x71C09A8] = 0x3C9F8757800  (.?AVClientActorManager@pa@@)  +80 -> 0x3C9F8757850
  [0x3C9F8757850] = 0x3CA8B8A7080  (.?AVClientChildOnlyInGameActor@pa@@)  +104 -> 0x3CA8B8A70E8
  [0x3CA8B8A70E8] = 0x3C9FCB8C000  +32 -> 0x3C9FCB8C020
=> 0x3C9FCB8C020 : 0x3CAA5392800  -> object .?AVClientStatusActorComponent@pa@@
```

Exit code 0. `+32` is decimal for hex `0x20`.

Command: `cdmp_probe chain exe+0x71C09A8 0x50 0x68 0x20 0x18 0x60 --type u32`

Timestamp: 2026-11-12 08:57:34.312

```
  [exe+0x71C09A8] = 0x3C9F8757800  (.?AVClientActorManager@pa@@)  +80 -> 0x3C9F8757850
  [0x3C9F8757850] = 0x3CA8B8A7080  (.?AVClientChildOnlyInGameActor@pa@@)  +104 -> 0x3CA8B8A70E8
  [0x3CA8B8A70E8] = 0x3C9FCB8C000  +32 -> 0x3C9FCB8C020
  [0x3C9FCB8C020] = 0x3CAA5392800  (.?AVClientStatusActorComponent@pa@@)  +24 -> 0x3CAA5392818
  [0x3CAA5392818] = 0x3CA8B8A71C0  +96 -> 0x3CA8B8A7220
=> 0x3CA8B8A7220 : 20 (0x14)
```

Exit code 0. `+24` is hex `0x18`. `+96` is hex `0x60`. Count is 20.

Command: `cdmp_probe chain exe+0x71C09A8 0x50 0x68 0x20 0x18 0x58`

Timestamp: 2026-11-12 08:57:34.363

```
  [exe+0x71C09A8] = 0x3C9F8757800  (.?AVClientActorManager@pa@@)  +80 -> 0x3C9F8757850
  [0x3C9F8757850] = 0x3CA8B8A7080  (.?AVClientChildOnlyInGameActor@pa@@)  +104 -> 0x3CA8B8A70E8
  [0x3CA8B8A70E8] = 0x3C9FCB8C000  +32 -> 0x3C9FCB8C020
  [0x3C9FCB8C020] = 0x3CAA5392800  (.?AVClientStatusActorComponent@pa@@)  +24 -> 0x3CAA5392818
  [0x3CAA5392818] = 0x3CA8B8A71C0  +88 -> 0x3CA8B8A7218
=> 0x3CA8B8A7218 : 0x3CAC87C8400
```

Exit code 0. `+88` is hex `0x58`.

Command: `cdmp_probe dump 0x3CAC87C8400 --size 2880`

Timestamp: 2026-11-12 08:57:49.378. Size is 20 entries times 144 (`0x90`). Exit code 0. The raw dump follows the parsed table.

The 2976 entry hypothesis, not adopted: int64 id at `+0x00`, current at `+0x08`, max at `+0x18`, divided by 1000. Under that reading:

| Index | Id | Current | Current/1000 | Max | Max/1000 |
|---:|---:|---:|---:|---:|---:|
| 0 | 0 | 900000 | 900 | 900000 | 900 |
| 1 | 1 | 0 | 0 | 0 | 0 |
| 2 | 2 | 0 | 0 | 0 | 0 |
| 3 | 18 | 39525 | not divisible | 100000 | 100 |
| 4 | 17 | 73145 | not divisible | 100000 | 100 |
| 5 | 19 | 23948 | not divisible | 100000 | 100 |
| 6 | 20 | 72309 | not divisible | 100000 | 100 |
| 7 | 55 | 93081 | not divisible | 400000 | 400 |
| 8 | 84 | 0 | 0 | 10000 | 10 |
| 9 | 82 | 0 | 0 | 100000 | 100 |
| 10 | 32 | 0 | 0 | 100000 | 100 |
| 11 | 33 | 100000 | 100 | 100000 | 100 |
| 12 | 22 | 260000 | 260 | 260000 | 260 |
| 13 | 23 | 70000 | 70 | 70000 | 70 |
| 14 | 24 | 120000 | 120 | 120000 | 120 |
| 15 | 86 | 100000 | 100 | 100000 | 100 |
| 16 | 81 | 0 | 0 | 100000 | 100 |
| 17 | 56 | 0 | 0 | 400000 | 400 |
| 18 | 83 | 0 | 0 | 100000 | 100 |
| 19 | 34 | 0 | 0 | 100000 | 100 |

No int64, either 32-bit half, or float in this 2880-byte dump equals 600 or 600000. Id 0 is 900/900 under the divide-by-1000 reading, which does not equal the skill-screen 600. Health is unverified. The status component class is verified. The entry layout above is a hypothesis that produced this table; it is not confirmed as the skill-screen health.

```
+0000  0000000000000000        0.0000       0.0000  
+0008  00000000000DBBA0                     0.0000  ptr
+0010  00000000000003E8                     0.0000  
+0018  00000000000DBBA0                     0.0000  ptr
+0020  0000000000000000        0.0000       0.0000  
+0028  0000000000000000        0.0000       0.0000  
+0030  0000000000000000        0.0000       0.0000  
+0038  0000000000025820                     0.0000  ptr
+0040  0000000000000000        0.0000       0.0000  
+0048  0000000000000002                     0.0000  
+0050  0000000000000000        0.0000       0.0000  
+0058  00000000000493E0                     0.0000  ptr
+0060  0000000000000000        0.0000       0.0000  
+0068  0000000000000000        0.0000       0.0000  
+0070  0000000000000000        0.0000       0.0000  
+0078  0000000000000000        0.0000       0.0000  
+0080  00000000000003E8                     0.0000  
+0088  0000000000000000        0.0000       0.0000  
+0090  0000000000000001                     0.0000  
+0098  0000000000000000        0.0000       0.0000  
+00A0  0000000000000000        0.0000       0.0000  
+00A8  0000000000000000        0.0000       0.0000  
+00B0  0000000000000000        0.0000       0.0000  
+00B8  0000000000000000        0.0000       0.0000  
+00C0  0000000000000000        0.0000       0.0000  
+00C8  0000000000025C75                     0.0000  ptr
+00D0  0000000000000000        0.0000       0.0000  
+00D8  0000000000000000        0.0000       0.0000  
+00E0  0000000000000000        0.0000       0.0000  
+00E8  0000000000000000        0.0000       0.0000  
+00F0  0000000000000000        0.0000       0.0000  
+00F8  0000000000000000        0.0000       0.0000  
+0100  0000000000000000        0.0000       0.0000  
+0108  0000000000000000        0.0000       0.0000  
+0110  0000000000000000        0.0000       0.0000  
+0118  0000000000000000        0.0000       0.0000  
+0120  0000000000000002                     0.0000  
+0128  0000000000000000        0.0000       0.0000  
+0130  0000000000000000        0.0000       0.0000  
+0138  0000000000000000        0.0000       0.0000  
+0140  0000000000000000        0.0000       0.0000  
+0148  0000000000000000        0.0000       0.0000  
+0150  0000000000000000        0.0000       0.0000  
+0158  0000000000025C75                     0.0000  ptr
+0160  0000000000000000        0.0000       0.0000  
+0168  0000000000000000        0.0000       0.0000  
+0170  0000000000000000        0.0000       0.0000  
+0178  0000000000000000        0.0000       0.0000  
+0180  0000000000000000        0.0000       0.0000  
+0188  0000000000000000        0.0000       0.0000  
+0190  0000000000000000        0.0000       0.0000  
+0198  0000000000000000        0.0000       0.0000  
+01A0  0000000000000000        0.0000       0.0000  
+01A8  0000000000000000        0.0000       0.0000  
+01B0  0000000000000012                     0.0000  
+01B8  0000000000009A65                     0.0000  
+01C0  FFFFFFFFFFFFFFCE                             
+01C8  00000000000186A0                     0.0000  
+01D0  0000000000000000        0.0000       0.0000  
+01D8  0000000000000000        0.0000       0.0000  
+01E0  0000000000000000        0.0000       0.0000  
+01E8  0000000000025C75                     0.0000  ptr
+01F0  0000000000000000        0.0000       0.0000  
+01F8  0000000000000000        0.0000       0.0000  
+0200  0000000000000000        0.0000       0.0000  
+0208  00000000000186A0                     0.0000  
+0210  0000000000000000        0.0000       0.0000  
+0218  0000000000000000        0.0000       0.0000  
+0220  0000000000000000        0.0000       0.0000  
+0228  0000000000000000        0.0000       0.0000  
+0230  FFFFFFFFFFFFFFCE                             
+0238  0000000000000000        0.0000       0.0000  
+0240  0000000000000011                     0.0000  
+0248  0000000000011DB9                     0.0000  ptr
+0250  FFFFFFFFFFFFFFCE                             
+0258  00000000000186A0                     0.0000  
+0260  0000000000000000        0.0000       0.0000  
+0268  0000000000000000        0.0000       0.0000  
+0270  0000000000000000        0.0000       0.0000  
+0278  0000000000025C75                     0.0000  ptr
+0280  0000000000000000        0.0000       0.0000  
+0288  0000000000000000        0.0000       0.0000  
+0290  0000000000000000        0.0000       0.0000  
+0298  00000000000186A0                     0.0000  
+02A0  0000000000000000        0.0000       0.0000  
+02A8  0000000000000000        0.0000       0.0000  
+02B0  0000000000000000        0.0000       0.0000  
+02B8  0000000000000000        0.0000       0.0000  
+02C0  FFFFFFFFFFFFFFCE                             
+02C8  0000000000000000        0.0000       0.0000  
+02D0  0000000000000013                     0.0000  
+02D8  0000000000005D8C                     0.0000  
+02E0  FFFFFFFFFFFFFFCE                             
+02E8  00000000000186A0                     0.0000  
+02F0  0000000000000000        0.0000       0.0000  
+02F8  0000000000000000        0.0000       0.0000  
+0300  0000000000000000        0.0000       0.0000  
+0308  0000000000025C75                     0.0000  ptr
+0310  0000000000000000        0.0000       0.0000  
+0318  0000000000000000        0.0000       0.0000  
+0320  0000000000000000        0.0000       0.0000  
+0328  00000000000186A0                     0.0000  
+0330  0000000000000000        0.0000       0.0000  
+0338  0000000000000000        0.0000       0.0000  
+0340  0000000000000000        0.0000       0.0000  
+0348  0000000000000000        0.0000       0.0000  
+0350  FFFFFFFFFFFFFFCE                             
+0358  0000000000000000        0.0000       0.0000  
+0360  0000000000000014                     0.0000  
+0368  0000000000011A75                     0.0000  ptr
+0370  FFFFFFFFFFFFFFCE                             
+0378  00000000000186A0                     0.0000  
+0380  0000000000000000        0.0000       0.0000  
+0388  0000000000000000        0.0000       0.0000  
+0390  0000000000000000        0.0000       0.0000  
+0398  0000000000025C75                     0.0000  ptr
+03A0  0000000000000000        0.0000       0.0000  
+03A8  0000000000000000        0.0000       0.0000  
+03B0  0000000000000000        0.0000       0.0000  
+03B8  00000000000186A0                     0.0000  
+03C0  0000000000000000        0.0000       0.0000  
+03C8  0000000000000000        0.0000       0.0000  
+03D0  0000000000000000        0.0000       0.0000  
+03D8  0000000000000000        0.0000       0.0000  
+03E0  FFFFFFFFFFFFFFCE                             
+03E8  0000000000000000        0.0000       0.0000  
+03F0  0000000000000037                     0.0000  
+03F8  0000000000016B99                     0.0000  
+0400  0000000000000000        0.0000       0.0000  
+0408  0000000000061A80                     0.0000  ptr
+0410  0000000000000000        0.0000       0.0000  
+0418  FFFFFFFFFFF9E580                             
+0420  0000000000000000        0.0000       0.0000  
+0428  0000000000025820                     0.0000  ptr
+0430  0000000000000000        0.0000       0.0000  
+0438  00000000000003A4                     0.0000  
+0440  0000000000000000        0.0000       0.0000  
+0448  0000000000061A80                     0.0000  ptr
+0450  FFFFFFFFFFF9E580                             
+0458  0000000000000000        0.0000       0.0000  
+0460  0000000000000000        0.0000       0.0000  
+0468  0000000000000000        0.0000       0.0000  
+0470  0000000000000000        0.0000       0.0000  
+0478  0000000000000000        0.0000       0.0000  
+0480  0000000000000054                     0.0000  
+0488  0000000000000000        0.0000       0.0000  
+0490  0000000000000000        0.0000       0.0000  
+0498  0000000000002710                     0.0000  
+04A0  0000000000000000        0.0000       0.0000  
+04A8  0000000000000000        0.0000       0.0000  
+04B0  0000000000000000        0.0000       0.0000  
+04B8  0000000000025C75                     0.0000  ptr
+04C0  0000000000000000        0.0000       0.0000  
+04C8  0000000000000000        0.0000       0.0000  
+04D0  0000000000000000        0.0000       0.0000  
+04D8  0000000000002710                     0.0000  
+04E0  0000000000000000        0.0000       0.0000  
+04E8  0000000000000000        0.0000       0.0000  
+04F0  0000000000000000        0.0000       0.0000  
+04F8  0000000000000000        0.0000       0.0000  
+0500  0000000000000000        0.0000       0.0000  
+0508  0000000000000000        0.0000       0.0000  
+0510  0000000000000052                     0.0000  
+0518  0000000000000000        0.0000       0.0000  
+0520  FFFFFFFFFFFFFC18                             
+0528  00000000000186A0                     0.0000  
+0530  0000000000000000        0.0000       0.0000  
+0538  0000000000000000        0.0000       0.0000  
+0540  0000000000000000        0.0000       0.0000  
+0548  0000000000025C75                     0.0000  ptr
+0550  0000000000000000        0.0000       0.0000  
+0558  0000000000000000        0.0000       0.0000  
+0560  0000000000000000        0.0000       0.0000  
+0568  00000000000186A0                     0.0000  
+0570  0000000000000000        0.0000       0.0000  
+0578  0000000000000000        0.0000       0.0000  
+0580  0000000000000000        0.0000       0.0000  
+0588  0000000000000000        0.0000       0.0000  
+0590  FFFFFFFFFFFFFC18                             
+0598  0000000000000000        0.0000       0.0000  
+05A0  0000000000000020                     0.0000  
+05A8  0000000000000000        0.0000       0.0000  
+05B0  0000000000000000        0.0000       0.0000  
+05B8  00000000000186A0                     0.0000  
+05C0  0000000000000000        0.0000       0.0000  
+05C8  0000000000000000        0.0000       0.0000  
+05D0  0000000000000000        0.0000       0.0000  
+05D8  0000000000025820                     0.0000  ptr
+05E0  0000000000000000        0.0000       0.0000  
+05E8  0000000000000000        0.0000       0.0000  
+05F0  0000000000000000        0.0000       0.0000  
+05F8  00000000000186A0                     0.0000  
+0600  0000000000000000        0.0000       0.0000  
+0608  0000000000000000        0.0000       0.0000  
+0610  0000000000000000        0.0000       0.0000  
+0618  0000000000000000        0.0000       0.0000  
+0620  0000000000000000        0.0000       0.0000  
+0628  0000000000000000        0.0000       0.0000  
+0630  0000000000000021                     0.0000  
+0638  00000000000186A0                     0.0000  
+0640  0000000000000000        0.0000       0.0000  
+0648  00000000000186A0                     0.0000  
+0650  0000000000000000        0.0000       0.0000  
+0658  0000000000000000        0.0000       0.0000  
+0660  0000000000000000        0.0000       0.0000  
+0668  0000000000025C75                     0.0000  ptr
+0670  0000000000000000        0.0000       0.0000  
+0678  0000000000000000        0.0000       0.0000  
+0680  0000000000000000        0.0000       0.0000  
+0688  00000000000186A0                     0.0000  
+0690  0000000000000000        0.0000       0.0000  
+0698  0000000000000000        0.0000       0.0000  
+06A0  0000000000000000        0.0000       0.0000  
+06A8  0000000000000000        0.0000       0.0000  
+06B0  0000000000000000        0.0000       0.0000  
+06B8  0000000000000000        0.0000       0.0000  
+06C0  0000000000000016                     0.0000  
+06C8  000000000003F7A0                     0.0000  ptr
+06D0  0000000000019640                     0.0000  
+06D8  000000000003F7A0                     0.0000  ptr
+06E0  0000000000000000        0.0000       0.0000  
+06E8  0000000000000000        0.0000       0.0000  
+06F0  000000000000FDE8                     0.0000  
+06F8  00000000000494EB                     0.0000  ptr
+0700  0000000000000000        0.0000       0.0000  
+0708  000000000000008E                     0.0000  
+0710  0000000000000000        0.0000       0.0000  
+0718  000000000001D4C0                     0.0000  
+0720  0000000000000000        0.0000       0.0000  
+0728  0000000000000000        0.0000       0.0000  
+0730  0000000000000000        0.0000       0.0000  
+0738  0000000000000000        0.0000       0.0000  
+0740  0000000000000000        0.0000       0.0000  
+0748  0000000000000000        0.0000       0.0000  
+0750  0000000000000017                     0.0000  
+0758  0000000000011170                     0.0000  ptr
+0760  0000000000000064                     0.0000  
+0768  0000000000011170                     0.0000  ptr
+0770  0000000000000000        0.0000       0.0000  
+0778  0000000000000000        0.0000       0.0000  
+0780  0000000000000000        0.0000       0.0000  
+0788  0000000000025820                     0.0000  ptr
+0790  0000000000000000        0.0000       0.0000  
+0798  0000000000000002                     0.0000  
+07A0  0000000000000000        0.0000       0.0000  
+07A8  0000000000007530                     0.0000  
+07B0  0000000000000000        0.0000       0.0000  
+07B8  0000000000000000        0.0000       0.0000  
+07C0  0000000000000000        0.0000       0.0000  
+07C8  0000000000000000        0.0000       0.0000  
+07D0  0000000000000064                     0.0000  
+07D8  0000000000000000        0.0000       0.0000  
+07E0  0000000000000018                     0.0000  
+07E8  000000000001D4C0                     0.0000  
+07F0  00000000000186A0                     0.0000  
+07F8  000000000001D4C0                     0.0000  
+0800  0000000000000000        0.0000       0.0000  
+0808  0000000000000000        0.0000       0.0000  
+0810  000000000000FDE8                     0.0000  
+0818  0000000000025C75                     0.0000  ptr
+0820  0000000000000000        0.0000       0.0000  
+0828  0000000000000000        0.0000       0.0000  
+0830  0000000000000000        0.0000       0.0000  
+0838  000000000001D4C0                     0.0000  
+0840  0000000000000000        0.0000       0.0000  
+0848  0000000000000000        0.0000       0.0000  
+0850  0000000000000000        0.0000       0.0000  
+0858  0000000000000000        0.0000       0.0000  
+0860  00000000000186A0                     0.0000  
+0868  0000000000000000        0.0000       0.0000  
+0870  0000000000000056                     0.0000  
+0878  00000000000186A0                     0.0000  
+0880  FFFFFFFFFFFFFC18                             
+0888  00000000000186A0                     0.0000  
+0890  0000000000000000        0.0000       0.0000  
+0898  0000000000000000        0.0000       0.0000  
+08A0  0000000000000000        0.0000       0.0000  
+08A8  0000000000025C75                     0.0000  ptr
+08B0  0000000000000000        0.0000       0.0000  
+08B8  0000000000000000        0.0000       0.0000  
+08C0  0000000000000000        0.0000       0.0000  
+08C8  00000000000186A0                     0.0000  
+08D0  0000000000000000        0.0000       0.0000  
+08D8  0000000000000000        0.0000       0.0000  
+08E0  0000000000000000        0.0000       0.0000  
+08E8  0000000000000000        0.0000       0.0000  
+08F0  FFFFFFFFFFFFFC18                             
+08F8  0000000000000000        0.0000       0.0000  
+0900  0000000000000051                     0.0000  
+0908  0000000000000000        0.0000       0.0000  
+0910  FFFFFFFFFFFE7960                             
+0918  00000000000186A0                     0.0000  
+0920  0000000000000000        0.0000       0.0000  
+0928  0000000000000000        0.0000       0.0000  
+0930  0000000000000000        0.0000       0.0000  
+0938  0000000000025C75                     0.0000  ptr
+0940  0000000000000000        0.0000       0.0000  
+0948  0000000000000000        0.0000       0.0000  
+0950  0000000000000000        0.0000       0.0000  
+0958  00000000000186A0                     0.0000  
+0960  0000000000000000        0.0000       0.0000  
+0968  0000000000000000        0.0000       0.0000  
+0970  0000000000000000        0.0000       0.0000  
+0978  0000000000000000        0.0000       0.0000  
+0980  FFFFFFFFFFFE7960                             
+0988  0000000000000000        0.0000       0.0000  
+0990  0000000000000038                     0.0000  
+0998  0000000000000000        0.0000       0.0000  
+09A0  0000000000000000        0.0000       0.0000  
+09A8  0000000000061A80                     0.0000  ptr
+09B0  0000000000000000        0.0000       0.0000  
+09B8  0000000000000000        0.0000       0.0000  
+09C0  0000000000000000        0.0000       0.0000  
+09C8  0000000000025820                     0.0000  ptr
+09D0  0000000000000000        0.0000       0.0000  
+09D8  0000000000000000        0.0000       0.0000  
+09E0  0000000000000000        0.0000       0.0000  
+09E8  0000000000061A80                     0.0000  ptr
+09F0  0000000000000000        0.0000       0.0000  
+09F8  0000000000000000        0.0000       0.0000  
+0A00  0000000000000000        0.0000       0.0000  
+0A08  0000000000000000        0.0000       0.0000  
+0A10  0000000000000000        0.0000       0.0000  
+0A18  0000000000000000        0.0000       0.0000  
+0A20  0000000000000053                     0.0000  
+0A28  0000000000000000        0.0000       0.0000  
+0A30  FFFFFFFFFFFFD8F0                             
+0A38  00000000000186A0                     0.0000  
+0A40  0000000000000000        0.0000       0.0000  
+0A48  0000000000000000        0.0000       0.0000  
+0A50  0000000000000000        0.0000       0.0000  
+0A58  0000000000025C75                     0.0000  ptr
+0A60  0000000000000000        0.0000       0.0000  
+0A68  0000000000000000        0.0000       0.0000  
+0A70  0000000000000000        0.0000       0.0000  
+0A78  00000000000186A0                     0.0000  
+0A80  0000000000000000        0.0000       0.0000  
+0A88  0000000000000000        0.0000       0.0000  
+0A90  0000000000000000        0.0000       0.0000  
+0A98  0000000000000000        0.0000       0.0000  
+0AA0  FFFFFFFFFFFFD8F0                             
+0AA8  0000000000000000        0.0000       0.0000  
+0AB0  0000000000000022                     0.0000  
+0AB8  0000000000000000        0.0000       0.0000  
+0AC0  0000000000000000        0.0000       0.0000  
+0AC8  00000000000186A0                     0.0000  
+0AD0  0000000000000000        0.0000       0.0000  
+0AD8  0000000000000000        0.0000       0.0000  
+0AE0  0000000000000000        0.0000       0.0000  
+0AE8  0000000000025C75                     0.0000  ptr
+0AF0  0000000000000000        0.0000       0.0000  
+0AF8  0000000000000000        0.0000       0.0000  
+0B00  0000000000000000        0.0000       0.0000  
+0B08  00000000000186A0                     0.0000  
+0B10  0000000000000000        0.0000       0.0000  
+0B18  0000000000000000        0.0000       0.0000  
+0B20  0000000000000000        0.0000       0.0000  
+0B28  0000000000000000        0.0000       0.0000  
+0B30  0000000000000000        0.0000       0.0000  
+0B38  0000000000000000        0.0000       0.0000  
```

## 10. Loading watch, first window

Temporary slot `exe+0x71C09A8`. The operator had replied `ready to load`. The watch was started and the operator was told to load. No slot was adopted. The same process pid 48460 was still running at the start.

Command: `cdmp_probe watch exe+0x71C09A8 0x50 0x68 0x40 0x140 0x2C0 0x90 --type vec3 --seconds 90`

Watch pid 29644. Started 2026-11-12 09:01:50.085. Ended 2026-11-12 09:03:20.081. Raw watch stdout:

```
[09:01:50.064] (-115.566, 552.821, -718.548)
```

The watch prints only when the value changes. It printed once. There was no `<chain broken>` line.

Verbose position chains during that window, same command `chain exe+0x71C09A8 0x50 0x68 0x40 0x140 0x2C0 0x90 --type vec3`. Every sample used the same manager `0x3C9F8757800`, actor `0x3CA8B8A7080`, character control `0x3CA8B7A6180`, and physics object `0x3C9E7AE0A00`, and the same local position. Exit code 0 each time.

| Timestamp | Label | Local position |
|---|---|---|
| 09:01:50.093 | during-watch-0 | `(-115.566, 552.821, -718.548)` |
| 09:02:12.154 | during-watch-22s | `(-115.566, 552.821, -718.548)` |
| 09:02:36.196 | during-watch-46s | `(-115.566, 552.821, -718.548)` |
| 09:03:00.237 | during-watch-70s | `(-115.566, 552.821, -718.548)` |

One chain after the watch had ended, at 2026-11-12 09:03:47.671:

```
  [exe+0x71C09A8] = 0x3C9F8757800  (.?AVClientActorManager@pa@@)  +80 -> 0x3C9F8757850
  [0x3C9F8757850] = 0x3CB296F5000  (.?AVClientChildOnlyInGameActor@pa@@)  +104 -> 0x3CB296F5068
  [0x3CB296F5068] = 0x3C9FCB81E00  +64 -> 0x3C9FCB81E40
  [0x3C9FCB81E40] = 0x3CB28F1A780  (.?AVClientCharacterControlActorComponent@pa@@)  +320 -> 0x3CB28F1A8C0
  [0x3CB28F1A8C0] = 0x3CB1B8B3F00  +704 -> 0x3CB1B8B41C0
  [0x3CB1B8B41C0] = 0x3CB2FD78480  +144 -> 0x3CB2FD78510
=> 0x3CB2FD78510 : (0.000, 1000.200, 0.000)
```

Exit code 0. The manager object was unchanged. The actor, character control, and physics pointers were different objects, and their classes still matched. The local position was `(0.000, 1000.200, 0.000)`, which is 0.200 from `(0, 1000, 0)`. That is one sample, taken after the watch had stopped, so the change itself was not in the watch log. It is not yet a verified load park. A second 90-second watch was started to see whether this value holds and then changes. No fresh process has been tested, and no slot was adopted.

## 11. Second watch and in-world confirmation

The operator later replied `in the world`. Temporary slot `exe+0x71C09A8` for the watch. The other two slots were read once after the operator's confirmation. No slot was adopted. Process pid 48460 continued.

Command: `cdmp_probe watch exe+0x71C09A8 0x50 0x68 0x40 0x140 0x2C0 0x90 --type vec3 --seconds 90`

Watch pid 24200. Started 2026-11-12 09:04:09.027. Ended 2026-11-12 09:05:39.069. Raw watch stdout:

```
[09:04:09.033] (7999.869, 1000.196, 999.332)
[09:04:09.300] (7999.875, 1000.192, 999.339)
[09:04:09.567] (7999.837, 1000.186, 999.340)
[09:04:09.832] (7999.726, 1000.179, 999.307)
[09:04:10.098] (7999.700, 1000.172, 999.349)
[09:04:10.363] (7999.720, 1000.163, 999.401)
[09:04:10.627] (7999.804, 1000.154, 999.411)
[09:04:10.892] (7999.837, 1000.145, 999.442)
[09:04:11.156] (7999.848, 1000.135, 999.512)
[09:04:11.421] (-108.574, 552.507, -741.753)
[09:04:11.686] (-108.595, 552.497, -741.812)
[09:04:11.951] (-108.656, 552.487, -741.813)
[09:04:12.216] (-108.699, 552.478, -741.766)
[09:04:12.480] (-108.728, 552.469, -741.694)
[09:04:12.745] (-108.782, 552.461, -741.691)
[09:04:13.011] (-108.833, 552.453, -741.711)
[09:04:13.276] (-108.828, 552.446, -741.794)
[09:04:13.540] (-108.874, 552.441, -741.820)
[09:04:13.805] (-108.899, 552.436, -741.807)
[09:04:14.069] (-108.897, 552.434, -741.732)
[09:04:14.335] (-108.917, 552.432, -741.730)
[09:04:14.599] (-108.921, 552.432, -741.734)
[09:04:16.451] (-108.921, 552.431, -741.734)
```

There was no `<chain broken>` line. From 09:04:09.033 through 09:04:11.156 the local value stayed near `(8000, 1000, 999)`. At 09:04:11.421 it jumped to about `(-108.6, 552.5, -741.8)` and settled at `(-108.921, 552.431, -741.734)`. Y stayed near 1000 only in the first group. X and Z in that group were not 0, so that group is not the `(0, 1000, 0)` park.

Verbose chains during the second watch, all exit code 0, all the same pointers: manager `0x3C9F8757800`, actor `0x3CB296F5000`, character control `0x3CB28F1A780`, physics `0x3CB2FD78480`.

| Timestamp | Label | Local position |
|---|---|---|
| 09:04:09.034 | watch2-0 | `(7999.871, 1000.195, 999.333)` |
| 09:04:29.085 | watch2-20s | `(-108.921, 552.431, -741.734)` |
| 09:04:54.144 | watch2-45s | `(-108.921, 552.431, -741.734)` |
| 09:05:19.183 | watch2-70s | `(-108.921, 552.431, -741.734)` |

After the operator said the character was standing in the world, one more chain at 2026-11-12 09:05:36.091 returned the same pointers and `(-108.921, 552.431, -741.734)`.

At 2026-11-12 09:06:19.196 all three manager slots were read. Each `chain exe+SLOT` with no offset returned manager `0x3C9F8757800`. Each position chain returned `(-108.921, 552.431, -741.734)` through the same actor and physics object. The slots still agree after this load. None is adopted. A fresh process was not started.

The `(0, 1000, 0)` park remains a single sample from section 10. It was not held across the second watch. The local `+0x90` value did change, on one stable pointer chain, from the large Y-near-1000 values to the in-world value. Chunk origin was not read, so none of these vec3s is a verified world position. No recorded chain hop was unreadable. The actor replacement itself fell between the last sample of the first watch and the 09:03:47 sample. These coordinates are not a load-detection rule.

## 12. Fresh process, title screen

The operator quit and started the game from Steam, then replied `title screen`. No heap address from pid 48460 was passed to the probe. No slot was adopted.

Command: `cdmp_probe info`

Timestamp: 2026-11-12 09:12:01.726

```
PID      35980
Module   D:\SteamLibrary\steamapps\common\Crimson Desert\bin64\CrimsonDesert.exe
Base     0x140000000  size 0x13CC4000
PE timestamp 0x6AED2E85 (UNKNOWN BUILD - offsets/sigs may be invalid)
```

Exit code 0. The process path reported by the operating system was the same module. This is not pid 48460.

Command: `cdmp_probe findptr exe+0x5946C40 --heap --max 20`

Timestamp: 2026-11-12 09:12:06.431

```
  0x40F30764600
1 found in heap
```

Exit code 0. This address was found in pid 35980. It is not an address from the previous process.

Command: `cdmp_probe rtti 0x40F30764600`

Timestamp: 2026-11-12 09:12:42.915

```
.?AVClientActorManager@pa@@
```

Exit code 0.

Command: `cdmp_probe findptr 0x40F30764600 --max 20`

```
  exe+0x71C09A8
  exe+0x7233960
2 found in exe image
```

Exit code 0. `exe+0x7233AA0` was not in this list.

Commands at 2026-11-12 09:12:49.328. `+80` is decimal for hex `0x50`.

`cdmp_probe chain exe+0x71C09A8`

```
=> exe+0x71C09A8 : 0x40F30764600  -> object .?AVClientActorManager@pa@@
```

Exit code 0.

`cdmp_probe chain exe+0x71C09A8 0x50`

```
  [exe+0x71C09A8] = 0x40F30764600  (.?AVClientActorManager@pa@@)  +80 -> 0x40F30764650
=> 0x40F30764650 : 0x0
```

Exit code 0.

`cdmp_probe chain exe+0x7233960`

```
=> exe+0x7233960 : 0x40F30764600  -> object .?AVClientActorManager@pa@@
```

Exit code 0.

`cdmp_probe chain exe+0x7233960 0x50`

```
  [exe+0x7233960] = 0x40F30764600  (.?AVClientActorManager@pa@@)  +80 -> 0x40F30764650
=> 0x40F30764650 : 0x0
```

Exit code 0.

`cdmp_probe chain exe+0x7233AA0`

```
=> exe+0x7233AA0 : 0x0
```

Exit code 0.

`cdmp_probe chain exe+0x7233AA0 0x50`

```
  [exe+0x7233AA0] = 0x0  +80 -> 0x50
=> 0x50 : <unreadable>
```

Exit code 0. The slot is null, so this hop is not a class. It is not walked further.

At this title screen, `exe+0x71C09A8` and `exe+0x7233960` resolve to `ClientActorManager`. `exe+0x7233AA0` does not. The actor pointer is null, so the position chain is not available yet. The load and the walk in this process have not been run.

## 13. Fresh process after the PC restart, title screen

The operator started the game again from Steam and replied `title screen`. No heap address from pid 48460 or pid 35980 was passed to the probe. No slot was adopted.

Command: `cdmp_probe info`

Timestamp: 2026-11-12 10:08:01.158

```
PID      16920
Module   D:\SteamLibrary\steamapps\common\Crimson Desert\bin64\CrimsonDesert.exe
Base     0x140000000  size 0x13CC4000
PE timestamp 0x6AED2E85 (UNKNOWN BUILD - offsets/sigs may be invalid)
```

Exit code 0.

Command: `cdmp_probe findptr exe+0x5946C40 --heap --max 20`

Timestamp: 2026-11-12 10:08:05.915

```
  0x58302767300
1 found in heap
```

Exit code 0.

Command: `cdmp_probe rtti 0x58302767300`

Timestamp: 2026-11-12 10:08:51.125

```
.?AVClientActorManager@pa@@
```

Exit code 0.

Command: `cdmp_probe findptr 0x58302767300 --max 20`

```
  exe+0x71C09A8
  exe+0x7233960
2 found in exe image
```

Exit code 0. `exe+0x7233AA0` was not returned.

Commands at 2026-11-12 10:08:51.125. `+80` is decimal for hex `0x50`.

`cdmp_probe chain exe+0x71C09A8`

```
=> exe+0x71C09A8 : 0x58302767300  -> object .?AVClientActorManager@pa@@
```

Exit code 0.

`cdmp_probe chain exe+0x71C09A8 0x50`

```
  [exe+0x71C09A8] = 0x58302767300  (.?AVClientActorManager@pa@@)  +80 -> 0x58302767350
=> 0x58302767350 : 0x0
```

Exit code 0.

`cdmp_probe chain exe+0x7233960`

```
=> exe+0x7233960 : 0x58302767300  -> object .?AVClientActorManager@pa@@
```

Exit code 0.

`cdmp_probe chain exe+0x7233960 0x50`

```
  [exe+0x7233960] = 0x58302767300  (.?AVClientActorManager@pa@@)  +80 -> 0x58302767350
=> 0x58302767350 : 0x0
```

Exit code 0.

`cdmp_probe chain exe+0x7233AA0`

```
=> exe+0x7233AA0 : 0x0
```

Exit code 0.

`cdmp_probe chain exe+0x7233AA0 0x50`

```
  [exe+0x7233AA0] = 0x0  +80 -> 0x50
=> 0x50 : <unreadable>
```

Exit code 0. The null slot was not walked further.

This title screen matches pid 35980. `exe+0x71C09A8` and `exe+0x7233960` resolve to `ClientActorManager` with a null actor hop. `exe+0x7233AA0` is null. The load and the walk in pid 16920 have not been run.

## 14. Same fresh process, after the load

The operator loaded a save in pid 16920 and replied `in the world`. The manager was found again by scanning for vtable `exe+0x5946C40`. No heap address from another process was passed in. No slot was adopted. The walk in this process has not been done.

Command: `cdmp_probe info`

Timestamp: 2026-11-12 10:10:24.586

```
PID      16920
Module   D:\SteamLibrary\steamapps\common\Crimson Desert\bin64\CrimsonDesert.exe
Base     0x140000000  size 0x13CC4000
PE timestamp 0x6AED2E85 (UNKNOWN BUILD - offsets/sigs may be invalid)
```

Exit code 0.

Command: `cdmp_probe findptr exe+0x5946C40 --heap --max 20`

Timestamp: 2026-11-12 10:10:29.608

```
  0x58302767300
1 found in heap
```

Exit code 0. A new search returned the same address the title-screen search found in this process. That is the same process, not an address copied from pid 35980 or pid 48460.

Command: `cdmp_probe rtti 0x58302767300`

Timestamp: 2026-11-12 10:11:31.086

```
.?AVClientActorManager@pa@@
```

Exit code 0.

Command: `cdmp_probe findptr 0x58302767300 --max 20`

```
  exe+0x71C09A8
  exe+0x7233960
  exe+0x7233AA0
3 found in exe image
```

Exit code 0. `exe+0x7233AA0` was null at the title screen in section 13. After this load it stores the manager.

Each slot was then read with `chain`, `chain 0x50`, and `chain 0x50 0x68 0x40 0x140 0x2C0 0x90 --type vec3`. All three slots returned the same classes and the same local position. `+80` is hex `0x50`, `+104` is `0x68`, `+64` is `0x40`, `+320` is `0x140`, `+704` is `0x2C0`, and `+144` is `0x90`. Representative raw output from `exe+0x71C09A8`:

```
=> exe+0x71C09A8 : 0x58302767300  -> object .?AVClientActorManager@pa@@

  [exe+0x71C09A8] = 0x58302767300  (.?AVClientActorManager@pa@@)  +80 -> 0x58302767350
=> 0x58302767350 : 0x583536A7BC0  -> object .?AVClientChildOnlyInGameActor@pa@@

  [exe+0x71C09A8] = 0x58302767300  (.?AVClientActorManager@pa@@)  +80 -> 0x58302767350
  [0x58302767350] = 0x583536A7BC0  (.?AVClientChildOnlyInGameActor@pa@@)  +104 -> 0x583536A7C28
  [0x583536A7C28] = 0x58307A22600  +64 -> 0x58307A22640
  [0x58307A22640] = 0x5832F644B00  (.?AVClientCharacterControlActorComponent@pa@@)  +320 -> 0x5832F644C40
  [0x5832F644C40] = 0x582E81D3100  +704 -> 0x582E81D33C0
  [0x582E81D33C0] = 0x582E9CB2F80  +144 -> 0x582E9CB3010
=> 0x582E9CB3010 : (-108.921, 552.431, -741.734)
```

Exit code 0 on every command. `exe+0x7233960` and `exe+0x7233AA0` printed the same actor `0x583536A7BC0`, the same character control `0x5832F644B00`, and the same local position. The component-table pointer and the inner pointer had no RTTI name, as in the earlier process. This local position has not yet been shown to change with a walk in pid 16920. It is not a world position.

## 15. Walk in pid 16920, then a position-copy search

The operator walked and replied `stopped`. Each position command re-walked the chain from the slot. No slot was adopted. No heap address from another process was passed in.

Timestamp of the three reads: 2026-11-12 10:13:14.739. All three slots still named `ClientActorManager` `0x58302767300`, actor `0x583536A7BC0`, and character control `0x5832F644B00`. The local position from every slot was `(-118.612, 553.858, -745.522)`.

| Slot | Local position |
|---|---|
| `exe+0x71C09A8` | `(-118.612, 553.858, -745.522)` |
| `exe+0x7233960` | `(-118.612, 553.858, -745.522)` |
| `exe+0x7233AA0` | `(-118.612, 553.858, -745.522)` |

Displacement from the section 14 sample `(-108.921, 552.431, -741.734)` is `(-9.691, 1.427, -3.788)`. The distance is 10.502. The same field moved with the walk. This remains a local position.

The stopped position's exact bytes were read at 2026-11-12 10:13:35.804 from the field the chain had just reached:

```
+0000  440A76F0C2ED3940     -118.6118     553.8584
+0008  00000000C43A616A     -745.5221       0.0000
```

Those 12 bytes were scanned at 2026-11-12 10:13:49.293.

Image scan: `0 match(es)`. No executable-image address currently holds this local position. The old 2976 static-position RVA was already a verified non-match and was not read as a world position.

Heap scan: 3 matches, `0x582E9CB3010`, `0x583072A6470`, and `0x583ADF71470`. The first is the physics field itself. Reads of the other two at 2026-11-12 10:15:09.135 both printed `(-118.612, 553.858, -745.522)`. They are copies of the local value, not a different world-space value. A chunk origin was not separated from them.

A dump of the physics object through `+0xC0` shows the local vec3 at `+0x90` and no second world-sized vec3 in that range. The first qword is `exe+0x60CD660`. `rtti` on this object was `(no RTTI)` in the earlier process, and that RVA is not adopted. Chunk origin and an independent world position remain unverified.

## Conclusions

| Item | Status | Evidence |
|---|---|---|
| PE timestamp and mapped base | verified | Section 1. Timestamp `0x6AED2E85`, base `0x140000000`, size `0x13CC4000`, unknown-build line present. |
| ClientActorManager vtable `0x5946C40` | verified | Section 2. One subobject-0 vtable at that RVA. COL `exe+0x61A0C38`. |
| ClientChildOnlyInGameActor vtable `0x5914D58` | verified | Section 2. One subobject-0 vtable at that RVA. COL `exe+0x6185438`. |
| ClientCharacterControlActorComponent vtable `0x59459F0` | verified | Section 2. One subobject-0 vtable at that RVA. COL `exe+0x619F8C0`. |
| ClientStatusActorComponent vtable `0x591C7C0` | verified | Section 2. One subobject-0 vtable at that RVA. COL `exe+0x618A040`. |
| WorldSystem RTTI class | none shown | Section 2. `classes WorldSystem` printed `0 shown`. |
| Manager object | verified | Section 3. Heap hit `0x3C9F8757800` is `.?AVClientActorManager@pa@@`. |
| Manager slot root | three manager-rooted slots, none adopted | Sections 3 and 8. All three stored the same manager. After the turn, all three position chains printed `(-115.566, 552.821, -718.548)`. |
| Controlled actor at title screen | class verified, not null | Section 3b. All three slots read manager `+0x50` as `0x3CA8B8A7080`, `.?AVClientChildOnlyInGameActor@pa@@`. |
| Controlled actor in the world | class verified | Section 4. All three slots read `+0x50` as `0x3CA8B8A7080`, `.?AVClientChildOnlyInGameActor@pa@@`. Same object as the title screen. |
| Character control | class verified | Section 4. `+0x68` then `+0x40` is `0x3CA8B7A6180`, `.?AVClientCharacterControlActorComponent@pa@@`. |
| Component table and inner pointer | readable, class unverified | Section 4. `+0x68` is `0x3C9FCB8C000` and `+0x140` is `0x3C9E61C3F00`. Both `rtti` results are `(no RTTI)`. |
| Physics object | pointer readable, class unverified | Section 4. `+0x2C0` is `0x3C9E7AE0A00` from all three slots. `rtti` returned `(no RTTI)`. `exe+0x60CD660` is not adopted. |
| Local position at physics `+0x90` | tracks the round trip; not world position | Sections 5–7. Standing `(-112.655, 552.443, -724.742)`, away by 54.039, return `(-115.798, 552.792, -718.831)` at 6.703 from the start. |
| World position, chunk origin, second copy | unverified | Section 16 has the later file trace and one standing sample. Not adopted. |
| Rotation axes | vertical stable, horizontal pair turns, forward unassigned | Section 8. `+0x70` stayed `(0.000, 1.000, 0.000)`. `+0x60` and `+0x80` rotated about 132 degrees, magnitudes 1.0000, dot products 0. |
| Health versus skill-screen 600 | unverified | Section 9. Status component class verified. No entry value is 600 or 600000. Id 0 reads 900/900 under divide-by-1000. No live HUD number was available. |
| Loading pointer behavior | unresolved | Sections 10 and 11 keep the raw samples. They do not define a coordinate readiness rule. `(0, 1000, 0)` was one sample. The second watch held near `(8000, 1000, 999)`, then an in-world local position. |
| Fresh process, title screen | two of three slots, repeated | Sections 12 and 13. Pid 35980 and pid 16920. `exe+0x71C09A8` and `exe+0x7233960` name `ClientActorManager`. `exe+0x7233AA0` is null. Actor hop is null. |
| Fresh process, after load and walk | all three slots track the walk | Sections 14 and 15. Pid 16920. After the load `(-108.921, 552.431, -741.734)`. After the walk `(-118.612, 553.858, -745.522)`, distance 10.502, same classes. |
| Chunk origin and independent world position | unverified | Sections 16–20. Candidates `exe+0x71AC2C0` and `exe+0x71CF140`. Stationary residual `(0.0000, -0.0503, 0.0000, 0.0000)`. After the short walk, `(0.0000, -0.0506, 0.0000, 0.0000)`. At 11:16:47 the origin changed from `(-8000, 0, -1000)` to `(-10000, 0, -4000)`. The rebase sample kept the world point. The next sample jumped. Settled residual `(0.0000, -0.0500, 0.0000, 0.0000)`. Y was not adjusted. Not adopted. |
| Two extra heap position records | unverified | Section 16. Pid 16920 only. Both reference one `hknpCompoundShape`. No pointer stores the translation. Values match local, not the static candidate. Parent object unnamed. |
| Offsets adopted into the mod | not adopted | Guard remains closed. `offsets.h` was not edited. |

## 16. Local-to-world file trace and a standing sample

The file trace is `docs/probe-3576/LOCAL-TO-WORLD.md`. Raw scanner output is `docs/probe-3576/local-to-world-scan.txt`. The scanner and its runner are `tools/probe-3576/scan_local_to_world.cpp` and `tools/probe-3576/run_scan.ps1`. No offset was adopted. No hook was installed.

The 2976 subtract-into-physics-`+0x90` byte string occurs once, at `exe+0x397DCB1`. Its `mov rcx, [rbx+0x2C0]` displacement is `0x2C0`. Its `vsubps` resolves to `exe+0x71AC2C0`, not `exe+0x6D56500`. The 2976 static XY/Z store occurs once, at `exe+0xA310EA`, and resolves to `exe+0x71CF140` and `exe+0x71CF148`. Nineteen of twenty tight `+0x90` add/sub pairs use `exe+0x71AC2C0`. The old globals are not among them. A live read of those old RVAs at 10:41:26 still showed the small file floats.

Pid 16920 was still the process from section 13. `info` at 10:31:32.483:

```text
PID      16920
Module   D:\SteamLibrary\steamapps\common\Crimson Desert\bin64\CrimsonDesert.exe
Base     0x140000000  size 0x13CC4000
PE timestamp 0x6AED2E85 (UNKNOWN BUILD - offsets/sigs may be invalid)
```

The temporary sampler `exe+0x71C09A8` was chained again at 10:31:32.612. Same manager `0x58302767300`, actor `0x583536A7BC0`, character control `0x5832F644B00`, physics `0x582E9CB2F80`, local field `0x582E9CB3010`, value `(-118.612, 553.858, -745.522)`. The slot is not adopted.

Precise float32 burst at 10:43:04.770:

```text
0x582E9CB3010: -118.6118
0x582E9CB3014: 553.8584
0x582E9CB3018: -745.5221
exe+0x71AC2C0: -8000.0000
exe+0x71AC2C4: 0.0000
exe+0x71AC2C8: -1000.0000
exe+0x71AC2CC: 0.0000
exe+0x71CF140: -8118.6118
exe+0x71CF144: 553.8081
exe+0x71CF148: -1745.5221
exe+0x71CF14C: 0.0000
```

Local plus the origin candidate is `(-8118.6118, 553.8584, -1745.5221)`. The static candidate differs by `(0, -0.0503, 0)` as printed. An earlier vec4 burst at 10:41:26 showed the same split. Double and i32 reads of those globals are not a second coordinate format. Details are in `LOCAL-TO-WORLD.md`. This pose did not include a walk or an origin change.

The section 15 copies were re-read in this same process. At 10:31:32 both `0x583072A6470` and `0x583ADF71470` still printed `(-118.612, 553.858, -745.522, 0.000)`. Their f64 and i32 views are the float bits. `findptr` of each translation found 0 image slots and 0 heap slots. Each record stores `.?AVhknpCompoundShape@@` `0x5830797A800` at minus `0x78`. Heap `findptr` of that shape found `0x583072A63F8`, `0x583079872E8`, `0x5832E82A5F8`, and `0x583ADF713F8`. `rtti` on the records and on the two extra containers is `(no RTTI)`. The physics object `+0x28` is a different shape, `.?AVhknpCapsuleShape@@` at `0x58352D621C0`. The first qword of the physics object is `exe+0x60CD660`. The first qword of the compound shape is `exe+0x56B8B08`. Neither is adopted.

At 10:41:26 the 3x3 before each copy was `(0.409, 0, -0.912)`, `(0, 1, 0, 0.944)`, `(0.912, 0, 0.409)`, then the local translation. Physics axes at 10:34:28 were `(0.409, 0, -0.912, 0)`, `(0, 1, 0, 0.912)`, `(0.912, 0, 0.409, 0)`, and the position was still the local vec4. The w-lane does not match. With origin `(-8000, 0, -1000)`, both records match local and do not match the static candidate. The parent object is unnamed. Classification across an origin change is still open.

## 17. Repeated stationary bursts

The burst procedure in `docs/probe-3576/LOCAL-TO-WORLD.md` was corrected before these samples. `tools/probe-3576/burst.ps1` reads four float32 values with `--count 4` from the chain's absolute field. It does not pass an address containing `+` except `exe+`. The slot `exe+0x71C09A8` is the temporary sampler, not a permanent root. No offset was adopted.

### Observations

The character was at the section 15 stopped pose. Two bursts were taken 4 seconds apart. No walk was requested between them.

Burst A, 2026-11-12 10:52:31.145 through 10:52:31.413. Pid 16920, base `0x140000000`, timestamp `0x6AED2E85`.

```text
[exe+0x71C09A8] = 0x58302767300  (.?AVClientActorManager@pa@@)
[0x58302767350] = 0x583536A7BC0  (.?AVClientChildOnlyInGameActor@pa@@)
[0x583536A7C28] = 0x58307A22600
[0x58307A22640] = 0x5832F644B00  (.?AVClientCharacterControlActorComponent@pa@@)
[0x5832F644C40] = 0x582E81D3100
[0x582E81D33C0] = 0x582E9CB2F80
=> 0x582E9CB3010 : (-118.612, 553.858, -745.522)
0x582E9CB3010: -118.6118
0x582E9CB3014: 553.8584
0x582E9CB3018: -745.5221
0x582E9CB301C: 0.0000
exe+0x71AC2C0: -8000.0000
exe+0x71AC2C4: 0.0000
exe+0x71AC2C8: -1000.0000
exe+0x71AC2CC: 0.0000
exe+0x71CF140: -8118.6118
exe+0x71CF144: 553.8081
exe+0x71CF148: -1745.5221
exe+0x71CF14C: 0.0000
0x582E9CB3010: -118.6118
0x582E9CB3014: 553.8584
0x582E9CB3018: -745.5221
0x582E9CB301C: 0.0000
```

Computed from those lines, residual = static − (local + origin), first local read, no component adjusted:

```text
local (-118.6118, 553.8584, -745.5221, 0.0000)
origin (-8000.0000, 0.0000, -1000.0000, 0.0000)
static (-8118.6118, 553.8081, -1745.5221, 0.0000)
local-plus-origin (-8118.6118, 553.8584, -1745.5221, 0.0000)
residual (0.0000, -0.0503, 0.0000, 0.0000)
bookend-delta (0.0000, 0.0000, 0.0000, 0.0000)
```

Burst B, 2026-11-12 10:52:35.086 through 10:52:35.294. The chain printed the same pointer values and the same classes. The four float32 groups printed the same numbers as burst A. Computed residual `(0.0000, -0.0503, 0.0000, 0.0000)`. Bookend delta `(0.0000, 0.0000, 0.0000, 0.0000)`.

### Conclusions

The two stationary bursts agree at the printed resolution. The residual vector is `(0.0000, -0.0503, 0.0000, 0.0000)`. The Y component was left as measured. This does not verify the candidates. The short walk is section 18. The origin-change burst is not in the log. The guard remains closed.

## 18. Short-walk stationary burst

### Observations

The user walked a short distance from the section 17 pose and said "stopped". Burst C was taken while they were standing still. Same process as sections 13–17: pid 16920, StartTime 11/12/2026 10:06:31 AM, base `0x140000000`, timestamp `0x6AED2E85`. Slot `exe+0x71C09A8` is still the temporary sampler.

Burst C, 2026-11-12 10:54:41.824 through 10:54:42.045.

```text
[exe+0x71C09A8] = 0x58302767300  (.?AVClientActorManager@pa@@)
[0x58302767350] = 0x583536A7BC0  (.?AVClientChildOnlyInGameActor@pa@@)
[0x583536A7C28] = 0x58307A22600
[0x58307A22640] = 0x5832F644B00  (.?AVClientCharacterControlActorComponent@pa@@)
[0x5832F644C40] = 0x582E81D3100
[0x582E81D33C0] = 0x582E9CB2F80
=> 0x582E9CB3010 : (-124.088, 553.335, -747.976)
0x582E9CB3010: -124.0879
0x582E9CB3014: 553.3345
0x582E9CB3018: -747.9763
0x582E9CB301C: 0.0000
exe+0x71AC2C0: -8000.0000
exe+0x71AC2C4: 0.0000
exe+0x71AC2C8: -1000.0000
exe+0x71AC2CC: 0.0000
exe+0x71CF140: -8124.0879
exe+0x71CF144: 553.2839
exe+0x71CF148: -1747.9763
exe+0x71CF14C: 0.0000
0x582E9CB3010: -124.0879
0x582E9CB3014: 553.3345
0x582E9CB3018: -747.9763
0x582E9CB301C: 0.0000
```

The script's computed block, residual = static − (local + origin), first local read, no component adjusted:

```text
local (-124.0879, 553.3345, -747.9763, 0.0000)
origin (-8000.0000, 0.0000, -1000.0000, 0.0000)
static (-8124.0879, 553.2839, -1747.9763, 0.0000)
local-plus-origin (-8124.0879, 553.3345, -1747.9763, 0.0000)
residual (0.0000, -0.0506, 0.0000, 0.0000)
bookend-delta (0.0000, 0.0000, 0.0000, 0.0000)
```

The chain printed the same pointer values and the same classes as bursts A and B. The local field stayed `0x582E9CB3010`. The origin candidate printed the same four floats as bursts A and B.

Printed-float change from bursts A and B to burst C:

```text
local-delta (-5.4761, -0.5239, -2.4542) distance 6.0237
static-delta (-5.4761, -0.5242, -2.4542) distance 6.0237
delta-of-deltas (0.0000, -0.0003, 0.0000)
```

The two heap records from section 16 were not read in this burst.

### Conclusions

The character moved about 6.0237 units and was still at both ends of the burst. The origin candidate stayed `(-8000.0000, 0.0000, -1000.0000, 0.0000)`. X and Z of local and of the static candidate changed by the same printed amount. The residual is `(0.0000, -0.0506, 0.0000, 0.0000)`. Its Y component differs from the stationary residual by `-0.0003` and was left as measured. This walk did not change the origin. It does not verify the candidates across an origin change. The candidates are not adopted. The guard remains closed.

## 19. Bounded origin watch

### Observations

`tools/probe-3576/origin-watch.ps1` sampled pid 16920 for a 180 second window starting 11:15:19. The raw lines are `docs/probe-3576/origin-watch-16920-111519.log`. An earlier file, `origin-watch-16920-invalid.log`, is not this window: its static address was overwritten after the first sample, and the character had not moved. Slot `exe+0x71C09A8` remains the temporary sampler. The local field stayed `0x582E9CB3010`.

The origin candidate stayed `(-8000.0000, 0.0000, -1000.0000, 0.0000)` while the character walked and then stood still. While walking, the static candidate lagged local plus origin by tenths on X and Z. While still, just before the change, X and Z residual returned to `0.0000` and Y was `-0.0597` on the 11:16:46 sample. Y was not adjusted.

At 11:16:47.902 the two origin reads inside one sample disagreed. Origin A was still `(-8000, 0, -1000)`. Origin B was `(-10000.0000, 0.0000, -4000.0000, 0.0000)`. Local changed by `(2000.0000, 0.0000, 3000.0000)`, the opposite of that origin step. Local plus the new origin was `(-8199.9629, 561.1732, -1727.9131)`, the same world point as the previous sample. The static candidate on that sample was still `(-8199.9629, 561.1219, -1727.9131)`. Its residual against the new world point was `(0.0000, -0.0513, 0.0000, 0.0000)`.

The next sample, 11:16:48.918, kept the new origin and moved to local `(-531.5918, 611.3008, -329.9185)`, world `(-10531.5918, 611.3008, -4329.9185)`, static `(-10531.5918, 611.2508, -4329.9185)`. Residual `(0.0000, -0.0500, 0.0000, 0.0000)`. The world step from the rebase point to this sample is about `(-2331.6289, 50.1276, -2602.0054)`. The following sample repeated those floats. The watch then stopped so the character could stand still.

### Conclusions

The origin candidate did change, by `(-2000, 0, -3000)`. The sample that contains the change preserves the world point: local moved by the opposite of the origin step. The following sample does not continue from that point. Whether that jump is a continuous walk, fast travel, or a load is not in the log. Y was left as measured. This does not adopt the candidates. The settled burst is section 20. The guard remains closed.

## 20. Settled burst after the origin change

### Observations

The operator replied "stopped" and did not name the stimulus. Burst D was taken while they were standing still. Same process as sections 13–19: pid 16920, StartTime 11/12/2026 10:06:31 AM, base `0x140000000`, timestamp `0x6AED2E85`. Slot `exe+0x71C09A8` is still the temporary sampler.

Burst D, 2026-11-12 11:18:52.528 through 11:18:52.749.

```text
[exe+0x71C09A8] = 0x58302767300  (.?AVClientActorManager@pa@@)
[0x58302767350] = 0x583536A7BC0  (.?AVClientChildOnlyInGameActor@pa@@)
[0x583536A7C28] = 0x58307A22600
[0x58307A22640] = 0x5832F644B00  (.?AVClientCharacterControlActorComponent@pa@@)
[0x5832F644C40] = 0x582E81D3100
[0x582E81D33C0] = 0x582E9CB2F80
=> 0x582E9CB3010 : (-531.592, 611.233, -329.918)
0x582E9CB3010: -531.5918
0x582E9CB3014: 611.2330
0x582E9CB3018: -329.9185
0x582E9CB301C: 0.0000
exe+0x71AC2C0: -10000.0000
exe+0x71AC2C4: 0.0000
exe+0x71AC2C8: -4000.0000
exe+0x71AC2CC: 0.0000
exe+0x71CF140: -10531.5918
exe+0x71CF144: 611.1830
exe+0x71CF148: -4329.9185
exe+0x71CF14C: 0.0000
0x582E9CB3010: -531.5918
0x582E9CB3014: 611.2330
0x582E9CB3018: -329.9185
0x582E9CB301C: 0.0000
```

The script's computed block, residual = static − (local + origin), first local read, no component adjusted:

```text
local (-531.5918, 611.2330, -329.9185, 0.0000)
origin (-10000.0000, 0.0000, -4000.0000, 0.0000)
static (-10531.5918, 611.1830, -4329.9185, 0.0000)
local-plus-origin (-10531.5918, 611.2330, -4329.9185, 0.0000)
residual (0.0000, -0.0500, 0.0000, 0.0000)
bookend-delta (0.0000, 0.0000, 0.0000, 0.0000)
```

The chain printed the same pointer values and the same classes as bursts A–C. The local field stayed `0x582E9CB3010`.

Printed-float change from the 11:16:48.918 watch sample to this burst:

```text
local-delta (0.0000, -0.0678, 0.0000)
static-delta (0.0000, -0.0678, 0.0000)
```

X and Z did not change. Local Y and static Y changed by the same printed amount. The residual Y value stayed `-0.0500`.

### Conclusions

The character was still at both ends of the burst. The origin candidate remained `(-10000.0000, 0.0000, -4000.0000, 0.0000)`. X and Z of the static candidate equal local plus origin. The residual is `(0.0000, -0.0500, 0.0000, 0.0000)`. Y was not adjusted.

Around the origin write, section 19 shows local moving by the opposite of the origin step, so that world point was unchanged. The next watch sample is not continuous with that point. The operator did not say whether that jump was a continuous walk, fast travel, or a load, so continuity is not required across it. The settled comparison is this residual. The candidates are not adopted. The guard remains closed.
