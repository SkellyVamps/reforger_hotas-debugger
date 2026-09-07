from pathlib import Path

# --- HOTASDebugger.c ---------------------------------------------------------
path = Path('Scripts/Game/HOTASDebugger/HOTASDebugger.c')
src = path.read_text(encoding='utf-8')

replacements = {
    'm_fHudScale = Math.Clamp(value.ToFloat(1.0), 0.5, 2.0);': 'm_fHudScale = Math.Clamp(value.ToFloat(1.0), 0.6, 2.0);',
    'case 1: return 16;': 'case 1: return 15;',
    'case 1: return Math.ClampInt(Math.Round((m_fHudScale - 0.5) * 10.0), 0, 15);': 'case 1: return Math.ClampInt(Math.Round((m_fHudScale - 0.6) * 10.0), 0, 14);',
    'case 1: return string.Format("%1x", (0.5 + optionIndex * 0.1).ToString(1));': 'case 1: return string.Format("%1x", (0.6 + optionIndex * 0.1).ToString(1));',
    'case 1: m_fHudScale = 0.5 + optionIndex * 0.1; break;': 'case 1: m_fHudScale = 0.6 + optionIndex * 0.1; break;',
}
for old, new in replacements.items():
    if old not in src:
        raise SystemExit(f'HOTASDebugger.c missing expected text: {old}')
    src = src.replace(old, new, 1)

needle = '''\tvoid SetHudPositionNormalized(float x, float y)\n\t{\n\t\tm_fHudPositionX = Math.Clamp(x, 0.0, 1.0);\n\t\tm_fHudPositionY = Math.Clamp(y, 0.0, 1.0);\n\t\tSaveHudSettings();\n\t\tif (m_bInitialized)\n\t\t\tRebuildHud();\n\t}\n'''
insert = needle + '''\n\t// Slider values are presented as 0..100%. Scale maps 0% -> 0.6x and\n\t// 100% -> 2.0x. Background opacity maps directly to 0..1 internally.\n\tfloat GetHudScalePercent()\n\t{\n\t\treturn Math.Clamp(((m_fHudScale - 0.6) / 1.4) * 100.0, 0.0, 100.0);\n\t}\n\n\tvoid SetHudScalePercent(float percent)\n\t{\n\t\tpercent = Math.Clamp(percent, 0.0, 100.0);\n\t\tm_fHudScale = 0.6 + (percent / 100.0) * 1.4;\n\t\tSaveHudSettings();\n\t\tif (m_bInitialized)\n\t\t\tRebuildHud();\n\t}\n\n\tfloat GetBackgroundOpacityPercent()\n\t{\n\t\treturn Math.Clamp(m_fBackgroundOpacity * 100.0, 0.0, 100.0);\n\t}\n\n\tvoid SetBackgroundOpacityPercent(float percent)\n\t{\n\t\tpercent = Math.Clamp(percent, 0.0, 100.0);\n\t\tm_fBackgroundOpacity = percent / 100.0;\n\t\tSaveHudSettings();\n\t\tif (m_bInitialized)\n\t\t\tRebuildHud();\n\t}\n'''
if needle not in src:
    raise SystemExit('HOTASDebugger.c SetHudPositionNormalized block not found')
src = src.replace(needle, insert, 1)
path.write_text(src, encoding='utf-8')

# --- HOTASSettingsTab.c ------------------------------------------------------
path = Path('Scripts/Game/HOTASDebugger/HOTASSettingsTab.c')
src = path.read_text(encoding='utf-8')

old = '''\tprotected SCR_SpinBoxComponent m_HotasConfig;\n\tprotected ref array<SCR_SpinBoxComponent> m_HudControls = {};\n'''
new = '''\tprotected SCR_SpinBoxComponent m_HotasConfig;\n\tprotected ref array<SCR_SpinBoxComponent> m_HudControls = {};\n\tprotected SCR_SliderComponent m_HudScaleSlider;\n\tprotected SCR_SliderComponent m_BackgroundOpacitySlider;\n'''
if old not in src:
    raise SystemExit('HOTASSettingsTab.c field block not found')
src = src.replace(old, new, 1)

