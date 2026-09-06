from pathlib import Path

path = Path('Scripts/Game/HOTASDebugger/HOTASSettingsTab.c')
src = path.read_text(encoding='utf-8')

old = '''\t\tif (index <= 0)\n\t\t{\n\t\t\tarray<ResourceName> emptyConfigs = {};\n\t\t\tbinding.SetCustomConfigs(emptyConfigs);\n\t\t\tbinding.Save();\n\t\t\tPrint("[HOTAS Debugger] Custom HOTAS input config cleared from Settings", LogLevel.NORMAL);\n\t\t\treturn;\n\t\t}\n'''
new = '''\t\tif (index <= 0)\n\t\t{\n\t\t\tarray<ResourceName> emptyConfigs = {};\n\t\t\tbinding.SetCustomConfigs(emptyConfigs);\n\t\t\tbinding.Save();\n\t\t\tRefreshSpinBoxArrows(component, index, m_UserConfigs.Count() + 1);\n\t\t\tGetGame().GetCallqueue().CallLater(RefreshAllSpinBoxArrows, 0, false);\n\t\t\tPrint("[HOTAS Debugger] Custom HOTAS input config cleared from Settings", LogLevel.NORMAL);\n\t\t\treturn;\n\t\t}\n'''
if src.count(old) != 1:
    raise RuntimeError('HOTAS config clear block mismatch')
src = src.replace(old, new, 1)

old = '''\t\tstring selectedConfig = m_UserConfigs.Get(configIndex);\n\t\tkeybindModule.SelectJoystickPresetPath(selectedConfig);\n\t\tPrint(string.Format("[HOTAS Debugger] HOTAS input config selected from Settings: %1", selectedConfig), LogLevel.NORMAL);\n'''
new = '''\t\tstring selectedConfig = m_UserConfigs.Get(configIndex);\n\t\tkeybindModule.SelectJoystickPresetPath(selectedConfig);\n\t\tRefreshSpinBoxArrows(component, index, m_UserConfigs.Count() + 1);\n\t\tGetGame().GetCallqueue().CallLater(RefreshAllSpinBoxArrows, 0, false);\n\t\tPrint(string.Format("[HOTAS Debugger] HOTAS input config selected from Settings: %1", selectedConfig), LogLevel.NORMAL);\n'''
if src.count(old) != 1:
    raise RuntimeError('HOTAS config selection block mismatch')
src = src.replace(old, new, 1)

old = '''\t\tSCR_PagingButtonComponent button = SCR_PagingButtonComponent.Cast(buttonWidget.FindHandler(SCR_PagingButtonComponent));\n\t\tif (button)\n\t\t\tbutton.SetEnabled(enabled, false);\n\t\telse\n\t\t\tbuttonWidget.SetEnabled(enabled);\n\n\t\t// SCR_PagingButtonComponent hides BackgroundImage when disabled. For settings\n\t\t// selectors we want the normal disabled/grey arrow instead of a disappearing one.\n\t\tWidget background = buttonWidget.FindAnyWidget("BackgroundImage");\n\t\tif (background)\n\t\t\tbackground.SetVisible(true);\n\n\t\tbuttonWidget.SetOpacity(enabled ? 1.0 : 0.35);\n'''
new = '''\t\tSCR_PagingButtonComponent button = SCR_PagingButtonComponent.Cast(buttonWidget.FindHandler(SCR_PagingButtonComponent));\n\t\tif (button)\n\t\t{\n\t\t\tbutton.SetDisabledOpacity(0.35);\n\t\t\tbutton.SetEnabled(enabled, false);\n\t\t}\n\t\telse\n\t\t{\n\t\t\tbuttonWidget.SetEnabled(enabled);\n\t\t\tif (enabled)\n\t\t\t\tbuttonWidget.SetOpacity(1.0);\n\t\t\telse\n\t\t\t\tbuttonWidget.SetOpacity(0.35);\n\t\t}\n\n\t\t// SCR_PagingButtonComponent hides BackgroundImage when disabled. For settings\n\t\t// selectors we want the normal disabled/grey arrow instead of a disappearing one.\n\t\tWidget background = buttonWidget.FindAnyWidget("BackgroundImage");\n\t\tif (background)\n\t\t\tbackground.SetVisible(true);\n'''
if src.count(old) != 1:
    raise RuntimeError('arrow visual block mismatch')
src = src.replace(old, new, 1)

path.write_text(src, encoding='utf-8')
