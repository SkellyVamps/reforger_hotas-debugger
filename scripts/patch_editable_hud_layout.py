from pathlib import Path

path = Path('Scripts/Game/HOTASDebugger/HOTASDebugger.c')
src = path.read_text(encoding='utf-8')

src = src.replace(
'''\tprotected RichTextWidget m_DebugText;\n\tprotected Widget m_HudBackground;''',
'''\tprotected RichTextWidget m_DebugText;\n\tprotected Widget m_HudBackground;\n\tprotected Widget m_HudLayoutRoot;\n\tprotected RichTextWidget m_InputText;\n\tprotected RichTextWidget m_SeparatorText;\n\tprotected RichTextWidget m_ActionText;\n\tprotected bool m_bUsingLayoutHud;''',
1,
)

old_shutdown = '''\t\tif (m_DebugText)\n\t\t\tm_DebugText.RemoveFromHierarchy();\n\t\tif (m_HudBackground)\n\t\t\tm_HudBackground.RemoveFromHierarchy();\n\n\t\tm_DebugText = null;\n\t\tm_HudBackground = null;'''
new_shutdown = '''\t\tif (m_HudLayoutRoot)\n\t\t\tm_HudLayoutRoot.RemoveFromHierarchy();\n\t\telse\n\t\t{\n\t\t\tif (m_DebugText)\n\t\t\t\tm_DebugText.RemoveFromHierarchy();\n\t\t\tif (m_HudBackground)\n\t\t\t\tm_HudBackground.RemoveFromHierarchy();\n\t\t}\n\n\t\tm_DebugText = null;\n\t\tm_HudBackground = null;\n\t\tm_HudLayoutRoot = null;\n\t\tm_InputText = null;\n\t\tm_SeparatorText = null;\n\t\tm_ActionText = null;\n\t\tm_bUsingLayoutHud = false;'''
if old_shutdown not in src:
    raise SystemExit('shutdown block not found')
src = src.replace(old_shutdown, new_shutdown, 1)

create_anchor = '''\t\tif (!workspace)\n\t\t{\n\t\t\tPrint("[HOTAS Debugger] Workspace is not available", LogLevel.ERROR);\n\t\t\treturn;\n\t\t}\n\n\t\tint left;'''
create_repl = '''\t\tif (!workspace)\n\t\t{\n\t\t\tPrint("[HOTAS Debugger] Workspace is not available", LogLevel.ERROR);\n\t\t\treturn;\n\t\t}\n\n\t\t// Normal mode prefers the Workbench-editable layout. Until the named widgets are\n\t\t// added in Layout Editor, we safely fall back to the script-created HUD below.\n\t\tif (!m_bDebugMode && TryCreateLayoutHud(workspace))\n\t\t\treturn;\n\n\t\tint left;'''
if create_anchor not in src:
    raise SystemExit('CreateHud workspace anchor not found')
src = src.replace(create_anchor, create_repl, 1)

method_anchor = '''\tprotected void GetHudPosition(WorkspaceWidget workspace, int width, int height, out int left, out int top)'''
method = '''\tprotected bool TryCreateLayoutHud(WorkspaceWidget workspace)\n\t{\n\t\tResourceName hudLayout = "{25F3F1C1A41EA7E1}UI/layouts/HUD/HOTAS/HOTASInputHUD.layout";\n\t\tm_HudLayoutRoot = workspace.CreateWidgets(hudLayout);\n\t\tif (!m_HudLayoutRoot)\n\t\t{\n\t\t\tPrint("[HOTAS Debugger] Could not load HOTASInputHUD.layout; using script HUD", LogLevel.WARNING);\n\t\t\treturn false;\n\t\t}\n\n\t\tm_InputText = RichTextWidget.Cast(m_HudLayoutRoot.FindAnyWidget("InputText"));\n\t\tm_SeparatorText = RichTextWidget.Cast(m_HudLayoutRoot.FindAnyWidget("SeparatorText"));\n\t\tm_ActionText = RichTextWidget.Cast(m_HudLayoutRoot.FindAnyWidget("ActionText"));\n\t\tm_HudBackground = m_HudLayoutRoot.FindAnyWidget("Background");\n\n\t\tif (!m_InputText || !m_SeparatorText || !m_ActionText)\n\t\t{\n\t\t\tPrint("[HOTAS Debugger] HOTASInputHUD.layout is present but needs named RichText widgets: InputText, SeparatorText, ActionText. Using script HUD until the layout is ready.", LogLevel.WARNING);\n\t\t\tm_HudLayoutRoot.RemoveFromHierarchy();\n\t\t\tm_HudLayoutRoot = null;\n\t\t\tm_InputText = null;\n\t\t\tm_SeparatorText = null;\n\t\t\tm_ActionText = null;\n\t\t\tm_HudBackground = null;\n\t\t\treturn false;\n\t\t}\n\n\t\tm_bUsingLayoutHud = true;\n\t\tm_SeparatorText.SetText("|");\n\t\tm_HudLayoutRoot.SetOpacity(0.0);\n\t\tif (m_HudBackground && !m_bBackgroundEnabled)\n\t\t\tm_HudBackground.SetOpacity(0.0);\n\n\t\tPrint("[HOTAS Debugger] Using Workbench-editable HOTASInputHUD.layout", LogLevel.NORMAL);\n\t\treturn true;\n\t}\n\n'''
if method_anchor not in src:
    raise SystemExit('GetHudPosition anchor not found')
