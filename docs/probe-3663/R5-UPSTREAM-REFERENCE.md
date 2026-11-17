# R5 — Samuel Cardillo PR #1 reference

Reference only: ByJunter/CrimsonDesertMultiplayer-NativeServer PR #1,
Samuel Cardillo, commit `a3904af3e1a32056dcf7a63bc7a469eb4231084b`.
Fetched the pull-request head as an object; no branch merge or source replacement.
Original attribution and GPL v3 remain applicable. Any later derived runtime
code must credit Samuel in its file and commit message.

The file-filtered Steam download used app 3321460, depot 3321461, manifest
5865785040848247748. The resulting executable reports 1.0.0.3576 and SHA-256
`C94AD6C3EA54970A6D2DA7A2719753210A7728A91698ED3405DD9125470ED98F`.
It is stored outside the checkout and was never executed. The installed
3663 executable remains
`8BE67178D9B9A149C361A731051665E0BA4D441E250AE1DFE645BD6E524345C8`.
The upstream build-specific static comments and reported live results are
separate provenance; neither is runtime evidence for 3663.

## EnterField comparison

Upstream `src/mod/session2/field_share.cpp` still declares a four-argument
`enter_detour(fm, res, field, companion)` and forwards all four arguments.
The native 3576 entry 2E63220 uses three arguments, independently confirming
the contract already established for 3663 at 2ECB130. Retain our typed
three-argument `field_share3663` adapter, guarded field locks, owned pins,
rollback and quarantine. Do not replace it with the old wrapper.

The 3576 native context also independently confirms AcquireField 2DA1550,
TryAddRef 1618170 and vector push 4A2890. These are reference addresses only;
3663 continues to use its separately verified catalog, never a copied delta.

## Guard candidates verified statically on both binaries

| Contract | 3576 | 3663 | Independent target check |
| --- | --- | --- | --- |
| Sequencer component leave | 23BBD00 | 242D4E0 | Named ServerSequencerActorComponent vtable slot 34, native body and entry/branch bytes |
| Owner-manager call point | 23BBD1E | 242D4FE | Whole-instruction midpoint; original conditional branch reaches own-manager call |
| Own-manager continuation | 23BBD2B | 242D50B | Native load of component+18 and original tail call |
| Steam achievement vf2 | 1983400 | 19E9070 | Named PlatformAchievementManager_Steam vtable slot 2 and native body |
| Achievement midpoint | 1983433 | 19E90A3 | Frame built; R15/RSi preserve owned arguments |
| Achievement cleanup/success | 198365E | 19E92CE | Both argument destructors, success value, original epilogue |

The target vtables are 5F70CB0 and 5BF4138 respectively. Both slots were read
from disk and checked against `.pdata` function entries. The target achievement
cleanup calls 3DB480 and 3E6F50; its rel32 bytes differ from the reference.
Never copy the old cleanup signature unchanged.

Samuel's field-leave rule preserves the owner's sequencer manager when a
different user's character leaves, while retaining the leaver's own unload.
His achievement rule remains sticky after remote play and follows native
argument cleanup rather than returning from a partially constructed frame.
These are useful candidates for the restricted 3663 route; this document
alone does not install either guard or claim live acceptance.

## Character guards and guest startup

The upstream character signatures help locate action-layer/update code, but
some are ambiguous: the post signature has two hits in each binary and the
restore signature has two target hits. Even unique heads need independent
native control-flow, object-layout and identity checks before adoption.
No character address or installer has been enabled from signatures alone.
The optional puppet/mount paths remain outside the minimal 3663 v1 route.

The subsequent file-only native comparison confirms the action chain
3576 update 9451E0 → 9496C0 → layers 251AC20 → layer 214F8D0 and
3663 update 9729C0 → 9770C0 → layers 258C5A0 → layer 21BD690.
Both layers bodies compare the chart at +118. The restore candidates
949EB0/9778B0 call pending 251B420/258CDA0 respectively. These paired edges
provide stronger evidence than a head match; they still grant no installer
permission and are not live character-guard acceptance.

Upstream's older Win32 save-hook route is not substituted for R5's tested
native profile resolver. S1/S2 first establish host startup without any file
sandbox and isolated guest title startup without a remote connection.

Local file-only evidence (ignored, retained for review):
`build/r5/reference-contracts.json`, `upstream-3576-native-context.txt`,
`upstream-guards3663-context.txt`, and `upstream-guard-rtti3663.json`.
Character contexts are `character3576-native-context.txt`,
`character3663-native-context.txt`, and `character3663-caller-context.txt`.
No runtime code has been copied at this checkpoint.

Sources: [pinned PR](https://github.com/ByJunter/CrimsonDesertMultiplayer-NativeServer/pull/1),
[field sharing](https://github.com/ByJunter/CrimsonDesertMultiplayer-NativeServer/blob/a3904af3e1a32056dcf7a63bc7a469eb4231084b/src/mod/session2/field_share.cpp),
[field-leave rule](https://github.com/ByJunter/CrimsonDesertMultiplayer-NativeServer/blob/a3904af3e1a32056dcf7a63bc7a469eb4231084b/src/mod/session2/field_leave_guard_logic.h),
[achievement rule](https://github.com/ByJunter/CrimsonDesertMultiplayer-NativeServer/blob/a3904af3e1a32056dcf7a63bc7a469eb4231084b/src/mod/session2/achievement_guard_logic.h).
