class HOTASSettingsSubMenu : SCR_SettingsSubMenuBase
{
	protected SCR_SpinBoxComponent m_HotasConfig;
	protected ref array<SCR_SpinBoxComponent> m_HudControls = {};
	protected SCR_SliderComponent m_HudScaleSlider;
	protected SCR_SliderComponent m_BackgroundOpacitySlider;
	protected ref array<SCR_EditBoxComponent> m_AxisLabelEditors = {};
	protected ref array<string> m_UserConfigs = {};
	protected bool m_bLoading;

	//------------------------------------------------------------------------------------------------
	override void OnTabCreate(Widget menuRoot, ResourceName buttonsLayout, int index)
	{
		super.OnTabCreate(menuRoot, buttonsLayout, index);

		m_bLoading = true;
		HOTASDebugController.GetInstance().ReloadHudSettings();

		m_HotasConfig = FindSpinBox("HOTASConfig");
		SetupHotasConfigSelector();
		SetupHudControls();
		SetupHudSliders();
		SetupAxisLabelEditors();
		m_bLoading = false;
	}

	//------------------------------------------------------------------------------------------------
	override void OnTabShow()
	{
		super.OnTabShow();

		m_bLoading = true;
		HOTASDebugController.GetInstance().ReloadHudSettings();
		SyncHotasConfigSelector();
		SyncHudControls();
		SyncHudSliders();
		SyncAxisLabelEditors();
		m_bLoading = false;
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
	protected SCR_SliderComponent FindSlider(string widgetName)
	{
		Widget widget = m_wRoot.FindAnyWidget(widgetName);
		if (!widget)
			return null;

		return SCR_SliderComponent.Cast(widget.FindHandler(SCR_SliderComponent));
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_EditBoxComponent FindEditBox(string widgetName)
	{
		Widget widget = m_wRoot.FindAnyWidget(widgetName);
		if (!widget)
			return null;

		return SCR_EditBoxComponent.Cast(widget.FindHandler(SCR_EditBoxComponent));
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_SettingsManagerKeybindModule GetKeybindModule()
	{
		return SCR_SettingsManagerKeybindModule.Cast(
			GetGame().GetSettingsManager().GetModule(ESettingManagerModuleType.SETTINGS_MANAGER_KEYBINDING)
		);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetupHotasConfigSelector()
	{
		if (!m_HotasConfig)
			return;

		m_HotasConfig.ClearAll();
		m_UserConfigs.Clear();

		m_HotasConfig.AddItem("Default / Other");

		// Match the vanilla Controller Presets settings page: user joystick configs are
		// discovered directly from the profile customInputConfigs folder.
		FileIO.FindFiles(m_UserConfigs.Insert, "$profile:.save/settings/customInputConfigs", ".conf");
		foreach (string config : m_UserConfigs)
			m_HotasConfig.AddItem(FilePath.StripPath(config));

		SyncHotasConfigSelector();
		m_HotasConfig.m_OnChanged.Insert(OnHotasConfigChanged);
	}

	//------------------------------------------------------------------------------------------------
	protected void SyncHotasConfigSelector()
	{
		if (!m_HotasConfig)
			return;

		SCR_SettingsManagerKeybindModule keybindModule = GetKeybindModule();
		if (!keybindModule)
		{
			m_HotasConfig.SetCurrentItem(0, false, false, false);
			return;
		}

		InputBinding binding = keybindModule.GetInputBindings();
		if (!binding)
		{
			m_HotasConfig.SetCurrentItem(0, false, false, false);
			return;
		}

		array<ResourceName> activeConfigs = {};
		binding.GetCustomConfigs(activeConfigs);

		int selected = 0;
		foreach (int i, string config : m_UserConfigs)
		{
			foreach (ResourceName activeConfig : activeConfigs)
			{
				if (activeConfig == config)
				{
					selected = i + 1;
					break;
				}
			}
			if (selected > 0)
				break;
		}

		m_HotasConfig.SetCurrentItem(selected, false, false, false);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnHotasConfigChanged(SCR_SpinBoxComponent component, int index)
	{
		if (m_bLoading)
			return;

		SCR_SettingsManagerKeybindModule keybindModule = GetKeybindModule();
		if (!keybindModule)
			return;

		InputBinding binding = keybindModule.GetInputBindings();
		if (!binding)
			return;

		if (index <= 0)
		{
			array<ResourceName> emptyConfigs = {};
			binding.SetCustomConfigs(emptyConfigs);
			binding.Save();
			Print("[HOTAS Debugger] Custom HOTAS input config cleared from Settings", LogLevel.NORMAL);
			ScheduleAxisAssignmentRefresh();
			return;
		}

		int configIndex = index - 1;
		if (!m_UserConfigs.IsIndexValid(configIndex))
			return;

		string selectedConfig = m_UserConfigs.Get(configIndex);
		keybindModule.SelectJoystickPresetPath(selectedConfig);
		Print(string.Format("[HOTAS Debugger] HOTAS input config selected from Settings: %1", selectedConfig), LogLevel.NORMAL);
		ScheduleAxisAssignmentRefresh();
	}

	//------------------------------------------------------------------------------------------------
	protected void ScheduleAxisAssignmentRefresh()
	{
		GetGame().GetCallqueue().CallLater(RefreshAxisAssignments, 0, false);
	}

	//------------------------------------------------------------------------------------------------
	protected void RefreshAxisAssignments()
	{
		HOTASDebugController.GetInstance().RefreshAssignedAxes();
		m_bLoading = true;
		SyncAxisLabelEditors();
		m_bLoading = false;
	}

	//------------------------------------------------------------------------------------------------
	protected void SetupHudControls()
	{
		m_HudControls.Clear();

		// Keep this array aligned to HOTASDebugController setting indices. Scale (2)
		// and Background Opacity (6) are sliders. Axis rows (7-10) are edit boxes, so FindSpinBox intentionally returns null.
		array<string> widgetNames = {
			"HUDEnabled",
			"HUDPosition",
			"HUDScale",
			"FadeDelay",
			"FadeDuration",
			"BackgroundEnabled",
			"BackgroundOpacity",
			"RollAxis",
			"PitchAxis",
			"ThrottleAxis",
			"YawAxis",
			"DebugMode"
		};

		HOTASDebugController controller = HOTASDebugController.GetInstance();
		for (int i = 0; i < controller.GetSettingsCount(); i++)
		{
			SCR_SpinBoxComponent control = FindSpinBox(widgetNames[i]);
			m_HudControls.Insert(control);
			if (!control)
				continue;

			control.ClearAll();
			int optionCount = controller.GetSettingOptionCount(i);
			for (int optionIndex = 0; optionIndex < optionCount; optionIndex++)
				control.AddItem(controller.GetSettingOptionLabel(i, optionIndex), optionIndex == optionCount - 1);

			control.SetCurrentItem(controller.GetSettingOptionIndex(i), false, false, false);
			control.m_OnChanged.Insert(OnHudSettingChanged);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void SetupHudSliders()
	{
		HOTASDebugController controller = HOTASDebugController.GetInstance();

		m_HudScaleSlider = FindSlider("HUDScale");
		if (m_HudScaleSlider)
		{
			m_HudScaleSlider.SetSliderSettings(0.0, 100.0, 1.0, "%1%");
			m_HudScaleSlider.SetValue(controller.GetHudScalePercent());
			m_HudScaleSlider.GetOnChangedFinal().Insert(OnHudScaleChanged);
		}

		m_BackgroundOpacitySlider = FindSlider("BackgroundOpacity");
		if (m_BackgroundOpacitySlider)
		{
			m_BackgroundOpacitySlider.SetSliderSettings(0.0, 100.0, 1.0, "%1%");
			m_BackgroundOpacitySlider.SetValue(controller.GetBackgroundOpacityPercent());
			m_BackgroundOpacitySlider.GetOnChangedFinal().Insert(OnBackgroundOpacityChanged);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void SyncHudControls()
	{
		HOTASDebugController controller = HOTASDebugController.GetInstance();
		for (int i = 0; i < m_HudControls.Count(); i++)
		{
			SCR_SpinBoxComponent control = m_HudControls[i];
			if (control)
				control.SetCurrentItem(controller.GetSettingOptionIndex(i), false, false, false);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void SyncHudSliders()
	{
		HOTASDebugController controller = HOTASDebugController.GetInstance();
		if (m_HudScaleSlider)
			m_HudScaleSlider.SetValue(controller.GetHudScalePercent());
		if (m_BackgroundOpacitySlider)
			m_BackgroundOpacitySlider.SetValue(controller.GetBackgroundOpacityPercent());
	}

	//------------------------------------------------------------------------------------------------
	protected void SetupAxisLabelEditors()
	{
		m_AxisLabelEditors.Clear();
		array<string> widgetNames = { "RollAxis", "PitchAxis", "ThrottleAxis", "YawAxis" };
		HOTASDebugController controller = HOTASDebugController.GetInstance();
		controller.RefreshAssignedAxes();

		for (int i = 0; i < widgetNames.Count(); i++)
		{
			SCR_EditBoxComponent editor = FindEditBox(widgetNames[i]);
			m_AxisLabelEditors.Insert(editor);
			if (!editor)
				continue;

			editor.SetLabel(controller.GetAxisSettingRowLabel(i));
			editor.SetValue(controller.GetAxisCustomLabel(i));
			editor.SetPlaceholderText("Custom HUD label");
			editor.m_OnConfirm.Insert(OnAxisLabelConfirmed);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void SyncAxisLabelEditors()
	{
		HOTASDebugController controller = HOTASDebugController.GetInstance();
		for (int i = 0; i < m_AxisLabelEditors.Count(); i++)
		{
			SCR_EditBoxComponent editor = m_AxisLabelEditors[i];
			if (!editor)
				continue;

			editor.SetLabel(controller.GetAxisSettingRowLabel(i));
			editor.SetValue(controller.GetAxisCustomLabel(i));
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnAxisLabelConfirmed(SCR_EditBoxComponent component, string value)
	{
		if (m_bLoading)
			return;

		for (int i = 0; i < m_AxisLabelEditors.Count(); i++)
		{
			if (m_AxisLabelEditors[i] != component)
				continue;

			HOTASDebugController.GetInstance().SetAxisCustomLabel(i, value);
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnHudSettingChanged(SCR_SpinBoxComponent component, int optionIndex)
	{
		if (m_bLoading)
			return;

		for (int i = 0; i < m_HudControls.Count(); i++)
		{
			if (m_HudControls[i] != component)
				continue;

			HOTASDebugController.GetInstance().SetSettingOptionIndex(i, optionIndex);
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnHudScaleChanged(SCR_SliderComponent component, float value)
	{
		if (m_bLoading)
			return;

		HOTASDebugController.GetInstance().SetHudScalePercent(value);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnBackgroundOpacityChanged(SCR_SliderComponent component, float value)
	{
		if (m_bLoading)
			return;

		HOTASDebugController.GetInstance().SetBackgroundOpacityPercent(value);
	}
}

//------------------------------------------------------------------------------------------------
// Add HOTAS as a normal peer of the game's Video / Audio / Interface settings tabs.
modded class SCR_SettingsSuperMenu
{
	protected static const string HOTAS_SETTINGS_TAB_IDENTIFIER = "HOTASInputHUDSettings";

	override void OnMenuOpen()
	{
		super.OnMenuOpen();

		if (!m_SuperMenuComponent || !m_SuperMenuComponent.GetTabView())
			return;

		m_SuperMenuComponent.GetTabView().RemoveTabByIdentifier(HOTAS_SETTINGS_TAB_IDENTIFIER);
		m_SuperMenuComponent.GetTabView().AddTab(
			"{8C52D9F7A31B640E}UI/layouts/Menus/SettingsSubMenus/HOTASSettings.layout",
			"HOTAS",
			true,
			identifier: HOTAS_SETTINGS_TAB_IDENTIFIER
		);
	}
}
