from pathlib import Path

path = Path('Scripts/Game/HOTASDebugger/HOTASDebugger.c')
src = path.read_text(encoding='utf-8')
old = 'return "Action fired, but active joystick binding lookup returned nothing";'
new = 'return "Non-Joystick Input";'
if old not in src:
    raise SystemExit('target string not found')
src = src.replace(old, new, 1)
path.write_text(src, encoding='utf-8')
