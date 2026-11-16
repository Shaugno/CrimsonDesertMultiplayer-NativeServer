"""Package completed offline Step3 and its one passive sitting."""
from pathlib import Path
import hashlib
import json
import re
root=Path(__file__).resolve().parents[2]
suite=root/'build/r4/step3-final-suite.txt'
assert '100% tests passed, 0 tests failed out of 110' in suite.read_text(encoding='utf-8-sig')
smoke=root/'build/r4/step3-final-smoke-run/e2e/cdmp.log'
text=smoke.read_text(encoding='utf-8-sig')
states,=re.findall(r'remote states (\d+)',text)
assert int(states)>0 and 'rejected 0' in text and 'Network stopped' in text and 'Worker exiting' in text
paths=['build/r4/step3-final-suite.txt','build/r4/step3-final-smoke.txt',
       'build/r4/step3-final-smoke-run/e2e/cdmp.log',
       'src/mod/session2/field_share3663.cpp','src/mod/session2/identity3663.cpp',
       'src/mod/session2/r4_native_bindings.h','src/coop3663/join3663_transaction.h',
       'src/coop3663/identity3663_policy.h','tests/join3663_transaction_tests.cpp','tests/identity3663_policy_tests.cpp',
       'docs/probe-3663/port-evidence/r4/core-adoptions.json','docs/probe-3663/port-evidence/r4/passive-result.json']
out=dict(step=3,result='offline implementation and passive evidence complete; target route unarmed pending Step4',
    suite='110/110 passed',suite_seconds=91.39,synthetic_remote_states=int(states),
    smoke='eight seconds, zero rejects, clean worker stop/unload; synthetic only',
    live_session='R2-P1-20261116-161703',join_attempts=0,
    implemented=['three-argument native EnterField backend gated by R3 pure policy',
        'typed FindUser and FindActor owning scopes; field reference and native field-access destructor',
        'two field locks in native order; native 16-byte VecPush; exact last-row insertion ticket',
        'native AddActor and RemoveActor body mutation observers; epoch/key/vector/count/row validation before rollback',
        'uncertain/used/changed insertion quarantines references and seals guest admission; never closes host field',
        'separate per-build lobby/status normalization and pre-phase invariant; host-derived thread affinity; uniqueness and login payload gates'],
    limitations=['C/D not yet armed or routed; adapters have no in-game execution evidence.',
        'Only the measured single-primary roster is admitted. Companion/factory branches remain refused.',
        'Any intervening native actor mutation conservatively prevents rollback, even if unrelated.',
        'Quarantine holds owning references until process exit and requires restart; not reconnect support.',
        'No legacy identity patch, TLS realm offset, four-argument EnterField or optional module is used.',
        'Live trace is ordinary local play. It does not prove cross-user sharing or rollback execution.'],
    evidence=[dict(path=str((root/name).resolve()),sha256=hashlib.sha256((root/name).read_bytes()).hexdigest().upper()) for name in paths])
(root/'docs/probe-3663/port-evidence/r4/step3-result.json').write_bytes((json.dumps(out,indent=2)+'\n').encode('utf-8'))
print({k:out[k] for k in ['step','result','suite','synthetic_remote_states','join_attempts']})
