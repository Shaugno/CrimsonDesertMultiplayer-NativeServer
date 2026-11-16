"""Normalize small committed text exports and retain original local hashes."""
import hashlib
import json
from pathlib import Path

root = Path(__file__).resolve().parents[2]
folder = root/'docs/probe-3663/port-evidence/r2'
manifest = folder/'text-hashes.json'
previous = json.loads(manifest.read_text(encoding='utf-8')) if manifest.exists() else {'files': []}
sources = {Path(row['repository']).name: Path(row['local']) for row in previous['files']}
rows = []
exports = []
for destination in sorted(folder.iterdir()):
    if destination.suffix not in ('.txt','.log'): continue
    # Some native tools emit outside build/r2. Preserve the original source
    # recorded by the capture rather than treating a normalized copy as raw.
    source = sources.get(destination.name, root/'build/r2'/destination.name)
    assert source.exists(),source
    raw = source.read_bytes()
    assert len(raw)<=5*1024*1024,source
    normalized = ('\n'.join(s.rstrip() for s in raw.decode('utf-8-sig').splitlines()).rstrip()+'\n').encode('utf-8')
    exports.append((destination, normalized))
    rows.append(dict(local=str(source),repository=str(destination.relative_to(root)),raw_bytes=len(raw),
        raw_sha256=hashlib.sha256(raw).hexdigest(),normalized_sha256=hashlib.sha256(normalized).hexdigest()))
# Validate every original before updating any normalized evidence.
for destination, normalized in exports:
    destination.write_bytes(normalized)
(folder/'text-hashes.json').write_text(json.dumps(dict(normalization='UTF-8, LF, strip trailing whitespace, one final newline',files=rows),indent=2)+'\n',encoding='utf-8')
print('Normalized and hashed',len(rows),'text evidence files')
