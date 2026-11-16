"""Independent native checks awaiting at least one standard match.

Unlike r2-reviewed.json, this catalog is never sufficient for adoption alone.
For globals/vtables, a standard function anchor plus the paired native operand
is allowed; the anchor proves correspondence, the operand proves the data RVA.
"""
import json
from pathlib import Path

root=Path(__file__).resolve().parents[2]
docs=root/'docs/probe-3663'
core=json.loads((docs/'R2-CORE-LIST.json').read_text())
result={}
def check(symbol,old,target,proof,evidence,anchor=None):
    rows=[r for r in core if r['symbol']==symbol and r['value'].lower()==hex(old).lower()]
    assert rows,(symbol,hex(old))
    for row in rows:
        result[row['r2']]=dict(target=hex(target),old=hex(old),symbol=symbol,
            second_check=proof,evidence=evidence,anchor=anchor)

check('kWorldSystemSlot',0x6D69190,0x72451C0,
    'Paired native consumers DD49D0/FB6F70 load this RIP-relative pointer then its +30 manager and call paired +50 actor getters; independent native HP updater and control-owner predicate use the same root.',
    'port-evidence/r2/player-controlled-reference.txt; player-controlled-target.txt; player-health-update-target.txt',
    [0xDD49D0,0xFB6F70])
check('kChunkOrigin',0x6D56500,0x7231330,
    'Paired native consumers 36A36A0/39F6B20 add the RIP-relative vec3 to physics+90 before subtracting a world-space target. The changed physics pointer is independently 2C0 -> 2D0.',
    'port-evidence/r2/player-inner-reference.txt; player-inner-target.txt',
    [0x36A36A0,0x39F6B20])
check('kStaticPlayerPos',0x6D77858,0x7253E60,
    'Paired native writers 941E20/A61070 populate the static world-position vector from the same controlled-stage position source and actor fallback, with changed target stage members.',
    'port-evidence/r2/player-physics-reference.txt; player-physics-target.txt',
    [0x941E20,0xA61070])
for symbol,old,target in [('kHpDefinitionsSlot',0x6D69528,0x7245558),('kHpDefinitionsAltSlot',0x6D69530,0x7245560)]:
    check(symbol,old,target,
        'Paired named native HP updaters 62FA00/71C4E0 select these two definition roots using the TLS mode, dereference +0/+8/+10 and read a u16 key (A0 -> 78). The key is runtime data, not literal zero.',
        'port-evidence/r2/player-health-update-reference.txt; player-health-update-target.txt',
        [0x62FA00,0x71C4E0])
for symbol,old,target,proof,evidence in [
    ('kAcquireField',0x2B07790,0x2E08520,'Pool/map lookup, two output ownership paths, open/owner assignment, strong-reference transfer; seven mapped caller neighbours.','field-contracts-reference.txt; field-acquire-enter-target.txt; field-mover-target.txt'),
    ('kEnterField',0x2BC04F0,0x2ECB130,'Native field admission and failure cleanup. ABI CHANGED: 3663 has three arguments and iterates its character list; matching this RVA does not authorize the legacy four-argument detour.','field-contracts-reference.txt; field-acquire-enter-target.txt'),
    ('kTryAddRef',0x14343A0,0x1658E90,'Native open-state/refcount path with field lock +18 and strong refs +10.','field-contracts-reference.txt; field-getter-target.txt'),
    ('kVecPush',0x40B5B0,0x4CAE10,'Native AddActor callee; 16-byte user entries, vector pointer/count/capacity, growth and element copy verified.','field-contracts-reference.txt; field-getter-target.txt')]:
    check(symbol,old,target,proof,'port-evidence/r2/'+evidence)

for symbol,old,target in [('kVtClientActorManager',0x55B62C8,0x59C2760),
    ('kVtControlledActor',0x5585C40,0x5990BF0),('kVtCharacterControl',0x55B5460,0x59C1660),
    ('kVtStatusComponent',0x558D868,0x5998B60)]:
    check(symbol,old,target,
        'Exact MSVC RTTI name, self-consistent COL/type/table chain in both images; native construction assigns this primary vtable. RTTI seeds are counted only as standard symbol evidence, not as an additional independent check; native construction supplies the latter.',
        'port-evidence/r2/player-rtti-old.json; player-rtti-new.json; player-contracts-target.txt; player-status-target.txt')
