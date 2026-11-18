# R8 L3 — field loading advanced, guest character-update crash captured

The tested build, 792a323, completed native staging and shared Enter.
At 4:58:39.553 PM Eastern shared Enter returned zero. The host then
logged owner_alias=aliased on the verified 73-byte 2364 payload. The
guest returned from native receive for that frame and issued the second
MoveFieldCompleteReq. The local reader resolved the guest's character
at the saved-level coordinates. The host and guest windows were
confirmed side by side. The exact save label was not recorded.

At 4:59:42.365 PM the guest's character-update worker read address 0x28
at CrimsonDesert.exe+0x258C8FD and exited with 0xC0000005. The captured
registers and exact-build disassembly show RDX=0 after 258C8F9 loaded
the selected layer's +0x18 pointer. The return chain follows 9770C0
and 9729C0 through the native CharacterControl update family identified
in R5. This is a different observed site from the legacy field-owner
crash. The victim actor and reason for the missing layer pointer are
unverified. No gameplay guard or speculative animation change was
added for this fault.
[Prior native character comparison](R5-UPSTREAM-REFERENCE.md#character-guards-and-guest-startup).

The first-chance capture preserved the instruction, accessed address,
registers and seven stack frames. There was no unhandled-filter
record or live minidump. The child-process fixture had verified that
the filter can produce a minidump when entered.

Between shared Enter and the first-chance fault, native receive
returned for 6,726 host-to-guest frames across 26 opcodes. The guest
queued 224 outgoing frames. The trace has no pending receive in
that window. Besides 2364, it recorded the second 2723, 78 copies
of ActorLoadDataAck (2005), the second MoveFieldCompleteAck (2268),
ChangeGameLevelStateAck (2669) and MoveFieldCompletePostAck (2792).
That follows the second-load sequence seen on 2976. 3663 opcodes
2891 and 2941, the loading completion and preload acknowledgments,
did not reach native receive in this window. The last inbound frame,
2849, returned about 152 ms before the worker fault. The trace
does not associate any particular packet with the failing layer.
The result records all opcode counts, timestamps and evidence hashes.
[L3 result and packet list](port-evidence/r8/step3-result.json).

All three real-save checkpoints matched for all 23 files, both
backups verified, and the guest sandbox still matched its backup.
The guest audit logged only the eight-byte sandbox startup self-check.
The prior R7 slot-2 writer remains unattributed. The host exited with
code 1 before the watchdog, and its exit was not accepted as a
verified normal quit. The crash stopped recording acceptance,
mutual movement, removal, reconnect and host reload. Both processes
are closed and the controlled installation was restored. The 3663
executable hash remained exact. No activation error was observed.

Budget use is **startup 0/5, join 1/3**. Two desktop shortcuts
expired without a game process or launcher error receipt. The
same verified runners, launched from a shorter local path, then
produced the two controlled process receipts. L3 remains blocked.
L4 was not started.
[R8 attempt ledger](port-evidence/r8/attempts.json).
[Game-free validation](port-evidence/r8/validation.json).

---

# R8 — guest crash capture and the shared-field owner acknowledgment

R8 starts with user-approved fresh budgets of five startup attempts
and three join attempts. The R7 ledger records its own five attempts.

## R7 save and packet evidence

The real slot-2 lobby and save files changed at 4:18:04 PM Eastern,
after the host loaded and before the guest character request at
4:18:41 PM. The guest was at title with NT save-only isolation verified.
The 23-file guest sandbox is byte-for-byte equal to its frozen
backup, including both slot-2 files. The timing is consistent with
a host load or autosave. The host log has no save-write event, and
the guest's 128 redirects combine queries and opens without a writer
PID or operation type. The writer and every individual guest save
operation cannot be established retroactively. The new guest audit
records the first 24 validated sandbox NtWriteFile calls by PID,
file class, length and status, plus a cumulative count. The existing
fixtures cover redirected writes, renames and mappings and reject
real-save handles and aliases. The audit alone covers NtWriteFile.
[Prior result and log hashes](port-evidence/r7/step3-transitional-field-result.json).

From the 4:19:18.011 PM shared Enter through the 4:19:25.601 PM
guest exit, the archived guest log has a cumulative pump line at
4:19:19.527 PM with delivered=2009. The preceding sample had
delivered=1998 at 4:19:14.517 PM. The log has no packet opcodes or
packet-level timestamps, so those counts do not identify the contents
of the post-Enter window. The last guest state was still loading.

## Comparison and scoped change

The 2976 capture has a type-1 MoveFieldAck (2174) after GameLoadingStart
and before the second MoveFieldCompleteReq. Only after that request
does it show the second ActorLoadDataAck burst (including opcode 2005),
MoveFieldCompleteAck, scene state, and the final LoadingCompleteReq.
Legacy guest delivery also has a native call fault boundary and
optional sequencer, scene, and schedule callbacks. The guest_lifecycle
tracker measures the move windows and disconnects; it does not itself
repair client field ownership. Samuel Cardillo's 3576 guest path adds
native opcode translation and a preloader for the distinct situation
where the client stays at state 0x11 waiting for a sequencer marker.
The 3663 crash has no evidence of that wait state.
[2976 sequence](../research/opcion-d-login.md#3-qué-responde-el-servidor-a-cada-req).
[Samuel's 3576 guest path](https://github.com/ByJunter/CrimsonDesertMultiplayer-NativeServer/blob/a3904af3e1a32056dcf7a63bc7a469eb4231084b/src/mod/session2/guest.cpp).

The earlier 2976 crash has a closer match: its dump shows the guest
dereferencing the missing character of the host user named in
MoveFieldAck. The legacy and Samuel 3576 hosts alias the owner only
in the guest's packet copy. The 3663 minimal host exited its frame
path before any equivalent handling. In the exact 3663 image,
2ECAF8D obtains ServerField+0x180 and passes it as argument four to
2C7A2B0. That writer emits 2364 with slot -1. The payload starts
with a one-byte discriminator, 32-bit actor, 32-bit move type, and
32-bit owner at payload+9. The writer's
fixed prefix is 17 bytes, and 26AB510/16446C0 serialize each record
in 56 bytes. CD4D70 reads the same prefix and passes the owner to
B370E0 and then 8882E0. This establishes the target wire field.
The R7 fault address was never recovered, so shared ownership
remains a diagnosis to verify live.
[2976 crash and fix](../research/pendientes/p8-cierre-campo.md).
[Samuel's 3576 host frame path](https://github.com/ByJunter/CrimsonDesertMultiplayer-NativeServer/blob/a3904af3e1a32056dcf7a63bc7a469eb4231084b/src/mod/session2/server.cpp).

The 3663 change runs after committed shared Enter, only on the
outgoing guest copy of a well-formed type-1 move for the verified
guest primary. It aliases the owner from the verified host key to
the verified guest key. Private staging, unrelated owners and
malformed packets keep their original bytes. The server's actual
field ownership and roster transaction are untouched. The host log
records the decision. No preloader, save-policy, activation, or
unrelated packet changes were made.

The guest now records bounded first-chance access violations from
the character login onward in a session-local file, with code,
accessed address, module-relative fault and stack addresses and
x64 registers. The handler uses fixed storage and returns
EXCEPTION_CONTINUE_SEARCH. The unhandled filter chains the previous
filter and can request one MiniDumpNormal from a dedicated thread
with a 1.8-second wait. If the game intercepts the exception before
the top-level filter, the first-chance evidence may survive without
a dump. No system-wide dump or reporting settings are used.

The guest packet trace records timestamp, direction, opcode, size
and whether native receive returned, without payloads. It is
bounded to 131,072 rows. A pending row identifies a call in progress,
not necessarily the cause of an exception on a different thread.
Use tools/port3663/r8_guest_trace.py with the explicit-offset shared
Enter timestamp to extract the post-Enter rows. Crash text, traces,
and dumps stay in the session evidence folder outside OneDrive.

All 120 tests passed in 109.79 seconds. The crash fixture verified
the previous filter, exception stream in the minidump, native exit
code, stack, and pending receive row. The NT fixture also verified
the diagnostic mappings with the sandbox active. Synthetic smoke
exchanged 78 states with zero rejects and unloaded cleanly. Adapter
enumeration, DNS and TCP passed with both fixture manifests intact.
[Offline validation](port-evidence/r8/validation.json).

## L3 brief

**Question.** Does the verified 2364 owner alias let the guest finish
loading into the host field after the two successful Acquire/Enter
stages? If the guest crashes, which module/offset faulted and which
opcodes reached native receive between shared Enter and the crash?

**Expected observation.** Both windows are usable side by side.
The host chooses and identifies a frozen save slot and stands still.
The guest selects the same slot once. The host reports private
staging, shared Enter, and owner_alias=aliased. If the guest reaches
control, the user records both views with Win+Alt+R, confirms mutual
walking and turning, checks removal on guest exit, tries reconnect
once within the same process, and verifies the host reload.

**Time limit and stop condition.** Verify the exact 3663 executable
and fresh external backups before launch. Both copies share the
fixed fourteen-minute watchdog. Stop on activation error, crash,
hang, refusal, or deadline. Collect the PID-specific crash, packet,
and exit evidence; compare pre-guest, post-guest and final manifests;
restore the test installation. L4 still requires an L3 pass and
the user's confirmation.

---

# R7 Step 5 — staged join verified, guest loading crash blocks L3

**The final build 3663 trial verified the two-stage field flow.** At
4:18:42 PM Eastern the guest's `0x3` destination took native Acquire
and entered its own field `0x82020000` with result zero. The guest then
reported the parking coordinates `(0, 1000, 0)`. At 4:19:17 PM the
second Acquire requested `0x1`, matched the host's level entry, and
passed every sharing predicate. The host inserted one guest row and
committed native Enter on `0x81020001` with result zero. The exact
same save label could not be verified in both windows.

The guest exited at 4:19:25 PM with an access violation
(`0xC0000005`), about 7.6 seconds after shared Enter. Its last
logged state was loading. The host detected loss during Loading
and quarantined the session. No usable dump or matching Windows
Application crash event was found, so the faulting instruction and
cause remain unknown. The crash stopped the trial before visual
acceptance, mutual movement, a verified recording, disconnect
removal, reconnect, or host reload. The host quit normally with
code zero. No activation error was observed in the startup retry or
subsequent L3 trials. L3 is unpassed and L4 was not started.

Both backups verified and all three manifests contained 23 real
save files. Two slot-2 files (`lobby.save` and `save.save`) changed
between the pre-guest and post-guest checkpoints. Their recorded
last-write time was 4:18:04 PM, before the guest's character request
at 4:18:41 PM. The manifest cannot attribute the writes to a
process. The final manifest matched the post-guest checkpoint.
The test installation was restored, both games are closed, and
the installed executable retained the exact 3663 SHA-256.

The final build preserved the R7 guest-only NT save redirect and
normal Windows profile. All 119 tests passed, including the native
staging and strict shared routes. The game-free smoke exchanged
78 remote states with zero rejects and unloaded cleanly. The
sandbox networking fixture passed adapter enumeration, DNS, and
both owned-loopback and Steam HTTPS TCP connects.
[Final offline validation](port-evidence/r7/transitional-field-validation.json).

| Attempt | Category | Outcome |
| --- | --- | --- |
| R7-S2-20261118-1016-ready | Startup | Server-unreachable dialog before title, with no numeric code. |
| R7-S2-RETRY-20261118-B | Startup | Title reached, normal menu exit, guest redirect and unchanged real saves verified. |
| R7-L3-20261118-1214 | Join | Character identity passed; Acquire refused. Host reloaded and quit. |
| R7-L3-SAME-SLOT-20261118-B | Join | `0x3` resolved to a different level entry; original Acquire guard refused. |
| R7-L3-TRANSITIONAL-20261118 | Join | Native staging and shared Enter both returned zero; guest crashed during subsequent loading. |

Usage is **startup 2/5, join 3/3**. The two expired desktop plans
created no game processes and consumed zero attempts. The join
budget is exhausted. The packet and field routing fix is `a39f326`.
The exact live result and exit evidence are in `2dc045f`. The
package contains the complete commit list since `9bbc3df`, every
attempt's log hashes, and the evidence checksums.
[Step 5 package](port-evidence/r7/step5-package.json).
[Final L3 trace and receipts](port-evidence/r7/step3-transitional-field-result.json).
[Attempt ledger](port-evidence/r7/attempts.json).

---

# R7 Step 3 — private staging field and final join attempt

The 2976 live captures show FieldInfoKey `0x3` entering a private
staging field at `(0, 1000, 0)`. The first loading completion is
followed by `LoginCharacterToGameReq` and `GameLoadingStartReq`,
then by a second field acquisition for the saved level. The legacy
adapter calls native Acquire on a destination mismatch and applies
sharing only when the destination matches the host. Samuel Cardillo's
3576 port at `a3904af` retains that behavior and lets Enter run
natively for the guest's own field. The 3663 implementation uses its
verified three-argument Enter ABI and its own guarded transaction.
[2976 live sequence](../research/opcion-d-accion-usuario2.md#31-secuencia-del-login-real-v).
[Samuel Cardillo's implementation](https://github.com/ByJunter/CrimsonDesertMultiplayer-NativeServer/blob/a3904af3e1a32056dcf7a63bc7a469eb4231084b/src/mod/session2/field_share.cpp).

The 3663 same-slot trace resolved `0x3` to a different level
entry and the guest showed the staging coordinates. The adapter
refused before native Enter, so the trace ends before a second
Acquire. That refusal did not itself quarantine the session. The
3663 packet and transform-sync evidence supports the two-stage
interpretation. Its saved-level Acquire still needs live confirmation.
[3663 request mapping](R2-LIFECYCLE.md#loading-message-and-duplicate-consumer-continuation).
[Prior same-slot trace](port-evidence/r7/step3-same-slot-result.json).

For a known guest with valid pins and a current session, Acquire
now calls native code for a resolved, distinct destination and logs
the routing decision once. Enter delegates only when it verifies
guest ownership of both the actor and the distinct target field,
and the existing guest identity binding is ready. Native owns that
field's entry and return status. The host destination still requires
the existing owner, Scope, lifetime and roster checks, with its
original exact rollback or quarantine. Unknown destinations and
unproved ownership still refuse.

The first loading acknowledgement now keeps the guest in Loading
until a shared Enter commits, so it cannot prematurely block the
native loading-start request (`2350`). The exact 3663 image's
`LoginCharacterToGameReq` RTTI vtable at `0x5F675B8` is stored by
the native initializer at `0x258B7C` into descriptor `0x6F35E20`.
The descriptor has request kind 4, opcode `2140` and a five-byte
minimum frame. The existing 2976 capture shows the empty, slot-0
form. The tunnel accepts only that form, once, after verified
private Enter and before shared Enter. The 3663 handler body and
actual live payload have yet to be observed. Character-number,
lobby, actor-key, and phase identity checks remain in force.

The full suite passed 119/119 in 106.62 seconds. The transaction
tests cover distinct-destination native Acquire, guest-owned native
Enter without roster insertion, and strict host-field admission
and rollback. The lifecycle tests keep the parking completion in
Loading and reject duplicate or malformed world-login requests.
The eight-second synthetic smoke exchanged 78 remote states with
zero rejects and unloaded cleanly. The NT sandbox's adapter,
DNS and TCP checks passed with both fixture manifests intact.
[Validation receipt](port-evidence/r7/transitional-field-validation.json).

**Question.** Does native Acquire and Enter for the private staging
field let the matching frozen save slot reach a second Acquire and
commit Enter on the host's field?

**Expected observation.** With the host stationary and the same
save slot selected in the guest, look for the one-shot
`guest_native_destination` line, guest-owned Enter result zero,
then a new Acquire with `dest_eq_entry=pass` and a committed shared
Enter. Record both usable side-by-side views with Win+Alt+R. Verify
mutual walking and turning, removal on guest disconnect, the
reconnect outcome, and host reload followed by normal Quit to
Desktop. No travel, combat, or mounts.

**Time limit and stop condition.** Startup usage is 2/5 and join
usage is 2/3. Require the exact 3663 executable hash, fresh verified
save backups outside OneDrive, and the shared fourteen-minute
watchdog. Verify both views and identify the same save slot before
one guest Load. Stop on activation error, crash, hang, refusal,
loading stall, or the watchdog deadline. Collect exact exits and
logs, verify both backups and the real-save manifest against the
pre-guest checkpoint, and restore the test installation. This is
the final available join attempt. L4 awaits user confirmation of
L3 acceptance.

---

# R7 Step 3 — same-slot diagnostic result (2026-11-18 Eastern)

**The first failed check was `dest_eq_entry`.** The user confirmed
two usable windows before selection and the same save slot in both
copies. The exact menu label was not supplied. Native guest login and
the character request's identity check passed.

At 3:35:03 PM Eastern, the host and guest pins, host field access,
hash header and bucket, destination lookup and host level entry all
passed. The resolved guest destination was a different entry from
the host's. The requested FieldInfoKey was `0x3` and the host runtime
field key was `0x81020001`. They belong to different key spaces.
Owner, open state, field-key guard, output Scope, context and final
field-pin checks were skipped. The trace does not establish why the
same menu slot led to different entries.

No guest actor was inserted. The guest stayed on the loading
animation, then both copies exited with code 0 before the watchdog.
The pre-guest and final manifests matched for all 23 real saves, both
backups verified, and the test installation was restored. No
activation error was observed.

Usage is **startup 2/5 and join 2/3**. L3 is unpassed. Acceptance
recording, movement, removal and reconnect required a successful
join. L4 and the separate cleanup plan remain on hold.

Host log SHA-256 `195D66F5C785B28B61770FEED0A9D7855515435AF287A3AE37BB09F8FF4BFAA4`.
Guest log SHA-256 `5E9238543825881023732B56C670D71FB63B64EFCEBEF45D313C2A6ECDBCC888`.
[Exact trace and receipts](port-evidence/r7/step3-same-slot-result.json).
[Offline validation](port-evidence/r7/acquire-diagnostic-validation.json).

---

# R7 Step 3 — same-slot Acquire diagnostic brief (2026-11-18 Eastern)

The first desktop plan expired unused at 3:13:07 PM Eastern. There was
no launch or launcher error receipt and no game process. The deployment
was restored after the deadline. The 23 real saves and frozen backups
still matched. The local finish receipt SHA-256 is
`81DA48460887E5E6905F351866F8FD403BF01A848274E5713E530C25C6101828`.
The `R7-L3-SAME-SLOT-20261118-B` backups are prepared and no timer is
active. This setup consumed zero attempts. Usage is startup 2/5 and
join 1/3. The diagnostic still awaits the desktop handoff.

**Question.** Does one guest request for the same frozen save slot as
the host resolve to the host's current level entry, and which sharing
predicate stops it if sharing fails?

**Expected observation.** The host loads a chosen slot and stands
still. The guest opens the matching slot after native login and both
usable windows are confirmed side by side. One bounded Acquire log
line records each predicate as pass, fail, or skipped and includes the
requested FieldInfoKey and the current host field key. The existing
level-entry comparison remains the decision. The two logged keys have
different roles, so numeric equality of those keys is not a premise.

**Time limit and stop condition.** Fresh verified backups outside OneDrive
and the exact 3663 executable hash are required. Both copies share the
fourteen-minute watchdog, with a six-minute reserve for guest startup.
Select the guest slot once. On field refusal, activation error, crash or
loading stall, stop, preserve the line and verify the saves. On a successful
join, record both views, verify mutual walking and turning, check actor
removal, try reconnect once if available, then verify host reload and
normal quit. No travel, combat or mounts. L4 and the separate cleanup
plan remain on hold.

## Sandbox sample review

The previous L3 guest log has 20 sampled names and a final periodic
snapshot with **71,775** refusals. The two sites were `NtCreateFile`
(line 364, 71,161) and `NtOpenFile` (line 371, 614). The log limits
names early, so those totals do not identify all later paths.

Seventeen sampled names are HID interfaces, one is the mount manager
device, one is a named-pipe namespace root, and one is
`\??\GLOBALROOT\C:\WINDOWS`. No sample identifies a required game
config, shader cache, crash/telemetry, or Steam disk file. The pipe root
does not identify an endpoint or its caller. The HID samples can be
relevant to input enumeration, but ordinary menu input worked in both
live sessions. The available evidence does not establish a required
permission change. The guest sandbox and save protections are unchanged.
The name-by-name report with usernames redacted stays local at
`build/r7/l3-denied-names-redacted.md` to avoid publishing device IDs.

Offline validation passed **119/119** in 139.90 seconds. The synthetic
smoke received 78 remote states with zero rejects and a clean unload.
The sandbox networking fixture passed. The compiled ASI contains the
new trace and the source seal covers it. The existing Acquire guards,
native calls and refusal behavior retain their original checks and
short-circuit order. [Validation and evidence](port-evidence/r7/acquire-diagnostic-validation.json).

References for classification
[HID interface GUID](https://learn.microsoft.com/en-us/windows-hardware/drivers/install/guid-devinterface-hid),
[Mount Manager](https://learn.microsoft.com/en-us/windows-hardware/drivers/storage/supporting-mount-manager-requests-in-a-storage-class-driver),
[named-pipe paths](https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/fltkernel/nf-fltkernel-fltcreatenamedpipefile),
and [Windows NT namespaces](https://learn.microsoft.com/en-us/windows/win32/fileio/naming-a-file).

---

# R7 Step 5 — startup passed, L3 blocked (2026-11-18 Eastern)

**The guest-only startup retry passed. L3 reached character identity
processing and then failed at field acquisition.** The field Acquire
guard returned `shared false` after all four preliminary pins passed.
No guest actor entered the host world. The later failed predicate and
native refusal code remain unknown. L4 and the separate cleanup plan
await the user's confirmation of a future L3 pass.

The networking fixture reproduced the original sandbox's NSI denial and
verified the fix against adapter enumeration, DNS and TCP. Production
logging now samples up to 20 distinct denied names and continues total
and per-site counts. The logging build passed **119/119** tests in
108.65 seconds and the synthetic smoke recorded
78 remote states, zero rejects and clean
unload. The later live tests used the matching build 3663 executable and
guest-only NT save redirect with the normal Windows profile.

| Attempt | Category | Observed outcome |
| --- | --- | --- |
| R7-S2-20261118-1016-ready | Startup | Server-unreachable dialog before title, without a numeric code. Activation status unconfirmed. |
| R7-S2-RETRY-20261118-B | Startup | Title reached and menu quit exited normally. NT isolation and all save manifests verified. No activation error observed. |
| R7-L3-20261118-1214 | Join | Native guest login and title refresh passed. Both windows were usable side by side. The host normalized 3081 and reached field acquisition with zero identity refusals. Field sharing refused and no actor was inserted. No activation error observed. |

The earlier unused retry plan created no process and consumed no
attempt. Current usage is **startup 2/5, join 1/3**. All four own-process
log hashes and the underlying evidence are indexed in
`port-evidence/r7/step5-package.json` and the step results below.

The guest stayed on the loading animation and the pinned process was
stopped after its ordinary window close did not exit it. Mutual
movement, removal and reconnect could not be exercised. No qualifying
recording was verified. The host reloaded its save at 12:28:42 PM Eastern
and quit normally at 12:29:19, before the 12:30:08 watchdog. Pre-guest,
post-guest and final manifests matched for all 23 real saves. Both
frozen backups verified and the six temporary installation files were
restored to their prior absent state. The installed executable retained
the exact 3663 hash and both game processes are closed.

Commits since `9bbc3df` before this package:

- `16d271d` Save user-approved R7 save-directory isolation plan
- `d52bd2f` R7 step 1: isolate guest saves without profile overrides
- `cac3d9d` R7 preflight: record time-gate hold and verified offline seals
- `1d25995` docs: add L4 Proxmox feasibility plan
- `4a5f7ed` R7 step 2: record guest startup blocked before title
- `f46a056` R7 step 5: package guest startup blocker and evidence
- `b9ab0af` R7 step 2: preserve Windows networking under guest save isolation
- `58cd22a` R7 step 2: log bounded guest sandbox refusals before retry
- `23607e4` R7 step 5: record expired retry plan and pending desktop handoff
- `17bc421` R7 step 2: verify title startup, normal quit and save isolation
- `e3c8a17` R7 step 3: record field acquisition refusal and save reload

---

# R7 Step 3 — L3 blocked at field acquisition (2026-11-18 Eastern)

The host (PID 34096) loaded its save. The guest (PID 40164) inherited the
normal profile with NT save isolation. The user confirmed two usable
windows side by side before selecting Play once on the guest. Both process
and window identities were recorded. The native guest login and title-slot
refresh worked, with no activation error observed.

The host normalized the 17-byte 3081 request and advanced through identity
processing with zero refusals. At 12:20:10.645 PM Eastern the field Acquire
hook logged this result:

```text
3663 join Acquire: shared false host pin 1 guest pin 1 host primary pin 1 field access 1
```

All four preliminary pins passed. The later sharing check failed, but its
individual predicate and native refusal code were not logged. Enter did not
run for this join, and zero guest actors were inserted or committed. The
guest remained on its loading animation. The identity-pinned guest was
stopped at 12:23:47 PM after the ordinary window close did not exit it.
Mutual movement, actor removal and reconnect could not be exercised.
Recording status: No recording supplied or verified.

The before/after guest checkpoints matched for all 23 real saves. Frozen
backups verified after exit and the temporary installation was restored.
Host save reload was verified.
The host exit receipt recorded `exited`, code
`0`.

The last guest periodic report counted 71,775 sandbox refusals and 20
sampled names. The raw names stay in the local role log. This does not
identify the failed field-sharing predicate.

Host log SHA-256 `F439DE5F222C49F41E84B94874E579E07DE652F2ADA35480F2141934122AEA44`.
Guest log SHA-256 `1B3C12525A6BDB19FD8D1C8636AAE85058F56227BF38DF2CB6568968521349A1`.
[Result and receipts](port-evidence/r7/step3-result.json).

Usage is **startup 2/5, join 1/3**. L3 is unpassed. L4 and the separate
cleanup plan stay on hold.

---

# R7 Step 3 — L3 prelaunch brief (2026-11-18 Eastern)

**Question.** Can the exact 3663 host and guest complete native character
admission, see one another move, remove the guest and preserve the host save?

**Expected observation.** Host loads into the world. The two identified
windowed views are usable side by side before guest character selection.
One guest request is admitted and spawns an actor. Record mutual walking
and turning, verify removal on guest Quit to Title, try reconnect once,
and record that result. Verify the real-save checkpoint across guest
play. The host then reloads its save and quits from the menu. Fresh
external backups verified against manifest SHA-256
`B65BE5AC79EEF990D2AC1E7785ABCCE6C34B63DE2B6754B1E5CDCB149076FFC8`.

**Time limit and stop condition.** Shared fourteen-minute watchdog from
host launch. Guest launch needs a ready host and six-minute reserve.
Stop on crash, hang, activation error, unsafe input, failed identity or
save verification, or a native admission failure. Collect log evidence
before diagnosis, with no repeated Play on refusal. The user records
with Win+Alt+R and reports the filename and duration. No travel, combat
or mounts.

# R7 Step 2 retry — passed

Guest PID 42092 launched at 12:13:26 PM Eastern. The usable title screen
was observed and the user confirmed menu quit. The guest exited at
12:14:17 with code 0 and no activation error. Its own log verifies NT save
isolation with the normal profile. The 23 real saves and both
backups matched and the installed test files were restored.

The last periodic snapshot has **8,919 refusals**. The log capped names
at 20, including 17 HID-interface samples. Device identifiers stay in
the local log, whose SHA-256 is `1858400A29ABDE464EBB570CC565311B31F8020CFBE5CA40A4D270F7E4B2FC8D`. The title menu accepted
input. L3 will check both windows again before guest selection.
[Result and receipts](port-evidence/r7/step2-retry-result.json).

Startup budget **2/5**, join budget **0/3** before L3.

---

# R7 Step 5 — retry readiness and launch hold (2026-11-18 Eastern)

Commit `58cd22a` adds live guest refusal logging and the validated build for
the Step 2 retry. The log reports up to 20 distinct readable object names
and periodic per-site and total counters. The cap, deduplication,
sanitization and counter continuity passed the owned fixture. The full
suite and public networking fixture passed. The details and brief follow
below.

The desktop plan for `R7-S2-RETRY-20261118` was offered at 12:04 PM
Eastern and expired unused at 12:06:13 PM. There was no launch receipt,
guest process, or new runtime log. The temporary installation was
restored, and all 23 real saves and both frozen backups still matched the
prelaunch manifest. The local restoration receipt SHA-256 is
`7E73848151A3B23EC72508950A50F1931B819C15D47D7C8797B74BA21468DAAA`.
The copy of the preceding session's log alias is historical evidence
already indexed in the original Step 2 result.

The fresh `R7-S2-RETRY-20261118-B` backups are prepared outside OneDrive.
The next plan is waiting for the user's desktop handoff. No deployment
or timer is active. Round usage remains **startup 1/5, join 0/3**, with the
original guest failure still the only live attempt. Step 2 retry, L3,
recording, removal, reconnect and host reload remain pending. L4 and the
separate cleanup plan still await the user's confirmation that L3 passed.

---

# R7 Step 2 retry — prelaunch brief (2026-11-18 Eastern)

**Question.** Can one guest using the offline networking fix reach the 3663
title screen and quit normally with NT save isolation active?

**Expected observation.** Title is usable and no startup or activation
dialog appears. The process log verifies save isolation and shows periodic
refusal totals with per-site counts. Up to 20 distinct readable names are
queued to the local log, with at most 256 UTF-16 code units per name. Names
are sanitized and long samples are marked. The unmodified game executable
matches 3663. The 23 real saves and both frozen backups match the fresh
outside-OneDrive manifest
`B65BE5AC79EEF990D2AC1E7785ABCCE6C34B63DE2B6754B1E5CDCB149076FFC8`
before and after the guest-only session.

**Time limit and stop condition.** One guest process with a ten-minute
watchdog. Quit to Desktop promptly at title, before character selection.
If an error dialog appears, record its exact text and any code, close it,
and stop live work. A crash, hang, missing isolation receipt, abnormal
exit or manifest mismatch also blocks L3. Preserve evidence and write
the Step 5 package. If Step 2 passes, record the result before beginning
L3 with separate fresh backups.

The logging hook queues samples without file I/O or a blocking sampling
lock. Refusal counts continue after the sample cap. The owned fixture
exercises duplicate names, truncation/control-character sanitization,
and two summary snapshots. Diagnostic logging does not change the
sandbox decision or the network device allowlist.

Offline validation passed **119/119** in 108.65 seconds. The synthetic
smoke received 78 remote states with zero rejects and a clean unload.
Adapter enumeration, DNS and both TCP checks passed with zero refusals
under the hooks. [Retry validation](port-evidence/r7/refusal-log-validation.json).

---

# R7 Step 5 — offline networking follow-up (2026-11-18 Eastern)

The guest sandbox's networking defect was reproduced and fixed offline.
The R7-S2 live log only reports its early isolation snapshot, with two
redirects, zero refusals and no denied names. In an owned reproduction,
the same sandbox refused five `NtCreateFile` calls to `\??\Nsi`.
Adapter enumeration returned access denied and public DNS failed.
The unhooked control and the final sandbox both resolved
steamcommunity.com, connected to its port 443 and connected to an owned
loopback listener. The final sandbox recorded zero network refusals.

The fix resolves bare DOS aliases to approved native network endpoints
and recognizes the exact NetBT adapter-device format. A DOS alias
pointing to an owned original save was still refused. The save fixture
also verified unchanged original and backup manifests and the existing
redirected operations. [Reproduction and coverage](R7-NETWORK-FOLLOWUP.md).

The full suite passed **119/119** in 173.34 seconds. The eight-second
synthetic smoke received 78 remote states, rejected zero and unloaded
cleanly. [Validation and evidence hashes](port-evidence/r7/network-fix-validation.json).

The offline fixture explains a plausible path to the startup dialog.
Whether that was the live cause or an activation error remains unknown.
Startup usage is still **1/5** and join usage **0/3**. No game launched or
test installation was deployed during the follow-up. The next live test
is one guest-only Step 2 retry in a later session using the new arming
receipt, fresh verified backups and the existing stop conditions. L3
requires a clean Step 2. L4 and the separate cleanup plan await the user's
confirmation that L3 passed.

---

# R7 Step 5 — original blocker package (2026-11-18 Eastern)

**Step 2 stopped before title. L3 and L4 remain pending.** The only R7
launch reached a Crimson Desert dialog saying **“The server is not
reachable.”** It displayed Retry and Next without a numeric code. This
confirms a startup blocker. Whether activation error 88500006 recurred
and why the server was unreachable remain unknown. The dialog was
closed and live work stopped for the day.

The guest inherited its normal Windows profile and its own log verified
NT save-directory isolation. The host route has no sandbox. Step 1
passed 119/119 passed in 110.93 seconds. The synthetic
smoke received 78 remote states with
zero rejects and unloaded cleanly. These are game-free checks. The
repeat executable hash matched 3663 before and after the live session.

| Attempt | Budget | Observation |
| --- | --- | --- |
| R7-S2-20261118-1016-ready | Startup 1/5 | Guest PID 51912 started at 10:19:54 AM, verified NT isolation, displayed the server error before title, and exited at 10:21:53 AM when the dialog was closed. Exit code `FFFFFFFF` preceded the watchdog. |
| L3 | Join 0/3 | Step 2 prerequisite unmet. Character admission, mutual movement, disconnect removal, reconnect, and host reload remain untested in R7. |
| L4 | Pending | Requires L3 acceptance and the user's go-ahead. The separate cleanup plan is also held. |

The original 23 save files and both backups outside
OneDrive matched SHA-256 manifest
`B65BE5AC79EEF990D2AC1E7785ABCCE6C34B63DE2B6754B1E5CDCB149076FFC8` after the session.
The six temporary test files were restored to their prior absent state.
The guest log SHA-256 is
`8B0B19355DD28C2B2F5CDEDFF2F6CB0AE9AC32363106AC3FB61B07CF1595C037`.
The exit receipt and the unchanged executable hash are recorded in
`port-evidence/r7/step2-result.json` (SHA-256 `5528B1BEA3BE83F5061FD69077E3B84C61EBE5F6DC381E58BE2A918B92EF06A7`).
The evidence index attributes only the guest log to this launch.
`port-evidence/r7/attempts.json` preserves the separate budgets. The
preflight that found the missing loader created zero game processes.

The OneDrive Git metadata was incomplete. The live evidence commits
use a separate checkout of the same branch outside OneDrive. All 680
source hashes matched the Step 1 seal after aligning checkout line
endings. The runtime binaries matched the same receipt. No activation
files were inspected, copied, or edited and the executable was not
patched.

Commits since `9bbc3df` before this package:

- `16d271d` Save user-approved R7 save-directory isolation plan
- `d52bd2f` R7 step 1: isolate guest saves without profile overrides
- `cac3d9d` R7 preflight: record time-gate hold and verified offline seals
- `1d25995` docs: add L4 Proxmox feasibility plan
- `4a5f7ed` R7 step 2: record guest startup blocked before title

---

# R7 Step 2 — guest-only startup test brief (2026-11-18 Eastern)

**Question.** Can one 3663 guest reach title and exit normally with the R7
save-directory redirect and its normal Windows profile?

**Expected observation.** Title appears without an activation error. The
guest log for this process confirms NT save isolation. The user chooses Quit
to Desktop from the menu without loading a character. The real save tree and
both frozen backups match the prelaunch manifest after exit.

**Time limit and stop condition.** Ten-minute process watchdog. Stop at title
and quit promptly. On error 88500006 or any activation error, exit the dialog,
stop all live work for today, and package the blocker. On a crash or hang,
stop and retain the logs before offline diagnosis and the full suite. A
save-manifest mismatch or missing isolation evidence blocks Step 3.

The time gate passed at 10:00 AM Eastern. The installed executable was
rehash-verified as 3663
(8BE67178D9B9A149C361A731051665E0BA4D441E250AE1DFE645BD6E524345C8).
The launch uses the validated Step 1 binaries and all 680 sealed source
files. The fresh, verified backups are outside OneDrive. Their manifest
SHA-256 is B65BE5AC79EEF990D2AC1E7785ABCCE6C34B63DE2B6754B1E5CDCB149076FFC8. The prelaunch attempt ledger is startup
0/5 and join 0/3.

### Result

One guest launched at 10:19:54 AM Eastern (PID 51912) and
verified NT save isolation in its own log. A startup dialog reported
“The server is not reachable” with Retry and Next. It displayed no
numeric code and the guest never reached title. Computer Use closed the
dialog at 10:21:53 AM. The pinned process exited before its watchdog
with code `FFFFFFFF`. The reason for the server error is unconfirmed.

The original 23 save files and both frozen backups matched the
prelaunch manifest. The six temporary installation files were restored
to their prior absent state and the executable hash still matched 3663.
The guest log SHA-256 is `8B0B19355DD28C2B2F5CDEDFF2F6CB0AE9AC32363106AC3FB61B07CF1595C037`.
See `port-evidence/r7/step2-result.json` for the receipts and hashes.

Step 2 is blocked. Startup budget **1/5** and join budget **0/3**.
The earlier setup stopped at a missing loader before any launch. The
loader was retrieved from the pinned, checksum-verified release. The
guest was launched only once. No retry or L3 launch was made.

---

# R7 Step 1 — offline checkpoint (2026-11-17 Eastern)

User-approved R7 supersedes earlier launch/profile rules. Plan saved/pushed
as **16d271d**. The production 3663 guest now uses **NT save-directory-only
isolation**, inherits normal USERPROFILE/LOCALAPPDATA, and refuses the profile
route. The host retains real saves and no sandbox. Both launch paths enforce
**no game launch before 2026-11-18 10:00 AM Eastern / 15:00 UTC**.

`R7-STEP1.md` records the change, reproducible fixture expectations, retained
IPC/pipe/small-stack/teardown protections, validation and next live sequence.
`port-evidence/r7/step1-validation.json` seals the final full-suite/synthetic
smoke and binary/evidence hashes. The separate R7 arming format rejects old
profile receipts. The R6 narrow character-login fix is unchanged.
Final validation: **119/119 in 110.93 seconds**; synthetic smoke **78 remote
states, zero rejects, clean unload**. Audit: no game process/controlled install.

**Steps 2–5 are pending the time gate and user-present tests. This is not the
final Step 5 package.** No R7 game launched or live budget was consumed:
startup **0/5**, join **0/3**, separate ledger under `port-evidence/r7`.
Live preflight requested while the user was present: checked **2026-11-17
10:48–10:49 Eastern**, still before the approved launch time. Both UTC and
local clocks agree. All 680 source seals, ASI/runner and Step 1 evidence hashes
reverified; no game process or any of the six controlled installation files
was present. No deployment, launch plan, timer or attempt was started. This
is a time-gate hold, not a completed Step 2 or an activation-error recurrence.
Activation causality/recovery remains unproven; L3/L4 remain unpassed.
After the deadline, run one guest-only title/quit check. If activation error
returns, stop live work and package the blocker; otherwise proceed to the
batched L3 acceptance with both usable views confirmed side by side and the
user's recording. Request the second PC only after L3 passes. Preserve all
earlier evidence and commit/push each remaining step without review gates.

---

# R6 Step 6 — review package (2026-11-17 Eastern)

Branch **gpt/3663-port** is ready for Claude's review. **The diagnostic
identified two concrete refusal causes and the targeted fix passes offline
validation. Post-fix L3 validation was blocked by anti-tamper purchase
revalidation before native login. L3/L4 remain unpassed.** This section
supersedes all prior status below; the two-player goal is not reached.

Fix/source checkpoint **8ff1732**; Step 4 blocker **ea1d4d8**, Step 5
prerequisite record **963e0f6**. The following packaging commit is branch
HEAD. `R6-REVIEW-PACKAGE.json` indexes all three setup/live experiments,
own-PID logs and receipt/manifest/screenshot hashes, exact diagnostic, fix,
tests, official activation guidance and cleanup. `R6-STEP1.md` through
`R6-STEP5.md` provide the step records. No review gate stopped the work.
The JSON also locates the frozen local bundle `R6-20261117-0952`: 60 unique
evidence files, about 1.26 MB, outside OneDrive, with an archival hash index.

## What the diagnostic established

At **09:21:57 Eastern**, R6-D1b host PID 45568 recorded:

```text
length=17 prefix13=04FFFFFFFFFFFFFF7F01000000
discriminator=0x04 number=0x7FFFFFFFFFFFFFFF slot=1
bound=true lobby_ready=true size=false number=false slot=true generation=true
state_epoch=1 current_epoch=1 state_user_key=0x901002FA current_user_key=0x901002FA
```

**Size and number failed.** Active/context/health, all bound subconditions,
lobby readiness, ordinary slot 1 and generation passed. Lobby edits=1;
status/phase edits=0. The boolean `broken=false` means healthy, not a third
failure. The first 13 bytes were captured safely; the extra u32's actual
value was not logged and is not invented. No actor insertion/commit ran.

The exact 3663 descriptor/vtable reader at **2C6AE10** consumes 1+8+4+4
bytes, followed by a possible UTF-16 remainder, and calls **2F4AE80**. Its
disk vtable/read-size checks and read-only extraction are sealed. A 19-byte
2976 example is not this run's request. The target client writer was not
independently located; its actual 17-byte emission is measured live.

## Targeted fix and retained checks

The gate now supports **only the observed 17-byte empty-remainder form**.
Old 13-byte, truncated and extended forms remain refused. The opaque final
u32 passes through unchanged. Forbidden slots 3/206 remain forbidden.

The existing legacy tunnel already translates a native Play sentinel into
the second conversation's assigned number; the separate 3663 core route
skipped that code. Its new target adapter changes only the number's eight
bytes in an **owned packet copy**, after active/healthy context, bound epoch/
guest owner, lobby readiness, native generation, exact form, discriminator 4
and slot checks. Unknown numbers and stale/malformed requests are not repaired.
The strict host admission check reruns afterward and still requires assigned
number `0x7FFFFFFFFFFFF002` plus all existing identity predicates. Direct
strict admission still rejects the native sentinel. Original packet/header,
slot and opaque u32 remain unchanged. No new native offset, hook, memory
identity write or protocol-envelope change was introduced.

The regression reproduces the actual prefix/length and verifies the narrow
translation, unchanged scalar/slot, and negative state/length/identity cases.
Latest full build/suite: **119/119, 106.30 seconds**. Eight-second synthetic
smoke: **78 remote states, zero rejects, metadata mask 0xF, clean unload**.
Baseline and logging-only runs also passed 119/119. These do not prove native
admission, field insertion or multiplayer behavior after the fix.

Current ASI SHA-256:
`5C78FD617E961FA6C20775F094918ED3DF7B20037D8CFC283A4CA06F3292C3DF`.
Desktop runner SHA-256:
`8985660E38F24A8B7256918087A5CBF9829A775DD92DD0FBCE66C4280E80CB09`.
The source/ASI/runner/executable seal verifies. R6 uses separate uniquely
named arming receipts and a fresh ledger; the unchanged validated R5 desktop
harness retains its compatible version-2/round-5 receipt format. No R5 record
or budget was reset or reused as an R6 counter.

## Every R6 setup/live experiment

| ID | Budget category | Actual result |
| --- | --- | --- |
| R6-D1 | Conservative startup setup | Unused two-minute plan expired; zero host/guest processes. Official reuse refused; cleanup verified, fresh session used. |
| R6-D1b | Join experiment | Both usable windows confirmed left/right before Play; one native character-login request captured the two failures. Guest normal exit 0; host fixed-deadline stop. User clarified host Quit had not been selected before the deadline; no hang established. |
| R6-L3-1 | Conservative join experiment | Guest existed but anti-tamper error 88500006 blocked startup before title/native login. No character request or fix-path execution. Guest exited from error, code FFFFFFFF; host normal exit 0. Cleanup verified. |

Fresh R6 usage is **startup 1/5, join 2/3**. One native guest login/character
request occurred, before the fix; **zero post-fix character requests** and
zero accepted joins. The third record is not evidence of a second native
login or a failed EnterField transaction. Remaining budget was not spent on
retries during an external activation hold. No live deadline was extended.
Preexisting log aliases copied during the unlaunched setup are excluded from
own-PID runtime evidence.

Full role log SHA-256 values:

- Diagnostic host: `98F5976FA39B3927B2A7620E8D93D2671A3D528888BEB922869F90F703596144`
- Diagnostic guest: `93C138A5832FBB7679A5CF6F8791552962FE289BFB90A78033435E10DD7814F4`
- Post-fix host: `19E939D5E5632CE6B3121F6589A43DB0BB675C89F22E2D22041761CE3AC2CB3D`
- Post-fix guest: `889B6137935F81C4BF0E1EF0A5F73C6323C64DB94F2EABCC564F3BEAB5FCF312`

## Current blocker and next runtime boundary

The post-fix host launched **09:41:46**, guest **09:45:22 Eastern**. The user
supplied the activation error screenshot at 09:45:39 and selected Exit;
guest exit was **09:45:50**. Native guest sent/delivered stayed zero and
remote=false. Its mod UDP connection is separate from native login. Host
quit normally at **09:48:40**, before its **09:55:46** watchdog. No native
character admission, canonicalization, field insertion or commit ran.

The [official CodeFusion error page](https://support.codefusion.technology/anti-tamper/?e=88500006&l=en-US&p=s&a=Crimson%20Desert)
uses public application data mapping **88500006 -> 24H**. Its official
English guidance says purchase revalidation is currently unavailable and
to wait **24 hours before trying again**. Public mapping/language snapshots
and hashes are sealed in `step4-results.json`. No cause, device count or
guaranteed reset time is inferred. No protection change, token copy/deletion,
executable patch, credential request or additional activation was attempted.
The official wait puts any later manual test no earlier than approximately
**November 18, 09:46 AM Eastern**, with recovery still unverified. No retry
or reminder was scheduled.

L3 still needs actual character admission and shared-world join, mutual
walking/turning, the user's recording showing both windows (filename and
duration), guest removal/reconnect result, joined-session real-save manifest
acceptance and a host save reload followed by normal quit. No recording was
produced after the startup error; the screenshot proves only that error.
L4 did not run: the required L3 pass is absent. No second PC was requested
and no guest kit was transferred, deployed or declared live-ready.

## Cleanup, attribution and package contents

The fresh **09:52 Eastern** audit verified zero game processes, zero owned
watchdogs and none of the six controlled installation files. Every frozen
host/guest backup for all three records still matches its manifest. All
**23 current real-save files** match the latest post-session manifest; all
three session finishes recorded unchanged real saves. No progress was
restored over. The two temporary desktop shortcuts and their empty task
folder were removed after exact target/content checks. Raw evidence stays
local; no binaries/media were committed or released.

Host uses backed-up real saves with no file sandbox. Guest uses verified
native USERPROFILE/CSIDL save-root isolation with zero file hooks; the
explicit NT fallback remains separately fixture-tested. Profile path
selection is not an OS permission boundary, and endpoint equality alone
does not exclude transient writes. Default Tier A, explicit guarded v1,
Steam-friends restriction, executable filtering and synthetic test-host
paths are preserved. Original sibling checkout/parent launcher stay untouched.

Original project history and GPL v3 are preserved. The R6 mapping continues
the existing checkout's legacy rule, not a newly invented native identity.
No new source was copied from Samuel Cardillo's PR. His credited R5
field-leave/achievement guards remain unchanged, with the pinned reference
and independent address evidence in `R5-UPSTREAM-REFERENCE.md`.

Commits since a2418f9: **660526d** plan, **8935b14** logging/tests,
**2460e27** diagnostic, **8ff1732** targeted fix/tests, **ea1d4d8** blocked
retry, **963e0f6** L4 prerequisite, followed by this Step 6 packaging commit
at HEAD. The JSON package includes full commit hashes/subjects, changed files,
all attempts, evidence hashes, the two validation checkpoints, official
support provenance, cleanup and the next manual test plan. After activation
recovery, repeat bounded L3 with fresh presence and backups; only a complete
L3 pass permits the second-PC Steam sitting. The package is ready for review
with these limitations; the runtime fix remains unverified.

---

# R5 Step 5 — updated review package (2026-11-17 Eastern)

Branch **gpt/3663-port** is ready for Claude's review with explicit failures.
**Both actual game windows were visible and arranged left/right before guest
Play, but L3 failed at character-login identity admission. L4 did not run.
The two-player goal is not reached.** This section supersedes all older
status below. No intermediate review approval stopped the work.

Source/validation checkpoint **de2bd84**; live Step 3 results **8c51972**;
Step 4 prerequisite record **48a14a1**. The following packaging commit is
branch HEAD. `R5-REVIEW-PACKAGE.json` indexes all seven experiments, own-PID
role log hashes, process exits, manifests, cleanup, validation and reference
usage. `port-evidence/r5/desktop-retry-results.json` contains the three new
experiments and the frozen window/capture records. Earlier raw evidence is
retained. No binary or media is committed or released.

## Actual runtime result

The desktop-launch fixture reproduced a context mismatch: command-launched
forms reported visible locally but the user saw neither; desktop-launched
forms were visible and user-confirmed. The sealed `R5DesktopRunner` then
launched real host and guest processes on the visible desktop. When Computer
Use denied the guest app without a visible approval prompt, the final trial
used an explicit manual user handoff through prepared shortcuts. No denied
app was automated through another route. The user arranged the actual game
windows; identity-bound reports verified host left/guest right before the
guest Play signal. The temporary desktop shortcuts were removed afterward.

In the final trial, host PID **47812** launched at **08:23:09**, guest PID
**25516** at **08:25:41 Eastern**. Native cryptogram/login and ten-entry title
refresh passed. Guest shader compilation delayed its usable menu. Both
views were user-confirmed; final geometry was verified at **08:33:34**.
After guest Play, the identity refusal counter increased to one at 08:34:07;
this counter is specific to character-login opcode **3081**. Status/phase
normalizations and EnterField insertion/commit counts remained zero. No
native fault was logged. The rejected payload and individual failed predicate
were not retained; exact length, requested number, slot and binding require
further evidence. No payload, identity or executable gate was relaxed.

Guest exit was normal, code 0, at **08:36:12**. Host closure was confirmed
by the user, but its pinned receipt records a **watchdog stop at 08:37:10**,
not normal quit. The common fourteen-minute deadline was never extended.
The user did not provide an exact guest loading/menu/error outcome. Mutual
movement, removal, reconnect and post-session host reload remain unverified.

The real-save manifest taken after guest exit exactly matches the host-only
pre-guest checkpoint: **zero changed files across 23 files**. Host autosaves
before that checkpoint changed the original snapshot and were retained.
This is endpoint equality, not process attribution or proof against transient
writes. Separate guest-only S2 acceptance remains valid. The ten-second
recording again contained **zero frames, 232 bytes**, SHA-256
`C674E1892CDD9CC75B447BED09F9197D1E85F4BE1E6E7ABFDDD4A575A76B8DF5`.
It was rejected and is not visual proof. Foreground sampling needs an owned
desktop reproduction; no capture root-cause fix is claimed.

## Complete R5 attempt accounting

| Attempt | Category | Outcome |
| --- | --- | --- |
| R5-S1 | Startup | Host loaded; first fixed-deadline stop; retained as incomplete. |
| R5-S2 | Startup | Isolated guest title and normal exit 0; real manifest unchanged. Passed. |
| R5-S1B | Startup | Host real-save load, native readiness, one minute in-world, normal exit 0. Passed. |
| R5-L3-1 | Join | Native guest login/title refresh, but two usable windows and character-load attempt not established; both deadline-stopped. |
| R5-L3-2 | Startup | Desktop host visible, loaded, normal exit 0; operator delay exhausted guest-start reserve; no guest launched. |
| R5-L3-2b | Join experiment | Guest app access denied before process launch; host normal exit 0. Conservatively retains already-promoted join count. |
| R5-L3-3 | Join | Both actual windows visible/arranged; native character-login refused before insertion; guest normal exit, host deadline stop; empty capture rejected. |

The preserved local ledger is **startup 4/5, join 3/3**. It includes the
startup-only and application-access-blocked trials after the earlier de2bd84
checkpoint. This accounting was stated at the live-presence touchpoint; the
resumed prompt's larger remaining estimate was not used to erase attempts.
There were two actual guest native-login conversations and zero accepted
joins. No further join launch was made. L4 requires a passed L3, so no second
PC was requested and no kit was transferred or deployed.

Every role's full log SHA-256 is indexed under `attempts` in
`R5-REVIEW-PACKAGE.json`; the new final pair is:

- Host: `BA0792C77DF272F5746BC1839CF984BDF8C96CCE9B7FB1225D215E0B9CB5A9AB`
- Guest: `7538009B1ED24C6D7D3B4A0BFEDC792BD4BE7EE44351136DEFC60DB00A020BCC`

## Implementation, validation and licensing

Host isolation is **none**: fresh backed-up real saves, normal host autosaves
allowed. Guest isolation uses USERPROFILE before creation and verifies the
actual native CSIDL_LOCAL_APPDATA save root against its copied profile by
canonical path/file identity, with **zero file hooks**. The corrected NT
fallback is separate and fixture-tested. Profile path selection is not an
OS permission barrier against arbitrary absolute writes. See `R5-ISOLATION.md`.

The unchanged source passed **119/119 tests in 115.14 seconds**, plus an
eight-second synthetic host/peer smoke with **78 remote states, zero rejects,
metadata mask 0xF and clean unload**. Current ASI SHA-256:
`08A8ABEB7431703444AFC09ED44986ED493DECF24D103B8A121C8701309840DD`;
desktop runner SHA-256:
`8985660E38F24A8B7256918087A5CBF9829A775DD92DD0FBCE66C4280E80CB09`.
The ASI's embedded banner is 2709B8DE; the source/hash seal identifies the
validated work committed as de2bd84. The complete source/binary/runner seal
was checked again after the sitting. Later edits are documentation/evidence
only; no native fix or runtime pass follows from the game-free suite.

Samuel Cardillo PR #1 remains pinned at
`a3904af3e1a32056dcf7a63bc7a469eb4231084b`, reference only, never merged.
Verified 3576 executable SHA-256:
`C94AD6C3EA54970A6D2DA7A2719753210A7728A91698ED3405DD9125470ED98F`,
Steam manifest **5865785040848247748**; the reference was never executed.
His credited field-leave and achievement rules were adopted in **966cb5f**,
with independent 3663 entry/vtable/branch/cleanup checks and GPL v3 preserved.
The upstream four-argument wrapper did not replace our native three-argument
3663 EnterField adapter. Character signatures were research only; optional
character, mounts and combat paths remain off. `R5-UPSTREAM-REFERENCE.md`
records the exact use. No source-build address was copied unchecked.

Commits since 756fcd5, in order:

`31748d3` plan; `5129358` Step 1 isolation; `c933954` Step 2 startup;
`966cb5f` credited guards; `76ecfa0` first L3/capture validation;
`b0c1fef` initial L4 blocker; `2709b8d` earlier package;
`de2bd84` desktop launch correction; `8c51972` visible-pair refusal results;
`48a14a1` updated L4 prerequisite; following Step 5 packaging commit at HEAD.
Full hashes, subjects and changed files are in the package JSON.

## Cleanup and next work

The fresh **08:40 Eastern** audit verified no game, owned watchdog or six
controlled installation files. Every host/guest frozen backup for all seven
sessions still matches its own manifest. All 23 current real-save files match
the latest post-session manifest. Historical pre-session real-save manifests
are snapshots, not a requirement to undo legitimate autosaves. No real save
was restored over. Source/ASI/runner/executable hashes remain verified.
Desktop shortcut cleanup is separately sealed. The original sibling checkout
and parent Start-Grok launcher remain untouched.

Next independent work is bounded reason-specific admission diagnostics and
an exact-3663 packet reproduction; do not accept a 19-byte legacy variant or
change the requested identity without target proof. Reproduce foreground and
birth sampling with an owned visible pair before another recording. A later
authorized live attempt would repeat visible left/right confirmation before
character selection, then all L3 acceptance checks within its fixed deadline.
Only actual L3 acceptance permits the second-PC L4 sitting. The present
package is ready for review with these remaining limitations.

---

# R5 Step 5 — review package (2026-11-16 Eastern)

Branch `gpt/3663-port`; source/validation checkpoint **76ecfa0**, Step 4
blocker commit **b0c1fef**; the following packaging commit is branch HEAD.
This section supersedes R4/R3/R2 status below. **S1/S2 startup acceptance
passes. L3 and L4 do not pass; the two-player goal is not reached.** No
review approval is required to continue, and the attempt caps are not
exhausted. A game-free two-window presence question is pending.

`R5-REVIEW-PACKAGE.json` indexes every commit since 756fcd5, changed files,
all startup/join outcomes and log/receipt/manifest hashes, final validation,
upstream reference usage and cleanup. `R5-ISOLATION.md`, `R5-STEP2.md`,
`R5-STEP3.md`, `R5-STEP4.md` and `R5-UPSTREAM-REFERENCE.md` explain the
implementation, observations and remaining boundaries.

## What changed and passed

The host uses fresh backed-up real saves and never installs the file
sandbox. Normal host autosave is allowed. The guest overrides USERPROFILE
before process creation; the actual native CSIDL_LOCAL_APPDATA save resolver
must identify the verified guest copy by canonical path and file identity.
The preferred guest route installs **zero file hooks**. The corrected NT
fallback remains explicit and fixture-tested. Profile path selection is not
an OS permission boundary against arbitrary absolute writes; NT refusal
tests and profile-resolver tests are separate claims. Combined host deltas
alone cannot identify their writer. No real progress is restored over.

The owned fixtures cover native path resolution, isolated mutations,
negative profile checks, host source drift, Steam/pagefile IPC, pipes,
128 KB stacks and teardown. Steam initialization succeeded in baseline,
NT and profile modes. The supported target remains exact 3663, SHA-256
`8BE67178D9B9A149C361A731051665E0BA4D441E250AE1DFE645BD6E524345C8`.
Default Tier A and the explicit guarded C/D route remain distinct. Steam
friends restrictions, executable filtering and original attribution remain.

Latest `scripts/build.ps1` validation: **118/118 tests, 106.05 seconds**.
Latest isolated eight-second synthetic smoke: **78 remote states, zero
rejects, metadata mask 0xF, clean unload**. Latest ASI SHA-256 is
`5006DAD5DF5508CBC0CD971944A022150B852E5F3B708AD6DDD75FDACAC6ECAA`,
embedded banner `966CB5F3`. It was rebuilt alongside the validation-tool
update and has not been retried in the game. Its source/receipt hashes
are in `port-evidence/r5/desktop-preflight-validation.json`. Synthetic
traffic is not two-player proof.

## Every R5 live experiment

| Attempt | Budget category | Actual outcome |
| --- | --- | --- |
| R5-S1 | Startup | Host title/save/world readiness and over one minute in-world; fixed ten-minute watchdog stop. Normal quit unestablished; incomplete and retained. |
| R5-S2 | Startup | Isolated guest title reached, native profile verified, file_hooks=0, networking off; user selected Quit to Desktop; normal exit 0. Passed. |
| R5-S1B | Startup | Host no sandbox, save loaded, native descriptors/route/world ready, over one minute in-world; user selected Quit to Desktop; normal exit 0. Passed. |
| R5-L3-1 | Join | Host loaded; remote guest native cryptogram/login and ten-entry title refresh passed. User could not locate two games. Zero character-load requests/EnterField inserts/commits. Both fixed-deadline stops; unpassed. |

Startup usage **3/5**, join usage **1/3**. Earlier-round counts reset under
the approved R5 rule. L3 reserved a prejoin startup slot, then transferred
to the join budget before remote guest launch so its native handshake was
included. No native fault was observed in these trials. The first S1 exit
timing remains unknown; the successful retry gave the entire action sequence
before launch and passed the unchanged-binary full suite again (116/116,
135.49 seconds), including owned normal/deadline watchdog reproductions.

Full log SHA-256 values:

- S1 host: `505F439D6D776719A154082421227CCF8090F46304F3B3D561992ABC539147EA`
- S2 guest: `4718053076AFA1CE8639854C99885357C5A594194B8693CC7610C1912A349DFA`
- S1B host: `7E6BB475285D9EDEA6B1E4138462B5A81C954C159FF3307D0AC4AD9493150D0B`
- L3 host: `01A3F0FBFB89D49356BFE7F0B636377BB618FCF58B9C4A82C215443AC37691B0`
- L3 guest: `4789B8BD7D9FA67100D5596F8B68600C20EFE93A859BDE7CF038047F8F67E1DA`

## L3/L4 blocker and independent fixes

Two controlled process identities were recorded, but two usable game windows
were not established. The user described one Steam-started game and could
not find two. That report does not identify a PID; no hidden-desktop,
bootstrap, minimization or title-eligibility cause is asserted. The command
tool, Steam and Explorer were later observed in session 1; the tool reported
WinSta0/Default. This does not reconstruct the ended game windows.

The local ten-second capture was a **232-byte empty AVI, zero game frames**,
SHA-256 `C674E1892CDD9CC75B447BED09F9197D1E85F4BE1E6E7ABFDDD4A575A76B8DF5`.
It is not menu/gameplay evidence. The tool's previous success on an empty
clip was reproduced with an owned empty AVI and fixed: empty clips, missing
either controlled window, and duplicate role PIDs now fail while retaining
metadata. A new PID/birth-bound read-only window reporter checks bounds,
monitor, minimized/cloaked/visible/owner/style/desktop state without moving,
activating or changing game windows. Its owned hidden-pair fixture passes;
the optional 45-second visible-pair test needs the user present and has not
run. Do that before another game attempt, and establish both controlled
windows before character-load or visual acceptance. No assistant UI control
was used.

Mutual native visibility/movement, guest removal, reconnect, a nonempty
two-window recording, post-L3 host save reload and live firing of the two
new guards remain unproved. **L4 did not run** because R5 requires L3 first.
No second PC was requested and no guest kit was deployed or declared ready.
The L4 launcher modes retain exact-build/profile/Steam guards; portable paths,
same-binary private kit, friendship/account, backups and common deadline
still need the later second-PC sitting. No prerequisite was bypassed.

## Samuel Cardillo reference and credit

PR #1 is pinned at `a3904af3e1a32056dcf7a63bc7a469eb4231084b`; it was
fetched only as a reference and never merged. The file-filtered Steam
manifest **5865785040848247748** produced verified 1.0.0.3576, SHA-256
`C94AD6C3EA54970A6D2DA7A2719753210A7728A91698ED3405DD9125470ED98F`.
The reference was never executed and the installed game was unchanged.

The upstream EnterField wrapper still forwards four arguments; its native
3576 callee independently confirms the three-argument contract, so our
typed 3663 adapter remains. Samuel's **field-leave** and **achievement**
rules were adopted into new credited files and commit **966cb5f**, preserving
GPL v3. Every target entry, vtable slot, midpoint, branch, full achievement
frame construction and owned-argument cleanup was independently checked on
3663 and cross-checked against the verified 3576 reference. No source-build
address was copied unchecked. Character signatures and paired caller chains
were used for research only; optional character/mount/combat installers were
not enabled. See the pinned links and exact proof in `R5-UPSTREAM-REFERENCE.md`.

## Final state and resumption

Fresh final audit at **22:26 Eastern**: no game or owned watchdog processes;
ASI/PDB/loader/two role INIs/test marker absent; all **23 real-save files**
and both frozen backups for each of the four sessions match their content
manifests. Current executable, binary and source seal verify. The complete
audit hash is in `R5-REVIEW-PACKAGE.json`; session evidence remains local
outside OneDrive. No real saves were restored, no binaries/media were
committed, and the original sibling checkout and parent launcher were not
changed.

Resume with the pending game-free visible-pair touchpoint, then resolve the
native window boundary with the read-only report before another bounded L3
attempt. Keep explicit live presence, exact steps, backups and watchdogs.
Only a passed L3 permits asking for the second PC/L4. Update this package
after actual new evidence; no intermediate code-review gate applies.

---

# R4 Step 6 — review package (2026-11-16)

Branch: `gpt/3663-port`. Runtime/source checkpoint:
`cce29cc9db01c38fcd6fa8d1b1c71bfbf951652c`; the following Step 6 packaging
commit is branch HEAD. This section supersedes the R3/R2 status below.
**Steps 1–4 are implemented; A/B/C/D are armed for the explicit guarded v1
route. L3 is unpassed, and L4 did not run. The two-player goal is not reached.**

Step 5 ends at the approved plan's live-attempt cap. Three controlled L3
startup experiments failed: one stalled SteamAPI error exit and two crashes.
All three count against the cap, although zero guest processes/native joins
ran. Earlier checkpoints counted native joins separately as zero; that does
not permit more startup launches in this round. Offline work continued and
the last reproduced stack defect was corrected and validated. It has not
been retried in the game. No intermediate review approval is requested.

`R4-REVIEW-PACKAGE.json` indexes commits, changed files, retained evidence
hashes, live outcomes, final validation and cleanup. `R4-STATUS.json` is the
additive row ledger; `R4-BLOCKERS.md` distinguishes implementation closure
from the missing runtime milestone. `R4-LIVE.md` records the experiments and
reproductions, including failures.

## Armed tiers, scope and behavior

| Tier | Closed / total | Armed | Evidence and current boundary |
| --- | ---: | --- | --- |
| A | 37/37 | Yes | Default HP-only reader; prior R3 L1 passed |
| B | 78/78 | Yes | Prior R3 L2 native-host loopback passed; distinct v1 transport route implemented |
| C | 150/150 | Explicit v1 | New three-argument join/reference/rollback and identity adapters; no live guest join |
| D | 130/130 | Explicit v1 | File sandbox, bounded guest pump, native stable logout and lifecycle guards; no live guest removal |

Closed rows include approved policies and scope retirements, not 395 native
RVA adoptions or executed runtime contracts. Default 3663 remains Tier A.
The explicit `[session2] core_v1=1` route requires exact build, A/B/C/D,
actual verified file redirection, consistent roles and Steam or strict local
loopback. Both controlled copies use separate sandbox copies. Optional
modules remain unreachable; 2976 and synthetic test-host routes remain.

R4 C110–C115 are retired because guest travel/fast-travel and time publication
are refused. The guest LoadingComplete publication flag is also suppressed;
the host-owned-field native owner check excludes its earlier publication
path. D020–D023, D063–D067, D100–D104 and D130 use approved file-level
redirection instead of legacy save/title callsite patches. Source handles,
unclassified save operations and unsafe aliases remain denied. The earlier
R3 stamina retirement remains. Mounts, combat and optional gameplay are off.

The native join backend pins typed users/actors/field access, takes field
locks in native order, inserts one exact row and calls original EnterField
with locks released. Only its unchanged unused row can roll back; uncertainty
quarantines pins and seals admission. Identity uses separately adopted target
entries, generation keys and native host-derived thread affinity. Native
faults halt new injection and propagate; they do not fabricate success.

Stable/title disconnect requests native LogoutSession first. Loading loss
quarantines rather than guessing completion payloads. Reconnect is unsupported:
the guest ends on a changed native conversation with guard code 0xCD67.
Host field replacement, uncertain rollback or loading loss requires restart.
Only the measured single-primary roster is admitted. See Steps 3–4 notes.

## Validation and final installation

- Final `scripts/build.ps1 -BuildDir <external r2-build>`: **114/114 passed, 94.75 seconds**.
- Final unique eight-second synthetic smoke: **78 remote states, zero rejects, metadata tier mask 0xF, clean worker exit/unload**. Synthetic results are not two-player proof.
- Expanded fixtures reproduce/fix MAP-only pagefile IPC, DOS pipe/device and pipe-directory reopen, native hook CRT teardown, and 128 KB-thread mapping overflow. The small-stack pre-fix fixture exits 0xC00000FD.
- SteamAPI comparison after Steam restart/fixes succeeds both without and with the sandbox; zero sandbox refusals and unchanged fixture real saves. This initializes Steam in owned fixtures and never launches the game.
- Every controlled startup verified fresh host/guest backups outside OneDrive, exact executable identity, sandbox startup, pinned fourteen-minute watchdog and installation restoration. All **23 real saves** match the manifests; no real saves were restored over progress.
- Final elevated inspection at 19:30 Eastern: no Crimson Desert process; ASI/PDB/loader/host and guest INIs/test marker absent; latest backups and real saves verified again. Local `build/r4/final-cleanup.json` is hashed in the package.
- Source/binary proof is `port-evidence/r4/stack-fixed-validation.json`. Build banner is `ab72135 + local changes`; the tested source is sealed by file hashes and committed as cce29cc. It is not claimed to embed the later commit label.
- Earlier optional S3 publication-race failure remains recorded and is not claimed fixed; that module is unreachable here. Final full suite passes.

Target executable: 1.0.0.3663, timestamp `0x6AF5D198`, base `0x140000000`,
image size `0x144E9000`, SHA256
`8BE67178D9B9A149C361A731051665E0BA4D441E250AE1DFE645BD6E524345C8`.
The downloaded 2976 reference was inspected only, never executed.

## R4 live sessions and attempts

All sessions used a written question, user presence, guarded exact build and
save backups. No UI control was used. Full questions, executable identity,
watchdog, exit and cleanup records are indexed in the result JSON.

| Session | Question / outcome | Log SHA256 |
| --- | --- | --- |
| R2-P1-20261116-161703 | Do vector/identity/lobby/key/session contexts agree? Passive sitting: 161 valid events, two native admission pairs, user-confirmed travel; no guest join. Natural exit before watchdog, later than requested quit. | 89B074B8DCF28CDD8A4E90627ED5E21F89BB8AFF78226F5DA05AFD8FCBF3B556 |
| R4-L3-20261116-184248 | Can restricted real processes join/show movement/remove guest without real-save changes? Host SteamAPI failed before native hooks. User Exit left Error window; verified process aborted, cleanup passed. | 8BC024B5C5E36650A1531053FF1065D90EBF04282467596121E391FDD184E3B1 |
| R4-L3-20261116-184928 | Same L3 question after shared-memory fix. SteamAPI failed; host exited 0xC0000005 at NtProtectVirtualMemory+14. Hook-removal defect reproduced/fixed offline. | 665B9B6B7C7D7769B458808E57D9ADFF9235AE832F45C75265969D845EA0DD6C |
| R4-L3-20261116-191955 | Same L3 question after IPC/teardown fixes. Steam and guarded host route activated, then crashed before title/guest. RSP below 128 KB thread stack; matching small-stack callback defect reproduced/corrected offline, no live retry. | F4FB331B482E902F097816A78AAD9309EBF012D1C44BD0BE877E9AD41181486D |

The last crash is consistent with callback stack exhaustion; the actual game
resolution is unproven. Existing Windows dumps stay local; only hashes and
bounded metadata are packaged. No two-window capture was reached, so there
is no L3 capture hash. User-supplied error screenshots are not committed.
No L4/second-PC deployment or Steam two-player result is claimed.

## Commits since 0f11f6e and remaining verification

- 5db3328 — save approved R4 plan.
- 3f11c53 — Step 1 file sandbox/fixtures.
- b38e36e — Step 2 scope filter.
- 9cc2c4a — passive collector checkpoint.
- 43a49f5 — Step 3 native backend and identity.
- 8c541af — Step 4 explicit v1 route and lifecycle arming.
- c0e5984 — Step 5 shared-memory startup correction.
- ab72135 — Step 5 Steam IPC and teardown corrections.
- cce29cc — Step 5 attempt record and offline stack correction.
- Branch HEAD — Step 6 review package.

Remaining: real host startup after the final correction; real guest joining;
mutual walking/turning visibility; guest disconnect removal; reconnect
outcome; full-session save manifests and local capture; then L4 on two PCs
over Steam. L4's guest-kit deployment is blocked on L3 acceptance. No further
game launch belongs to this round's exhausted live budget.

For a later authorized live round, repeat fresh backups/exact hash and the
controlled host-title/world preflight first, then the guest-title/initial join,
mutual movement, local capture, stable disconnect, reconnect and cleanup
sequence. A full suite/compile alone cannot settle those requirements.
The branch is ready for Claude's final review as an **incomplete runtime
milestone with a documented blocker package**, not a working two-player port.

Original attribution/history/GPL v3 are preserved. The original sibling
checkout and parent Start-Grok.ps1 were not changed.

---

# R3 Step 5 — review package (2026-11-16)

Branch: `gpt/3663-port`. Source through `c2d3604c00ff872250cb9be0ef11ff307303b1e4`; the subsequent
Step 5 packaging commit is branch HEAD. This section supersedes the R2 status below.
**A/B are armed; L1 and L2 passed. C/D stay closed. The native two-player goal is not reached.**

The approved conditional R3 deliverable is complete: working Steps 1–2,
independent C/D work and exact blockers, followed by this Step 5 package.
No intermediate review approval is requested. `R3-REVIEW-PACKAGE.json` has
the source/summary inventory, verified local evidence hashes and commits;
`R3-STATUS.json` and `R3-BLOCKERS.md` retain every remaining contract.

| Tier | Closed / total | Armed | Current behavior |
| --- | ---: | --- | --- |
| A | 37/37 | Yes | HP-only reader; definition consensus and park/chain/readiness guards |
| B | 78/78 | Yes | Explicit loopback native-host cryptogram tunnel and real pose streaming |
| C | 130/150 | No | Named/traced facts and three-argument pure guard/rollback policy; no native join backend |
| D | 109/130 | No | Complete observed title membership policy on owned data; no native guest-save/lifecycle backend |

Closed rows include two approved A policies and one stamina scope retirement,
not 354 adopted native addresses. R2 evidence/history remains unchanged.

## Validation and current installation

- `scripts/build.ps1 -BuildDir <external r2-build>`: **106/106**, 98.33 seconds.
- Final `tools/port3663/smoke.ps1`: 78 synthetic remote states and clean unload; game-free evidence only.
- Guard matrix covers exact 3663 identity/dependencies, wrong timestamp/size/base, unknown builds, 3576 denial and preserved 2976/test-host paths.
- New tests cover target packet bounds, reliable handshake shape, trace recursion/unwinding, three-argument type/owner/roster/rollback refusal and bounded metadata membership.
- Game closed. Test ASI/PDB/loader/INI removed. Every R3 live session verified backups and unchanged saves (23 files each). No saves were restored over player progress.
- First L2 launch safely refused before hooks because derived Troc descriptors were not ready. Bounded named readiness fixed that timing; retry passed. Earlier fixture failures and corrections remain recorded.
- Evidence archival correction: one old pump smoke wrapper transcript is now empty (cause unestablished). Its earlier 153-byte hash is preserved but not counted as retained proof. The matching immutable ASI host log and final suite/smoke remain verified; see evidence-corrections.json.
- Pump cleanup initially hit OneDrive CLOUD placeholders. Exact hashes were recovered with the read-only verifier; subsequent R3 backups are outside OneDrive. General no-link guards remain.
- Supported target: executable 1.0.0.3663, timestamp `0x6AF5D198`, base `0x140000000`, size `0x144E9000`.
- Target SHA256: `8BE67178D9B9A149C361A731051665E0BA4D441E250AE1DFE645BD6E524345C8`. Rechecked at final cleanup.

## Live evidence

Each session used presence, a written question, exact identity, verified save
backup, pinned fourteen-minute watchdog and removal. Full questions/receipts
and log paths are in the result JSON/manifest. No UI control was used.

- **R3-L1-20261116-110000 — passed**. Does the HP-only production reader track pose and changing HP through load/travel?
  Log SHA256: `24B4A1D623E32927B40E00F4AA1AC9F3DE6631CC7F251656FD50169AEAC72D38`; result: `port-evidence/r3/l1-result.json`.
- **R3-PUMP-20261116-112400 — passed**. Does the candidate pump expose the expected ClientNet/session/queue context?
  Log SHA256: `3AE7D89F56928A85137E9D3E469072CD49F2D0E40FF6983D3C83FF0834E4EC65`; result: `port-evidence/r3/pump-result.json`.
- **R3-L2-20261116-122500 — safely_refused_before_hooks**. Does the native private-session handshake and movement stream work?
  Log SHA256: `C7420EF2EAD37AC2624323529DAC852606009383443D1CABBB9A233603A5641E`; result: `port-evidence/r3/l2-refusal-result.json`.
- **R3-L2-20261116-123100 — passed**. Does the native private-session handshake and movement stream work?
  Log SHA256: `5708AFDD4DAE5A0440206BACC7566D2E464DDFB7E2433C20AD3EB67532034095`; result: `port-evidence/r3/l2-result.json`.
- **R3-JOIN-20261116-123800 — passive_subset_passed**. What are the original acquisition/entry roster, owners, results and travel/lobby order?
  Log SHA256: `BEDEC75A356D506D8AE3FFB998BD9012A735CC5E390B077BE95D96B5EC2533DA`; result: `port-evidence/r3/join-result.json`.

L1 logged HP 900 → 1.8 and regeneration with user-confirmed damage/travel;
no exact post-damage screen value was supplied. Pump recorded 125528 matching
contexts. L2 copied one real request 2401 (291 bytes), injected it once into
the private native session, and received native Ack 2128 (330 bytes) at the
peer; 1044 real pose samples returned, no rejections. Native host used UDP
initiation toward the loopback peer; this is not a native guest client or
Steam/two-player test. Normal process exit was verified; D teardown was not.
The field trace captured 16 valid records/eight pairs through two distinct
fields. Both single-primary admissions returned success and users/actors
0→1. No guest insertion, factory/failure path, native refcount or rollback
was measured. Status/login-manager/phase sites were not instrumented.

## Changes, blockers and next verification

`R3-STEP2.md` describes the shared per-build tunnel, guarded queue/pump
route and C/D isolation. `R3-JOIN-ADAPTER.md` defines the new three-argument
transaction, reference/lock order and rollback contract. Its pure policy
is tested but the native insertion/reference/rollback backend is absent.
Nine additional C facts were adopted after the eight recipient rows:
named lobby/status tables and slot 10, traced EnterField, and phase entry
2C17410/new guard. Generic VecPush still conflicts (VT/native callee 4CAE10
versus default BinDiff 14AD670); the named-RTTI exception does not settle it.
Ordinary server owner+A0 and roster kind 0 do not prove the separate C144
Actor_OwnerUser+90 or C066 class contracts. Travel clock publication is now
inlined and cannot reuse the old standalone detour. **20 C rows remain.**

`R3-STEP4.md` records the independent title eligibility implementation.
D130 still needs coherent native metadata-vector ownership/parser, alongside
early save interception, title/save/lock and lifecycle contracts. **21 D
rows remain.** The exact symbols and missing facts are in `R3-BLOCKERS.md`.
Guest-role target startup remains denied; L3/L4 were not attempted and the
second PC was not requested while these prerequisites were missing.

Manual verification after further implementation: first rerun the full
game-free suite and smoke. Prove the missing C helper/reference/rollback
backend and guest identity/travel paths before arming C. Complete early D
guest save protection before any two-instance launch. Then use fresh
separate save backups/presence/watchdogs for L3 native walking/turning,
disconnect removal/reconnect and unchanged host saves; L4 requires the
second legitimate PC over Steam friends and a load/travel on each side.
Do not promote loopback/policy tests into two-player evidence.

## Commits since 69075db

Source/evidence commits through c2d3604c00ff872250cb9be0ef11ff307303b1e4 are below. Run
`git log 69075db..gpt/3663-port` to include this subsequent packaging commit.

- `ee69b36` R3: save approved port execution plan
- `8365f55` R3 step 1: arm guarded HP-only 3663 Tier A
- `348da83` R3 step 1 L1: verify production pose and changing HP
- `be6614d` R3 step 2 preparation: validate bounded passive pump probe
- `0b4657b` R3 step 3 preparation: adopt named recipient slots under approved rule
- `375e07a` R3 L1: document acceptance and preserve immutable loopback log provenance
- `7185c1b` R3 step 2: verify pump context and recover cloud-backup audit
- `6cb39e3` R3 step 2: arm the guarded native host tunnel on build 3663
- `3b3965b` R3 step 3: prepare bounded passive field entry and exit traces
- `fe3404e` R3 step 2: wait for native descriptor readiness before installing hooks
- `b3a5ef0` R3 step 2: accept the live native tunnel and real movement stream
- `4caeb34` R3 step 3: adopt traced field and named identity contracts; keep join closed
- `c2d3604` R3 step 4: test title metadata eligibility; keep native guest protection closed

Original ByJunter attribution, history and GPL v3 preserved. Sibling
original checkout and parent Start-Grok.ps1 untouched. No binaries/release,
saves, new game screenshots or raw new decompiler dumps published.

<!-- R3 PACKAGE END -->

---

Historical R2 package follows; its all-tiers-off state is superseded above.

# R2 Step 7 — blocker package for final review (2026-11-16)

Branch: `gpt/3663-port`. The native two-player goal was **not reached**.
Furthest tier armed: **none**. Furthest L1–L4 passed: **none**. Three passive
P1 observations are evidence for specific questions, not native-tier acceptance.

The exact final source commit, full commit list since `2c80c21`, archive file
hashes and source/evidence inventory are in the generated **R2-MANIFEST.json**.
It is generated from clean committed HEAD inside the local Step 7 archive.
R2-STATUS.json is the machine-readable status; R2-MATCHES.md/JSON contains
every row, including all blocked rows and their missing facts. The archive is
a source/evidence review delta, with governing files, attribution, GPL v3 and
SOURCE-DIFF.patch. No executable/mod binary, save, credential, downloaded tool,
analysis database, release or main-branch publication is included.

## Core and actual permissions

| Tier | Adopted | Blocked | Armed |
|---|---:|---:|---|
| A | 34 | 3 | no |
| B | 73 | 5 | no |
| C | 113 | 37 | no |
| D | 109 | 21 | no |

Total: **395 core contracts, 329 adopted, 66 blocked**; 17 R1 adoptions
retained. The compiled catalog contains 265 independently reviewed layout/
guard contracts and 58 adopted RVAs. These numbers overlap the row evidence;
they are not extra completed game features. All four completeness flags are
false. The 3663 PlayerLayout and complete B/C/D module adapters are absent.
2976 behavior is preserved; 3576 remains recognized and unarmed. Optional
modules remain unreachable on 3663, and the synthetic fallback stays unchanged.

## What was completed and why native work stays gated

Step 1 cut the reachable core below 500 rows. Step 2 completed whole-image
Ghidra AutoVT and default BinDiff using independently exported saved analyses
and exact image identities. AutoVT accepted 138,587 unique associations;
default BinDiff produced 207,217 function pairs. All raw output hashes,
algorithm counts and caveats are in port-evidence/r2/standard-completion.json.
The conservative run is supplemental. A redundant third stock subset was
stopped after required results completed; its exact process identity and
partial-log/config hashes are retained, no partial result counted.

AutoVT’s original markup warning is retained: the persisted audit found zero
error flags and zero failed markup items across 46 inspected core/anchor
associations, which does not prove no transient warning occurred. BinDiff’s
missing-basic-block diagnostics are retained. Standard similarity is not an
ABI guarantee. Native checks found further changed fields and two concrete
conflicts: VecPush and the control/self recipient-finder swap. Neither was
resolved by silently preferring the desired address.

Step 3 established the changed three-argument EnterField ABI and new roster,
but entry correspondence and complete ownership/rollback remain unproved.
AcquireField is adopted; the legacy four-argument detour is not installed on
the new entry. Travel clock publication is embedded in a larger transition,
so its old inline hook cannot simply be redirected.

Step 4 prepared/tested dependency guards and partial tables, then committed
each tier’s blocked decision. A remains blocked on real loading readiness
and the unpublished HP-only layout. B still lacks descriptor range/scan, pump
correspondence and an actual per-build native packet-bound path. C additionally
lacks full join/identity/containment adapters and ownership. D lacks complete
save callsites, moving-state contracts, title eligibility and native cleanup
barriers. Each exact row is listed below. Constants-only preparation is not
a completed port. No unsupported guard, Steam-friends restriction, executable
filter or explicit test-host route was bypassed; protocol remains version 8.

Step 5 completed the passive questions below, with L1–L4 gated by their unmet
prerequisites. No second PC was requested prematurely. Step 6’s comment fix
was completed in c5f7074; the fallback was not expanded. This Step 7 package
records the resulting blocker boundary without requesting intermediate review.

## Validation and binary identities

Latest full `scripts/build.ps1`, x64 MSVC RelWithDebInfo: **101/101 passed,
87.04 seconds**. Latest game-free host/peer loopback: **77 remote states,
zero rejected, clean Network stopped and Worker exiting**. Older runs had
78 states; the final status parses the actual log rather than assuming that
exact timing-dependent count. This is synthetic validation, not L2.

The guard matrix covers all 256 request/completeness combinations, every
lower-tier dependency, wrong size/timestamp/base, missing/unknown identity
and recognized 3576. Actual 3663 startup returns no legacy game reads, hooks,
overlay, synthetic feed or networking. The native HP helper refuses missing,
ambiguous or changing definitions and rechecks the source chains.

Failures/fixes/reruns are retained in port-evidence/r2/validation-deviations.md:
PID reuse in the 3576 fixture hit correctly protected existing archives; adding
process creation identity fixed the test path. A separate run was refused by
game-closed checks when an unmodified game launched during validation; the
user closed it and the full game-closed rerun passed. Earlier build/exporter
and matching failures remain in chronological evidence, including the
one-line defined-function exporter fix and graph-equivalence fixture.
Session hash records now distinguish canonical Git bytes from original raw
capture hashes. Package generation verifies every referenced live evidence
file and all raw standard-output identities before producing the archive.

Binaries are **local validation outputs only**, not included in the package:

| Output | Bytes | SHA-256 |
|---|---:|---|
| cdmp.asi | 15945216 | `8AA59BE1F8D70DE14736619B46D2785B6CDB498FC9E05B101C8DE05207818673` |
| cdmp_host.exe | 737280 | `4B3DA924D51443E1D7269373976E4AD8DDBDF74918B16F899C3C31E8DC4C2629` |
| cdmp_peer.exe | 2723328 | `2A2FB5B34B5D416D85588FDF504778111CA4BDC010EB6C441CD3DBA83999F1CE` |

Installed target SHA-256:
`8BE67178D9B9A149C361A731051665E0BA4D441E250AE1DFE645BD6E524345C8`.
Reference SHA-256:
`57DA440D72F4DB974F25FEF047CF84C4DADD999A88CB2A3C5AF4C9BD67FDE1E7`.
No target drift was accepted; the downloaded reference was never executed.

## Passive sessions, cleanup and limitations

- **072000, HP definition:** both registries select key 0; unique native
  900000/900000 agrees with inventory 900/900. No HP variation was observed.
  The sitting exceeded fifteen minutes during delayed exit; that deviation
  is retained. Verified cleanup preserved normal save progress and led to
  the independent exact-process fourteen-minute watchdog.
- **075900, loading:** collection began after load because JSON DateTime
  conversion rejected a valid receipt. User-confirmed travel did not appear
  in the unsynchronized interval. The timestamp fix and precision regression
  preserve exact deadline checking. Watchdog termination is recorded as such,
  not a crash. Installation restored and backup verified.
- **100800, loading rerun:** collection verified active before Continue.
  438 samples show title not-ready, initial park, placement, movement/turning
  and a 182.35 m jump in the user-confirmed travel interval. No distinct
  not-ready interval appears during travel. Normal exit in about 4.5 minutes
  from launch; all 23 save files unchanged, backup verified, installation
  restored. No UI control or new screen capture used.

Each session’s question, exact executable identity, result, canonical log/
capture hashes and original capture hashes are in its result.json under
port-evidence/r2/live/. R2-LIVE-SESSIONS.md gives the ordered test disposition.
No native game crash or two-player success is claimed. The final cleanup
check found no game process and no test ASI/PDB/loader/config installed. The
optional matcher was identity-verified before stopping; other apps and the
original sibling checkout were left alone.

The actual new trace narrows, rather than closes, A028. Native client handlers
now identify candidate completion flags, but their full start/reset and
lifetime contract is unproved. The candidate owner predicate calls locking/
allocating helpers; neither it nor reference-acquisition vslots may be invoked
by an external passive reader. The existing pointer-only collector must not
be repeated as though it can answer this new question.

## Exact remaining rows

| Tier | Row / symbol | Missing fact |
|---|---|---|
| A | A026 kStatId_Health | P1 HP confirmed both definition registries select key 0 and unique raw 900000/900000 agrees with visible 900/900. Publish the definition-derived selector only with adopted definition-slot RVAs; one full-HP state does not prove literal-ID stability. |
| A | A027 kStatId_Stamina | The legacy reader still consumes stamina key 22. Prove the target stamina key, or publish an actual HP-only 3663 layout and explicitly revise target reachability; the unpublished helper alone does not retire this row. |
| A | A028 kLoadingParkHeight | P1 synchronized from title captured chain_not_ready, initial park at (0,1000,0), placement, walking/turning and user-confirmed fast travel with a 182.35m world jump. During travel the actor chain stayed coherent with no distinct park/not-ready interval; the initial park rule alone does not prove readiness during travel or pointer lifetime. |
| B | B052 kTrocInstancesBegin | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kTrocInstancesBegin at 2976 0x6A51AE0 |
| B | B053 kTrocInstancesEnd | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kTrocInstancesEnd at 2976 0x6A82C80 |
| B | B065 kNetLoopPump | Standard support for candidate 0x3a8d94 (or its documented native operand anchor) is absent; independent check: Decoded seven-byte MOV RAX,[R15+E0] at the pre-flush native NetworkThread point; R15 is ClientNet, incoming queue+18 is handled before this point, then synchronous session+10 is flushed and returned queue+28 freed. Both complete parent loops reviewed. This is a safetyhook mid-point, not an inline entry. |
| C | C001 kSamSlot | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kSamSlot at 2976 0x6DA67D8 |
| C | C009 kToNearExceptOneControlVtable | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kToNearExceptOneControlVtable at 2976 0x5B25358 |
| C | C010 kToNearExceptOneSelfVtable | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kToNearExceptOneSelfVtable at 2976 0x5B25378 |
| C | C011 kToNearVtable | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kToNearVtable at 2976 0x5B25418 |
| C | C012 kExceptOneVf1 | Resolve contradictory direct standard evidence for candidate 0x2d6c360; its anchor cannot override the conflict. independent check: Exact named RTTI primary-vtable slot 1 identifies control/self/near independently; paired native bodies follow finder+10 to actor+68/TS+1A0 and the field mover (4F0 -> 508). Except-one implementations preserve native reference acquisition/release without filling the output; near tail-dispatches its full recipient-list helper. Default BinDiff swaps the similar control/self bodies, which is a conflict, not corroboration. |
| C | C013 kExceptOneSelfVf1 | Resolve contradictory direct standard evidence for candidate 0x2d6c230; its anchor cannot override the conflict. independent check: Exact named RTTI primary-vtable slot 1 identifies control/self/near independently; paired native bodies follow finder+10 to actor+68/TS+1A0 and the field mover (4F0 -> 508). Except-one implementations preserve native reference acquisition/release without filling the output; near tail-dispatches its full recipient-list helper. Default BinDiff swaps the similar control/self bodies, which is a conflict, not corroboration. |
| C | C014 kToNearVf1 | Standard support for candidate 0x2d6c210 (or its documented native operand anchor) is absent; independent check: Exact named RTTI primary-vtable slot 1 identifies control/self/near independently; paired native bodies follow finder+10 to actor+68/TS+1A0 and the field mover (4F0 -> 508). Except-one implementations preserve native reference acquisition/release without filling the output; near tail-dispatches its full recipient-list helper. Default BinDiff swaps the similar control/self bodies, which is a conflict, not corroboration. |
| C | C015 kExceptOneVf1Head | Confirm 3663 code consumes kExceptOneVf1Head for the same native role as src/mod/session2/offsets_s2.h:101 ({0x40, 0x53, 0x48, 0x83, 0xEC, 0x50, 0x48, 0x8B, 0x41, 0x10, 0x48, 0x8B, 0x48, 0x68, 0x48, 0x8B, 0x99, 0xA0, 0x01, 0x00, 0x00}) |
| C | C016 kToNearVf1Head | Confirm 3663 code consumes kToNearVf1Head for the same native role as src/mod/session2/offsets_s2.h:103 ({0x48, 0x8B, 0x41, 0x10, 0x48, 0x8B, 0x48, 0x68, 0x48, 0x8B, 0x89, 0xA0, 0x01, 0x00, 0x00, 0xE9}) |
| D | D014 kWindowsLockVf1 | Standard support for candidate 0x1593240 (or its documented native operand anchor) is absent; independent check: Named WindowsLock<3> vslot 1 has the paired native thunk that dispatches this through vslot+18. Exact body and named RTTI tables are reviewed; shared use by other WindowsLock classes does not create a different native role. |
| D | D015 kWindowsLockVf2 | Standard support for candidate 0xe297f0 (or its documented native operand anchor) is absent; independent check: Named WindowsLock<3> vslot 2 has the paired native thunk that dispatches this through vslot+20. Exact body and named RTTI tables are reviewed; shared use by other WindowsLock classes does not create a different native role. |
| D | D020 kSlotRefreshReqSequence | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kSlotRefreshReqSequence at 2976 0x2A22475 |
| D | D021 kSlotRefreshReqSequenceBytes | Confirm 3663 code consumes kSlotRefreshReqSequenceBytes for the same native role as src/mod/session2/offsets_s2.h:191 ({ 0x48, 0x8B, 0x05, 0x1C, 0x72, 0x34, 0x04, 0x4C, 0x8B, 0x78, 0x48, 0x4C, 0x89, 0x7D, 0x07, 0x45, 0x32, 0xF6, 0x44, 0x88, 0x75, 0x0F, 0x49, 0x8B, 0x1F, 0x48, 0x89, 0x5D, 0x77, 0x48, 0x8B, 0x03, 0x48, 0x8B, 0xCB, 0xFF, 0x50, 0x08, 0x90, 0x45, 0x38, 0x77, 0x60, 0x75, 0x0C, 0x41, 0xC6, 0x47, 0x60, 0x01, 0x41, 0xB6, 0x01, 0x44, 0x88, 0x75, 0x0F, 0x48, 0x8B, 0x03, 0x48, 0x8B, 0xCB, 0xFF, 0x50, 0x10, 0x90, 0x45, 0x84, 0xF6, 0x75, 0x05, 0xE9, 0x91, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x0D, 0xCF, 0x71, 0x34, 0x04, 0x48, 0x8B, 0x49, 0x48, 0xE8, 0x4E, 0xE7, 0xE4, 0xFE, 0x90, 0x49, 0x8B, 0x1F, 0x48, 0x89, 0x5D, 0x77, 0x48, 0x8B, 0x03, 0x48, 0x8B, 0xCB, 0xFF, 0x50, 0x08, 0x90, 0x41, 0xC6, 0x47, 0x60, 0x00, 0x48, 0x8B, 0x03, 0x48, 0x8B, 0xCB, 0xFF, 0x50, 0x10}) |
| D | D022 kTitlePlayReadsSaveMgr | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kTitlePlayReadsSaveMgr at 2976 0x65DC3E |
| D | D023 kTitlePlayReadsSaveMgrBytes | Confirm 3663 code consumes kTitlePlayReadsSaveMgrBytes for the same native role as src/mod/session2/offsets_s2.h:212 ({0x4C, 0x8B, 0x1D, 0x4B, 0xB5, 0x70, 0x06, 0x49, 0x8B, 0x83, 0x88, 0x00, 0x00, 0x00}) |
| C | C043 kEnterField | Standard support for candidate 0x2ecb130 (or its documented native operand anchor) is absent; independent check: Native field admission and failure cleanup. ABI CHANGED: 3663 has three arguments and iterates its character list; matching this RVA does not authorize the legacy four-argument detour. |
| C | C047 kVecPush | Resolve contradictory direct standard evidence for candidate 0x4cae10; its anchor cannot override the conflict. independent check: Native AddActor callee; 16-byte user entries, vector pointer/count/capacity, growth and element copy verified. |
| C | C066 kCharacterClass | Confirm 3663 code consumes kCharacterClass for the same native role as src/mod/session2/identity_fix.h:63 (0xA) |
| C | C067 kLobbyLoad | Standard support for candidate 0x2e879a0 (or its documented native operand anchor) is absent; independent check: Paired ServerLobbyActorComponent native loads obtain the owner lock and INT64_MAX number, call paired lobby-entry writers with the same argument roles and propagate results. Exact RTTI vtable slot +50 independently identifies both entries; RTTI labels alone are not a second native-body check. |
| C | C069 kLobbyVtable | Standard support for candidate 0x5f6f2e8 (or its documented native operand anchor) is absent; independent check: Exact MSVC RTTI class/COL identifies this component vtable; slot +50 points to the independently reviewed native loader with matching role and changed fields. Standard loader agreement can anchor the paired slot relationship; RTTI naming alone does not satisfy adoption. |
| C | C070 kLobbyConstant | Standard support for candidate 0x2e87db0 (or its documented native operand anchor) is absent; independent check: Native lobby loader instruction MOV RAX,7FFFFFFFFFFFFFFF followed by a stack store; its address-taken qword becomes the paired entry writer character-number argument. Target stack slot changes to RBP-40; no old instruction patch bytes are reused. |
| C | C072 kLobbyAddCall | Standard support for candidate 0x2e87e10 (or its documented native operand anchor) is absent; independent check: Native E8 call from the lobby loader to paired entry writer 2143E70/237EA60, with component, result, address-of-number, slot/name data and extra native arguments. This callsite proof does not establish a safe detour ABI. |
| C | C074 kClearLobbySite | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kClearLobbySite at 2976 0x2C3E0FF |
| C | C075 kClearLobbySiteBytes | Confirm 3663 code consumes kClearLobbySiteBytes for the same native role as src/mod/session2/identity_fix.cpp:227 ({0x49, 0x8B, 0x47, 0x68, 0x48, 0x8B, 0x58, 0x10, 0x48, 0x8B, 0x7B, 0x08}) |
| C | C076 kClearListSite | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kClearListSite at 2976 0x2C3E126 |
| C | C077 kClearListSiteBytes | Confirm 3663 code consumes kClearListSiteBytes for the same native role as src/mod/session2/identity_fix.cpp:229 ({0x48, 0x8B, 0x4B, 0x18}) |
| C | C078 kStatusVtable | Standard support for candidate 0x5f63430 (or its documented native operand anchor) is absent; independent check: Exact MSVC RTTI class/COL identifies this component vtable; slot +50 points to the independently reviewed native loader with matching role and changed fields. Standard loader agreement can anchor the paired slot relationship; RTTI naming alone does not satisfy adoption. |
| C | C079 kStatusLoad | Standard support for candidate 0x2c160b0 (or its documented native operand anchor) is absent; independent check: Paired ServerStatusActorComponent loads call the common status loader, propagate its result and write INT64_MAX through the character-number pointer, changed +70 -> +90; target LoginCharacter lookup consumes +90 independently. Exact named RTTI slot +50 supplies entry identity. |
| C | C080 kLoginMgrSite | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kLoginMgrSite at 2976 0x2777A18 |
| C | C081 kLoginMgrSiteBytes | Confirm 3663 code consumes kLoginMgrSiteBytes for the same native role as src/mod/session2/identity_fix.cpp:235 ({0x48, 0x8B, 0x0D, 0x89, 0x1C, 0x5F, 0x04, 0x48, 0x8B, 0x09}) |
| C | C099 kPhases | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kPhases at 2976 0x2941020 |
| C | C100 kPhasesHead | Confirm 3663 code consumes kPhasesHead for the same native role as src/mod/session2/identity_probe.cpp:51 ({0x48, 0x89, 0x5C, 0x24, 0x08, 0x44, 0x89, 0x4C, 0x24, 0x20, 0x4C, 0x89, 0x44, 0x24, 0x18, 0x55, 0x56, 0x57, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x8D, 0x6C, 0x24, 0xD9, 0x48, 0x81, 0xEC, 0xC0, 0x00, 0x00, 0x00, 0x4C, 0x8B, 0xFA, 0x4C, 0x8B, 0x71, 0x08, 0x49, 0x8B, 0x7E, 0x78}) |
| C | C101 kStatusVtable | Standard support for candidate 0x5f63430 (or its documented native operand anchor) is absent; independent check: Exact MSVC RTTI class/COL identifies this component vtable; slot +50 points to the independently reviewed native loader with matching role and changed fields. Standard loader agreement can anchor the paired slot relationship; RTTI naming alone does not satisfy adoption. |
| C | C102 kStatusLoad | Standard support for candidate 0x2c160b0 (or its documented native operand anchor) is absent; independent check: Paired ServerStatusActorComponent loads call the common status loader, propagate its result and write INT64_MAX through the character-number pointer, changed +70 -> +90; target LoginCharacter lookup consumes +90 independently. Exact named RTTI slot +50 supplies entry identity. |
| C | C103 kLoadStub | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kLoadStub at 2976 0x35EA90 |
| C | C110 kTravel | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kTravel at 2976 0x2BC0A30 |
| C | C111 kTravelHead | Confirm 3663 code consumes kTravelHead for the same native role as src/mod/session2/clock_gate.cpp:22 ({0x48, 0x89, 0x5C, 0x24, 0x10, 0x48, 0x89, 0x74, 0x24, 0x18, 0x48, 0x89, 0x7C, 0x24, 0x20, 0x55, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x8D, 0x6C, 0x24, 0xC9, 0x48, 0x81, 0xEC, 0xC0, 0x00, 0x00, 0x00, 0x48, 0x8B, 0xF2, 0x48, 0x8B, 0xF9}) |
| C | C112 kTravelOwnerCmp | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kTravelOwnerCmp at 2976 0x2BC0A78 |
| C | C113 kTravelOwnerCmpBytes | Confirm 3663 code consumes kTravelOwnerCmpBytes for the same native role as src/mod/session2/clock_gate.cpp:26 ({0x39, 0x82, 0x80, 0x01, 0x00, 0x00}) |
| C | C114 kTravelGetterCall | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kTravelGetterCall at 2976 0x2BC0CC7 |
| C | C115 kTravelGetterCallBytes | Confirm 3663 code consumes kTravelGetterCallBytes for the same native role as src/mod/session2/clock_gate.cpp:28 ({0xE8, 0xD4, 0x48, 0x52, 0xFF}) |
| C | C122 kSessionId | Confirm 3663 code consumes kSessionId for the same native role as src/mod/session2/server.cpp:76 (0) |
| D | D030 kSamBucketMaxEntries | Confirm 3663 code consumes kSamBucketMaxEntries for the same native role as src/mod/session2/guest_lifecycle.h:55 ((kSamBucketBytes - 8) / 8) |
| D | D044 kTsMoving | Confirm 3663 code consumes kTsMoving for the same native role as src/mod/session2/guest_lifecycle.h:199 (0x3DC) |
| D | D045 kFmMoving | Confirm 3663 code consumes kFmMoving for the same native role as src/mod/session2/guest_lifecycle.h:200 (0x58) |
| D | D063 kCallSites[0].site | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kCallSites[0].site at 2976 0x2C17881 |
| D | D064 kCallSites[1].site | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kCallSites[1].site at 2976 0x2C3E1CE |
| D | D065 kCallSites[2].site | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kCallSites[2].site at 2976 0x1042ABF3 |
| D | D066 kCallSites[3].site | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kCallSites[3].site at 2976 0xCBD86CE |
| D | D067 kCallSites[4].site | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kCallSites[4].site at 2976 0x103BE99A |
| D | D100 kRetUserLogin | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kRetUserLogin at 2976 0x2C17886 |
| D | D101 kRetLobby3093 | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kRetLobby3093 at 2976 0x2C3E1D3 |
| D | D102 kRetCharacter3093 | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kRetCharacter3093 at 2976 0x1042ABF8 |
| D | D103 kRetAutosave | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kRetAutosave at 2976 0xCBD86D3 |
| D | D104 kRetManual2910 | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kRetManual2910 at 2976 0x103BE99F |
| D | D115 kReporterHead | Confirm 3663 code consumes kReporterHead for the same native role as src/mod/session2/report_guard.cpp:20 ({0x48, 0x89, 0x5C, 0x24, 0x10, 0x55, 0x56, 0x57, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57}) |
| B | B075 kMaxPacketSize | The legacy shared packet limit 7FE0 differs from the target native PostPacket bound 7FDA and ordinary decoder payload bound 7FC1. A target-specific acceptance bound and complete framing check remain necessary. |
| C | C144 kActor_OwnerUser | Confirm 3663 code consumes kActor_OwnerUser for the same native role as src/mod/game/offsets.h:230 (0x90) |
| D | D130 NativeTitleSlotDomain | Target title eligibility adds per-entry u32-list membership at +68/count+70 against available .admeta IDs at global 71AEAA0/71AEAA8. Target producer 14AA720 reads those IDs in addition to names; reference producer 128D360 only builds names. The new global dependencies, list lifetime/coherence and complete target parser remain unadopted/unimplemented. The old domain-only playable count is insufficient. |
| B | B078 NativeTrocScanSkip | The native descriptor scan skips 80h bytes after a match, then advances eight more. Target minimum instance span/alignment for every scanned descriptor is not yet established; the skip must not hide a neighbouring target descriptor. |

## Smallest next work and manual validation

1. Keep the game closed; follow the recorded client loading start/completion
   path to establish which non-mutating fields reset and become ready for the
   controlled actor, with lifetime/coherence rules. No native helper calls.
2. Extend the passive collector only after that field contract is reviewed;
   test missing, changing and incoherent data offline. Then request one exact
   user-present travel session with fresh backups and the existing watchdog.
3. Once all A values are adopted, publish the real HP-only per-build layout,
   explicitly retire unreachable legacy stamina selection for that target,
   rerun guards/full CTest/loopback and perform real L1 before progressing.
4. Resolve the independent B/C/D contract and adapter gaps recorded above.
   L2, L3 and the second-PC L4 remain ordered by the plan; do not substitute
   loopback or simulated actors for native two-player acceptance.

Reproduction tools: r2_refresh.ps1 regenerates the native catalogs and join
from the exact completed outputs; r2_match_tests.py verifies the join/conflict
rules; scripts/build.ps1 -BuildDir <local r2-build> builds/tests; smoke.ps1
uses only game-free test hosts; r2_status.py materializes the status;
package_r2.py packages a clean committed HEAD and verifies its manifests.
Source/runtime changes, evidence and commands are inspectable through the
commits below, the generated manifest, and SOURCE-DIFF.patch.

## Commits since round 1

- `90c12f5` R2 preflight: record passing baseline and missing plan blocker
- `29bc5df` R2 step 1: cut option-D game contracts into four core tiers
- `c5f7074` R2 step 6: clarify per-peer highest-generation admission
- `5a72f3b` R2 step 2 checkpoint: preserve native proofs and closed reader guards
- `5df9866` R2 step 2 checkpoint: verify native layouts and prepare passive HP trace
- `06e8b72` R2 tier A: isolate disabled reader route and validate startup effects
- `2daa8a3` R2 step 2 checkpoint: verify native wire and lifecycle contracts
- `30d105e` R2 step 2 checkpoint: verify identity layout and full exporter equivalence
- `7edb209` R2 step 2 checkpoint: verify save contracts and prepare loading trace
- `8e1b8ef` R2 step 2 checkpoint: verify save and receive contracts; harden evidence joins
- `5bf8fe4` R2 step 2 checkpoint: verify native descriptor field roles
- `2208404` R2 step 3: record field-sharing ABI assessment and ownership blocker
- `718ced6` R2 tier A preparation: guard definition-derived native HP reads
- `18635b3` R2 preflight: verify identity/save contracts and prepare guarded passive sessions
- `1fa910d` R2 step 5 P1 HP: record UI match and enforce hard session deadline
- `8d51290` R2 step 5: preserve loading trace and validate deadline receipts
- `90e92bb` R2 step 2: verify lifecycle and clock publication contracts
- `9c63f13` R2 step 2: join whole-image standard matches and native checks
- `dba9721` R2 step 2: complete default matching and native conflict audit
- `baebf12` R2 step 3: record field-sharing ABI and ownership blockers
- `582c2e9` R2 step 4 tier B: record closed-gate decision and validation
- `99d5329` R2 step 4 tier C: record closed-gate decision and validation
- `7992fa5` R2 step 4 tier D: record closed-gate decision and validation
- `185e63a` R2 step 4 tier A: retain closed guard after loading trace
- `6c3817e` R2 step 5: preserve synchronized loading trace and cleanup
- `521b4fd` R2 step 5: include bounded passive session logs
- `180da75` R2 tier C-D: verify native session and save-lock contracts

The generated manifest appends the Step 7 source commit and records the exact final commit.

---

# R2 preflight — 2026-11-15: execution plan missing

The user requested Steps 1–7 and per-step/tier commits on gpt/3663-port.
AGENTS.md and CLAUDE.md were read. docs/GPT-3663-PORT-PLAN-R2.md is absent
locally, from every fetched remote branch, and from fetched Git history;
searches of Downloads and prior workspaces did not recover it.

Required independent baseline passed: scripts/build.ps1, x64 RelWithDebInfo,
101/101 CTest tests in 109.20 seconds. The installed executable and stored
2976 reference retain their previously verified hashes; the game was closed.
No production source, guard, offsets, deployment or saves were changed.
No R2 step/tier or Step 7 package is claimed complete without its instructions.

See R2-PREFLIGHT-BLOCKER-20261115.md and r2-evidence/ for the exact search
scope, build output and current file identity. Required next input is the R2
plan's contents at the requested path or a retrievable file/ref. Existing
execution and push authorization remains in effect when that input is available.
The Phase 9 section below describes the historical R1 package at 2c80c21.

---

# Phase 9 — blocker package for Claude's final review

**The native two-player 3663 goal was not reached.** This is the plan's blocker
handoff, with all independent completed work preserved and pushed.
No live two-player, native spawn/movement, disconnect/removal or save-isolation
result is claimed. The 3663 gameplay gate remains unarmed.

Branch: `gpt/3663-port` at
https://github.com/Shaugno/CrimsonDesertMultiplayer-NativeServer/tree/gpt/3663-port.
Base: `a8656e87a0c43db400bdf863193c1afb353d6de0` (`origin/grok/compat-guard`).
Final package commit: the Phase 9 commit containing this section. Its exact
full SHA and the full commit list including Phase 9 are written **after the
commit/push** into `PHASE9-MANIFEST.json` inside the local review ZIP in
`dist/phase9/`. `tools/port3663/package_phase9.py` reproduces the package directly
from committed Git blobs, not working-tree files. The final chat also names
the final SHA. No release or binary publication was performed.

## Completion boundary

| Phase | Actual result |
|---|---|
| 0 | Complete: all five review fixes, regressions and full suites pass |
| 1 | Complete required references: 2976 downloaded/verified; optional 3576 midpoint unavailable |
| 2 | Partial: reproducible conservative inventory; semantic/minimal-reachability audit remains |
| 3 | Partial: 17 corroborated values; complete core map unresolved |
| 4 | Partial: separate static header and strict recognition; native modules unarmed/unrouted |
| 5 | Complete game-free validation of this guarded checkpoint, not of a functional port |
| 6 | Blocked before any live session |
| 7 | Blocked before any two-instance session |
| 8 | Blocked before any two-PC Steam session or guest kit |
| 9 | Blocker review package |

Contiguous implementation completion ends at Phase 1. Independent work extends
through the Phase 5 validation. The Phase 6–8 commits record blocked status;
their existence is not evidence that live acceptance passed.

## Build, identities and hashes

x64 RelWithDebInfo, VS 18 Community, MSVC toolset 14.52.37025 / compiler
19.52.37029.0, Windows SDK 10.0.26100.0, C++20 and static CRT. The tested ASI
contains clean source build ID **3C3EC37B** (source commit
3c3ec37b713d7e676cd6bb898b21e32e92075699). Later commits are analysis/reporting
changes. Binary SHA-256 values and sizes: `port-evidence/phase5-binary-hashes.json`.
ASI hash: `5E2FC9BEF6D8802150936FC381F23AE89C061AB53A78F5E3DEEA9592D67C9FEE`.
Compiled binaries remain local in `build/bin/RelWithDebInfo` and `build/e2e`.
The Phase 9 review ZIP deliberately contains source/evidence only.

2976 reference (outside repo):
`C:\Users\shaug\.codex\visualizations\2026\11\14\01a1dbbd-2726-7441-9a57-1b454819b3e7\reference-bins\2976\CrimsonDesert.exe`,
SHA-256 `57DA440D72F4DB974F25FEF047CF84C4DADD999A88CB2A3C5AF4C9BD67FDE1E7`.
The user completed the single Steam depot touchpoint. See PHASE1-REFERENCES.md.

Installed 3663 target: timestamp 0x6AF5D198, SizeOfImage 0x144E9000, preferred
base 0x140000000, SHA-256
`8BE67178D9B9A149C361A731051665E0BA4D441E250AE1DFE645BD6E524345C8`.
Re-hashed for Phase 3 and each Phase 6–8 preflight, with no drift. JSON file
facts are in port-evidence/. Neither game executable was executed. Installed
files and saves were not changed or read as a live process.

Evidence SHA-256 values for the committed Git-blob bytes are recorded in
PORT-EVIDENCE-SHA256.json; this avoids ambiguity from Windows checkout newline
conversion. The ZIP's PHASE9-MANIFEST.json hashes every included file and records
the exact final commit. Evidence text has trailing whitespace normalized.

## Inventory, matches and the next experiment

PORT-INVENTORY.md and its JSON/literal supplement are pinned to the 2976 source
baseline. They include 1292 named rows and 8261 inline scalar occurrences in
193 files. These are a conservative audit superset, including policy values
and disabled headers, not 9553 required game offsets.

PORT-MATCHES.md / PORT-MATCHES-LITERALS.md list every row and its missing check.
**17 named values adopted as strong static identities; 1275 named plus 8261
inline occurrences remain unadopted/unclassified.** Three PE identity fields
are separate file facts. Only the 17 reviewed values enter build_3663.h;
none enables native execution. Weak discoveries remain in evidence only.

Methods: paired targeted Ghidra 12.1.4 analysis; Zydis whole-pdata normalized
fingerprints (242245/247296 decoded functions; 57160 unique candidates); named
RTTI/COL/vtable slots; existing short signatures; call-neighbour ranking;
strict pkt3663 entry verifier. This is not completed project-wide BSim/Version
Tracking. Ghidra/JDK projects/tools remain local; reproducible scripts and
commands are in tools/port3663/README.md. A concrete changed field is the heap
selector TLS byte: 2976 +0x1FD becomes 3663 +0x1FE, independently confirmed at
multiple native callers. Copying layouts unchanged is unsafe.

All blocking groups, with precise missing evidence:

1. **Field sharing:** old AcquireField 0x2B07790 and EnterField 0x2BC04F0 have
   no corroborated target after signature, normalized-function and neighbour
   matching. Confirm current function entries, the two locks, field user list,
   counts, reference ownership and rollback before installing H1/H2.
2. **Identity/status:** component-load and phase hooks have multiple candidates
   without independent role/register checks. Confirm current lobby/status
   number storage and post-load ordering before any identity write.
3. **Player/SAM and cleanup:** confirm singleton provenance, guarded player/HP
   chain, exact-key lookup, field user/actor accounting, native logout and
   deferred removal barriers. The 2976 maps cannot simply be reused.
4. **Wire completeness:** current descriptor IDs are discovery evidence only;
   confirm each handler/writer's exact consumption, conditional fields, sizes,
   channel/kind semantics and frame tag/tail usage. No old ID or layout adopted.
5. **Guest saves:** matching one refresh thunk does not establish early guest
   interception, save ownership or load ordering. These remain a live blocker.

**Smallest next experiment:** file-only trace of the named 3663
LoginCharacterToFieldServerReq handler at 0x2C6AE10 to its field-acquisition and
entry call paths; corroborate both targets and the actual list/lock writers
against the paired 2976 functions. This directly targets the first core blocker.
The existing coop3663 structural decoders/FakeActorBackend do not provide field
sharing, initialized native characters, identity fixups or deferred cleanup;
they are preserved as fallback research/tests, without expansion in this run.

## Exact validation and live-session record

| Check | Command / evidence | Result |
|---|---|---|
| Phase 0 baseline | `scripts/build.ps1`; phase0-baseline.txt | 101/101, 139.68 s |
| Phase 0 fixed | `scripts/build.ps1`; phase0-fixed.txt | 101/101, 93.77 s |
| Focused feed/decoder | build target cdmp_tests_coop3663, then executable | Passed |
| Phase 4 guard | build target cdmp_tests_build_guard, then executable; phase4-guard.txt | Passed, including wrong-size 3663 and zero gameplay callbacks |
| Phase 4 ASI | `cmake --build build --config RelWithDebInfo --target cdmp_mod` | Passed |
| Phase 5 full build/suite | `scripts/build.ps1`; phase5-suite.txt | 101/101, 111.81 s |
| Phase 5 synthetic loopback | `tools/port3663/smoke.ps1`; build/e2e host/peer, UDP 39763, 8 s | 78 remote states, sent 85/recv 83, rejected 0, clean shutdown/unload |
| Phase 6 | PHASE6-LIVE-STATUS.md; phase6-preflight-identity.json | NOT STARTED; blocked |
| Phase 7 | PHASE7-TWO-INSTANCE-STATUS.md; phase7-preflight-identity.json | NOT STARTED; blocked |
| Phase 8 | PHASE8-TWO-PC-STATUS.md; phase8-preflight-identity.json | NOT STARTED; blocked |

Failures/corrections: initial sandbox configure could not resolve github.com
for pinned dependencies; the authorized network-enabled build succeeded.
The first normal push stalled in the configured credential helper; the existing
authenticated GitHub CLI helper completed all phase pushes without new login.
A Python syntax-check invocation passed a literal wildcard; compileall corrected
the invocation and passed. No CTest failures or live crashes occurred in this
run. Full outputs are retained; the corrected commands, not suppressed results,
are the recorded validation. Self-review and scope are in PHASE5-VALIDATION.md.

No live session began, so no save backup, live log/capture or crash context was
generated. The Phase 6–8 files state the question, expected observation, time
limit and stop condition for future sessions. No deployment/removal or launcher
command ran. The combined Phase 6–7 presence and Phase 8 second-PC touchpoints
remain unused until their offline prerequisites are satisfied.

## What works and what remains disabled

Works: Phase 0 synthetic lifecycle/decoder fixes; reproducible static reference
comparison; a limited corroborated value header; strict 3663 identity rejection
of all gameplay side effects; complete game-free suite and synthetic transport.

Disabled on 3663: all native modules, including the intended minimal path,
plus mounts/rope/shops/docked gimmicks, NPC/enemy/control/combat probes,
scene/schedule extras, map/minimap and overlay. Compiled legacy defaults do not
grant permission because the central gate remains closed. 2976 address tables,
legacy module implementations, wire version, Steam-friends restriction and
executable filter are preserved. 3576 stays recognized and unarmed.

Do not deploy this branch as a working 3663 co-op kit. Passing loopback is only
synthetic transport evidence. Native walk/turn, creation/removal, reconnect,
save safety and real Steam tunnel acceptance remain unverified.

## Commits since the requested base

All checkpoint commits below were pushed to origin/gpt/3663-port. The final
Phase 9 package commit and its full SHA are appended to this list in the ZIP
manifest generated after publication.

| Commit | Subject |
|---|---|
| 3cc9c28018de903fdc8a5bfcceb322033b42dace | Phase 0: fix offline feed generation fences and review findings |
| 5abdc221b524e6d12b674ff55f2f26dcba19d550 | Phase 1: verify and record static reference executables |
| 57097568652233fff6088d548e7b7c3f0101e16d | Phase 2: inventory option-D dependencies and record 3663 feature boundary |
| 878d14fe129b5569c606318677e9703f0a4ce972 | Phase 3: corroborate static matches and document unresolved core port rows |
| 3c3ec37b713d7e676cd6bb898b21e32e92075699 | Phase 4: recognize 3663 while keeping incomplete gameplay path unarmed |
| 8e454e945f744f03ce1c8e6a59dcc6f8b369afc4 | Phase 5: validate guarded build and synthetic loopback, pin inventory source |
| fc61b770a39bfb78a48d8b6f61a7afb983699b81 | Phase 6: record live prerequisites blocked and verified build identity |
| 5b3e54062c437ad4a56ecd3d8218e6e08a3fd583 | Phase 7: record blocked two-instance acceptance and preparation |
| 0d4490706fecce9060756dcd84331257f6162e92 | Phase 8: record blocked Steam two-PC acceptance and guest-kit prerequisites |

---

## Historical checkpoints (superseded by the Phase 9 status above)

# 3663 port run — Phases 0–1 complete, later phases in progress

Branch: gpt/3663-port, based on a8656e87a0c43db400bdf863193c1afb353d6de0.
The user's current request and docs/GPT-3663-PORT-PLAN.md supersede the historical
review/offline-only cadence below. Live work remains limited to Phases 6–8 and
the plan's prerequisites; no live work has started.

Phase 0: all five review findings fixed, focused regressions pass, baseline
101/101 (139.68 s), post-fix full suite 101/101 (93.77 s). See PHASE0-PORT.md
and port-evidence/phase0-{baseline,fixed}.txt for full commands/output.
Phase 1: the user completed the depot download; the copied 2976 reference
matches SHA-256 57DA440D72F4DB974F25FEF047CF84C4DADD999A88CB2A3C5AF4C9BD67FDE1E7.
Paths and target/reference identities are in PHASE1-REFERENCES.md and its
referenced JSON files. Both executables are file-analysis inputs only.
The production 3663 path is still blocked. No two-player result is claimed.

Phase 2 is a conservative inventory with semantic/reachability audit remaining.
Phase 3 performed paired Ghidra analysis, whole-pdata normalized matching,
RTTI/slot and signature checks, and call-neighbour ranking. Seventeen values
are corroborated static identities. The complete core table is not resolved.
PORT-MATCHES.md lists every named row, the literal supplement, and five precise
blocking groups, including the next file-only field-sharing experiment.
Raw discovery confidence is not automatically promoted. No weak candidate is
allowed into a runtime path. Phase 4 added the corroborated static-value header
and strict recognized-but-unarmed 3663 identity. Guard regressions and the ASI
build pass; the complete per-build module routing remains blocked. See
PHASE4-IMPLEMENTATION.md. Phases 5–9 independent work is continuing.

The following sections are preserved historical checkpoints.

# Handoff: 3663 acknowledgement findings fixed; walk prototype blocked

User-requested transfer to Claude: [implementation handoff](../CLAUDE-HANDOFF.md).
Continue offline; preserve any newer local changes. The user requested this
progress checkpoint be committed and pushed to Shaugno before Claude takes over.
Earlier references to uncommitted work describe its pre-publication status.

## Latest checkpoint: concrete character-control component reader

See [COMPONENT-READERS-20261114.md](COMPONENT-READERS-20261114.md).
Resolved character-control and transform-sync class vtables through file RTTI.
Added an offline structural character-control decoder for its conditional
fields, variable bytes, resource IDs and variable entries. Nested reads are
accounted for; native object stride is not used as wire length. Focused tests
pass. Full offline suite passed 101/101 in 90.26 seconds, including the prior
controller fixture isolation fix. git diff --check passed. Native character creation is still
unimplemented; component inventory, transform structures, resource validity,
and ownership remain unresolved. No game use or deployment. Changes local.

Earlier checkpoints below retain their historical scope.

## Current Codex checkpoint: component initialization and cleanup

See [COMPONENT-LIFECYCLE-20261114.md](COMPONENT-LIFECYCLE-20261114.md).
New file evidence establishes the compact component list's initialization
order, shared-reader deserialization, reverse cleanup order, and a deferred
cleanup callback. The concrete factory and component schemas remain unknown;
these native list entries are not wire layouts. Failed-initialization rollback
and deferred completion ownership are still unresolved.

An offline decoder now checks the body relation tail only when its start is
independently known. It never guesses where opaque components end. Focused
tests passed. Full suite: 100/101 passed in 89.65 seconds. The controller
fixture hit a stale directory from a recycled dummy PID. Its test-only log
naming now includes the test process identity/creation time; the rebuilt
controller test passed in 49.03 seconds (CTest total 49.38). No failure was
suppressed and old evidence was preserved. The other 100 tests were not
rerun after this isolated fixture change. No game launch,
attachment, native calls, deployment, or push; changes remain local.

Next dependency: resolve the descriptor factory and each required component
reader, initialization phase and cleanup contract before native actor work.
The earlier checkpoint sections below retain their historical validation.

## Current Codex checkpoint: spawn/rotation/removal offline parsers

Latest evidence and implementation:
[SPAWN-HEADING-REMOVAL-20261114.md](SPAWN-HEADING-REMOVAL-20261114.md).
The bounded offline library now parses spawn record envelopes, the
file-traced four-component rotation, and the removal layout (including
its leading byte). A labeled synthetic test composes decoded fields through
fake actor create/move/remove and verifies late samples are refused.
Native actor creation/removal is still not implemented: component-specific
spawn bodies, native ownership/thread contracts, and live conventions remain
unresolved. Nothing was deployed, no game process was opened, and guards
are unchanged. Current implementation is local and uncommitted.

Validation: full suite 101/101 in 90.26 seconds; the final composed fixture
was added during that run and then separately rebuilt and passed. See the
new report for exact scope, evidence hash, and remaining dependencies.

The paragraphs below retain earlier checkpoints and their historical limits.

Latest offline implementation: `decode_move_ack_spatial_fields` now extracts
only the file-proven raw word and three finite spatial floats from the
239-byte full-parse MoveActorAck layout. It checks header, file-descriptor
opcode, exact frame length, and retained byte coverage. It does not provide
heading, coordinate-space classification, or live readiness and is not
connected to a game backend. New regression coverage and the full independent
suite passed: 101/101 in 98.97 seconds. Later tracer-only extensions rebuilt
and ran successfully. See the updated movement-consumer note for lifecycle
types and the remaining work. Nothing has been deployed or pushed.

Codex takeover update, 2026-11-14: the user assigned implementation and
investigation to Codex. New file-only results are in
[MOVEMENT-CONSUMERS-20261114.md](MOVEMENT-CONSUMERS-20261114.md) and
ack-trace-consumers-20261114.txt. The three floats at payload offsets
196/200/204 reach a conversion using 1000-unit spatial cells. Coordinate
space and live meaning remain unverified. The earlier blocker description
below is historical; independent investigation is continuing.

Date: 2026-11-14. Branch `grok/compat-guard`, HEAD `502577a`, local
diff uncommitted. This is a blocker handoff. Two installations have not
been shown walking or turning. No actor was spawned, no packet was
replayed, and no diagnostic was deployed. Production guards, protocol
`kVersion`, and the installed game file were not changed.

## Findings from ACK-HANDLER-REVIEW-20261114

1. The 17-byte MoveActorAck decoder is removed. `exe+0x26A8CE0` consumes
   the same reader between the first 4-byte read and the later reads.
2. The flat ActorLoad u16 decoder is removed. Slot 2's start is a runtime
   subtraction. `exe+0x9D68E0` receives the reader and reads further bytes
   itself; the caller's u16 is not its integer argument.
3. The literal header bound matches `cmp ax, cx` / `jb` at
   `exe+0xCA8B01`. Declared length `0x7FC1` passes. Only a greater length
   fails when `bound_flag` is 0. ActorLoad slot 2 uses a different bound
   and is not accepted by this gate.

## What the success path consumes

File-only `SEC_IMAGE` trace. The tool printed
`identity=build-3663 timestamp=0x6AF5D198 adopted=0` and
`move-reader-vtable=0x5B3BE18 slot28=0x141643CD0`. Listings:
`ack-trace-20261114.txt`, `ack-trace-helpers-20261114.txt`,
`ack-trace-nested-20261114.txt`.

MoveActorAck builds its reader at `rbp-0x80` with vtable `exe+0x5B3BE18`,
base `bytes+5` at reader `+0x20`, declared length at reader `+0x28`, and
cursor 0 at reader `+0x2C`. Slot `+0x28` is `exe+0x1643CD0`. That function
copies exactly `r8d` bytes and advances the cursor by the same count. The
handler fails the later reads when the slot is `exe+0x3A6E20`.

`exe+0x16436F0` only grows the pointer array at reader `+0x10` and
increments reader `+0x18`. It does not call slot `+0x28`.

On the success path the cursor therefore advances by:

| Step | Bytes | Running cursor |
| --- | ---: | ---: |
| first read into `rbp+0x148` | 4 | 4 |
| `exe+0x14CB240` | 136 | 140 |
| `exe+0x1644720` into object `+0x88` | 40 | 180 |
| `exe+0x1644640` into object `+0xB0` | 16 | 196 |
| object `+0xC0`, `+0xCC`, `+0xD0`, `+0xD8`, `+0xDC`, `+0xDD` | 12+4+8+4+1+1 | 226 |
| one byte, then 8, then 4 | 1+8+4 | 239 |

`exe+0xCA8D63` requires that cursor to equal the declared length. A
successful return has consumed 239 payload bytes. The 136-byte prefix is
`exe+0x14CB150` twice (64 each) plus one 8-byte read. Each 64 is
`exe+0x1644720` (12 + 16 + 12) plus two 12-byte reads. This is the
handler's own success path, not a captured message.

For the object-kind branch `[type+1] == 1`, `exe+0x55EF60` copies:

| Source | Payload offset | Store |
| --- | ---: | --- |
| 16 bytes at object `+0xB0` | 180 | `[A+0x55C]` |
| 8 bytes at object `+0xC0` | 196 | `[A+0x550]` |
| 4 bytes at object `+0xC8` | 204 | `[A+0x558]` |
| 8 bytes at `rbp-0x40` | 227 | `[D+0x90]` |

With S=qword[r14], A=qword[qword[qword[S+8]+0x68]+0x1A0], whereas
D=qword[S+0xE8]. These are different pointer paths; aliasing is unproved.

Those are byte copies. The instructions do not show a world position or a
heading. No decoder maps them to movement.

`exe+0x9D68E0` reads 1 byte, then 4, masks that dword with `0x30000000`,
and compares it with `0x10000000`. The matching arm calls
`[result+0x130]` with the reader. Allocation/removal through downstream
callees has not been established or ruled out.

## Tests

`ack_header_ok` checks the span, the declared-length equality, and the
inclusive `0x7FC1` bound. The movement and actor-load field decoders are
gone. The regression covers `0x7FC0`, `0x7FC1`, and `0x7FC2` with flag 0,
`0x7FC2` with flag 1, a short span, and a length mismatch.

Game was closed. Sibling RelWithDebInfo build, MSVC 19.52, then CTest:
101/101 passed, 0 failed, 124.67 s. The tracer changes after that run do
not link into a tested binary. No game process was launched or stopped.

## Blocker

Two-player walking is not demonstrated. The missing fact is the meaning
of the four stores above and a 3663 create/remove operation. The 239-byte
cursor map does not identify a player position, a heading, or an actor
lifetime, so neither a position reader nor a packet relay is selected.

Smallest next experiment: file-only, find one later read of
`[inner+0x550]` or `[inner+0x90]` that uses the value in a demonstrated
world-coordinate operation, or show that those slots are not coordinates.
Do not deploy a decoder and do not repeat the empty post-ready capture
for this question. A second installation cannot show the other player
until that fact exists.
