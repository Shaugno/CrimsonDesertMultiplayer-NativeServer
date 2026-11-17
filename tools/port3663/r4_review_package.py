"""Assemble Step 6 from retained proofs; never launch/attach/deploy anything."""
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import subprocess

root=Path(__file__).resolve().parents[2]
docs=root/'docs/probe-3663'
source='cce29cc9db01c38fcd6fa8d1b1c71bfbf951652c'
def read(path):return json.loads(Path(path).read_text(encoding='utf-8-sig'))
def sha(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest().upper()
def git(*args):return subprocess.check_output(['git',*args],cwd=root).decode().strip()
def proof(entry):
    path=Path(entry['path'])
    assert path.is_file(),f'Missing evidence: {path}'
    assert sha(path)==entry['sha256'],f'Evidence changed: {path}'
    return entry
assert git('branch','--show-current')=='gpt/3663-port'
assert git('rev-parse',source)==source
validation=read(docs/'port-evidence/r4/stack-fixed-validation.json')
attempts=read(docs/'port-evidence/r4/startup-trials.json')
status=read(docs/'R4-STATUS.json')
passive=read(docs/'port-evidence/r4/passive-result.json')
cleanup=read(root/'build/r4/final-cleanup.json')
assert validation['suite']=='114/114 passed' and validation['synthetic_rejects']==0
assert attempts['live_attempt_budget_used']==attempts['live_attempt_budget_limit']==3
assert attempts['native_join_attempts']==attempts['guest_launches']==0
assert cleanup['game_process_count']==0 and not any(x['exists'] for x in cleanup['files'])
assert cleanup['save_verification']['real_manifest_unchanged'] and cleanup['save_verification']['backup_manifests_verified']
retained=[]
for item in validation['evidence']+attempts['evidence']+passive['local_evidence']:
    retained.append(proof(item))
for trial in attempts['trials']:
    retained.extend(proof(e) for e in trial['evidence'])
    folder=Path(trial['evidence'][0]['path']).parent
    for name in ['deployment.json','save-manifest.tsv','verification.json']:
        retained.append(proof(dict(path=str(folder/name),sha256=sha(folder/name))))
    deployment=read(folder/'deployment.json')
    record_name={'R4-L3-20261116-184248':'step4-result.json',
        'R4-L3-20261116-184928':'ipc-fixed-validation.json',
        'R4-L3-20261116-191955':'startup-fixed-validation.json'}[trial['session']]
    validated=read(docs/'port-evidence/r4'/record_name)
    assert deployment['arming_sha256']==validated['arming_receipt_sha256']
    deployed_asi,=[e for e in deployment['files'] if e['name']=='cdmp.asi']
    assert deployed_asi['installed_sha256']==validated['asi_sha256']
    trial['deployed_files']=deployment['files']
    trial['binary_validation_record']='port-evidence/r4/'+record_name
    trial['executable_identity']=dict(version='1.0.0.3663',timestamp='0x6AF5D198',
        base='0x140000000',image_size='0x144E9000',sha256=cleanup['executable_sha256'])
# Step 3 source hashes belong to commit 43a49f5. Only its immutable local
# build/smoke proofs are compared to present files, not superseded source.
for e in read(docs/'port-evidence/r4/step3-result.json')['evidence']:
    if '/build/' in e['path'].replace('\\','/').lower():retained.append(proof(e))
receipt=read(root/'build/r4/arming-receipt.json')
assert sha(root/'build/r4/arming-receipt.json')==validation['arming_receipt_sha256']
assert sha(receipt['asi_path'])==validation['asi_sha256']
metadata_changes=[]
for e in receipt['source_manifest']:
    actual=sha(root/e['path'])
    if actual!=e['sha256']:
        # This script updates only the report's Step 6 state after the tested
        # source checkpoint. No compiled code or live launcher may differ.
        assert e['path']=='tools/port3663/r4_status.py',f'Tested source changed: {e["path"]}'
        metadata_changes.append(dict(path=e['path'],validated_sha256=e['sha256'],packaging_sha256=actual,
            reason='report-only Step 6 completion label; not compiled/runtime code'))
changed=git('diff','--name-only','0f11f6e',source).splitlines()
assert not any(Path(p).suffix.lower() in ('.asi','.exe','.dll','.pdb','.dmp','.png','.avi','.mp4') for p in changed)
commits=[]
for line in git('log','--reverse','--format=%H%x09%s',f'0f11f6e..{source}').splitlines():
    commit,title=line.split('\t',1);commits.append(dict(commit=commit,title=title))
inventory=sorted(set(changed+['docs/probe-3663/HANDOFF-CURRENT.md','tools/port3663/r4_review_package.py']))
inventory=[dict(path=p,sha256=sha(root/p)) for p in inventory if (root/p).is_file()]
result=dict(round=4,step=6,branch='gpt/3663-port',baseline='0f11f6e',source_commit=source,
    packaging_commit='branch HEAD after R4 step 6 commit; excluded from its own content hash',
    packaged_utc=datetime.now(timezone.utc).isoformat(),
    outcome='review package complete; two-player runtime milestone incomplete',
    steps={'1':'implemented','2':'implemented','3':'implemented with passive sitting',
           '4':'implemented/explicitly armed','5':'blocked at three failed L3 startup experiments; L3/L4 unpassed',
           '6':'review package complete'},
    tiers=status['tiers'],default_behavior='Tier A reader; C/D requires explicit guarded v1 with verified sandbox',
    scope_retirements={'guest_travel_time':read(docs/'port-evidence/r4/scope-result.json')['retired_by_scope'],
        'legacy_save_title_callsites':read(docs/'port-evidence/r4/step1-result.json')['retired_by_scope'],
        'reason':'R4 approves guest travel/time refusal and verified file-layer redirection; optional modules remain unreachable'},
    final_validation=validation,live_attempts=attempts,passive_session=passive,
    runtime_claims={'L3_passed':False,'L4_passed':False,'native_guest_join_executed':False,
        'disconnect_removal_verified_live':False,'reconnect_tested_live':False,'two_window_capture':None,
        'last_stack_correction_tested_in_game':False},
    cleanup=cleanup,cleanup_local_proof=dict(path=str(root/'build/r4/final-cleanup.json'),sha256=sha(root/'build/r4/final-cleanup.json')),
    remaining_blocked_ledger_rows=[r['row'] for r in status['rows'] if r['disposition']=='blocked'],
    blockers_file='R4-BLOCKERS.md',limitations=status['limitations'],
    attribution='Original authors/history/GPL v3 retained; original sibling and parent launcher untouched',
    commits_since_baseline=commits,files_changed_since_baseline=inventory,
    verified_retained_proofs=retained,packaging_only_changes_after_validation=metadata_changes,
    local_arming_receipt='Retained for provenance; exhausted live budget forbids further R4 launches; report metadata also changed after sealing.',
    evidence_policy='No new raw game dumps, screenshots, captures or executable binaries committed; existing Windows crash files remain local and are hashed.')
out=docs/'R4-REVIEW-PACKAGE.json'
out.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8')
print(dict(source=source,commits=len(commits),files=len(inventory),verified_proofs=len(retained),
    suite=validation['suite'],L3=False,L4=False,live_cap='3/3'))
