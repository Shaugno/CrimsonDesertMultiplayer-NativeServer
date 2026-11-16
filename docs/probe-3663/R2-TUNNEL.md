# R2 tunnel audit — native facts and remaining compatibility work

No Tier B path is armed. Reviewed field/entry-guard facts are compiled as
unused `build_3663.h::tunnel_layout` constants; they are separate from the 17
retained R1 adoptions. `r2-reviewed.json` records each row's evidence.

Native target PostPacket 0x3A9960 allocates 0x8030 bytes, stores frame/base
pointers at +8000/+8028, initializes packet-kind +800E to zero, uses u16
capacity +800C and lengths +8008/+800A, and copies packet bytes at frame+3.
It adds 0x1F transport bytes and writes length/channel to the frame. The
3663 LoginCharacter decoder separately requires a five-byte ordinary Troc
header, u16 payload length at +3 and a payload cursor at +5.

The native packet bound needs an explicit target path: PostPacket rejects
packet length above 0x7FDA, and native client/server receive checks
`u16(packet_length - 3) <= 0x7FD7`. The shared mod helper currently accepts
0x7FE0 bytes. Do not mark B075 verified unchanged. Preserve the 2976 behavior;
enforce the independently verified target bound in a per-build tunnel path
before forwarding anything native. This changes an acceptance bound, not the
serialized mod protocol. No such path is published yet.

The native writer actively fills +8010 (channel) and +8018/+8020 (epoch
values). The mod repurposes +8010/+8018 for its tag/magic. Their locations are
readable facts, but their safe reuse depends on every relevant dequeue and
release consumer. B046/B047 remain blocked on that downstream lifetime and
unused-space proof; initialization alone would not establish it.

The native session constructor and receivers confirm +88 is a u32 key pointer,
+90 is the user actor, and +98 is crypto. Native initialization of the named
virtual async session confirms host name +18, the B0/B8 epoch pair, C0 word,
and ServerNet's session at +18. Native PostPacket uses the no-session sentinel
0x7FFFFFFFFFFFFFFF for both epoch fields.

Further work: standard-correlator support for remaining network roots and
function entries; both queue ownership directions and packet release paths;
descriptor range/roles and changed native opcodes; verified mid-hook location
for the client pump; target bounds and a complete per-build module accessor.
Tier B also requires Tier A. Synthetic host/peer success does not establish
any of these native contracts or count as L2.
