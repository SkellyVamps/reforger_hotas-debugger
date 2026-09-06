from pathlib import Path

path = Path('Scripts/Game/HOTASDebugger/HOTASDebugger.c')
src = path.read_text(encoding='utf-8')

src = src.replace(
'''\tprotected TextWidget m_DebugText;\n\tprotected ref array<string> m_WatchedActions = {};\n\tprotected bool m_bInitialized;\n\tprotected bool m_bDebugMode = false;\n\tprotected int m_iEventCounter;''',
'''\tprotected TextWidget m_DebugText;\n\tprotected Widget m_HudBackground;\n\tprotected ref array<string> m_WatchedActions = {};\n\tprotected bool m_bInitialized;\n\tprotected bool m_bDebugMode = false;\n\tprotected int m_iEventCounter;\n\n\t// Normal HUD user settings. Values are loaded from $profile:HOTASHudSettings.txt.\n\tprotected string m_sHudPosition = "bottom_center";\n\tprotected float m_fHudScale = 1.0;\n\tprotected int m_iFadeDelayMs = 1800;\n\tprotected int m_iFadeDurationMs = 350;\n\tprotected bool m_bBackgroundEnabled = true;\n\tprotected float m_fBackgroundOpacity = 0.55;\n\tprotected float m_fFadeOpacity = 1.0;''',
1,
)

src = src.replace(
'''\t\tBuildActionList();\n\t\tCreateHud();''',
'''\t\tBuildActionList();\n\t\tLoadHudSettings();\n\t\tCreateHud();''',
1,
)

src = src.replace(
'''\t\tif (m_DebugText)\n\t\t\tm_DebugText.RemoveFromHierarchy();\n\n\t\tm_DebugText = null;''',
'''\t\tGetGame().GetCallqueue().Remove(StartFade);\n\t\tGetGame().GetCallqueue().Remove(FadeStep);\n\n\t\tif (m_DebugText)\n\t\t\tm_DebugText.RemoveFromHierarchy();\n\t\tif (m_HudBackground)\n\t\t\tm_HudBackground.RemoveFromHierarchy();\n\n\t\tm_DebugText = null;\n\t\tm_HudBackground = null;''',
1,
)

