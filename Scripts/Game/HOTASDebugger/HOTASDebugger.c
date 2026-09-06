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

	// Raw joystick axis numbers used by this HOTAS. Users can remap these in HOTASHudSettings.txt.
	protected int m_iRollAxis = 0;
	protected int m_iPitchAxis = 1;
	protected int m_iThrottleAxis = 2;
	protected int m_iYawAxis = 5;

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

	protected void CreateHud()
	{
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace)
		{
			Print("[HOTAS Debugger] Workspace is not available", LogLevel.ERROR);
			return;
		}

		int left;
		int top;
		int width;
		int height;
		int flags = WidgetFlags.VISIBLE | WidgetFlags.IGNORE_CURSOR | WidgetFlags.NOFOCUS | WidgetFlags.NO_LOCALIZATION;

		if (m_bDebugMode)
		{
			left = 40;
			top = 120;
			width = 900;
			height = 180;
			flags |= WidgetFlags.WRAP_TEXT;
		}
		else
		{
			// Leave extra horizontal room so longer readable action labels are not clipped.
			width = Math.Round(1040 * m_fHudScale);
			height = Math.Round(72 * m_fHudScale);
			GetHudPosition(workspace, width, height, left, top);
			flags |= WidgetFlags.CENTER | WidgetFlags.VCENTER;

			if (m_bBackgroundEnabled)
			{
				m_HudBackground = workspace.CreateWidgetInWorkspace(
					WidgetType.PanelWidgetTypeID,
					left,
					top,
					width,
					height,
					WidgetFlags.VISIBLE | WidgetFlags.IGNORE_CURSOR | WidgetFlags.NOFOCUS,
					Color.FromInt(0xFF101418),
					999
				);
				if (m_HudBackground)
					m_HudBackground.SetOpacity(m_fBackgroundOpacity);
			}
		}

		Widget widget = workspace.CreateWidgetInWorkspace(
			WidgetType.TextWidgetTypeID,
			left,
			top,
			width,
			height,
			flags,
			Color.White,
			1000
		);

		m_DebugText = TextWidget.Cast(widget);
		if (!m_DebugText)
		{
			Print("[HOTAS Debugger] Could not create TextWidget", LogLevel.ERROR);
			return;
		}

		if (m_bDebugMode)
		{
			m_DebugText.SetExactFontSize(24);
			m_DebugText.SetOutline(2, 0xFF000000);
			m_DebugText.SetTextWrapping(true);
			m_DebugText.SetText("HOTAS INPUT DEBUG\nWaiting for a watched input action...");
		}
		else
		{
			m_DebugText.SetExactFontSize(Math.Round(26 * m_fHudScale));
			m_DebugText.SetOutline(Math.Max(1, Math.Round(2 * m_fHudScale)), 0xF0000000);
			m_DebugText.SetShadow(Math.Max(1, Math.Round(2 * m_fHudScale)), 0xC0000000, 1.0, 2, 2);
			m_DebugText.SetTextWrapping(false);
			m_DebugText.SetText("");
			m_DebugText.SetOpacity(0.0);
			if (m_HudBackground)
				m_HudBackground.SetOpacity(0.0);
		}

		m_DebugText.SetBold(true);
	}

	protected void GetHudPosition(WorkspaceWidget workspace, int width, int height, out int left, out int top)
	{
		int marginX = Math.Round(48 * m_fHudScale);
		int marginY = Math.Round(54 * m_fHudScale);
		int screenWidth = workspace.GetWidth();
		int screenHeight = workspace.GetHeight();

		left = (screenWidth - width) / 2;
		top = screenHeight - height - marginY;

		if (m_sHudPosition == "top_left")
		{
			left = marginX;
			top = marginY;
		}
		else if (m_sHudPosition == "top_center")
		{
			left = (screenWidth - width) / 2;
			top = marginY;
		}
		else if (m_sHudPosition == "top_right")
		{
			left = screenWidth - width - marginX;
			top = marginY;
		}
		else if (m_sHudPosition == "center_left")
		{
			left = marginX;
			top = (screenHeight - height) / 2;
		}
		else if (m_sHudPosition == "center")
		{
			left = (screenWidth - width) / 2;
			top = (screenHeight - height) / 2;
		}
		else if (m_sHudPosition == "center_right")
		{
			left = screenWidth - width - marginX;
			top = (screenHeight - height) / 2;
		}
		else if (m_sHudPosition == "bottom_left")
		{
			left = marginX;
			top = screenHeight - height - marginY;
		}
		else if (m_sHudPosition == "bottom_right")
		{
			left = screenWidth - width - marginX;
			top = screenHeight - height - marginY;
		}
	}

	protected void LoadHudSettings()
	{
		string settingsPath = "$profile:HOTASHudSettings.txt";
		if (!FileIO.FileExists(settingsPath))
		{
			FileHandle defaults = FileIO.OpenFile(settingsPath, FileMode.WRITE);
			if (defaults)
			{
				defaults.WriteLine("# HOTAS Input HUD settings");
				defaults.WriteLine("# position: top_left, top_center, top_right, center_left, center, center_right, bottom_left, bottom_center, bottom_right");
				defaults.WriteLine("position=bottom_center");
				defaults.WriteLine("scale=1.0");
				defaults.WriteLine("fade_delay_ms=1800");
				defaults.WriteLine("fade_duration_ms=350");
				defaults.WriteLine("background=1");
				defaults.WriteLine("background_opacity=0.55");
				defaults.WriteLine("# Raw joystick axis mapping. Set an unused control to -1.");
				defaults.WriteLine("roll_axis=0");
				defaults.WriteLine("pitch_axis=1");
				defaults.WriteLine("throttle_axis=2");
				defaults.WriteLine("yaw_axis=5");
				defaults.Close();
			}
		}

		FileHandle file = FileIO.OpenFile(settingsPath, FileMode.READ);
		if (!file)
			return;

		string line;
		while (file.ReadLine(line) >= 0)
		{
			line = line.Trim();
			if (line.IsEmpty() || line.StartsWith("#"))
				continue;

			ref array<string> parts = {};
			line.Split("=", parts, false);
			if (parts.Count() < 2)
				continue;

			string key = parts[0].Trim();
			string value = parts[1].Trim();
			if (key == "position")
				m_sHudPosition = value;
			else if (key == "scale")
				m_fHudScale = Math.Clamp(value.ToFloat(1.0), 0.5, 2.0);
			else if (key == "fade_delay_ms")
				m_iFadeDelayMs = Math.ClampInt(value.ToInt(1800), 0, 10000);
			else if (key == "fade_duration_ms")
				m_iFadeDurationMs = Math.ClampInt(value.ToInt(350), 0, 5000);
			else if (key == "background")
				m_bBackgroundEnabled = value.ToInt(1) != 0;
			else if (key == "background_opacity")
				m_fBackgroundOpacity = Math.Clamp(value.ToFloat(0.55), 0.0, 1.0);
			else if (key == "roll_axis")
				m_iRollAxis = Math.ClampInt(value.ToInt(0), -1, 63);
			else if (key == "pitch_axis")
				m_iPitchAxis = Math.ClampInt(value.ToInt(1), -1, 63);
			else if (key == "throttle_axis")
				m_iThrottleAxis = Math.ClampInt(value.ToInt(2), -1, 63);
			else if (key == "yaw_axis")
				m_iYawAxis = Math.ClampInt(value.ToInt(5), -1, 63);
		}
		file.Close();

		Print(string.Format("[HOTAS Debugger] HUD settings: position=%1 scale=%2 fade=%3/%4 background=%5 opacity=%6", m_sHudPosition, m_fHudScale, m_iFadeDelayMs, m_iFadeDurationMs, m_bBackgroundEnabled, m_fBackgroundOpacity), LogLevel.NORMAL);
		Print(string.Format("[HOTAS Debugger] Axis mapping: roll=%1 pitch=%2 throttle=%3 yaw=%4", m_iRollAxis, m_iPitchAxis, m_iThrottleAxis, m_iYawAxis), LogLevel.NORMAL);
	}

	protected void ShowHud()
	{
		if (m_bDebugMode || !m_DebugText)
			return;

		ScriptCallQueue queue = GetGame().GetCallqueue();
		queue.Remove(StartFade);
		queue.Remove(FadeStep);
		m_fFadeOpacity = 1.0;
		m_DebugText.SetOpacity(1.0);
		if (m_HudBackground)
			m_HudBackground.SetOpacity(m_fBackgroundOpacity);
		queue.CallLater(StartFade, m_iFadeDelayMs, false);
	}

	protected void StartFade()
	{
		if (m_bDebugMode || !m_DebugText)
			return;

		if (m_iFadeDurationMs <= 0)
		{
			m_DebugText.SetOpacity(0.0);
			if (m_HudBackground)
				m_HudBackground.SetOpacity(0.0);
			return;
		}

		m_fFadeOpacity = 1.0;
		GetGame().GetCallqueue().CallLater(FadeStep, 50, true);
	}

	protected void FadeStep()
	{
		if (!m_DebugText)
		{
			GetGame().GetCallqueue().Remove(FadeStep);
			return;
		}

		m_fFadeOpacity -= 50.0 / m_iFadeDurationMs;
		if (m_fFadeOpacity <= 0.0)
		{
			m_fFadeOpacity = 0.0;
			GetGame().GetCallqueue().Remove(FadeStep);
		}

		m_DebugText.SetOpacity(m_fFadeOpacity);
		if (m_HudBackground)
			m_HudBackground.SetOpacity(m_fBackgroundOpacity * m_fFadeOpacity);
	}

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

	protected string MakeReadableBinding(string bindingsText)
	{
		ref array<string> bindings = {};
		bindingsText.Split(" / ", bindings, true);
		string result;

		foreach (string binding : bindings)
		{
			string readable = binding;
			int buttonPos = binding.IndexOf(":button");
			int axisPos = binding.IndexOf(":axis");
			if (buttonPos >= 0)
			{
				int number = binding.Substring(buttonPos + 7, binding.Length() - buttonPos - 7).ToInt() + 1;
				readable = string.Format("BUTTON %1", number);
			}
			else if (axisPos >= 0)
			{
				string axisText = binding.Substring(axisPos + 5, binding.Length() - axisPos - 5);
				string direction;
				if (axisText.EndsWith("+"))
					direction = "+";
				else if (axisText.EndsWith("-"))
					direction = "-";

				int rawAxis = axisText.ToInt();
				string axisName;
				if (rawAxis == m_iRollAxis && m_iRollAxis >= 0)
					axisName = "ROLL";
				else if (rawAxis == m_iPitchAxis && m_iPitchAxis >= 0)
					axisName = "PITCH";
				else if (rawAxis == m_iThrottleAxis && m_iThrottleAxis >= 0)
					axisName = "THROTTLE";
				else if (rawAxis == m_iYawAxis && m_iYawAxis >= 0)
					axisName = "YAW";

				if (!axisName.IsEmpty())
					readable = string.Format("%1 %2", axisName, direction);
				else
					readable = string.Format("AXIS %1%2", rawAxis + 1, direction);
			}

			if (!result.IsEmpty())
				result += " / ";
			result += readable;
		}

		return result;
	}

	protected string MakeReadableActionName(string actionName)
	{
		switch (actionName)
		{
			case "CharacterNextWeapon": return "Next Weapon";
			case "TurretNextWeapon": return "Next Weapon";
			case "TurretWeaponNextRippleQuantity": return "Missile Ripple";
			case "TurretWeaponNextFireMode": return "Next Fire Mode";
			case "TurretReload": return "Reload";
			case "TurretFire": return "Fire";
			case "HelicopterCyclicForward": return "Cyclic Forward";
			case "HelicopterCyclicBack": return "Cyclic Back";
			case "HelicopterCyclicLeft": return "Cyclic Left";
			case "HelicopterCyclicRight": return "Cyclic Right";
			case "HelicopterAntiTorqueLeft": return "Pedal Left";
			case "HelicopterAntiTorqueRight": return "Pedal Right";
			case "HelicopterCollectiveIncrease": return "Collective Up";
			case "HelicopterCollectiveDecrease": return "Collective Down";
			case "HelicopterWheelBrake": return "Wheel Brake";
			case "HelicopterWheelBrakePersistent": return "Parking Brake";
			case "HelicopterAutohoverToggle": return "Auto Hover";
			case "HelicopterLightsTaxiToggle": return "Taxi Lights";
			case "HelicopterLightsLandingToggle": return "Landing Lights";
			case "HelicopterEngineStart": return "Engine Start";
			case "HelicopterEngineStop": return "Engine Stop";
			case "HelicopterFire": return "Fire";
			case "HelicopterSightDeploy": return "Deploy Sight";
			case "HelicopterSightZeroing": return "Sight Zeroing";
			case "VehicleDoorToggle": return "Toggle Door";
			case "PerformAction": return "Use / Confirm";
			case "SelectAction": return "Select Action";
			case "GadgetMap": return "Map";
			case "Freelook": return "Freelook";
			case "FreelookReset": return "Center View";
			case "FreelookUp": return "Look Up";
			case "FreelookDown": return "Look Down";
			case "FreelookLeft": return "Look Left";
			case "FreelookRight": return "Look Right";
			case "FocusToggle": return "Focus";
			case "VONChannel": return "Voice Channel";
			case "VONDirectToggle": return "Direct Voice";
			case "PFC_Pitch": return "Pitch";
			case "PFC_Roll": return "Roll";
			case "PFC_Yaw": return "Yaw";
			case "PFC_ThrottleAxis": return "Throttle";
			case "PFC_GearToggle": return "Landing Gear";
			case "PFC_Flaps": return "Flaps";
			case "PFC_Airbrake": return "Airbrake";
		}

		string readable = actionName;
		readable.Replace("WCS_Armament_", "WCS ");
		readable.Replace("PFC_", "PFC ");
		readable.Replace("_", " ");
		return readable;
	}

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
