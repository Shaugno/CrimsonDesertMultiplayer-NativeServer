# 3663 join adapter: established ABI, pure policy, native backend blocked

The 3663 entry is **EnterField(mover, result, field)** at `2ECB130`.
The old four-argument detour must never be redirected here. R3 adopted the
entry after its unique native caller/body and two complete passive traces.
Both calls returned the supplied result pointer with code zero. The roster
had one kind-0 entry matching primary key A0100001 and owner 90100000; native
admission changed each distinct field from zero to one user/actor. These are
ordinary local admissions, not cross-user sharing or rollback evidence.

The roster is mover+68/count+70, stride 3C, with u16 kind+0 and u32 key+4.
The mover's holder supplies its primary actor at holder+8. Native ServerActor
owner at +A0 agreed with the field owner at +180 in both observed admissions.
This does **not** resolve C144: that separate legacy `Actor_OwnerUser+90`
contract must not be renamed to the server member. Likewise, roster kind 0
does not establish the separate C066 character-class constant.

The new pure policy is `src/coop3663/join3663_policy.h`, tested independently
and not linked to the ASI. Its function type accepts exactly three arguments.
It requires coherent caller-owned lifetime tokens, retained primary/user
references, an open field, an idle empty mover, a distinct guest owner, and
the measured single-primary roster. Other roster shapes fail closed. These
checks identify a candidate transaction; they grant no native permission.

The native transaction, once its outstanding contracts are proved, must:

1. Retain the host field with the adopted TryAddRef/open-state protocol. Keep
   the primary actor and owning user alive through native reference wrappers.
   A raw lookup pointer is insufficient. AcquireField's output owns a field
   at +8 with validity +10, optional holder+18 and new-field flag+20; replacing
   it requires releasing its old ownership correctly and marking the reused
   host field **not new**, so failure cleanup cannot close the host's field.
2. Revalidate under the two field locks in native order: field+18, then the
   lock at [field+8]. Add one guest user entry only if absent, with zero actor
   count, and retain an exact insertion ticket. The native row is 16 bytes.
   Its storage growth must use a proved native helper/ownership path.
3. Release those locks before the original three-argument EnterField, keeping
   all references. Native AddActor performs the actor/count admission; the
   adapter must not duplicate that increment. Preserve the original result
   pointer and error code.
4. On failure, reacquire both locks and undo only its own still-unused entry.
   A reused user key/address is insufficient. The ticket must remain valid
   across native mutations and the same epoch/field lifetime; no native
   serial member is presumed. A positive actor count, changed identity or
   uncertain insertion requires quarantine, never removal of another user,
   the host's field or a native actor. Release retained references through
   their actual native wrappers on every path.

The pure rollback policy tests no-insertion, exact unused insertion, positive
actor count and changed serial/generation/epoch. The native reference,
mutation-observation, insertion and rollback backend is **not implemented**.
Compilation and policy tests do not establish those native contracts.

The immediate blocker is C047: accepted VT/native AddActor points to 4CAE10,
while default whole-image BinDiff maps legacy 40B5B0 to 14AD670 (confidence
0.952574). R3's named-RTTI exception does not resolve this generic helper.
The current trace did not instrument that helper or exercise a failure,
companion or factory branch. Additional missing rows include SAM/actor
identity, login-manager and clear-list sites, session key semantics, and the
travel clock publication now inlined into 2ECA480. Their exact list remains
in the R3 blocker ledger. C stays false; no native join hook is installed.

Nine further address/guard facts were adopted separately: lobby/status named
primary vtables and slot 10, EnterField, and phase entry 2C17410 with its new
entry bytes. The latter has default BinDiff confidence 0.985887 and matching
component initialization/lifecycle behavior; the earlier 2C171E0 signature
lead was a fragment of a different function. These adoptions do not complete
the native adapter. See identity-adoptions.json and join-result.json.
