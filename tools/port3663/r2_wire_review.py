"""Materialize reviewed native descriptor IDs and separately reviewed payloads.

Requires the paired RTTI/descriptor exports, initializer disassembly and target
handler review. No function/global RVA is adopted by this script.
"""
import json
from pathlib import Path
import re

root=Path(__file__).resolve().parents[2]
docs=root/'docs/probe-3663'
folder=root/'build/r2'
core=json.loads((docs/'R2-CORE-LIST.json').read_text())
candidates=json.loads((folder/'request-candidates.json').read_text())
candidates+=json.loads((folder/'tunnel-control-candidates.json').read_text())
base=0x140000000
def sections(path):
    text=path.read_text(encoding='utf-8-sig')
    parts=re.split(r'^RVA=(0x[0-9A-Fa-f]+|[0-9]+)\s*$',text,flags=re.M)
    return {int(parts[i],0):parts[i+1] for i in range(1,len(parts),2)}

initializers={i:sections(folder/f'request-initializers-{suffix}.txt')
              for i,suffix in [('old','reference'),('new','target')]}
handlers=sections(folder/'request-handlers-target.txt')
for image,suffix in [('old','reference'),('new','target')]:
    initializers[image].update(sections(folder/f'tunnel-control-native-{suffix}.txt'))
handlers.update(sections(folder/'tunnel-control-native-target.txt'))

# Native registration reads the named descriptor's actual +C opcode. Native
# lookup uses the registered u16 key and kind mask before selecting its handler.
registration=(folder/'troc-register-target.txt').read_text(encoding='utf-8-sig').lower()
lookup=(folder/'troc-dispatch-lookup-target.txt').read_text(encoding='utf-8-sig').lower()
insertion=(folder/'troc-dispatch-insert-target.txt').read_text(encoding='utf-8-sig').lower()
assert 'movzx r8d,word ptr [0x146f173cc]' in registration
assert 'lea rax,[0x146f173c0]' in registration and 'call 0x14165ff90' in registration
assert 'cmp word ptr [rdx + 0x4],r11w' in lookup
assert 'test dword ptr [rcx + 0x8],esi' in lookup
assert '*(undefined2 *)(pivar5 + 1) = *(undefined2 *)*param_2' in insertion
descriptors={i:{r['name']:r for r in candidates if r['image']==i} for i in ('old','new')}
for image,expected in [('old','57DA440D72F4DB974F25FEF047CF84C4DADD999A88CB2A3C5AF4C9BD67FDE1E7'),
                       ('new','8BE67178D9B9A149C361A731051665E0BA4D441E250AE1DFE645BD6E524345C8')]:
    raw=json.loads((folder/f'request-rtti-{image}.json').read_text())
    assert raw['identity']['sha256']==expected

