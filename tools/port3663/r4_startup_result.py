"""Seal failed startup trials and game-free diagnoses; no launch or memory attach."""
import hashlib
import json
from pathlib import Path

root=Path(__file__).resolve().parents[2]
local=Path(r'C:\Users\shaug\.codex\visualizations\2026\11\14\01a1dbbd-2726-7441-9a57-1b454819b3e7\r4-live')
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest().upper()
def read(path):return json.loads(path.read_text(encoding='utf-8-sig'))
trials=[]
for name in ['R4-L3-20261116-184248','R4-L3-20261116-184928','R4-L3-20261116-191955']:
    folder=local/name
    launch=read(folder/'launch-host.json');watch=read(folder/'watchdog-host.json');finish=read(folder/'finish.json')
    assert watch['game_pid']==launch['game_pid'] and watch['state']=='exited'
    assert finish['restored'] and finish['real_saves_unchanged']
    assert not (folder/'launch-guest.json').exists()
    files=['question.md','session.json','launch-host.json','watchdog-host.json','finish.json','evidence/cdmp.host.log']
    if (folder/'manual-stop.json').exists():files.append('manual-stop.json')
    trials.append(dict(session=name,host=launch,watchdog=watch,
        result=('Steam initialized; guarded host active; crash before title/guest' if name.endswith('191955') else
                'SteamAPI startup failure before native join; no guest launch'),
        installation_restored=True,real_saves_unchanged=True,
        evidence=[dict(path=str(folder/f),sha256=sha(folder/f)) for f in files]))
steam=root/'build/r4/steam-stack-fixed-results.txt'
text=steam.read_text(encoding='utf-8-sig')
assert text.count('SteamAPI_InitFlat result 0')==2 and 'steam-sandbox exit 0' in text
assert 'verified=1 redirected=2 refused=0' in text
assert 'real manifest unchanged=False' not in text
proofs=['build/r4/ipc-repro-test.txt','build/r4/teardown-repro-test.txt',
    'build/r4/teardown-fixed-suite.txt','build/r4/pipe-repro-test.txt',
    'build/r4/pipe-root-test.txt','build/r4/steam-stack-fixed-results.txt',
    'build/r4/startup2-crash-summary.json','build/r4/startup3-crash-summary.json',
    'build/r4/stack-repro-test.txt','build/r4/stack-fixed-suite.txt','build/r4/stack-fixed-smoke.txt']
result=dict(step=5,trials=trials,controlled_startup_trials=3,startup_crashes=2,
    failed_L3_experiments=3,live_attempt_budget_used=3,live_attempt_budget_limit=3,
    budget_policy='Count all three failed L3 startup experiments, including the first stalled Exit, toward the plan cap; zero native guest joins is not permission for additional launches.',
    native_join_attempts=0,failed_native_join_attempts=0,guest_launches=0,
    L3='not established; live attempt cap reached',L4='not attempted; requires L3',
    fixed_offline=['MAP-only pagefile shared memory classification',
        'named pipe device path and typed empty-relative-name pipe reopen',
        'native hook removal preserves execute permission for both patch ranges',
        'small-stack mapping callback reproduction: move two nested 64 KB name buffers to heap'],
    steam_fixture='after Steam restart and pipe corrections: success with/without hooks; zero sandbox refusals',
    caveat='The third crash is consistent with reproduced callback stack exhaustion. The correction has not been retried in the real game. Game-free corrections do not establish runtime or two-player acceptance.',
    evidence=[dict(path=str(root/f),sha256=sha(root/f)) for f in proofs])
out=root/'docs/probe-3663/port-evidence/r4/startup-trials.json'
out.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8')
print({k:result[k] for k in ['controlled_startup_trials','startup_crashes','native_join_attempts','steam_fixture']})
