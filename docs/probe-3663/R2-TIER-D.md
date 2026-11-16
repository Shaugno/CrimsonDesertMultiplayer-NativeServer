# R2 Step 4 Tier D — Removal, reconnect and save protection

Decision: **blocked and unarmed**. 108/130 contracts are adopted; 22 remain blocked. All lower tiers are mandatory. The compiled completeness flag remains false.

Target title-slot eligibility now depends on an available-resource ID vector and per-entry list membership. Complete producer/lifetime/eligibility handling is absent. Save callsite/return identities and moving-state members remain blocked. A native release function returning is not proof that deferred destruction and reconnect ownership barriers are complete.

Implementation boundary: the existing legacy modules remain on their 2976 addresses. The target evidence catalog in build_3663.h grants no install permission. A complete target module accessor/adapter has not been published, and this report does not count a constants-only catalog as an implemented native port. No unsupported address fallback, new protocol format, or Steam policy change was introduced.

Guard validation: build_guard_tests exercises all 256 request/completeness combinations, each tier on/off through lower-tier dependencies, wrong image size/timestamp/base, unknown/missing identity and recognized 3576. It asserts actual 3663 startup is unarmed and cannot route legacy game reads, hooks, overlay, synthetic state or network callbacks. The target-only A callback is isolated. This proves denial, not successful native operation.

Latest game-closed scripts/build.ps1 x64 MSVC RelWithDebInfo: **101/101, 87.03 seconds**. Synthetic cdmp_host/cdmp_peer loopback: **78 remote states, zero rejected, clean network stop and worker exit**. It is not L2 or native two-player evidence. The preceding failures, fixes and reruns are retained in port-evidence/r2/validation-deviations.md and its referenced logs. Documentation-only tier decisions reuse this unchanged validated build.

Self-review: no new runtime thread or escaped reader lifetime; native calls remain behind closed startup gates; payloads retain existing bounds/serialization and friends restriction; target identity is exact; no address-based peer identity; no new deployment or reconnect/cleanup behavior is claimed. Deferred native ownership and per-build bounds remain prerequisites.

