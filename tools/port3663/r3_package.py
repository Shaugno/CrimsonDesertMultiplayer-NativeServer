"""Build the R3 Step 5 review manifest and prepend the current handoff.

Raw session/decompiler data stays local. Referenced hashes are verified before
publishing the package. The packaging commit itself is identified by branch
HEAD; a manifest cannot contain its own commit/content hash.
"""
import hashlib
import json
from pathlib import Path
import subprocess

ROOT=Path(__file__).resolve().parents[2]
DOCS=ROOT/'docs/probe-3663'
def git(*args):
    return subprocess.check_output(['git',*args],cwd=ROOT).decode('utf-8').strip()
def read(name):return json.loads((DOCS/name).read_text(encoding='utf-8-sig'))
def digest(path):return hashlib.sha256(path.read_bytes()).hexdigest().upper()
assert git('branch','--show-current')=='gpt/3663-port'
subprocess.run(['git','merge-base','--is-ancestor','69075db','HEAD'],cwd=ROOT,check=True)
status=read('R3-STATUS.json')
assert {k:v['armed'] for k,v in status['tiers'].items()}==dict(A=True,B=True,C=False,D=False)
assert status['tiers']['C']['blocked']==20 and status['tiers']['D']['blocked']==21
source_head=git('rev-parse','HEAD')
commits=[dict(commit=line.split('\t',1)[0],subject=line.split('\t',1)[1])
         for line in git('log','--reverse','--format=%H%x09%s','69075db..HEAD').splitlines()]

cache={};verified={};unavailable=[]
corrections=read('port-evidence/r3/evidence-corrections.json')['corrections']
exceptions={str(Path(item['path']).resolve()):item for item in corrections}
def inspect(value):
    if isinstance(value,dict):
        if isinstance(value.get('path'),str) and isinstance(value.get('sha256'),str):
            path=Path(value['path'])
            if not path.is_absolute():path=ROOT/path
            key=str(path.resolve())
            if key not in cache:cache[key]=digest(path)
            if cache[key]!=value['sha256'].upper():
                correction=exceptions.get(key)
                assert correction and correction['historical_sha256']==value['sha256'].upper() and \
                    correction['observed_sha256']==cache[key],f'Evidence changed: {path}'
                if correction not in unavailable:unavailable.append(correction)
            else:verified[key]=cache[key]
        for item in value.values():inspect(item)
    elif isinstance(value,list):
        for item in value:inspect(item)
for path in sorted((DOCS/'port-evidence/r3').glob('*.json')):inspect(json.loads(path.read_text(encoding='utf-8-sig')))

sessions=[]
for name,question in [
    ('l1-result.json','Does the HP-only production reader track pose and changing HP through load/travel?'),
    ('pump-result.json','Does the candidate pump expose the expected ClientNet/session/queue context?'),
    ('l2-refusal-result.json','Does the native private-session handshake and movement stream work?'),
    ('l2-result.json','Does the native private-session handshake and movement stream work?'),
    ('join-result.json','What are the original acquisition/entry roster, owners, results and travel/lobby order?')]:
    result=read('port-evidence/r3/'+name)
    runtime=result.get('runtime_log')
    if runtime is None:runtime=next(f for f in result['files'] if Path(f['path']).name=='cdmp.log')
    sessions.append(dict(session=result['session'],controller_session=result['controller_session'],question=question,
        result=result['result'],identity_sha256='8BE67178D9B9A149C361A731051665E0BA4D441E250AE1DFE645BD6E524345C8',
        log_sha256=runtime['sha256'],log_path=runtime['path'],result_file='port-evidence/r3/'+name))
assert read('port-evidence/r3/l1-result.json')['result']=='passed'
assert read('port-evidence/r3/l2-result.json')['result']=='passed'
checks=read('port-evidence/r3/step4-validation.json')
assert checks['full_suite']['passed']==106 and checks['full_suite']['failed']==0
cleanup_path=ROOT/'build/r3/final-cleanup.json'
cleanup=json.loads(cleanup_path.read_text(encoding='utf-8-sig'))
assert cleanup['game_closed'] and not any(f['present'] for f in cleanup['deployment_files'])

changed=set(git('diff','--name-only','69075db..HEAD').splitlines())
changed.update(git('ls-files','--others','--exclude-standard').splitlines())
exclude={'docs/probe-3663/HANDOFF-CURRENT.md','docs/probe-3663/R3-REVIEW-PACKAGE.json'}
inventory=[]
for name in sorted(changed-exclude):
    path=ROOT/name
    if not path.is_file():continue
    assert path.suffix.lower() not in {'.exe','.dll','.asi','.pdb','.png','.bin'},f'Unexpected binary: {name}'
    text=path.read_text(encoding='utf-8-sig')
    inventory.append(dict(path=name,bytes=path.stat().st_size,sha256_raw=digest(path),
        sha256_utf8_lf=hashlib.sha256(text.encode('utf-8')).hexdigest().upper()))
