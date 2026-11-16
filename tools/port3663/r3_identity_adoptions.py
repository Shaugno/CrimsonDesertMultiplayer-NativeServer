"""Apply R3 named-slot and traced-entry rules, preserving correlator conflicts."""
from pathlib import Path
import hashlib
import json
import re
import sqlite3
from pe_identity import PE

ROOT=Path(__file__).resolve().parents[2]
LOCAL=Path(r'C:\Users\shaug\.codex\visualizations\2026\11\14\01a1dbbd-2726-7441-9a57-1b454819b3e7')
old=PE(LOCAL/'reference-bins/2976/CrimsonDesert.exe')
new=PE(Path(r'D:\SteamLibrary\steamapps\common\Crimson Desert\bin64\CrimsonDesert.exe'))
assert old.sha256=='57DA440D72F4DB974F25FEF047CF84C4DADD999A88CB2A3C5AF4C9BD67FDE1E7'
assert new.sha256=='8BE67178D9B9A149C361A731051665E0BA4D441E250AE1DFE645BD6E524345C8'
db=sqlite3.connect((LOCAL/'analysis-tools/r2-nativefix/bindiff/old_vs_new.BinDiff').as_uri()+'?mode=ro',uri=True)
core={r['r2']:r for r in json.loads((ROOT/'docs/probe-3663/R2-CORE-LIST.json').read_text(encoding='utf-8'))}
rows={}
for name,source_table,target_table,source_fn,target_fn,table_rows,fn_rows in [
    ('ServerLobbyActorComponent',0x5B2BE58,0x5F6F2E8,0x2B7F4D0,0x2E879A0,['C069'],['C067']),
    ('ServerStatusActorComponent',0x5B203F0,0x5F63430,0x293FE70,0x2C160B0,['C078','C101'],['C079','C102'])]:
    for key,image,table,fn in [('old',old,source_table,source_fn),('new',new,target_table,target_fn)]:
        types=json.loads((ROOT/f'build/r3/identity-rtti-{key}.json').read_text(encoding='utf-8'))['types']
        typ,=[t for t in types if t['name']==f'.?AV{name}@pa@@']
        primary=[v for v in typ['vtables'] if image.u32(image.offset(int(v['locator'],0)+4,4))==0]
        vt,=primary
        assert int(vt['rva'],0)==table
        assert image.u64(image.offset(table+10*8,8))==image.image_base+fn
    if name=='ServerStatusActorComponent':
        source=(ROOT/'build/r3/status-native-old.txt').read_text(encoding='utf-8')
        target=(ROOT/'build/r3/status-native-new.txt').read_text(encoding='utf-8')
        assert ' + 0x70)' in source and ' + 0x90)' in target
        assert '0x7fffffffffffffff' in source and '0x7fffffffffffffff' in target
        behavior='Both slot-10 thunks reach native five-argument loads: deserialize via base parser; propagate errors; reset character number to INT64_MAX through member 70->90; apply the owner/lobby counter floor. Owner virtual call changed F0->160, recorded rather than presumed unchanged.'
    else:
        source=(ROOT/'docs/probe-3663/port-evidence/r2/identity-contracts-reference.txt').read_text(encoding='utf-8')
        target=(ROOT/'docs/probe-3663/port-evidence/r2/identity-contracts-target.txt').read_text(encoding='utf-8')
        assert '0x7fffffffffffffff' in source and '0x7fffffffffffffff' in target
        behavior='Paired native lobby bodies lock the component owner, consume the reader, populate/search character entries and pass INT64_MAX to the entry writer. Two ordinary live calls returned their result pointers with success; not guest identity normalization evidence.'
    conflict=[dict(target=hex(t-old.image_base),similarity=s,confidence=c) for t,s,c in db.execute(
        'select address2,similarity,confidence from function where address1=?',(old.image_base+source_fn,))]
    for key in table_rows+fn_rows:
        rows[key]=dict(symbol=core[key]['symbol'],old=core[key]['value'],target=hex(target_table if key in table_rows else target_fn),
            adopted=True,method='R3 rule 2: exact named primary vtable and slot 10 plus independent native body',
            name=name,slot=10,body=behavior,default_BinDiff=conflict,tier_c_armed=False)

