"""Seal R5 game-free checks and a versioned controlled-launch receipt."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
root=Path(__file__).resolve().parents[2]
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest().upper()
def write(p,v):Path(p).write_text(json.dumps(v,indent=2)+'\n',encoding='utf-8')
p=argparse.ArgumentParser();p.add_argument('--build-dir',type=Path,required=True)
p.add_argument('--prefix',default='step1');p.add_argument('--step',type=int,default=1);p.add_argument('--min-tests',type=int,default=116)
a=p.parse_args();assert re.fullmatch(r'[a-zA-Z0-9_-]+',a.prefix)
suite=root/f'build/r5/{a.prefix}-suite.txt';smoke=root/f'build/r5/{a.prefix}-smoke.txt'
log=root/f'build/r5/{a.prefix}-smoke-run/e2e/cdmp.log'
text=suite.read_text(encoding='utf-8-sig');total,=re.findall(r'100% tests passed, 0 tests failed out of (\d+)',text)
assert int(total)>=a.min_tests;seconds,=re.findall(r'Total Test time \(real\) =\s*([\d.]+) sec',text)
assert '3663 compiled tier mask 0xF (metadata only)' in smoke.read_text(encoding='utf-8-sig')
assert 'Mod stopped and unloaded cleanly' in smoke.read_text(encoding='utf-8-sig')
lt=log.read_text(encoding='utf-8-sig');states,=re.findall(r'remote states (\d+)',lt)
assert int(states)>0 and 'rejected 0' in lt and 'Network stopped' in lt and 'Worker exiting' in lt
binary=a.build_dir.resolve()/'bin/RelWithDebInfo';asi=binary/'cdmp.asi'
assert sha(asi)==sha(log.parent/'cdmp.asi')
paths=subprocess.check_output(['git','ls-files','-z','--cached','--others','--exclude-standard'],cwd=root).decode().split('\0')
sources=sorted(set(f for f in paths if f and (f.startswith(('src/','tests/','scripts/','tools/','cmake/','third_party/safetyhook-patch/')) or f=='CMakeLists.txt') and (root/f).is_file()))
manifest=[dict(path=f,sha256=sha(root/f)) for f in sources]
receipt=dict(version=2,round=5,compiled_tier_mask=15,executable_sha256='8BE67178D9B9A149C361A731051665E0BA4D441E250AE1DFE645BD6E524345C8',
    full_suite_passed=True,synthetic_v1_metadata_verified=True,isolation_method='profile',host_isolation='none',
    asi_path=str(asi),asi_sha256=sha(asi),pdb_path=str(binary/'cdmp.pdb'),
    watchdog_sha256=sha(root/'tools/port3663/r5_session_watchdog.ps1'),source_manifest=manifest)
write(root/'build/r5/arming-receipt.json',receipt)
extra=root/'build/r5/step1-steam-profile.txt';et=extra.read_text(encoding='utf-8-sig')
assert et.count('SteamAPI_InitFlat result 0')>=3 and 'method=profile file_hooks=0' in et
assert 'Profile case negative=True exit=60' in et and 'Profile case negative=False exit=0' in et
result=dict(round=5,step=a.step,suite=f'{total}/{total} passed',seconds=float(seconds),synthetic_remote_states=int(states),synthetic_rejects=0,
    smoke='eight-second synthetic host/peer only; tier metadata 0xF; clean unload',asi_sha256=sha(asi),
    default_route='Tier A reader',host='explicit v1; backed-up real saves; zero file sandbox hooks',
    guest='USERPROFILE set before process start; native CSIDL_LOCAL_APPDATA save root must identify verified guest copy; zero file hooks',
    fallback='explicit NT default-deny mode retained and fixture-tested, not installed in the primary profile route',
    limits=['Profile save-path isolation is not an OS permission boundary against arbitrary absolute-path writes.',
        'Fixtures exercise mutations through the actual native save resolver; the NT fallback separately tests hard-coded real-path refusal/redirection.',
        'Concurrent real-save host deltas cannot be attributed to one process by a combined manifest alone; guest-only evidence remains required.',
        'No game runtime acceptance follows from these game-free checks.'],
    arming_receipt_sha256=sha(root/'build/r5/arming-receipt.json'),
    evidence=[dict(path=str(f.resolve()),sha256=sha(f)) for f in [suite,smoke,log,extra]],
    source_manifest_sha256=hashlib.sha256(json.dumps(manifest,sort_keys=True).encode()).hexdigest().upper())
out=root/f'docs/probe-3663/port-evidence/r5/{a.prefix}-validation.json';out.parent.mkdir(parents=True,exist_ok=True);write(out,result)
print({k:result[k] for k in ['round','step','suite','seconds','synthetic_remote_states']})
