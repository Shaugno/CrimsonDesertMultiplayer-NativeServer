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