status_row,=[r['r2'] for r in core if r['symbol']=='kVtStatusComponent']
for image,table,method in [('old',0x558D868,0x625E00),('new',0x5998B60,0x712190)]:
    types=json.loads((docs/f'port-evidence/r2/player-rtti-{image}.json').read_text())['types']
    typ,=[t for t in types if t['name']=='.?AVClientStatusActorComponent@pa@@']
    vt,=[v for v in typ['vtables'] if int(v['rva'],0)==table]
    slot,=[s for s in vt['slots'] if s['index']==19]
    assert int(slot['rva'],0)==method
result[status_row]['anchor']=[0x625E00,0x712190]
result[status_row]['second_check'] += ' Paired primary-table slot 19 points to 625E00/712190. Both native virtual bodies obtain actor at component+8, test its key high bits, dispatch the same descriptor-type domain and invoke the status update. This matched virtual-member/slot relationship anchors the table independently of its RTTI label; it is not a claim that the table is a RIP operand inside that method.'
result[status_row]['evidence'] += '; player-status-virtual-old.txt; player-status-virtual-new.txt'
check('kVtPhysicsObject',0x5D00F88,0x614A050,
    'Native physics constructors 1408ECE0/11B11940 assign these vtables and construct the rotation/position base; mapped constructor/destructor call neighbours, no MSVC COL assumed.',
    'port-evidence/r2/player-physics-reference.txt; player-physics-constructor-target.txt; player-physics-target.txt',
    [0x1408ECE0,0x11B11940])
check('kLoadSlot',0x186F570,0x1A16C40,
    'Paired native four-argument LoadSlot verifies the same slot domain, actor presence, lock/busy gate, storage-backed load, result propagation and busy cleanup; named LoginCharacter implementation invokes it in the corresponding native login sequence.',
    'port-evidence/r2/save-manager-reference.txt; save-manager-target.txt; field-login-role-target.txt')
check('kServerActorLookupSlot',0x6D69698,0x72456D0,
    'Named SaveDataSlotRefreshReq consumes the RIP-relative lookup root and +48 save manager, takes/releases its lock, tests/sets +60 busy and calls the retained refresh function.',
    'port-evidence/ghidra-3663-core-handlers.txt; port-evidence/r2/save-manager-reference.txt',
    [0x2A22380,0x2D133B0])
for symbol,old,target,role in [
    ('kErrBusy.site',0x186F6DF,0x1A16DCE,'busy return instruction'),
    ('kErrBusy.global',0x6CF7CA4,0x71D0CFC,'busy error-code global'),
    ('kErrInvalidSlot.site',0x186F62C,0x1A16D1F,'first invalid-slot return instruction'),
    ('kErrInvalidSlot.global',0x6CF7900,0x71D092C,'invalid-slot error-code global')]:
    check(symbol,old,target,
        f'Paired LoadSlot native {role}: exact MOV EAX,[RIP+error] decoded in both files, corresponding slot-domain or busy-under-lock branch and result propagation. No numeric error value is assumed unchanged.',
        'port-evidence/r2/save-manager-reference.txt; save-manager-target.txt',
        [0x186F570,0x1A16C40])

check('kLobbyLoad',0x2B7F4D0,0x2E879A0,
    'Paired ServerLobbyActorComponent native loads obtain the owner lock and INT64_MAX number, call paired lobby-entry writers with the same argument roles and propagate results. Exact RTTI vtable slot +50 independently identifies both entries; RTTI labels alone are not a second native-body check.',
    'port-evidence/identity-rtti-3663.json; port-evidence/r2/identity-contracts-reference.txt; identity-contracts-target.txt; identity-entry-reference.txt; identity-entry-target.txt')
check('kStatusLoad',0x293FE70,0x2C160B0,
    'Paired ServerStatusActorComponent loads call the common status loader, propagate its result and write INT64_MAX through the character-number pointer, changed +70 -> +90; target LoginCharacter lookup consumes +90 independently. Exact named RTTI slot +50 supplies entry identity.',
    'port-evidence/identity-rtti-3663.json; port-evidence/r2/identity-contracts-reference.txt; identity-contracts-target.txt')
