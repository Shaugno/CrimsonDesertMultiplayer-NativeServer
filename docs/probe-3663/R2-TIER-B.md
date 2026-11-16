# R2 Step 4 Tier B — Internal tunnel

Decision: **blocked and unarmed**. 73/78 contracts are adopted; 5 remain blocked. All lower tiers are mandatory. The compiled completeness flag remains false.

Descriptor storage range B052/B053 and scan stride B078 remain unproved; the pump point B065 lacks sufficient standard support; packet bound B075 requires a real per-build acceptance path. Native target PostPacket uses 7FDA, so the legacy 7FE0 bound cannot be copied. Queue/tag ownership is proved only for complete mod-created zero-kind C2S buffers injected directly into Q0.

Implementation boundary: the existing legacy modules remain on their 2976 addresses. The target evidence catalog in build_3663.h grants no install permission. A complete target module accessor/adapter has not been published, and this report does not count a constants-only catalog as an implemented native port. No unsupported address fallback, new protocol format, or Steam policy change was introduced.

Guard validation: build_guard_tests exercises all 256 request/completeness combinations, each tier on/off through lower-tier dependencies, wrong image size/timestamp/base, unknown/missing identity and recognized 3576. It asserts actual 3663 startup is unarmed and cannot route legacy game reads, hooks, overlay, synthetic state or network callbacks. The target-only A callback is isolated. This proves denial, not successful native operation.

Latest game-closed scripts/build.ps1 x64 MSVC RelWithDebInfo: **101/101, 87.03 seconds**. Synthetic cdmp_host/cdmp_peer loopback: **78 remote states, zero rejected, clean network stop and worker exit**. It is not L2 or native two-player evidence. The preceding failures, fixes and reruns are retained in port-evidence/r2/validation-deviations.md and its referenced logs. Documentation-only tier decisions reuse this unchanged validated build.

Self-review: no new runtime thread or escaped reader lifetime; native calls remain behind closed startup gates; payloads retain existing bounds/serialization and friends restriction; target identity is exact; no address-based peer identity; no new deployment or reconnect/cleanup behavior is claimed. Deferred native ownership and per-build bounds remain prerequisites.

| Row | Contract | Exact missing fact |
|---|---|---|
| B052 | kTrocInstancesBegin | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kTrocInstancesBegin at 2976 0x6A51AE0 |
| B053 | kTrocInstancesEnd | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kTrocInstancesEnd at 2976 0x6A82C80 |
| B065 | kNetLoopPump | Standard support for candidate 0x3a8d94 (or its documented native operand anchor) is absent; independent check: Decoded seven-byte MOV RAX,[R15+E0] at the pre-flush native NetworkThread point; R15 is ClientNet, incoming queue+18 is handled before this point, then synchronous session+10 is flushed and returned queue+28 freed. Both complete parent loops reviewed. This is a safetyhook mid-point, not an inline entry. |
| B075 | kMaxPacketSize | The legacy shared packet limit 7FE0 differs from the target native PostPacket bound 7FDA and ordinary decoder payload bound 7FC1. A target-specific acceptance bound and complete framing check remain necessary. |
| B078 | NativeTrocScanSkip | The native descriptor scan skips 80h bytes after a match, then advances eight more. Target minimum instance span/alignment for every scanned descriptor is not yet established; the skip must not hide a neighbouring target descriptor. |

Next validation when prerequisites are resolved: regenerate the catalog, implement the actual per-build route, rerun guards/full CTest/loopback, then perform the plan’s ordered live test with fresh backups and presence. Never enable this flag merely to test an incomplete table.
