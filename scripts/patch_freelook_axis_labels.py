from pathlib import Path


def replace_once(text: str, old: str, new: str, label: str) -> str:
    count = text.count(old)
    if count != 1:
        raise RuntimeError(f"{label}: expected exactly one match, found {count}")
    return text.replace(old, new, 1)


# -----------------------------------------------------------------------------
# HOTASDebugger.c
# -----------------------------------------------------------------------------
path = Path("Scripts/Game/HOTASDebugger/HOTASDebugger.c")
src = path.read_text(encoding="utf-8")

src = replace_once(
    src,
    '''\tprotected string m_sYawAxisBinding;\n\tprotected int m_iRollAxis = -1;''',
    '''\tprotected string m_sYawAxisBinding;\n\n\t// Direction-sensitive Free Look bindings. Unlike flight-axis bindings, keep the +/-\n\t// direction because each thumbstick direction can have its own player-facing label.\n\tprotected string m_sFreelookUpAxisBinding;\n\tprotected string m_sFreelookDownAxisBinding;\n\tprotected string m_sFreelookRightAxisBinding;\n\tprotected string m_sFreelookLeftAxisBinding;\n\n\tprotected int m_iRollAxis = -1;''',
    "add Free Look binding fields",
)

src = replace_once(
    src,
    '''\tprotected string m_sYawAxisLabel = "Yaw";\n\n\tstatic HOTASDebugController GetInstance()''',
    '''\tprotected string m_sYawAxisLabel = "Yaw";\n\n\t// Optional direction-specific Free Look labels. Empty means the HUD falls back to\n\t// the normal raw AXIS N+/- display. These are deliberately user text rather than\n\t// hardware assumptions so a hat, ministick, thumbstick, etc. can be named correctly.\n\tprotected string m_sFreelookUpLabel;\n\tprotected string m_sFreelookDownLabel;\n\tprotected string m_sFreelookRightLabel;\n\tprotected string m_sFreelookLeftLabel;\n\n\tstatic HOTASDebugController GetInstance()''',
    "add Free Look label fields",
)

src = replace_once(
    src,
    '''\t\t\t\tdefaults.WriteLine("yaw_label=Yaw");\n\t\t\t\tdefaults.Close();''',
    '''\t\t\t\tdefaults.WriteLine("yaw_label=Yaw");\n\t\t\t\tdefaults.WriteLine("# Optional direction-specific Free Look labels. Leave blank to show the raw AXIS N+/- binding.");\n\t\t\t\tdefaults.WriteLine("freelook_up_label=");\n\t\t\t\tdefaults.WriteLine("freelook_down_label=");\n\t\t\t\tdefaults.WriteLine("freelook_right_label=");\n\t\t\t\tdefaults.WriteLine("freelook_left_label=");\n\t\t\t\tdefaults.Close();''',
    "write Free Look defaults",
)

src = replace_once(
    src,
    '''\t\t\telse if (key == "yaw_label")\n\t\t\t\tm_sYawAxisLabel = value;\n\t\t}\n\t\tfile.Close();''',
    '''\t\t\telse if (key == "yaw_label")\n\t\t\t\tm_sYawAxisLabel = value;\n\t\t\telse if (key == "freelook_up_label")\n\t\t\t\tm_sFreelookUpLabel = value;\n\t\t\telse if (key == "freelook_down_label")\n\t\t\t\tm_sFreelookDownLabel = value;\n\t\t\telse if (key == "freelook_right_label")\n\t\t\t\tm_sFreelookRightLabel = value;\n\t\t\telse if (key == "freelook_left_label")\n\t\t\t\tm_sFreelookLeftLabel = value;\n\t\t}\n\t\tfile.Close();''',
    "load Free Look labels",
)

src = replace_once(
    src,
    '''\t\tfile.WriteLine(string.Format("yaw_label=%1", m_sYawAxisLabel));\n\t\tfile.Close();''',
    '''\t\tfile.WriteLine(string.Format("yaw_label=%1", m_sYawAxisLabel));\n\t\tfile.WriteLine("# Optional direction-specific Free Look labels. Leave blank to show the raw AXIS N+/- binding.");\n\t\tfile.WriteLine(string.Format("freelook_up_label=%1", m_sFreelookUpLabel));\n\t\tfile.WriteLine(string.Format("freelook_down_label=%1", m_sFreelookDownLabel));\n\t\tfile.WriteLine(string.Format("freelook_right_label=%1", m_sFreelookRightLabel));\n\t\tfile.WriteLine(string.Format("freelook_left_label=%1", m_sFreelookLeftLabel));\n\t\tfile.Close();''',
    "save Free Look labels",
)

