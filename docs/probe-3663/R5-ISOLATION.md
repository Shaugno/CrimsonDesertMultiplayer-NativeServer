# R5 Step 1 — host real saves and verified guest profile

Reproduction: R4 installed a file sandbox in the host to make a combined
manifest immutable. Its three host startups stopped before title/guest join.
R5 explicitly removes host isolation, permits ordinary host autosave, and
requires guest-only isolation and save verification instead.

The exact 3663 native resolver at 157BB40 calls SHGetFolderPathW at 157BBD3
with CSIDL_LOCAL_APPDATA (1C), then appends `/Pearl Abyss`. The verified
directory formatter 157C240 consumes that root and `%#/%#/save/%#`. Raw
import leads were confirmed in native disassembly, not adopted as hook
addresses. A separate SHGetKnownFolderPath caller uses the Screenshots GUID;
it is not substituted for the save resolver.

Fresh owned-process tests show LOCALAPPDATA alone does not redirect this
resolver. USERPROFILE does, provided its AppData/Local directory exists;
setting both variables yields that same isolated path. The initial probe
with a nonexistent profile failed; it was corrected to test an existing
profile tree. No registry or account profile setting is changed.

The launcher creates a guest profile outside OneDrive and copies a fresh,
verified backup to its AppData/Local/Pearl Abyss/CD/save. It sets USERPROFILE
and LOCALAPPDATA only in the guest's environment before process creation.
Startup independently resolves CSIDL_LOCAL_APPDATA, compares the resolved
save directory's canonical name and file identity to the expected copy,
checks disjoint plain trees and the frozen manifest, then exercises actual
create/write/read/delete in that resolved directory. A mismatch refuses
guest startup. **The profile route installs zero file hooks.**

The host never calls the isolation installer. Its launcher clears inherited
guest isolation variables and explicitly marks real-save operation. Both
roles still have fresh verified backups before every session. The controlled
marker refuses untagged relaunches or conflicting role/isolation settings.
Only exact-build compiled permissions and the explicit v1 configuration
grant the route; profile variables alone grant no gameplay permission.

The guest-only S2 route is explicit `guest_startup_only=1`, with networking
and session installers off. It tests title startup/isolation through a local
native conversation without pretending an absent remote host can handshake.
It cannot enable multiplayer. Normal L3 guest startup retains the restricted
native bridge and all its guards.

The corrected R4 NT layer is retained as an explicit fallback. Its fixture
still denies/redirects hard-coded real paths and preexisting source handles,
tests mappings/pipes/teardown/small stacks, and proves zero original bytes
changed. A second fixture permits legitimate concurrent host source changes
while preserving the guest's frozen backup/copy and its zero source writes.

The primary profile fixture exercises the game's actual resolver, mutations,
IPC and a 128 KB thread with file_hooks=0; the original/backup SHA256
manifests stay unchanged. A wrong profile is refused. SteamAPI initialization
succeeds in baseline, NT and profile fixtures. The profile method confines
the game's save-path selection; it is not an OS permission boundary against
arbitrary absolute-path writes. The two fixture claims are kept distinct.

Launcher verification records host deltas rather than treating normal
autosave as a failure. Guest-only sessions require strict real-save equality.
Concurrent L3 deltas alone do not identify their writer; separate guest-only
manifest evidence is required. Host saves must actually load after live
sessions; hashes/compilation cannot establish that acceptance.

Startup watchdogs are at most ten minutes; L3 uses a common fourteen-minute
deadline. Fresh R5 budgets are separate: five startup and three join
experiments. `r5_attempts.py` persists the ledger; a prejoin experiment reserves
a startup slot and transfers to the join budget only when joining is begun.
Earlier rounds are preserved and their counts do not carry into R5.

Validation and immutable hashes are in
`port-evidence/r5/step1-validation.json`. No S1/S2/L3/L4 result is claimed by
this offline implementation. Samuel Cardillo's PR #1 is pinned separately
as reference; no branch merge or upstream code adoption is part of Step 1.
