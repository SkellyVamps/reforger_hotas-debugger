//------------------------------------------------------------------------------------------------
// In-game HOTAS binding editor.
//
// File ownership boundary:
// - This mod may create and overwrite only files it owns.
// - HOTASHudSettings.txt remains managed by the existing HUD/settings code.
// - This editor owns only HOTAS_Config.conf in customInputConfigs.
// - Every other customInputConfigs/*.conf file is read-only and may only be imported.
// - HOTAS_Config.conf is fully regenerated on every save so removed bindings cannot linger.
class HOTASBindingDefinition
{
	string m_sDisplayName;
	string m_sCaptureAction;
	string m_sConfigAction;
	string m_sFilterPreset;
	string m_sCategory;
	bool m_bFullAxis;

	void HOTASBindingDefinition(string displayName, string captureAction, string configAction, string filterPreset, string category, bool fullAxis = false)
	{
		m_sDisplayName = displayName;
		m_sCaptureAction = captureAction;
		m_sConfigAction = configAction;
		m_sFilterPreset = filterPreset;
		m_sCategory = category;
		m_bFullAxis = fullAxis;
	}
}

//------------------------------------------------------------------------------------------------
class HOTASBindingsSubMenu : SCR_SettingsSubMenuBase
{
	protected static const string MANAGED_CONFIG_PATH = "$profile:.save/settings/customInputConfigs/HOTAS_Config.conf";
	protected static const string MANAGED_CONFIG_NAME = "HOTAS_Config.conf";
	protected static const string MANAGED_SIGNATURE = "// HOTAS Debugger managed config v1";
	protected static const string CONFIG_DIRECTORY = "$profile:.save/settings/customInputConfigs";

	protected ref array<ref HOTASBindingDefinition> m_Definitions = {};
	protected ref array<string> m_Bindings = {};
	protected ref array<string> m_ImportConfigs = {};

	protected SCR_SpinBoxComponent m_ImportSelector;
	protected SCR_SpinBoxComponent m_ActionSelector;
	protected SCR_ButtonTextComponent m_ImportButton;
	protected SCR_ButtonTextComponent m_ActivateButton;
	protected SCR_ButtonTextComponent m_ResetButton;
	protected SCR_ButtonTextComponent m_BindButton;
	protected SCR_ButtonTextComponent m_ClearButton;
	protected SCR_ButtonTextComponent m_CancelCaptureButton;
	protected SCR_LabelComponent m_ManagedStatusLabel;
	protected SCR_LabelComponent m_CurrentBindingLabel;
	protected SCR_LabelComponent m_EditorStatusLabel;
	protected ref InputBinding m_CaptureBinding;
	protected int m_iSelectedAction;
	protected bool m_bCapturing;

	//------------------------------------------------------------------------------------------------
	override void OnTabCreate(Widget menuRoot, ResourceName buttonsLayout, int index)
	{
		super.OnTabCreate(menuRoot, buttonsLayout, index);

		HOTASDebugController.GetInstance().PrepareForSettings();
		BuildDefinitions();
		ResolveWidgets();
		EnsureManagedConfig();
		RefreshImportSelector();
		RefreshActionSelector();
		RefreshManagedStatus();
		RefreshCurrentBinding();
		SetEditorStatus("Select an action, then choose Bind Input.");
	}

	//------------------------------------------------------------------------------------------------
	override void OnTabShow()
	{
		super.OnTabShow();

		HOTASDebugController.GetInstance().PrepareForSettings();
		EnsureManagedConfig();
		RefreshImportSelector();
		RefreshManagedStatus();
		RefreshCurrentBinding();
	}

