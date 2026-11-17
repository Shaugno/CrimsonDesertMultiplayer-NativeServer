# R5 Step 3 — visible desktop retry results (2026-11-17)

**L3 remains unpassed.** Both actual controlled game windows were finally
visible to the user and arranged left/right before guest character selection.
Native login and title refresh passed; the guest's Play request was refused
by the host identity admission gate before actor insertion. L4 cannot run.
This section supersedes the historical status below; original evidence stays.

The code checkpoint is **de2bd84**, with the unchanged sealed ASI
`08A8ABEB7431703444AFC09ED44986ED493DECF24D103B8A121C8701309840DD`
and desktop runner
`8985660E38F24A8B7256918087A5CBF9829A775DD92DD0FBCE66C4280E80CB09`.
Its full suite remains **119/119, 115.14 seconds**; synthetic smoke recorded
78 remote states, zero rejects and clean unload. The source/binary/runner
seal was rechecked after the live sitting. No native code changed afterward.

## Three additional recorded experiments

| ID | Category | Result |
| --- | --- | --- |
| R5-L3-2 | Startup | Visible desktop host loaded and exited normally, code 0. Operator delayed host-load instructions while trying frame arrangement and missed the six-minute guest-start reserve; no guest launched. |
| R5-L3-2b | Join experiment | Visible host loaded and exited normally, code 0. Computer Use denied the guest application, and the user saw no approval prompt. No guest process or native join ran. Conservatively retains its already-promoted join count. |
| R5-L3-3 | Join | User manually started both validated desktop runners from prepared shortcuts. Both controlled windows were visible and arranged before Play; native character-login request refused before insertion. Guest exited normally, host deadline-stopped. |

Startup usage is **4/5**, join usage **3/3**. The saved ledger includes
the startup-only trial and blocked application-access experiment after de2bd84;
the resumed prompt's estimate of two remaining in each category was stale.
No record was erased or counter reset. No further join launch was made.
There were two guest native-login conversations in R5, not three successful
joins. The application-access block was handled by an explicit manual user
handoff, never by launching the denied app through another automation tool.

## R5-L3-3 observations and acceptance

The host PID 47812 launched at **08:23:09 Eastern**, the guest PID 25516
at **08:25:41**. Both used the exact 3663 executable and fresh verified
external backups. Host file sandbox hooks were absent; the guest verified
its native profile save root with zero file hooks. The common watchdog
deadline remained **08:37:09**, with no extension. Shader compilation
delayed the usable guest menu. Native cryptogram/login and ten-entry title
refresh were recorded at 08:28:44. These do not establish a character join.

The user confirmed both usable windows visible side by side. Initial reports
still showed vertical overlap, so character selection stayed on hold until
the actual left/right geometry was verified at **08:33:34**: host HWND
5117678 `[0,0,1296,839]`, guest HWND 4526474 `[1281,0,2577,839]`.
Their outer borders overlap 15 pixels and the guest's outer edge extends
17 pixels past a 2560-pixel monitor; no pixel-disjoint-border claim is made.
The user acknowledged the corrected arrangement, then guest Play was signaled.
Frozen PID/birth-bound window reports and confirmation are retained locally.

After Play, the native guest sent count changed from two to three and the
host identity refusal count from zero to one (08:34:07). Host status/phase
normalizations stayed zero; EnterField insertions/commits stayed zero.
`identity3663::admit` increments this counter only for **3081**, identifying
the character-login admission boundary. No native fault was logged. The
payload and individual failed predicate were not retained, so the exact
cause is unresolved. The user later confirmed both copies closed but gave
no exact guest menu/loading/error result; none is invented here.

The guest exited normally, code 0, at **08:36:12**. The host remained alive
until the pinned watchdog stopped it at **08:37:10**, exit 0x3663. User
confirmation of closure does not make that a normal exit. Host reload after
the session, mutual visibility/movement, removal and reconnect remain
unverified; no actors existed to support removal acceptance.

The real-save manifest after guest exit matches the host-only pre-guest
checkpoint: **zero changed files** across all 23. Host autosaves before that
checkpoint changed the original pre-session snapshot and were retained.
Endpoint equality is observed; concurrent manifests alone cannot identify
a writer or exclude transient writes. The earlier isolated S2 guest-only
manifest acceptance remains separate. All seven sessions' frozen backup
copies verify, current real saves match the latest post-session manifest,
and all controlled installation files and watchdogs are gone. No real save
was restored over.

The user started the ten-second desktop capture, but it again produced a
**232-byte zero-frame AVI**, SHA-256
`C674E1892CDD9CC75B447BED09F9197D1E85F4BE1E6E7ABFDDD4A575A76B8DF5`.
The corrected validator rejected it. No foreground test PID was retained;
there is no useful visual/movement recording. Focus sequence, desktop
context and birth-validation rejection cannot be distinguished from its
metadata. Visible window inventory does not make an empty clip valid.

## Independent analysis and next reproducible work