| Row | Contract | Exact missing fact |
|---|---|---|
| D013 | kWindowsLock3Vtable | Standard support for candidate 0x59874c8 (or its documented native operand anchor) is absent; independent check: Exact mangled WindowsLock<3> name, primary COL and vslots 1/2 identify the table; native target save-manager constructor writes it to this+8 and stores that pointer at this+0. The independently verified virtual dispatch thunks corroborate the role. |
| D014 | kWindowsLockVf1 | Standard support for candidate 0x1593240 (or its documented native operand anchor) is absent; independent check: Named WindowsLock<3> vslot 1 has the paired native thunk that dispatches this through vslot+18. Exact body and named RTTI tables are reviewed; shared use by other WindowsLock classes does not create a different native role. |
| D015 | kWindowsLockVf2 | Standard support for candidate 0xe297f0 (or its documented native operand anchor) is absent; independent check: Named WindowsLock<3> vslot 2 has the paired native thunk that dispatches this through vslot+20. Exact body and named RTTI tables are reviewed; shared use by other WindowsLock classes does not create a different native role. |
| D020 | kSlotRefreshReqSequence | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kSlotRefreshReqSequence at 2976 0x2A22475 |
| D021 | kSlotRefreshReqSequenceBytes | Confirm 3663 code consumes kSlotRefreshReqSequenceBytes for the same native role as src/mod/session2/offsets_s2.h:191 ({ 0x48, 0x8B, 0x05, 0x1C, 0x72, 0x34, 0x04, 0x4C, 0x8B, 0x78, 0x48, 0x4C, 0x89, 0x7D, 0x07, 0x45, 0x32, 0xF6, 0x44, 0x88, 0x75, 0x0F, 0x49, 0x8B, 0x1F, 0x48, 0x89, 0x5D, 0x77, 0x48, 0x8B, 0x03, 0x48, 0x8B, 0xCB, 0xFF, 0x50, 0x08, 0x90, 0x45, 0x38, 0x77, 0x60, 0x75, 0x0C, 0x41, 0xC6, 0x47, 0x60, 0x01, 0x41, 0xB6, 0x01, 0x44, 0x88, 0x75, 0x0F, 0x48, 0x8B, 0x03, 0x48, 0x8B, 0xCB, 0xFF, 0x50, 0x10, 0x90, 0x45, 0x84, 0xF6, 0x75, 0x05, 0xE9, 0x91, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x0D, 0xCF, 0x71, 0x34, 0x04, 0x48, 0x8B, 0x49, 0x48, 0xE8, 0x4E, 0xE7, 0xE4, 0xFE, 0x90, 0x49, 0x8B, 0x1F, 0x48, 0x89, 0x5D, 0x77, 0x48, 0x8B, 0x03, 0x48, 0x8B, 0xCB, 0xFF, 0x50, 0x08, 0x90, 0x41, 0xC6, 0x47, 0x60, 0x00, 0x48, 0x8B, 0x03, 0x48, 0x8B, 0xCB, 0xFF, 0x50, 0x10}) |
| D022 | kTitlePlayReadsSaveMgr | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kTitlePlayReadsSaveMgr at 2976 0x65DC3E |
| D023 | kTitlePlayReadsSaveMgrBytes | Confirm 3663 code consumes kTitlePlayReadsSaveMgrBytes for the same native role as src/mod/session2/offsets_s2.h:212 ({0x4C, 0x8B, 0x1D, 0x4B, 0xB5, 0x70, 0x06, 0x49, 0x8B, 0x83, 0x88, 0x00, 0x00, 0x00}) |
| D030 | kSamBucketMaxEntries | Confirm 3663 code consumes kSamBucketMaxEntries for the same native role as src/mod/session2/guest_lifecycle.h:55 ((kSamBucketBytes - 8) / 8) |
| D044 | kTsMoving | Confirm 3663 code consumes kTsMoving for the same native role as src/mod/session2/guest_lifecycle.h:199 (0x3DC) |
| D045 | kFmMoving | Confirm 3663 code consumes kFmMoving for the same native role as src/mod/session2/guest_lifecycle.h:200 (0x58) |
| D063 | kCallSites[0].site | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kCallSites[0].site at 2976 0x2C17881 |
| D064 | kCallSites[1].site | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kCallSites[1].site at 2976 0x2C3E1CE |
| D065 | kCallSites[2].site | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kCallSites[2].site at 2976 0x1042ABF3 |
| D066 | kCallSites[3].site | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kCallSites[3].site at 2976 0xCBD86CE |
| D067 | kCallSites[4].site | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kCallSites[4].site at 2976 0x103BE99A |
| D100 | kRetUserLogin | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kRetUserLogin at 2976 0x2C17886 |
| D101 | kRetLobby3093 | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kRetLobby3093 at 2976 0x2C3E1D3 |
| D102 | kRetCharacter3093 | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kRetCharacter3093 at 2976 0x1042ABF8 |
| D103 | kRetAutosave | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kRetAutosave at 2976 0xCBD86D3 |
| D104 | kRetManual2910 | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kRetManual2910 at 2976 0x103BE99F |
| D115 | kReporterHead | Confirm 3663 code consumes kReporterHead for the same native role as src/mod/session2/report_guard.cpp:20 ({0x48, 0x89, 0x5C, 0x24, 0x10, 0x55, 0x56, 0x57, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57}) |
| D130 | NativeTitleSlotDomain | Target title eligibility adds per-entry u32-list membership at +68/count+70 against available .admeta IDs at global 71AEAA0/71AEAA8. Target producer 14AA720 reads those IDs in addition to names; reference producer 128D360 only builds names. The new global dependencies, list lifetime/coherence and complete target parser remain unadopted/unimplemented. The old domain-only playable count is insufficient. |

Next validation when prerequisites are resolved: regenerate the catalog, implement the actual per-build route, rerun guards/full CTest/loopback, then perform the plan’s ordered live test with fresh backups and presence. Never enable this flag merely to test an incomplete table.
