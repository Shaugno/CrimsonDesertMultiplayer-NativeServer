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
    'A/B/C/D compiled and armed for explicit guarded v1; the default target installation remains the Tier A reader.',
    'Native host/guest route, identity, join and stable disconnect are implemented. L3/L4 execution and acceptance remain separate.',
    'The file sandbox and controlled launcher are verified offline. The guarded host route activated in the third startup; native guest joining and gameplay C/D have no live validation.',
    'L3 failed in three controlled startup experiments, counted against the plan live-attempt cap. Latest stack correction is offline-validated only. L4 requires L3 and did not run.',
    'Reconnect is unsupported. Guest conversation change exits with guard code 0xCD67; loading loss quarantines and requires restart.',
    'Guest travel/time and legacy per-callsite save/title patches are outside 3663 v1 scope.',
    'One prior full run exposed the existing optional S3 ring publication race; final Step 1 and Step 2 suites passed. That race is not claimed fixed.'
]
status['steps'] = {'1': 'complete; file sandbox and offline fixture verified',
                   '2': 'complete; native control filter; travel/time scope retired',
                   '3': 'complete; native adapter, identity and passive sitting; historical Step3 evidence retained',
                   '4': 'complete; explicit guarded v1 route; 114/114 tests and synthetic tier-metadata smoke; see Step4 result',
                   '5': 'blocked at live attempt cap: three failed L3 startups, zero guest/native joins; final offline fixes validated; L4 prerequisite unmet; see R4-LIVE.md',
                   '6': 'complete review package; see R4-REVIEW-PACKAGE.json and HANDOFF-CURRENT.md; L3/L4 unpassed'}
for row in status['rows']:
    if row['row'] in save['retired_by_scope']:
        row.update(disposition='retired_by_scope', target=None, authority='port-evidence/r4/step1-result.json',
                   missing='', reason=save['retirement_reason'])
    if row['row'] in scope['retired_by_scope']:
        row.update(disposition='retired_by_scope', target=None, authority='port-evidence/r4/scope-result.json',
                   missing='', reason='R4 v1 refuses guest travel/fast-travel and time publication before host native dispatch; these legacy publication adapters remain unreachable.')
adoptions_path=docs/'port-evidence/r4/core-adoptions.json'
if adoptions_path.exists():
    adoptions=json.loads(adoptions_path.read_text(encoding='utf-8'))['rows']
    for row in status['rows']:
        if row['row'] in adoptions: row.update(adoptions[row['row']])
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
