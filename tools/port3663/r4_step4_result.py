"""Seal the completed offline checks; no game launch or tier permission here."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess

root = Path(__file__).resolve().parents[2]

def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest().upper()

def write(path, value):
    Path(path).write_text(json.dumps(value, indent=2) + '\n', encoding='utf-8')

parser = argparse.ArgumentParser()
parser.add_argument('--build-dir', type=Path, required=True)
args = parser.parse_args()
binary = args.build_dir.resolve() / 'bin/RelWithDebInfo'
suite = root / 'build/r4/step4-final-suite.txt'
smoke = root / 'build/r4/step4-final-smoke.txt'
log = root / 'build/r4/step4-final-smoke-run/e2e/cdmp.log'
suite_text = suite.read_text(encoding='utf-8-sig')
assert '100% tests passed, 0 tests failed out of 114' in suite_text
seconds, = re.findall(r'Total Test time \(real\) = ([\d.]+) sec', suite_text)
smoke_text = smoke.read_text(encoding='utf-8-sig')
assert '3663 compiled tier mask 0xF (metadata only)' in smoke_text
assert 'Mod stopped and unloaded cleanly' in smoke_text
log_text = log.read_text(encoding='utf-8-sig')
states, = re.findall(r'remote states (\d+)', log_text)
assert int(states) > 0 and 'rejected 0' in log_text
assert 'Network stopped' in log_text and 'Worker exiting' in log_text
asi = binary / 'cdmp.asi'
assert sha(asi) == sha(log.parent / 'cdmp.asi')
paths = subprocess.check_output(['git', 'ls-files', '-z', '--cached', '--others', '--exclude-standard'], cwd=root).decode().split('\0')
sources = sorted(set(p for p in paths if p and (
    p.startswith(('src/', 'tests/', 'scripts/', 'tools/', 'cmake/', 'patches/'))
    or p == 'CMakeLists.txt') and (root / p).is_file()))
manifest = [dict(path=p, sha256=sha(root / p)) for p in sources]
receipt = dict(version=1, compiled_tier_mask=15,
    executable_sha256='8BE67178D9B9A149C361A731051665E0BA4D441E250AE1DFE645BD6E524345C8',
    full_suite_passed=True, synthetic_v1_metadata_verified=True,
    asi_path=str(asi), asi_sha256=sha(asi), pdb_path=str(binary / 'cdmp.pdb'),
    watchdog_sha256=sha(root / 'tools/port3663/r4_session_watchdog.ps1'),
    source_manifest=manifest)
receipt_path = root / 'build/r4/arming-receipt.json'
write(receipt_path, receipt)
result = dict(step=4, result='offline implementation and guarded experimental C/D arming complete; L3/L4 untested',
    suite='114/114 passed', suite_seconds=float(seconds),
    synthetic_remote_states=int(states), synthetic_rejects=0,
    smoke='eight seconds; compiled tier metadata 0xF; clean stop/unload; synthetic only',
    compiled_tiers=list('ABCD'), default_route='Tier A reader',
    explicit_route='session2 core_v1=1; exact build; actual verified file sandbox; role and transport checks',
    join_attempts=0, live_acceptance={'L3': 'not attempted', 'L4': 'not attempted'},
    safeguards=['native logout only at title/stable lifecycle; loss during loading quarantines',
        'new six-argument LoadingComplete hook disables guest world-clock publication',
        'native faults seal admission and injection, retain uncertain pins, and continue exception search',
        'typed user/actor generation checks, field lock order and conservative rollback',
        'bounded queues; native packet work on verified NetworkThread; no legacy optional installers',
        'launcher fresh backups and sandboxes; verified watchdog; installation restoration even after save drift',
        'controlled marker refuses lost-role or unsandboxed relaunch; default installed configuration unchanged'],
    limitations=['No native C/D execution or two-player acceptance is established by these offline checks.',
        'Reconnect is unsupported: changed guest native conversation exits with guard code 0xCD67.',
        'Loading disconnect, host field replacement or uncertain insertion requires both processes to restart.',
        'Only single-primary ordinary-character walking/turning/running is admitted; optional gameplay stays unreachable.',
        'File sandbox is an API interception layer, not an operating-system security boundary.',
        'The existing optional S3 publication race is not claimed fixed; this route does not install it.'],
    source_manifest_sha256=hashlib.sha256(json.dumps(manifest, sort_keys=True).encode()).hexdigest().upper(),
    arming_receipt_sha256=sha(receipt_path), asi_sha256=sha(asi),
    evidence=[dict(path=str(p.resolve()), sha256=sha(p)) for p in (suite, smoke, log)])
write(root / 'docs/probe-3663/port-evidence/r4/step4-result.json', result)
print({k: result[k] for k in ('step', 'suite', 'suite_seconds', 'synthetic_remote_states', 'join_attempts')})
