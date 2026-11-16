"""Validate paired passive field records without retaining raw addresses in Git."""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import re

p=argparse.ArgumentParser(description=__doc__)
p.add_argument('session',type=Path);p.add_argument('output',type=Path)
args=p.parse_args();root=args.session.resolve()
log=(root/'cdmp.log').read_text(encoding='utf-8-sig')
pattern=r'R3Join: seq (\d+) site (\d+) thread (\d+) valid (\w+) args ([0-9A-F,]+) return ([0-9A-F]+) result ([0-9A-F]+) field ([0-9A-F]+) field_owner ([0-9A-F]+) users (\d+) actors (\d+) primary ([0-9A-F]+) key ([0-9A-F]+) owner ([0-9A-F]+) owner_key ([0-9A-F]+) roster (\d+)'
events=[]
for m in re.finditer(pattern,log):
    v=m.groups()
    events.append(dict(seq=int(v[0]),site=int(v[1]),thread=int(v[2]),valid=v[3]=='true',
        args=[int(x,16) for x in v[4].split(',')],returned=int(v[5],16),result=int(v[6],16),
        field=int(v[7],16),field_owner=int(v[8],16),users=int(v[9]),actors=int(v[10]),
        primary=int(v[11],16),key=int(v[12],16),owner=int(v[13],16),owner_key=int(v[14],16),count=int(v[15])))
assert events and all(e['valid'] for e in events)
assert not re.search(r'R3Join: collection stopped|\] ERR ',log)
counts=Counter(e['site'] for e in events)
assert all(counts[i]==2 for i in range(8))
stacks={};pairs=[]
for e in events:
    key=(e['thread'],e['site']//2)
    if e['site']%2==0:stacks.setdefault(key,[]).append(e)
    else:
        before=stacks[key].pop()
        assert e['args']==before['args'] and e['returned']==before['args'][1] and e['result']==0
        pairs.append((before,e))
assert all(not stack for stack in stacks.values())
rosters={}
for seq,index,kind,key in re.findall(r'R3Join: seq (\d+) roster (\d+) kind (\d+) key ([0-9A-F]+)',log):
    rosters.setdefault(int(seq),[]).append((int(index),int(kind),int(key,16)))
entries=[]
for before,after in pairs:
    if before['site']!=2:continue
    assert before['field']==after['field'] and before['count']==after['count']==1
    assert rosters[before['seq']]==rosters[after['seq']]==[(0,0,before['key'])]
    assert before['primary']==after['primary'] and before['owner']==after['owner']
    assert before['field_owner']==after['field_owner']==before['owner_key']==after['owner_key']
    assert (before['users'],before['actors'],after['users'],after['actors'])==(0,0,1,1)
    entries.append(dict(entry_seq=before['seq'],exit_seq=after['seq'],roster_count=1,roster_kind=0,
        primary_key=hex(before['key']),owner_key=hex(before['owner_key']),roster_matches_primary=True,
        primary_owner_matches_field_owner=True,users=[0,1],actors=[0,1],result=0,returns_result_pointer=True))
entered=[e for e in events if e['site']==2]
assert len(entered)==2 and entered[0]['field']!=entered[1]['field']
saves=json.loads((root/'save-verification.json').read_text(encoding='utf-8-sig'))
watch=json.loads((root/'watchdog.json').read_text(encoding='utf-8-sig'))
assert saves['backup_verified'] and watch['state']=='exited'
records=[]
for name in ['cdmp.log','question.md','installed-receipt.json','deployed-hashes.json','save-verification.json','watchdog.json','session-profile.json']:
    f=root/name;records.append(dict(path=str(f),bytes=f.stat().st_size,sha256=hashlib.sha256(f.read_bytes()).hexdigest().upper()))
result=dict(session='R3-JOIN-'+root.name.removeprefix('R2-P1-'),controller_session=root.name,
    result='passive_subset_passed',exe_sha256=saves['exe_sha256'],events=len(events),site_counts=dict(counts),
    call_order=[e['site'] for e in events],entry_exit_pairs=len(pairs),all_records_valid=True,
    native_data_writes=False,native_calls=False,context_changes=False,networking=False,
    enter_abi='Three arguments: mover, result, field. Recorded R9 is incidental hardware context, never a fourth companion argument.',
    enter_results=entries,same_primary_and_owner_across_fields=True,distinct_fields=2,
    user_confirmation='done (in response to Continue, pause, fast travel, pause, walk/turn, quit)',
    termination='normal exit; watchdog exited',save_verification=saves,installation_restored=True,
    limitations=['No guest/second-user insertion or failure rollback exercised',
        'No companion, missing-actor factory branch, native reference count or acquired new-field flag sampled',
        'Status/login-manager/add-actor thunks were omitted under candidate-entry rule',
        'ServerActor+A0 owner observations do not prove the separate legacy Actor_OwnerUser+90 contract C144',
        'Field counts are guarded diagnostic reads, not a lock-held transaction'],files=records)
args.output.write_bytes((json.dumps(result,indent=2)+'\n').encode('utf-8'))
print(f'Passive subset PASS: {len(events)} records, {len(pairs)} pairs, two distinct fields; C still closed')
