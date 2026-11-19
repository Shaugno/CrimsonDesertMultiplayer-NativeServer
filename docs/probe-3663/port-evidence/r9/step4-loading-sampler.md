# R9 Step 4 loading sampler

The guest samples once per second for at most 180 samples after the second
native `MoveFieldAck` (2364) returns from `ClientNet::OnRecvPacket`. It reads
memory only and reports an unreadable chain explicitly. The sampler does not
repeat, synthesize or block any loading packet.

The installed 3663 executable's SHA-256 is
`8BE67178D9B9A149C361A731051665E0BA4D441E250AE1DFE645BD6E524345C8`.
Its RTTI names `.?AVClientSequencerStageManager@pa@@` and locates that class's
vtable at RVA `0x59D57E8` (see `sequencer-rtti-3663.json`). The
constructor at `0x9C85530` installs that vtable at `0x9C85559` and zeroes
the word at object `+0x258` at `0x9C8563E`, covering the candidate marker
byte at `+0x259`. The 3663 field-change function at `0x96D04A0` writes its
state at object `+0x15C` at `0x96D04E7`. The world slot `0x72451C0` and
controlled actor manager/actor chain were already independently adopted in
the 3663 player layout. Each live manager read requires this exact vtable.

Samuel Cardillo's 3576 preloader reads the state at controller `+0x15C`
and the marker at client sequencer `+0x259` through the same object-chain
shape. The 3663 static checks above support observing those fields; the
marker **semantics** still require live correlation. A state `0x11` with a
low marker will be recorded as an observation, not an automatic reason to
inject the 3576 preload acknowledgement. Samuel's release predicate also
requires a guest-owned field, whereas the shared destination is host-owned.
See [Samuel's implementation](https://github.com/ByJunter/CrimsonDesertMultiplayer-NativeServer/blob/a3904af3e1a32056dcf7a63bc7a469eb4231084b/src/mod/session2/guest.cpp).

