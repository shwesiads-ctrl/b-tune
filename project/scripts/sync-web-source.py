#!/usr/bin/env python3
"""Copy readable DSP and downloadable source snapshot into the workbench."""
from pathlib import Path
import json, re, shutil, zipfile
root = Path(__file__).resolve().parent.parent
public = root / 'Web/public'
public.mkdir(parents=True, exist_ok=True)
shutil.copy2(root/'Source/VocalDSP.h', public/'VocalDSP.h')
rows = []
for m in re.finditer(r'\{"([^"]+)","([^"]+)",([^{}]+)\}', (root/'Source/Parameters.h').read_text()):
    n = m[3].replace('f', '').split(',')
    if len(n) == 5:
        rows.append(dict(id=m[1],name=m[2],lo=float(n[0]),hi=float(n[1]),step=float(n[2]),initial=float(n[3]),page=int(n[4])))
(root/'Web/src/parameters.json').write_text(json.dumps(rows, indent=2))
with zipfile.ZipFile(public/'MataleTune_Source.zip','w',zipfile.ZIP_DEFLATED) as z:
    for p in sorted(root.rglob('*')):
        rel=p.relative_to(root)
        if not p.is_file() or any(x in ('node_modules','dist','.git','__pycache__') or x.startswith('build') for x in rel.parts[:-1]):
            continue
        if p.suffix in ('.zip','.log','.pyc'):
            continue
        z.write(p,'MataleTuneVST3/'+str(rel))
print('Updated shared parameters, readable DSP and source ZIP.')
