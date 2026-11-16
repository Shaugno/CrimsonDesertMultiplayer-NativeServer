"""Materialize the honest Step 7 blocker status from checked source/evidence."""
import argparse
from collections import Counter
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import re
import subprocess

ROOT = Path(__file__).resolve().parents[2]
DOCS = ROOT / 'docs/probe-3663'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build-root', type=Path, required=True)
    args = parser.parse_args()
    branch = subprocess.check_output(['git', 'branch', '--show-current'], cwd=ROOT).decode().strip()
    assert branch == 'gpt/3663-port'
    matches = json.loads((DOCS/'R2-MATCHES.json').read_text(encoding='utf-8'))
    core = json.loads((DOCS/'R2-CORE-LIST.json').read_text(encoding='utf-8'))
    assert {r['row'] for r in matches['rows']} == {r['r2'] for r in core}
    header = (ROOT/'src/mod/game/build_3663.h').read_text(encoding='utf-8')
    armed = [t for t in 'ABCD' if f'kTier{t}Complete = true;' in header]
    assert not armed, 'Reassess the blocker status against actual live acceptance before changing this report'
    evidence = DOCS/'port-evidence/r2'
    build = (evidence/'session-lock-final-build.txt').read_text(encoding='utf-8')
    host = (evidence/'session-lock-final-host.txt').read_text(encoding='utf-8')
    assert '100% tests passed, 0 tests failed out of 101' in build
    seconds = float(re.search(r'Total Test time \(real\) =\s+([\d.]+)', build)[1])
    remote_states = int(re.findall(r'remote states (\d+)', host)[-1])
    rejected = int(re.findall(r'rejected (\d+)', host)[-1])
    assert remote_states > 0 and rejected == 0
    assert 'Network stopped' in host and 'Worker exiting' in host
    binaries = []
    for name in ('cdmp.asi', 'cdmp_host.exe', 'cdmp_peer.exe'):
        path = args.build_root/'bin/RelWithDebInfo'/name
        with path.open('rb') as stream:
            digest = hashlib.file_digest(stream, 'sha256').hexdigest().upper()
        binaries.append(dict(path=str(path.resolve()), bytes=path.stat().st_size, sha256=digest))
    sessions = []
    for path in sorted((evidence/'live').glob('*/result.json')):
        result = json.loads(path.read_text(encoding='utf-8'))
        assert not result['l1_passed'] and not result['production_tiers_armed']
        sessions.append(dict(id=result['session'], result=result['result'], l1_passed=False,
                             evidence=str(path.relative_to(ROOT)).replace('\\', '/')))
    sessions.extend(dict(id=f'L{i}', result='blocked', passed=False) for i in range(1, 5))
    counts = Counter((r['tier'], 'adopted' if r['adopted'] else 'blocked') for r in matches['rows'])
    status = dict(
        generated_utc=datetime.now(timezone.utc).isoformat(), branch=branch,
        package_type='Step 7 blocker package', native_goal_reached=False,
        native_two_player_demonstrated=False, tiers_armed=armed,
        furthest_tier_armed=None, furthest_live_session_passed=None,
        final_commit='R2-MANIFEST.json final_commit, generated from clean committed HEAD',
        base_commit='2c80c21d31e11fbc05cc83883502b27d4f63ffb3',
        counts={t: {s: counts[t, s] for s in ('adopted', 'blocked')} for t in 'ABCD'},
        steps={'1': 'completed; 395 core game contracts, 17 retained R1 adoptions',
               '2': 'completed required whole-image AutoVT and default BinDiff; per-row native/conflict checks retained',
               '3': 'blocked field-sharing assessment; changed three-argument ABI and unresolved roster ownership',
               '4': 'guard framework and partial evidence tables prepared; all four native tiers blocked/unarmed',
               '5': 'three passive sessions recorded; L1-L4 blocked, no two-player acceptance',
               '6': 'completed in c5f7074; fallback scope unchanged',
               '7': 'blocker package; exact clean source commit and archive hashes in generated manifest'},
        validation=dict(configuration='x64 MSVC RelWithDebInfo', command='scripts/build.ps1 -BuildDir <local r2-build>',
                        tests_passed=101, tests_failed=0, seconds=seconds,
                        log='port-evidence/r2/session-lock-final-build.txt',
                        loopback=dict(remote_states=remote_states, rejected=rejected, clean_stop=True, native_evidence=False),
                        failures_and_reruns='port-evidence/r2/validation-deviations.md'),
        binaries=binaries, live_sessions=sessions,
        remaining=[dict(tier=r['tier'], row=r['row'], symbol=r['symbol'], missing=r['missing'],
                        next_experiment=r.get('next_experiment')) for r in matches['rows'] if not r['adopted']],
        next_experiment='First establish the client loading flags start/reset, actor association and lifetime contract offline. Then extend the passive collector to sample the verified fields across fast travel. Never invoke native reference-acquisition/control helpers or infer readiness from stable pointers.',
        cleanup='Last passive session exited normally; 23 save files unchanged, backup verified, test installation removed and no game remains. Optional supplemental matcher stopped with exact identity verification after required runs completed; inputs retained and no partial result counted.',
        scope='2976 behavior preserved, 3576 recognized/unarmed, 3663 recognized/unarmed. No optional module or fallback expansion, wire-version change, unsupported guard bypass, binary release, or claim of native multiplayer success.')
    (DOCS/'R2-STATUS.json').write_bytes((json.dumps(status, indent=2)+'\n').encode('utf-8'))
    print(json.dumps(dict(counts=status['counts'], blocked=len(status['remaining']), tiers_armed=armed,
                          tests=status['validation']['tests_passed'])))


if __name__ == '__main__':
    main()
