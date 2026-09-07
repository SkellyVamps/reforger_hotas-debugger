from pathlib import Path

controller_path = Path('Scripts/Game/HOTASDebugger/HOTASDebugger.c')
settings_path = Path('Scripts/Game/HOTASDebugger/HOTASSettingsTab.c')
layout_path = Path('UI/layouts/Menus/SettingsSubMenus/HOTASSettings.layout')

controller = controller_path.read_text(encoding='utf-8')
settings = settings_path.read_text(encoding='utf-8')
layout = layout_path.read_text(encoding='utf-8')


def replace_once(src: str, old: str, new: str, label: str) -> str:
    if old not in src:
        raise SystemExit(f'Missing expected block: {label}')
    return src.replace(old, new, 1)

# -----------------------------------------------------------------------------
# Controller: assignments come from the active config/runtime binding, while the
# profile settings only store the player-facing custom text labels.
controller = replace_once(
    controller,
    '''\t// Raw joystick axis numbers used by this HOTAS. Users can remap these in HOTASHudSettings.txt.\n\tprotected int m_iRollAxis = 0;\n\tprotected int m_iPitchAxis = 1;\n\tprotected int m_iThrottleAxis = 2;\n\tprotected int m_iYawAxis = 5;\n''',
    '''\t// Axis assignments are discovered from the currently active HOTAS config.\n\t// Keep the complete joystickX:axisY key internally so two devices that both\n\t// expose axis0 do not get mistaken for one another.\n\tprotected string m_sRollAxisBinding;\n\tprotected string m_sPitchAxisBinding;\n\tprotected string m_sThrottleAxisBinding;\n\tprotected string m_sYawAxisBinding;\n\tprotected int m_iRollAxis = -1;\n\tprotected int m_iPitchAxis = -1;\n\tprotected int m_iThrottleAxis = -1;\n\tprotected int m_iYawAxis = -1;\n\n\t// Player-editable HUD text for each discovered axis.\n\tprotected string m_sRollAxisLabel = "Roll";\n\tprotected string m_sPitchAxisLabel = "Pitch";\n\tprotected string m_sThrottleAxisLabel = "Throttle";\n\tprotected string m_sYawAxisLabel = "Yaw";\n''',
    'axis fields'
)

controller = replace_once(
    controller,
    '''\t\tBuildActionList();\n\t\tLoadHudSettings();\n\t\tCreateHud();\n''',
    '''\t\tBuildActionList();\n\t\tLoadHudSettings();\n\t\tRefreshAssignedAxesFromBindings();\n\t\tCreateHud();\n''',
    'initial assignment refresh'
)

controller = replace_once(
    controller,
    '''\t\t\t\tdefaults.WriteLine("# Raw joystick axis mapping. Set an unused control to -1.");\n\t\t\t\tdefaults.WriteLine("roll_axis=0");\n\t\t\t\tdefaults.WriteLine("pitch_axis=1");\n\t\t\t\tdefaults.WriteLine("throttle_axis=2");\n\t\t\t\tdefaults.WriteLine("yaw_axis=5");\n''',
    '''\t\t\t\tdefaults.WriteLine("# Axis numbers are detected from the active HOTAS config. These values only rename the HUD display.");\n\t\t\t\tdefaults.WriteLine("roll_label=Roll");\n\t\t\t\tdefaults.WriteLine("pitch_label=Pitch");\n\t\t\t\tdefaults.WriteLine("throttle_label=Throttle");\n\t\t\t\tdefaults.WriteLine("yaw_label=Yaw");\n''',
    'default label settings'
)

