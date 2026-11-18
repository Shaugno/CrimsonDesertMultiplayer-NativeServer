"""Seal R7's game-free NT-isolation checks; never deploy or launch the game."""
import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import re
import subprocess

ROOT = Path(__file__).resolve().parents[2]
GAME_SHA = '8BE67178D9B9A149C361A731051665E0BA4D441E250AE1DFE645BD6E524345C8'


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest().upper()


def write_new(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open('x', encoding='utf-8') as output:
        json.dump(value, output, indent=2)
        output.write('\n')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build-dir', type=Path, required=True)
    parser.add_argument('--prefix', default='step1')
    parser.add_argument('--network-follow-up', action='store_true',
                        help='Seal the R7 Step 2 offline networking fix and its baseline/fixed probes')
    parser.add_argument('--refusal-log', action='store_true',
                        help='Also require the production refusal logging regression')
    parser.add_argument('--acquire-diagnostic', action='store_true',
                        help='Seal the diagnostics-only R7 L3 build')
    args = parser.parse_args()
    assert re.fullmatch(r'[a-zA-Z0-9_-]+', args.prefix)
    assert not args.network_follow_up or args.prefix != 'step1'
    assert not args.refusal_log or args.network_follow_up
    assert not args.acquire_diagnostic or args.refusal_log
    local = ROOT / 'build/r7'
    suite = local / f'{args.prefix}-suite.txt'
    smoke = local / f'{args.prefix}-smoke.txt'
    log = local / f'{args.prefix}-smoke-run/e2e/cdmp.log'
    ctest = local / f'{args.prefix}-ctest.log'
    boundary = local / f'{args.prefix}-boundary.json'
    boundary_data = json.loads(boundary.read_text(encoding='utf-8-sig'))
    assert boundary_data['round'] == 7
    if args.acquire_diagnostic:
        ledger = json.loads((ROOT / 'docs/probe-3663/port-evidence/r7/attempts.json').read_text(encoding='utf-8'))
        assert boundary_data['live_attempts'] == len(ledger['attempts'])
    else:
        assert boundary_data['live_attempts'] == (1 if args.network_follow_up else 0)
    assert not boundary_data['game_processes']
    assert not any(f['exists'] for f in boundary_data['installed_controlled_files'])
    assert boundary_data['source_plan_sha256'] == boundary_data['stored_plan_sha256']
    additional_evidence = (
        local / 'baseline-suite.txt', local / 'step1-first-suite.txt',
        local / 'step1-first-ctest.log', local / 'launcher-focused.txt',
    )
    if args.network_follow_up:
        assert boundary_data['offline_launches'] == 0
        assert boundary_data['game_executable_sha256'] == GAME_SHA
        before_network = local / f'{args.prefix}-before-network.txt'
        after_network = local / f'{args.prefix}-after-network.txt'
        before_text = before_network.read_text(encoding='utf-8-sig')
        assert "name='\\??\\Nsi'" in before_text and 'GetAdaptersAddresses=5' in before_text
        assert 'getaddrinfo(steamcommunity.com)=11001' in before_text
        network_text = after_network.read_text(encoding='utf-8-sig')
        for line in ('GetAdaptersAddresses=0', 'getaddrinfo(steamcommunity.com)=0',
                     'TCP connect(owned loopback)=0', 'TCP connect(steamcommunity.com:443)=0'):
            assert network_text.count(line) == 2, line
        assert 'network-baseline exit=0; real and backup manifest unchanged=True' in network_text
        assert 'network-sandbox exit=0; real and backup manifest unchanged=True' in network_text
        assert 'Fixture refusal' not in network_text
        additional_evidence = (before_network, after_network)
    suite_text = suite.read_text(encoding='utf-8-sig')
    total, = re.findall(r'100% tests passed, 0 tests failed out of (\d+)', suite_text)
    seconds, = re.findall(r'Total Test time \(real\) =\s*([\d.]+) sec', suite_text)
    assert int(total) >= 119
    fixture_text = ctest.read_text(encoding='utf-8-sig')
    assert 'R7 NT fixture PASS: inherited profile/native resolver unchanged; save-only redirect;' in fixture_text
    if args.network_follow_up:
        assert 'Fixture disk alias denied=1 removed=1' in fixture_text
    if args.refusal_log:
        assert 'R7 refusal log fixture PASS: 20 distinct names, 25 named refusals' in fixture_text
    smoke_text = smoke.read_text(encoding='utf-8-sig')
    assert '3663 compiled tier mask 0xF (metadata only)' in smoke_text
    assert 'Mod stopped and unloaded cleanly' in smoke_text
    native_log = log.read_text(encoding='utf-8-sig')
    states, = re.findall(r'remote states (\d+)', native_log)
    assert int(states) > 0 and 'rejected 0' in native_log
    assert 'Network stopped' in native_log and 'Worker exiting' in native_log
    binary = args.build_dir.resolve() / 'bin/RelWithDebInfo'
    asi = binary / 'cdmp.asi'
    assert sha(asi) == sha(log.parent / 'cdmp.asi')
    if args.acquire_diagnostic:
        assert b'3663 join Acquire predicates:' in asi.read_bytes()
    paths = subprocess.check_output(
        ['git', 'ls-files', '-z', '--cached', '--others', '--exclude-standard'], cwd=ROOT
    ).decode().split('\0')
    sources = sorted(set(f for f in paths if f and (f.startswith((
        'src/', 'tests/', 'scripts/', 'tools/', 'cmake/', 'third_party/safetyhook-patch/'
    )) or f == 'CMakeLists.txt') and (ROOT / f).is_file()))
    manifest = [dict(path=f, sha256=sha(ROOT / f)) for f in sources]
    runner = binary / 'r5_desktop_runner.exe'
    receipt = dict(
        version=3, round=7, not_before_utc='2026-11-18T15:00:00Z',
        compiled_tier_mask=15, executable_sha256=GAME_SHA,
        full_suite_passed=True, synthetic_v1_metadata_verified=True,
        isolation_method='nt', host_isolation='none',
        asi_path=str(asi), asi_sha256=sha(asi), pdb_path=str(binary / 'cdmp.pdb'),
        watchdog_sha256=sha(ROOT / 'tools/port3663/r5_session_watchdog.ps1'),
        desktop_runner_path=str(runner), desktop_runner_sha256=sha(runner),
        source_manifest=manifest,
    )
    if args.network_follow_up:
        receipt['sandbox_network_verified'] = True
    if args.refusal_log:
        receipt['bounded_refusal_log_verified'] = True
    if args.acquire_diagnostic:
        receipt['acquire_predicate_trace_compiled'] = True
    arming_path = local / f'{args.prefix}-arming-receipt.json'
    result_path = ROOT / f'docs/probe-3663/port-evidence/r7/{args.prefix}-validation.json'
    assert not arming_path.exists() and not result_path.exists(), 'Use a fresh evidence prefix'
    write_new(arming_path, receipt)
    result = dict(
        round=7, step=3 if args.acquire_diagnostic else (2 if args.network_follow_up else 1),
        recorded_utc=datetime.now(timezone.utc).isoformat(),
        suite=f'{total}/{total} passed', seconds=float(seconds),
        synthetic_remote_states=int(states), synthetic_rejects=0,
        smoke='eight-second owned host/peer; metadata mask 0xF; clean unload',
        asi_sha256=sha(asi), desktop_runner_sha256=sha(runner),
        guest='NT save-directory hooks only; normal USERPROFILE/LOCALAPPDATA inherited',
        host='backed-up real saves; no sandbox',
        not_before_utc=receipt['not_before_utc'],
        activation_cause=(
            'Sandbox DNS failure reproduced and corrected in fixtures; R7-S2 live causality is unconfirmed.'
            if args.network_follow_up else 'unproven hypothesis; no R7 game launch or activation test'
        ),
        limits=[
            'Owned TEMP fixture data only; no game runtime acceptance.',
            'Steam-style named pagefile MAP-only IPC and pipes are tested without SteamAPI or game activation.',
            'Historical fake-profile fixtures remain game-free; production rejects that route.',
            'Concurrent host save deltas cannot attribute writes by process; guest-only manifest check remains required.',
            'L3 and L4 remain unpassed; the R6 character-login fix awaits live execution.',
        ],
        arming_receipt=dict(path=str(arming_path.resolve()), sha256=sha(arming_path)),
        evidence=[dict(path=str(f.resolve()), sha256=sha(f))
                  for f in (suite, ctest, smoke, log, boundary, *additional_evidence)],
        source_manifest_sha256=hashlib.sha256(json.dumps(manifest, sort_keys=True).encode()).hexdigest().upper(),
    )
    if args.network_follow_up:
        result['sandbox_network_verified'] = receipt['sandbox_network_verified']
    if args.refusal_log:
        result['bounded_refusal_log_verified'] = receipt['bounded_refusal_log_verified']
    if args.acquire_diagnostic:
        result['acquire_predicate_trace_compiled'] = receipt['acquire_predicate_trace_compiled']
    write_new(result_path, result)
    print(json.dumps({k: result[k] for k in ('round', 'step', 'suite', 'seconds', 'synthetic_remote_states')}))


if __name__ == '__main__':
    main()
