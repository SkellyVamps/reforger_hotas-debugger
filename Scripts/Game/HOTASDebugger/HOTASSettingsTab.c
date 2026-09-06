class HOTASSettingsSubMenu : SCR_SettingsSubMenuBase
{
	protected SCR_SpinBoxComponent m_HotasConfig;
	protected ref array<SCR_SpinBoxComponent> m_HudControls = {};
	protected ref array<string> m_UserConfigs = {};
	protected bool m_bLoading;
	protected Widget m_PreviewHost;
	protected Widget m_PreviewSquare;
	protected Widget m_PreviewSquareBackground;
	protected Widget m_ScreenPreview;
	protected Widget m_ScreenPreviewBackground;
	protected Widget m_HudPositionPreview;

	//------------------------------------------------------------------------------------------------
	override void OnTabCreate(Widget menuRoot, ResourceName buttonsLayout, int index)
	{
		super.OnTabCreate(menuRoot, buttonsLayout, index);

		m_bLoading = true;
		HOTASDebugController.GetInstance().ReloadHudSettings();

		m_HotasConfig = FindSpinBox("HOTASConfig");
		SetupHotasConfigSelector();
		SetupHudControls();
		SetupHudPositionPreview();
		m_bLoading = false;
		GetGame().GetCallqueue().CallLater(UpdateHudPositionPreview, 0, false);
	}

	//------------------------------------------------------------------------------------------------
	override void OnTabShow()
	{
		super.OnTabShow();

		m_bLoading = true;
		HOTASDebugController.GetInstance().ReloadHudSettings();
		SyncHotasConfigSelector();
		SyncHudControls();
		m_bLoading = false;

		GetGame().GetCallqueue().Remove(UpdateHudPositionPreview);
		GetGame().GetCallqueue().CallLater(UpdateHudPositionPreview, 0, false);
		GetGame().GetCallqueue().CallLater(UpdateHudPositionPreview, 250, true);
	}

	//------------------------------------------------------------------------------------------------
	override void OnTabHide()
	{
		GetGame().GetCallqueue().Remove(UpdateHudPositionPreview);
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
			return;
		}

		int configIndex = index - 1;
		if (!m_UserConfigs.IsIndexValid(configIndex))
			return;

		string selectedConfig = m_UserConfigs.Get(configIndex);
		keybindModule.SelectJoystickPresetPath(selectedConfig);
		Print(string.Format("[HOTAS Debugger] HOTAS input config selected from Settings: %1", selectedConfig), LogLevel.NORMAL);
	}


	//------------------------------------------------------------------------------------------------
	protected void SetupHudPositionPreview()
	{
		m_PreviewHost = m_wRoot.FindAnyWidget("HUDPreviewHost");
		m_PreviewSquare = m_wRoot.FindAnyWidget("HUDPreviewSquare");
		m_PreviewSquareBackground = m_wRoot.FindAnyWidget("HUDPreviewSquareBackground");
		m_ScreenPreview = m_wRoot.FindAnyWidget("HUDScreenPreview");
		m_ScreenPreviewBackground = m_wRoot.FindAnyWidget("HUDScreenPreviewBackground");
		m_HudPositionPreview = m_wRoot.FindAnyWidget("HUDPositionPreview");
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateHudPositionPreview()
	{
		if (!m_PreviewHost || !m_PreviewSquare || !m_PreviewSquareBackground || !m_ScreenPreview || !m_ScreenPreviewBackground || !m_HudPositionPreview)
			return;

		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace)
			return;

		float hostWidthPx;
		float hostHeightPx;
		m_PreviewHost.GetScreenSize(hostWidthPx, hostHeightPx);
		float hostWidth = workspace.DPIUnscale(hostWidthPx);
		float hostHeight = workspace.DPIUnscale(hostHeightPx);
		if (hostWidth <= 1 || hostHeight <= 1)
			return;

		// The outer preview stays square, while the inner screen preserves the player's
		// actual current display aspect ratio. This makes ultrawide, 16:9, 16:10 and
		// other resolutions preview the same normalized HUD placement used in game.
		float squareSize = Math.Min(hostWidth, hostHeight) - 24;
		if (squareSize <= 32)
			return;

		float squareLeft = (hostWidth - squareSize) * 0.5;
		float squareTop = (hostHeight - squareSize) * 0.5;
		FrameSlot.SetPos(m_PreviewSquare, squareLeft, squareTop);
		FrameSlot.SetSize(m_PreviewSquare, squareSize, squareSize);
		FrameSlot.SetPos(m_PreviewSquareBackground, 0, 0);
		FrameSlot.SetSize(m_PreviewSquareBackground, squareSize, squareSize);

		float screenWidth = workspace.GetWidth();
		float screenHeight = workspace.GetHeight();
		if (screenWidth <= 0 || screenHeight <= 0)
			return;

		float inset = 24;
		float available = squareSize - inset * 2;
		if (available <= 1)
			return;

		float screenAspect = screenWidth / screenHeight;
		float previewWidth = available;
		float previewHeight = available / screenAspect;
		if (previewHeight > available)
		{
			previewHeight = available;
			previewWidth = available * screenAspect;
		}

		float screenLeft = (squareSize - previewWidth) * 0.5;
		float screenTop = (squareSize - previewHeight) * 0.5;
		FrameSlot.SetPos(m_ScreenPreview, screenLeft, screenTop);
		FrameSlot.SetSize(m_ScreenPreview, previewWidth, previewHeight);
		FrameSlot.SetPos(m_ScreenPreviewBackground, 0, 0);
		FrameSlot.SetSize(m_ScreenPreviewBackground, previewWidth, previewHeight);

		HOTASDebugController controller = HOTASDebugController.GetInstance();
		int positionIndex = controller.GetSettingOptionIndex(1);
		float hudScale = 0.5 + controller.GetSettingOptionIndex(2) * 0.1;

		float hudWidth = 700 * hudScale;
		float hudHeight = 70 * hudScale;
		float marginX = 48 * hudScale;
		float marginY = 54 * hudScale;
		float hudLeft = (screenWidth - hudWidth) * 0.5;
		float hudTop = screenHeight - hudHeight - marginY;

		switch (positionIndex)
		{
			case 0: hudLeft = marginX; hudTop = marginY; break;
			case 1: hudLeft = (screenWidth - hudWidth) * 0.5; hudTop = marginY; break;
			case 2: hudLeft = screenWidth - hudWidth - marginX; hudTop = marginY; break;
			case 3: hudLeft = marginX; hudTop = (screenHeight - hudHeight) * 0.5; break;
			case 4: hudLeft = (screenWidth - hudWidth) * 0.5; hudTop = (screenHeight - hudHeight) * 0.5; break;
			case 5: hudLeft = screenWidth - hudWidth - marginX; hudTop = (screenHeight - hudHeight) * 0.5; break;
			case 6: hudLeft = marginX; hudTop = screenHeight - hudHeight - marginY; break;
			case 7: hudLeft = (screenWidth - hudWidth) * 0.5; hudTop = screenHeight - hudHeight - marginY; break;
			case 8: hudLeft = screenWidth - hudWidth - marginX; hudTop = screenHeight - hudHeight - marginY; break;
		}

		float previewHudWidth = previewWidth * (hudWidth / screenWidth);
		float previewHudHeight = previewHeight * (hudHeight / screenHeight);
		float previewHudLeft = previewWidth * (hudLeft / screenWidth);
		float previewHudTop = previewHeight * (hudTop / screenHeight);

		if (previewHudWidth < 8)
			previewHudWidth = 8;
		if (previewHudHeight < 5)
			previewHudHeight = 5;

		FrameSlot.SetPos(m_HudPositionPreview, previewHudLeft, previewHudTop);
		FrameSlot.SetSize(m_HudPositionPreview, previewHudWidth, previewHudHeight);

		if (controller.GetSettingOptionIndex(0) == 0)
			m_HudPositionPreview.SetOpacity(0.3);
		else
			m_HudPositionPreview.SetOpacity(0.9);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetupHudControls()
	{
		m_HudControls.Clear();

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
	protected void OnHudSettingChanged(SCR_SpinBoxComponent component, int optionIndex)
	{
		if (m_bLoading)
			return;

		for (int i = 0; i < m_HudControls.Count(); i++)
		{
			if (m_HudControls[i] != component)
				continue;

			HOTASDebugController.GetInstance().SetSettingOptionIndex(i, optionIndex);
			UpdateHudPositionPreview();
			return;
		}
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