controller = replace_once(
    controller,
    '''\t\t\telse if (key == "roll_axis")\n\t\t\t\tm_iRollAxis = Math.ClampInt(value.ToInt(0), -1, 63);\n\t\t\telse if (key == "pitch_axis")\n\t\t\t\tm_iPitchAxis = Math.ClampInt(value.ToInt(1), -1, 63);\n\t\t\telse if (key == "throttle_axis")\n\t\t\t\tm_iThrottleAxis = Math.ClampInt(value.ToInt(2), -1, 63);\n\t\t\telse if (key == "yaw_axis")\n\t\t\t\tm_iYawAxis = Math.ClampInt(value.ToInt(5), -1, 63);\n''',
    '''\t\t\telse if (key == "roll_label")\n\t\t\t\tm_sRollAxisLabel = value;\n\t\t\telse if (key == "pitch_label")\n\t\t\t\tm_sPitchAxisLabel = value;\n\t\t\telse if (key == "throttle_label")\n\t\t\t\tm_sThrottleAxisLabel = value;\n\t\t\telse if (key == "yaw_label")\n\t\t\t\tm_sYawAxisLabel = value;\n''',
    'load custom labels'
)

controller = replace_once(
    controller,
    '''\t\tPrint(string.Format("[HOTAS Debugger] Axis mapping: roll=%1 pitch=%2 throttle=%3 yaw=%4", m_iRollAxis, m_iPitchAxis, m_iThrottleAxis, m_iYawAxis), LogLevel.NORMAL);\n''',
    '''\t\tPrint(string.Format("[HOTAS Debugger] Axis HUD labels: roll=%1 pitch=%2 throttle=%3 yaw=%4", m_sRollAxisLabel, m_sPitchAxisLabel, m_sThrottleAxisLabel, m_sYawAxisLabel), LogLevel.NORMAL);\n''',
    'settings log'
)

controller = replace_once(
    controller,
    '''\t\tfile.WriteLine("# Raw joystick axis mapping. Human-facing menu labels are one-based; -1 disables a semantic label.");\n\t\tfile.WriteLine(string.Format("roll_axis=%1", m_iRollAxis));\n\t\tfile.WriteLine(string.Format("pitch_axis=%1", m_iPitchAxis));\n\t\tfile.WriteLine(string.Format("throttle_axis=%1", m_iThrottleAxis));\n\t\tfile.WriteLine(string.Format("yaw_axis=%1", m_iYawAxis));\n''',
    '''\t\tfile.WriteLine("# Axis numbers are detected from the active HOTAS config. These values only rename the HUD display.");\n\t\tfile.WriteLine(string.Format("roll_label=%1", m_sRollAxisLabel));\n\t\tfile.WriteLine(string.Format("pitch_label=%1", m_sPitchAxisLabel));\n\t\tfile.WriteLine(string.Format("throttle_label=%1", m_sThrottleAxisLabel));\n\t\tfile.WriteLine(string.Format("yaw_label=%1", m_sYawAxisLabel));\n''',
    'save custom labels'
)

