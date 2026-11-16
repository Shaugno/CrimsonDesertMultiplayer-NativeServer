"""Create the Step 7 source/evidence review archive from a clean committed HEAD.

Requires the final R2-STATUS.json and real standard matcher outputs. No game,
mod binary, save, credential, downloaded tool or analysis database is packaged.
"""
from collections import Counter
from datetime import datetime,timezone
import hashlib
import json
from pathlib import Path
import subprocess
import zipfile

ROOT=Path(__file__).resolve().parents[2]
BASE='2c80c21d31e11fbc05cc83883502b27d4f63ffb3'

def git(*args):
    return subprocess.check_output(['git',*args],cwd=ROOT)
def digest(data): return hashlib.sha256(data).hexdigest().upper()
def main():
    branch=git('branch','--show-current').decode().strip()
    if branch!='gpt/3663-port': raise SystemExit('Wrong package branch')
    if git('status','--porcelain').strip(): raise SystemExit('Commit the final source/evidence first')
    final=git('rev-parse','HEAD').decode().strip()
    # Include the review delta and the repository's governing/attribution files.
    paths=set(git('diff','--name-only','--diff-filter=ACMRT',f'{BASE}..HEAD').decode().splitlines())
    paths.update(['AGENTS.md','CLAUDE.md','LICENSE','README.md','CMakeLists.txt'])
    payload={}
    forbidden={'.exe','.dll','.asi','.pdb','.sav','.save','.zip','.gpr','.gzf','.binexport','.bindiff','.bin'}
    for path in sorted(paths):
        parts=Path(path).parts
        if Path(path).is_absolute() or '..' in parts or Path(path).suffix.lower() in forbidden or parts[0].lower() in ('backups','build','dist','reference','captures'):
            raise SystemExit(f'Forbidden package path {path}')
        payload[path]=git('show',f'HEAD:{path}')
    prefix='docs/probe-3663/'
    status=json.loads(payload[prefix+'R2-STATUS.json'].decode('utf-8-sig'))
    matches=json.loads(payload[prefix+'R2-MATCHES.json'].decode('utf-8-sig'))
    core=json.loads(payload[prefix+'R2-CORE-LIST.json'].decode('utf-8-sig'))
    # Check the bytes that reviewers actually receive, after Git's newline
    # normalization, rather than trusting a working-tree/raw capture digest.
    for path,data in payload.items():
        if '/live/' not in path or not path.endswith('/result.json'): continue
        session=json.loads(data.decode('utf-8-sig'))
        for item in session['files']:
            assert Path(item['name']).name==item['name'],item['name']
            included=path.rsplit('/',1)[0]+'/'+item['name']
            assert included in payload,('Missing live evidence',included)
            assert len(payload[included])==item['bytes'],included
            assert digest(payload[included])==item['sha256'].upper(),included
    assert {r['row'] for r in matches['rows']}=={r['r2'] for r in core}
    for row in matches['rows']:
        assert bool(row['missing'])!=bool(row['adopted']),row['row']
        if row['adopted']: assert row['target'] is not None and row['method']!='none'
    for item in matches['standard_outputs']:
        raw=Path(item['path'])
        assert raw.exists() and raw.stat().st_size==item['bytes']
        with raw.open('rb') as stream:
            assert hashlib.file_digest(stream,'sha256').hexdigest().lower()==item['sha256'].lower()
    header=payload['src/mod/game/build_3663.h'].decode('utf-8-sig')
    armed=status['tiers_armed']
    for tier in 'ABCD':
        assert f'kTier{tier}Complete = {str(tier in armed).lower()};' in header
        if tier in armed:
            assert all(r['adopted'] for r in matches['rows'] if r['tier']==tier)
            assert all(lower in armed for lower in 'ABCD'[:'ABCD'.index(tier)])
    if status['native_goal_reached']:
        assert armed==list('ABCD')
        assert all(any(s['id']==name and s['result']=='passed' for s in status['live_sessions']) for name in ('L1','L2','L3','L4'))
    payload['SOURCE-DIFF.patch']=git('diff','--no-ext-diff','--no-textconv','--no-color',f'{BASE}..HEAD')
    payload['READ-ME-FIRST.md']=('''# R2 Step 7 review package

Start with docs/probe-3663/HANDOFF-CURRENT.md, R2-STATUS.json and R2-MANIFEST.json.
This is a committed source/evidence review delta, not a standalone game build.
The manifest records the exact final commit and every commit since round 1.
Read the actual armed tiers and live-session outcomes; synthetic loopback
does not establish native two-player behavior. SOURCE-DIFF.patch is the full
review diff. No game/mod binaries, saves or analysis databases are included.
Preserve ByJunter attribution and the included GPL v3 license.
''').encode()
    commits=[]
    for line in git('log','--reverse','--format=%H%x09%s',f'{BASE}..HEAD').decode().splitlines():
        sha,subject=line.split('\t',1); commits.append(dict(sha=sha,subject=subject))
    counts=Counter((r['tier'],'adopted' if r['adopted'] else 'blocked') for r in matches['rows'])
    manifest=dict(package='R2 Step 7 review package',created_utc=datetime.now(timezone.utc).isoformat(),
        branch=branch,base_commit=BASE,final_commit=final,commits_since_base=commits,
        status=status,counts={t:{k:counts[t,k] for k in ('adopted','blocked')} for t in 'ABCD'},
        files={p:dict(bytes=len(d),sha256=digest(d)) for p,d in sorted(payload.items())})
    manifest_bytes=(json.dumps(manifest,indent=2)+'\n').encode()
    destination=ROOT/'dist/step7'; destination.mkdir(parents=True,exist_ok=True)
    archive=destination/f'R2-3663-port-{final[:12]}.zip'
    with zipfile.ZipFile(archive,'w',zipfile.ZIP_DEFLATED,compresslevel=9) as package:
        for path,data in payload.items(): package.writestr(path,data)
        package.writestr('R2-MANIFEST.json',manifest_bytes)
    with zipfile.ZipFile(archive) as package:
        assert package.testzip() is None
        for path,info in manifest['files'].items(): assert digest(package.read(path))==info['sha256']
    (destination/'R2-MANIFEST.json').write_bytes(manifest_bytes)
    archive_hash=digest(archive.read_bytes())
    (destination/(archive.name+'.sha256')).write_text(f'{archive_hash}  {archive.name}\n')
    print(json.dumps(dict(archive=str(archive),final_commit=final,sha256=archive_hash,bytes=archive.stat().st_size,files=len(payload)+1)))

if __name__=='__main__': main()
