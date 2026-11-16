# R4 Step 2 — guest travel and time scope

Before this change, the only armed 3663 B route admitted one cryptogram; it
had no explicit v1 travel policy for a later join path. Reusing the 2976
request numbers would refuse unrelated target messages. Expected behavior:
the guest's native travel/time controls never reach the host native queue,
and loading-completion requests remain available to the later core gate.

The paired named RTTI descriptors, initializer stores, actual descriptor+C
opcode values and vslot-2 handlers establish these target IDs:

| Native request | 2976 | 3663 | v1 guest policy |
|---|---:|---:|---|
| ReloadStageStartReq | 2283 | 3018 | Refuse fast travel/reload |
| MoveFieldCheatReq | 2838 | 2647 | Refuse field-move cheat |
| SetFieldTimeReq | 2125 | 2150 | Refuse time publication |
| AITeleportReq | 2091 | 2236 | Refuse optional AI teleport |

The actual ReloadStageStart handler 2D55170 validates the ordinary envelope,
decodes its arguments to exact exhaustion, and calls 2B586B0, whose native
roster/mover preparation reaches 2EA0920. SetFieldTime's thunk 2CA68F0 reaches
TransformSync through actor components+1A0 and consumer 2ED4D20. The field-move
cheat handler currently decodes and returns success without a world mutation;
its refusal is a conservative scope policy. AI teleport resolves and checks
actor ownership before its native component call. No handler address here is
adopted as a hook or native-call permission; no payload ABI is guessed.

`r4_scope.py` reproduces the four IDs from the hashed executables and local
paired exports; scope-result.json publishes summaries and proof hashes.
`blocked_guest_control_3663` adds these to the already verified target native
shutdown/pointer/local-connection policy. The host checks it before either
the B-only queue path or any later injection path. The target guest Send
path consumes refused controls before network enqueue; its installer remains
closed pending C/D. The B cryptogram-only gate remains in force.

LoginUser 2554, LoginCharacter 3081, GameLoadingStart 2350,
MoveFieldComplete 2723, LoadingComplete 2789 and PreLoadSequencer 2372 are
not classified as travel controls. This does not itself admit them: native
descriptor, identity, phase and tier gates still apply. The host's own local
travel is outside this guest filter.

R4 rule 1 retires **C110–C115 by scope** for v1. Their legacy publication
patches are not installed or redirected to target code. Guest fast travel,
time controls, AI teleport and other optional modules are not v1 features.
Host native field loading remains native. The 2976 policy and installers
retain their original values and paths.

Tests exercise all 65,536 opcode values, each refused request at host admission,
the separate login/loading messages, legacy-number non-reuse, malformed frame
rejection and unchanged legacy bounds. Full build/suite results and the
current remaining rows are recorded with the Step 2 result and R4-STATUS.json.
No R4 live session or join attempt has occurred.

Validation: scripts/build.ps1 completed the x64 build and **108/108 tests,
90.98 seconds**. No new live evidence. C is 136/150 closed and D 124/130
closed after approved scope retirements; both remain unarmed.
