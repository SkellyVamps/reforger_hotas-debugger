from pathlib import Path

path = Path('Scripts/Game/HOTASDebugger/HOTASDebugger.c')
src = path.read_text(encoding='utf-8')

old = '''\t\telse\n\t\t{\n\t\t\twidth = Math.Round(760 * m_fHudScale);\n\t\t\theight = Math.Round(72 * m_fHudScale);'''
new = '''\t\telse\n\t\t{\n\t\t\t// Leave extra horizontal room so longer readable action labels are not clipped.\n\t\t\twidth = Math.Round(1040 * m_fHudScale);\n\t\t\theight = Math.Round(72 * m_fHudScale);'''
if old not in src:
    raise SystemExit('normal HUD size block not found')
src = src.replace(old, new, 1)

path.write_text(src, encoding='utf-8')