controller = replace_once(
    controller,
    '''\tvoid ReloadHudSettings()\n\t{\n\t\tLoadHudSettings();\n\t}\n''',
    '''\tvoid ReloadHudSettings()\n\t{\n\t\tLoadHudSettings();\n\t\tRefreshAssignedAxesFromBindings();\n\t}\n\n\tvoid RefreshAssignedAxes()\n\t{\n\t\tRefreshAssignedAxesFromBindings();\n\t}\n\n\tint GetAxisAssignmentRaw(int axisIndex)\n\t{\n\t\tswitch (axisIndex)\n\t\t{\n\t\t\tcase 0: return m_iRollAxis;\n\t\t\tcase 1: return m_iPitchAxis;\n\t\t\tcase 2: return m_iThrottleAxis;\n\t\t\tcase 3: return m_iYawAxis;\n\t\t}\n\t\treturn -1;\n\t}\n\n\tstring GetAxisAssignmentDisplayName(int axisIndex)\n\t{\n\t\tint rawAxis = GetAxisAssignmentRaw(axisIndex);\n\t\tif (rawAxis < 0)\n\t\t\treturn "Unassigned";\n\t\treturn string.Format("Axis %1", rawAxis + 1);\n\t}\n\n\tstring GetAxisSettingRowLabel(int axisIndex)\n\t{\n\t\tstring logicalName;\n\t\tswitch (axisIndex)\n\t\t{\n\t\t\tcase 0: logicalName = "Roll Axis"; break;\n\t\t\tcase 1: logicalName = "Pitch Axis"; break;\n\t\t\tcase 2: logicalName = "Throttle Axis"; break;\n\t\t\tcase 3: logicalName = "Yaw Axis"; break;\n\t\t\tdefault: logicalName = "Axis"; break;\n\t\t}\n\n\t\treturn string.Format("%1 - %2", logicalName, GetAxisAssignmentDisplayName(axisIndex));\n\t}\n\n\tstring GetAxisCustomLabel(int axisIndex)\n\t{\n\t\tswitch (axisIndex)\n\t\t{\n\t\t\tcase 0: return m_sRollAxisLabel;\n\t\t\tcase 1: return m_sPitchAxisLabel;\n\t\t\tcase 2: return m_sThrottleAxisLabel;\n\t\t\tcase 3: return m_sYawAxisLabel;\n\t\t}\n\t\treturn string.Empty;\n\t}\n\n\tvoid SetAxisCustomLabel(int axisIndex, string value)\n\t{\n\t\tvalue = value.Trim();\n\t\tswitch (axisIndex)\n\t\t{\n\t\t\tcase 0: m_sRollAxisLabel = value; break;\n\t\t\tcase 1: m_sPitchAxisLabel = value; break;\n\t\t\tcase 2: m_sThrottleAxisLabel = value; break;\n\t\t\tcase 3: m_sYawAxisLabel = value; break;\n\t\t\tdefault: return;\n\t\t}\n\n\t\tSaveHudSettings();\n\t}\n\n\tprotected string NormalizeAxisBinding(string binding)\n\t{\n\t\tint axisPos = binding.IndexOf(":axis");\n\t\tif (axisPos < 0)\n\t\t\treturn string.Empty;\n\n\t\tstring normalized = binding;\n\t\tif (normalized.EndsWith("+") || normalized.EndsWith("-"))\n\t\t\tnormalized = normalized.Substring(0, normalized.Length() - 1);\n\t\treturn normalized;\n\t}\n\n\tprotected int GetRawAxisFromBinding(string binding)\n\t{\n\t\tint axisPos = binding.IndexOf(":axis");\n\t\tif (axisPos < 0)\n\t\t\treturn -1;\n\n\t\tstring axisText = binding.Substring(axisPos + 5, binding.Length() - axisPos - 5);\n\t\tif (axisText.EndsWith("+") || axisText.EndsWith("-"))\n\t\t\taxisText = axisText.Substring(0, axisText.Length() - 1);\n\t\treturn axisText.ToInt(-1);\n\t}\n\n\tprotected string GetAxisBindingFromAction(string actionName)\n\t{\n\t\tstring bindingsText = GetJoystickBindings(actionName);\n\t\tref array<string> bindings = {};\n\t\tbindingsText.Split(" / ", bindings, true);\n\t\tforeach (string binding : bindings)\n\t\t{\n\t\t\tstring normalized = NormalizeAxisBinding(binding);\n\t\t\tif (!normalized.IsEmpty())\n\t\t\t\treturn normalized;\n\t\t}\n\t\treturn string.Empty;\n\t}\n\n\tprotected string ResolveAxisBindingFromActions(string primaryAction, string negativeAction, string positiveAction)\n\t{\n\t\tstring binding = GetAxisBindingFromAction(primaryAction);\n\t\tif (!binding.IsEmpty())\n\t\t\treturn binding;\n\n\t\tbinding = GetAxisBindingFromAction(negativeAction);\n\t\tif (!binding.IsEmpty())\n\t\t\treturn binding;\n\n\t\treturn GetAxisBindingFromAction(positiveAction);\n\t}\n\n\tprotected void RefreshAssignedAxesFromBindings()\n\t{\n\t\tm_sRollAxisBinding = ResolveAxisBindingFromActions("PFC_Roll", "HelicopterCyclicLeft", "HelicopterCyclicRight");\n\t\tm_sPitchAxisBinding = ResolveAxisBindingFromActions("PFC_Pitch", "HelicopterCyclicForward", "HelicopterCyclicBack");\n\t\tm_sThrottleAxisBinding = ResolveAxisBindingFromActions("PFC_ThrottleAxis", "HelicopterCollectiveDecrease", "HelicopterCollectiveIncrease");\n\t\tm_sYawAxisBinding = ResolveAxisBindingFromActions("PFC_Yaw", "HelicopterAntiTorqueLeft", "HelicopterAntiTorqueRight");\n\n\t\tm_iRollAxis = GetRawAxisFromBinding(m_sRollAxisBinding);\n\t\tm_iPitchAxis = GetRawAxisFromBinding(m_sPitchAxisBinding);\n\t\tm_iThrottleAxis = GetRawAxisFromBinding(m_sThrottleAxisBinding);\n\t\tm_iYawAxis = GetRawAxisFromBinding(m_sYawAxisBinding);\n\n\t\tPrint(string.Format("[HOTAS Debugger] Config axis assignments: roll=%1 pitch=%2 throttle=%3 yaw=%4", GetAxisAssignmentDisplayName(0), GetAxisAssignmentDisplayName(1), GetAxisAssignmentDisplayName(2), GetAxisAssignmentDisplayName(3)), LogLevel.NORMAL);\n\t}\n''',
    'axis assignment helper methods'
)

