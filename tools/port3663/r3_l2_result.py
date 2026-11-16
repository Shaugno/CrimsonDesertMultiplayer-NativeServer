"""Summarise the exact R3 loopback native-host session, keeping raw logs local."""
import argparse
import hashlib
import json
from pathlib import Path
import re

parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('session',type=Path)
parser.add_argument('output',type=Path)
args=parser.parse_args()
root=args.session.resolve()
log=(root/'cdmp.log').read_text(encoding='utf-8-sig')
peer=(root/'peer.stdout.txt').read_text(encoding='utf-8-sig')
saves=json.loads((root/'save-verification.json').read_text(encoding='utf-8-sig'))
watch=json.loads((root/'watchdog.json').read_text(encoding='utf-8-sig'))
assert saves['exe_sha256']=='8BE67178D9B9A149C361A731051665E0BA4D441E250AE1DFE645BD6E524345C8'
assert 'Host version: 1.0.0.3663' in log and '0x6AF5D198' in log
requests=re.findall(r'native/captured packet opcode 2401 bytes (\d+)',log)
responses=re.findall(r'native response opcode 2128 bytes (\d+)',peer)
assert len(requests)==len(responses)==1
assert 'its buffers 1 handed 1 seen' in log
assert 'queued 1 refused 0 pending 0 halted false' in log
states=max(map(int,re.findall(r'remote states (\d+)',log)))
echoed=max(map(int,re.findall(r'real states echoed (\d+)',peer)))
assert states>0 and echoed>0 and watch['state']=='exited' and saves['backup_verified']
assert not re.search(r'\] ERR |EXCEPTIONS|halted true',log)
files=[]
for name in ['cdmp.log','peer.stdout.txt','peer.stderr.txt','question.md','deployed-hashes.json',
             'installed-receipt.json','save-verification.json','watchdog.json','session-profile.json','peer-process.json']:
    p=root/name
    files.append(dict(path=str(p),bytes=p.stat().st_size,sha256=hashlib.sha256(p.read_bytes()).hexdigest().upper()))
result=dict(session='R3-L2-'+root.name.removeprefix('R2-P1-'),controller_session=root.name,result='passed',
    exe_sha256=saves['exe_sha256'],native_request=dict(opcode=2401,bytes=int(requests[0]),handed=1,seen=1),
    native_response=dict(opcode=2128,bytes=int(responses[0]),peer_received=1),
    movement=dict(host_remote_states=states,peer_real_states_echoed=echoed,synthetic=False),
    rejection_count=max(map(int,re.findall(r'Net:.*?rejected (\d+)',log))),
    user_confirmation='done (in response to Continue, walk/turn, fast travel, walk/turn, quit)',
    initialization='Boundary objects initially had the base Troc vtable; derived named descriptors became ready before hook installation. 514 handlers found; 513 blocked, only named cryptogram admitted.',
    termination='normal process exit; independent watchdog state exited, not deadline-stopped',
    save_verification=saves,installation_restored=True,
    native_scope='Private session constructor/fields, owned packet-buffer allocation, native Q0 push, tagged receiver session argument, native send and Q2 return. No login/world/save request or actor creation.',
    networking='Native host, UDP initiator to explicit loopback peer; Steam/friends code unchanged',
    limitations=['Not two-player evidence','Native guest join and reconnect remain gated by C/D',
                 'Native process exit verified; no claim of D session/actor teardown or an in-process Network stopped log',
                 'Only opcode/length/counters logged; raw packet payloads not retained'],files=files)
args.output.write_bytes((json.dumps(result,indent=2)+'\n').encode('utf-8'))
print(f'L2 PASS: native exchange, {states} real states, normal exit, backups verified')