start = src.index('\tprotected void CreateHud()')
end = src.index('\n\tprotected void OnActionTriggered', start)
new_create = r'''\tprotected void CreateHud()
\t{
\t\tWorkspaceWidget workspace = GetGame().GetWorkspace();
\t\tif (!workspace)
\t\t{
\t\t\tPrint("[HOTAS Debugger] Workspace is not available", LogLevel.ERROR);
\t\t\treturn;
\t\t}

\t\tint left;
\t\tint top;
\t\tint width;
\t\tint height;
\t\tint flags = WidgetFlags.VISIBLE | WidgetFlags.IGNORE_CURSOR | WidgetFlags.NOFOCUS | WidgetFlags.NO_LOCALIZATION;

\t\tif (m_bDebugMode)
\t\t{
\t\t\tleft = 40;
\t\t\ttop = 120;
\t\t\twidth = 900;
\t\t\theight = 180;
\t\t\tflags |= WidgetFlags.WRAP_TEXT;
\t\t}
\t\telse
\t\t{
\t\t\twidth = Math.Round(760 * m_fHudScale);
\t\t\theight = Math.Round(72 * m_fHudScale);
\t\t\tGetHudPosition(workspace, width, height, left, top);
\t\t\tflags |= WidgetFlags.CENTER | WidgetFlags.VCENTER;

\t\t\tif (m_bBackgroundEnabled)
\t\t\t{
\t\t\t\tm_HudBackground = workspace.CreateWidgetInWorkspace(
\t\t\t\t\tWidgetType.PanelWidgetTypeID,
\t\t\t\t\tleft,
\t\t\t\t\ttop,
\t\t\t\t\twidth,
\t\t\t\t\theight,
\t\t\t\t\tWidgetFlags.VISIBLE | WidgetFlags.IGNORE_CURSOR | WidgetFlags.NOFOCUS,
\t\t\t\t\tColor.FromInt(0xFF101418),
\t\t\t\t\t999
\t\t\t\t);
\t\t\t\tif (m_HudBackground)
\t\t\t\t\tm_HudBackground.SetOpacity(m_fBackgroundOpacity);
\t\t\t}
\t\t}

\t\tWidget widget = workspace.CreateWidgetInWorkspace(
\t\t\tWidgetType.TextWidgetTypeID,
\t\t\tleft,
\t\t\ttop,
\t\t\twidth,
\t\t\theight,
\t\t\tflags,
\t\t\tColor.White,
\t\t\t1000
\t\t);

\t\tm_DebugText = TextWidget.Cast(widget);
\t\tif (!m_DebugText)
\t\t{
\t\t\tPrint("[HOTAS Debugger] Could not create TextWidget", LogLevel.ERROR);
\t\t\treturn;
\t\t}

\t\tif (m_bDebugMode)
\t\t{
\t\t\tm_DebugText.SetExactFontSize(24);
\t\t\tm_DebugText.SetOutline(2, 0xFF000000);
\t\t\tm_DebugText.SetTextWrapping(true);
\t\t\tm_DebugText.SetText("HOTAS INPUT DEBUG\\nWaiting for a watched input action...");
\t\t}
\t\telse
\t\t{
\t\t\tm_DebugText.SetExactFontSize(Math.Round(26 * m_fHudScale));
\t\t\tm_DebugText.SetOutline(Math.Max(1, Math.Round(2 * m_fHudScale)), 0xF0000000);
\t\t\tm_DebugText.SetShadow(Math.Max(1, Math.Round(2 * m_fHudScale)), 0xC0000000, 1.0, 2, 2);
\t\t\tm_DebugText.SetTextWrapping(false);
\t\t\tm_DebugText.SetText("");
\t\t\tm_DebugText.SetOpacity(0.0);
\t\t\tif (m_HudBackground)
\t\t\t\tm_HudBackground.SetOpacity(0.0);
\t\t}

\t\tm_DebugText.SetBold(true);
\t}

\tprotected void GetHudPosition(WorkspaceWidget workspace, int width, int height, out int left, out int top)
\t{
\t\tint marginX = Math.Round(48 * m_fHudScale);
\t\tint marginY = Math.Round(54 * m_fHudScale);
\t\tint screenWidth = workspace.GetWidth();
\t\tint screenHeight = workspace.GetHeight();

\t\tleft = (screenWidth - width) / 2;
\t\ttop = screenHeight - height - marginY;

\t\tif (m_sHudPosition == "top_left")
\t\t{
\t\t\tleft = marginX;
\t\t\ttop = marginY;
\t\t}
\t\telse if (m_sHudPosition == "top_center")
\t\t{
\t\t\tleft = (screenWidth - width) / 2;
\t\t\ttop = marginY;
\t\t}
\t\telse if (m_sHudPosition == "top_right")
\t\t{
\t\t\tleft = screenWidth - width - marginX;
\t\t\ttop = marginY;
\t\t}
\t\telse if (m_sHudPosition == "center_left")
\t\t{
\t\t\tleft = marginX;
\t\t\ttop = (screenHeight - height) / 2;
\t\t}
\t\telse if (m_sHudPosition == "center")
\t\t{
\t\t\tleft = (screenWidth - width) / 2;
\t\t\ttop = (screenHeight - height) / 2;
\t\t}
\t\telse if (m_sHudPosition == "center_right")
\t\t{
\t\t\tleft = screenWidth - width - marginX;
\t\t\ttop = (screenHeight - height) / 2;
\t\t}
\t\telse if (m_sHudPosition == "bottom_left")
\t\t{
\t\t\tleft = marginX;
\t\t\ttop = screenHeight - height - marginY;
\t\t}
\t\telse if (m_sHudPosition == "bottom_right")
\t\t{
\t\t\tleft = screenWidth - width - marginX;
\t\t\ttop = screenHeight - height - marginY;
\t\t}
\t}

\tprotected void LoadHudSettings()
\t{
\t\tstring settingsPath = "$profile:HOTASHudSettings.txt";
\t\tif (!FileIO.FileExists(settingsPath))
\t\t{
\t\t\tFileHandle defaults = FileIO.OpenFile(settingsPath, FileMode.WRITE);
\t\t\tif (defaults)
\t\t\t{
\t\t\t\tdefaults.WriteLine("# HOTAS Input HUD settings");
\t\t\t\tdefaults.WriteLine("# position: top_left, top_center, top_right, center_left, center, center_right, bottom_left, bottom_center, bottom_right");
\t\t\t\tdefaults.WriteLine("position=bottom_center");
\t\t\t\tdefaults.WriteLine("scale=1.0");
\t\t\t\tdefaults.WriteLine("fade_delay_ms=1800");
\t\t\t\tdefaults.WriteLine("fade_duration_ms=350");
\t\t\t\tdefaults.WriteLine("background=1");
\t\t\t\tdefaults.WriteLine("background_opacity=0.55");
\t\t\t\tdefaults.Close();
\t\t\t}
\t\t}

\t\tFileHandle file = FileIO.OpenFile(settingsPath, FileMode.READ);
\t\tif (!file)
\t\t\treturn;

\t\tstring line;
\t\twhile (file.ReadLine(line) >= 0)
\t\t{
\t\t\tline = line.Trim();
\t\t\tif (line.IsEmpty() || line.StartsWith("#"))
\t\t\t\tcontinue;

\t\t\tref array<string> parts = {};
\t\t\tline.Split("=", parts, false);
\t\t\tif (parts.Count() < 2)
\t\t\t\tcontinue;

\t\t\tstring key = parts[0].Trim();
\t\t\tstring value = parts[1].Trim();
\t\t\tif (key == "position")
\t\t\t\tm_sHudPosition = value;
\t\t\telse if (key == "scale")
\t\t\t\tm_fHudScale = Math.Clamp(value.ToFloat(1.0), 0.5, 2.0);
\t\t\telse if (key == "fade_delay_ms")
\t\t\t\tm_iFadeDelayMs = Math.ClampInt(value.ToInt(1800), 0, 10000);
\t\t\telse if (key == "fade_duration_ms")
\t\t\t\tm_iFadeDurationMs = Math.ClampInt(value.ToInt(350), 0, 5000);
\t\t\telse if (key == "background")
\t\t\t\tm_bBackgroundEnabled = value.ToInt(1) != 0;
\t\t\telse if (key == "background_opacity")
\t\t\t\tm_fBackgroundOpacity = Math.Clamp(value.ToFloat(0.55), 0.0, 1.0);
\t\t}
\t\tfile.Close();

\t\tPrint(string.Format("[HOTAS Debugger] HUD settings: position=%1 scale=%2 fade=%3/%4 background=%5 opacity=%6", m_sHudPosition, m_fHudScale, m_iFadeDelayMs, m_iFadeDurationMs, m_bBackgroundEnabled, m_fBackgroundOpacity), LogLevel.NORMAL);
\t}

\tprotected void ShowHud()
\t{
\t\tif (m_bDebugMode || !m_DebugText)
\t\t\treturn;

\t\tScriptCallQueue queue = GetGame().GetCallqueue();
\t\tqueue.Remove(StartFade);
\t\tqueue.Remove(FadeStep);
\t\tm_fFadeOpacity = 1.0;
\t\tm_DebugText.SetOpacity(1.0);
\t\tif (m_HudBackground)
\t\t\tm_HudBackground.SetOpacity(m_fBackgroundOpacity);
\t\tqueue.CallLater(StartFade, m_iFadeDelayMs, false);
\t}

\tprotected void StartFade()
\t{
\t\tif (m_bDebugMode || !m_DebugText)
\t\t\treturn;

\t\tif (m_iFadeDurationMs <= 0)
\t\t{
\t\t\tm_DebugText.SetOpacity(0.0);
\t\t\tif (m_HudBackground)
\t\t\t\tm_HudBackground.SetOpacity(0.0);
\t\t\treturn;
\t\t}

\t\tm_fFadeOpacity = 1.0;
\t\tGetGame().GetCallqueue().CallLater(FadeStep, 50, true);
\t}

\tprotected void FadeStep()
\t{
\t\tif (!m_DebugText)
\t\t{
\t\t\tGetGame().GetCallqueue().Remove(FadeStep);
\t\t\treturn;
\t\t}

\t\tm_fFadeOpacity -= 50.0 / m_iFadeDurationMs;
\t\tif (m_fFadeOpacity <= 0.0)
\t\t{
\t\t\tm_fFadeOpacity = 0.0;
\t\t\tGetGame().GetCallqueue().Remove(FadeStep);
\t\t}

\t\tm_DebugText.SetOpacity(m_fFadeOpacity);
\t\tif (m_HudBackground)
\t\t\tm_HudBackground.SetOpacity(m_fBackgroundOpacity * m_fFadeOpacity);
\t}
'''
src = src[:start] + new_create + src[end:]

