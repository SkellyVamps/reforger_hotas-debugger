from pathlib import Path

path = Path('Scripts/Game/HOTASDebugger/HOTASDebugger.c')
src = path.read_text(encoding='utf-8')
old = '''\t\tm_InputBinding = m_InputManager.CreateUserBinding();\n\t\tBuildActionList();\n'''
new = '''\t\tm_InputBinding = m_InputManager.CreateUserBinding();\n\n#ifdef WORKBENCH\n\t\tLoadWorkbenchTestConfig();\n#endif\n\n\t\tBuildActionList();\n'''
if old not in src:
    raise SystemExit('Initialize insertion point not found')
src = src.replace(old, new, 1)

marker = '''\tprotected void RegisterListeners()\n\t{\n'''
method = '''#ifdef WORKBENCH\n\tprotected void LoadWorkbenchTestConfig()\n\t{\n\t\tif (!m_InputBinding)\n\t\t\treturn;\n\n\t\tref array<ResourceName> customConfigs = {};\n\t\tResourceName testConfig = \"$profile:.save/settings/customInputConfigs/Solr1 v5.3.conf\";\n\t\tcustomConfigs.Insert(testConfig);\n\t\tm_InputBinding.SetCustomConfigs(customConfigs);\n\n\t\tPrint(string.Format(\"[HOTAS Debugger] Workbench test config requested: %1\", testConfig), LogLevel.NORMAL);\n\t}\n#endif\n\n'''
if marker not in src:
    raise SystemExit('RegisterListeners marker not found')
src = src.replace(marker, method + marker, 1)
path.write_text(src, encoding='utf-8')
