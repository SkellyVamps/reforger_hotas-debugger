from pathlib import Path

path = Path('Scripts/Game/HOTASDebugger/HOTASDebugger.c')
src = path.read_text(encoding='utf-8')

src = src.replace(
'''\tprotected float m_fBackgroundOpacity = 0.55;\n\tprotected float m_fFadeOpacity = 1.0;''',
'''\tprotected float m_fBackgroundOpacity = 0.55;\n\tprotected float m_fFadeOpacity = 1.0;\n\n\t// Raw joystick axis numbers used by this HOTAS. Users can remap these in HOTASHudSettings.txt.\n\tprotected int m_iRollAxis = 0;\n\tprotected int m_iPitchAxis = 1;\n\tprotected int m_iThrottleAxis = 2;\n\tprotected int m_iYawAxis = 5;''',
1,
)

src = src.replace(
'''\t\t\t\tdefaults.WriteLine("background=1");\n\t\t\t\tdefaults.WriteLine("background_opacity=0.55");''',
'''\t\t\t\tdefaults.WriteLine("background=1");\n\t\t\t\tdefaults.WriteLine("background_opacity=0.55");\n\t\t\t\tdefaults.WriteLine("# Raw joystick axis mapping. Set an unused control to -1.");\n\t\t\t\tdefaults.WriteLine("roll_axis=0");\n\t\t\t\tdefaults.WriteLine("pitch_axis=1");\n\t\t\t\tdefaults.WriteLine("throttle_axis=2");\n\t\t\t\tdefaults.WriteLine("yaw_axis=5");''',
1,
)

src = src.replace(
'''\t\t\telse if (key == "background_opacity")\n\t\t\t\tm_fBackgroundOpacity = Math.Clamp(value.ToFloat(0.55), 0.0, 1.0);''',
'''\t\t\telse if (key == "background_opacity")\n\t\t\t\tm_fBackgroundOpacity = Math.Clamp(value.ToFloat(0.55), 0.0, 1.0);\n\t\t\telse if (key == "roll_axis")\n\t\t\t\tm_iRollAxis = Math.ClampInt(value.ToInt(0), -1, 63);\n\t\t\telse if (key == "pitch_axis")\n\t\t\t\tm_iPitchAxis = Math.ClampInt(value.ToInt(1), -1, 63);\n\t\t\telse if (key == "throttle_axis")\n\t\t\t\tm_iThrottleAxis = Math.ClampInt(value.ToInt(2), -1, 63);\n\t\t\telse if (key == "yaw_axis")\n\t\t\t\tm_iYawAxis = Math.ClampInt(value.ToInt(5), -1, 63);''',
1,
)

src = src.replace(
'''\t\tPrint(string.Format("[HOTAS Debugger] HUD settings: position=%1 scale=%2 fade=%3/%4 background=%5 opacity=%6", m_sHudPosition, m_fHudScale, m_iFadeDelayMs, m_iFadeDurationMs, m_bBackgroundEnabled, m_fBackgroundOpacity), LogLevel.NORMAL);''',
'''\t\tPrint(string.Format("[HOTAS Debugger] HUD settings: position=%1 scale=%2 fade=%3/%4 background=%5 opacity=%6 axes R/P/T/Y=%7/%8/%9/%10", m_sHudPosition, m_fHudScale, m_iFadeDelayMs, m_iFadeDurationMs, m_bBackgroundEnabled, m_fBackgroundOpacity, m_iRollAxis, m_iPitchAxis, m_iThrottleAxis, m_iYawAxis), LogLevel.NORMAL);''',
1,
)

old = '''\t\t\t\tint rawAxis = axisText.ToInt();\n\t\t\t\tstring axisName;\n\t\t\t\tswitch (rawAxis)\n\t\t\t\t{\n\t\t\t\t\tcase 0: axisName = "ROLL"; break;\n\t\t\t\t\tcase 1: axisName = "PITCH"; break;\n\t\t\t\t\tcase 2: axisName = "THROTTLE"; break;\n\t\t\t\t\tcase 5: axisName = "YAW"; break;\n\t\t\t\t}\n\n\t\t\t\tif (!axisName.IsEmpty())\n\t\t\t\t\treadable = string.Format("%1 %2", axisName, direction);\n\t\t\t\telse\n\t\t\t\t\treadable = string.Format("AXIS %1%2", rawAxis + 1, direction);'''
new = '''\t\t\t\tint rawAxis = axisText.ToInt();\n\t\t\t\tstring axisName;\n\t\t\t\tif (rawAxis == m_iRollAxis && m_iRollAxis >= 0)\n\t\t\t\t\taxisName = "ROLL";\n\t\t\t\telse if (rawAxis == m_iPitchAxis && m_iPitchAxis >= 0)\n\t\t\t\t\taxisName = "PITCH";\n\t\t\t\telse if (rawAxis == m_iThrottleAxis && m_iThrottleAxis >= 0)\n\t\t\t\t\taxisName = "THROTTLE";\n\t\t\t\telse if (rawAxis == m_iYawAxis && m_iYawAxis >= 0)\n\t\t\t\t\taxisName = "YAW";\n\n\t\t\t\tif (!axisName.IsEmpty())\n\t\t\t\t\treadable = string.Format("%1 %2", axisName, direction);\n\t\t\t\telse\n\t\t\t\t\treadable = string.Format("AXIS %1%2", rawAxis + 1, direction);'''
if old not in src:
    raise SystemExit('readable axis block not found')
src = src.replace(old, new, 1)

path.write_text(src, encoding='utf-8')
