from pathlib import Path

path = Path('Scripts/Game/HOTASDebugger/HOTASDebugger.c')
src = path.read_text(encoding='utf-8')


def replace_once(old, new, label):
    global src
    if old not in src:
        raise SystemExit(f'{label} anchor not found')
    src = src.replace(old, new, 1)


replace_once(
'''\tprotected Widget m_HudLayoutRoot;\n\tprotected RichTextWidget m_InputText;''',
'''\tprotected Widget m_HudLayoutRoot;\n\tprotected Widget m_HudRootWidget;\n\tprotected RichTextWidget m_InputText;''',
'layout root field',
)

replace_once(
'''\tprotected bool m_bInitialized;\n\tprotected bool m_bDebugMode = false;\n\tprotected int m_iEventCounter;''',
'''\tprotected bool m_bInitialized;\n\tprotected bool m_bDebugMode = false;\n\tprotected bool m_bHudEnabled = true;\n\tprotected int m_iEventCounter;\n\n\tprotected static const int HOTAS_CONTEXT_NONE = 0;\n\tprotected static const int HOTAS_CONTEXT_TURRET = 1;\n\tprotected static const int HOTAS_CONTEXT_HELICOPTER = 2;\n\tprotected static const int HOTAS_CONTEXT_FIXED_WING = 3;''',
'HUD state fields',
)

replace_once(
'''\tprotected void RegisterListeners()\n\t{\n\t\tforeach (string actionName : m_WatchedActions)\n\t\t\tm_InputManager.AddActionListener(actionName, EActionTrigger.DOWN, OnActionTriggered);\n\t}''',
'''\tprotected void RegisterListeners()\n\t{\n\t\tforeach (string actionName : m_WatchedActions)\n\t\t\tm_InputManager.AddActionListener(actionName, EActionTrigger.DOWN, OnActionTriggered);\n\n\t\tm_InputManager.AddActionListener("HOTASSettingsToggle", EActionTrigger.DOWN, OnSettingsToggle);\n\t}''',
'RegisterListeners',
)

replace_once(
'''\t\tif (m_InputManager)\n\t\t{\n\t\t\tforeach (string actionName : m_WatchedActions)\n\t\t\t\tm_InputManager.RemoveActionListener(actionName, EActionTrigger.DOWN, OnActionTriggered);\n\t\t}''',
'''\t\tif (m_InputManager)\n\t\t{\n\t\t\tforeach (string actionName : m_WatchedActions)\n\t\t\t\tm_InputManager.RemoveActionListener(actionName, EActionTrigger.DOWN, OnActionTriggered);\n\n\t\t\tm_InputManager.RemoveActionListener("HOTASSettingsToggle", EActionTrigger.DOWN, OnSettingsToggle);\n\t\t}''',
'Shutdown listeners',
)

replace_once(
'''\t\tif (m_HudLayoutRoot)\n\t\t\tm_HudLayoutRoot.RemoveFromHierarchy();\n\t\telse\n\t\t{\n\t\t\tif (m_DebugText)\n\t\t\t\tm_DebugText.RemoveFromHierarchy();\n\t\t\tif (m_HudBackground)\n\t\t\t\tm_HudBackground.RemoveFromHierarchy();\n\t\t}\n\n\t\tm_DebugText = null;\n\t\tm_HudBackground = null;\n\t\tm_HudLayoutRoot = null;\n\t\tm_InputText = null;\n\t\tm_SeparatorText = null;\n\t\tm_ActionText = null;\n\t\tm_bUsingLayoutHud = false;''',
'''\t\tDestroyHud();''',
'Shutdown HUD cleanup',
)