contracts={
    'ConnectCryptogramToServerReq':(2319,['kOpConnectCryptogramToServerReq']),
    'LoginUserToFieldServerReq':(2490,['kOpLoginUser','kOpLoginUserToFieldServerReq']),
    'LoginCharacterToFieldServerReq':(3093,['kOpLoginCharacter','kOpLoginCharacterToFieldServerReq','kOpLoginCharacterReq']),
    'MainPlayerDataStartAck':(2756,['kOpMainPlayerDataStartAck']),
    'MainUserDataStartAck':(2646,['kOpMainUserDataStartAck']),
    'MoveFieldAck':(2174,['kOpMoveFieldAck']),
    'ScopeSenderAck':(2042,['kOpScopeSenderAck']),
    'ActorLoadDataAck':(2005,['kOpActorAppear']),
    'CharacterLobbyDataAck':(2869,['kOpDataAck']),
    'SetFieldTimeReq':(2125,['kOpSetFieldTimeReq']),
    'ReserveShutDownServerReq':(2790,['kOpReserveShutDownServerReq']),
    'SaveAutoReq':(3037,['kOpSaveAutoReq']),
    'SaveToFileCurrentPlayerReq':(2910,['kOpSaveToFileCurrentPlayerReq']),
    'SaveToFileCurrentPlayerAck':(3016,['kOpSaveToFileCurrentPlayerAck']),
    'CreateCharacterReq':(2711,['kOpCreateCharacterReq']),
    'NewGameStartReq':(2178,['kOpNewGameStartReq']),
    'LogoutSessionToServerReq':(2001,['kOpLogoutSessionReq']),
    'LogoutSessionToServerAck':(2002,['kOpLogoutSessionAck']),
    'LogoutUserToServerAck':(2003,['kOpLogoutUserAck']),
    'LogoutCharacterToFieldServerAck':(2679,['kOp2679']),
}
result={}
for short,(old_id,symbols) in contracts.items():
    name=f'.?AVTrocTr{short}@pa@@'
    old,new=[descriptors[i][name] for i in ('old','new')]
    assert old['opcode']==old_id
    for image,row in [('old',old),('new',new)]:
        block=initializers[image][int(row['initializer'],0)].lower()
        assert f'lea rax,[0x{base+int(row["vtable"],0):x}]' in block
        assert f'mov qword ptr [0x{base+int(row["instance"],0):x}],rax' in block
    body=handlers[int(new['handler'],0)]
    assert 'Function=' in body and 'BLOCKED' not in body
    rows=[r for r in core if r['symbol'] in symbols and r['value']==str(old_id)]
    assert rows,short
    for row in rows:
        result[row['r2']]=dict(target=str(new['opcode']),old=row['value'],symbol=row['symbol'],
            method='native named descriptor, verified initializer and handler',
            second_check=f'{name}: source opcode {old_id}, target {new["opcode"]}; paired native initializer stores the RTTI vtable into its exact instance. Target vslot 2 handler {new["handler"]} reviewed. Native registration CB4BE0 reads descriptor+C into the registry key; insertion 165FF90 stores the u16 key, lookup 165FAC0 compares it and checks descriptor kind before dispatch. Native ServerOnRecvPacket selects that descriptor and handler. ID adoption does not adopt payload or hook ABI.',
            evidence='port-evidence/r2/request-rtti-old.json; request-rtti-new.json; request-candidates.json; request-initializers-reference.txt; request-initializers-target.txt; request-handlers-target.txt; tunnel-control-candidates.json; tunnel-control-native-reference.txt; tunnel-control-native-target.txt; troc-register-target.txt; troc-dispatch-lookup-target.txt; troc-dispatch-insert-target.txt')

def payload(symbol,description,proof,members=None):
    row,=[r for r in core if r['symbol']==symbol]
    result[row['r2']]=dict(target=description,old=row['value'],symbol=symbol,
        method='paired native payload consumption',second_check=proof,
        evidence='port-evidence/r2/request-handlers-reference.txt; request-handlers-target.txt; request-candidates.json')
    if members: result[row['r2']]['members']=members

payload('SaveCurrentAckLayout','14 B; key0 slot4 ok8 flag9 result10',
    'Named old BE02B0 and target D42840 decode consecutive 4,4,1,1,4-byte fields and require exact payload consumption. Current-player save completion uses the slot/flag/result in the corresponding native callbacks. Both have the ordinary five-byte packet envelope.',
    dict(kSaveCurrentAckSize=14,kSaveCurrentAckKey=0,kSaveCurrentAckSlot=4,kSaveCurrentAckOk=8,kSaveCurrentAckFlag=9,kSaveCurrentAckResult=10))
payload('SaveReq2910Layout','slot0:u32; length4:u32; UTF16+8; terminator; 10+2n bytes; n<=30',
    'Named old 2A21B80 and target 2D12AC0 consume a u32 slot, u32 name length <31, 2*n+2 UTF16 bytes including a checked NUL, advance the reader by that amount and require exact payload exhaustion.',
    dict(kSaveRequestSlot=0,kSaveRequestNameLength=4,kSaveRequestName=8,kSaveRequestFixedSize=10,kSaveRequestMaxName=30))
payload('LoginCharacterSaveLayout','flag0:u8; characterNo1:u64; slot9:u32',
    'Named old 298BA10 and target 2C6AE10 decode a one-byte flag, eight-byte character number, then a four-byte slot, in that order. This proves the prefix used by the guest slot selector, not an assertion that the entire native payload ends at byte13.',
    dict(kLoginCharacterFlag=0,kLoginCharacterNumber=1,kLoginCharacterSlot=9))
for symbol,value in [('kInternalHeaderSize','3'),('kLogoutAckSize','7')]:
    payload(symbol,value,
        'Named LogoutSession/User Ack handlers consume a u32 at packet+3; paired descriptors require exactly seven bytes. Named LogoutSession Req descriptor requires exactly three bytes and its handler uses the existing session context without decoding an ordinary u16 payload length. This confirms the distinct internal header, not the ordinary five-byte format.')

(docs/'r2-wire-reviewed.json').write_text(json.dumps(dict(rows=result),indent=2)+'\n')
print('Reviewed native IDs/payload contracts',len(result),'(no RVA adoption)')
