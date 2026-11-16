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
+8 and title's WorldSystem-to-save-manager hop remain unproved.

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
slot u32 at +0 and metadata qword at +8. Target eligibility for the title-only
0..2 / 100..108 domain remains unproved (D130). Manager LoadSlot's broader
domain above is not substituted for this UI predicate.

Remaining work includes standard support for native flush and all classified return/call sites and
entry signatures, the rest of the save manager's layout, native removal and
rejoin barriers, and early guest-role save redirection. Standard matching is
still in progress. Tier D is not armed; no guest or two-instance test has run.

Evidence: port-evidence/r2/save-manager-reference.txt and save-manager-target.txt;
the retained R1 target refresh/handler exports; r2-reviewed.json and
r2-rva-checks.json. No save files or private backup manifest are committed.