trace=json.loads((ROOT/'docs/probe-3663/port-evidence/r3/join-result.json').read_text(encoding='utf-8'))
assert trace['all_records_valid'] and trace['distinct_fields']==2 and len(trace['enter_results'])==2
assert 0x2ECB130 in new.entries
rows['C043']=dict(symbol='kEnterField',old='0x2BC04F0',target='0x2ECB130',adopted=True,
    method='R3 rule 3: unique native caller/body candidate and paired passive entry/exit results',
    abi='CHANGED four arguments to three; legacy detour forbidden',
    observation='Two distinct fields, same primary/owner, roster count 1 kind 0, users/actors 0->1, result 0 and original result pointer returned.',
    default_BinDiff=[dict(target=hex(t-old.image_base),similarity=s,confidence=c) for t,s,c in db.execute(
        'select address2,similarity,confidence from function where address1=?',(old.image_base+0x2BC04F0,))],tier_c_armed=False)

standard,=db.execute('select address2,similarity,confidence from function where address1=?',(old.image_base+0x2941020,)).fetchall()
assert standard[0]==new.image_base+0x2C17410 and standard[2]>.98 and 0x2C17410 in new.entries
vt_path=LOCAL/'analysis-tools/r2-nativefix/vt-matches.tsv'
vt_rows=[line.split('\t') for line in vt_path.read_text(encoding='utf-8').splitlines()[1:]]
phase_vt=[r for r in vt_rows if r[0].split(':')[-1].lower()=='142941020' and r[3]=='ACCEPTED']
assert all(int(r[1].split(':')[-1],16)==new.image_base+0x2C17410 for r in phase_vt)

for name in ['phase-old.txt','phase-standard-target.txt']:
    body=(ROOT/'build/r3'/name).read_text(encoding='utf-8')
    assert ' + 0x78]' in body and ' + 0x80]' in body and ' + 0x12]' in body and ' + 0x5e)' in body
rows['C099']=dict(symbol='kPhases',old='0x2941020',target='0x2C17410',adopted=True,
    method='Whole-image default BinDiff plus independent paired native consumption',similarity=standard[1],confidence=standard[2],
    accepted_VT=phase_vt,body='Four-argument ABI retained; owner component vector+78/count+80 stride16, per-component init flag+12, lifecycle phases+5E and reference/lock/error paths. Transform helper now takes an additional result pair; not reused blindly.',tier_c_armed=False)
length=len(re.findall(r'0x[0-9A-Fa-f]{2}',core['C100']['value']))
data=new.data[new.offset(0x2C17410,length):][:length]
rows['C100']=dict(symbol='kPhasesHead',old=core['C100']['value'],target=data.hex(' '),adopted=True,
    method='Exact native entry bytes for C099; target first instruction is five bytes',tier_c_armed=False)
proofs=[]
for name in ['build/r3/identity-rtti-old.json','build/r3/identity-rtti-new.json',
             'build/r3/status-native-old.txt','build/r3/status-native-new.txt','build/r3/phase-old.txt',
             'build/r3/phase-standard-target.txt','docs/probe-3663/port-evidence/r3/join-result.json',
             'docs/probe-3663/port-evidence/r2/identity-contracts-reference.txt',
             'docs/probe-3663/port-evidence/r2/identity-contracts-target.txt']:
    p=ROOT/name;proofs.append(dict(path=str(p.resolve()),sha256=hashlib.sha256(p.read_bytes()).hexdigest().upper()))
out=ROOT/'docs/probe-3663/port-evidence/r3/identity-adoptions.json'
out.write_bytes((json.dumps(dict(rows=rows,evidence=proofs,limits='C remains closed: these facts do not prove cross-user ownership, rollback or the remaining module contracts.'),indent=2)+'\n').encode('utf-8'))
header=ROOT/'src/mod/game/build_3663.h';text=header.read_text(encoding='utf-8')
begin,end='// R3 join catalog BEGIN','// R3 join catalog END'
if begin in text:text=text[:text.index(begin)]+text[text.index(end)+len(end):]
catalog=begin+'\nnamespace r3_join {\n'
for key,row in rows.items():
    if key=='C100':catalog+='inline constexpr uint8_t kPhasesHead[] = {'+', '.join(hex(b) for b in data)+'}; // C100\n'
    elif key not in ['C101','C102']:
        name=row['symbol'] if key not in ['C078','C079'] else row['symbol']
        catalog+=f'inline constexpr uintptr_t {name} = {row["target"]}; // {key}\n'
catalog+='} // namespace r3_join\n'+end+'\n'
text=text.replace('}  // namespace cdmp::game::build_3663',catalog+'\n}  // namespace cdmp::game::build_3663')
header.write_bytes(text.encode('utf-8'))
print(f'{len(rows)} new C adoptions; C remains closed; no legacy four-argument hook redirected')
