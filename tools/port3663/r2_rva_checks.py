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

(docs/'r2-rva-checks.json').write_text(json.dumps(result,indent=2)+'\n')
print('Independent RVA checks awaiting standard support',len(result))
