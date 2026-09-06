class HOTASDebugController
{
	protected static ref HOTASDebugController s_Instance;

	protected InputManager m_InputManager;
	protected ref InputBinding m_InputBinding;
	protected TextWidget m_DebugText;
	protected Widget m_HudBackground;
	protected ref array<string> m_WatchedActions = {};
	protected bool m_bInitialized;
	protected bool m_bDebugMode = false;
	protected int m_iEventCounter;

	// Normal HUD user settings. Values are loaded from $profile:HOTASHudSettings.txt.
	protected string m_sHudPosition = "bottom_center";
	protected float m_fHudScale = 1.0;
	protected int m_iFadeDelayMs = 1800;
	protected int m_iFadeDurationMs = 350;
	protected bool m_bBackgroundEnabled = true;
	protected float m_fBackgroundOpacity = 0.55;
	protected float m_fFadeOpacity = 1.0;

	static HOTASDebugController GetInstance()
	{
		if (!s_Instance)
			s_Instance = new HOTASDebugController();

		return s_Instance;
	}

	void Initialize()
	{
		if (m_bInitialized)
		{
			Print("[HOTAS Debugger] Reinitializing for new play session", LogLevel.NORMAL);
			Shutdown();
		}

		m_InputManager = GetGame().GetInputManager();
		if (!m_InputManager)
		{
			Print("[HOTAS Debugger] InputManager is not available", LogLevel.ERROR);
			return;
		}

		m_InputBinding = m_InputManager.CreateUserBinding();

#ifdef WORKBENCH
		LoadWorkbenchTestConfig();
#endif

		BuildActionList();
		LoadHudSettings();
		CreateHud();
		RegisterListeners();

		m_bInitialized = true;
		Print("[HOTAS Debugger] Initialized", LogLevel.NORMAL);
	}

	void Shutdown()
	{
		if (!m_bInitialized)
			return;

		if (m_InputManager)
		{
			foreach (string actionName : m_WatchedActions)
				m_InputManager.RemoveActionListener(actionName, EActionTrigger.DOWN, OnActionTriggered);
		}

		GetGame().GetCallqueue().Remove(StartFade);
		GetGame().GetCallqueue().Remove(FadeStep);

		if (m_DebugText)
			m_DebugText.RemoveFromHierarchy();
		if (m_HudBackground)
			m_HudBackground.RemoveFromHierarchy();

		m_DebugText = null;
		m_HudBackground = null;
		m_InputBinding = null;
		m_InputManager = null;
		m_bInitialized = false;
	}

#ifdef WORKBENCH
	protected void LoadWorkbenchTestConfig()
	{
		if (!m_InputBinding)
			return;

		ref array<ResourceName> customConfigs = {};
		ResourceName testConfig = "$profile:.save/settings/customInputConfigs/Solr1 v5.4.conf";
		customConfigs.Insert(testConfig);
		m_InputBinding.SetCustomConfigs(customConfigs);
		m_InputBinding.Save();

		ref array<ResourceName> activeConfigs = {};
		m_InputBinding.GetCustomConfigs(activeConfigs);
		Print(string.Format("[HOTAS Debugger] Workbench test config requested: %1 | active custom configs=%2", testConfig, activeConfigs.Count()), LogLevel.NORMAL);
	}
#endif

	protected void RegisterListeners()
	{
		foreach (string actionName : m_WatchedActions)
			m_InputManager.AddActionListener(actionName, EActionTrigger.DOWN, OnActionTriggered);
	}

\tprotected void CreateHud()
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

	protected void OnActionTriggered(float value = 0.0, EActionTrigger reason = 0, string actionName = string.Empty)
	{
		if (actionName.IsEmpty())
			return;

		m_iEventCounter++;

		string bindingsText = GetJoystickBindings(actionName);
		string readableAction = MakeReadableActionName(actionName);
		string output;
		if (m_bDebugMode)
		{
			output = string.Format(
				"HOTAS INPUT DEBUG  #%1\nInput: %2\nAction: %3\nRaw action: %4\nValue: %5",
				m_iEventCounter,
				bindingsText,
				readableAction,
				actionName,
				value.ToString(2)
			);
		}
		else
		{
			output = string.Format("%1   •   %2", MakeReadableBinding(bindingsText), readableAction);
		}

		if (m_DebugText)
		{
			m_DebugText.SetText(output);
			ShowHud();
		}

		Print(string.Format("[HOTAS Debugger] %1 | %2 | value=%3", actionName, bindingsText, value), LogLevel.NORMAL);
	}

	protected string GetJoystickBindings(string actionName)
	{
		if (!m_InputManager)
			return "InputManager unavailable";

		string joystickBindings;

		// Query the active runtime ActionManager first. The separate InputBinding object can
		// report zero bindings even while a custom joystick config is actively driving actions.
		for (int bindIndex = 0; bindIndex < 16; bindIndex++)
		{
			ref array<string> keyStack = {};
			ref array<BaseContainer> filterStack = {};
			bool found = m_InputManager.GetActionKeybinding(
				actionName,
				keyStack,
				filterStack,
				EInputDeviceType.JOYSTICK,
				string.Empty,
				bindIndex
			);

			if (!found)
				break;

			foreach (string binding : keyStack)
			{
				if (!joystickBindings.IsEmpty())
					joystickBindings += " / ";

				joystickBindings += binding;
			}
		}

		if (!joystickBindings.IsEmpty())
			return joystickBindings;

		// Fallback for actions where the runtime manager does not expose an indexed binding.
		ref array<string> keyStackFallback = {};
		ref array<BaseContainer> filterStackFallback = {};
		if (m_InputManager.GetActionKeybinding(actionName, keyStackFallback, filterStackFallback, EInputDeviceType.JOYSTICK, string.Empty, -1))
		{
			foreach (string binding : keyStackFallback)
			{
				if (!joystickBindings.IsEmpty())
					joystickBindings += " / ";

				joystickBindings += binding;
			}
		}

		if (!joystickBindings.IsEmpty())
			return joystickBindings;

		// Final compatibility fallback to InputBinding.
		if (m_InputBinding)
		{
			ref array<string> bindings = {};
			if (m_InputBinding.GetBindings(actionName, bindings, EInputDeviceType.JOYSTICK, string.Empty, false))
			{
				foreach (string binding : bindings)
				{
					if (!joystickBindings.IsEmpty())
						joystickBindings += " / ";

					joystickBindings += binding;
				}
			}
		}

		if (joystickBindings.IsEmpty())
			return "Action fired, but active joystick binding lookup returned nothing";

		return joystickBindings;
	}

\tprotected string MakeReadableBinding(string bindingsText)
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

	protected void BuildActionList()
	{
		m_WatchedActions.Clear();

		// Helicopter flight and systems
		m_WatchedActions.Insert("HelicopterCollectiveIncrease");
		m_WatchedActions.Insert("HelicopterCollectiveDecrease");
		m_WatchedActions.Insert("HelicopterAntiTorqueLeft");
		m_WatchedActions.Insert("HelicopterAntiTorqueRight");
		m_WatchedActions.Insert("HelicopterCyclicForward");
		m_WatchedActions.Insert("HelicopterCyclicBack");
		m_WatchedActions.Insert("HelicopterCyclicLeft");
		m_WatchedActions.Insert("HelicopterCyclicRight");
		m_WatchedActions.Insert("HelicopterWheelBrake");
		m_WatchedActions.Insert("HelicopterWheelBrakePersistent");
		m_WatchedActions.Insert("HelicopterAutohoverToggle");
		m_WatchedActions.Insert("HelicopterLightsTaxiToggle");
		m_WatchedActions.Insert("HelicopterLightsLandingToggle");
		m_WatchedActions.Insert("HelicopterEngineStart");
		m_WatchedActions.Insert("HelicopterEngineStop");
		m_WatchedActions.Insert("HelicopterFire");
		m_WatchedActions.Insert("HelicopterSightDeploy");
		m_WatchedActions.Insert("HelicopterSightZeroing");

		// Character / vehicle / turret actions used by the configurator
		m_WatchedActions.Insert("CharacterFire");
		m_WatchedActions.Insert("CharacterNextWeapon");
		m_WatchedActions.Insert("CharacterNextFireMode");
		m_WatchedActions.Insert("CharacterNextMuzzle");
		m_WatchedActions.Insert("VehicleFire");
		m_WatchedActions.Insert("VehicleNextWeapon");
		m_WatchedActions.Insert("VehicleDoorToggle");
		m_WatchedActions.Insert("TurretFire");
		m_WatchedActions.Insert("TurretReload");
		m_WatchedActions.Insert("TurretNextWeapon");
		m_WatchedActions.Insert("TurretWeaponNextFireMode");
		m_WatchedActions.Insert("TurretADS");
		m_WatchedActions.Insert("TurretADSHold");
		m_WatchedActions.Insert("TurretRotateLeft");
		m_WatchedActions.Insert("TurretRotateRight");
		m_WatchedActions.Insert("TurretAimUp");
		m_WatchedActions.Insert("TurretAimDown");
		m_WatchedActions.Insert("TurretAimLeft");
		m_WatchedActions.Insert("TurretAimRight");
		m_WatchedActions.Insert("WeaponToggleSightsIllumination");
		m_WatchedActions.Insert("WeaponSwitchOptics");
		m_WatchedActions.Insert("FocusToggle");
		m_WatchedActions.Insert("Freelook");
		m_WatchedActions.Insert("FreelookReset");
		m_WatchedActions.Insert("FreelookUp");
		m_WatchedActions.Insert("FreelookDown");
		m_WatchedActions.Insert("FreelookLeft");
		m_WatchedActions.Insert("FreelookRight");
		m_WatchedActions.Insert("VONDirectToggle");
		m_WatchedActions.Insert("VONChannel");
		m_WatchedActions.Insert("GadgetMap");
		m_WatchedActions.Insert("PerformAction");
		m_WatchedActions.Insert("SelectAction");
		m_WatchedActions.Insert("GetOut");
		m_WatchedActions.Insert("JumpOut");

		// WCS Armament
		m_WatchedActions.Insert("WCS_Armament_CycleWeapon");
		m_WatchedActions.Insert("WCS_Armament_DeployFlares");
		m_WatchedActions.Insert("WCS_Armament_DeployChaffs");
		m_WatchedActions.Insert("WCS_Armament_TurretStabilizationToggle");
		m_WatchedActions.Insert("WCS_Armament_VehicleAim");
		m_WatchedActions.Insert("WCS_Armament_CycleWeaponFireMode");
		m_WatchedActions.Insert("WCS_Armament_ActivateLock");
		m_WatchedActions.Insert("WCS_Armament_DeploySmoke");
		m_WatchedActions.Insert("WCS_Armament_RadarToggle");
		m_WatchedActions.Insert("WCS_Armament_FireContinuousSmokeDispenser");
		m_WatchedActions.Insert("TurretWeaponNextRippleQuantity");
		m_WatchedActions.Insert("WCS_Armament_ConfirmLock");

		// Fixed-wing / Propeller Flight Core
		m_WatchedActions.Insert("PFC_Pitch");
		m_WatchedActions.Insert("PFC_Roll");
		m_WatchedActions.Insert("PFC_Yaw");
		m_WatchedActions.Insert("PFC_ThrottleAxis");
		m_WatchedActions.Insert("PFC_ThrottleUp");
		m_WatchedActions.Insert("PFC_ThrottleDown");
		m_WatchedActions.Insert("PFC_GearToggle");
		m_WatchedActions.Insert("PFC_Flaps");
		m_WatchedActions.Insert("PFC_Airbrake");
		m_WatchedActions.Insert("PFC_WheelBrake");
		m_WatchedActions.Insert("PFC_WheelBrakePersistent");
		m_WatchedActions.Insert("PFC_EngineStart");
		m_WatchedActions.Insert("PFC_EngineStop");
		m_WatchedActions.Insert("PFC_TrimUp");
		m_WatchedActions.Insert("PFC_TrimDown");
		m_WatchedActions.Insert("PFC_TrimReset");
	}
}

modded class SCR_BaseGameMode
{
	override void OnGameModeStart()
	{
		super.OnGameModeStart();

		// The debugger has no authority-side gameplay logic. Initialize the local HUD/input listeners
		// on every machine where this addon is loaded; dedicated servers have no workspace and simply
		// fail the HUD creation harmlessly.
		GetGame().GetCallqueue().CallLater(HOTASDebugController.GetInstance().Initialize, 1000, false);
	}

	override void OnGameModeEnd(SCR_GameModeEndData endData)
	{
		HOTASDebugController.GetInstance().Shutdown();
		super.OnGameModeEnd(endData);
	}
}
