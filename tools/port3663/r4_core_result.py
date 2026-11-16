"""Validate the bounded, passive R4 trace; publish summaries, never raw pointers."""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import re


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def analyze(log):
    events = {}
    for line in log.splitlines():
        m = re.search(r'R4Core: seq (\d+) point (\d+) (\w+) T(\d+) valid (true|false) context (.*)', line)
        if m:
            seq, point, name, thread, valid, context = m.groups()
            assert int(seq) not in events, 'duplicate event'
            events[int(seq)] = dict(point=int(point), name=name, thread=int(thread),
                                    valid=valid == 'true', context=[int(x, 16) for x in context.split(',')],
                                    data=[], roots=[])
        m = re.search(r'R4Core: seq (\d+) (data|roots) (\d+) (.*)', line)
        if m:
            seq, kind, start, values = m.groups()
            target = events[int(seq)][kind]
            assert len(target) == int(start), 'missing/reordered fragment'
            target.extend(int(x, 16) for x in values.split(','))
    assert events and sorted(events) == list(range(1, len(events) + 1)), 'sequence gap'
    assert all(e['valid'] and len(e['data']) == 24 and len(e['roots']) == 16 for e in events.values())
    assert not re.search(r'R4Core:.*(?:guard stop|overflow|dropped [1-9])', log, re.I)
    by_name = {n: [e for e in events.values() if e['name'] == n] for n in {e['name'] for e in events.values()}}
    get = lambda n: by_name.get(n, [])
    entries, exits = get('VecEntry'), get('VecExit')
    assert len(entries) == len(exits) and entries
    for a, b in zip(entries, exits):
        x, y = a['data'], b['data']
        assert a['thread'] == b['thread'] and x[0] == y[0]
        assert x[2:4] == [0, 0] and y[2:4] == [1, 1]
        assert x[4] == y[4] and x[5] == y[5] == 1 and x[6] == 0x14EB68840
    roots = [e['roots'] for e in events.values() if e['roots'][0]]
    assert roots and all(r[0] == r[5] and r[1] == r[6] == 0x145F69100 for r in roots)
    assert all(r[10:13] == [0x1459874C8, 0x141593240, 0x140E297F0] for r in roots)
    assert all(e['data'][7] == 0x7FFFFFFFFFFFFFFF for e in get('LobbyAdd'))
    locks = get('FieldLocks')
    assert locks and all(e['data'][5:8] == [0x145B3FFB0, 0x141593320, 0x141593370]
                         and e['data'][9:12] == [0x145BCEE40, 0x141593320, 0x141593370] for e in locks)
    return dict(events=len(events), counts=dict(sorted(Counter(e['name'] for e in events.values()).items())),
                invalid_events=0, sequence_gaps=0, vector_pairs=len(entries),
                vector_field_keys=sorted({hex(e['data'][8]) for e in entries}),
                vector_transition='count/capacity 0/0 -> 1/1; row actor count 1; native AddActor caller EB6883B',
                key_class_pairs=sorted({f"definition {e['data'][4]} -> class {e['data'][2] >> 28:X}"
                                        for e in get('KeyClass')}),
                load_dispatch_calls=len(get('Component')),
                load_stub_dispatch_calls=sum(e['data'][2] == 0x1403ABEE0 for e in get('Component')),
                session_ids=sorted({e['data'][3] for e in get('SessionId')}),
                login_manager_site_hits=len(get('LoginMgr')),
                identity_threads=sorted({e['thread'] for n in ['LobbyAdd', 'ClearLobby', 'ClearList', 'Phases'] for e in get(n)}),
                network_threads=sorted({e['thread'] for e in get('SessionId')}),
                roots='SAM root equals dereferenced login-manager holder; exact named SAM and WindowsLock3 vtables/slots agree',
                field_locks='two distinct WindowsRWLock types, slot 1/2 = 1593320/1593370; NOT WindowsLock3')


def main():
    p = argparse.ArgumentParser()
    p.add_argument('--session-dir', type=Path, required=True)
    p.add_argument('--out', type=Path, required=True)
    a = p.parse_args()
    folder = a.session_dir.resolve()
    log = folder / 'cdmp.log'
    result = analyze(log.read_text(encoding='utf-8-sig'))
    saves = json.loads((folder / 'save-verification.json').read_text(encoding='utf-8-sig'))
    watchdog = json.loads((folder / 'watchdog.json').read_text(encoding='utf-8-sig'))
    assert saves['backup_verified'] and saves['live_saves_unchanged']
    assert saves['before_files'] == saves['after_files'] == 23
    assert watchdog['state'] == 'exited', 'watchdog forced exit or incomplete'
    result.update(session=folder.name, source_commit='9cc2c4a', executable_sha256=saves['exe_sha256'],
                  question='Do the native vector, lobby/login, key-class and session contexts agree with the expected object identities?',
                  result='passive evidence collected; no join attempted',
                  user_report='requested sequence done, including fast travel and Quit to Desktop',
                  exit='observed natural process exit; no forced termination; no exit code was collected',
                  requested_quit_local='2026-11-16 16:38 America/New_York',
                  observed_exit_utc=watchdog['ended_utc'], deadline_utc=watchdog['deadline_utc'],
                  real_saves_unchanged=23, join_attempts=0, gameplay_networking=False,
                  cleanup='Finish controller restored prior installation; elevated host process/file inspection confirmed no game process or deployed ASI/PDB/loader',
                  limits=['Bounded samples are ordinary local admissions, not guest join/rollback evidence.',
                          'No LoginMgr site hit; root equality is separate corroboration of static getter/context proof.',
                          'OwnerKey data19 read vtable+20, outside the three-entry ScopeAttacher table: rejected. Correct native Dispose is slot2 (+10), 1659370 -> 1659380.',
                          'Memory-readable validity does not validate a function pointer or permit a call.',
                          'Requested 16:38 quit was exceeded; observed exit remained within fourteen-minute watchdog and fifteen-minute session limit.'],
                  local_evidence=[dict(path=str(f), sha256=digest(f)) for f in
                                  [log, folder/'question.md', folder/'save-verification.json',
                                   folder/'watchdog.json', folder/'deployed-hashes.json', folder/'evidence-hashes.json']])
    a.out.write_bytes((json.dumps(result, indent=2)+'\n').encode('utf-8'))
    print(json.dumps({k:v for k,v in result.items() if k not in ['local_evidence','limits']}, indent=2))


if __name__ == '__main__':
    main()
