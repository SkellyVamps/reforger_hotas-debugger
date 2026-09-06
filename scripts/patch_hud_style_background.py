from pathlib import Path

path = Path('Scripts/Game/HOTASDebugger/HOTASDebugger.c')
src = path.read_text(encoding='utf-8')

src = src.replace('protected TextWidget m_DebugText;', 'protected RichTextWidget m_DebugText;', 1)

src = src.replace(
'''\t\t\t// Leave extra horizontal room so longer readable action labels are not clipped.\n\t\t\twidth = Math.Round(1040 * m_fHudScale);\n\t\t\theight = Math.Round(72 * m_fHudScale);''',
'''\t\t\t// Keep the HUD wide enough for readable labels, but always inside the current viewport.\n\t\t\tint horizontalPadding = Math.Round(24 * m_fHudScale);\n\t\t\twidth = Math.Min(Math.Round(1360 * m_fHudScale), workspace.GetWidth() - horizontalPadding);\n\t\t\theight = Math.Round(72 * m_fHudScale);''',
1,
)

old_bg = '''\t\t\tif (m_bBackgroundEnabled)\n\t\t\t{\n\t\t\t\tm_HudBackground = workspace.CreateWidgetInWorkspace(\n\t\t\t\t\tWidgetType.PanelWidgetTypeID,\n\t\t\t\t\tleft,\n\t\t\t\t\ttop,\n\t\t\t\t\twidth,\n\t\t\t\t\theight,\n\t\t\t\t\tWidgetFlags.VISIBLE | WidgetFlags.IGNORE_CURSOR | WidgetFlags.NOFOCUS,\n\t\t\t\t\tColor.FromInt(0xFF101418),\n\t\t\t\t\t999\n\t\t\t\t);\n\t\t\t\tif (m_HudBackground)\n\t\t\t\t\tm_HudBackground.SetOpacity(m_fBackgroundOpacity);\n\t\t\t}'''
new_bg = '''\t\t\tif (m_bBackgroundEnabled)\n\t\t\t{\n\t\t\t\t// PanelWidget is only a container and does not draw a visible fill by itself.\n\t\t\t\t// A full ProgressBar gives us a reliable colorable rectangle for the HUD backdrop.\n\t\t\t\tm_HudBackground = workspace.CreateWidgetInWorkspace(\n\t\t\t\t\tWidgetType.ProgressBarWidgetTypeID,\n\t\t\t\t\tleft,\n\t\t\t\t\ttop,\n\t\t\t\t\twidth,\n\t\t\t\t\theight,\n\t\t\t\t\tWidgetFlags.VISIBLE | WidgetFlags.IGNORE_CURSOR | WidgetFlags.NOFOCUS,\n\t\t\t\t\tColor.FromInt(0xFF101418),\n\t\t\t\t\t999\n\t\t\t\t);\n\t\t\t\tProgressBarWidget backgroundBar = ProgressBarWidget.Cast(m_HudBackground);\n\t\t\t\tif (backgroundBar)\n\t\t\t\t{\n\t\t\t\t\tbackgroundBar.SetMin(0.0);\n\t\t\t\t\tbackgroundBar.SetMax(1.0);\n\t\t\t\t\tbackgroundBar.SetCurrent(1.0);\n\t\t\t\t}\n\t\t\t\tif (m_HudBackground)\n\t\t\t\t\tm_HudBackground.SetOpacity(m_fBackgroundOpacity);\n\t\t\t}'''
if old_bg not in src:
    raise SystemExit('background block not found')
src = src.replace(old_bg, new_bg, 1)

src = src.replace('WidgetType.TextWidgetTypeID,', 'WidgetType.RichTextWidgetTypeID,', 1)
src = src.replace('m_DebugText = TextWidget.Cast(widget);', 'm_DebugText = RichTextWidget.Cast(widget);', 1)

old_output = 'output = string.Format("%1   •   %2", MakeReadableBinding(bindingsText), readableAction);'
new_output = 'output = string.Format("<color rgba=\"226,167,80,255\">%1</color>   |   <color rgba=\"255,255,255,255\">%2</color>", MakeReadableBinding(bindingsText), readableAction);'
if old_output not in src:
    raise SystemExit('normal output format not found')
src = src.replace(old_output, new_output, 1)

path.write_text(src, encoding='utf-8')
