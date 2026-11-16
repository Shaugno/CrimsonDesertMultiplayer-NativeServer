"""Reproduce the reviewed R2 core cut. No numeric-literal sweep is an adoption proof."""
import json
import re
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / 'docs/probe-3663'
inventory = json.loads((OUT / 'PORT-INVENTORY.json').read_text())['named']
rows = []
policies = []

def select(file, tier, use, *, names=None, start=0, end=100000, exclude=()):
    for old in inventory:
        path, line = old['source'].rsplit(':', 1)
        if path != file or not start <= int(line) <= end:
            continue
        symbol = old['symbol']
        if names is not None and symbol not in names.split():
            continue
        if symbol in exclude:
            policies.append((old['source'], symbol, old['value']))
            continue
        rows.append(dict(old, tier=tier, use=use))

G = 'src/mod/game/'
S = 'src/mod/session2/'
select(G+'offsets.h', 'A', 'PlayerReader::resolve_actor/read/read_stats: guarded local snapshot',
       start=16, end=58, exclude=('kMaxStatEntries',))
select(G+'offsets.h', 'A', 'PlayerReader::read loading sentinel', names='kLoadingParkHeight')
select(S+'offsets_s2.h', 'B', 'server_install/guest_install: session, queue, buffer and descriptor access',
       start=12, end=160, exclude=('kSamSlot','kSam_FreeKeys','kSam_ActorCount','kServerUserActorVtable',
       'kUser_Key','kUser_Session','kServerActor_Controller','kServerActor_Key','kToNearExceptOneControlVtable',
       'kToNearExceptOneSelfVtable','kToNearVtable','kExceptOneVf1','kExceptOneSelfVf1','kToNearVf1',
       'kExceptOneVf1Head','kToNearVf1Head','kFinder_Actor'))
# Those exclusions are moved to the first tier needing them, not policy.
policies.clear()
select(S+'offsets_s2.h', 'C', 'server users and native recipient selection', names='kSamSlot kSam_FreeKeys kSam_ActorCount kServerUserActorVtable kUser_Key kUser_Session kServerActor_Controller kServerActor_Key kToNearExceptOneControlVtable kToNearExceptOneSelfVtable kToNearVtable kExceptOneVf1 kExceptOneSelfVf1 kToNearVf1 kExceptOneVf1Head kToNearVf1Head kFinder_Actor')
select(S+'offsets_s2.h', 'D', 'guest refresh_title_slots and protected save-manager table', start=166, end=213)
select(S+'field_share.h', 'C', 'share_candidate/share_give_field/enter_insert/enter_undo/field snapshot')
select(S+'field_share.cpp', 'C', 'field_share_install and acquire/enter detours', end=37)
select(S+'identity_fix.h', 'C', 'identity_fix_install: unique user identity and status/lobby layout',
       exclude=('kUser2','kOldPuppet','kMaxEntries','kMaxActors'))
select(S+'identity_fix.cpp', 'C', 'identity_fix_install code validation, lobby/status and login-manager access', start=215, end=245)
select(S+'identity_probe.h', 'C', 'shared identity helpers used by the fix and guest-save hooks')
select(S+'identity_probe.cpp', 'C', 'identity_shared_hooks_install: shared load/phases entries and status validation',
       names='kLoad kLoadHead kPhases kPhasesHead kStatusVtable kStatusLoad kLoadStub')
select(S+'clock_gate.h', 'C', 'clock_gate_install: prevent the guest changing host field time')
select(S+'clock_gate.cpp', 'C', 'clock_gate_install entry and call-neighbour checks', end=37)
select(S+'server.cpp', 'C', 'server login filtering and native main-user/main-player acknowledgement tracking',
       names='kSessionId kOpLoginCharacterToFieldServerReq kOpLoginUserToFieldServerReq kOpMainPlayerDataStartAck kOpMainUserDataStartAck kOpMoveFieldAck kOpNak')
select(S+'guest.cpp', 'B', 'guest handshake and epoch ownership', names='kOpConnectCryptogramToServerReq kNoEpoch kServerFrameworkMode')
select(S+'guest.cpp', 'C', 'guest native login and inbound packet classification', names='kOpScopeSenderAck kOpActorAppear kOpDataAck kOp2679 kOpLoginCharacterToFieldServerReq kOpLoginUserToFieldServerReq')
select(S+'guest.cpp', 'D', 'guest native shutdown filter', names='kOpReserveShutDownServerReq')
select(S+'guest_lifecycle.h', 'D', 'snapshot/remove/barrier: SAM map, user slots, actor and field-move state',
       exclude=('kSamUserListMax','kMaxDepartedKeys'))