src = src.replace(method_anchor, method + method_anchor, 1)

old_show = '''\tprotected void ShowHud()\n\t{\n\t\tif (m_bDebugMode || !m_DebugText)\n\t\t\treturn;\n\n\t\tScriptCallQueue queue = GetGame().GetCallqueue();\n\t\tqueue.Remove(StartFade);\n\t\tqueue.Remove(FadeStep);\n\t\tm_fFadeOpacity = 1.0;\n\t\tm_DebugText.SetOpacity(1.0);\n\t\tif (m_HudBackground)\n\t\t\tm_HudBackground.SetOpacity(m_fBackgroundOpacity);\n\t\tqueue.CallLater(StartFade, m_iFadeDelayMs, false);\n\t}'''
new_show = '''\tprotected void ShowHud()\n\t{\n\t\tif (m_bDebugMode)\n\t\t\treturn;\n\t\tif (m_bUsingLayoutHud && !m_HudLayoutRoot)\n\t\t\treturn;\n\t\tif (!m_bUsingLayoutHud && !m_DebugText)\n\t\t\treturn;\n\n\t\tScriptCallQueue queue = GetGame().GetCallqueue();\n\t\tqueue.Remove(StartFade);\n\t\tqueue.Remove(FadeStep);\n\t\tm_fFadeOpacity = 1.0;\n\n\t\tif (m_bUsingLayoutHud)\n\t\t\tm_HudLayoutRoot.SetOpacity(1.0);\n\t\telse\n\t\t{\n\t\t\tm_DebugText.SetOpacity(1.0);\n\t\t\tif (m_HudBackground)\n\t\t\t\tm_HudBackground.SetOpacity(m_fBackgroundOpacity);\n\t\t}\n\n\t\tqueue.CallLater(StartFade, m_iFadeDelayMs, false);\n\t}'''
if old_show not in src:
    raise SystemExit('ShowHud block not found')
src = src.replace(old_show, new_show, 1)

old_start = '''\tprotected void StartFade()\n\t{\n\t\tif (m_bDebugMode || !m_DebugText)\n\t\t\treturn;\n\n\t\tif (m_iFadeDurationMs <= 0)\n\t\t{\n\t\t\tm_DebugText.SetOpacity(0.0);\n\t\t\tif (m_HudBackground)\n\t\t\t\tm_HudBackground.SetOpacity(0.0);\n\t\t\treturn;\n\t\t}\n\n\t\tm_fFadeOpacity = 1.0;\n\t\tGetGame().GetCallqueue().CallLater(FadeStep, 50, true);\n\t}'''
new_start = '''\tprotected void StartFade()\n\t{\n\t\tif (m_bDebugMode)\n\t\t\treturn;\n\t\tif (m_bUsingLayoutHud && !m_HudLayoutRoot)\n\t\t\treturn;\n\t\tif (!m_bUsingLayoutHud && !m_DebugText)\n\t\t\treturn;\n\n\t\tif (m_iFadeDurationMs <= 0)\n\t\t{\n\t\t\tif (m_bUsingLayoutHud)\n\t\t\t\tm_HudLayoutRoot.SetOpacity(0.0);\n\t\t\telse\n\t\t\t{\n\t\t\t\tm_DebugText.SetOpacity(0.0);\n\t\t\t\tif (m_HudBackground)\n\t\t\t\t\tm_HudBackground.SetOpacity(0.0);\n\t\t\t}\n\t\t\treturn;\n\t\t}\n\n\t\tm_fFadeOpacity = 1.0;\n\t\tGetGame().GetCallqueue().CallLater(FadeStep, 50, true);\n\t}'''
if old_start not in src:
    raise SystemExit('StartFade block not found')
src = src.replace(old_start, new_start, 1)

