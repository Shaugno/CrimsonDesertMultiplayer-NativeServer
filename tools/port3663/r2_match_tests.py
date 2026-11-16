"""Synthetic regression checks for fail-closed evidence joins, never port proof."""
import csv
import json
from pathlib import Path
import sqlite3
import subprocess
import sys

root=Path(__file__).resolve().parents[2]
fixture=root/'build/r2/match-fixture'
fixture.mkdir(parents=True,exist_ok=True)
base=0x140000000
source,target=0x2B07790,0x2E08520
anchor,anchor_target=0xDD49D0,0xFB6F70
core=json.loads((root/'docs/probe-3663/R2-CORE-LIST.json').read_text())
field,=[r['r2'] for r in core if r['symbol']=='kAcquireField']
def run(name,vt_target,bd_target,accepted=True,root_vt=None,root_bd=None):
    folder=fixture/name
    folder.mkdir(exist_ok=True)
    database=folder/'fixture.sqlite'
    db=sqlite3.connect(database)
    db.execute('DROP TABLE IF EXISTS function')
    db.execute('CREATE TABLE function(address1 INTEGER,address2 INTEGER,similarity REAL,confidence REAL)')
    if bd_target:
        db.execute('INSERT INTO function VALUES (?,?,1,1)',(base+source,base+bd_target))
    db.execute('INSERT INTO function VALUES (?,?,1,1)',(base+anchor,base+anchor_target))
    if root_bd:
        db.execute('INSERT INTO function VALUES (?,?,1,1)',(base+0x6D69190,base+root_bd))
    db.commit();db.close()
    table=folder/'fixture.tsv'
    with table.open('w',newline='') as stream:
        writer=csv.writer(stream,delimiter='\t')
        writer.writerow(['source','destination','type','status','method','similarity','confidence'])
        if vt_target:
            writer.writerow([f'{base+source:x}',f'{base+vt_target:x}','FUNCTION',
                'ACCEPTED' if accepted else 'AVAILABLE','SYNTHETIC FIXTURE',1,1])
        writer.writerow([f'{base+anchor:x}',f'{base+anchor_target:x}','FUNCTION','ACCEPTED','SYNTHETIC FIXTURE',1,1])
        if root_vt:
            writer.writerow([f'{base+0x6D69190:x}',f'{base+root_vt:x}','DATA','ACCEPTED','SYNTHETIC CONFLICT',1,1])
    subprocess.run([sys.executable,str(root/'tools/port3663/r2_matches.py'),'--vt',str(table),
        '--bindiff',str(database),'--output-dir',str(folder)],cwd=root,check=True,capture_output=True)
    return {r['row']:r for r in json.loads((folder/'R2-MATCHES.json').read_text())['rows']}

both=run('agreement',target,target)
assert both[field]['adopted']
assert both['A031']['adopted'] and 'anchor' in both['A031']['method']
assert not both['A026']['adopted']  # no invented health ID
assert run('single-vt-plus-native',target,None)[field]['adopted']
assert not run('conflicting-bindiff',target,target+0x10)[field]['adopted']
assert not run('conflicting-vt',target+0x10,target)[field]['adopted']
assert not run('unaccepted-vt',target,None,False)[field]['adopted']
assert not run('global-vt-conflict-with-good-anchor',target,target,root_vt=0x72451C8)['A031']['adopted']
assert not run('global-bd-conflict-with-good-anchor',target,target,root_bd=0x72451C8)['A031']['adopted']
manual=fixture/'manual-rva-bypass.json'
manual.write_text(json.dumps({field:dict(target=hex(target),old=hex(source).upper().replace('X','x'),
    symbol='kAcquireField',method='untrusted manual RVA',second_check='fixture',evidence='fixture')}))
folder=fixture/'single-vt-plus-native'
attempt=subprocess.run([sys.executable,str(root/'tools/port3663/r2_matches.py'),
    '--vt',str(folder/'fixture.tsv'),'--bindiff',str(folder/'fixture.sqlite'),
    '--reviewed',str(manual),'--output-dir',str(fixture/'manual-denied')],cwd=root,capture_output=True,text=True)
assert attempt.returncode!=0 and 'Manual RVA proof cannot bypass' in attempt.stderr
print('Evidence join fixtures PASS: dual agreement, single-plus-native, operand anchor, direct/global conflicts, unaccepted VT and manual-RVA bypass rejection. Synthetic only; no docs output changed.')