select(S+'guest_save_host.cpp', 'D', 'guest_save_install: native save/load/flush and protected slot access', end=91)
select(S+'guest_save_logic.h', 'D', 'guest save native file header, slot-name map, and classified native return sites',
       names='kHeaderSize kMagic kSlotNameMap kMapBucketCount kMapBuckets kMapEntries kBucketSize kBucketPairs kMaxBucketPairs kEntryKey kEntryValue kKeyChars kKeyLength kSlotNames kSlotNameCount kRetUserLogin kRetLobby3093 kRetCharacter3093 kRetAutosave kRetManual2910 kRetFlushLogout kRetFlushTimer')
select(S+'guest_save_wire.h', 'D', 'guest save policy names native request/ack identifiers; mod control format is excluded', start=67, end=73)
select(S+'report_guard.cpp', 'D', 'required native report guard installed before save writes', end=20)
select('src/common/troc_frame.h', 'B', 'verify native packet framing; preserve the mod batch/control format', names='kHeaderSize kMaxPacketSize')
select('src/common/troc_frame.h', 'D', 'verify native internal logout framing', names='kOpLogoutSessionReq kOpLogoutSessionAck kOpLogoutUserAck kInternalHeaderSize kLogoutAckSize')
select(S+'session2.cpp', 'C', 'required deny filter protects host native world time', names='kOpSetFieldTimeReq')
# Shared layer guards are a native player-safety dependency, not a mount feature.
select(G+'offsets.h', 'C', 'puppet_install_layer_guards: shared action-layer crash containment',
       names='kActionLayerAdvance kActionLayerAdvanceHead kActionLayersAdvance kActionLayersAdvanceHead kControl_Layer0 kLayer_Playback kLayer_Chart kActor_Key kActor_OwnerUser')
select('src/mod/net/horse_diag.cpp', 'C', 'existing share_field path unconditionally reaches character containment; must split from disabled horse diagnostics before arming',
       names='kCcUpdate kCcPost kRestore kPending')

def raw(tier, file, needle, symbol, value, use):
    lines = (ROOT / file).read_text(encoding='utf-8').splitlines()
    found = next(i+1 for i,s in enumerate(lines) if needle in s)
    rows.append(dict(tier=tier, symbol=symbol, value=value, source=f'{file}:{found}', use=use, id='inline'))

raw('A', G+'player_layout.h', 'stat_units = 1000.0f', 'NativeStatUnits', '1000.0f', 'native int64 stat conversion scale (not a timeout)')
raw('A', G+'player_layout.h', 'stat_id_bytes = 8', 'StatIdReadWidth', '8', 'legacy reader width; target native ID width must be confirmed separately from padding')
# Preserve existing R2 IDs while correcting a root omitted by the first line cut.
select(G+'offsets.h', 'A', 'PlayerReader root and inherited save-manager root', names='kWorldSystemSlot')
H = 'docs/probe-3663/port-evidence/r2/player-health-update-reference.txt'
raw('A', H, 'MOV R10,qword ptr [0x146d69528]', 'kHpDefinitionsSlot', '0x6D69528', 'native HP updater selects game-data definition registry')
raw('A', H, 'CMOVNZ R10,qword ptr [0x146d69530]', 'kHpDefinitionsAltSlot', '0x6D69530', 'alternate native HP definition registry; no assumed selector ID')
raw('A', H, 'MOV RAX,qword ptr [R10]', 'kHpDefinitionsRoot', '0x00', 'first native definition pointer hop')
raw('A', H, 'MOV RDX,qword ptr [RAX + 0x8]', 'kHpDefinitionsTables', '0x08', 'second native definition pointer hop')
raw('A', H, 'MOV RAX,qword ptr [RDX + 0x10]', 'kHpDefinitionsObject', '0x10', 'third native definition pointer hop')
raw('A', H, 'MOVZX ECX,word ptr [RAX + 0xa0]', 'kHpDefinitionId', '0xA0', 'native HP key field; target field is different')
raw('C', S+'field_share.cpp', 'lock_call(field + fld::kLock1, 8)', 'FieldLockAcquireVslot', '8', 'both field lock acquire slots')
raw('C', S+'field_share.cpp', 'lock_call(second, 0x10)', 'FieldLockReleaseVslot', '0x10', 'both field lock release slots')
raw('C', S+'field_share.cpp', '!peek(pj + 0x68', 'HostCharacterComponents', '0x68', 'field-share traversal to host components')
raw('C', S+'field_share.cpp', '!peek(c68 + 0x1A0', 'HostTransformSyncComponent', '0x1A0', 'field-share traversal to native transform sync')
raw('C', S+'field_share.h', 'struct FieldUserEntry', 'FieldUserEntryLayout', '16 B; user +0, actors +8', 'native field user vector element, confirm both member offsets and stride')
raw('D', S+'guest_save_logic.h', 'constexpr bool slot_number_valid', 'NativeSlotNumberDomain', '0..3;100..108;200..207', 'verify supported native slots before choosing reserved mod slot204')
raw('D', S+'guest_save_logic.h', 'constexpr bool slot_memory_only', 'NativeMemoryOnlySlots', '201;205..207', 'verify which slots cannot be persisted')
raw('D', S+'guest_save_guest.cpp', 'u32 key | u32 slot | u8 ok', 'SaveCurrentAckLayout', '14 B; key0 slot4 ok8 flag9 result10', 'native 3016 result parser')
raw('C', S+'session2.cpp', 'return blocked_native_control(opcode', 'NativeForbiddenRequests', '2790,2038,2004,1001..1008', 'native shutdown/pointer IDs plus unchanged conservative connection-deny interval policy; field-time ID is a separate row')
raw('D', S+'guest_save_wire.h', 'inline std::vector<uint8_t> make_save_req', 'SaveReq2910Layout', 'slot0:u32; length4:u32; UTF16+8; terminator; 10+2n bytes; n<=30', 'verify native 2910 writer and handler, including maximum native name length')
raw('D', S+'guest_save_wire.h', 'kOpLoginCharacterReq =', 'LoginCharacterSaveLayout', 'flag0:u8; characterNo1:u64; slot9:u32', 'native 3093 payload used for selecting and holding the guest save')
raw('D', S+'guest_save_logic.h', 'i.version = load_le', 'NativeSaveHeaderLayout',
    'magic0:u32; version4:u16 in1..2; size6:u16=80h; flagsCh:u32 mask2; raw12h:u32>0; data16h:u32 in1..raw',
    'native save validator schema, including inline field offsets and domain checks; file/raw size caps remain policy')
