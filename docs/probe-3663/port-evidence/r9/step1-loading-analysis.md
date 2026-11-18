# R9 Step 1 — loading pipeline, exact build 3663

The three R8 guest packet rings record every native guest send and
receive by opcode, direction, size and return status. The adjacent
[timeline](loading-timeline.json) gives every distinct guest request
after shared Enter with its count and first/last time. It retains
unknown request opcodes numerically. The original rings and host logs
are in the external session evidence directories named by each trial.

| Event after shared Enter | A1 | A2 | A3 |
| --- | --- | --- | --- |
| Shared Enter result | 0 at 21:58:39.553 UTC | 0 at 22:26:22.604 UTC | 0 at 22:58:33.116 UTC |
| Guest MoveFieldCompleteReq 2723 | sent once | sent once | sent once |
| Host MoveFieldCompleteAck 2268 delivered to native guest | once | absent before host crash | once |
| Host ActorLoadDataAck 2005 delivered | 78 | 1 | 77 |
| Guest LoadingCompleteReq 2789 | absent | absent | absent |
| Host LoadingCompleteAck 2891 | absent | absent | absent |
| Host PlayAllPreLoadSequencerAck 2941 | absent | absent | absent |

The guest's first loading cycle did receive 2941 and 2891 in A1 and
A2. It sent 2789 in that cycle, then sent GameLoadingStartReq 2350
for the move into the host's field. The native 3663 v1 request-name
allow-list already includes PreLoadSequencerReq and
LoadingCompleteReq. Its Lifecycle admits them while loading.
The guest ring records requests at the native Send hook before the
tunnel. There is no second 2789 or PreLoadSequencerReq 2372 in any
ring, so the guest did not issue those requests. The tunnel and
host admission path therefore cannot explain their absence.

The 2976 second-load capture sends MoveFieldCompleteReq, receives
its acknowledgement, then sends LoadingCompleteReq and receives its
acknowledgement. It does not show a PreLoadSequencerReq or a
PlayAllPreLoadSequencerAck as a prerequisite in that second cycle.
The missing exchange in 3663 begins at the client's second
LoadingCompleteReq. A1 and A3 received the preceding 2268 and
substantial actor bursts, then faulted before sending 2789. A2's
host crashed before the second 2268, so it cannot test the same
boundary.

Samuel Cardillo's 3527/3576 guest preloader observes loading state
`0x11` and sequencer marker below two. It repeats a previously
received preload acknowledgement for the current character, or
synthesizes one after a bounded wait. Its predicate also requires
the field to be owned by the guest. The R8 shared destination is
owned by the host, so the published predicate does not authorize
that behavior here. Its 3576 locations and vtable require an
independent 3663 match. A3's class-B null playback sample does not
establish that the missing sequencer marker caused the crash.
[Samuel's implementation](https://github.com/ByJunter/CrimsonDesertMultiplayer-NativeServer/blob/a3904af3e1a32056dcf7a63bc7a469eb4231084b/src/mod/session2/preload_release_logic.h).

No loading request is synthesized from this evidence. The host now
records a bounded, opcode-only tunnel ingress/egress ring next to
its crash capture. In a diagnostic L3, this shows whether the host
emits 2268, 2941 or 2891 and whether a guest request reached the
host tunnel. The guest ring identifies whether each response was
delivered into native receive. The game may still stop before the
second LoadingCompleteReq, in which case client loading state and
marker need a verified 3663 read before a pipeline fix.