anchor = '''\tvoid SetAxisCustomLabel(int axisIndex, string value)\n\t{\n\t\tvalue = value.Trim();\n\t\tswitch (axisIndex)\n\t\t{\n\t\t\tcase 0: m_sRollAxisLabel = value; break;\n\t\t\tcase 1: m_sPitchAxisLabel = value; break;\n\t\t\tcase 2: m_sThrottleAxisLabel = value; break;\n\t\t\tcase 3: m_sYawAxisLabel = value; break;\n\t\t\tdefault: return;\n\t\t}\n\t\tSaveHudSettings();\n\t}\n\n'''
insert = anchor + '''\tstring GetFreelookAssignmentDisplayName(int directionIndex)\n\t{\n\t\tstring binding;\n\t\tswitch (directionIndex)\n\t\t{\n\t\t\tcase 0: binding = m_sFreelookUpAxisBinding; break;\n\t\t\tcase 1: binding = m_sFreelookDownAxisBinding; break;\n\t\t\tcase 2: binding = m_sFreelookRightAxisBinding; break;\n\t\t\tcase 3: binding = m_sFreelookLeftAxisBinding; break;\n\t\t}\n\n\t\tint axisPos = binding.IndexOf(":axis");\n\t\tif (axisPos < 0)\n\t\t\treturn "Unassigned";\n\n\t\tstring axisText = binding.Substring(axisPos + 5, binding.Length() - axisPos - 5);\n\t\tstring direction;\n\t\tif (axisText.EndsWith("+"))\n\t\t{\n\t\t\tdirection = "+";\n\t\t\taxisText = axisText.Substring(0, axisText.Length() - 1);\n\t\t}\n\t\telse if (axisText.EndsWith("-"))\n\t\t{\n\t\t\tdirection = "-";\n\t\t\taxisText = axisText.Substring(0, axisText.Length() - 1);\n\t\t}\n\n\t\tint rawAxis = axisText.ToInt(-1);\n\t\tif (rawAxis < 0)\n\t\t\treturn "Unassigned";\n\n\t\treturn string.Format("AXIS %1%2", rawAxis + 1, direction);\n\t}\n\n\tstring GetFreelookSettingRowLabel(int directionIndex)\n\t{\n\t\tstring logicalName;\n\t\tswitch (directionIndex)\n\t\t{\n\t\t\tcase 0: logicalName = "Free Look Up"; break;\n\t\t\tcase 1: logicalName = "Free Look Down"; break;\n\t\t\tcase 2: logicalName = "Free Look Right"; break;\n\t\t\tcase 3: logicalName = "Free Look Left"; break;\n\t\t\tdefault: logicalName = "Free Look"; break;\n\t\t}\n\n\t\treturn string.Format("%1 - %2", logicalName, GetFreelookAssignmentDisplayName(directionIndex));\n\t}\n\n\tstring GetFreelookCustomLabel(int directionIndex)\n\t{\n\t\tswitch (directionIndex)\n\t\t{\n\t\t\tcase 0: return m_sFreelookUpLabel;\n\t\t\tcase 1: return m_sFreelookDownLabel;\n\t\t\tcase 2: return m_sFreelookRightLabel;\n\t\t\tcase 3: return m_sFreelookLeftLabel;\n\t\t}\n\t\treturn string.Empty;\n\t}\n\n\tvoid SetFreelookCustomLabel(int directionIndex, string value)\n\t{\n\t\tvalue = value.Trim();\n\t\tswitch (directionIndex)\n\t\t{\n\t\t\tcase 0: m_sFreelookUpLabel = value; break;\n\t\t\tcase 1: m_sFreelookDownLabel = value; break;\n\t\t\tcase 2: m_sFreelookRightLabel = value; break;\n\t\t\tcase 3: m_sFreelookLeftLabel = value; break;\n\t\t\tdefault: return;\n\t\t}\n\t\tSaveHudSettings();\n\t}\n\n'''
src = replace_once(src, anchor, insert, "add Free Look settings accessors")