# Use human-readable binding/action text in normal HUD and trigger fade timing.
src = src.replace(
'''\t\telse\n\t\t{\n\t\t\toutput = string.Format("%1   |   %2", bindingsText, readableAction);\n\t\t}\n\n\t\tif (m_DebugText)\n\t\t\tm_DebugText.SetText(output);''',
'''\t\telse\n\t\t{\n\t\t\toutput = string.Format("%1   •   %2", MakeReadableBinding(bindingsText), readableAction);\n\t\t}\n\n\t\tif (m_DebugText)\n\t\t{\n\t\t\tm_DebugText.SetText(output);\n\t\t\tShowHud();\n\t\t}''',
1,
)

start = src.index('\tprotected string MakeReadableActionName(string actionName)')
end = src.index('\n\tprotected void BuildActionList()', start)
new_helpers = r'''\tprotected string MakeReadableBinding(string bindingsText)
\t{
\t\tref array<string> bindings = {};
\t\tbindingsText.Split(" / ", bindings, true);
\t\tstring result;

\t\tforeach (string binding : bindings)
\t\t{
\t\t\tstring readable = binding;
\t\t\tint buttonPos = binding.IndexOf(":button");
\t\t\tint axisPos = binding.IndexOf(":axis");
\t\t\tif (buttonPos >= 0)
\t\t\t{
\t\t\t\tint number = binding.Substring(buttonPos + 7, binding.Length() - buttonPos - 7).ToInt() + 1;
\t\t\t\treadable = string.Format("BUTTON %1", number);
\t\t\t}
\t\t\telse if (axisPos >= 0)
\t\t\t{
\t\t\t\tstring axisText = binding.Substring(axisPos + 5, binding.Length() - axisPos - 5);
\t\t\t\tstring direction;
\t\t\t\tif (axisText.EndsWith("+"))
\t\t\t\t\tdirection = "+";
\t\t\t\telse if (axisText.EndsWith("-"))
\t\t\t\t\tdirection = "-";
\t\t\t\tint axisNumber = axisText.ToInt() + 1;
\t\t\t\treadable = string.Format("AXIS %1%2", axisNumber, direction);
\t\t\t}

\t\t\tif (!result.IsEmpty())
\t\t\t\tresult += " / ";
\t\t\tresult += readable;
\t\t}

\t\treturn result;
\t}

\tprotected string MakeReadableActionName(string actionName)
\t{
\t\tswitch (actionName)
\t\t{
\t\t\tcase "CharacterNextWeapon": return "Next Weapon";
\t\t\tcase "TurretNextWeapon": return "Next Weapon";
\t\t\tcase "TurretWeaponNextRippleQuantity": return "Missile Ripple";
\t\t\tcase "TurretWeaponNextFireMode": return "Next Fire Mode";
\t\t\tcase "TurretReload": return "Reload";
\t\t\tcase "TurretFire": return "Fire";
\t\t\tcase "HelicopterCyclicForward": return "Cyclic Forward";
\t\t\tcase "HelicopterCyclicBack": return "Cyclic Back";
\t\t\tcase "HelicopterCyclicLeft": return "Cyclic Left";
\t\t\tcase "HelicopterCyclicRight": return "Cyclic Right";
\t\t\tcase "HelicopterAntiTorqueLeft": return "Pedal Left";
\t\t\tcase "HelicopterAntiTorqueRight": return "Pedal Right";
\t\t\tcase "HelicopterCollectiveIncrease": return "Collective Up";
\t\t\tcase "HelicopterCollectiveDecrease": return "Collective Down";
\t\t\tcase "HelicopterWheelBrake": return "Wheel Brake";
\t\t\tcase "HelicopterWheelBrakePersistent": return "Parking Brake";
\t\t\tcase "HelicopterAutohoverToggle": return "Auto Hover";
\t\t\tcase "HelicopterLightsTaxiToggle": return "Taxi Lights";
\t\t\tcase "HelicopterLightsLandingToggle": return "Landing Lights";
\t\t\tcase "HelicopterEngineStart": return "Engine Start";
\t\t\tcase "HelicopterEngineStop": return "Engine Stop";
\t\t\tcase "HelicopterFire": return "Fire";
\t\t\tcase "HelicopterSightDeploy": return "Deploy Sight";
\t\t\tcase "HelicopterSightZeroing": return "Sight Zeroing";
\t\t\tcase "VehicleDoorToggle": return "Toggle Door";
\t\t\tcase "PerformAction": return "Use / Confirm";
\t\t\tcase "SelectAction": return "Select Action";
\t\t\tcase "GadgetMap": return "Map";
\t\t\tcase "Freelook": return "Freelook";
\t\t\tcase "FreelookReset": return "Center View";
\t\t\tcase "FreelookUp": return "Look Up";
\t\t\tcase "FreelookDown": return "Look Down";
\t\t\tcase "FreelookLeft": return "Look Left";
\t\t\tcase "FreelookRight": return "Look Right";
\t\t\tcase "FocusToggle": return "Focus";
\t\t\tcase "VONChannel": return "Voice Channel";
\t\t\tcase "VONDirectToggle": return "Direct Voice";
\t\t\tcase "PFC_Pitch": return "Pitch";
\t\t\tcase "PFC_Roll": return "Roll";
\t\t\tcase "PFC_Yaw": return "Yaw";
\t\t\tcase "PFC_ThrottleAxis": return "Throttle";
\t\t\tcase "PFC_GearToggle": return "Landing Gear";
\t\t\tcase "PFC_Flaps": return "Flaps";
\t\t\tcase "PFC_Airbrake": return "Airbrake";
\t\t}

\t\tstring readable = actionName;
\t\treadable.Replace("WCS_Armament_", "WCS ");
\t\treadable.Replace("PFC_", "PFC ");
\t\treadable.Replace("_", " ");
\t\treturn readable;
\t}
'''
src = src[:start] + new_helpers + src[end:]

path.write_text(src, encoding='utf-8')