# Axis rows are no longer selectable settings; their edit boxes are handled by HOTASSettingsSubMenu.
controller = replace_once(
    controller,
    '''\t\t\tcase 6:\n\t\t\tcase 7:\n\t\t\tcase 8:\n\t\t\tcase 9: return 65;\n''',
    '''\t\t\tcase 6:\n\t\t\tcase 7:\n\t\t\tcase 8:\n\t\t\tcase 9: return 0;\n''',
    'axis option counts'
)

controller = replace_once(
    controller,
    '''\t\t\tcase 6: return Math.ClampInt(m_iRollAxis + 1, 0, 64);\n\t\t\tcase 7: return Math.ClampInt(m_iPitchAxis + 1, 0, 64);\n\t\t\tcase 8: return Math.ClampInt(m_iThrottleAxis + 1, 0, 64);\n\t\t\tcase 9: return Math.ClampInt(m_iYawAxis + 1, 0, 64);\n''',
    '''\t\t\tcase 6:\n\t\t\tcase 7:\n\t\t\tcase 8:\n\t\t\tcase 9: return 0;\n''',
    'axis option indices'
)

controller = replace_once(
    controller,
    '''\t\t\tcase 6:\n\t\t\tcase 7:\n\t\t\tcase 8:\n\t\t\tcase 9:\n\t\t\t\tif (optionIndex == 0) return "Disabled";\n\t\t\t\treturn string.Format("Axis %1", optionIndex);\n''',
    '''\t\t\tcase 6:\n\t\t\tcase 7:\n\t\t\tcase 8:\n\t\t\tcase 9:\n\t\t\t\treturn GetAxisAssignmentDisplayName(index - 6);\n''',
    'axis option labels'
)

controller = replace_once(
    controller,
    '''\t\t\tcase 6: m_iRollAxis = optionIndex - 1; break;\n\t\t\tcase 7: m_iPitchAxis = optionIndex - 1; break;\n\t\t\tcase 8: m_iThrottleAxis = optionIndex - 1; break;\n\t\t\tcase 9: m_iYawAxis = optionIndex - 1; break;\n''',
    '''\t\t\tcase 6:\n\t\t\tcase 7:\n\t\t\tcase 8:\n\t\t\tcase 9: return;\n''',
    'remove manual axis assignment'
)

