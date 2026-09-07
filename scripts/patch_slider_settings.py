from pathlib import Path

path = Path('Scripts/Game/HOTASDebugger/HOTASDebugger.c')
src = path.read_text(encoding='utf-8')

replacements = {
    'm_fHudScale = Math.Clamp(value.ToFloat(1.0), 0.5, 2.0);': 'm_fHudScale = Math.Clamp(value.ToFloat(1.0), 0.6, 2.0);',
    'case 2: return 16;': 'case 2: return 15;',
    'case 2: return Math.ClampInt(Math.Round((m_fHudScale - 0.5) * 10.0), 0, 15);': 'case 2: return Math.ClampInt(Math.Round((m_fHudScale - 0.6) * 10.0), 0, 14);',
    'case 2: return string.Format("%1x", (0.5 + optionIndex * 0.1).ToString(1));': 'case 2: return string.Format("%1x", (0.6 + optionIndex * 0.1).ToString(1));',
    'case 2: m_fHudScale = 0.5 + optionIndex * 0.1; break;': 'case 2: m_fHudScale = 0.6 + optionIndex * 0.1; break;',
}

for old, new in replacements.items():
    if old not in src:
        raise SystemExit(f'Missing expected text: {old}')
    src = src.replace(old, new, 1)

needle = '''\tvoid ReloadHudSettings()\n\t{\n\t\tLoadHudSettings();\n\t}\n'''
insert = '''\tvoid ReloadHudSettings()\n\t{\n\t\tLoadHudSettings();\n\t}\n\n\t// Settings-tab slider values are human-facing percentages. HUD scale maps\n\t// 0% -> 0.6x and 100% -> 2.0x, while opacity maps directly to 0..1.\n\tfloat GetHudScalePercent()\n\t{\n\t\treturn Math.Clamp(((m_fHudScale - 0.6) / 1.4) * 100.0, 0.0, 100.0);\n\t}\n\n\tvoid SetHudScalePercent(float percent)\n\t{\n\t\tpercent = Math.Clamp(percent, 0.0, 100.0);\n\t\tm_fHudScale = 0.6 + (percent / 100.0) * 1.4;\n\t\tSaveHudSettings();\n\t\tif (m_bInitialized)\n\t\t\tRebuildHud();\n\t}\n\n\tfloat GetBackgroundOpacityPercent()\n\t{\n\t\treturn Math.Clamp(m_fBackgroundOpacity * 100.0, 0.0, 100.0);\n\t}\n\n\tvoid SetBackgroundOpacityPercent(float percent)\n\t{\n\t\tpercent = Math.Clamp(percent, 0.0, 100.0);\n\t\tm_fBackgroundOpacity = percent / 100.0;\n\t\tSaveHudSettings();\n\t\tif (m_bInitialized)\n\t\t\tRebuildHud();\n\t}\n'''
if needle not in src:
    raise SystemExit('ReloadHudSettings block not found')
src = src.replace(needle, insert, 1)

path.write_text(src, encoding='utf-8')
