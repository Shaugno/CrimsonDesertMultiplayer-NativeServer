"""Merge historical R2 facts with approved R3 dispositions; never arm a tier."""
from collections import Counter
import json
from pathlib import Path
import re

ROOT=Path(__file__).resolve().parents[2]
DOCS=ROOT/'docs/probe-3663'
core=json.loads((DOCS/'R2-CORE-LIST.json').read_text(encoding='utf-8'))
historical={r['row']:r for r in json.loads((DOCS/'R2-MATCHES.json').read_text(encoding='utf-8'))['rows']}
assert len(core)==len(historical)==395
rows={}
for row in core:
    fact=historical[row['r2']]
    rows[row['r2']]=dict(row=row['r2'],tier=row['tier'],symbol=row['symbol'],
        disposition='adopted' if fact['adopted'] else 'blocked',target=fact.get('target'),
        authority='R2-MATCHES.json',missing='' if fact['adopted'] else fact['missing'])
def adopt(key,fact,authority):
    assert key in rows and fact['adopted']
    rows[key].update(disposition='adopted',target=fact.get('target'),authority=authority,missing='')
for name in ['descriptor-bounds.json','recipient-adoptions.json','identity-adoptions.json']:
    for key,fact in json.loads((DOCS/'port-evidence/r3'/name).read_text(encoding='utf-8'))['rows'].items():
        adopt(key,fact,'R3 '+name)
adopt('B065',json.loads((DOCS/'port-evidence/r3/pump-adoption.json').read_text(encoding='utf-8')),'R3 pump-adoption.json')
for key,target in [('B075','outer 0x7FDA; ordinary payload 0x7FC1'),('B078','extra skip 0; aligned scan increment 8')]:
    adopt(key,dict(adopted=True,target=target),'R3 packet-scan-adoptions.json')
for key,disposition,target in [('A026','approved_policy','native definition consensus; no guessed literal ID'),
    ('A027','retired_by_scope','3663 stamina unreachable'),
    ('A028','approved_policy','park/changed chain/missing HP definitions => not ready')]:
    rows[key].update(disposition=disposition,target=target,authority='User-approved R3 rule 1 + successful L1',missing='')
updates={
    'C047':'Accepted VT/native AddActor identifies candidate 4CAE10; default BinDiff instead maps legacy 40B5B0 to 14AD670 (confidence 0.952574). Resolve the generic-helper conflict and its native allocation/growth ownership; named-RTTI rule 2 does not settle it. The passive trace did not instrument this helper.',
    'C066':'Roster kind 0 and observed primary key A0100001 do not prove the separate legacy character-class constant 0xA in its actual producer/consumer role.',
    'C070':'Lobby parent 2E879A0 is now adopted. Candidate constant site 2E87DB0 still needs the target stack-slot/argument lifetime, exact relocated instruction and safe per-build patch semantics; the old instruction patch must not be reused.',
    'C072':'Lobby parent is adopted, but candidate call 2E87E10 to entry writer 237EA60 still lacks a proved target callsite detour ABI/argument ownership and safe guard/relocation strategy.',
    'C110':'Legacy standalone Travel 2BC0A30 is inlined into target transition 2ECA480. Need a new verified publication adapter; 2ECB660 is roster-position preparation, not this function.',
    'C111':'The old standalone Travel entry guard cannot be redirected. Establish an exact target midpoint guard/relocation and native calling context for the inlined publication adapter.',
    'C112':'Native owner comparison is at target 2ECAA97 inside 2ECA480; prove the guest ownership predicate and safe suppression context rather than copying the legacy compare patch.',
    'C113':'Establish live guard bytes/relocation and register/flags semantics for the new inlined owner-comparison point; no legacy compare bytes are reused.',
    'C114':'Target time publication is inlined near 2ECAB4C into bank 6F31C98. Need the exact target getter/publication context and safe adapter, not a guessed standalone callsite.',
    'C115':'The old E8 callsite bytes do not establish the new inlined publication ABI/guard. Verify the replacement point and original instruction effects.',
    'C144':'The observed native ServerActor owner at +A0 is also +A0 in the paired reference AddActor body. It does not prove the separate legacy Actor_OwnerUser+90 contract; identify its actual object type/consumer and target member.',
    'D130':'Observed positive title predicate is implemented/tested on owned data. Native parser remains blocked on coherent slot+68/count+70 ID-vector ownership, available IDs at 71AEAA0/count71AEAA8, producer 14AA720 and copy/clear lifetime. Do not substitute the broader LoadSlot domain or the old inverted callback.'}
for key,missing in updates.items():
    assert rows[key]['disposition']=='blocked'
    rows[key]['missing']=missing
header=(ROOT/'src/mod/game/build_3663.h').read_text(encoding='utf-8')
flags={tier:re.search(r'kTier'+tier+r'Complete = (true|false);',header)[1]=='true' for tier in 'ABCD'}
counts={}
for tier in 'ABCD':
    selected=[r for r in rows.values() if r['tier']==tier]
    states=Counter(r['disposition'] for r in selected)
    counts[tier]=dict(total=len(selected),closed=sum(r['disposition']!='blocked' for r in selected),
        blocked=states['blocked'],dispositions=dict(states),armed=flags[tier])
    assert not flags[tier] or not states['blocked'],f'Armed {tier} has blocked rows'
assert not flags['B'] or flags['A']
assert not flags['C'] or flags['B']
assert not flags['D'] or flags['C']
result=dict(baseline='69075db',rows=list(rows.values()),tiers=counts,
    limitations=['Closed rows include approved policies and one scope retirement, not only native RVA adoptions.',
        'A/B armed; B is the explicit native-host loopback diagnostic. Native guest join and reconnect remain closed.',
        'C additionally requires the real reference/insertion/rollback backend; pure policy tests are not native proof.',
        'D130 predicate logic is tested offline; native data lifetimes/parser remain blocked.'])
(DOCS/'R3-STATUS.json').write_bytes((json.dumps(result,indent=2)+'\n').encode('utf-8'))
md=['# R3 exact remaining contracts','',
    'Merged R2 history plus approved R3 dispositions. Historical R2 files are unchanged.',
    'Closed rows include policy decisions and a scope retirement; this is not a count of native addresses.','',
    '| Tier | Closed / total | Blocked | Armed |','| --- | ---: | ---: | --- |']
for tier,c in counts.items():md.append(f'| {tier} | {c["closed"]} / {c["total"]} | {c["blocked"]} | {c["armed"]} |')
md+=['','The native C backend and guest-save/lifecycle D backend are also incomplete.',
     'See R3-JOIN-ADAPTER.md and R3-STEP4.md; these are blockers, not a request for review approval.','']
for tier in 'CD':
    md+=[f'## Tier {tier}','','| Row | Symbol | Missing fact |','| --- | --- | --- |']
    for row in rows.values():
        if row['tier']==tier and row['disposition']=='blocked':
            missing=row['missing'].replace('|','/').replace('\n',' ')
            if row['row']=='D130':missing+=' Pure predicate logic now exists; coherent native ownership/parser is still missing.'
            md.append(f'| {row["row"]} | {row["symbol"]} | {missing} |')
    md.append('')
(DOCS/'R3-BLOCKERS.md').write_bytes(('\n'.join(md)+'\n').encode('utf-8'))
print(json.dumps(counts,indent=2))
