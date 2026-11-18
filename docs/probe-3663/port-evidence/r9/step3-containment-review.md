# R9 Step 3 — delivered-actor containment review

The R8 third attempt sampled a class-B actor's selected playback
pointer as null, then exited with an access violation in the
CharacterControl helper. The host sent 77 ActorLoadDataAck 2005
frames after shared Enter. The current diagnostic ring does not
record payloads, so it does not prove that actor `0xB01002C7`
was one of those 77, or that its null layer caused the terminating
exception. The host and guest primaries are class A and retain
their separate, narrow guard.

Samuel Cardillo's remote-update guard uses a remote ownership test
before short-circuiting character updates. A 3663 version could
track actor keys from validated native 2005 deliveries for the
current guest session. It would require a verified 3663 2005 actor
key parser, an epoch-bound bounded set, removal on actor unload and
disconnect, and the same exact `258C8FD` null read, native frame,
type and backlink checks already used by the primary guard. The
key would also have to differ from the local guest primary. Only
the helper's original epilogue would be eligible. Unknown actors,
stale keys, non-null playback and unverified native frames would
follow the original update.
[Samuel's remote-update guard](https://github.com/ByJunter/CrimsonDesertMultiplayer-NativeServer/blob/a3904af3e1a32056dcf7a63bc7a469eb4231084b/src/mod/net/horse_diag.cpp).

This containment policy is **not enabled**. The missing second
LoadingCompleteReq is observed before client control, and the class-B
sample does not establish why native loading is unfinished. The
next run records both host and guest opcode paths. If the failure
repeats, identify the actor in a payload-free, validated roster
probe and verify its lifecycle before considering this guard.