old_step = '''\tprotected void FadeStep()\n\t{\n\t\tif (!m_DebugText)\n\t\t{\n\t\t\tGetGame().GetCallqueue().Remove(FadeStep);\n\t\t\treturn;\n\t\t}\n\n\t\tm_fFadeOpacity -= 50.0 / m_iFadeDurationMs;\n\t\tif (m_fFadeOpacity <= 0.0)\n\t\t{\n\t\t\tm_fFadeOpacity = 0.0;\n\t\t\tGetGame().GetCallqueue().Remove(FadeStep);\n\t\t}\n\n\t\tm_DebugText.SetOpacity(m_fFadeOpacity);\n\t\tif (m_HudBackground)\n\t\t\tm_HudBackground.SetOpacity(m_fBackgroundOpacity * m_fFadeOpacity);\n\t}'''
new_step = '''\tprotected void FadeStep()\n\t{\n\t\tif (m_bUsingLayoutHud && !m_HudLayoutRoot)\n\t\t{\n\t\t\tGetGame().GetCallqueue().Remove(FadeStep);\n\t\t\treturn;\n\t\t}\n\t\tif (!m_bUsingLayoutHud && !m_DebugText)\n\t\t{\n\t\t\tGetGame().GetCallqueue().Remove(FadeStep);\n\t\t\treturn;\n\t\t}\n\n\t\tm_fFadeOpacity -= 50.0 / m_iFadeDurationMs;\n\t\tif (m_fFadeOpacity <= 0.0)\n\t\t{\n\t\t\tm_fFadeOpacity = 0.0;\n\t\t\tGetGame().GetCallqueue().Remove(FadeStep);\n\t\t}\n\n\t\tif (m_bUsingLayoutHud)\n\t\t\tm_HudLayoutRoot.SetOpacity(m_fFadeOpacity);\n\t\telse\n\t\t{\n\t\t\tm_DebugText.SetOpacity(m_fFadeOpacity);\n\t\t\tif (m_HudBackground)\n\t\t\t\tm_HudBackground.SetOpacity(m_fBackgroundOpacity * m_fFadeOpacity);\n\t\t}\n\t}'''
if old_step not in src:
    raise SystemExit('FadeStep block not found')
src = src.replace(old_step, new_step, 1)

old_action = '''\t\tstring readableAction = MakeReadableActionName(actionName);\n\t\tstring output;\n\t\tif (m_bDebugMode)\n\t\t{\n\t\t\toutput = string.Format(\n\t\t\t\t"HOTAS INPUT DEBUG  #%1\\nInput: %2\\nAction: %3\\nRaw action: %4\\nValue: %5",\n\t\t\t\tm_iEventCounter,\n\t\t\t\tbindingsText,\n\t\t\t\treadableAction,\n\t\t\t\tactionName,\n\t\t\t\tvalue.ToString(2)\n\t\t\t);\n\t\t}\n\t\telse\n\t\t{\n\t\t\toutput = string.Format("<color rgba=\\\"226,167,80,255\\\">%1</color> | <color rgba=\\\"255,255,255,255\\\">%2</color>", MakeReadableBinding(bindingsText), readableAction);\n\t\t}\n\n\t\tif (m_DebugText)\n\t\t{\n\t\t\tm_DebugText.SetText(output);\n\t\t\tShowHud();\n\t\t}'''
new_action = '''\t\tstring readableAction = MakeReadableActionName(actionName);\n\t\tif (m_bDebugMode)\n\t\t{\n\t\t\tstring output = string.Format(\n\t\t\t\t"HOTAS INPUT DEBUG  #%1\\nInput: %2\\nAction: %3\\nRaw action: %4\\nValue: %5",\n\t\t\t\tm_iEventCounter,\n\t\t\t\tbindingsText,\n\t\t\t\treadableAction,\n\t\t\t\tactionName,\n\t\t\t\tvalue.ToString(2)\n\t\t\t);\n\t\t\tif (m_DebugText)\n\t\t\t\tm_DebugText.SetText(output);\n\t\t}\n\t\telse if (m_bUsingLayoutHud)\n\t\t{\n\t\t\tm_InputText.SetText(MakeReadableBinding(bindingsText));\n\t\t\tm_SeparatorText.SetText("|");\n\t\t\tm_ActionText.SetText(readableAction);\n\t\t\tShowHud();\n\t\t}\n\t\telse if (m_DebugText)\n\t\t{\n\t\t\tstring output = string.Format("<color rgba=\\\"226,167,80,255\\\">%1</color> | <color rgba=\\\"255,255,255,255\\\">%2</color>", MakeReadableBinding(bindingsText), readableAction);\n\t\t\tm_DebugText.SetText(output);\n\t\t\tShowHud();\n\t\t}'''
if old_action not in src:
    raise SystemExit('OnActionTriggered formatting block not found')
src = src.replace(old_action, new_action, 1)

path.write_text(src, encoding='utf-8')
