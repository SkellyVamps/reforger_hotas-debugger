//------------------------------------------------------------------------------------------------
// UI polish and interaction fixes for the test-branch HOTAS binding editor.
// Keeps the import picker as a real drop-down and makes capture cancellation explicit/reliable.
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
	}

	//------------------------------------------------------------------------------------------------
	protected void SetupPolishedBindingControls()
	{
		m_ImportDropdown = SCR_ComboBoxComponent.GetComboBoxComponent("ImportConfig", m_wRoot);

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
	override protected void RefreshImportSelector()
	{
		m_ImportDropdown = SCR_ComboBoxComponent.GetComboBoxComponent("ImportConfig", m_wRoot);
		if (!m_ImportDropdown)
			return;

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
		SetEditorStatus(string.Format("Imported %1 supported bindings from %2. Source file was not modified.", importedCount, FilePath.StripPath(sourcePath)));
	}

	//------------------------------------------------------------------------------------------------
	override protected void SetCaptureControls(bool capturing)
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
		if (m_ActivateButton)
			m_ActivateButton.GetRootWidget().SetEnabled(!capturing);

		if (m_ImportDropdown)
			m_ImportDropdown.GetRootWidget().SetEnabled(!capturing);

		if (m_CancelCaptureButton)
		{
			Widget cancelRoot = m_CancelCaptureButton.GetRootWidget();
			cancelRoot.SetVisible(true);
			cancelRoot.SetEnabled(capturing);
			if (capturing)
				cancelRoot.SetOpacity(1.0);
			else
				cancelRoot.SetOpacity(0.35);
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
		SetEditorStatus("Input capture canceled.");
	}
}