for symbol,old,target,anchor in [
    ('kLobbyVtable',0x5B2BE58,0x5F6F2E8,[0x2B7F4D0,0x2E879A0]),
    ('kStatusVtable',0x5B203F0,0x5F63430,[0x293FE70,0x2C160B0])]:
    check(symbol,old,target,
        'Exact MSVC RTTI class/COL identifies this component vtable; slot +50 points to the independently reviewed native loader with matching role and changed fields. Standard loader agreement can anchor the paired slot relationship; RTTI naming alone does not satisfy adoption.',
        'port-evidence/identity-rtti-3663.json; port-evidence/r2/identity-contracts-reference.txt; identity-contracts-target.txt',anchor)
check('kLobbyConstant',0x2B7F66C,0x2E87DB0,
    'Native lobby loader instruction MOV RAX,7FFFFFFFFFFFFFFF followed by a stack store; its address-taken qword becomes the paired entry writer character-number argument. Target stack slot changes to RBP-40; no old instruction patch bytes are reused.',
    'port-evidence/r2/identity-contracts-reference.txt; identity-contracts-target.txt',[0x2B7F4D0,0x2E879A0])
check('kLobbyAddCall',0x2B7F6DB,0x2E87E10,
    'Native E8 call from the lobby loader to paired entry writer 2143E70/237EA60, with component, result, address-of-number, slot/name data and extra native arguments. This callsite proof does not establish a safe detour ABI.',
    'port-evidence/r2/identity-contracts-reference.txt; identity-contracts-target.txt; identity-entry-reference.txt; identity-entry-target.txt',[0x2B7F4D0,0x2E879A0])
check('kSave',0x186E450,0x1A15930,
    'Paired native SavePlayerToSlot has five argument roles (manager/result/slot/actor/options), formats slot name, serializes owning user and character, walks the pending file records, commits and propagates failure. Target manual-save wrapper 2BCD7E0 invokes it under the same manager busy/lock gate. No delta-only identification.',
    'port-evidence/r2/save-manager-reference.txt; save-wrapper-target.txt; save-write-target.txt')
check('kGetOrCreate',0x2355EC0,0x25BB470,
    'Paired three-argument GetOrCreateSlot takes the storage lock, resolves a name map or calls native CreateSlot, then applies the exact slot-prefix/memory-only rule. Target SavePlayerToSlot and component writer independently call this thunk. Denuvo bodies and native constants are reviewed.',
    'port-evidence/r2/save-storage-reference.txt; save-storage-target.txt; save-create-target.txt; save-slot-construct-target.txt; r2-save-constants.json')
check('kAddFile',0x235B100,0x25C0900,
    'Paired AddFile allocates 140 bytes, constructs native filename/header at +98, respects slot memory flags, inserts into slot+8 name map and appends entry to +688/+690. Target found by raw allocation-immediate lead, then full native role/ownership review; the lead itself is not adoption evidence.',
    'port-evidence/r2/save-storage-reference.txt; save-addfile-target.txt; save-name-map-reference.txt; save-name-map-target.txt; r2-save-constants.json')
check('kLoad',0x20688A0,0x22951B0,
    'Paired LoadComponentsFromFile, reached directly by matched-role LoadSlot, resolves slot name in storage, chooses lobby/save by actor descriptor, loads component data through each component vslot +50, and propagates absent-slot or component-load errors. Target status/lobby loaders supply independent downstream role checks. Instruction body and hook ABI still need their own review.',
    'port-evidence/r2/save-slot-construct-reference.txt; save-write-target.txt; identity-contracts-target.txt')
for symbol,old,target,anchor,role in [
    ('kErrSaveFail.site',0x186E780,0x1A15C2A,[0x186E450,0x1A15930],'failure MOV instruction after empty pending-file vector'),
    ('kErrSaveFail.global',0x6CF7CA0,0x71D0CF8,[0x186E450,0x1A15930],'RIP-relative failure result global'),
    ('kErrAbsentSlot.site',0x2069615,0x2295FCF,[0x20688A0,0x22951B0],'missing slot/name branch error MOV'),
    ('kErrAbsentSlot.global',0x6CF781C,0x71D0848,[0x20688A0,0x22951B0],'RIP-relative absent-slot result global')]:
    check(symbol,old,target,
        'Paired native '+role+'; explicit branch role, result propagation and file-backed RIP operand verified. Numeric error codes are not presumed unchanged.',
        'port-evidence/r2/save-manager-reference.txt; save-slot-construct-reference.txt; save-write-target.txt',anchor)
