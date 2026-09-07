//------------------------------------------------------------------------------------------------
// HOTAS settings-menu extensions that do not belong in the core HUD/controller implementation.
// Keep UI-only behavior here so HOTASSettingsTab.c can remain focused on the base settings tab.
modded class HOTASSettingsSubMenu
{
	protected Widget m_ResetButtonsRoot;
	protected SCR_ButtonTextComponent m_ResetHudButton;
	protected SCR_ButtonTextComponent m_ResetLabelsButton;

	//------------------------------------------------------------------------------------------------
	override void OnTabCreate(Widget menuRoot, ResourceName buttonsLayout, int index)
	{
		super.OnTabCreate(menuRoot, buttonsLayout, index);
		SetupResetButtons();
	}

	// Persist editable labels as the text changes. OnConfirm remains registered by the
	// base tab as a fallback for keyboard/controller confirmation.
	override protected void SetupAxisLabelEditors()
	{
		super.SetupAxisLabelEditors();

		foreach (SCR_EditBoxComponent editor : m_AxisLabelEditors)
		{
			if (editor)
				editor.m_OnChanged.Insert(OnAxisLabelConfirmed);
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void SetupFreelookLabelEditors()
	{
		super.SetupFreelookLabelEditors();

		foreach (SCR_EditBoxComponent editor : m_FreelookLabelEditors)
		{
			if (editor)
				editor.m_OnChanged.Insert(OnFreelookLabelConfirmed);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void SetupResetButtons()
	{
		Widget content = m_wRoot.FindAnyWidget("Content");
		if (!content)
			return;

		m_ResetButtonsRoot = GetGame().GetWorkspace().CreateWidgets(
			"{32A5DE9BF3C4DA65}UI/layouts/Menus/SettingsSubMenus/HOTASResetButtons.layout",
			content
		);
		if (!m_ResetButtonsRoot)
			return;

		m_ResetHudButton = SCR_ButtonTextComponent.GetButtonText("ResetHudSettings", m_ResetButtonsRoot);
		if (m_ResetHudButton)
			m_ResetHudButton.m_OnClicked.Insert(OnResetHudSettings);

		m_ResetLabelsButton = SCR_ButtonTextComponent.GetButtonText("ResetLabels", m_ResetButtonsRoot);
		if (m_ResetLabelsButton)
			m_ResetLabelsButton.m_OnClicked.Insert(OnResetLabels);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnResetHudSettings()
	{
		HOTASDebugController.GetInstance().ResetHudPresentationSettings();

		m_bLoading = true;
		SyncHudControls();
		SyncHudSliders();
		SyncHudPositionPreviewFromController();
		m_bLoading = false;

		RefreshAllSpinBoxArrows();
		UpdateHudPositionPreview();
		Print("[HOTAS Debugger] HUD presentation settings reset to defaults", LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnResetLabels()
	{
		HOTASDebugController.GetInstance().ResetCustomLabels();

		m_bLoading = true;
		SyncAxisLabelEditors();
		SyncFreelookLabelEditors();
		m_bLoading = false;

		UpdateHudPositionPreview();
		Print("[HOTAS Debugger] Custom HUD labels reset to defaults", LogLevel.NORMAL);
	}
}
