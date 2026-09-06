from pathlib import Path

path = Path('Scripts/Game/HOTASDebugger/HOTASDebugger.c')
src = path.read_text(encoding='utf-8')

old = '''\t\t\telse if (axisPos >= 0)\n\t\t\t{\n\t\t\t\tstring axisText = binding.Substring(axisPos + 5, binding.Length() - axisPos - 5);\n\t\t\t\tstring direction;\n\t\t\t\tif (axisText.EndsWith("+"))\n\t\t\t\t\tdirection = "+";\n\t\t\t\telse if (axisText.EndsWith("-"))\n\t\t\t\t\tdirection = "-";\n\t\t\t\tint axisNumber = axisText.ToInt() + 1;\n\t\t\t\treadable = string.Format("AXIS %1%2", axisNumber, direction);\n\t\t\t}\n'''

new = '''\t\t\telse if (axisPos >= 0)\n\t\t\t{\n\t\t\t\tstring axisText = binding.Substring(axisPos + 5, binding.Length() - axisPos - 5);\n\t\t\t\tstring direction;\n\t\t\t\tif (axisText.EndsWith("+"))\n\t\t\t\t\tdirection = "+";\n\t\t\t\telse if (axisText.EndsWith("-"))\n\t\t\t\t\tdirection = "-";\n\n\t\t\t\tint rawAxis = axisText.ToInt();\n\t\t\t\tstring axisName;\n\t\t\t\tswitch (rawAxis)\n\t\t\t\t{\n\t\t\t\t\tcase 0: axisName = "ROLL"; break;\n\t\t\t\t\tcase 1: axisName = "PITCH"; break;\n\t\t\t\t\tcase 2: axisName = "THROTTLE"; break;\n\t\t\t\t\tcase 5: axisName = "YAW"; break;\n\t\t\t\t}\n\n\t\t\t\tif (!axisName.IsEmpty())\n\t\t\t\t\treadable = string.Format("%1 %2", axisName, direction);\n\t\t\t\telse\n\t\t\t\t\treadable = string.Format("AXIS %1%2", rawAxis + 1, direction);\n\t\t\t}\n'''

if old not in src:
    raise SystemExit('axis formatting block not found')

path.write_text(src.replace(old, new, 1), encoding='utf-8')