The identity gate requires exact 13-byte payload length, assigned guest
number, a permitted slot, and matching epoch/user/generation state. The
runtime counter localizes the refusal but does not select one predicate.
The old 2976 research also contains a 19-byte login variant; it is a
hypothesis to check against the exact 3663 serializer, not permission to
accept trailing bytes. Native title refresh and the resulting requested
number likewise require packet-specific evidence. No gate, payload check,
offset, native identity rule or Steam restriction was relaxed.

Next independent work: retain bounded reason-specific admission metadata
and prove the actual target packet with an owned reproduction before any
repair; reproduce capture with an owned desktop pair and preserve sampled
foreground/birth rejection reasons without capturing unrelated windows.
The current recording failure has no reproduced root-cause fix. Further
runtime validation would require a later authorized budget; no review gate
was inserted into the remaining offline work.

`port-evidence/r5/desktop-retry-results.json` seals each new trial's own-PID
logs, launches, exits, manifests, cleanup, arranged-window reports and empty
capture metadata. Stale guest log aliases in host-only trials are excluded.
The earlier trial's evidence remains in `step3-results.json`.

---

# R5 Step 3 — L3 boundary and window-visibility blocker

**L3 is unpassed.** The one counted join experiment, R5-L3-1, reached native
guest login and native title refresh, but no guest character-load request
was observed. Startup usage is 3/5; join usage is 1/3. The remaining budget
does not establish a reason to relaunch without resolving the visibility
boundary. No review approval is being requested.

## Recorded experiment

On 2026-11-16 Eastern, the controlled host PID 31020 launched at 21:48:17
and loaded its save. The isolated guest PID 6616 launched at 21:52:12.
Both used the source checkpoint 966cb5f, ASI SHA-256
`603D727CC94EF6DA3C2FF5F620967FC912621F383DD062D92685A8EB6C0D27DF`,
embedded banner `C9339541 + local changes`, exact target executable,
verified external backups and a common deadline of 22:02:17.

The host installed the credited field-leave guard. The guest verified its
native profile save root with zero file hooks and installed the credited
achievement guard before native traffic. Native cryptogram/login completed,
host lobby normalization counted one, and the original title parser refreshed
ten entries. The guest's sticky remote guard became active. There was no
logged native fault, refusal or quarantine, and both processes remained alive.
These observations establish the bootstrap boundary; they do not establish
title eligibility, character loading, native actor creation or visibility.

The host recorded zero EnterField insertions/commits and zero guest status
or phase normalization. The user reported seeing one game, described as
Steam-started, and could not locate two games. That report does not identify
a PID. Process identities and log aliases alone cannot establish two usable
windows. No hidden-desktop, bootstrap, minimization or native title-predicate
cause is asserted from this record.

The ten-second capture produced a 232-byte zero-frame AVI with no foreground
test PID. Its SHA-256 is
`C674E1892CDD9CC75B447BED09F9197D1E85F4BE1E6E7ABFDDD4A575A76B8DF5`.
It is retained locally as failed evidence, never gameplay proof. Both exact
processes were stopped by their fixed watchdogs at the shared deadline;
neither exit is called normal. All 23 real-save files and both frozen backups
match their manifests, and all six controlled installation files were removed.
No real progress was restored over. Host reload after this L3 session,
mutual movement, guest removal and reconnect remain unverified.

The full immutable log/receipt/manifest/capture-metadata hashes are in
`port-evidence/r5/step3-results.json`. The original arming receipt is retained
inside the session evidence before the later validation receipt was updated.

## Independent work after the blocker

The command tool, Explorer and Steam were observed in session 1; the tool
reported WinSta0/Default. No game was running at that check. This narrows the
investigation but cannot reconstruct the game windows from the ended trial.

`r5_window_report.ps1` now enumerates only the PID/birth-verified controlled
pair's windows and reports visibility, minimized state, DWM cloaking, owner,
extended style, monitor intersection, bounds and desktop name. It performs
no game-memory access and never activates, moves, renames or closes another
app's window. Its optional 45-second visible fixture creates only two owned
forms, with explicit presence required and no game or Steam launch. The
owned hidden-pair fixture passes. User confirmation of a visible pair is
still pending; no further game launch has run.

Window style visibility does not establish that a person can see a window;
an obscured window can retain that style. DWM cloaking is a separate state.
The report's candidate predicate is a diagnostic heuristic, not an Alt+Tab
or gameplay guarantee. See Microsoft's [IsWindowVisible contract](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-iswindowvisible)
and [DWM window attributes](https://learn.microsoft.com/en-us/windows/win32/api/dwmapi/ne-dwmapi-dwmwindowattribute).

The capture tool previously returned success even with zero frames. This
was reproduced using an owned empty AVI and fixed: a capture now fails if
empty or missing either controlled foreground PID, preserving its metadata.
Owned two-frame, zero-frame, missing-window and duplicate-PID cases pass.
No existing app UI is controlled by these checks.

The new full `scripts/build.ps1` run passed **118/118 tests, 106.05 seconds**;
the isolated eight-second smoke recorded 78 remote states, zero rejects and
clean unload. `port-evidence/r5/desktop-preflight-validation.json` seals this
new binary/source. It has not been retried in the game. Native guard firing
is still unproved. A game-free visibility sitting was requested; an actual
two-window confirmation and useful foreground capture are required before
any later L3 acceptance.
