from pathlib import Path

path = Path('Scripts/Game/HOTASDebugger/HOTASDebugger.c')
src = path.read_text(encoding='utf-8')

src = src.replace(
"\tprotected bool m_bInitialized;\n\tprotected int m_iEventCounter;",
"\tprotected bool m_bInitialized;\n\tprotected bool m_bDebugMode = false;\n\tprotected int m_iEventCounter;",
1,
)

old_create = '''\tprotected void CreateHud()\n\t{\n\t\tWorkspaceWidget workspace = GetGame().GetWorkspace();\n\t\tif (!workspace)\n\t\t{\n\t\t\tPrint(\"[HOTAS Debugger] Workspace is not available\", LogLevel.ERROR);\n\t\t\treturn;\n\t\t}\n\n\t\tWidget widget = workspace.CreateWidgetInWorkspace(\n\t\t\tWidgetType.TextWidgetTypeID,\n\t\t\t40,\n\t\t\t120,\n\t\t\t900,\n\t\t\t180,\n\t\t\tWidgetFlags.VISIBLE | WidgetFlags.IGNORE_CURSOR | WidgetFlags.NOFOCUS | WidgetFlags.WRAP_TEXT,\n\t\t\tColor.White,\n\t\t\t1000\n\t\t);\n\n\t\tm_DebugText = TextWidget.Cast(widget);\n\t\tif (!m_DebugText)\n\t\t{\n\t\t\tPrint(\"[HOTAS Debugger] Could not create TextWidget\", LogLevel.ERROR);\n\t\t\treturn;\n\t\t}\n\n\t\tm_DebugText.SetExactFontSize(24);\n\t\tm_DebugText.SetBold(true);\n\t\tm_DebugText.SetOutline(2, 0xFF000000);\n\t\tm_DebugText.SetTextWrapping(true);\n\t\tm_DebugText.SetText(\"HOTAS INPUT DEBUG\\nWaiting for a watched input action...\");\n\t}\n'''

new_create = '''\tprotected void CreateHud()\n\t{\n\t\tWorkspaceWidget workspace = GetGame().GetWorkspace();\n\t\tif (!workspace)\n\t\t{\n\t\t\tPrint(\"[HOTAS Debugger] Workspace is not available\", LogLevel.ERROR);\n\t\t\treturn;\n\t\t}\n\n\t\tint left;\n\t\tint top;\n\t\tint width;\n\t\tint height;\n\t\tint flags = WidgetFlags.VISIBLE | WidgetFlags.IGNORE_CURSOR | WidgetFlags.NOFOCUS | WidgetFlags.NO_LOCALIZATION;\n\n\t\tif (m_bDebugMode)\n\t\t{\n\t\t\tleft = 40;\n\t\t\ttop = 120;\n\t\t\twidth = 900;\n\t\t\theight = 180;\n\t\t\tflags |= WidgetFlags.WRAP_TEXT;\n\t\t}\n\t\telse\n\t\t{\n\t\t\twidth = 900;\n\t\t\theight = 92;\n\t\t\tleft = (workspace.GetWidth() - width) / 2;\n\t\t\ttop = workspace.GetHeight() - height - 70;\n\t\t\tflags |= WidgetFlags.CENTER | WidgetFlags.VCENTER;\n\t\t}\n\n\t\tWidget widget = workspace.CreateWidgetInWorkspace(\n\t\t\tWidgetType.TextWidgetTypeID,\n\t\t\tleft,\n\t\t\ttop,\n\t\t\twidth,\n\t\t\theight,\n\t\t\tflags,\n\t\t\tColor.White,\n\t\t\t1000\n\t\t);\n\n\t\tm_DebugText = TextWidget.Cast(widget);\n\t\tif (!m_DebugText)\n\t\t{\n\t\t\tPrint(\"[HOTAS Debugger] Could not create TextWidget\", LogLevel.ERROR);\n\t\t\treturn;\n\t\t}\n\n\t\tif (m_bDebugMode)\n\t\t{\n\t\t\tm_DebugText.SetExactFontSize(24);\n\t\t\tm_DebugText.SetOutline(2, 0xFF000000);\n\t\t\tm_DebugText.SetTextWrapping(true);\n\t\t\tm_DebugText.SetText(\"HOTAS INPUT DEBUG\\nWaiting for a watched input action...\");\n\t\t}\n\t\telse\n\t\t{\n\t\t\tm_DebugText.SetExactFontSize(28);\n\t\t\tm_DebugText.SetOutline(3, 0xE0000000);\n\t\t\tm_DebugText.SetShadow(2, 0xB0000000, 1.0, 2, 2);\n\t\t\tm_DebugText.SetTextWrapping(false);\n\t\t\tm_DebugText.SetText(\"HOTAS INPUT HUD\");\n\t\t}\n\n\t\tm_DebugText.SetBold(true);\n\t}\n'''

if old_create not in src:
    raise SystemExit('CreateHud block not found')
src = src.replace(old_create, new_create, 1)

old_output = '''\t\tstring output = string.Format(\n\t\t\t\"HOTAS INPUT DEBUG  #%1\\nInput: %2\\nAction: %3\\nRaw action: %4\\nValue: %5\",\n\t\t\tm_iEventCounter,\n\t\t\tbindingsText,\n\t\t\treadableAction,\n\t\t\tactionName,\n\t\t\tvalue.ToString(2)\n\t\t);\n'''

new_output = '''\t\tstring output;\n\t\tif (m_bDebugMode)\n\t\t{\n\t\t\toutput = string.Format(\n\t\t\t\t\"HOTAS INPUT DEBUG  #%1\\nInput: %2\\nAction: %3\\nRaw action: %4\\nValue: %5\",\n\t\t\t\tm_iEventCounter,\n\t\t\t\tbindingsText,\n\t\t\t\treadableAction,\n\t\t\t\tactionName,\n\t\t\t\tvalue.ToString(2)\n\t\t\t);\n\t\t}\n\t\telse\n\t\t{\n\t\t\toutput = string.Format(\"%1   |   %2\", bindingsText, readableAction);\n\t\t}\n'''

if old_output not in src:
    raise SystemExit('OnAction output block not found')
src = src.replace(old_output, new_output, 1)

path.write_text(src, encoding='utf-8')