create_anchor = '''\tprotected void CreateHud()\n\t{'''
create_helpers = '''\tprotected void DestroyHud()\n\t{\n\t\tGetGame().GetCallqueue().Remove(StartFade);\n\t\tGetGame().GetCallqueue().Remove(FadeStep);\n\n\t\tif (m_HudLayoutRoot)\n\t\t\tm_HudLayoutRoot.RemoveFromHierarchy();\n\t\telse\n\t\t{\n\t\t\tif (m_DebugText)\n\t\t\t\tm_DebugText.RemoveFromHierarchy();\n\t\t\tif (m_HudBackground)\n\t\t\t\tm_HudBackground.RemoveFromHierarchy();\n\t\t}\n\n\t\tm_DebugText = null;\n\t\tm_HudBackground = null;\n\t\tm_HudLayoutRoot = null;\n\t\tm_HudRootWidget = null;\n\t\tm_InputText = null;\n\t\tm_SeparatorText = null;\n\t\tm_ActionText = null;\n\t\tm_bUsingLayoutHud = false;\n\t}\n\n\tprotected void RebuildHud()\n\t{\n\t\tDestroyHud();\n\t\tCreateHud();\n\t}\n\n\tprotected void OnSettingsToggle(float value = 0.0, EActionTrigger reason = 0, string actionName = string.Empty)\n\t{\n\t\tMenuManager menuManager = GetGame().GetMenuManager();\n\t\tif (!menuManager)\n\t\t\treturn;\n\n\t\tMenuBase existing = menuManager.FindMenuByPreset(ChimeraMenuPreset.HOTASSettingsMenu);\n\t\tif (existing)\n\t\t\tmenuManager.CloseMenu(existing);\n\t\telse\n\t\t\tmenuManager.OpenMenu(ChimeraMenuPreset.HOTASSettingsMenu);\n\t}\n\n'''
replace_once(create_anchor, create_helpers + create_anchor, 'CreateHud method')

replace_once(
'''\t\tif (!workspace)\n\t\t{\n\t\t\tPrint("[HOTAS Debugger] Workspace is not available", LogLevel.ERROR);\n\t\t\treturn;\n\t\t}\n\n\t\t// Normal mode prefers the Workbench-editable layout.''',
'''\t\tif (!workspace)\n\t\t{\n\t\t\tPrint("[HOTAS Debugger] Workspace is not available", LogLevel.ERROR);\n\t\t\treturn;\n\t\t}\n\n\t\tif (!m_bHudEnabled)\n\t\t\treturn;\n\n\t\t// Normal mode prefers the Workbench-editable layout.''',
'CreateHud enabled gate',
)

replace_once(
'''\t\tm_InputText = RichTextWidget.Cast(m_HudLayoutRoot.FindAnyWidget("InputText"));\n\t\tm_SeparatorText = RichTextWidget.Cast(m_HudLayoutRoot.FindAnyWidget("SeparatorText"));\n\t\tm_ActionText = RichTextWidget.Cast(m_HudLayoutRoot.FindAnyWidget("ActionText"));\n\t\tm_HudBackground = m_HudLayoutRoot.FindAnyWidget("Background");\n\n\t\tif (!m_InputText || !m_SeparatorText || !m_ActionText)''',
'''\t\tm_HudRootWidget = m_HudLayoutRoot.FindAnyWidget("HudRoot");\n\t\tm_InputText = RichTextWidget.Cast(m_HudLayoutRoot.FindAnyWidget("InputText"));\n\t\tm_SeparatorText = RichTextWidget.Cast(m_HudLayoutRoot.FindAnyWidget("SeparatorText"));\n\t\tm_ActionText = RichTextWidget.Cast(m_HudLayoutRoot.FindAnyWidget("ActionText"));\n\t\tm_HudBackground = m_HudLayoutRoot.FindAnyWidget("BackgroundImage");\n\t\tif (!m_HudBackground)\n\t\t\tm_HudBackground = m_HudLayoutRoot.FindAnyWidget("Background");\n\n\t\tif (!m_HudRootWidget || !m_InputText || !m_SeparatorText || !m_ActionText)''',
'layout widget lookup',
)

replace_once(
'''\t\t\tm_HudLayoutRoot = null;\n\t\t\tm_InputText = null;''',
'''\t\t\tm_HudLayoutRoot = null;\n\t\t\tm_HudRootWidget = null;\n\t\t\tm_InputText = null;''',
'layout failure cleanup',
)

replace_once(
'''\t\tm_bUsingLayoutHud = true;\n\t\tm_SeparatorText.SetText("|");\n\t\tm_HudLayoutRoot.SetOpacity(0.0);\n\t\tif (m_HudBackground && !m_bBackgroundEnabled)\n\t\t\tm_HudBackground.SetOpacity(0.0);''',
'''\t\tm_bUsingLayoutHud = true;\n\t\tm_SeparatorText.SetText("|");\n\t\tApplyLayoutHudPresentation();\n\t\tm_HudLayoutRoot.SetOpacity(0.0);''',
'layout success setup',
)

