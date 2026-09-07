class HOTASSettingsSubMenu : SCR_SettingsSubMenuBase
{
	protected SCR_SpinBoxComponent m_HotasConfig;
	protected ref array<SCR_SpinBoxComponent> m_HudControls = {};
	protected SCR_SliderComponent m_HudScaleSlider;
	protected SCR_SliderComponent m_BackgroundOpacitySlider;
	protected ref array<string> m_UserConfigs = {};
	protected bool m_bLoading;
	protected Widget m_PreviewHost;
	protected Widget m_PreviewSquare;
	protected Widget m_PreviewSquareBackground;
	protected Widget m_ScreenPreview;
	protected Widget m_ScreenPreviewBackground;
	protected Widget m_HudPositionPreview;
	protected ref HOTASHudPositionDragHandler m_HudDragHandler;
	protected bool m_bDraggingHudPosition;
	protected float m_fHudDragOffsetX;
	protected float m_fHudDragOffsetY;
	protected float m_fPreviewPositionX = 0.5;
	protected float m_fPreviewPositionY = 0.95;

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
		SetupHudPositionPreview();
		SyncHudPositionPreviewFromController();
		m_bLoading = false;
		GetGame().GetCallqueue().CallLater(RefreshAllSpinBoxArrows, 0, false);
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
		SyncHudSliders();
		SyncHudPositionPreviewFromController();
		m_bLoading = false;
		GetGame().GetCallqueue().CallLater(RefreshAllSpinBoxArrows, 0, false);

		GetGame().GetCallqueue().Remove(UpdateHudPositionPreview);
		GetGame().GetCallqueue().CallLater(UpdateHudPositionPreview, 0, false);
		GetGame().GetCallqueue().CallLater(UpdateHudPositionPreview, 250, true);
	}

	//------------------------------------------------------------------------------------------------
	override void OnTabHide()
	{
		GetGame().GetCallqueue().Remove(UpdateHudPositionPreview);
		GetGame().GetCallqueue().Remove(UpdateHudPositionDrag);
		if (m_bDraggingHudPosition)
			EndHudPositionDrag();
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
	protected SCR_SliderComponent FindSlider(string widgetName)
	{
		Widget widget = m_wRoot.FindAnyWidget(widgetName);
		if (!widget)
			return null;

		return SCR_SliderComponent.Cast(widget.FindHandler(SCR_SliderComponent));
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
		RefreshSpinBoxArrows(m_HotasConfig, selected, m_UserConfigs.Count() + 1);
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
			RefreshSpinBoxArrows(component, index, m_UserConfigs.Count() + 1);
			GetGame().GetCallqueue().CallLater(RefreshAllSpinBoxArrows, 0, false);
			Print("[HOTAS Debugger] Custom HOTAS input config cleared from Settings", LogLevel.NORMAL);
			return;
		}

		int configIndex = index - 1;
		if (!m_UserConfigs.IsIndexValid(configIndex))
			return;

		string selectedConfig = m_UserConfigs.Get(configIndex);
		keybindModule.SelectJoystickPresetPath(selectedConfig);
		RefreshSpinBoxArrows(component, index, m_UserConfigs.Count() + 1);
		GetGame().GetCallqueue().CallLater(RefreshAllSpinBoxArrows, 0, false);
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

		if (m_HudPositionPreview)
		{
			m_HudDragHandler = new HOTASHudPositionDragHandler(this);
			m_HudPositionPreview.AddHandler(m_HudDragHandler);
		}
	}

	protected void SyncHudPositionPreviewFromController()
	{
		HOTASDebugController.GetInstance().GetHudPositionNormalized(m_fPreviewPositionX, m_fPreviewPositionY);
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
		float hudScale = 0.6 + (controller.GetHudScalePercent() / 100.0) * 1.4;

		float hudWidth = 700 * hudScale;
		float hudHeight = 70 * hudScale;
		float travelX = Math.Max(0.0, screenWidth - hudWidth);
		float travelY = Math.Max(0.0, screenHeight - hudHeight);
		float hudLeft = travelX * m_fPreviewPositionX;
		float hudTop = travelY * m_fPreviewPositionY;

		float previewHudWidth = previewWidth * (hudWidth / screenWidth);
		float previewHudHeight = previewHeight * (hudHeight / screenHeight);
		float previewHudLeft = previewWidth * (hudLeft / screenWidth);
		float previewHudTop = previewHeight * (hudTop / screenHeight);

		// Keep the preview handle large enough to grab reliably with the mouse.
		if (previewHudWidth < 20)
			previewHudWidth = 20;
		if (previewHudHeight < 12)
			previewHudHeight = 12;

		FrameSlot.SetPos(m_HudPositionPreview, previewHudLeft, previewHudTop);
		FrameSlot.SetSize(m_HudPositionPreview, previewHudWidth, previewHudHeight);

		if (controller.GetSettingOptionIndex(0) == 0)
			m_HudPositionPreview.SetOpacity(0.3);
		else
			m_HudPositionPreview.SetOpacity(0.9);
	}


	//------------------------------------------------------------------------------------------------
	void BeginHudPositionDrag()
	{
		if (!m_HudPositionPreview || !m_ScreenPreview)
			return;

		int mouseX;
		int mouseY;
		WidgetManager.GetMousePos(mouseX, mouseY);

		float boxX;
		float boxY;
		m_HudPositionPreview.GetScreenPos(boxX, boxY);
		m_fHudDragOffsetX = mouseX - boxX;
		m_fHudDragOffsetY = mouseY - boxY;

		m_bDraggingHudPosition = true;
		GetGame().GetCallqueue().Remove(UpdateHudPositionDrag);
		GetGame().GetCallqueue().CallLater(UpdateHudPositionDrag, 16, true);
		UpdateHudPositionDrag();
	}

	//------------------------------------------------------------------------------------------------
	void EndHudPositionDrag()
	{
		if (!m_bDraggingHudPosition)
			return;

		m_bDraggingHudPosition = false;
		GetGame().GetCallqueue().Remove(UpdateHudPositionDrag);
		HOTASDebugController.GetInstance().SetHudPositionNormalized(m_fPreviewPositionX, m_fPreviewPositionY);
		UpdateHudPositionPreview();
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateHudPositionDrag()
	{
		if (!m_bDraggingHudPosition || !m_HudPositionPreview || !m_ScreenPreview)
			return;

		int mouseX;
		int mouseY;
		WidgetManager.GetMousePos(mouseX, mouseY);

		float screenX;
		float screenY;
		float screenWidth;
		float screenHeight;
		float boxWidth;
		float boxHeight;
		m_ScreenPreview.GetScreenPos(screenX, screenY);
		m_ScreenPreview.GetScreenSize(screenWidth, screenHeight);
		m_HudPositionPreview.GetScreenSize(boxWidth, boxHeight);

		float travelX = screenWidth - boxWidth;
		float travelY = screenHeight - boxHeight;
		if (travelX <= 0 || travelY <= 0)
			return;

		m_fPreviewPositionX = Math.Clamp((mouseX - screenX - m_fHudDragOffsetX) / travelX, 0.0, 1.0);
		m_fPreviewPositionY = Math.Clamp((mouseY - screenY - m_fHudDragOffsetY) / travelY, 0.0, 1.0);
		UpdateHudPositionPreview();
	}

	//------------------------------------------------------------------------------------------------
	protected void RefreshAllSpinBoxArrows()
	{
		if (m_HotasConfig)
			RefreshSpinBoxArrows(m_HotasConfig, m_HotasConfig.GetCurrentIndex(), m_UserConfigs.Count() + 1);

		HOTASDebugController controller = HOTASDebugController.GetInstance();
		for (int i = 0; i < m_HudControls.Count(); i++)
		{
			SCR_SpinBoxComponent control = m_HudControls[i];
			if (!control)
				continue;
			RefreshSpinBoxArrows(control, control.GetCurrentIndex(), controller.GetSettingOptionCount(i));
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void RefreshSpinBoxArrows(SCR_SpinBoxComponent control, int selected, int optionCount)
	{
		if (!control || optionCount <= 0)
			return;

		Widget root = control.GetRootWidget();
		if (!root)
			return;

		// These selectors cycle, so both arrows stay visible whenever more than one option exists.
		bool arrowsEnabled = optionCount > 1;
		RefreshArrowButton(root.FindAnyWidget("ButtonLeft"), arrowsEnabled);
		RefreshArrowButton(root.FindAnyWidget("ButtonRight"), arrowsEnabled);
	}

	//------------------------------------------------------------------------------------------------
	protected void RefreshArrowButton(Widget buttonWidget, bool enabled)
	{
		if (!buttonWidget)
			return;

		SCR_PagingButtonComponent button = SCR_PagingButtonComponent.Cast(buttonWidget.FindHandler(SCR_PagingButtonComponent));
		if (button)
		{
			button.SetDisabledOpacity(0.35);
			button.SetEnabled(enabled, false);
		}
		else
		{
			buttonWidget.SetEnabled(enabled);
		}

		// Paging buttons hide their background when disabled. Force both visual layers
		// back on after SetEnabled so the arrows never disappear from the settings row.
		if (enabled)
			buttonWidget.SetOpacity(1.0);
		else
			buttonWidget.SetOpacity(0.35);

		Widget background = buttonWidget.FindAnyWidget("BackgroundImage");
		if (background)
		{
			background.SetVisible(true);
			background.SetOpacity(1.0);
		}

		Widget panel = buttonWidget.FindAnyWidget("Panel");
		if (panel)
		{
			panel.SetVisible(true);
			panel.SetOpacity(1.0);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void SetupHudControls()
	{
		m_HudControls.Clear();

		array<string> widgetNames = {
			"HUDEnabled",
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

			int currentIndex = controller.GetSettingOptionIndex(i);
			control.SetCurrentItem(currentIndex, false, false, false);
			RefreshSpinBoxArrows(control, currentIndex, optionCount);
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
	protected void SyncHudSliders()
	{
		HOTASDebugController controller = HOTASDebugController.GetInstance();
		if (m_HudScaleSlider)
			m_HudScaleSlider.SetValue(controller.GetHudScalePercent());
		if (m_BackgroundOpacitySlider)
			m_BackgroundOpacitySlider.SetValue(controller.GetBackgroundOpacityPercent());
	}

	//------------------------------------------------------------------------------------------------
	protected void SyncHudControls()
	{
		HOTASDebugController controller = HOTASDebugController.GetInstance();
		for (int i = 0; i < m_HudControls.Count(); i++)
		{
			SCR_SpinBoxComponent control = m_HudControls[i];
			if (control)
			{
				int currentIndex = controller.GetSettingOptionIndex(i);
				control.SetCurrentItem(currentIndex, false, false, false);
				RefreshSpinBoxArrows(control, currentIndex, controller.GetSettingOptionCount(i));
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnHudScaleChanged(SCR_SliderComponent component, float value)
	{
		if (m_bLoading)
			return;

		HOTASDebugController.GetInstance().SetHudScalePercent(value);
		UpdateHudPositionPreview();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnBackgroundOpacityChanged(SCR_SliderComponent component, float value)
	{
		if (m_bLoading)
			return;

		HOTASDebugController.GetInstance().SetBackgroundOpacityPercent(value);
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

			HOTASDebugController controller = HOTASDebugController.GetInstance();
			controller.SetSettingOptionIndex(i, optionIndex);
			RefreshSpinBoxArrows(component, optionIndex, controller.GetSettingOptionCount(i));
			GetGame().GetCallqueue().CallLater(RefreshAllSpinBoxArrows, 0, false);
			UpdateHudPositionPreview();
			return;
		}
	}
}


//------------------------------------------------------------------------------------------------
// Mouse handler for the orange HUD preview bar. Keeping this separate prevents the
// settings submenu's root handler from being rebound when the preview widget is hooked.
class HOTASHudPositionDragHandler : ScriptedWidgetEventHandler
{
	protected HOTASSettingsSubMenu m_Owner;

	void HOTASHudPositionDragHandler(HOTASSettingsSubMenu owner)
	{
		m_Owner = owner;
	}

	override bool OnMouseButtonDown(Widget w, int x, int y, int button)
	{
		if (button != 0 || !m_Owner)
			return false;

		m_Owner.BeginHudPositionDrag();
		return true;
	}

	override bool OnMouseButtonUp(Widget w, int x, int y, int button)
	{
		if (button != 0 || !m_Owner)
			return false;

		m_Owner.EndHudPositionDrag();
		return true;
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