manifest=dict(package='R3 Step 5',baseline='69075db',source_head=source_head,
    packaging_commit='The subsequent Step 5 commit at gpt/3663-port HEAD; excluded from its own manifest',
    objective='Native two-player prototype not reached; R3 conditional A/B deliverable achieved',
    steps={'1':'complete; A armed and L1 passed','2':'complete; guarded native-host loopback B and L2 passed',
           '3':'blocked; 130/150 contracts adopted, passive subset/pure policy done, native backend absent',
           '4':'blocked; 109/130 contracts adopted, title predicate policy done, save/lifecycle backend absent',
           '5':'review package ready'},tiers=status['tiers'],validation=checks,
    sessions=sessions,cleanup=cleanup,commits_since_69075db=commits,
    verified_local_evidence=[dict(path=k,sha256=v) for k,v in sorted(verified.items())],
    unavailable_historical_evidence=unavailable,
    source_and_summary_inventory=inventory,
    limits=['B is an explicit loopback-only native-host cryptogram diagnostic; guest join/Steam runtime/reconnect unproved',
            'No L3/L4, second-PC test, remote actor creation/removal or two-player evidence',
            'Pure C/D policies are not native implementation or runtime proof',
            'No binaries, new raw decompiler dumps, saves or screenshots in this package'])
(DOCS/'R3-REVIEW-PACKAGE.json').write_bytes((json.dumps(manifest,indent=2)+'\n').encode('utf-8'))

lines=['# R3 Step 5 — review package (2026-11-16)','',
    'Branch: `gpt/3663-port`. Source through `'+source_head+'`; the subsequent',
    'Step 5 packaging commit is branch HEAD. This section supersedes the R2 status below.',
    '**A/B are armed; L1 and L2 passed. C/D stay closed. The native two-player goal is not reached.**','',
    'The approved conditional R3 deliverable is complete: working Steps 1–2,',
    'independent C/D work and exact blockers, followed by this Step 5 package.',
    'No intermediate review approval is requested. `R3-REVIEW-PACKAGE.json` has',
    'the source/summary inventory, verified local evidence hashes and commits;',
    '`R3-STATUS.json` and `R3-BLOCKERS.md` retain every remaining contract.','',
    '| Tier | Closed / total | Armed | Current behavior |','| --- | ---: | --- | --- |',
    '| A | 37/37 | Yes | HP-only reader; definition consensus and park/chain/readiness guards |',
    '| B | 78/78 | Yes | Explicit loopback native-host cryptogram tunnel and real pose streaming |',
    '| C | 130/150 | No | Named/traced facts and three-argument pure guard/rollback policy; no native join backend |',
    '| D | 109/130 | No | Complete observed title membership policy on owned data; no native guest-save/lifecycle backend |','',
    'Closed rows include two approved A policies and one stamina scope retirement,',
    'not 354 adopted native addresses. R2 evidence/history remains unchanged.','',
    '## Validation and current installation','',
    '- `scripts/build.ps1 -BuildDir <external r2-build>`: **106/106**, 98.33 seconds.',
    '- Final `tools/port3663/smoke.ps1`: '+str(checks['loopback']['remote_states'])+' synthetic remote states and clean unload; game-free evidence only.',
    '- Guard matrix covers exact 3663 identity/dependencies, wrong timestamp/size/base, unknown builds, 3576 denial and preserved 2976/test-host paths.',
    '- New tests cover target packet bounds, reliable handshake shape, trace recursion/unwinding, three-argument type/owner/roster/rollback refusal and bounded metadata membership.',
    '- Game closed. Test ASI/PDB/loader/INI removed. Every R3 live session verified backups and unchanged saves (23 files each). No saves were restored over player progress.',
    '- First L2 launch safely refused before hooks because derived Troc descriptors were not ready. Bounded named readiness fixed that timing; retry passed. Earlier fixture failures and corrections remain recorded.',
    '- Evidence archival correction: one old pump smoke wrapper transcript is now empty (cause unestablished). Its earlier 153-byte hash is preserved but not counted as retained proof. The matching immutable ASI host log and final suite/smoke remain verified; see evidence-corrections.json.',
    '- Pump cleanup initially hit OneDrive CLOUD placeholders. Exact hashes were recovered with the read-only verifier; subsequent R3 backups are outside OneDrive. General no-link guards remain.',
    '- Supported target: executable 1.0.0.3663, timestamp `0x6AF5D198`, base `0x140000000`, size `0x144E9000`.',
    '- Target SHA256: `8BE67178D9B9A149C361A731051665E0BA4D441E250AE1DFE645BD6E524345C8`. Rechecked at final cleanup.','',
    '## Live evidence','',
    'Each session used presence, a written question, exact identity, verified save',
    'backup, pinned fourteen-minute watchdog and removal. Full questions/receipts',
    'and log paths are in the result JSON/manifest. No UI control was used.','']
