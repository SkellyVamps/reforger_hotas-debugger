from pathlib import Path

path = Path('Scripts/Game/HOTASDebugger/HOTASDebugger.c')
src = path.read_text(encoding='utf-8')

old = '''\t\tm_InputBinding.SetCustomConfigs(customConfigs);\n\n\t\tPrint(string.Format(\"[HOTAS Debugger] Workbench test config requested: %1\", testConfig), LogLevel.NORMAL);\n'''
new = '''\t\tm_InputBinding.SetCustomConfigs(customConfigs);\n\t\tm_InputBinding.Save();\n\n\t\tref array<ResourceName> activeConfigs = {};\n\t\tm_InputBinding.GetCustomConfigs(activeConfigs);\n\t\tPrint(string.Format(\"[HOTAS Debugger] Workbench test config requested: %1 | active custom configs=%2\", testConfig, activeConfigs.Count()), LogLevel.NORMAL);\n'''
if old not in src:
    raise SystemExit('Workbench config block not found')
src = src.replace(old, new, 1)

old = '''\t\tbool found = m_InputBinding.GetBindings(actionName, bindings, EInputDeviceType.INVALID, string.Empty, false);\n'''
new = '''\t\tbool found = m_InputBinding.GetBindings(actionName, bindings, EInputDeviceType.JOYSTICK, string.Empty, false);\n'''
if old not in src:
    raise SystemExit('GetBindings device block not found')
src = src.replace(old, new, 1)

path.write_text(src, encoding='utf-8')
