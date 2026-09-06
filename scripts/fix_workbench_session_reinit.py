from pathlib import Path

path = Path('Scripts/Game/HOTASDebugger/HOTASDebugger.c')
src = path.read_text(encoding='utf-8')

old = '''\tvoid Initialize()\n\t{\n\t\tif (m_bInitialized)\n\t\t\treturn;\n'''
new = '''\tvoid Initialize()\n\t{\n\t\tif (m_bInitialized)\n\t\t{\n\t\t\tPrint("[HOTAS Debugger] Reinitializing for new play session", LogLevel.NORMAL);\n\t\t\tShutdown();\n\t\t}\n'''
if old not in src:
    raise SystemExit('Initialize guard not found')
src = src.replace(old, new, 1)

old = '''\tvoid Shutdown()\n\t{\n\t\tif (!m_bInitialized || !m_InputManager)\n\t\t\treturn;\n\n\t\tforeach (string actionName : m_WatchedActions)\n\t\t\tm_InputManager.RemoveActionListener(actionName, EActionTrigger.DOWN, OnActionTriggered);\n\n\t\tif (m_DebugText)\n\t\t\tm_DebugText.RemoveFromHierarchy();\n\n\t\tm_DebugText = null;\n\t\tm_InputBinding = null;\n\t\tm_InputManager = null;\n\t\tm_bInitialized = false;\n\t}\n'''
new = '''\tvoid Shutdown()\n\t{\n\t\tif (!m_bInitialized)\n\t\t\treturn;\n\n\t\tif (m_InputManager)\n\t\t{\n\t\t\tforeach (string actionName : m_WatchedActions)\n\t\t\t\tm_InputManager.RemoveActionListener(actionName, EActionTrigger.DOWN, OnActionTriggered);\n\t\t}\n\n\t\tif (m_DebugText)\n\t\t\tm_DebugText.RemoveFromHierarchy();\n\n\t\tm_DebugText = null;\n\t\tm_InputBinding = null;\n\t\tm_InputManager = null;\n\t\tm_bInitialized = false;\n\t}\n'''
if old not in src:
    raise SystemExit('Shutdown block not found')
src = src.replace(old, new, 1)

path.write_text(src, encoding='utf-8')
