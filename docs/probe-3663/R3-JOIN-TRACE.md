# R3 passive field trace

Result: passive subset passed. R3-JOIN-20261116-123800 captured 16 valid
records, eight pairs, two distinct fields through load and fast travel. No
collection stop, native call, data/context change or networking. User confirmed
completion; normal exit, all 23 saves unchanged, installation restored. See
port-evidence/r3/join-result.json. Later static work adopted status slot 10 and
the correct phase function; neither was instrumented in this session.

Expected behavior and reproduction: the legacy four-argument EnterField hook
cannot accept the target three-argument entry or its roster. During a normal
local load and fast travel, record the original AcquireField, EnterField,
travel transition and lobby calls without changing them. This is observation
only; no reference/lock/lookup/factory call, argument change or game data write.

The collector uses four unique paired native body/caller candidates under R3
rule 3. Eight exact-byte mid hooks cover .pdata entries and decoded shared
epilogues. Frame pairing uses native thread, role and reconstructed entry RSP,
including recursion and exception-unwind tests. Event storage, depth and roster
reads are bounded. An unreadable/changing chain, count over 64, depth overflow
or dropped record stops collection; native execution always continues. Hook
objects remain process-owned and are never unloaded while the game runs.

Records contain raw local arguments, result pointer/code, field owner/counts,
primary actor/key/owner and roster kinds/keys before/after EnterField. They do
not resolve all roster actors: that requires a proven read-only map walk or
observing an eligible admission site. Jump thunks StatusLoad, LoginMgr and
AddActor are not .pdata entries and are omitted. The 2C171E0 signature lead is
inside 2C16F10, a different two-argument body; it is not adopted as legacy
Phases. These omissions may leave C blocked after an ordinary local trace.

Before live: 104/104 tests (85.69 seconds) and clean game-free host/peer smoke.
Native and synthetic evidence remain separate. C/D stay false. See
join-preflight.json and join-validation.json for guards, omissions and hashes.
The session controller uses fresh verified backups outside OneDrive, explicit
presence, no UI control, the fourteen-minute watchdog and deployment removal.
