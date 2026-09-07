from pathlib import Path

script_path = Path('Scripts/Game/HOTASDebugger/HOTASSettingsTab.c')
layout_path = Path('UI/layouts/Menus/SettingsSubMenus/HOTASSettings.layout')

src = script_path.read_text(encoding='utf-8')
layout = layout_path.read_text(encoding='utf-8')

old_arrow_calls = '''\t\tRefreshArrowButton(root.FindAnyWidget("ButtonLeft"), selected > 0);\n\t\tRefreshArrowButton(root.FindAnyWidget("ButtonRight"), selected < optionCount - 1);'''
new_arrow_calls = '''\t\t// These selectors cycle, so both arrows stay visible whenever more than one option exists.\n\t\tbool arrowsEnabled = optionCount > 1;\n\t\tRefreshArrowButton(root.FindAnyWidget("ButtonLeft"), arrowsEnabled);\n\t\tRefreshArrowButton(root.FindAnyWidget("ButtonRight"), arrowsEnabled);'''
if old_arrow_calls not in src:
    raise SystemExit('arrow call block not found')
src = src.replace(old_arrow_calls, new_arrow_calls, 1)

old_arrow_func = '''\tprotected void RefreshArrowButton(Widget buttonWidget, bool enabled)\n\t{\n\t\tif (!buttonWidget)\n\t\t\treturn;\n\n\t\tSCR_PagingButtonComponent button = SCR_PagingButtonComponent.Cast(buttonWidget.FindHandler(SCR_PagingButtonComponent));\n\t\tif (button)\n\t\t{\n\t\t\tbutton.SetDisabledOpacity(0.35);\n\t\t\tbutton.SetEnabled(enabled, false);\n\t\t}\n\t\telse\n\t\t{\n\t\t\tbuttonWidget.SetEnabled(enabled);\n\t\t\tif (enabled)\n\t\t\t\tbuttonWidget.SetOpacity(1.0);\n\t\t\telse\n\t\t\t\tbuttonWidget.SetOpacity(0.35);\n\t\t}\n\n\t\t// SCR_PagingButtonComponent hides BackgroundImage when disabled. For settings\n\t\t// selectors we want the normal disabled/grey arrow instead of a disappearing one.\n\t\tWidget background = buttonWidget.FindAnyWidget("BackgroundImage");\n\t\tif (background)\n\t\t\tbackground.SetVisible(true);\n\t}\n'''
new_arrow_func = '''\tprotected void RefreshArrowButton(Widget buttonWidget, bool enabled)\n\t{\n\t\tif (!buttonWidget)\n\t\t\treturn;\n\n\t\tSCR_PagingButtonComponent button = SCR_PagingButtonComponent.Cast(buttonWidget.FindHandler(SCR_PagingButtonComponent));\n\t\tif (button)\n\t\t{\n\t\t\tbutton.SetDisabledOpacity(0.35);\n\t\t\tbutton.SetEnabled(enabled, false);\n\t\t}\n\t\telse\n\t\t{\n\t\t\tbuttonWidget.SetEnabled(enabled);\n\t\t}\n\n\t\t// Paging buttons hide their background when disabled. Force both visual layers\n\t\t// back on after SetEnabled so the arrows never disappear from the settings row.\n\t\tif (enabled)\n\t\t\tbuttonWidget.SetOpacity(1.0);\n\t\telse\n\t\t\tbuttonWidget.SetOpacity(0.35);\n\n\t\tWidget background = buttonWidget.FindAnyWidget("BackgroundImage");\n\t\tif (background)\n\t\t{\n\t\t\tbackground.SetVisible(true);\n\t\t\tbackground.SetOpacity(1.0);\n\t\t}\n\n\t\tWidget panel = buttonWidget.FindAnyWidget("Panel");\n\t\tif (panel)\n\t\t{\n\t\t\tpanel.SetVisible(true);\n\t\t\tpanel.SetOpacity(1.0);\n\t\t}\n\t}\n'''
if old_arrow_func not in src:
    raise SystemExit('arrow function block not found')
src = src.replace(old_arrow_func, new_arrow_func, 1)

src = src.replace('''\t\tif (previewHudWidth < 8)\n\t\t\tpreviewHudWidth = 8;\n\t\tif (previewHudHeight < 5)\n\t\t\tpreviewHudHeight = 5;''', '''\t\t// Keep the preview handle large enough to grab reliably with the mouse.\n\t\tif (previewHudWidth < 20)\n\t\t\tpreviewHudWidth = 20;\n\t\tif (previewHudHeight < 12)\n\t\t\tpreviewHudHeight = 12;''', 1)

# Use the normal large paging arrows and cycle every multi-option selector. This avoids
# the light-arrow prefab state that could leave the arrow graphics invisible.
layout = layout.replace('m_bUseLightArrows 1', 'm_bUseLightArrows 0')
layout = layout.replace('m_bShowHints 0', 'm_bCycleMode 1 m_bShowHints 0')

old_preview = '''          ImageWidgetClass "{8C52D9F7A31B6514}" {\n           Name "HUDPositionPreview"\n           Slot FrameWidgetSlot "{8C52D9F7A31B6515}" {\n            PositionX 120\n            PositionY 205\n            SizeX 120\n            SizeY 16\n            Alignment 0 0\n           }\n           Color 0.7605 0.3865 0.0802 1\n           Size 1024 1024\n          }'''
new_preview = '''          ButtonWidgetClass "{8C52D9F7A31B6514}" {\n           Name "HUDPositionPreview"\n           Slot FrameWidgetSlot "{8C52D9F7A31B6515}" {\n            PositionX 120\n            PositionY 205\n            SizeX 120\n            SizeY 16\n            Alignment 0 0\n           }\n           {\n            ImageWidgetClass "{8C52D9F7A31B6516}" {\n             Name "HUDPositionPreviewFill"\n             Slot ButtonWidgetSlot "{8C52D9F7A31B6517}" {\n              HorizontalAlign 3\n              VerticalAlign 3\n             }\n             Color 0.7605 0.3865 0.0802 1\n             Size 1024 1024\n            }\n           }\n          }'''
if old_preview not in layout:
    raise SystemExit('HUD preview widget block not found')
layout = layout.replace(old_preview, new_preview, 1)

script_path.write_text(src, encoding='utf-8')
layout_path.write_text(layout, encoding='utf-8')
