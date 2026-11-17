"""Separate persistent R5 startup/join budgets. No process launch or game access."""
import argparse
from datetime import datetime,timezone
import json
from pathlib import Path
import re
root=Path(__file__).resolve().parents[2];file=root/'build/r5/attempts.json'
p=argparse.ArgumentParser();p.add_argument('action',choices=['status','begin','promote','finish'])
p.add_argument('--kind',choices=['startup','prejoin','join']);p.add_argument('--id');p.add_argument('--session');p.add_argument('--outcome');p.add_argument('--note',default='')
a=p.parse_args()
state=json.loads(file.read_text(encoding='utf-8')) if file.exists() else dict(round=5,baseline='756fcd5',startup_limit=5,join_limit=3,attempts=[])
assert state['round']==5
def counts():return {kind:sum(1 for e in state['attempts'] if (e['kind'] in ('startup','prejoin') if kind=='startup' else e['kind']=='join')) for kind in ['startup','join']}
if a.action=='begin':
    assert a.kind and a.id and a.session and re.fullmatch(r'[A-Za-z0-9_-]+',a.id)
    assert not any(e['id']==a.id for e in state['attempts'])
    category='startup' if a.kind in ('startup','prejoin') else 'join';assert counts()[category]<state[category+'_limit'],'R5 budget exhausted'
    state['attempts'].append(dict(id=a.id,kind=a.kind,session=a.session,outcome='active',started_utc=datetime.now(timezone.utc).isoformat(),note=a.note))
elif a.action in ('promote','finish'):
    e,=[e for e in state['attempts'] if e['id']==a.id]
    if a.action=='promote':
        assert e['kind']=='prejoin' and counts()['join']<state['join_limit']
        e['kind']='join';e['promoted_utc']=datetime.now(timezone.utc).isoformat()
    else:
        assert a.outcome;e['outcome']=a.outcome;e['finished_utc']=datetime.now(timezone.utc).isoformat();e['note']=a.note
if a.action!='status' or not file.exists():
    file.parent.mkdir(parents=True,exist_ok=True);temp=file.with_suffix('.tmp');temp.write_text(json.dumps(state,indent=2)+'\n',encoding='utf-8');temp.replace(file)
print(json.dumps(dict(round=5,used=counts(),limits=dict(startup=5,join=3),attempts=state['attempts']),indent=2))