position_anchor = '''\tprotected void GetHudPosition(WorkspaceWidget workspace, int width, int height, out int left, out int top)'''
presentation = '''\tprotected void ApplyLayoutHudPresentation()\n\t{\n\t\tif (!m_HudRootWidget || !m_InputText || !m_SeparatorText || !m_ActionText)\n\t\t\treturn;\n\n\t\tWorkspaceWidget workspace = GetGame().GetWorkspace();\n\t\tif (!workspace)\n\t\t\treturn;\n\n\t\tint width = Math.Round(700 * m_fHudScale);\n\t\tint height = Math.Round(70 * m_fHudScale);\n\t\tint left;\n\t\tint top;\n\t\tGetHudPosition(workspace, width, height, left, top);\n\n\t\tFrameSlot.SetPos(m_HudRootWidget, left, top);\n\t\tFrameSlot.SetSize(m_HudRootWidget, width, height);\n\n\t\tint fontSize = Math.Round(26 * m_fHudScale);\n\t\tm_InputText.SetExactFontSize(fontSize);\n\t\tm_SeparatorText.SetExactFontSize(fontSize);\n\t\tm_ActionText.SetExactFontSize(fontSize);\n\n\t\tif (m_HudBackground)\n\t\t{\n\t\t\tif (m_bBackgroundEnabled)\n\t\t\t\tm_HudBackground.SetOpacity(m_fBackgroundOpacity);\n\t\t\telse\n\t\t\t\tm_HudBackground.SetOpacity(0.0);\n\t\t}\n\t}\n\n'''
replace_once(position_anchor, presentation + position_anchor, 'GetHudPosition anchor')

replace_once(
'''\t\t\t\tdefaults.WriteLine("# HOTAS Input HUD settings");\n\t\t\t\tdefaults.WriteLine("# position: top_left, top_center, top_right, center_left, center, center_right, bottom_left, bottom_center, bottom_right");''',
'''\t\t\t\tdefaults.WriteLine("# HOTAS Input HUD settings");\n\t\t\t\tdefaults.WriteLine("hud_enabled=1");\n\t\t\t\tdefaults.WriteLine("debug_mode=0");\n\t\t\t\tdefaults.WriteLine("# position: top_left, top_center, top_right, center_left, center, center_right, bottom_left, bottom_center, bottom_right");''',
'default settings header',
)

replace_once(
'''\t\t\tstring key = parts[0].Trim();\n\t\t\tstring value = parts[1].Trim();\n\t\t\tif (key == "position")''',
'''\t\t\tstring key = parts[0].Trim();\n\t\t\tstring value = parts[1].Trim();\n\t\t\tif (key == "hud_enabled")\n\t\t\t\tm_bHudEnabled = value.ToInt(1) != 0;\n\t\t\telse if (key == "debug_mode")\n\t\t\t\tm_bDebugMode = value.ToInt(0) != 0;\n\t\t\telse if (key == "position")''',
'parse new settings',
)

