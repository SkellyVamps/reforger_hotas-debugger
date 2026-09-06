from pathlib import Path
path = Path('Scripts/Game/HOTASDebugger/HOTASDebugger.c')
src = path.read_text(encoding='utf-8')
src = src.replace('\\t', '\t')
src = src.replace('\\\\n', '\\n')
path.write_text(src, encoding='utf-8')