controller = replace_once(
    controller,
    '''\t\t\t\tint rawAxis = axisText.ToInt();\n\t\t\t\tstring axisName;\n\t\t\t\tif (rawAxis == m_iRollAxis && m_iRollAxis >= 0)\n\t\t\t\t\taxisName = "ROLL";\n\t\t\t\telse if (rawAxis == m_iPitchAxis && m_iPitchAxis >= 0)\n\t\t\t\t\taxisName = "PITCH";\n\t\t\t\telse if (rawAxis == m_iThrottleAxis && m_iThrottleAxis >= 0)\n\t\t\t\t\taxisName = "THROTTLE";\n\t\t\t\telse if (rawAxis == m_iYawAxis && m_iYawAxis >= 0)\n\t\t\t\t\taxisName = "YAW";\n''',
    '''\t\t\t\tint rawAxis = axisText.ToInt();\n\t\t\t\tstring normalizedBinding = NormalizeAxisBinding(binding);\n\t\t\t\tstring axisName;\n\t\t\t\tif (!m_sRollAxisBinding.IsEmpty() && normalizedBinding == m_sRollAxisBinding)\n\t\t\t\t\taxisName = m_sRollAxisLabel;\n\t\t\t\telse if (!m_sPitchAxisBinding.IsEmpty() && normalizedBinding == m_sPitchAxisBinding)\n\t\t\t\t\taxisName = m_sPitchAxisLabel;\n\t\t\t\telse if (!m_sThrottleAxisBinding.IsEmpty() && normalizedBinding == m_sThrottleAxisBinding)\n\t\t\t\t\taxisName = m_sThrottleAxisLabel;\n\t\t\t\telse if (!m_sYawAxisBinding.IsEmpty() && normalizedBinding == m_sYawAxisBinding)\n\t\t\t\t\taxisName = m_sYawAxisLabel;\n''',
    'HUD custom axis labels'
)

# -----------------------------------------------------------------------------
# Settings tab: axis rows become edit boxes. The left label shows semantic +
# detected config axis, and the text box is only the player's HUD label.
settings = replace_once(
    settings,
    '''\tprotected SCR_SliderComponent m_BackgroundOpacitySlider;\n\tprotected ref array<string> m_UserConfigs = {};\n''',
    '''\tprotected SCR_SliderComponent m_BackgroundOpacitySlider;\n\tprotected ref array<SCR_EditBoxComponent> m_AxisLabelEditors = {};\n\tprotected ref array<string> m_UserConfigs = {};\n''',
    'axis editor fields'
)

settings = replace_once(
    settings,
    '''\t\tSetupHotasConfigSelector();\n\t\tSetupHudControls();\n\t\tSetupHudSliders();\n\t\tSetupHudPositionPreview();\n''',
    '''\t\tSetupHotasConfigSelector();\n\t\tSetupHudControls();\n\t\tSetupHudSliders();\n\t\tSetupAxisLabelEditors();\n\t\tSetupHudPositionPreview();\n''',
    'setup axis editors'
)

settings = replace_once(
    settings,
    '''\t\tSyncHotasConfigSelector();\n\t\tSyncHudControls();\n\t\tSyncHudSliders();\n\t\tSyncHudPositionPreviewFromController();\n''',
    '''\t\tSyncHotasConfigSelector();\n\t\tSyncHudControls();\n\t\tSyncHudSliders();\n\t\tSyncAxisLabelEditors();\n\t\tSyncHudPositionPreviewFromController();\n''',
    'sync axis editors'
)

settings = replace_once(
    settings,
    '''\tprotected SCR_SliderComponent FindSlider(string widgetName)\n\t{\n\t\tWidget widget = m_wRoot.FindAnyWidget(widgetName);\n\t\tif (!widget)\n\t\t\treturn null;\n\n\t\treturn SCR_SliderComponent.Cast(widget.FindHandler(SCR_SliderComponent));\n\t}\n''',
    '''\tprotected SCR_SliderComponent FindSlider(string widgetName)\n\t{\n\t\tWidget widget = m_wRoot.FindAnyWidget(widgetName);\n\t\tif (!widget)\n\t\t\treturn null;\n\n\t\treturn SCR_SliderComponent.Cast(widget.FindHandler(SCR_SliderComponent));\n\t}\n\n\t//------------------------------------------------------------------------------------------------\n\tprotected SCR_EditBoxComponent FindEditBox(string widgetName)\n\t{\n\t\tWidget widget = m_wRoot.FindAnyWidget(widgetName);\n\t\tif (!widget)\n\t\t\treturn null;\n\n\t\treturn SCR_EditBoxComponent.Cast(widget.FindHandler(SCR_EditBoxComponent));\n\t}\n''',
    'find edit box helper'
)

