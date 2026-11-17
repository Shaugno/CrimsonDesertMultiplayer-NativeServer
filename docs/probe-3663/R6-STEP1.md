# R6 Step 1 — character-login refusal diagnostics

Expected reproduction: exact build 3663, explicit v1 host and isolated guest,
both real windows visible left/right before guest Play. R5-L3-3 incremented
the host's opcode-3081 identity refusal counter before any actor insertion,
but did not retain the failed predicate. This change reports that predicate;
it does not repair or admit the request.

`identity3663::admit` still makes its decision with the existing generation
equality and unchanged `identity::login_character` policy. No length, number,
slot, epoch, ownership, build, Steam or hook check was relaxed. Early inactive,
failed and unavailable-context refusals are also observable. A copied state
snapshot is taken under the existing leaf lock, then formatting/logging occurs
outside it. There are no new game-memory reads, offsets, hooks or native calls.

One line per host process/attempt starts `3663 identity refused 3081:`. It
contains actual length, at most the first 13 owned packet bytes, independently
available discriminator/number/slot fields, active/broken/context, bound,
lobby-ready, exact-size, number, slot and generation predicates. Bound is
expanded into state-health, nonzero/matching epoch, nonzero guest and matching
user predicates. State/current epochs and native user generation keys and
lobby/status/phase edit counts are included. Truncated fields say unavailable;
a malformed or extended prefix is observed without being accepted. A once
flag prevents a peer from flooding the asynchronous logger.

## Offline target and reference checks

The current target hash remains
`8BE67178D9B9A149C361A731051665E0BA4D441E250AE1DFE645BD6E524345C8`.
The opcode descriptor's named RTTI is
`TrocTrLoginCharacterToFieldServerReq`, vtable **5F676B8**, initializer
**2588D0**, instance **6F359A0**, opcode **3081**. Its current disk vtable
slot 2 points to **2C6AE10**. Read-only Ghidra extraction of that entry shows:

| Reader site | Bytes consumed | Payload meaning |
| --- | --- | --- |
| 2C6AED0..2C6AEDE | 1 | Discriminator at 0 |
| 2C6AF1F..2C6AF2D | 8 | Character number at 1 |
| 2C6AF51..2C6AF5F | 4 | Save slot at 9 |
| 2C6AF7B..2C6AF89 | 4 | Additional field at 13 |
| 2C6AF9D..2C6B08B | Remaining bytes / 2 | UTF-16 remainder at 17, consumed to the declared end |

The handler starts the reader at packet+5, checks the declared payload length,
and calls the independently known character-login body **2F4AE80** after
decoding. Its native contract therefore requires the additional u32 beyond
the first 13 bytes, with an even UTF-16 remainder. This is a strong candidate
for the refusal, not a record of the actual R5/R6 request's length or fields.
The exact-13-byte admission gate stays intact until the requested diagnostic
run. No candidate address from this analysis is installed as a new hook.

The second vtable slot examined, C99580, appends a pointer to a vector; it is
not a packet writer. Targeted immediate-based writer candidates were also
rejected: unrelated bodies, truncated extraction or timed-out decompilation
do not prove a serializer. The actual client writer is not yet independently
located. The diagnostic run must provide its emitted first 13 bytes and full
length. No source-build writer or unchecked address is substituted.

Existing 2976 research (`docs/research/opcion-d-login.md`, section 4.3) describes
the same field prefix, extra u32 and UTF-16 tail, including a 19-byte observed
request. Its `identity_fix_admit` checks a minimum nine-byte prefix and assigned
number; Samuel Cardillo's pinned 3576 reference commit a3904af uses the same
minimum-prefix check. Neither establishes that 3663 should accept exactly
19 bytes or that any particular slot/number is safe. No reference code was
copied in this logging change, and no broader legacy acceptance was adopted.

`lobby_ready` is set by the host-side guest-owned lobby normalization after
the original native load and owner/epoch/uniqueness/affinity checks. The R5
host log records lobby edits=1, so that path did succeed. The guest log cannot
report this host-only adapter's state; its title refresh count is a separate
observation. Whether the native guest later requested the assigned number
must come from the diagnostic line, not the earlier edit count.

## Validation and live boundary

Baseline `scripts/build.ps1`: **119/119, 112.08 seconds**. Post-change full
build/suite: **119/119, 111.19 seconds**. The extended identity test checks
formatted fields/counters, every truncated prefix, extended length, forbidden
slots, wrong number, binding/epoch/generation failures and once-only emission.
Its 640-case comparison confirms diagnostic predicates agree with the existing
admission decision. Eight-second synthetic smoke: **78 remote states, zero
rejects, tier metadata 0xF, clean unload**. These are game-free results.

R6 has a fresh separate local ledger, startup 5/join 3. A prejoin reservation
will transfer to join only once a real guest launch receipt exists; an app
denial before the guest exists stays startup. The R5 ledger remains untouched.
The validated R5 desktop harness and its version-2/round-5-compatible receipt
format are retained without weakening their checks; R6 evidence/budgets and
uniquely named arming receipts are separate. This is format compatibility,
not a reuse of the R5 budget. No R6 game has launched at this checkpoint.

Next: one user-present diagnostic sitting, fresh external backups, desktop
runner, actual left/right confirmation before guest selection, fixed common
watchdog and normal host quit before deadline. If admission succeeds instead,
continue acceptance in the same sitting with the user's own recording; record
its filename and duration. Otherwise close both copies, retain the single
refusal line, and fix only the diagnosed cause with a regression test.