src = replace_once(
    src,
    '''\tprotected string GetAxisBindingFromAction(string actionName)\n\t{\n\t\tstring bindingsText = GetJoystickBindings(actionName);\n\t\tref array<string> bindings = {};\n\t\tbindingsText.Split(" / ", bindings, true);\n\t\tforeach (string binding : bindings)\n\t\t{\n\t\t\tstring normalized = NormalizeAxisBinding(binding);\n\t\t\tif (!normalized.IsEmpty())\n\t\t\t\treturn normalized;\n\t\t}\n\t\treturn string.Empty;\n\t}\n\n''',
    '''\tprotected string GetAxisBindingFromAction(string actionName)\n\t{\n\t\tstring bindingsText = GetJoystickBindings(actionName);\n\t\tref array<string> bindings = {};\n\t\tbindingsText.Split(" / ", bindings, true);\n\t\tforeach (string binding : bindings)\n\t\t{\n\t\t\tstring normalized = NormalizeAxisBinding(binding);\n\t\t\tif (!normalized.IsEmpty())\n\t\t\t\treturn normalized;\n\t\t}\n\t\treturn string.Empty;\n\t}\n\n\tprotected string GetDirectionalAxisBindingFromAction(string actionName)\n\t{\n\t\tstring bindingsText = GetJoystickBindings(actionName);\n\t\tref array<string> bindings = {};\n\t\tbindingsText.Split(" / ", bindings, true);\n\t\tforeach (string binding : bindings)\n\t\t{\n\t\t\tif (binding.IndexOf(":axis") >= 0)\n\t\t\t\treturn binding;\n\t\t}\n\t\treturn string.Empty;\n\t}\n\n''',
    "add directional binding lookup",
)

src = replace_once(
    src,
    '''\t\tm_sYawAxisBinding = ResolveAxisBindingFromActions("PFC_Yaw", "HelicopterAntiTorqueLeft", "HelicopterAntiTorqueRight");\n\n\t\tm_iRollAxis = GetRawAxisFromBinding(m_sRollAxisBinding);''',
    '''\t\tm_sYawAxisBinding = ResolveAxisBindingFromActions("PFC_Yaw", "HelicopterAntiTorqueLeft", "HelicopterAntiTorqueRight");\n\n\t\tm_sFreelookUpAxisBinding = GetDirectionalAxisBindingFromAction("FreelookUp");\n\t\tm_sFreelookDownAxisBinding = GetDirectionalAxisBindingFromAction("FreelookDown");\n\t\tm_sFreelookRightAxisBinding = GetDirectionalAxisBindingFromAction("FreelookRight");\n\t\tm_sFreelookLeftAxisBinding = GetDirectionalAxisBindingFromAction("FreelookLeft");\n\n\t\tm_iRollAxis = GetRawAxisFromBinding(m_sRollAxisBinding);''',
    "refresh Free Look bindings",
)

src = replace_once(
    src,
    '''\t\tPrint(string.Format("[HOTAS Debugger] Config axis assignments: roll=%1 pitch=%2 throttle=%3 yaw=%4", GetAxisAssignmentDisplayName(0), GetAxisAssignmentDisplayName(1), GetAxisAssignmentDisplayName(2), GetAxisAssignmentDisplayName(3)), LogLevel.NORMAL);\n\t}\n''',
    '''\t\tPrint(string.Format("[HOTAS Debugger] Config axis assignments: roll=%1 pitch=%2 throttle=%3 yaw=%4", GetAxisAssignmentDisplayName(0), GetAxisAssignmentDisplayName(1), GetAxisAssignmentDisplayName(2), GetAxisAssignmentDisplayName(3)), LogLevel.NORMAL);\n\t\tPrint(string.Format("[HOTAS Debugger] Free Look assignments: up=%1 down=%2 right=%3 left=%4", GetFreelookAssignmentDisplayName(0), GetFreelookAssignmentDisplayName(1), GetFreelookAssignmentDisplayName(2), GetFreelookAssignmentDisplayName(3)), LogLevel.NORMAL);\n\t}\n''',
    "log Free Look bindings",
)

src = src.replace('MakeReadableBinding(bindingsText)', 'MakeReadableBinding(bindingsText, actionName)')
if src.count('MakeReadableBinding(bindingsText, actionName)') < 2:
    raise RuntimeError("expected both HUD MakeReadableBinding calls to include actionName")

