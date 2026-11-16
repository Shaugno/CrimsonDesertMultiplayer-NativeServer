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
release consumer. Initialization alone did not establish safe reuse. The downstream review
below now supplies a narrowly scoped proof for mod-created C-to-S buffers;
native packet/request buffers are not treated as unused storage.

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

## Downstream packet ownership and pump continuation

Named thread strings identify paired server initialization 100B5BF0/E6F6530.
Their exact thread-entry operands identify ServerRunThread 2773390/2A0E1A0
and run loops 27733F0/2A0E200. Both pop Q0 at ServerNet+20, test buffer+800E,
dispatch the zero-kind packet through the native session at ServerNet+18
and push the unchanged allocation to return queue+78. The nonzero request
branch alone dereferences buffer+8010 as a request object.

Paired client loops 35B800/3A8AC0 use ClientNet in R15. They drain incoming
queue+18, flush through synchronous session+10, and drain returned C-to-S
buffers from queue+28. The free path reads buffer+8028, clears its frame
pointer and frees that allocation. It does not read +8010/+8018/+8020.

B046/B047 are now proved only for the existing mod injection contract:
allocate the complete buffer, set request-kind zero, frame/base pointers to
itself, and push it directly to native Q0. This bypasses native PostPacket
and its forwarder. Tag/magic reuse is not authorized on native-created
buffers or nonzero-kind requests; epoch/in-flight ownership remains mandatory.
The downstream path and native receiver body were inspected, not executed.
No target injection path is enabled by this proof.

The pre-flush client midpoint is 35BAD4 -> candidate 3A8D94, at the decoded
seven-byte MOV RAX,[R15+E0]. Its native queue ordering and register role are
reviewed, and target entry/midpoint guard bytes are catalogued. This site
requires safetyhook::create_mid and separate standard support; it must not
receive an inline entry hook. ClientNet root candidate is 72451A0 from that
loop's actual PostPacket operand; adjacent 72451A8 is a different object.
ServerNet root candidate 72456B8 comes from named native loading-request
error-post operands. These globals remain unadopted pending standard anchors.

The catalog now has 234 reviewed contracts plus 17 retained R1 values and
55 independent RVA candidates. A and all higher tiers remain closed.
The 234-contract build passed 101/101 tests in 89.13 seconds; game-free smoke
completed cleanly. Remaining native bounds, server-to-client queue ownership,
descriptor range/scan and target module routing still prevent Tier B arming.

The final alias check follows both framework producers: old 9E2C260 and
target 9BA4680. Their descriptor carries B/B+80/B+100/B+180, with framework
queue-block member 140 -> 148. Target server start 2A0CE20 binds those values
to +20/+78/+80, and client start 3A8040 binds +18/+20/+28 through its native
virtual-init thunk. Thus both return-queue aliases are established by actual
setup code as well as the consumers. The client code at B897BA is a decoded
caller fragment, not a proposed inline-hook entry. No fragment RVA is adopted.
