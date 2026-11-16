# R2 lifecycle and save-manager audit — Tier D closed

Native save-manager slot entries changed **0x68 → 0x78 bytes**. Target refresh
0x1A18320 (body 0xBB0CBC0) rebuilds the vector at manager+40. Its clear callee
0xBB1A850 multiplies the index by 0x78 before destroying each entry, and sort
0xBB1DBE0 independently copies/swaps entries using the same stride. The legacy
title-slot reader must not be routed to this table unchanged.

Other confirmed member roles: lookup+48 is the server save manager, its first
word is the lock pointer, busy is +60, storage is +50, slot vector/count are
+40/+48, and storage folder count is +40. Native LoadSlot and the named refresh
handler corroborate lock/busy/storage ownership. Paired title-vector population
and fallback consumers now also confirm capacity +4C, slot number +0 and the
metadata qword +8; the qword's units are not inferred. The lock's self-offset
+8 remains unproved. The later native title eligibility consumer 758EA0
independently confirms WorldSystem-to-save-manager +88 and the same vector.

Paired LoadSlot 186F570/1A16C40 explicitly accepts 0..3, 100..108 and 200..207.
Native refresh independently checks that domain before its special disk
exclusions. The later paired storage review independently proves the
memory-only set below. No native function was called or native slot written.

The target save wrappers confirm descriptor pointer actor+88 (byte+1), owner
user actor+A0, user main character +D0, and actor key +60. These member facts
do not prove reconnect/removal ownership, cleanup barriers or guest-save
interception order. Existing save hooks remain unreachable on 3663.

The paired LoadSlot branches provide candidates for its busy and invalid-slot
error instructions/globals. Those RVAs require standard-correlator support;
their runtime numeric error values are never presumed unchanged. The normal
login decoder and save/ack layouts are documented separately in R2-WIRE.md.

The storage review identifies candidate SavePlayerToSlot `186E450→1A15930`,
GetOrCreateSlot `2355EC0→25BB470`, AddFile `235B100→25C0900`, and component
loader `20688A0→22951B0`. These require standard support; the native checks
and error-site operand anchors are catalogued separately. The target AddFile
lead came from its 140-byte allocation instruction and was verified through
the full body, filename/header construction, name-map insertion and vector
append. A raw immediate match is not adoption evidence.

Native GetOrCreate applies the `slot` numeric suffix rule for **201,205..207**,
setting +699. Target CommitSlot `25BAC30` independently skips disk persistence
for +699 or the constructor's in-memory argument at +69C. The obfuscated
target subtraction is checked against file bytes: `F9D00CD-[B7A5B48]=CD`.
The reserved mod slot204 choice remains policy.

Paired AddFile allocates 140 bytes and constructs an 80-byte file header at
+98, using the file-backed SAVE magic. It inserts into slot+8's counted map
and appends the named entry at vector+688/count+690. Paired map consumers
prove bucket-count+0, bucket-base+10, entry-pointer-array+18, 100-byte buckets,
pairs+8, maximum 31 pairs, entry key+8/value+10, and key chars+0/length+8.

The actual native file loaders `2357DA0→25BD480` confirm every inline header
check: magic+0, u16 version+4 in 1..2, u16 size+6 equal 80, flags+C masked by
FFFFFFFD equal zero, u32 raw+12 and data+16 nonzero, and data<=raw. The target
then copies all 80 bytes to file+98. This was added as stable core row D126;
the original cut omitted that inline schema. Existing row IDs were preserved;
the inventory was then 388. The later title-parser cut correction brings it
to 394. Local file/raw size and ratio caps remain policy.

Native FlushPendingSaveData candidate 1871930→1A192D0 transfers manager+5C's
pending slot to the output, clears it to FFFFFFFF, checks busy/lock/generation,
and commits existing storage. Both native call paths have been reviewed:
logout 28F26C0→2BC29F0 (call 28F273A→2BC2A6A; return +5), and timer
10839370→ED09970 (call 108393C8→ED099C8; return +5). These are independently
checked candidates, not adopted RVAs. The logout caller also identifies the
login/save-context global 6D696A8→72456E0 and its +10 save-manager pointer.