src = src.replace('\t\tSetupHudControls();\n\t\tSetupHudPositionPreview();', '\t\tSetupHudControls();\n\t\tSetupHudSliders();\n\t\tSetupHudPositionPreview();', 1)
src = src.replace('\t\tSyncHudControls();\n\t\tSyncHudPositionPreviewFromController();', '\t\tSyncHudControls();\n\t\tSyncHudSliders();\n\t\tSyncHudPositionPreviewFromController();', 1)

needle = '''\tprotected SCR_SpinBoxComponent FindSpinBox(string widgetName)\n\t{\n\t\tWidget widget = m_wRoot.FindAnyWidget(widgetName);\n\t\tif (!widget)\n\t\t\treturn null;\n\n\t\treturn SCR_SpinBoxComponent.Cast(widget.FindHandler(SCR_SpinBoxComponent));\n\t}\n'''
insert = needle + '''\n\t//------------------------------------------------------------------------------------------------\n\tprotected SCR_SliderComponent FindSlider(string widgetName)\n\t{\n\t\tWidget widget = m_wRoot.FindAnyWidget(widgetName);\n\t\tif (!widget)\n\t\t\treturn null;\n\n\t\treturn SCR_SliderComponent.Cast(widget.FindHandler(SCR_SliderComponent));\n\t}\n'''
if needle not in src:
    raise SystemExit('HOTASSettingsTab.c FindSpinBox block not found')
src = src.replace(needle, insert, 1)

old = '\t\tfloat hudScale = 0.5 + controller.GetSettingOptionIndex(1) * 0.1;'
new = '\t\tfloat hudScale = 0.6 + (controller.GetHudScalePercent() / 100.0) * 1.4;'
if old not in src:
    raise SystemExit('HOTASSettingsTab.c preview scale line not found')
src = src.replace(old, new, 1)

needle = '''\tprotected void SyncHudControls()\n\t{\n'''
slider_block = '''\tprotected void SetupHudSliders()\n\t{\n\t\tHOTASDebugController controller = HOTASDebugController.GetInstance();\n\n\t\tm_HudScaleSlider = FindSlider("HUDScale");\n\t\tif (m_HudScaleSlider)\n\t\t{\n\t\t\tm_HudScaleSlider.SetSliderSettings(0.0, 100.0, 1.0, "%1%");\n\t\t\tm_HudScaleSlider.SetValue(controller.GetHudScalePercent());\n\t\t\tm_HudScaleSlider.GetOnChangedFinal().Insert(OnHudScaleChanged);\n\t\t}\n\n\t\tm_BackgroundOpacitySlider = FindSlider("BackgroundOpacity");\n\t\tif (m_BackgroundOpacitySlider)\n\t\t{\n\t\t\tm_BackgroundOpacitySlider.SetSliderSettings(0.0, 100.0, 1.0, "%1%");\n\t\t\tm_BackgroundOpacitySlider.SetValue(controller.GetBackgroundOpacityPercent());\n\t\t\tm_BackgroundOpacitySlider.GetOnChangedFinal().Insert(OnBackgroundOpacityChanged);\n\t\t}\n\t}\n\n\t//------------------------------------------------------------------------------------------------\n\tprotected void SyncHudSliders()\n\t{\n\t\tHOTASDebugController controller = HOTASDebugController.GetInstance();\n\t\tif (m_HudScaleSlider)\n\t\t\tm_HudScaleSlider.SetValue(controller.GetHudScalePercent());\n\t\tif (m_BackgroundOpacitySlider)\n\t\t\tm_BackgroundOpacitySlider.SetValue(controller.GetBackgroundOpacityPercent());\n\t}\n\n\t//------------------------------------------------------------------------------------------------\n'''
if needle not in src:
    raise SystemExit('HOTASSettingsTab.c SyncHudControls marker not found')
src = src.replace(needle, slider_block + needle, 1)