src = replace_once(
    src,
    '''\tprotected string MakeReadableBinding(string bindingsText)\n\t{''',
    '''\tprotected string GetFreelookLabelForAction(string actionName)\n\t{\n\t\tswitch (actionName)\n\t\t{\n\t\t\tcase "FreelookUp": return m_sFreelookUpLabel;\n\t\t\tcase "FreelookDown": return m_sFreelookDownLabel;\n\t\t\tcase "FreelookRight": return m_sFreelookRightLabel;\n\t\t\tcase "FreelookLeft": return m_sFreelookLeftLabel;\n\t\t}\n\t\treturn string.Empty;\n\t}\n\n\tprotected string MakeReadableBinding(string bindingsText, string actionName = string.Empty)\n\t{''',
    "add action-sensitive Free Look label helper",
)

src = replace_once(
    src,
    '''\t\t\t\tstring normalizedBinding = NormalizeAxisBinding(binding);\n\t\t\t\tstring axisName;\n\t\t\t\tif (!m_sRollAxisBinding.IsEmpty() && normalizedBinding == m_sRollAxisBinding)''',
    '''\t\t\t\tstring normalizedBinding = NormalizeAxisBinding(binding);\n\t\t\t\tstring axisName = GetFreelookLabelForAction(actionName);\n\t\t\t\tif (!axisName.IsEmpty())\n\t\t\t\t{\n\t\t\t\t\treadable = axisName;\n\t\t\t\t}\n\t\t\t\telse if (!m_sRollAxisBinding.IsEmpty() && normalizedBinding == m_sRollAxisBinding)''',
    "prefer Free Look custom label",
)

src = replace_once(
    src,
    '''\t\t\t\telse if (!m_sYawAxisBinding.IsEmpty() && normalizedBinding == m_sYawAxisBinding)\n\t\t\t\t\taxisName = m_sYawAxisLabel;\n\n\t\t\t\tif (!axisName.IsEmpty())\n\t\t\t\t\treadable = string.Format("%1 %2", axisName, direction);\n\t\t\t\telse\n\t\t\t\t\treadable = string.Format("AXIS %1%2", rawAxis + 1, direction);''',
    '''\t\t\t\telse if (!m_sYawAxisBinding.IsEmpty() && normalizedBinding == m_sYawAxisBinding)\n\t\t\t\t\taxisName = m_sYawAxisLabel;\n\n\t\t\t\tif (!GetFreelookLabelForAction(actionName).IsEmpty())\n\t\t\t\t{\n\t\t\t\t\t// Direction is already part of the user's text (for example "Thumbstick Up").\n\t\t\t\t}\n\t\t\t\telse if (!axisName.IsEmpty())\n\t\t\t\t\treadable = string.Format("%1 %2", axisName, direction);\n\t\t\t\telse\n\t\t\t\t\treadable = string.Format("AXIS %1%2", rawAxis + 1, direction);''',
    "avoid appending +/- to Free Look custom label",
)

src = src.replace('case "FreelookUp": return "Look Up";', 'case "FreelookUp": return "Free Look Up";')
src = src.replace('case "FreelookDown": return "Look Down";', 'case "FreelookDown": return "Free Look Down";')
src = src.replace('case "FreelookLeft": return "Look Left";', 'case "FreelookLeft": return "Free Look Left";')
src = src.replace('case "FreelookRight": return "Look Right";', 'case "FreelookRight": return "Free Look Right";')

path.write_text(src, encoding="utf-8")


# -----------------------------------------------------------------------------
# HOTASSettingsTab.c
# -----------------------------------------------------------------------------
path = Path("Scripts/Game/HOTASDebugger/HOTASSettingsTab.c")
src = path.read_text(encoding="utf-8")

src = replace_once(
    src,
    '''\tprotected ref array<SCR_EditBoxComponent> m_AxisLabelEditors = {};\n\tprotected ref array<string> m_UserConfigs = {};''',
    '''\tprotected ref array<SCR_EditBoxComponent> m_AxisLabelEditors = {};\n\tprotected ref array<SCR_EditBoxComponent> m_FreelookLabelEditors = {};\n\tprotected ref array<string> m_UserConfigs = {};''',
    "add Free Look editor array",
)