The actual remote title parser uses its own stride constant, so that use is
now included separately instead of treating the similarly named header alias
as sufficient. Native population 1871D60→1A19720 and fallback
1872620→1A1A080 use stride 68→78. Copy helpers 18735F0→1A1B1A0 copy the
slot u32 at +0 and metadata qword at +8. Target title consumer 758EA0 confirms
the 0..2/100..108 domain, but adds an important gate: each entry's u32 list
at +68/count+70 must be covered by a global set at 71AEAA0/71AEAA8 before
that entry makes the predicate true. Its meaning and producers are not yet
proved; it is not labelled as DLC or another feature by assumption. This
extra vector accounts for native state the legacy domain-only parser does
not consume. D130 remains blocked on the complete target eligibility rule,
not just the already observed number range. The producer review now identifies
these as .admeta metadata IDs: paired file-pattern producers 128D360/14AA720
enumerate `*.admeta`; target additionally reads a u32 after each metadata name
and builds the ID vector at 71AEAA0/count71AEAA8. Reference only builds names.
The target lobby save path 2E88000 copies that ID set into saved metadata;
title population copies its list into the new entry vector. These are new
dependencies without an established old ID-vector counterpart, still requiring
an adoption/guard strategy and a complete target parser. The operands and
pattern bytes are reproducible with r2_admeta_constants.py. This consumer is not asserted
to be the exact counterpart of the old inverted predicate 65EAD0. Manager
LoadSlot's broader domain is not substituted for the title rule.

Remaining work includes standard support for native flush and all classified return/call sites and
entry signatures, the rest of the save manager's layout, native removal and
rejoin barriers, and early guest-role save redirection. Standard matching is
still in progress. Tier D is not armed; no guest or two-instance test has run.

Evidence: port-evidence/r2/save-manager-reference.txt and save-manager-target.txt;
the retained R1 target refresh/handler exports; r2-reviewed.json and
r2-rva-checks.json. No save files or private backup manifest are committed.

## Loading-message and duplicate-consumer continuation

Paired named descriptors and verified native initializer/handler bodies confirm:
GameLoadingStartReq 2437 -> 2350, MoveFieldCompleteReq 2549 -> 2723,
LoadingCompleteReq 2445 -> 2789, MoveFieldCompleteAck 2144 -> 2268, and
LoadingCompleteAck 2762 -> 2891. Existing verified descriptors also cover the
lifecycle aliases LoginCharacterReq 3093 -> 3081 and MoveFieldAck 2174 -> 2364.
The target request handlers consume an ordinary envelope and exact payload
exhaustion; loading-start requires an empty body, the two completion requests
decode one byte. This adopts identifiers only, not a reordered state machine.

Following the named requests identifies native server transform-sync transition
functions: loading-start 2BC9200 -> 2ED5980, move-complete 2BC6BB0 -> 2ED29D0,
and loading-complete 2BCAF00 -> 2ED7960. They obtain the actor at component+8;
the handlers reach that component through actor+68/component+1A0. These are
server transitions, not a proved read-only client loading predicate. Their
RVAs remain investigation candidates without standard matching support.

The existing FindActor pair is re-exported in full. Its low-20-bit key mask
is a native scalar, not an RVA; the inventory classification is corrected
without changing row IDs or values. The shift-by-eight bucket addressing
proves the 256-byte stride. It does not establish the maximum bucket count
or cleanup lifetime. The character-number lookup proves the repeated
manager+B8/count+C0 actor-vector uses. Repeated server user/actor-key and
owning-user fields now link to their existing native consumers; this does
not infer the different client owner+90 field. No tier is armed.

The named ServerUserActor vslot-2 cleanup walker confirms user+68/component+18,
slot array+18/count+20 and 16-byte records. It clears each child pointer and
validity byte before native actor release, then releases the user. Paired
release 2A83650/2D79D00 swap-removes from manager+B8/count+C0, clears actor+60
and increments recycled-key count+D0. Those layout facts are proved; complete
retirement order, concurrent snapshots and reconnect safety remain unverified.

Target save-manager construction 1A15340 independently proves the first word
is this+8, where it initializes the named WindowsLock<3> object. Its slot
vector, capacity, busy/pending fields and storage construction identify the
manager role. Exact RTTI tables map lock vslots 1/2 to native dispatch thunks
1593240/E297F0. Those function/vtable RVAs still require standard support;
the lock self-offset is no longer unproved.

Final constructor check: the source save-manager constructor is CBD2920
in .xtls, independently matched by default BinDiff to target 1A15340. Its
LEA/store at CBD2956 assigns WindowsLock<3> 557C120 to this+8, corresponding
to target 1A1537C and 59874C8. The shared slot-vector/pending/busy initialization
and named RTTI corroborate the relationship. D013 is adopted through this
constructor operand; the short lock-dispatch function RVAs still lack standard
support. The native user-session getter is independently proved at user+1F8
through named slot +E8 and actual recipient-list consumers.
