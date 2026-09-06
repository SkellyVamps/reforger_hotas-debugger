class HOTASDebugController
{
	protected static ref HOTASDebugController s_Instance;

	protected InputManager m_InputManager;
	protected ref InputBinding m_InputBinding;
	protected TextWidget m_DebugText;
	protected ref array<string> m_WatchedActions = {};
	protected bool m_bInitialized;
	protected int m_iEventCounter;

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

		if (m_DebugText)
			m_DebugText.RemoveFromHierarchy();

		m_DebugText = null;
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
		ResourceName testConfig = "$profile:.save/settings/customInputConfigs/Solr1 v5.3.conf";
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

		Widget widget = workspace.CreateWidgetInWorkspace(
			WidgetType.TextWidgetTypeID,
			40,
			120,
			900,
			180,
			WidgetFlags.VISIBLE | WidgetFlags.IGNORE_CURSOR | WidgetFlags.NOFOCUS | WidgetFlags.WRAP_TEXT,
			Color.White,
			1000
		);

		m_DebugText = TextWidget.Cast(widget);
		if (!m_DebugText)
		{
			Print("[HOTAS Debugger] Could not create TextWidget", LogLevel.ERROR);
			return;
		}

		m_DebugText.SetExactFontSize(24);
		m_DebugText.SetBold(true);
		m_DebugText.SetOutline(2, 0xFF000000);
		m_DebugText.SetTextWrapping(true);
		m_DebugText.SetText("HOTAS INPUT DEBUG\nWaiting for a watched input action...");
	}

	protected void OnActionTriggered(float value = 0.0, EActionTrigger reason = 0, string actionName = string.Empty)
	{
		if (actionName.IsEmpty())
			return;

		m_iEventCounter++;

		string bindingsText = GetJoystickBindings(actionName);
		string readableAction = MakeReadableActionName(actionName);
		string output = string.Format(
			"HOTAS INPUT DEBUG  #%1\nInput: %2\nAction: %3\nRaw action: %4\nValue: %5",
			m_iEventCounter,
			bindingsText,
			readableAction,
			actionName,
			value.ToString(2)
		);

		if (m_DebugText)
			m_DebugText.SetText(output);

		Print(string.Format("[HOTAS Debugger] %1 | %2 | value=%3", actionName, bindingsText, value), LogLevel.NORMAL);
	}

	protected string GetJoystickBindings(string actionName)
	{
		if (!m_InputBinding)
			return "Binding API unavailable";

		ref array<string> bindings = {};
		bool found = m_InputBinding.GetBindings(actionName, bindings, EInputDeviceType.JOYSTICK, string.Empty, false);
		if (!found || bindings.IsEmpty())
			return "No binding reported";

		string joystickBindings;
		foreach (string binding : bindings)
		{
			if (binding.IndexOf("joystick") != 0)
				continue;

			if (!joystickBindings.IsEmpty())
				joystickBindings += " / ";

			joystickBindings += binding;
		}

		if (joystickBindings.IsEmpty())
			return "Action fired, but no joystick binding was returned";

		return joystickBindings;
	}

	protected string MakeReadableActionName(string actionName)
	{
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