check('kFlush',0x1871930,0x1A192D0,
    'Paired six-argument native FlushPendingSaveData verifies busy/lock and generation, transfers/clears pending slot+5C, formats the slot, commits existing storage and notifies the result. Target was located from the verified busy-error global, then body and both direct callers were reviewed. No delta-only identification.',
    'port-evidence/r2/save-manager-reference.txt; save-flush-target.txt; save-busy-leads-target.json; save-flush-callers-reference.txt; save-flush-callers-target.txt')
for symbol,old,target,anchor,role in [
    ('kCallSites[5].site',0x28F273A,0x2BC2A6A,[0x28F26C0,0x2BC29F0],'logout-phase flush call'),
    ('kRetFlushLogout',0x28F273F,0x2BC2A6F,[0x28F26C0,0x2BC29F0],'return following logout-phase flush'),
    ('kCallSites[6].site',0x108393C8,0xED099C8,[0x10839370,0xED09970],'flush-timer native call'),
    ('kRetFlushTimer',0x108393CD,0xED099CD,[0x10839370,0xED09970],'return following flush-timer call')]:
    check(symbol,old,target,
        'Paired native '+role+': exact E8 to reviewed FlushPendingSaveData, same six argument roles, result handling and owning actor/save-context use. Raw call leads are only discovery; native instruction boundaries and both callers are reviewed. Return is exactly call+5.',
        'port-evidence/r2/save-flush-callers-reference.txt; save-flush-callers-target.txt; save-flush-call-leads-target.json',anchor)
check('kLoginMgrSlot',0x6D696A8,0x72456E0,
    'Paired logout callers read this exact RIP-relative global context. Its first pointer is passed to actor/login registry lookup; context+10 is the native save manager. Both roles and dereference levels are verified in native code; this does not adopt registry list fields or FindByCharacterNo.',
    'port-evidence/r2/save-flush-callers-reference.txt; save-flush-callers-target.txt; identity-contracts-reference.txt',
    [0x28F26C0,0x2BC29F0])
for symbol,old,target,role in [
    ('kFindByNoScan',0x2A84066,0x2D7A726,'manager vector/count validation window'),
    ('kFindByNoRead',0x2A84090,0x2D7A750,'actor component/status validation window')]:
    check(symbol,old,target,
        'Paired native FindByCharacterNo '+role+': same requested u64 number comparison, manager+68 read-lock protocol and owned result holder; vector+B8/count+C0 with pointer stride eight, and changed status number pointer+90. Target discovered by a raw displacement lead, then complete native body and instruction boundaries reviewed. This is a validation site, not a function-entry hook.',
        'port-evidence/r2/identity-contracts-reference.txt; identity-find-by-number-target.txt; identity-scan-leads-target.json',
        [0x2A83F80,0x2D7A640])
check('kServerFrameworkMode',0x6A65564,0x6F29B70,
    'Paired native server receive loads this exact RIP-relative dword as the TrocFind kind-mask argument. File-backed initial values are independently read as 11h in each exact image; this is not the descriptor server-kind constant 4 and no runtime value is presumed. The native consumer correspondence and operand establish the global role.',
    'port-evidence/r2/troc-role-reference.txt; troc-role-target.txt; troc-mode-operands.json',
    [0x2773C20,0x2A0EA70])
check('kClientOnRecvPacket',0x35CA00,0x3A9CF0,
    'Paired receive entries bound the u16 size-minus-three by 7FD7, obtain session crypto+98, dispatch through native TrocFind with kind mask 1, enforce descriptor min/max lengths and flag bits, build the request with session/key/handler/frame fields, invoke vslot+8 and enqueue to client+E8 or +F0. Cleanup retains target-specific destructors and TLS+1FE. Native entry boundaries and all argument roles are reviewed; a raw immediate search was discovery only.',
    'port-evidence/r2/tunnel-client-receive-reference.txt; client-receive-verified-target.txt')