src = replace_once(
    src,
    '''\t\tSetupHudSliders();\n\t\tSetupAxisLabelEditors();\n\t\tm_bLoading = false;''',
    '''\t\tSetupHudSliders();\n\t\tSetupAxisLabelEditors();\n\t\tSetupFreelookLabelEditors();\n\t\tm_bLoading = false;''',
    "setup Free Look editors",
)

src = replace_once(
    src,
    '''\t\tSyncHudSliders();\n\t\tSyncAxisLabelEditors();\n\t\tm_bLoading = false;''',
    '''\t\tSyncHudSliders();\n\t\tSyncAxisLabelEditors();\n\t\tSyncFreelookLabelEditors();\n\t\tm_bLoading = false;''',
    "sync Free Look editors on show",
)

src = replace_once(
    src,
    '''\t\tSyncAxisLabelEditors();\n\t\tm_bLoading = false;\n\t}\n\n\t//------------------------------------------------------------------------------------------------\n\tprotected void SetupHudControls()''',
    '''\t\tSyncAxisLabelEditors();\n\t\tSyncFreelookLabelEditors();\n\t\tm_bLoading = false;\n\t}\n\n\t//------------------------------------------------------------------------------------------------\n\tprotected void SetupHudControls()''',
    "sync Free Look after config change",
)

anchor = '''\tprotected void OnAxisLabelConfirmed(SCR_EditBoxComponent component, string value)\n\t{\n\t\tif (m_bLoading)\n\t\t\treturn;\n\n\t\tfor (int i = 0; i < m_AxisLabelEditors.Count(); i++)\n\t\t{\n\t\t\tif (m_AxisLabelEditors[i] != component)\n\t\t\t\tcontinue;\n\n\t\t\tHOTASDebugController.GetInstance().SetAxisCustomLabel(i, value);\n\t\t\treturn;\n\t\t}\n\t}\n\n'''
insert = anchor + '''\t//------------------------------------------------------------------------------------------------\n\tprotected void SetupFreelookLabelEditors()\n\t{\n\t\tm_FreelookLabelEditors.Clear();\n\t\tarray<string> widgetNames = { "FreelookUpAxis", "FreelookDownAxis", "FreelookRightAxis", "FreelookLeftAxis" };\n\t\tarray<string> placeholders = { "e.g. Thumbstick Up", "e.g. Thumbstick Down", "e.g. Thumbstick Right", "e.g. Thumbstick Left" };\n\t\tHOTASDebugController controller = HOTASDebugController.GetInstance();\n\n\t\tfor (int i = 0; i < widgetNames.Count(); i++)\n\t\t{\n\t\t\tSCR_EditBoxComponent editor = FindEditBox(widgetNames[i]);\n\t\t\tm_FreelookLabelEditors.Insert(editor);\n\t\t\tif (!editor)\n\t\t\t\tcontinue;\n\n\t\t\teditor.SetLabel(controller.GetFreelookSettingRowLabel(i));\n\t\t\teditor.SetValue(controller.GetFreelookCustomLabel(i));\n\t\t\teditor.SetPlaceholderText(placeholders[i]);\n\t\t\teditor.m_OnConfirm.Insert(OnFreelookLabelConfirmed);\n\t\t}\n\t}\n\n\t//------------------------------------------------------------------------------------------------\n\tprotected void SyncFreelookLabelEditors()\n\t{\n\t\tHOTASDebugController controller = HOTASDebugController.GetInstance();\n\t\tfor (int i = 0; i < m_FreelookLabelEditors.Count(); i++)\n\t\t{\n\t\t\tSCR_EditBoxComponent editor = m_FreelookLabelEditors[i];\n\t\t\tif (!editor)\n\t\t\t\tcontinue;\n\n\t\t\teditor.SetLabel(controller.GetFreelookSettingRowLabel(i));\n\t\t\teditor.SetValue(controller.GetFreelookCustomLabel(i));\n\t\t}\n\t}\n\n\t//------------------------------------------------------------------------------------------------\n\tprotected void OnFreelookLabelConfirmed(SCR_EditBoxComponent component, string value)\n\t{\n\t\tif (m_bLoading)\n\t\t\treturn;\n\n\t\tfor (int i = 0; i < m_FreelookLabelEditors.Count(); i++)\n\t\t{\n\t\t\tif (m_FreelookLabelEditors[i] != component)\n\t\t\t\tcontinue;\n\n\t\t\tHOTASDebugController.GetInstance().SetFreelookCustomLabel(i, value);\n\t\t\treturn;\n\t\t}\n\t}\n\n'''
src = replace_once(src, anchor, insert, "add Free Look edit-box handlers")