settings = replace_once(
    settings,
    '''\t\t\tPrint("[HOTAS Debugger] Custom HOTAS input config cleared from Settings", LogLevel.NORMAL);\n\t\t\treturn;\n''',
    '''\t\t\tPrint("[HOTAS Debugger] Custom HOTAS input config cleared from Settings", LogLevel.NORMAL);\n\t\t\tScheduleAxisAssignmentRefresh();\n\t\t\treturn;\n''',
    'refresh after config clear'
)

settings = replace_once(
    settings,
    '''\t\tPrint(string.Format("[HOTAS Debugger] HOTAS input config selected from Settings: %1", selectedConfig), LogLevel.NORMAL);\n\t}\n\n\n\t//------------------------------------------------------------------------------------------------\n\tprotected void SetupHudPositionPreview()\n''',
    '''\t\tPrint(string.Format("[HOTAS Debugger] HOTAS input config selected from Settings: %1", selectedConfig), LogLevel.NORMAL);\n\t\tScheduleAxisAssignmentRefresh();\n\t}\n\n\t//------------------------------------------------------------------------------------------------\n\tprotected void ScheduleAxisAssignmentRefresh()\n\t{\n\t\tGetGame().GetCallqueue().CallLater(RefreshAxisAssignments, 50, false);\n\t}\n\n\t//------------------------------------------------------------------------------------------------\n\tprotected void RefreshAxisAssignments()\n\t{\n\t\tHOTASDebugController.GetInstance().RefreshAssignedAxes();\n\t\tm_bLoading = true;\n\t\tSyncAxisLabelEditors();\n\t\tm_bLoading = false;\n\t}\n\n\n\t//------------------------------------------------------------------------------------------------\n\tprotected void SetupHudPositionPreview()\n''',
    'config refresh helpers'
)

settings = replace_once(
    settings,
    '''\tprotected void SyncHudSliders()\n\t{\n\t\tHOTASDebugController controller = HOTASDebugController.GetInstance();\n\t\tif (m_HudScaleSlider)\n\t\t\tm_HudScaleSlider.SetValue(controller.GetHudScalePercent());\n\t\tif (m_BackgroundOpacitySlider)\n\t\t\tm_BackgroundOpacitySlider.SetValue(controller.GetBackgroundOpacityPercent());\n\t}\n\n\t//------------------------------------------------------------------------------------------------\n\tprotected void SyncHudControls()\n''',
    '''\tprotected void SyncHudSliders()\n\t{\n\t\tHOTASDebugController controller = HOTASDebugController.GetInstance();\n\t\tif (m_HudScaleSlider)\n\t\t\tm_HudScaleSlider.SetValue(controller.GetHudScalePercent());\n\t\tif (m_BackgroundOpacitySlider)\n\t\t\tm_BackgroundOpacitySlider.SetValue(controller.GetBackgroundOpacityPercent());\n\t}\n\n\t//------------------------------------------------------------------------------------------------\n\tprotected void SetupAxisLabelEditors()\n\t{\n\t\tm_AxisLabelEditors.Clear();\n\t\tarray<string> widgetNames = { "RollAxis", "PitchAxis", "ThrottleAxis", "YawAxis" };\n\t\tHOTASDebugController controller = HOTASDebugController.GetInstance();\n\t\tcontroller.RefreshAssignedAxes();\n\n\t\tfor (int i = 0; i < widgetNames.Count(); i++)\n\t\t{\n\t\t\tSCR_EditBoxComponent editor = FindEditBox(widgetNames[i]);\n\t\t\tm_AxisLabelEditors.Insert(editor);\n\t\t\tif (!editor)\n\t\t\t\tcontinue;\n\n\t\t\teditor.SetLabel(controller.GetAxisSettingRowLabel(i));\n\t\t\teditor.SetValue(controller.GetAxisCustomLabel(i));\n\t\t\teditor.SetPlaceholderText("Custom HUD label");\n\t\t\teditor.m_OnConfirm.Insert(OnAxisLabelConfirmed);\n\t\t}\n\t}\n\n\t//------------------------------------------------------------------------------------------------\n\tprotected void SyncAxisLabelEditors()\n\t{\n\t\tHOTASDebugController controller = HOTASDebugController.GetInstance();\n\t\tfor (int i = 0; i < m_AxisLabelEditors.Count(); i++)\n\t\t{\n\t\t\tSCR_EditBoxComponent editor = m_AxisLabelEditors[i];\n\t\t\tif (!editor)\n\t\t\t\tcontinue;\n\n\t\t\teditor.SetLabel(controller.GetAxisSettingRowLabel(i));\n\t\t\teditor.SetValue(controller.GetAxisCustomLabel(i));\n\t\t}\n\t}\n\n\t//------------------------------------------------------------------------------------------------\n\tprotected void OnAxisLabelConfirmed(SCR_EditBoxComponent component, string value)\n\t{\n\t\tif (m_bLoading)\n\t\t\treturn;\n\n\t\tfor (int i = 0; i < m_AxisLabelEditors.Count(); i++)\n\t\t{\n\t\t\tif (m_AxisLabelEditors[i] != component)\n\t\t\t\tcontinue;\n\n\t\t\tHOTASDebugController.GetInstance().SetAxisCustomLabel(i, value);\n\t\t\treturn;\n\t\t}\n\t}\n\n\t//------------------------------------------------------------------------------------------------\n\tprotected void SyncHudControls()\n''',
    'axis editor methods'
)

