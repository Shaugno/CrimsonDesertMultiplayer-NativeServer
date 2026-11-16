"""Normalize small committed text exports and retain original local hashes."""
import hashlib
import json
from pathlib import Path

root = Path(__file__).resolve().parents[2]
folder = root/'docs/probe-3663/port-evidence/r2'
rows = []
for destination in sorted(folder.iterdir()):
    if destination.suffix not in ('.txt','.log'): continue
    source = root/'build/r2'/destination.name
    if destination.name=='checkpoint-loopback.log': source = root/'build/e2e/cdmp.log'
    assert source.exists(),source
    raw = source.read_bytes()
    assert len(raw)<=5*1024*1024,source
    normalized = ('\n'.join(s.rstrip() for s in raw.decode('utf-8-sig').splitlines()).rstrip()+'\n').encode('utf-8')
    destination.write_bytes(normalized)
    rows.append(dict(local=str(source),repository=str(destination.relative_to(root)),raw_bytes=len(raw),
        raw_sha256=hashlib.sha256(raw).hexdigest(),normalized_sha256=hashlib.sha256(normalized).hexdigest()))
(folder/'text-hashes.json').write_text(json.dumps(dict(normalization='UTF-8, LF, strip trailing whitespace, one final newline',files=rows),indent=2)+'\n')
print('Normalized and hashed',len(rows),'text evidence files')