needle = '''\tprotected void OnHudSettingChanged(SCR_SpinBoxComponent component, int optionIndex)\n\t{\n'''
callbacks = '''\tprotected void OnHudScaleChanged(SCR_SliderComponent component, float value)\n\t{\n\t\tif (m_bLoading)\n\t\t\treturn;\n\n\t\tHOTASDebugController.GetInstance().SetHudScalePercent(value);\n\t\tUpdateHudPositionPreview();\n\t}\n\n\t//------------------------------------------------------------------------------------------------\n\tprotected void OnBackgroundOpacityChanged(SCR_SliderComponent component, float value)\n\t{\n\t\tif (m_bLoading)\n\t\t\treturn;\n\n\t\tHOTASDebugController.GetInstance().SetBackgroundOpacityPercent(value);\n\t}\n\n\t//------------------------------------------------------------------------------------------------\n'''
if needle not in src:
    raise SystemExit('HOTASSettingsTab.c OnHudSettingChanged marker not found')
src = src.replace(needle, callbacks + needle, 1)
path.write_text(src, encoding='utf-8')

# --- HOTASSettings.layout ----------------------------------------------------
path = Path('UI/layouts/Menus/SettingsSubMenus/HOTASSettings.layout')
src = path.read_text(encoding='utf-8')

old = '''      ButtonWidgetClass "{8C52D9F7A31B6424}" : "{C9DF0E6590F6C388}UI/layouts/WidgetLibrary/SpinBox/WLib_SpinBox.layout" {\n       Name "HUDScale"\n       Slot LayoutSlot "{8C52D9F7A31B6425}" { Padding 4 4 4 4 }\n       components { SCR_SpinBoxComponent "{5472C6CBC0640458}" { m_sLabel "Scale" m_sLabelLayout "{F003823FF141983C}UI/layouts/Menus/SettingsMenu/CustomWidgets/SettingsLabel.layout" m_bUseLightArrows 0 m_bCycleMode 1 m_bShowHints 0 } }\n      }\n'''
new = '''      ButtonWidgetClass "{8C52D9F7A31B6424}" : "{4A41296C0E9A889F}UI/layouts/WidgetLibrary/WLib_Slider.layout" {\n       Name "HUDScale"\n       Slot LayoutSlot "{8C52D9F7A31B6425}" { Padding 4 4 4 4 }\n       components {\n        SCR_SliderComponent "{548294960C7399D9}" {\n         m_sLabel "Scale"\n         m_sLabelLayout "{F003823FF141983C}UI/layouts/Menus/SettingsMenu/CustomWidgets/SettingsLabel.layout"\n         m_fMinValue 0\n         m_fMaxValue 100\n         m_fStep 1\n         m_sFormatText "%1%"\n         m_bRoundValue 1\n         m_iDecimalPrecision 0\n        }\n       }\n      }\n'''
if old not in src:
    raise SystemExit('HOTASSettings.layout HUDScale block not found')
src = src.replace(old, new, 1)

old = '''      ButtonWidgetClass "{8C52D9F7A31B6432}" : "{C9DF0E6590F6C388}UI/layouts/WidgetLibrary/SpinBox/WLib_SpinBox.layout" {\n       Name "BackgroundOpacity"\n       Slot LayoutSlot "{8C52D9F7A31B6433}" { Padding 4 4 4 4 }\n       components { SCR_SpinBoxComponent "{5472C6CBC0640458}" { m_sLabel "Background Opacity" m_sLabelLayout "{F003823FF141983C}UI/layouts/Menus/SettingsMenu/CustomWidgets/SettingsLabel.layout" m_bUseLightArrows 0 m_bCycleMode 1 m_bShowHints 0 } }\n      }\n'''
new = '''      ButtonWidgetClass "{8C52D9F7A31B6432}" : "{4A41296C0E9A889F}UI/layouts/WidgetLibrary/WLib_Slider.layout" {\n       Name "BackgroundOpacity"\n       Slot LayoutSlot "{8C52D9F7A31B6433}" { Padding 4 4 4 4 }\n       components {\n        SCR_SliderComponent "{548294960C7399D9}" {\n         m_sLabel "Background Opacity"\n         m_sLabelLayout "{F003823FF141983C}UI/layouts/Menus/SettingsMenu/CustomWidgets/SettingsLabel.layout"\n         m_fMinValue 0\n         m_fMaxValue 100\n         m_fStep 1\n         m_sFormatText "%1%"\n         m_bRoundValue 1\n         m_iDecimalPrecision 0\n        }\n       }\n      }\n'''
if old not in src:
    raise SystemExit('HOTASSettings.layout BackgroundOpacity block not found')
src = src.replace(old, new, 1)
path.write_text(src, encoding='utf-8')
