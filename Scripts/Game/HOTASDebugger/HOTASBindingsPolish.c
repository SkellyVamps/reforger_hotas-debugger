//------------------------------------------------------------------------------------------------
// UI polish and interaction fixes for the test-branch HOTAS binding editor.
// Keeps the import picker as a real drop-down, removes obsolete right-side binding buttons,
// provides wrapped status text, and makes capture cancellation explicit/reliable.
class HOTASCaptureCancelClickHandler : ScriptedWidgetEventHandler
{
	protected HOTASBindingsSubMenu m_Owner;

	void HOTASCaptureCancelClickHandler(HOTASBindingsSubMenu owner)
	{
		m_Owner = owner;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (button != 0 || !m_Owner)
			return false;

		m_Owner.CancelCaptureFromUI();
		return true;
	}
}

//------------------------------------------------------------------------------------------------
modded class HOTASBindingsSubMenu
{
	protected SCR_ComboBoxComponent m_ImportDropdown;
	protected RichTextWidget m_EditorStatusText;
	protected RichTextWidget m_ManagedStatusText;
	protected ref HOTASCaptureCancelClickHandler m_CancelClickHandler;
	protected bool m_bPolishInitialized;

	//------------------------------------------------------------------------------------------------
	override void OnTabCreate(Widget menuRoot, ResourceName buttonsLayout, int index)
	{
		super.OnTabCreate(menuRoot, buttonsLayout, index);
		SetupPolishedBindingControls();
	}

	//------------------------------------------------------------------------------------------------
	override void OnTabShow()
	{
		super.OnTabShow();
		SetupPolishedBindingControls();
		RefreshImportSelector();
		RefreshManagedStatus();
	}

	//------------------------------------------------------------------------------------------------
	override protected void ResolveWidgets()
	{
		// The native-style page no longer has the old action spinbox or right-side Bind/Clear buttons.
		// Resolve only widgets that still exist so the settings log stays clean.
		m_ImportSelector = null;
		m_ActionSelector = null;
		m_ManagedStatusLabel = null;
		m_CurrentBindingLabel = null;
		m_EditorStatusLabel = null;

		m_ImportButton = SCR_ButtonTextComponent.GetButtonText("ImportSelected", m_wRoot);
		m_ActivateButton = SCR_ButtonTextComponent.GetButtonText("ActivateManaged", m_wRoot);
		m_ResetButton = SCR_ButtonTextComponent.GetButtonText("ResetManaged", m_wRoot);
		m_BindButton = null;
		m_ClearButton = null;
		m_CancelCaptureButton = SCR_ButtonTextComponent.GetButtonText("CancelCapture", m_wRoot);

		if (m_ImportButton)
			m_ImportButton.m_OnClicked.Insert(OnImportSelected);
		if (m_ActivateButton)
			m_ActivateButton.m_OnClicked.Insert(OnActivateManaged);
		if (m_ResetButton)
			m_ResetButton.m_OnClicked.Insert(OnResetManaged);
		if (m_CancelCaptureButton)
			m_CancelCaptureButton.m_OnClicked.Insert(OnCancelCapture);

		SetCaptureControls(false);
	}

	//------------------------------------------------------------------------------------------------
	protected void HideImportDropdownLabel()
	{
		if (!m_ImportDropdown)
			return;

		// Do not call UseLabel(false) after the combo has initialized. The vanilla label contains an
		// SCR_AutomaticScrollComponent; removing that hierarchy leaves its focus callback with stale
		// widget references and can throw a VM null-pointer exception. Keep it alive, clear it, and
		// hide it instead.
		m_ImportDropdown.SetLabel(string.Empty);
		Widget labelWidget = m_ImportDropdown.GetLabelWidget();
		if (labelWidget)
		{
			labelWidget.SetVisible(false);
			labelWidget.SetEnabled(false);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void SetupPolishedBindingControls()
	{
		m_ImportDropdown = SCR_ComboBoxComponent.GetComboBoxComponent("ImportConfig", m_wRoot);
		HideImportDropdownLabel();

		m_EditorStatusText = RichTextWidget.Cast(m_wRoot.FindAnyWidget("EditorStatusText"));
		m_ManagedStatusText = RichTextWidget.Cast(m_wRoot.FindAnyWidget("ManagedConfigStatusText"));

		if (!m_bPolishInitialized)
		{
			if (m_ImportDropdown)
				m_ImportDropdown.m_OnChanged.Insert(OnImportDropdownChanged);

			Widget cancelWidget = m_wRoot.FindAnyWidget("CancelCapture");
			if (cancelWidget)
			{
				m_CancelClickHandler = new HOTASCaptureCancelClickHandler(this);
				cancelWidget.AddHandler(m_CancelClickHandler);
			}

			m_bPolishInitialized = true;
		}

		SetCaptureControls(m_bCapturing);
	}

	//------------------------------------------------------------------------------------------------
	override protected void SetEditorStatus(string text)
	{
		if (!m_EditorStatusText && m_wRoot)
			m_EditorStatusText = RichTextWidget.Cast(m_wRoot.FindAnyWidget("EditorStatusText"));

		if (m_EditorStatusText)
			m_EditorStatusText.SetText(text);
	}

	//------------------------------------------------------------------------------------------------
	override protected void RefreshManagedStatus()
	{
		if (!m_ManagedStatusText && m_wRoot)
			m_ManagedStatusText = RichTextWidget.Cast(m_wRoot.FindAnyWidget("ManagedConfigStatusText"));

		if (!m_ManagedStatusText)
			return;

		string statusText = "AVAILABLE";
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

		m_ManagedStatusText.SetText(string.Format("HOTAS_Config.conf (%1)", statusText));
	}

	//------------------------------------------------------------------------------------------------
	override protected void RefreshImportSelector()
	{
		m_ImportDropdown = SCR_ComboBoxComponent.GetComboBoxComponent("ImportConfig", m_wRoot);
		if (!m_ImportDropdown)
			return;

		HideImportDropdownLabel();
		m_ImportDropdown.ClearAll();
		m_ImportConfigs.Clear();
		m_ImportDropdown.AddItem("Select read-only config...");

		array<string> configs = {};
		FileIO.FindFiles(configs.Insert, CONFIG_DIRECTORY, ".conf");
		foreach (string config : configs)
		{
			if (FilePath.StripPath(config) == MANAGED_CONFIG_NAME)
				continue;

			m_ImportConfigs.Insert(config);
			m_ImportDropdown.AddItem(FilePath.StripPath(config));
		}

		m_ImportDropdown.SetCurrentItem(0, false, false, false);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnImportDropdownChanged(SCR_ComboBoxComponent component, int index)
	{
		if (index <= 0)
		{
			SetEditorStatus("Select an external config to import. The source file remains read-only.");
			return;
		}

		int configIndex = index - 1;
		if (!m_ImportConfigs.IsIndexValid(configIndex))
			return;

		SetEditorStatus(string.Format("Ready to import %1 into HOTAS_Config.conf.", FilePath.StripPath(m_ImportConfigs[configIndex])));
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnImportSelected()
	{
		if (!m_ImportDropdown)
			m_ImportDropdown = SCR_ComboBoxComponent.GetComboBoxComponent("ImportConfig", m_wRoot);

		if (!m_ImportDropdown)
		{
			SetEditorStatus("Import drop-down is unavailable.");
			return;
		}

		int importIndex = m_ImportDropdown.GetCurrentIndex() - 1;
		if (!m_ImportConfigs.IsIndexValid(importIndex))
		{
			SetEditorStatus("Choose an external .conf file from the drop-down first.");
			return;
		}

		string sourcePath = m_ImportConfigs[importIndex];
		ClearBindings();
		int importedCount = LoadConfigIntoBindings(sourcePath, false);
		WriteManagedConfig();
		ActivateManagedConfig();
		RefreshCurrentBinding();
		RefreshManagedStatus();
		SetEditorStatus(string.Format("Imported %1 supported bindings from %2. Source file was not modified.", importedCount, FilePath.StripPath(sourcePath)));
	}

	//------------------------------------------------------------------------------------------------
	override protected void SetCaptureControls(bool capturing)
	{
		m_bCapturing = capturing;

		if (m_ImportButton)
			m_ImportButton.GetRootWidget().SetEnabled(!capturing);
		if (m_ResetButton)
			m_ResetButton.GetRootWidget().SetEnabled(!capturing);
		if (m_ActivateButton)
			m_ActivateButton.GetRootWidget().SetEnabled(!capturing);

		if (m_ImportDropdown)
			m_ImportDropdown.GetRootWidget().SetEnabled(!capturing);

		if (m_CancelCaptureButton)
		{
			Widget cancelRoot = m_CancelCaptureButton.GetRootWidget();
			cancelRoot.SetVisible(capturing);
			cancelRoot.SetEnabled(capturing);
			cancelRoot.SetOpacity(1.0);
		}

		InputManager inputManager = GetGame().GetInputManager();
		if (!inputManager)
			return;

		inputManager.RemoveActionListener(UIConstants.MENU_ACTION_BACK, EActionTrigger.DOWN, OnCaptureBack);
#ifdef WORKBENCH
		inputManager.RemoveActionListener(UIConstants.MENU_ACTION_BACK_WB, EActionTrigger.DOWN, OnCaptureBack);
#endif

		if (!capturing)
			return;

		inputManager.AddActionListener(UIConstants.MENU_ACTION_BACK, EActionTrigger.DOWN, OnCaptureBack);
#ifdef WORKBENCH
		inputManager.AddActionListener(UIConstants.MENU_ACTION_BACK_WB, EActionTrigger.DOWN, OnCaptureBack);
#endif
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCaptureBack()
	{
		if (!m_bCapturing)
			return;

		CancelCaptureFromUI();
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnCancelCapture()
	{
		CancelCaptureFromUI();
	}

	//------------------------------------------------------------------------------------------------
	void CancelCaptureFromUI()
	{
		if (!m_bCapturing)
			return;

		StopCapture(true);
		RefreshCurrentBinding();
		SetEditorStatus("Input capture canceled.");
		Print("[HOTAS Bindings] Input capture canceled.");
	}
}
