from pathlib import Path

path = Path('Scripts/Game/HOTASDebugger/HOTASDebugger.c')
src = path.read_text(encoding='utf-8')
old = 'output = string.Format("<color rgba="226,167,80,255">%1</color>   |   <color rgba="255,255,255,255">%2</color>", MakeReadableBinding(bindingsText), readableAction);'
new = 'output = string.Format("<color rgba=\\\"226,167,80,255\\\">%1</color> | <color rgba=\\\"255,255,255,255\\\">%2</color>", MakeReadableBinding(bindingsText), readableAction);'
if old not in src:
    raise SystemExit('broken RichText output line not found')
path.write_text(src.replace(old, new, 1), encoding='utf-8')