# The title refresh is reached by a remote connection, not only its test flag.
# Its parser has a separate stride from the offsets_s2 alias.
select(S+'slot_refresh.h','D','read_slot_table -> slot_refresh::parse_entries uses its own native stride',names='kEntrySize')
raw('D', S+'slot_refresh.cpp', 'std::memcpy(&e.slot', 'TitleSlotNumberOffset', '0',
    'title slot entry u32 number offset')
raw('D', S+'slot_refresh.cpp', 'std::memcpy(&e.time', 'TitleSlotTimeOffset', '8',
    'title slot entry i64 time offset')
raw('D', S+'slot_refresh.cpp', 'if (e.slot < 3', 'NativeTitleSlotDomain', '0..2;100..108',
    'native title enable predicate; unlike retry/count caps this is a game eligibility rule')
raw('B', S+'guest.cpp', 'peek(crypto + 0x24', 'NativeCryptoEnabledOffset', '0x24',
    'guest packet extraction rejects encrypted native frames based on this game field')
raw('B', S+'guest.cpp', '!peek(pb + 0x8020', 'NativePacketEpochOffset', '0x8020',
    'guest_send takes the native packet epoch before the session fallback')
raw('B', S+'server.cpp', 'a += 0x80;', 'NativeTrocScanSkip', '0x80',
    'load_channels skips native instance storage after a matching RTTI descriptor; verify the minimum instance span or remove this optimization on the target path')
# Split aggregate address arrays: each independent call/error address is a row.
expanded = []
for row in rows:
    if row['symbol'] == 'kCallSites':
        for i, (address, callee) in enumerate(re.findall(r'\{(0x[0-9A-Fa-f]+), (k\w+)\}', row['value'])):
            expanded.append(dict(row, symbol=f'kCallSites[{i}].site', value=address, use='native call site to '+callee))
    elif row['symbol'] in ('kErrBusy','kErrInvalidSlot','kErrAbsentSlot','kErrSaveFail'):
        for member, value in zip(('site','global'), re.findall(r'0x[0-9A-Fa-f]+', row['value'])):
            expanded.append(dict(row, symbol=row['symbol']+'.'+member, value=value))
    else:
        expanded.append(row)
rows = expanded