check('kLoading',0x219D7F0,0x23DC630,
    'Named native LoadingCompleteReq reaches paired server transition 2BCAF00/2ED7960, then the six-argument completion implementation. Both test fifth-byte publication flag, copy component+450 as 32 bytes to mode-selected world clock, unpause field+17C and continue actor loading. The entry ABI and actual branch role are reviewed; no legacy-hook installation is authorized yet.',
    'port-evidence/r2/clock-gate-old.txt; clock-gate-new.txt; lifecycle-handlers-old.txt; lifecycle-handlers-new.txt; lifecycle-loading-old.txt; lifecycle-loading-new.txt')
check('kLoadingFlagCmp',0x219D82B,0x23DC66B,
    'Decoded CMP [RBP+50],R12B followed by JZ skips the clock publication block in the paired six-argument completion implementation; the fifth argument and later loading continuation are preserved.',
    'port-evidence/r2/clock-gate-old.txt; clock-gate-new.txt',[0x219D7F0,0x23DC630])
check('kWorldTime',0x6A6D238,0x6F31C98,
    'The actual mode-selected YMM store operand in paired loading completion maps source 6A6D238 to target 6F31C98. The other clock bank is 6A6D218 -> 6F31C78, not substituted for the core bank; both hold 32 bytes.',
    'port-evidence/r2/clock-gate-old.txt; clock-gate-new.txt',[0x219D7F0,0x23DC630])
check('kNetLoopPump',0x35BAD4,0x3A8D94,
    'Decoded seven-byte MOV RAX,[R15+E0] at the pre-flush native NetworkThread point; R15 is ClientNet, incoming queue+18 is handled before this point, then synchronous session+10 is flushed and returned queue+28 freed. Both complete parent loops reviewed. This is a safetyhook mid-point, not an inline entry.',
    'port-evidence/r2/network-pump-reference.txt; tunnel-layout-target.txt',[0x35B800,0x3A8AC0])
check('kClientNetSlot',0x6D69170,0x72451A0,
    'Paired client loops 35B800/3A8AC0 load this RIP-relative ClientNet pointer, test its +112 connection byte and pass it to native PostPacket 35C670/3A9960 with a locally constructed request. Adjacent framework pointer 6D69178 -> 72451A8 is distinct.',
    'port-evidence/r2/network-pump-reference.txt; tunnel-layout-target.txt',[0x35B800,0x3A8AC0])
check('kServerNetSlot',0x6D69680,0x72456B8,
    'Paired named GameLoadingStartReq handlers load this RIP-relative native ServerNet pointer and pass it to the native error-packet post with opcode 3F5 and length11. Independent completion handlers use the same pointer. No numeric delta or neighbouring global is substituted.',
    'port-evidence/r2/lifecycle-handlers-old.txt; lifecycle-handlers-new.txt',[0x29C59D0,0x2CAD9E0])
for symbol,old,target,proof in [
    ('kWindowsLockVf1',0x1371340,0x1593240,'Named WindowsLock<3> vslot 1 has the paired native thunk that dispatches this through vslot+18.'),
    ('kWindowsLockVf2',0xCE4EF0,0xE297F0,'Named WindowsLock<3> vslot 2 has the paired native thunk that dispatches this through vslot+20.')]:
    check(symbol,old,target,proof+' Exact body and named RTTI tables are reviewed; shared use by other WindowsLock classes does not create a different native role.',
        'port-evidence/r2/save-lock-rtti-old.json; save-lock-rtti-new.json; save-lock-native-old.txt; save-lock-native-new.txt')
check('kWindowsLock3Vtable',0x557C120,0x59874C8,
    'Exact mangled WindowsLock<3> name, primary COL and vslots 1/2 identify the table; native target save-manager constructor writes it to this+8 and stores that pointer at this+0. The independently verified virtual dispatch thunks corroborate the role.',
    'port-evidence/r2/save-lock-rtti-old.json; save-lock-rtti-new.json; save-lock-native-old.txt; save-lock-native-new.txt',[0x1371340,0x1593240])
(docs/'r2-rva-checks.json').write_text(json.dumps(result,indent=2)+'\n')
print('Independent RVA checks awaiting standard support',len(result))