show_anchor = '''\tprotected void ShowHud()'''
settings_methods = '''\tprotected int BoolToInt(bool value)\n\t{\n\t\tif (value)\n\t\t\treturn 1;\n\t\treturn 0;\n\t}\n\n\tprotected void SaveHudSettings()\n\t{\n\t\tFileHandle file = FileIO.OpenFile("$profile:HOTASHudSettings.txt", FileMode.WRITE);\n\t\tif (!file)\n\t\t\treturn;\n\n\t\tfile.WriteLine("# HOTAS Input HUD settings");\n\t\tfile.WriteLine(string.Format("hud_enabled=%1", BoolToInt(m_bHudEnabled)));\n\t\tfile.WriteLine(string.Format("debug_mode=%1", BoolToInt(m_bDebugMode)));\n\t\tfile.WriteLine(string.Format("position=%1", m_sHudPosition));\n\t\tfile.WriteLine(string.Format("scale=%1", m_fHudScale));\n\t\tfile.WriteLine(string.Format("fade_delay_ms=%1", m_iFadeDelayMs));\n\t\tfile.WriteLine(string.Format("fade_duration_ms=%1", m_iFadeDurationMs));\n\t\tfile.WriteLine(string.Format("background=%1", BoolToInt(m_bBackgroundEnabled)));\n\t\tfile.WriteLine(string.Format("background_opacity=%1", m_fBackgroundOpacity));\n\t\tfile.WriteLine("# Raw joystick axis mapping. Human-facing menu labels are one-based; -1 disables a semantic label.");\n\t\tfile.WriteLine(string.Format("roll_axis=%1", m_iRollAxis));\n\t\tfile.WriteLine(string.Format("pitch_axis=%1", m_iPitchAxis));\n\t\tfile.WriteLine(string.Format("throttle_axis=%1", m_iThrottleAxis));\n\t\tfile.WriteLine(string.Format("yaw_axis=%1", m_iYawAxis));\n\t\tfile.Close();\n\t}\n\n\tint GetSettingsCount()\n\t{\n\t\treturn 12;\n\t}\n\n\tstring GetSettingLabel(int index)\n\t{\n\t\tswitch (index)\n\t\t{\n\t\t\tcase 0: return "HUD Enabled";\n\t\t\tcase 1: return "Position";\n\t\t\tcase 2: return "Scale";\n\t\t\tcase 3: return "Fade Delay";\n\t\t\tcase 4: return "Fade Duration";\n\t\t\tcase 5: return "Background";\n\t\t\tcase 6: return "Background Opacity";\n\t\t\tcase 7: return "Roll Axis";\n\t\t\tcase 8: return "Pitch Axis";\n\t\t\tcase 9: return "Throttle Axis";\n\t\t\tcase 10: return "Yaw Axis";\n\t\t\tcase 11: return "Debug Mode";\n\t\t}\n\t\treturn "Unknown";\n\t}\n\n\tprotected string GetAxisSettingValue(int rawAxis)\n\t{\n\t\tif (rawAxis < 0)\n\t\t\treturn "Disabled";\n\t\treturn string.Format("Axis %1", rawAxis + 1);\n\t}\n\n\tprotected string GetHudPositionDisplayName()\n\t{\n\t\tswitch (m_sHudPosition)\n\t\t{\n\t\t\tcase "top_left": return "Top Left";\n\t\t\tcase "top_center": return "Top Center";\n\t\t\tcase "top_right": return "Top Right";\n\t\t\tcase "center_left": return "Center Left";\n\t\t\tcase "center": return "Center";\n\t\t\tcase "center_right": return "Center Right";\n\t\t\tcase "bottom_left": return "Bottom Left";\n\t\t\tcase "bottom_right": return "Bottom Right";\n\t\t}\n\t\treturn "Bottom Center";\n\t}\n\n\tstring GetSettingValue(int index)\n\t{\n\t\tswitch (index)\n\t\t{\n\t\t\tcase 0:\n\t\t\t\tif (m_bHudEnabled) return "On";\n\t\t\t\treturn "Off";\n\t\t\tcase 1: return GetHudPositionDisplayName();\n\t\t\tcase 2: return string.Format("%1x", m_fHudScale.ToString(1));\n\t\t\tcase 3: return string.Format("%1 ms", m_iFadeDelayMs);\n\t\t\tcase 4: return string.Format("%1 ms", m_iFadeDurationMs);\n\t\t\tcase 5:\n\t\t\t\tif (m_bBackgroundEnabled) return "On";\n\t\t\t\treturn "Off";\n\t\t\tcase 6: return string.Format("%1%", Math.Round(m_fBackgroundOpacity * 100));\n\t\t\tcase 7: return GetAxisSettingValue(m_iRollAxis);\n\t\t\tcase 8: return GetAxisSettingValue(m_iPitchAxis);\n\t\t\tcase 9: return GetAxisSettingValue(m_iThrottleAxis);\n\t\t\tcase 10: return GetAxisSettingValue(m_iYawAxis);\n\t\t\tcase 11:\n\t\t\t\tif (m_bDebugMode) return "On";\n\t\t\t\treturn "Off";\n\t\t}\n\t\treturn "";\n\t}\n\n\tprotected int GetHudPositionIndex()\n\t{\n\t\tswitch (m_sHudPosition)\n\t\t{\n\t\t\tcase "top_left": return 0;\n\t\t\tcase "top_center": return 1;\n\t\t\tcase "top_right": return 2;\n\t\t\tcase "center_left": return 3;\n\t\t\tcase "center": return 4;\n\t\t\tcase "center_right": return 5;\n\t\t\tcase "bottom_left": return 6;\n\t\t\tcase "bottom_center": return 7;\n\t\t\tcase "bottom_right": return 8;\n\t\t}\n\t\treturn 7;\n\t}\n\n\tprotected void SetHudPositionIndex(int index)\n\t{\n\t\twhile (index < 0)\n\t\t\tindex += 9;\n\t\twhile (index >= 9)\n\t\t\tindex -= 9;\n\n\t\tswitch (index)\n\t\t{\n\t\t\tcase 0: m_sHudPosition = "top_left"; break;\n\t\t\tcase 1: m_sHudPosition = "top_center"; break;\n\t\t\tcase 2: m_sHudPosition = "top_right"; break;\n\t\t\tcase 3: m_sHudPosition = "center_left"; break;\n\t\t\tcase 4: m_sHudPosition = "center"; break;\n\t\t\tcase 5: m_sHudPosition = "center_right"; break;\n\t\t\tcase 6: m_sHudPosition = "bottom_left"; break;\n\t\t\tcase 7: m_sHudPosition = "bottom_center"; break;\n\t\t\tcase 8: m_sHudPosition = "bottom_right"; break;\n\t\t}\n\t}\n\n\tvoid AdjustSetting(int index, int direction)\n\t{\n\t\tif (direction == 0)\n\t\t\tdirection = 1;\n\n\t\tswitch (index)\n\t\t{\n\t\t\tcase 0:\n\t\t\t\tm_bHudEnabled = !m_bHudEnabled;\n\t\t\t\tbreak;\n\t\t\tcase 1:\n\t\t\t\tSetHudPositionIndex(GetHudPositionIndex() + direction);\n\t\t\t\tbreak;\n\t\t\tcase 2:\n\t\t\t\tm_fHudScale = Math.Clamp(m_fHudScale + (0.1 * direction), 0.5, 2.0);\n\t\t\t\tbreak;\n\t\t\tcase 3:\n\t\t\t\tm_iFadeDelayMs = Math.ClampInt(m_iFadeDelayMs + (100 * direction), 0, 10000);\n\t\t\t\tbreak;\n\t\t\tcase 4:\n\t\t\t\tm_iFadeDurationMs = Math.ClampInt(m_iFadeDurationMs + (50 * direction), 0, 5000);\n\t\t\t\tbreak;\n\t\t\tcase 5:\n\t\t\t\tm_bBackgroundEnabled = !m_bBackgroundEnabled;\n\t\t\t\tbreak;\n\t\t\tcase 6:\n\t\t\t\tm_fBackgroundOpacity = Math.Clamp(m_fBackgroundOpacity + (0.05 * direction), 0.0, 1.0);\n\t\t\t\tbreak;\n\t\t\tcase 7:\n\t\t\t\tm_iRollAxis = Math.ClampInt(m_iRollAxis + direction, -1, 63);\n\t\t\t\tbreak;\n\t\t\tcase 8:\n\t\t\t\tm_iPitchAxis = Math.ClampInt(m_iPitchAxis + direction, -1, 63);\n\t\t\t\tbreak;\n\t\t\tcase 9:\n\t\t\t\tm_iThrottleAxis = Math.ClampInt(m_iThrottleAxis + direction, -1, 63);\n\t\t\t\tbreak;\n\t\t\tcase 10:\n\t\t\t\tm_iYawAxis = Math.ClampInt(m_iYawAxis + direction, -1, 63);\n\t\t\t\tbreak;\n\t\t\tcase 11:\n\t\t\t\tm_bDebugMode = !m_bDebugMode;\n\t\t\t\tbreak;\n\t\t\tdefault:\n\t\t\t\treturn;\n\t\t}\n\n\t\tSaveHudSettings();\n\t\tRebuildHud();\n\t}\n\n'''
replace_once(show_anchor, settings_methods + show_anchor, 'ShowHud anchor')

