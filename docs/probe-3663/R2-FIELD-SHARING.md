# R2 Step 3 — native field-sharing contracts

Static investigation is in progress. Candidate function RVAs below still await
the Step 2 standard-correlator check; none authorizes a hook. Tier C is closed.
The native layout proofs are catalogued separately in `r2-reviewed.json` and
compiled as unused constants in `build_3663.h`.

Continuation decision: the offline ABI/ownership assessment is complete to
the boundary below. The field-sharing port remains blocked; no legacy detour
is redirected to a candidate address. Step 2 matching continues independently.
See R2-FIELD-TRACE.md for the smallest next observational experiment and its
unmet implementation/preflight requirements. That document is a trace design,
not a claim that a collector or live session has passed.

The named 3663 LoginCharacterToFieldServerReq decoder at 0x2C6AE10 calls
0x2F4AE80. This performs the native save/load/login-manager sequence, including
0x1A16C40 and the login-manager lookup 0x2D79620 (Denuvo body 0xEAC9720).
Acquisition is later in the field transition, not a direct call in this decoder.
Following the shared FieldMover caller gives this candidate correspondence:

| Native role | 2976 RVA | 3663 candidate RVA | Independent code evidence |
|---|---|---|---|
| FieldMover transition caller | 0x2BBFA80 | 0x2ECA480 | Seven uniquely mapped direct call neighbours; same acquisition/entry/result cleanup sequence |
| AcquireField | 0x2B07790 | 0x2E08520 | Destination-key hash lookup, pool acquisition, open/owner assignment, outgoing reference replacement |
| EnterField | 0x2BC04F0 | 0x2ECB130 | Uses the acquired field and travelling character list, native actor insertion, failure rollback |
| AddActor | 0x2AFF1E0 | 0x2DFD3A0 | Thunks to 0x1064C770 / 0xEB68740; both locks, owner-user lookup, admission counts and rollback |
| Assign field to mover | 0x2BBE900 | 0x2EC9200 | Thunks to 0x106D9F50 / 0xEBF3A10; busy/existing-field rejection, copies key and field pointer under actor lock |
| Leave field | 0x2BBEAB0 | 0x2EC93B0 | Releases current field, clears pointer and resets mover key to 0x30000 |
| TryAddRef | 0x14343A0 | 0x1658E90 | Tests open state and increments strong reference under lock |
| 16-byte user-vector push | 0x40B5B0 | 0x4CAE10 | Native AddActor callee; stride 16, count +8, capacity +C, bounded growth and allocator choice |

The field contracts are not an unchanged object layout:

| Member | 2976 | 3663 |
|---|---|---|
| Strong references / first lock / second-lock pointer | +10 / +18 / +8 | +10 / +18 / +8 |
| Open flag / owner user key | +4A / +180 | +4A / +180 |
| User vector / count / total user actors | +2A0 / +2A8 / +2B0 | +2A8 / +2B0 / +2B8 |
| Field key / level entry | +2C8 / +2D0 | +2D0 / +2D8 |
| TransformSync → FieldMover | +4F0 | +508 |
| Mover key / field pointer / u16 busy counter | +8 / +10 / +5A | +8 / +10 / +4A |

Both AddActor bodies acquire field+18 first and [field+8] second through lock
vslot +8, then release through +10. User entries are `{u64 user, u32 actors,
u32 padding}`, with a 16-byte stride. Admission finds the actor's owning user
at actor+A0. Failure removes the inserted actor count and, when unused, the user
entry. The target's obfuscated rollback decrement is independently one:
the file value at RVA 0x12569640 is 0x1F38148, XOR 0x1F38149.

AcquireField still looks up out+24 in manager +48/+4C/+58/+60. Its result owns a
strong field reference at out+8 with validity +10, optional holder +18 and
new-field flag +20. A shared existing field must retain that reference and set
new-field to zero; otherwise native failure cleanup can close the host's field.
No production path has been changed to call these candidate functions.

The entry ABI changed: 2976 takes `(mover, result, field, companion)`, whereas
3663 takes `(mover, result, field)` and traverses its character list. Installing
the legacy detour and changing only the RVA would be wrong. A target adapter
must prove ownership for the entire list, insert once under both locks, call
the three-argument original and undo only its own still-unused insertion on
failure. It must preserve the 2976 four-argument implementation.

The new roster is concrete: target EnterField walks the mover vector at +68,
u32 count +70, with a 0x3C-byte stride. Each record contains a kind at +0 and
an actor key at +4. It resolves existing actors through 2D78E40, or invokes
factory 2EA4860, then admits each resolved actor and assigns that actor's own
mover. The factory obtains its primary actor from component+8 and passes it
as the owner argument to 2E98B30. This does not yet prove all existing roster
actors belong to the travelling user, or every factory branch's owner and
rollback. The reference main actor was admitted explicitly before handling
its companion; the target loop is a substantial behavioral change. A bounded
single-main-character gate is a possible smaller adapter, but it still needs
native lookup/lifetime and primary-roster proof before implementation/arming.
These probe-only roster details have not been promoted into production or
used to declare Tier C complete.

Remaining Step 3 facts: standard-correlator support for the candidate entries;
complete list ownership/rollback conditions for the new adapter;
verified target entry signatures and all host/guest identity prerequisites.
Native EnterField returns the caller's result pointer on success and propagated
error exits; both acquisition and entry therefore preserve pointer-to-result
semantics. The three-argument entry ABI itself is established by the native
register uses and caller. It does not establish safe roster-wide ownership.
The smallest passive experiment, if static list ownership remains ambiguous,
is a guarded entry/exit trace at the verified AcquireField and EnterField
entries during one ordinary local login: record arguments, list owners, result,
and native call order, without calling or changing anything. This trace is
not prepared or authorized to launch until those entry RVAs are adopted and
the normal presence, backup, deployment and stop conditions are recorded.

Evidence: `port-evidence/r2/field-*.txt`; canonical and original local hashes
are in `port-evidence/r2/text-hashes.json`. The `.pdata` candidate scanner is
not counted as either standard whole-program correlator.

Factory follow-up: the full 2E98B30 body has distinct initialization paths,
including owner-key propagation to different intermediate fields and a
post-registration failure loop that requests native retirement for each
created actor. Lookup 2D78E40 returns a reference/validity wrapper after its
locked map lookup and vslot+B8 acquisition. It is therefore not a raw pointer
lookup that a passive collector may call. The factory owner argument alone
still does not prove every final actor+A0 owner or rollback lifetime. The
full export is field-roster-factory-followup.txt; no adapter is installed.

Completed standard-result update: AcquireField 2E08520 and TryAddRef 1658E90
are adopted. EnterField 2ECB130 still lacks standard support and its changed
roster ownership remains unresolved. Full default BinDiff maps legacy VecPush
40B5B0 to 14AD670, conflicting with accepted VT 4CAE10 and the native AddActor
callee check. C047 is withdrawn to blocked while this conflict is investigated;
the conservative profile had no competing pair. Neither result may silently
override the other. All Tier C permissions remain false.