	//------------------------------------------------------------------------------------------------
	override void OnTabHide()
	{
		StopCapture(true);
		super.OnTabHide();
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_SpinBoxComponent FindSpinBox(string widgetName)
	{
		Widget widget = m_wRoot.FindAnyWidget(widgetName);
		if (!widget)
			return null;

		return SCR_SpinBoxComponent.Cast(widget.FindHandler(SCR_SpinBoxComponent));
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_LabelComponent FindLabel(string widgetName)
	{
		Widget widget = m_wRoot.FindAnyWidget(widgetName);
		if (!widget)
			return null;

		return SCR_LabelComponent.Cast(widget.FindHandler(SCR_LabelComponent));
	}

	//------------------------------------------------------------------------------------------------
	protected void ResolveWidgets()
	{
		m_ImportSelector = FindSpinBox("ImportConfig");
		m_ActionSelector = FindSpinBox("BindingAction");
		m_ManagedStatusLabel = FindLabel("ManagedConfigStatus");
		m_CurrentBindingLabel = FindLabel("CurrentBinding");
		m_EditorStatusLabel = FindLabel("EditorStatus");

		m_ImportButton = SCR_ButtonTextComponent.GetButtonText("ImportSelected", m_wRoot);
		m_ActivateButton = SCR_ButtonTextComponent.GetButtonText("ActivateManaged", m_wRoot);
		m_ResetButton = SCR_ButtonTextComponent.GetButtonText("ResetManaged", m_wRoot);
		m_BindButton = SCR_ButtonTextComponent.GetButtonText("BindInput", m_wRoot);
		m_ClearButton = SCR_ButtonTextComponent.GetButtonText("ClearBinding", m_wRoot);
		m_CancelCaptureButton = SCR_ButtonTextComponent.GetButtonText("CancelCapture", m_wRoot);

		if (m_ImportSelector)
			m_ImportSelector.m_OnChanged.Insert(OnImportSelectionChanged);
		if (m_ActionSelector)
			m_ActionSelector.m_OnChanged.Insert(OnActionSelectionChanged);
		if (m_ImportButton)
			m_ImportButton.m_OnClicked.Insert(OnImportSelected);
		if (m_ActivateButton)
			m_ActivateButton.m_OnClicked.Insert(OnActivateManaged);
		if (m_ResetButton)
			m_ResetButton.m_OnClicked.Insert(OnResetManaged);
		if (m_BindButton)
			m_BindButton.m_OnClicked.Insert(OnBindInput);
		if (m_ClearButton)
			m_ClearButton.m_OnClicked.Insert(OnClearBinding);
		if (m_CancelCaptureButton)
			m_CancelCaptureButton.m_OnClicked.Insert(OnCancelCapture);

		SetCaptureControls(false);
	}

	//------------------------------------------------------------------------------------------------
	protected void BuildDefinitions()
	{
		if (!m_Definitions.IsEmpty())
			return;

		// Helicopter flight
		AddDefinition("Collective Increase", "HelicopterCollectiveIncrease", "HelicopterCollectiveIncrease", "up", "Helicopter");
		AddDefinition("Collective Decrease", "HelicopterCollectiveDecrease", "HelicopterCollectiveDecrease", "down", "Helicopter");
		AddDefinition("Anti-Torque Left", "HelicopterAntiTorqueLeft", "HelicopterAntiTorqueLeft", "left", "Helicopter");
		AddDefinition("Anti-Torque Right", "HelicopterAntiTorqueRight", "HelicopterAntiTorqueRight", "right", "Helicopter");
		AddDefinition("Cyclic Forward", "HelicopterCyclicForward", "HelicopterCyclicForward", "forward", "Helicopter");
		AddDefinition("Cyclic Back", "HelicopterCyclicBack", "HelicopterCyclicBack", "back", "Helicopter");
		AddDefinition("Cyclic Left", "HelicopterCyclicLeft", "HelicopterCyclicLeft", "left", "Helicopter");
		AddDefinition("Cyclic Right", "HelicopterCyclicRight", "HelicopterCyclicRight", "right", "Helicopter");
		AddDefinition("Wheel Brake", "HelicopterWheelBrake", "HelicopterWheelBrake", "pressed", "Helicopter");
		AddDefinition("Parking Brake", "HelicopterWheelBrakePersistent", "HelicopterWheelBrakePersistent", "pressed", "Helicopter");
		AddDefinition("Auto Hover", "HelicopterAutohoverToggle", "HelicopterAutohoverToggle", "click", "Helicopter");
		AddDefinition("Taxi Lights", "HelicopterLightsTaxiToggle", "HelicopterLightsTaxiToggle", "toggle", "Helicopter");
		AddDefinition("Landing Lights", "HelicopterLightsLandingToggle", "HelicopterLightsLandingToggle", "toggle", "Helicopter");
		AddDefinition("Engine Start", "HelicopterEngineStart", "HelicopterEngineStart", "hold", "Helicopter");
		AddDefinition("Engine Stop", "HelicopterEngineStop", "HelicopterEngineStop", "click", "Helicopter");
		AddDefinition("Helicopter Fire", "HelicopterFire", "HelicopterFire", "hold", "Helicopter");
		AddDefinition("Sight Deploy", "HelicopterSightDeploy", "HelicopterSightDeploy", "click", "Helicopter");
		AddDefinition("Sight Zeroing Increase", "HelicopterSightZeroing", "HelicopterSightZeroing", "up", "Helicopter");
		AddDefinition("Sight Zeroing Decrease", "HelicopterSightZeroing", "HelicopterSightZeroing", "down", "Helicopter");

		// Character / weapons / vehicle
		AddDefinition("Fire", "CharacterFire", "CharacterFire", "hold", "Weapons");
		AddDefinition("Next Weapon", "CharacterNextWeapon", "CharacterNextWeapon", "click", "Weapons");
		AddDefinition("Next Fire Mode", "CharacterNextFireMode", "CharacterNextFireMode", "click", "Weapons");
		AddDefinition("Next Muzzle", "CharacterNextMuzzle", "CharacterNextMuzzle", "click", "Weapons");
		AddDefinition("Vehicle Fire", "VehicleFire", "VehicleFire", "hold", "Weapons");
		AddDefinition("Sight Illumination", "WeaponToggleSightsIllumination", "WeaponToggleSightsIllumination", "click", "Weapons");
		AddDefinition("Switch Optics", "WeaponSwitchOptics", "WeaponSwitchOptics", "click", "Weapons");
		AddDefinition("Focus Toggle", "FocusToggle", "FocusToggle", "click", "Weapons");

		// Turret
		AddDefinition("Turret Fire", "TurretFire", "TurretFire", "hold", "Turret");
		AddDefinition("Turret Reload", "TurretReload", "TurretReload", "click", "Turret");
		AddDefinition("Turret Next Weapon", "TurretNextWeapon", "TurretNextWeapon", "hold", "Turret");
		AddDefinition("Turret Next Fire Mode", "TurretNextFireMode", "TurretNextFireMode", "click", "Turret");
		AddDefinition("Turret Ripple Quantity", "TurretWeaponNextRippleQuantity", "TurretWeaponNextRippleQuantity", "click", "Turret");
		AddDefinition("Turret ADS Toggle", "TurretADS", "TurretADS", "click", "Turret");
		AddDefinition("Turret ADS Hold", "TurretADSHold", "TurretADSHold", "hold", "Turret");
		AddDefinition("Turret Rotate Left", "TurretRotateLeft", "TurretRotateLeft", "left", "Turret");
		AddDefinition("Turret Rotate Right", "TurretRotateRight", "TurretRotateRight", "right", "Turret");
		AddDefinition("Turret Aim Up", "TurretAimUp", "TurretAimUp", "up", "Turret");
		AddDefinition("Turret Aim Down", "TurretAimDown", "TurretAimDown", "down", "Turret");
		AddDefinition("Turret Aim Left", "TurretAimLeft", "TurretAimLeft", "left", "Turret");
		AddDefinition("Turret Aim Right", "TurretAimRight", "TurretAimRight", "right", "Turret");

		// Interaction
		AddDefinition("Perform Action", "PerformAction", "PerformAction", "pressed", "Interaction");
		AddDefinition("Previous Action", "SelectAction", "SelectAction", "previous", "Interaction");
		AddDefinition("Next Action", "SelectAction", "SelectAction", "next", "Interaction");
		AddDefinition("Get Out", "GetOut", "GetOut", "click", "Interaction");
		AddDefinition("Jump Out", "JumpOut", "JumpOut", "click", "Interaction");
		AddDefinition("Vehicle Door Toggle", "VehicleDoorToggle", "VehicleDoorToggle", "click", "Interaction");

		// View / communications
		AddDefinition("Free Look", "Freelook", "Freelook", "hold", "View");
		AddDefinition("Free Look Reset", "FreelookReset", "FreelookReset", "click", "View");
		AddDefinition("Free Look Up", "FreelookUp", "FreelookUp", "up", "View");
		AddDefinition("Free Look Down", "FreelookDown", "FreelookDown", "down", "View");
		AddDefinition("Free Look Left", "FreelookLeft", "FreelookLeft", "left", "View");
		AddDefinition("Free Look Right", "FreelookRight", "FreelookRight", "right", "View");
		AddDefinition("Direct Voice Toggle", "VONDirectToggle", "VONDirectToggle", "click", "Communications");
		AddDefinition("Voice Channel", "VONChannel", "VONChannel", "hold", "Communications");
		AddDefinition("Map", "GadgetMap", "GadgetMap", "select", "General");

		// Fixed wing / PFC
		AddDefinition("PFC Pitch Up", "PFC_Pitch", "PFC_Pitch", "back", "Fixed Wing");
		AddDefinition("PFC Pitch Down", "PFC_Pitch", "PFC_Pitch", "forward", "Fixed Wing");
		AddDefinition("PFC Roll Right", "PFC_Roll", "PFC_Roll", "right", "Fixed Wing");
		AddDefinition("PFC Roll Left", "PFC_Roll", "PFC_Roll", "left", "Fixed Wing");
		AddDefinition("PFC Yaw Right", "PFC_Yaw", "PFC_Yaw", "right", "Fixed Wing");
		AddDefinition("PFC Yaw Left", "PFC_Yaw", "PFC_Yaw", "left", "Fixed Wing");
		AddDefinition("PFC Throttle Axis", "PFC_ThrottleAxis", "PFC_ThrottleAxis", "forward", "Fixed Wing", true);
		AddDefinition("PFC Throttle Up", "PFC_ThrottleUp", "PFC_ThrottleUp", "hold", "Fixed Wing");
		AddDefinition("PFC Throttle Down", "PFC_ThrottleDown", "PFC_ThrottleDown", "hold", "Fixed Wing");
		AddDefinition("Aircraft Next Weapon", "VehicleNextWeapon", "VehicleNextWeapon", "click", "Fixed Wing");
		AddDefinition("PFC Gear Toggle", "PFC_GearToggle", "PFC_GearToggle", "click", "Fixed Wing");
		AddDefinition("PFC Flaps", "PFC_Flaps", "PFC_Flaps", "click", "Fixed Wing");
		AddDefinition("PFC Airbrake", "PFC_Airbrake", "PFC_Airbrake", "click", "Fixed Wing");
		AddDefinition("PFC Wheel Brake", "PFC_WheelBrake", "PFC_WheelBrake", "pressed", "Fixed Wing");
		AddDefinition("PFC Parking Brake", "PFC_WheelBrakePersistent", "PFC_WheelBrakePersistent", "pressed", "Fixed Wing");
		AddDefinition("PFC Engine Start", "PFC_EngineStart", "PFC_EngineStart", "hold", "Fixed Wing");
		AddDefinition("PFC Engine Stop", "PFC_EngineStop", "PFC_EngineStop", "click", "Fixed Wing");
		AddDefinition("PFC Trim Up", "PFC_TrimUp", "PFC_TrimUp", "hold", "Fixed Wing");
		AddDefinition("PFC Trim Down", "PFC_TrimDown", "PFC_TrimDown", "hold", "Fixed Wing");
		AddDefinition("PFC Trim Reset", "PFC_TrimReset", "PFC_TrimReset", "click", "Fixed Wing");

		// WCS Armament (optional mod actions)
		AddDefinition("WCS Cycle Weapon", "WCS_Armament_CycleWeapon", "WCS_Armament_CycleWeapon", "click", "WCS Armament");
		AddDefinition("WCS Deploy Flares", "WCS_Armament_DeployFlares", "WCS_Armament_DeployFlares", "hold", "WCS Armament");
		AddDefinition("WCS Deploy Chaff", "WCS_Armament_DeployChaffs", "WCS_Armament_DeployChaffs", "hold", "WCS Armament");
		AddDefinition("WCS Turret Stabilization", "WCS_Armament_TurretStabilizationToggle", "WCS_Armament_TurretStabilizationToggle", "click", "WCS Armament");
		AddDefinition("WCS Vehicle Aim", "WCS_Armament_VehicleAim", "WCS_Armament_VehicleAim", "hold", "WCS Armament");
		AddDefinition("WCS Cycle Fire Mode", "WCS_Armament_CycleWeaponFireMode", "WCS_Armament_CycleWeaponFireMode", "click", "WCS Armament");
		AddDefinition("WCS Activate Lock", "WCS_Armament_ActivateLock", "WCS_Armament_ActivateLock", "hold", "WCS Armament");
		AddDefinition("WCS Confirm Lock", "WCS_Armament_ConfirmLock", "WCS_Armament_ConfirmLock", "click", "WCS Armament");
		AddDefinition("WCS Deploy Smoke", "WCS_Armament_DeploySmoke", "WCS_Armament_DeploySmoke", "hold", "WCS Armament");
		AddDefinition("WCS Radar Toggle", "WCS_Armament_RadarToggle", "WCS_Armament_RadarToggle", "click", "WCS Armament");
		AddDefinition("WCS Continuous Smoke", "WCS_Armament_FireContinuousSmokeDispenser", "WCS_Armament_FireContinuousSmokeDispenser", "hold", "WCS Armament");
	}

	//------------------------------------------------------------------------------------------------
	protected void AddDefinition(string displayName, string captureAction, string configAction, string filterPreset, string category, bool fullAxis = false)
	{
		m_Definitions.Insert(new HOTASBindingDefinition(displayName, captureAction, configAction, filterPreset, category, fullAxis));
		m_Bindings.Insert(string.Empty);
	}

	//------------------------------------------------------------------------------------------------
	protected void EnsureManagedConfig()
	{
		FileIO.MakeDirectory(CONFIG_DIRECTORY);
		bool managedFileReady = false;

		if (FileIO.FileExists(MANAGED_CONFIG_PATH))
		{
			FileHandle existingFile = FileIO.OpenFile(MANAGED_CONFIG_PATH, FileMode.READ);
			if (existingFile)
			{
				string firstLine;
				if (existingFile.ReadLine(firstLine) > 0 && firstLine == MANAGED_SIGNATURE)
					managedFileReady = true;
				existingFile.Close();
			}
		}

		if (!managedFileReady)
		{
			ClearBindings();
			WriteManagedConfig();
			SetEditorStatus("Created a fresh HOTAS_Config.conf.");
			return;
		}

		ClearBindings();
		LoadConfigIntoBindings(MANAGED_CONFIG_PATH, true);
	}

	//------------------------------------------------------------------------------------------------
	protected void ClearBindings()
	{
		for (int i = 0; i < m_Bindings.Count(); i++)
			m_Bindings[i] = string.Empty;
	}

	//------------------------------------------------------------------------------------------------
	protected void RefreshImportSelector()
	{
		if (!m_ImportSelector)
			return;

		m_ImportSelector.ClearAll();
		m_ImportConfigs.Clear();
		m_ImportSelector.AddItem("Select read-only config...");

		array<string> configs = {};
		FileIO.FindFiles(configs.Insert, CONFIG_DIRECTORY, ".conf");
		foreach (string config : configs)
		{
			if (FilePath.StripPath(config) == MANAGED_CONFIG_NAME)
				continue;

			m_ImportConfigs.Insert(config);
			m_ImportSelector.AddItem(FilePath.StripPath(config));
		}

		m_ImportSelector.SetCurrentItem(0, false, false, false);
	}

	//------------------------------------------------------------------------------------------------
	protected void RefreshActionSelector()
	{
		if (!m_ActionSelector)
			return;

		m_ActionSelector.ClearAll();
		foreach (HOTASBindingDefinition definition : m_Definitions)
			m_ActionSelector.AddItem(string.Format("[%1] %2", definition.m_sCategory, definition.m_sDisplayName));

		m_iSelectedAction = 0;
		if (!m_Definitions.IsEmpty())
			m_ActionSelector.SetCurrentItem(0, false, false, false);
	}

	//------------------------------------------------------------------------------------------------
	protected void RefreshManagedStatus()
	{
		if (!m_ManagedStatusLabel)
			return;

		string statusText = "available";
		SCR_SettingsManagerKeybindModule keybindModule = GetKeybindModule();
		if (keybindModule)
		{
			InputBinding binding = keybindModule.GetInputBindings();
			if (binding)
			{
				array<ResourceName> activeConfigs = {};
				binding.GetCustomConfigs(activeConfigs);
				foreach (ResourceName activeConfig : activeConfigs)
				{
					if (activeConfig == MANAGED_CONFIG_PATH)
					{
						statusText = "ACTIVE";
						break;
					}
				}
			}
		}

		m_ManagedStatusLabel.SetText(string.Format("Managed file: HOTAS_Config.conf (%1)", statusText));
	}

	//------------------------------------------------------------------------------------------------
	protected void RefreshCurrentBinding()
	{
		if (!m_CurrentBindingLabel || !m_Definitions.IsIndexValid(m_iSelectedAction))
			return;

		HOTASBindingDefinition definition = m_Definitions[m_iSelectedAction];
		string bindingText = m_Bindings[m_iSelectedAction];
		if (bindingText.IsEmpty())
			bindingText = "Unassigned";

		m_CurrentBindingLabel.SetText(string.Format("%1: %2", definition.m_sDisplayName, bindingText));
	}

	//------------------------------------------------------------------------------------------------
	protected void SetEditorStatus(string text)
	{
		if (m_EditorStatusLabel)
			m_EditorStatusLabel.SetText(text);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetCaptureControls(bool capturing)
	{
		m_bCapturing = capturing;

		if (m_BindButton)
			m_BindButton.GetRootWidget().SetEnabled(!capturing);
		if (m_ClearButton)
			m_ClearButton.GetRootWidget().SetEnabled(!capturing);
		if (m_ImportButton)
			m_ImportButton.GetRootWidget().SetEnabled(!capturing);
		if (m_ResetButton)
			m_ResetButton.GetRootWidget().SetEnabled(!capturing);
		if (m_CancelCaptureButton)
			m_CancelCaptureButton.GetRootWidget().SetVisible(capturing);
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_SettingsManagerKeybindModule GetKeybindModule()
	{
		if (!GetGame() || !GetGame().GetSettingsManager())
			return null;

		return SCR_SettingsManagerKeybindModule.Cast(
			GetGame().GetSettingsManager().GetModule(ESettingManagerModuleType.SETTINGS_MANAGER_KEYBINDING)
		);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnImportSelectionChanged(SCR_SpinBoxComponent component, int index)
	{
	}

	//------------------------------------------------------------------------------------------------
	protected void OnActionSelectionChanged(SCR_SpinBoxComponent component, int index)
	{
		if (!m_Definitions.IsIndexValid(index))
			return;

		m_iSelectedAction = index;
		RefreshCurrentBinding();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnImportSelected()
	{
		if (!m_ImportSelector)
			return;

		int importIndex = m_ImportSelector.GetCurrentIndex() - 1;
		if (!m_ImportConfigs.IsIndexValid(importIndex))
		{
			SetEditorStatus("Choose an external .conf file first.");
			return;
		}

		string sourcePath = m_ImportConfigs[importIndex];
		ClearBindings();
		int importedCount = LoadConfigIntoBindings(sourcePath, false);
		WriteManagedConfig();
		ActivateManagedConfig();
		RefreshCurrentBinding();
		SetEditorStatus(string.Format("Imported %1 supported bindings from %2. Source file was not modified.", importedCount, FilePath.StripPath(sourcePath)));
	}

	//------------------------------------------------------------------------------------------------
	protected void OnActivateManaged()
	{
		ActivateManagedConfig();
		SetEditorStatus("HOTAS_Config.conf activated.");
	}

	//------------------------------------------------------------------------------------------------
	protected void OnResetManaged()
	{
		StopCapture(true);
		ClearBindings();
		WriteManagedConfig();
		ActivateManagedConfig();
		RefreshCurrentBinding();
		SetEditorStatus("HOTAS_Config.conf reset. All managed bindings were cleared.");
	}

	//------------------------------------------------------------------------------------------------
	protected void OnClearBinding()
	{
		if (!m_Definitions.IsIndexValid(m_iSelectedAction))
			return;

		m_Bindings[m_iSelectedAction] = string.Empty;
		WriteManagedConfig();
		ActivateManagedConfig();
		RefreshCurrentBinding();
		SetEditorStatus("Binding cleared and managed config regenerated.");
	}

	//------------------------------------------------------------------------------------------------
	protected void OnBindInput()
	{
		if (m_bCapturing || !m_Definitions.IsIndexValid(m_iSelectedAction))
			return;

		InputManager inputManager = GetGame().GetInputManager();
		if (!inputManager)
		{
			SetEditorStatus("InputManager is unavailable.");
			return;
		}

		HOTASBindingDefinition definition = m_Definitions[m_iSelectedAction];
		m_CaptureBinding = inputManager.CreateUserBinding();
		if (!m_CaptureBinding)
		{
			SetEditorStatus("Could not create a temporary capture binding.");
			return;
		}

		if (!m_CaptureBinding.FindAction(definition.m_sCaptureAction))
		{
			m_CaptureBinding = null;
			SetEditorStatus(string.Format("%1 is not available in the currently loaded game/mod set.", definition.m_sCaptureAction));
			return;
		}

		EInputBindingAxleCapture axleCapture = EInputBindingAxleCapture.HALF_AXLE;
		if (definition.m_bFullAxis)
			axleCapture = EInputBindingAxleCapture.FULL_AXLE;

		m_CaptureBinding.StartCapture(definition.m_sCaptureAction, EInputDeviceType.JOYSTICK, string.Empty, false, axleCapture);
		SetCaptureControls(true);
		SetEditorStatus(string.Format("Listening for joystick input for %1... Press Cancel to stop.", definition.m_sDisplayName));

		GetGame().GetCallqueue().Remove(PollCapture);
		GetGame().GetCallqueue().CallLater(PollCapture, 100, true);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCancelCapture()
	{
		StopCapture(true);
		SetEditorStatus("Input capture canceled.");
	}

	//------------------------------------------------------------------------------------------------
	protected void StopCapture(bool cancelCapture)
	{
		GetGame().GetCallqueue().Remove(PollCapture);

		if (cancelCapture && m_CaptureBinding && m_CaptureBinding.GetCaptureState() == EInputBindingCaptureState.CAPTURING)
			m_CaptureBinding.CancelCapture();

		m_CaptureBinding = null;
		SetCaptureControls(false);
	}

	//------------------------------------------------------------------------------------------------
	protected void PollCapture()
	{
		if (!m_bCapturing || !m_CaptureBinding)
			return;

		if (m_CaptureBinding.GetCaptureState() == EInputBindingCaptureState.CAPTURING)
			return;

		if (!m_Definitions.IsIndexValid(m_iSelectedAction))
		{
			StopCapture(false);
			return;
		}

		HOTASBindingDefinition definition = m_Definitions[m_iSelectedAction];
		array<string> capturedBindings = {};
		m_CaptureBinding.GetBindings(definition.m_sCaptureAction, capturedBindings, EInputDeviceType.JOYSTICK, string.Empty, false);

		string capturedBinding;
		foreach (string candidate : capturedBindings)
		{
			if (candidate.IndexOf("joystick") >= 0)
				capturedBinding = candidate;
		}

		GetGame().GetCallqueue().Remove(PollCapture);
		m_CaptureBinding = null;
		SetCaptureControls(false);

		if (capturedBinding.IsEmpty())
		{
			SetEditorStatus("No joystick input was captured.");
			return;
		}

		m_Bindings[m_iSelectedAction] = capturedBinding;
		WriteManagedConfig();
		ActivateManagedConfig();
		RefreshCurrentBinding();
		SetEditorStatus(string.Format("Bound %1 to %2.", definition.m_sDisplayName, capturedBinding));
	}

	//------------------------------------------------------------------------------------------------
	protected int LoadConfigIntoBindings(string path, bool managedFile)
	{
		FileHandle file = FileIO.OpenFile(path, FileMode.READ);
		if (!file)
			return 0;

		if (!managedFile)
			ClearBindings();

		string currentAction;
		string currentFilter;
		string line;
		int importedCount;

		while (file.ReadLine(line) > 0)
		{
			int actionToken = line.IndexOf("Action ");
			int braceToken = line.IndexOf("{");
			if (actionToken >= 0 && braceToken > actionToken)
			{
				currentAction = line.Substring(actionToken + 7, braceToken - actionToken - 7);
				currentAction.Replace(" ", "");
				currentAction.Replace("\t", "");
				currentFilter = string.Empty;
				continue;
			}

			int filterToken = line.IndexOf("FilterPreset \"");
			if (filterToken >= 0)
			{
				string filterTail = line.Substring(filterToken + 14, line.Length() - filterToken - 14);
				int filterEnd = filterTail.IndexOf("\"");
				if (filterEnd >= 0)
					currentFilter = filterTail.Substring(0, filterEnd);
				continue;
			}

			int inputToken = line.IndexOf("Input \"");
			if (inputToken < 0 || currentAction.IsEmpty())
				continue;

			string inputTail = line.Substring(inputToken + 7, line.Length() - inputToken - 7);
			int inputEnd = inputTail.IndexOf("\"");
			if (inputEnd < 0)
				continue;

			string inputValue = inputTail.Substring(0, inputEnd);
			if (inputValue.IndexOf("joystick") < 0)
				continue;

			int definitionIndex = FindDefinitionForImport(currentAction, currentFilter);
			if (definitionIndex < 0)
				continue;

			m_Bindings[definitionIndex] = inputValue;
			importedCount++;
			currentFilter = string.Empty;
		}

		file.Close();
		return importedCount;
	}

	//------------------------------------------------------------------------------------------------
	protected int FindDefinitionForImport(string configAction, string filterPreset)
	{
		int firstMatch = -1;

		for (int i = 0; i < m_Definitions.Count(); i++)
		{
			HOTASBindingDefinition definition = m_Definitions[i];
			if (definition.m_sConfigAction != configAction)
				continue;

			if (firstMatch < 0)
				firstMatch = i;

			if (!filterPreset.IsEmpty() && definition.m_sFilterPreset == filterPreset)
				return i;
		}

		return firstMatch;
	}

	//------------------------------------------------------------------------------------------------
	protected bool HasBindingForConfigAction(string configAction)
	{
		for (int i = 0; i < m_Definitions.Count(); i++)
		{
			if (m_Definitions[i].m_sConfigAction == configAction && !m_Bindings[i].IsEmpty())
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsFirstDefinitionForConfigAction(int index)
	{
		if (!m_Definitions.IsIndexValid(index))
			return false;

		string configAction = m_Definitions[index].m_sConfigAction;
		for (int i = 0; i < index; i++)
		{
			if (m_Definitions[i].m_sConfigAction == configAction)
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected string MakeGuidPrefix(string prefix, int index)
	{
		string number = (index + 1).ToString();
		string padding = "000000000000000";
		int keep = 15 - number.Length();
		if (keep < 0)
			keep = 0;

		return prefix + padding.Substring(0, keep) + number;
	}

	//------------------------------------------------------------------------------------------------
	protected bool NeedsNegativeMultiplier(HOTASBindingDefinition definition)
	{
		if (definition.m_sConfigAction == "PFC_Pitch" && definition.m_sFilterPreset == "forward")
			return true;
		if (definition.m_sConfigAction == "PFC_Roll" && definition.m_sFilterPreset == "left")
			return true;
		if (definition.m_sConfigAction == "PFC_Yaw" && definition.m_sFilterPreset == "left")
			return true;

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected void WriteSpecialFilter(FileHandle file, HOTASBindingDefinition definition, int definitionIndex)
	{
		string filterGuid = MakeGuidPrefix("C", definitionIndex);

		if (definition.m_sConfigAction == "CharacterNextWeapon")
		{
			file.WriteLine(string.Format("      Filter InputFilterSingleClick \"%1\" {", filterGuid));
			file.WriteLine("      }");
			return;
		}

		if (definition.m_sConfigAction == "TurretNextWeapon")
		{
			file.WriteLine(string.Format("      Filter InputFilterHoldOnce \"%1\" {", filterGuid));
			file.WriteLine("       HoldDuration 25");
			file.WriteLine("      }");
			return;
		}

		if (definition.m_sConfigAction == "SelectAction" || definition.m_sConfigAction == "HelicopterSightZeroing")
		{
			file.WriteLine(string.Format("      Filter InputFilterRepeat \"%1\" {", filterGuid));
			if (definition.m_sFilterPreset == "next" || definition.m_sFilterPreset == "down")
				file.WriteLine("       Multiplier -1");
			file.WriteLine("      }");
			return;
		}

		if (NeedsNegativeMultiplier(definition))
		{
			file.WriteLine(string.Format("      Filter InputFilterValue \"%1\" {", filterGuid));
			file.WriteLine("       Multiplier -1");
			file.WriteLine("      }");
			return;
		}

		if (definition.m_sFilterPreset == "toggle")
		{
			file.WriteLine(string.Format("      Filter InputFilterDown \"%1\" {", filterGuid));
			file.WriteLine("      }");
			return;
		}

		if (definition.m_sFilterPreset == "hold" && (definition.m_sConfigAction.Contains("Engine") || definition.m_sConfigAction.Contains("ADS")))
		{
			file.WriteLine(string.Format("      Filter InputFilterHold \"%1\" {", filterGuid));
			if (definition.m_sConfigAction.Contains("ADSHold"))
				file.WriteLine("       HoldDuration -1");
			file.WriteLine("      }");
			return;
		}

		if (definition.m_sConfigAction.Contains("Reset"))
		{
			file.WriteLine(string.Format("      Filter InputFilterSingleClick \"%1\" {", filterGuid));
			file.WriteLine("      }");
			return;
		}

		if (definition.m_sConfigAction == "HelicopterSightDeploy" || definition.m_sConfigAction == "VehicleDoorToggle")
		{
			file.WriteLine(string.Format("      Filter InputFilterClick \"%1\" {", filterGuid));
			file.WriteLine("      }");
			return;
		}

		if (definition.m_sConfigAction.Contains("EngineStop"))
		{
			file.WriteLine(string.Format("      Filter InputFilterHoldOnce \"%1\" {", filterGuid));
			file.WriteLine("      }");
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool WriteManagedConfig()
	{
		FileIO.MakeDirectory(CONFIG_DIRECTORY);
		FileHandle file = FileIO.OpenFile(MANAGED_CONFIG_PATH, FileMode.WRITE);
		if (!file)
		{
			SetEditorStatus("ERROR: Could not write HOTAS_Config.conf.");
			return false;
		}

		file.WriteLine(MANAGED_SIGNATURE);
		file.WriteLine("// This file is fully regenerated by Reforger HOTAS Debugger.");
		file.WriteLine("// Other custom input configs are read-only to this mod.");
		file.WriteLine("ActionManager {");
		file.WriteLine(" Actions {");

		for (int i = 0; i < m_Definitions.Count(); i++)
		{
			if (!IsFirstDefinitionForConfigAction(i))
				continue;

			HOTASBindingDefinition firstDefinition = m_Definitions[i];
			string configAction = firstDefinition.m_sConfigAction;
			if (!HasBindingForConfigAction(configAction))
				continue;

			file.WriteLine(string.Format("  Action %1 {", configAction));
			if (configAction == "SelectAction" || configAction == "HelicopterSightZeroing")
				file.WriteLine("   Type AnalogRelative");
			file.WriteLine(string.Format("   InputSource InputSourceSum \"%1\" {", MakeGuidPrefix("A", i)));
			file.WriteLine("    Sources {");

			for (int j = 0; j < m_Definitions.Count(); j++)
			{
				HOTASBindingDefinition definition = m_Definitions[j];
				if (definition.m_sConfigAction != configAction || m_Bindings[j].IsEmpty())
					continue;

				string inputValue = m_Bindings[j];
				if (definition.m_bFullAxis && inputValue.Length() > 0)
				{
					string lastCharacter = inputValue.Substring(inputValue.Length() - 1, 1);
					if (lastCharacter == "+" || lastCharacter == "-")
						inputValue = inputValue.Substring(0, inputValue.Length() - 1);
				}

				file.WriteLine(string.Format("     InputSourceValue \"%1\" {", MakeGuidPrefix("B", j)));
				file.WriteLine(string.Format("      FilterPreset \"%1\"", definition.m_sFilterPreset));
				file.WriteLine(string.Format("      Input \"%1\"", inputValue));
				WriteSpecialFilter(file, definition, j);
				file.WriteLine("     }");
			}

			file.WriteLine("    }");
			file.WriteLine("   }");
			file.WriteLine("  }");
		}

		file.WriteLine(" }");
		file.WriteLine("}");
		file.Close();
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void ActivateManagedConfig()
	{
		SCR_SettingsManagerKeybindModule keybindModule = GetKeybindModule();
		if (!keybindModule)
		{
			SetEditorStatus("Managed config saved, but keybind module is unavailable.");
			return;
		}

		keybindModule.SelectJoystickPresetPath(MANAGED_CONFIG_PATH);
		HOTASDebugController.GetInstance().PrepareForSettings();
		HOTASDebugController.GetInstance().RefreshAssignedAxes();
		RefreshManagedStatus();
	}
}

//------------------------------------------------------------------------------------------------
// Add the binding editor as a separate peer settings tab.
// The existing HOTAS HUD settings tab remains separate and is not auto-opened.
modded class SCR_SettingsSuperMenu
{
	protected static const string HOTAS_BINDINGS_TAB_IDENTIFIER = "HOTASInputBindingEditor";

	override void OnMenuOpen()
	{
		super.OnMenuOpen();

		if (!m_SuperMenuComponent || !m_SuperMenuComponent.GetTabView())
			return;

		m_SuperMenuComponent.GetTabView().RemoveTabByIdentifier(HOTAS_BINDINGS_TAB_IDENTIFIER);
		m_SuperMenuComponent.GetTabView().AddTab(
			"{A7B3D419C82E6F10}UI/layouts/Menus/SettingsSubMenus/HOTASBindings.layout",
			"HOTAS Bindings",
			true,
			identifier: HOTAS_BINDINGS_TAB_IDENTIFIER
		);
	}
}
