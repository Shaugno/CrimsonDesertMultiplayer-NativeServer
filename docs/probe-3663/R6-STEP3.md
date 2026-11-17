# R6 Step 3 — fix only the measured layout and identity mismatches

The R6 diagnostic showed length **17**, native number
`0x7FFFFFFFFFFFFFFF`, ordinary slot **1**, and successful lobby/binding/
epoch/generation checks. The code now addresses those two failures only.
No slot, save profile, hook site, native function address, lifecycle guard,
Steam restriction or field-sharing transaction was changed.

## Observed native layout

`identity::login_character` now requires **exactly 17 bytes**: the independently
checked native 3663 reader consumes u8 discriminator, u64 number, u32 slot,
and another u32. The final scalar is carried unchanged; its runtime value was
not captured and is not assigned a new meaning. The native reader can consume
a UTF-16 remainder, but this minimal route adopts only the observed empty-tail
form. Old 13-byte, truncated, odd or extended forms remain refused. No legacy
19-byte example is substituted for this run's actual request.

## Conversation identity translation

The existing non-core branch in `server.cpp` already maps a real client's
single-player Play sentinel into the second conversation's assigned identity.
The separate 3663 core branch returns before that old code, so it omitted the
translation. The R6 live log independently confirms the same native sentinel
on the exact target, together with a ready assigned guest lobby. The older
title analysis (`docs/research/pendientes/p8-titulo.md`) explains why native
Play uses the sentinel independently of its received lobby number.

The 3663 core branch now makes an **owned packet copy** for opcode 3081 only.
A small target adapter translates just its u64 number when all hold:

- identity adapter active and healthy, verified current context;
- bound state, matching nonzero conversation epoch and current guest owner;
- lobby ready and matching native user generation;
- the observed 17-byte form and discriminator 4;
- native sentinel number, with slot neither 3 nor 206.

Only the number's eight bytes are changed to the existing assigned guest
constant `0x7FFFFFFFFFFFF002`. Unknown numbers, forbidden slots, malformed
forms and stale state are not repaired. The caller's original packet, header,
discriminator, slot and opaque u32 stay untouched. No game-memory identity
field is written by this adapter. The existing target lobby/status hooks
retain their separately verified behavior.

The resulting copy must still pass **strict `identity3663::admit`**. That
check continues to require the assigned guest number and every existing
lobby/binding/generation/slot predicate. It revalidates state after translation;
a changed epoch or owner refuses admission. Lifecycle, channel, executable,
queue, halted-epoch and native transaction checks remain in force. The raw
game-packet transport envelope/protocol format is unchanged.

A bounded conversion log says that strict admission follows; it is not proof
of native admission, insertion or a successful join. The one-shot refusal
logger remains and uses the corrected expected size. Direct strict admission
still refuses the native sentinel; only the narrow validated copy adapter maps
it. The host's own client and all legacy paths are unchanged.

## Regression and validation

The regression uses the actual recorded first 13 bytes and total length 17.
Its final u32 is an explicitly artificial fixture sentinel, and the test proves
it survives unchanged. It checks pre-translation strict refusal, post-translation
strict acceptance, already-assigned identity, unknown numbers, forbidden slots,
all truncated/extended lengths, wrong discriminator, and every binding/epoch/
generation gate. Rejected copies are byte-identical. Diagnostic equivalence,
bounded prefix decoding and once-only logging tests remain.

Full `scripts/build.ps1`: **119/119, 106.30 seconds**. Synthetic smoke and the
uniquely named source/ASI arming seal are recorded in
`port-evidence/r6/r6-step3-validation.json`. No runtime fix is claimed until
the next L3 sitting demonstrates admission and actual mutual native actors.
The R6 budget before that sitting is startup **1/5**, join **1/3**.

The user clarified they had not selected host Quit to Desktop before the
watchdog; no hang was established. The next sitting's normal host
quit instruction is batched in advance, with time reserved for reload and quit.
User-recorded two-window evidence replaces the failed automated capture.
