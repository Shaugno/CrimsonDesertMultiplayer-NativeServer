# R3 exact remaining contracts

Merged R2 history plus approved R3 dispositions. Historical R2 files are unchanged.
Closed rows include policy decisions and a scope retirement; this is not a count of native addresses.

| Tier | Closed / total | Blocked | Armed |
| --- | ---: | ---: | --- |
| A | 37 / 37 | 0 | True |
| B | 78 / 78 | 0 | True |
| C | 130 / 150 | 20 | False |
| D | 109 / 130 | 21 | False |

The native C backend and guest-save/lifecycle D backend are also incomplete.
See R3-JOIN-ADAPTER.md and R3-STEP4.md; these are blockers, not a request for review approval.

## Tier C

| Row | Symbol | Missing fact |
| --- | --- | --- |
| C001 | kSamSlot | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kSamSlot at 2976 0x6DA67D8 |
| C047 | kVecPush | Accepted VT/native AddActor identifies candidate 4CAE10; default BinDiff instead maps legacy 40B5B0 to 14AD670 (confidence 0.952574). Resolve the generic-helper conflict and its native allocation/growth ownership; named-RTTI rule 2 does not settle it. The passive trace did not instrument this helper. |
| C066 | kCharacterClass | Roster kind 0 and observed primary key A0100001 do not prove the separate legacy character-class constant 0xA in its actual producer/consumer role. |
| C070 | kLobbyConstant | Lobby parent 2E879A0 is now adopted. Candidate constant site 2E87DB0 still needs the target stack-slot/argument lifetime, exact relocated instruction and safe per-build patch semantics; the old instruction patch must not be reused. |
| C072 | kLobbyAddCall | Lobby parent is adopted, but candidate call 2E87E10 to entry writer 237EA60 still lacks a proved target callsite detour ABI/argument ownership and safe guard/relocation strategy. |
| C074 | kClearLobbySite | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kClearLobbySite at 2976 0x2C3E0FF |
| C075 | kClearLobbySiteBytes | Confirm 3663 code consumes kClearLobbySiteBytes for the same native role as src/mod/session2/identity_fix.cpp:227 ({0x49, 0x8B, 0x47, 0x68, 0x48, 0x8B, 0x58, 0x10, 0x48, 0x8B, 0x7B, 0x08}) |
| C076 | kClearListSite | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kClearListSite at 2976 0x2C3E126 |
| C077 | kClearListSiteBytes | Confirm 3663 code consumes kClearListSiteBytes for the same native role as src/mod/session2/identity_fix.cpp:229 ({0x48, 0x8B, 0x4B, 0x18}) |
| C080 | kLoginMgrSite | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kLoginMgrSite at 2976 0x2777A18 |
| C081 | kLoginMgrSiteBytes | Confirm 3663 code consumes kLoginMgrSiteBytes for the same native role as src/mod/session2/identity_fix.cpp:235 ({0x48, 0x8B, 0x0D, 0x89, 0x1C, 0x5F, 0x04, 0x48, 0x8B, 0x09}) |
| C103 | kLoadStub | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kLoadStub at 2976 0x35EA90 |
| C110 | kTravel | Legacy standalone Travel 2BC0A30 is inlined into target transition 2ECA480. Need a new verified publication adapter; 2ECB660 is roster-position preparation, not this function. |
| C111 | kTravelHead | The old standalone Travel entry guard cannot be redirected. Establish an exact target midpoint guard/relocation and native calling context for the inlined publication adapter. |
| C112 | kTravelOwnerCmp | Native owner comparison is at target 2ECAA97 inside 2ECA480; prove the guest ownership predicate and safe suppression context rather than copying the legacy compare patch. |
| C113 | kTravelOwnerCmpBytes | Establish live guard bytes/relocation and register/flags semantics for the new inlined owner-comparison point; no legacy compare bytes are reused. |
| C114 | kTravelGetterCall | Target time publication is inlined near 2ECAB4C into bank 6F31C98. Need the exact target getter/publication context and safe adapter, not a guessed standalone callsite. |
| C115 | kTravelGetterCallBytes | The old E8 callsite bytes do not establish the new inlined publication ABI/guard. Verify the replacement point and original instruction effects. |
| C122 | kSessionId | Confirm 3663 code consumes kSessionId for the same native role as src/mod/session2/server.cpp:76 (0) |
| C144 | kActor_OwnerUser | The observed native ServerActor owner at +A0 is also +A0 in the paired reference AddActor body. It does not prove the separate legacy Actor_OwnerUser+90 contract; identify its actual object type/consumer and target member. |

## Tier D

| Row | Symbol | Missing fact |
| --- | --- | --- |
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
| D130 | NativeTitleSlotDomain | Observed positive title predicate is implemented/tested on owned data. Native parser remains blocked on coherent slot+68/count+70 ID-vector ownership, available IDs at 71AEAA0/count71AEAA8, producer 14AA720 and copy/clear lifetime. Do not substitute the broader LoadSlot domain or the old inverted callback. Pure predicate logic now exists; coherent native ownership/parser is still missing. |