replace_once(
'''\tprotected void ShowHud()\n\t{\n\t\tif (m_bDebugMode)''',
'''\tprotected void ShowHud()\n\t{\n\t\tif (!m_bHudEnabled)\n\t\t\treturn;\n\t\tif (m_bDebugMode)''',
'ShowHud enabled gate',
)

start = src.find('\tprotected bool IsPlayerInAircraftOrTurret()\n\t{')
end_marker = '\n\tprotected void OnActionTriggered(float value = 0.0, EActionTrigger reason = 0, string actionName = string.Empty)'
end = src.find(end_marker, start)
if start < 0 or end < 0:
    raise SystemExit('aircraft/turret context method block not found')
context_helpers = '''\tprotected int GetPlayerHotasContext()\n\t{\n\t\tChimeraCharacter character = ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());\n\t\tif (!character)\n\t\t\treturn HOTAS_CONTEXT_NONE;\n\n\t\tCompartmentAccessComponent compartmentAccess = character.GetCompartmentAccessComponent();\n\t\tif (!compartmentAccess || !compartmentAccess.IsInCompartment())\n\t\t\treturn HOTAS_CONTEXT_NONE;\n\n\t\tBaseCompartmentSlot slot = compartmentAccess.GetCompartment();\n\t\tif (!slot)\n\t\t\treturn HOTAS_CONTEXT_NONE;\n\n\t\tif (TurretCompartmentSlot.Cast(slot))\n\t\t\treturn HOTAS_CONTEXT_TURRET;\n\n\t\tif (!PilotCompartmentSlot.Cast(slot))\n\t\t\treturn HOTAS_CONTEXT_NONE;\n\n\t\tIEntity vehicle = compartmentAccess.GetVehicleCompartmentManagerOwner();\n\t\tif (!vehicle)\n\t\t\tvehicle = slot.GetOwner();\n\t\tif (!vehicle)\n\t\t\treturn HOTAS_CONTEXT_NONE;\n\n\t\tif (vehicle.FindComponent(HelicopterControllerComponent))\n\t\t\treturn HOTAS_CONTEXT_HELICOPTER;\n\n\t\tif (vehicle.FindComponent(SCR_CarControllerComponent))\n\t\t\treturn HOTAS_CONTEXT_NONE;\n\t\tif (vehicle.FindComponent(SCR_TrackedControllerComponent))\n\t\t\treturn HOTAS_CONTEXT_NONE;\n\n\t\t// A pilot seat without a known ground-vehicle controller is treated as fixed-wing.\n\t\t// This keeps compatibility with PFC and other modded aircraft controllers.\n\t\treturn HOTAS_CONTEXT_FIXED_WING;\n\t}\n\n\tprotected bool IsSharedHotasAction(string actionName)\n\t{\n\t\treturn actionName == "VehicleDoorToggle"\n\t\t\t|| actionName == "FocusToggle"\n\t\t\t|| actionName == "Freelook"\n\t\t\t|| actionName == "FreelookReset"\n\t\t\t|| actionName == "FreelookUp"\n\t\t\t|| actionName == "FreelookDown"\n\t\t\t|| actionName == "FreelookLeft"\n\t\t\t|| actionName == "FreelookRight"\n\t\t\t|| actionName == "VONDirectToggle"\n\t\t\t|| actionName == "VONChannel"\n\t\t\t|| actionName == "GadgetMap"\n\t\t\t|| actionName == "PerformAction"\n\t\t\t|| actionName == "SelectAction"\n\t\t\t|| actionName == "GetOut"\n\t\t\t|| actionName == "JumpOut";\n\t}\n\n\tprotected bool IsAircraftWeaponAction(string actionName)\n\t{\n\t\treturn actionName == "VehicleFire"\n\t\t\t|| actionName == "VehicleNextWeapon"\n\t\t\t|| actionName == "TurretFire"\n\t\t\t|| actionName == "TurretReload"\n\t\t\t|| actionName == "TurretNextWeapon"\n\t\t\t|| actionName == "TurretWeaponNextFireMode"\n\t\t\t|| actionName == "TurretWeaponNextRippleQuantity"\n\t\t\t|| actionName == "TurretADS"\n\t\t\t|| actionName == "TurretADSHold"\n\t\t\t|| actionName == "WeaponToggleSightsIllumination"\n\t\t\t|| actionName == "WeaponSwitchOptics";\n\t}\n\n\tprotected bool IsAircraftWcsAction(string actionName)\n\t{\n\t\tif (!actionName.StartsWith("WCS_Armament_"))\n\t\t\treturn false;\n\n\t\t// Ground-only smoke/stabilization actions should not appear while flying.\n\t\tif (actionName == "WCS_Armament_TurretStabilizationToggle")\n\t\t\treturn false;\n\t\tif (actionName == "WCS_Armament_DeploySmoke")\n\t\t\treturn false;\n\t\tif (actionName == "WCS_Armament_FireContinuousSmokeDispenser")\n\t\t\treturn false;\n\n\t\treturn true;\n\t}\n\n\tprotected bool IsTurretWcsAction(string actionName)\n\t{\n\t\tif (!actionName.StartsWith("WCS_Armament_"))\n\t\t\treturn false;\n\n\t\t// Countermeasure actions are aircraft-specific.\n\t\tif (actionName == "WCS_Armament_DeployFlares")\n\t\t\treturn false;\n\t\tif (actionName == "WCS_Armament_DeployChaffs")\n\t\t\treturn false;\n\n\t\treturn true;\n\t}\n\n\tprotected bool IsActionAllowedForContext(string actionName, int context)\n\t{\n\t\tif (IsSharedHotasAction(actionName))\n\t\t\treturn true;\n\n\t\tif (context == HOTAS_CONTEXT_TURRET)\n\t\t{\n\t\t\tif (actionName.StartsWith("Turret"))\n\t\t\t\treturn true;\n\t\t\tif (actionName.StartsWith("Weapon"))\n\t\t\t\treturn true;\n\t\t\tif (actionName == "VehicleFire" || actionName == "VehicleNextWeapon")\n\t\t\t\treturn true;\n\t\t\treturn IsTurretWcsAction(actionName);\n\t\t}\n\n\t\tif (context == HOTAS_CONTEXT_HELICOPTER)\n\t\t{\n\t\t\tif (actionName.StartsWith("Helicopter"))\n\t\t\t\treturn true;\n\t\t\tif (IsAircraftWeaponAction(actionName))\n\t\t\t\treturn true;\n\t\t\treturn IsAircraftWcsAction(actionName);\n\t\t}\n\n\t\tif (context == HOTAS_CONTEXT_FIXED_WING)\n\t\t{\n\t\t\tif (actionName.StartsWith("PFC_"))\n\t\t\t\treturn true;\n\t\t\tif (IsAircraftWeaponAction(actionName))\n\t\t\t\treturn true;\n\t\t\treturn IsAircraftWcsAction(actionName);\n\t\t}\n\n\t\treturn false;\n\t}\n'''
src = src[:start] + context_helpers + src[end:]

