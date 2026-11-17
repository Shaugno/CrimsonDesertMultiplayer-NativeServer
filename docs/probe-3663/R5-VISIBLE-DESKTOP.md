# R5 visible desktop launch correction (2026-11-17)

The first visible-pair test was launched by the command tool. Its own
Win32 report said both fixture forms were visible and uncloaked, but the
user saw neither and the desktop window inventory did not expose them.
This reproduces the observation/launch-context mismatch without a game.
Do not treat that command context's visible flags as proof of user visibility.

The same owned two-form fixture was then launched with the desktop app
launcher. Both returned windows were present (HOST 333328, GUEST 202306),
and the user confirmed seeing both. The local records are
`build/r5/visible-pair-20261117.json` and
`build/r5/desktop-runner/visible-pair-start.json` / `visible-pair-end.json`.
Neither fixture launched Steam or the game. No R5 game budget was consumed.

`R5DesktopRunner.cs` supplies a narrowly scoped GUI-subsystem starter for
the visible desktop. It does not execute a shell/terminal or accept an
arbitrary executable/command. `launch-2inst-3663.ps1 -DesktopLaunchPlan`
performs the usual preparation/arming/deployment checks and exports only
explicit Steam/role/save-isolation values, never the inherited environment.
The plan expires after two minutes. The desktop app revalidates source,
runner/ASI/executable/deployment hashes, real-user save-root identity,
frozen manifests, roles, inventory, host birth/readiness and common deadline
before it starts the game directly. Host real saves remain unsandboxed;
guest profile isolation remains mandatory and is checked again in the mod.

The controller holds a query/synchronize/terminate handle pinned by exact
image path and birth time; it has no VM access. It writes the same role
launch/watchdog receipts and stops only its owned process at the deadline.
Cleanup now also requires ended controller receipts instead of relying only
on another context's process inventory. The desktop controller writes
read-only window metadata from its own context. The existing capture and
window-report helper C# sources are compiled into it; the capture mode
records only foreground, identity-verified test games and rejects empty or
missing-window clips. These modes do not enter gameplay input or move game
windows. Requested window arrangement uses the computer-use API.

Owned normal/deadline/birth-mismatch/path-escape/frozen-save-mutation tests
pass. Negative plans prove missing presence is refused before executable
access, and arbitrary command lines are rejected. Full build/suite:
**119/119, 115.14 seconds**. Unique eight-second synthetic smoke: 78 remote
states, zero rejects and clean unload. The new seal is
`port-evidence/r5/visible-desktop-v2-validation.json` and includes the desktop
runner hash. No native game-memory/protocol behavior was changed here.

The user has authorized the L3 retry with both actual game windows visibly
arranged side by side before guest character selection. Both usable windows,
native readiness and a useful two-window recording must be established;
fixture success alone does not pass L3. Remaining game budgets before that
retry are startup 2/5 and join 2/3. L4 still requires an actual L3 pass.
