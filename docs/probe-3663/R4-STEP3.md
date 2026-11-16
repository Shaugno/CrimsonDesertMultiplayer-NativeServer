# R4 Step 3 — native backend investigation and implementation record

Reproduction: the R3 three-argument EnterField policy has no native reference,
insertion or rollback backend. The target guest installer is still closed;
reusing the legacy four-argument detour or its identity patch bytes is invalid.
Expected behavior is a separately guarded target backend that retains native
objects, acquires field locks in native order, inserts one exact unused guest
row, calls three-argument EnterField with locks released, and rolls back only
its own unchanged unused insertion on failure. Uncertainty quarantines the
guest path and preserves the host field.

The next passive sitting asks: do the independently identified target
vector, identity/lobby/login, key-class and session-queue contexts agree with
the measured native object identities during an ordinary load and fast travel?
It is logging only: no register/context changes, game-data writes, native
function calls, packet replay or networking. C/D remain off.

New file evidence:
- C144 is a **u32 owning-user key at actor+90**, distinct from the pointer at
  +A0. Named MoveActorReq handlers 2977E10/2C52CD0 compare that key with the
  session user's vf170 result. Existing C007 recipient-finder evidence already
  proves the same key member. The earlier R3 owner-pointer observation alone
  did not settle this separate contract.
- The standard-matched login-manager context is 2A12A30; its actual read site
  2A12A58 loads 72456E0 then dereferences the holder. Standard-matched
  character-login context 2F4AE80 contains clear-lobby at 2F4AF52 and
  clear-list at 2F4AF79. Instruction-size changes prevent using simple offsets
  from the reference body.
- Native SAM getter 2775800 maps strongly to 2A10830 and returns holder
  address 7283378. Its actual static teardown is 55288C0. The weak default
  teardown match 54EEA70 clears an unrelated holder 7245578 and is rejected.
- Exact named IActorComponent slot 10 and many derived components map the
  shared load stub 35EA90 to 3ABEE0. The passive component-dispatch return
  point is 2295D80 inside the adopted loader.
- The native character-key producer is in ServerUserActor's 2D740D0 path:
  definition type byte is shifted by 28 and installed into key class bits
  before its virtual initialization. The trace will correlate that input,
  resulting key and actual character/owner.
- The actual AddActor vector caller is EB6883B (return EB68840) inside
  EB68740. The helper 4CAE10 uses 16-byte entries, native allocation/free
  selected by TLS+1FE, the measured growth rule and count increment. The
  conflicting default generic-helper match remains recorded until the
  permitted callsite trace settles its use.

No candidate is armed by this document. A query at 23E0FF0 produced a junk
fragment and is rejected; the actual standard-matched movement predicate is
23E1090. The probe uses verified instruction boundaries in complete contexts.
Raw exports remain local; final summaries and hashes will identify evidence.

Collector checkpoint: 108/108 tests passed in 90.41 seconds after adding
passive native reference-wrapper observations. The prepared core session is
R2-P1-20261116-161703, with 23 save files verified outside OneDrive. Presence
is confirmed; passive deployment is complete, awaiting launch/title. This
checkpoint is not a completed Step 3 or native join acceptance.
