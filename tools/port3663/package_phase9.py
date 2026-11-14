"""Package committed Phase 9 source/evidence, excluding binaries and saves."""
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import subprocess
import zipfile

ROOT = Path(__file__).resolve().parents[2]
BASE = 'a8656e87a0c43db400bdf863193c1afb353d6de0'


def git(*args):
    return subprocess.check_output(['git', *args], cwd=ROOT)


def digest(data):
    return hashlib.sha256(data).hexdigest().upper()


def main():
    branch = git('branch', '--show-current').decode().strip()
    if branch != 'gpt/3663-port':
        raise SystemExit('Package must be generated from gpt/3663-port')
    final = git('rev-parse', 'HEAD').decode().strip()
    paths = set(git('diff', '--name-only', '--diff-filter=ACMRT', f'{BASE}..HEAD').decode().splitlines())
    paths.update(['AGENTS.md', 'CLAUDE.md', 'LICENSE', 'README.md', 'CMakeLists.txt'])
    payload = {}
    forbidden = {'.exe', '.dll', '.asi', '.pdb', '.zip', '.gpr', '.gzf', '.sav'}
    for path in sorted(paths):
        if Path(path).is_absolute() or '..' in Path(path).parts or Path(path).suffix.lower() in forbidden:
            raise SystemExit(f'Forbidden artifact path: {path}')
        payload[path] = git('show', f'HEAD:{path}')
    payload['SOURCE-DIFF.patch'] = git('diff', '--no-ext-diff', '--no-textconv', '--no-color', f'{BASE}..HEAD')
    payload['READ-ME-FIRST.md'] = b'''# Phase 9 blocker review package

Start with docs/probe-3663/HANDOFF-CURRENT.md and PHASE9-MANIFEST.json.
The native two-player goal was not reached. The 3663 gameplay path stays unarmed.
This package contains committed source changes and evidence, with no game/mod
binaries, saves or Ghidra projects. SOURCE-DIFF.patch is the full review diff.
The manifest identifies the exact final commit and all commits since the base.
Synthetic loopback is not two-player evidence. Preserve ByJunter attribution
and the included GPL v3 license.
'''
    commits = []
    for line in git('log', '--reverse', '--format=%H%x09%s', f'{BASE}..HEAD').decode().splitlines():
        sha, subject = line.split('\t', 1)
        commits.append(dict(sha=sha, subject=subject))
    counts = json.loads(payload['docs/probe-3663/PORT-MATCH-COUNTS.json'])
    binaries = json.loads(payload['docs/probe-3663/port-evidence/phase5-binary-hashes.json'].decode('utf-8-sig'))
    manifest = dict(
        package='Phase 9 blocker handoff for Claude review',
        created_utc=datetime.now(timezone.utc).isoformat(), branch=branch,
        repository='https://github.com/Shaugno/CrimsonDesertMultiplayer-NativeServer',
        base_commit=BASE, final_commit=final, commits_since_base=commits,
        native_goal_reached=False, gameplay_3663_armed=False,
        contiguous_completed_phases=[0, 1],
        phases={'0':'complete', '1':'complete; optional midpoint unavailable',
                '2':'partial; conservative inventory, semantic audit remains',
                '3':'partial; 17 corroborated static values, core gaps remain',
                '4':'partial; strict identity recognition, no native module routing',
                '5':'passed for guarded checkpoint only',
                '6':'blocked; no live session', '7':'blocked; no two-instance session',
                '8':'blocked; no Steam two-PC session', '9':'blocker package'},
        build={'configuration':'x64 RelWithDebInfo', 'visual_studio':'18 Community',
               'msvc_toolset':'14.52.37025', 'compiler':'19.52.37029.0', 'sdk':'10.0.26100.0',
               'binary_source_commit':'3c3ec37b713d7e676cd6bb898b21e32e92075699',
               'binary_hashes':binaries}, inventory_and_matches=counts,
        tests={'phase0_baseline':'101/101; 139.68 seconds',
               'phase0_fixed':'101/101; 93.77 seconds',
               'phase5':'101/101; 111.81 seconds',
               'synthetic_loopback':'8 seconds; 78 remote states; 0 rejected; clean unload',
               'live':'none'},
        blockers=['FieldManager acquisition/entry and field-list/rollback contract',
                  'Native lobby/status identity and post-load hook contracts',
                  'Player/SAM roots, health and reconnect/removal barriers',
                  'Complete current login/movement/frame/payload audit',
                  'Guest save interception and early-load ordering'],
        smallest_next_experiment='Trace named 3663 LoginCharacterToFieldServerReq handler 0x2C6AE10 to field acquisition/entry; corroborate both targets and current list writers against 2976.',
        files={path:dict(sha256=digest(data), bytes=len(data)) for path, data in sorted(payload.items())})
    manifest_bytes = (json.dumps(manifest, indent=2) + '\n').encode('utf-8')
    out = ROOT / 'dist/phase9'
    out.mkdir(parents=True, exist_ok=True)
    archive = out / f'Phase9-3663-port-{final[:12]}.zip'
    with zipfile.ZipFile(archive, 'w', compression=zipfile.ZIP_DEFLATED, compresslevel=9) as package:
        for path, data in payload.items(): package.writestr(path, data)
        package.writestr('PHASE9-MANIFEST.json', manifest_bytes)
    with zipfile.ZipFile(archive) as package:
        if package.testzip() is not None: raise RuntimeError('ZIP CRC validation failed')
        for path, info in manifest['files'].items():
            if digest(package.read(path)) != info['sha256']: raise RuntimeError(f'Hash mismatch: {path}')
    (out / 'PHASE9-MANIFEST.json').write_bytes(manifest_bytes)
    archive_hash = digest(archive.read_bytes())
    (out / (archive.name + '.sha256')).write_text(f'{archive_hash}  {archive.name}\n', encoding='utf-8')
    print(json.dumps(dict(archive=str(archive), manifest=str(out / 'PHASE9-MANIFEST.json'),
                         final_commit=final, files=len(payload)+1, sha256=archive_hash, bytes=archive.stat().st_size)))


if __name__ == '__main__':
    main()