for session in sessions:
    lines += ['- **'+session['session']+' — '+session['result']+'**. '+session['question'],
              '  Log SHA256: `'+session['log_sha256']+'`; result: `'+session['result_file']+'`.']
lines += ['',
    'L1 logged HP 900 → 1.8 and regeneration with user-confirmed damage/travel;',
    'no exact post-damage screen value was supplied. Pump recorded 125528 matching',
    'contexts. L2 copied one real request 2401 (291 bytes), injected it once into',
    'the private native session, and received native Ack 2128 (330 bytes) at the',
    'peer; 1044 real pose samples returned, no rejections. Native host used UDP',
    'initiation toward the loopback peer; this is not a native guest client or',
    'Steam/two-player test. Normal process exit was verified; D teardown was not.',
    'The field trace captured 16 valid records/eight pairs through two distinct',
    'fields. Both single-primary admissions returned success and users/actors',
    '0→1. No guest insertion, factory/failure path, native refcount or rollback',
    'was measured. Status/login-manager/phase sites were not instrumented.','',
    '## Changes, blockers and next verification','',
    '`R3-STEP2.md` describes the shared per-build tunnel, guarded queue/pump',
    'route and C/D isolation. `R3-JOIN-ADAPTER.md` defines the new three-argument',
    'transaction, reference/lock order and rollback contract. Its pure policy',
    'is tested but the native insertion/reference/rollback backend is absent.',
    'Nine additional C facts were adopted after the eight recipient rows:',
    'named lobby/status tables and slot 10, traced EnterField, and phase entry',
    '2C17410/new guard. Generic VecPush still conflicts (VT/native callee 4CAE10',
    'versus default BinDiff 14AD670); the named-RTTI exception does not settle it.',
    'Ordinary server owner+A0 and roster kind 0 do not prove the separate C144',
    'Actor_OwnerUser+90 or C066 class contracts. Travel clock publication is now',
    'inlined and cannot reuse the old standalone detour. **20 C rows remain.**','',
    '`R3-STEP4.md` records the independent title eligibility implementation.',
    'D130 still needs coherent native metadata-vector ownership/parser, alongside',
    'early save interception, title/save/lock and lifecycle contracts. **21 D',
    'rows remain.** The exact symbols and missing facts are in `R3-BLOCKERS.md`.',
    'Guest-role target startup remains denied; L3/L4 were not attempted and the',
    'second PC was not requested while these prerequisites were missing.','',
    'Manual verification after further implementation: first rerun the full',
    'game-free suite and smoke. Prove the missing C helper/reference/rollback',
    'backend and guest identity/travel paths before arming C. Complete early D',
    'guest save protection before any two-instance launch. Then use fresh',
    'separate save backups/presence/watchdogs for L3 native walking/turning,',
    'disconnect removal/reconnect and unchanged host saves; L4 requires the',
    'second legitimate PC over Steam friends and a load/travel on each side.',
    'Do not promote loopback/policy tests into two-player evidence.','',
    '## Commits since 69075db','',
    'Source/evidence commits through '+source_head+' are below. Run',
    '`git log 69075db..gpt/3663-port` to include this subsequent packaging commit.','']
for commit in commits:lines.append('- `'+commit['commit'][:7]+'` '+commit['subject'])
lines += ['',
    'Original ByJunter attribution, history and GPL v3 preserved. Sibling',
    'original checkout and parent Start-Grok.ps1 untouched. No binaries/release,',
    'saves, new game screenshots or raw new decompiler dumps published.','',
    '<!-- R3 PACKAGE END -->','',
    '---','',
    'Historical R2 package follows; its all-tiers-off state is superseded above.','']
handoff=DOCS/'HANDOFF-CURRENT.md'
old=handoff.read_text(encoding='utf-8')
if '<!-- R3 PACKAGE END -->' in old:
    old=old.split('Historical R2 package follows; its all-tiers-off state is superseded above.\n',1)[1].lstrip()
handoff.write_bytes(('\n'.join(lines)+'\n'+old).encode('utf-8'))
print(f'R3 package: {len(inventory)} source/summary files, {len(verified)} verified local evidence files, {len(commits)} commits; A/B passed, C/D blocked')
