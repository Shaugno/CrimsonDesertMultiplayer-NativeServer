"""Build R4's additive ledger without rewriting earlier round evidence."""
from collections import Counter
import json
from pathlib import Path
import re

root = Path(__file__).resolve().parents[2]
docs = root / 'docs/probe-3663'
status = json.loads((docs / 'R3-STATUS.json').read_text(encoding='utf-8'))
save = json.loads((docs / 'port-evidence/r4/step1-result.json').read_text(encoding='utf-8'))
scope = json.loads((docs / 'port-evidence/r4/scope-result.json').read_text(encoding='utf-8'))
assert save['final_validation']['suite'] == '108/108 passed'
status['r4_baseline'] = '0f11f6e'
status['preceding_ledger'] = 'R3-STATUS.json'
status['limitations'] = [
    'Closed rows include approved policies and R4 scope retirements; they are not all native RVA adoptions.',
    'A/B armed; B remains the explicit native-host loopback diagnostic. C/D and native guest startup remain closed.',
    'C requires the native reference/insertion/rollback backend and identity fixes; pure policies are not native proof.',
    'The file sandbox is verified offline, with no in-game validation yet. Its launcher Run path awaits C/D.',
    'Guest travel/time and legacy per-callsite save/title patches are outside 3663 v1 scope.',
    'One prior full run exposed the existing optional S3 ring publication race; final Step 1 and Step 2 suites passed. That race is not claimed fixed.'
]
status['steps'] = {'1': 'complete; file sandbox and offline fixture verified; launch awaits C/D',
                   '2': 'complete; native control filter; travel/time scope retired',
                   '3': 'pending native backend and passive sitting',
                   '4': 'pending lifecycle implementation and arming',
                   '5': 'pending L3 then L4; no R4 live sessions or join attempts',
                   '6': 'pending final review package'}
for row in status['rows']:
    if row['row'] in save['retired_by_scope']:
        row.update(disposition='retired_by_scope', target=None, authority='port-evidence/r4/step1-result.json',
                   missing='', reason=save['retirement_reason'])
    if row['row'] in scope['retired_by_scope']:
        row.update(disposition='retired_by_scope', target=None, authority='port-evidence/r4/scope-result.json',
                   missing='', reason='R4 v1 refuses guest travel/fast-travel and time publication before host native dispatch; these legacy publication adapters remain unreachable.')
header = (root / 'src/mod/game/build_3663.h').read_text(encoding='utf-8')
status['tiers'] = {}
for tier in 'ABCD':
    rows = [r for r in status['rows'] if r['tier'] == tier]
    counts = Counter(r['disposition'] for r in rows)
    armed, = re.findall(rf'kTier{tier}Complete = (true|false);', header)
    status['tiers'][tier] = dict(total=len(rows), closed=len(rows)-counts['blocked'],
                                blocked=counts['blocked'], dispositions=dict(counts), armed=armed=='true')
(docs / 'R4-STATUS.json').write_bytes((json.dumps(status, indent=2)+'\n').encode('utf-8'))
print({tier: (s['closed'], s['total'], s['armed']) for tier,s in status['tiers'].items()})