# -----------------------------------------------------------------------------
# Layout: replace only the four axis spin boxes with vanilla edit boxes.
axis_rows = {
    'Roll': ('8C52D9F7A31B6436', '8C52D9F7A31B6437', 'RollAxis'),
    'Pitch': ('8C52D9F7A31B6438', '8C52D9F7A31B6439', 'PitchAxis'),
    'Throttle': ('8C52D9F7A31B6440', '8C52D9F7A31B6441', 'ThrottleAxis'),
    'Yaw': ('8C52D9F7A31B6442', '8C52D9F7A31B6443', 'YawAxis'),
}
for logical, (widget_guid, slot_guid, widget_name) in axis_rows.items():
    old = f'''      ButtonWidgetClass "{{{widget_guid}}}" : "{{C9DF0E6590F6C388}}UI/layouts/WidgetLibrary/SpinBox/WLib_SpinBox.layout" {{\n       Name "{widget_name}"\n       Slot LayoutSlot "{{{slot_guid}}}" {{ Padding 4 4 4 4 }}\n       components {{ SCR_SpinBoxComponent "{{5472C6CBC0640458}}" {{ m_sLabel "{logical} Axis" m_sLabelLayout "{{F003823FF141983C}}UI/layouts/Menus/SettingsMenu/CustomWidgets/SettingsLabel.layout" m_bUseLightArrows 0 m_bCycleMode 1 m_bShowHints 0 }} }}\n      }}\n'''
    new = f'''      ButtonWidgetClass "{{{widget_guid}}}" : "{{0022F0B45ADBC5AC}}UI/layouts/WidgetLibrary/EditBox/WLib_EditBox.layout" {{\n       Name "{widget_name}"\n       Slot LayoutSlot "{{{slot_guid}}}" {{ Padding 4 4 4 4 }}\n       components {{\n        SCR_EditBoxComponent "{{547290FFBD5B33E9}}" {{\n         m_sLabel "{logical} Axis - Unassigned"\n         m_sLabelLayout "{{F003823FF141983C}}UI/layouts/Menus/SettingsMenu/CustomWidgets/SettingsLabel.layout"\n         m_bShowWriteIcon 0\n        }}\n       }}\n      }}\n'''
    layout = replace_once(layout, old, new, f'{logical} axis row')

controller_path.write_text(controller, encoding='utf-8')
settings_path.write_text(settings, encoding='utf-8')
layout_path.write_text(layout, encoding='utf-8')