replace_once(
'''\t\t// Ignore watched actions completely unless the local player is currently\n\t\t// occupying an aircraft pilot seat or a turret seat.\n\t\tif (!IsPlayerInAircraftOrTurret())\n\t\t\treturn;\n\n\t\tm_iEventCounter++;''',
'''\t\tif (!m_bHudEnabled)\n\t\t\treturn;\n\n\t\tint hotasContext = GetPlayerHotasContext();\n\t\tif (hotasContext == HOTAS_CONTEXT_NONE)\n\t\t\treturn;\n\t\tif (!IsActionAllowedForContext(actionName, hotasContext))\n\t\t\treturn;\n\n\t\tm_iEventCounter++;''',
'OnActionTriggered context gate',
)

path.write_text(src, encoding='utf-8')

# The layout's alpha is moved to the runtime background-opacity setting. Keeping the image
# itself opaque preserves roughly the existing appearance at the default 55% setting.
layout_path = Path('UI/layouts/HUD/HOTAS/HOTASInputHUD.layout')
layout = layout_path.read_text(encoding='utf-8')
old_color = 'Color 0.0052 0.007 0.0091 0.6'
new_color = 'Color 0.0052 0.007 0.0091 1'
if old_color not in layout:
    raise SystemExit('HUD background color anchor not found')
layout_path.write_text(layout.replace(old_color, new_color, 1), encoding='utf-8')