path.write_text(src, encoding="utf-8")


# -----------------------------------------------------------------------------
# HOTASSettings.layout
# -----------------------------------------------------------------------------
path = Path("UI/layouts/Menus/SettingsSubMenus/HOTASSettings.layout")
src = path.read_text(encoding="utf-8")

anchor = '''      ButtonWidgetClass "{8C52D9F7A31B6442}" : "{0022F0B45ADBC5AC}UI/layouts/WidgetLibrary/EditBox/WLib_EditBox.layout" {\n       Name "YawAxis"\n       Slot LayoutSlot "{8C52D9F7A31B6443}" { Padding 4 4 4 4 }\n       components {\n        SCR_EditBoxComponent "{547290FFBD5B33E9}" {\n         m_sLabel "Yaw Axis - Unassigned"\n         m_sLabelLayout "{F003823FF141983C}UI/layouts/Menus/SettingsMenu/CustomWidgets/SettingsLabel.layout"\n         m_bShowWriteIcon 0\n        }\n       }\n      }\n'''
addition = anchor + '''      ButtonWidgetClass "{8C52D9F7A31B6448}" : "{0022F0B45ADBC5AC}UI/layouts/WidgetLibrary/EditBox/WLib_EditBox.layout" {\n       Name "FreelookUpAxis"\n       Slot LayoutSlot "{8C52D9F7A31B6449}" { Padding 4 4 4 4 }\n       components {\n        SCR_EditBoxComponent "{547290FFBD5B33E9}" {\n         m_sLabel "Free Look Up - Unassigned"\n         m_sLabelLayout "{F003823FF141983C}UI/layouts/Menus/SettingsMenu/CustomWidgets/SettingsLabel.layout"\n         m_bShowWriteIcon 0\n        }\n       }\n      }\n      ButtonWidgetClass "{8C52D9F7A31B644A}" : "{0022F0B45ADBC5AC}UI/layouts/WidgetLibrary/EditBox/WLib_EditBox.layout" {\n       Name "FreelookDownAxis"\n       Slot LayoutSlot "{8C52D9F7A31B644B}" { Padding 4 4 4 4 }\n       components {\n        SCR_EditBoxComponent "{547290FFBD5B33E9}" {\n         m_sLabel "Free Look Down - Unassigned"\n         m_sLabelLayout "{F003823FF141983C}UI/layouts/Menus/SettingsMenu/CustomWidgets/SettingsLabel.layout"\n         m_bShowWriteIcon 0\n        }\n       }\n      }\n      ButtonWidgetClass "{8C52D9F7A31B644C}" : "{0022F0B45ADBC5AC}UI/layouts/WidgetLibrary/EditBox/WLib_EditBox.layout" {\n       Name "FreelookRightAxis"\n       Slot LayoutSlot "{8C52D9F7A31B644D}" { Padding 4 4 4 4 }\n       components {\n        SCR_EditBoxComponent "{547290FFBD5B33E9}" {\n         m_sLabel "Free Look Right - Unassigned"\n         m_sLabelLayout "{F003823FF141983C}UI/layouts/Menus/SettingsMenu/CustomWidgets/SettingsLabel.layout"\n         m_bShowWriteIcon 0\n        }\n       }\n      }\n      ButtonWidgetClass "{8C52D9F7A31B644E}" : "{0022F0B45ADBC5AC}UI/layouts/WidgetLibrary/EditBox/WLib_EditBox.layout" {\n       Name "FreelookLeftAxis"\n       Slot LayoutSlot "{8C52D9F7A31B644F}" { Padding 4 4 4 4 }\n       components {\n        SCR_EditBoxComponent "{547290FFBD5B33E9}" {\n         m_sLabel "Free Look Left - Unassigned"\n         m_sLabelLayout "{F003823FF141983C}UI/layouts/Menus/SettingsMenu/CustomWidgets/SettingsLabel.layout"\n         m_bShowWriteIcon 0\n        }\n       }\n      }\n'''
src = replace_once(src, anchor, addition, "add Free Look settings rows")
path.write_text(src, encoding="utf-8")