adopted = {m.group(1):m.group(2) for m in re.finditer(r'inline constexpr \w+ (\w+) = [^;]+;[^\n]*\b(N\d{4})\b', (ROOT/G/'build_3663.h').read_text())}
counts = Counter()
for row in rows:
    counts[row['tier']] += 1
    row['r2'] = f"{row['tier']}{counts[row['tier']]:03}"
    row['adopted_r1'] = row['id'] in adopted.values()
    # Every retained row is a game contract. Replace R1's unaudited mixed label;
    # excluded mod limits/protocol fields are explained in the scope below.
    row['classification'] = 'game'
    symbol,value = row['symbol'],row['value']
    if value.startswith('{'): kind='native signature'
    elif symbol.startswith('kOp') or symbol.startswith('kStatId_') or symbol=='NativeForbiddenRequests': kind='native identifier'
    elif 'Vtable' in symbol or symbol.startswith('kVt'): kind='vtable RVA'
    elif symbol=='kNoEpoch': kind='native epoch sentinel'
    elif symbol=='kLoadingParkHeight': kind='native loading sentinel'
    elif re.fullmatch(r'0x[0-9A-Fa-f]+',value) and 0x10000<int(value,16)<0x173AB000: kind='function/global RVA'
    elif 'Layout' in symbol: kind='native layout contract'
    elif any(s in symbol for s in ('Size','Stride','Count','Entries','Capacity','Width','Units','Overhead','Header')): kind='native member/size contract'
    else: kind='native member/value contract'
    row['kind']=kind
assert len(rows) < 500
assert sum(r['adopted_r1'] for r in rows) == 17
previous=OUT/'R2-CORE-LIST.json'
if previous.exists():
    indexed={r['r2']:r for r in rows}
    for old in json.loads(previous.read_text()):
        assert old['r2'] in indexed and (old['symbol'],old['value'])==(indexed[old['r2']]['symbol'],indexed[old['r2']]['value']),f"Core ID drift: {old['r2']}"
(OUT/'R2-CORE-LIST.json').write_text(json.dumps(rows, indent=2)+'\n')
intro = '''# R2 core list — reviewed reachability cut

Scope baseline: 90c12f5. Reader refactoring adds an explicit native ID-width
contract; declaration locations follow the working branch. Reproduce with
`python tools/port3663/r2_core.py`. Stable R2 IDs refer to the generated JSON.
Each row is a game contract, including signatures and native wire layouts;
a compound native structure/signature is shown in full as one contract.
R1 done means the existing evidence is retained, not that its tier is armed.

Reachability: mod worker -> PlayerReader (A); session2::install -> server_install
and guest_install -> queues, send/receive, Troc descriptor scan (B); create_session
-> recipient finders, field_share, identity_shared_hooks and clock_gate (C);
logout/barrier/rejoin -> guest_lifecycle, save_hooks, guest_save_host/guest,
guest_save_tunnel, refresh_title_slots and report_guard (D). Shared contracts are
assigned at their first tier; consumers in higher tiers inherit that permission.

actor_lookup and camera are excluded: PlayerReader never calls them. RTTI parser
metadata offsets are MSVC x64 ABI invariants, not game offsets; RTTI names are
independent matching evidence. Save_hooks uses Windows file APIs, not game RVAs.
rejoin_logic and mod save/control formats are policy/protocol, verified by tests.

The call audit found a coupling the R1 scope obscured: share_field calls both
puppet_install_layer_guards and horse_diag_install(contain=true). Their shared
character safety contracts remain C rows, while mount diagnostics themselves
must be split off and stay unreachable. Deleting the safety guards is not a port.
All PHASE2-SCOPE optional modules stay disabled; optional probe-only identity
sites are excluded, but shared load/phases entries remain.

Policy values stay unchanged: timeouts, rate/log limits, maximum bounded scans,
queue/backlog caps, retry/generation counts, epoch budgets, reserved mod actor
IDs and slot204 choice, mod magic/tag contents, exit codes, packet batch/control
IDs, protocol versions and wire fields. Native buffer locations used for mod
tags remain game rows because their unused-space claim must be proved. Native
container capacities derived from stride are game contracts, not policy limits.
MaxStatEntries=64, loading radius=5, MaxEntries=64, MaxActors=0x4000 and C4
SamUserListMax=0x4000 are policy bounds. Stamina stays included because today's
reader reads it; no unimplemented exclusion is used to make A appear complete.

'''
intro += 'Counts: '+', '.join(f'{t} {counts[t]}' for t in 'ABCD')+f'; total {len(rows)}; R1 adopted 17.\n\n'
intro += '| Row | Tier | Game contract | Symbol | 2976 value | Source | Use | Evidence status |\n|---|---|---|---|---|---|---|---|\n'
for row in rows:
    value = row['value'].replace('|','\\|')
    intro += f"| {row['r2']} | {row['tier']} | {row['kind']} | {row['symbol']} | `{value}` | {row['source']} | {row['use']} | {'R1 done' if row['adopted_r1'] else 'needs target evidence'} |\n"
(OUT/'R2-CORE-LIST.md').write_text(intro)
print(dict(counts), 'total', len(rows), 'R1 done', sum(r['adopted_r1'] for r in rows))
