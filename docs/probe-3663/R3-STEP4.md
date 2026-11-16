# R3 Step 4 — save protection assessment, D remains closed

Two-instance and second-PC tests have not run. C is incomplete and D still
has unresolved early guest-save interception, title/save sites, slot refresh,
Windows-lock identity, moving-state and reporter contracts. The target guest
process remains denied before the legacy save redirect. No guest deployment
or second-PC request is justified yet.

Independent progress: `src/coop3663/title3663_eligibility.h` implements the
complete observed predicate of native target consumer 758EA0 on owned data.
An entry must be in 0..2 or 100..108, and **every** required metadata ID must
occur in the available `.admeta` ID set. Empty requirements pass; unsupported
slots do not. IDs are opaque u32 values; no DLC meaning is inferred. This is
the target consumer's positive predicate, not an asserted counterpart of the
old inverted title callback or the broader LoadSlot domain.

The game-free tests cover missing IDs, unordered/duplicate IDs, empty sets,
supported-domain edges, rejected slots including 3/204, incoherent input and
bounded maximum input. Explicit diagnostic caps are 128 entries and 4096 IDs
per view; these are policy, not claimed native allocation limits. Sorting a
bounded owned copy avoids a quadratic membership scan. Borrowed input views
must already have valid lifetimes; this helper never reads game memory.

D130 remains blocked. A production parser must prove coherent ownership of
the native slot+68/count+70 ID vectors and the available vector at
71AEAA0/count71AEAA8, including the 14AA720 producer and saved-entry copy/clear
lifecycle. The native 0x78 stride alone is insufficient. There is no native
parser/guest-save backend wired to this pure helper and no D permission.

The remaining D rows retain their R2 missing facts in the R3 blocker ledger.
Before L3: complete C and all early guest save protection; verify separated
guest saves and backups; then test native creation/removal/reconnect and
unchanged host saves. L4 additionally requires the second legitimate PC and
Steam friends transport. A loopback cryptogram/state test is not that proof.
