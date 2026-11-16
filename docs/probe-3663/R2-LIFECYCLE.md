# R2 lifecycle and save-manager audit — Tier D closed

Native save-manager slot entries changed **0x68 → 0x78 bytes**. Target refresh
0x1A18320 (body 0xBB0CBC0) rebuilds the vector at manager+40. Its clear callee
0xBB1A850 multiplies the index by 0x78 before destroying each entry, and sort
0xBB1DBE0 independently copies/swaps entries using the same stride. The legacy
title-slot reader must not be routed to this table unchanged.

Other confirmed member roles: lookup+48 is the server save manager, its first
word is the lock pointer, busy is +60, storage is +50, slot vector/count are
+40/+48, and storage folder count is +40. Native LoadSlot and the named refresh
handler corroborate lock/busy/storage ownership. The lock's self-offset +8,
vector capacity +4C, title's WorldSystem-to-save-manager hop, and the complete
table entry layout beyond its stride remain unproved.

Paired LoadSlot 186F570/1A16C40 explicitly accepts 0..3, 100..108 and 200..207.
Native refresh independently checks that domain before its special disk
exclusions. This does not prove the memory-only slot set or authorize choosing
or writing any native slot. No native function was called.

The target save wrappers confirm descriptor pointer actor+88 (byte+1), owner
user actor+A0, user main character +D0, and actor key +60. These member facts
do not prove reconnect/removal ownership, cleanup barriers or guest-save
interception order. Existing save hooks remain unreachable on 3663.

The paired LoadSlot branches provide candidates for its busy and invalid-slot
error instructions/globals. Those RVAs require standard-correlator support;
their runtime numeric error values are never presumed unchanged. The normal
login decoder and save/ack layouts are documented separately in R2-WIRE.md.

Remaining work includes native save/flush/get-or-create/file-object contracts,
every classified return/call site, file-header consumption, slot-name maps,
memory-only behavior, native removal and rejoin barriers, and early guest-role
save redirection. Standard matching is still in progress. Tier D is not armed,
and no guest or two-instance test has run.

Evidence: port-evidence/r2/save-manager-reference.txt and save-manager-target.txt;
the retained R1 target refresh/handler exports; r2-reviewed.json and
r2-rva-checks.json. No save files or private backup manifest are committed.
