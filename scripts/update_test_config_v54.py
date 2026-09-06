from pathlib import Path

path = Path('Scripts/Game/HOTASDebugger/HOTASDebugger.c')
src = path.read_text(encoding='utf-8')
old = 'Solr1 v5.3.conf'
new = 'Solr1 v5.4.conf'
if old not in src:
    raise SystemExit(f'{old} not found')
path.write_text(src.replace(old, new, 1), encoding='utf-8')
