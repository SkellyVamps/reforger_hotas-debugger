//------------------------------------------------------------------------------------------------
// HOTAS settings-menu extensions that do not belong in the core HUD/controller implementation.
// Keep UI-only behavior here so HOTASSettingsTab.c can remain focused on the base settings tab.
modded class HOTASSettingsSubMenu
{
	protected Widget m_ResetButtonsRoot;
	protected SCR_ButtonTextComponent m_ResetHudButton;
	protected SCR_ButtonTextComponent m_ResetLabelsButton;

	protected Widget m_PreviewExtrasRoot;
	protected RichTextWidget m_LiveInputReadable;
	protected TextWidget m_LiveInputRaw;
	protected int m_iLastLiveInputRevision = -1;

	//------------------------------------------------------------------------------------------------
	override void OnTabCreate(Widget menuRoot, ResourceName buttonsLayout, int index)
	{
		super.OnTabCreate(menuRoot, buttonsLayout, index);
		SetupResetButtons();
		SetupLiveInputTester();
	}

	//------------------------------------------------------------------------------------------------
	override void OnTabShow()
	{
		super.OnTabShow();

		m_iLastLiveInputRevision = -1;
		UpdateLiveInputTester();
		GetGame().GetCallqueue().Remove(UpdateLiveInputTester);
		GetGame().GetCallqueue().CallLater(UpdateLiveInputTester, 100, true);
	}

	//------------------------------------------------------------------------------------------------
	override void OnTabHide()
	{
		GetGame().GetCallqueue().Remove(UpdateLiveInputTester);
		super.OnTabHide();
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
	protected void SetupLiveInputTester()
	{
		Widget previewPane = m_wRoot.FindAnyWidget("HUDPreviewPane");
		if (!previewPane)
			return;

		m_PreviewExtrasRoot = GetGame().GetWorkspace().CreateWidgets(
			"{6CA52592FD16D476}UI/layouts/Menus/SettingsSubMenus/HOTASPreviewExtras.layout",
			previewPane
		);
		if (!m_PreviewExtrasRoot)
			return;

		m_LiveInputReadable = RichTextWidget.Cast(m_PreviewExtrasRoot.FindAnyWidget("LiveInputReadable"));
		m_LiveInputRaw = TextWidget.Cast(m_PreviewExtrasRoot.FindAnyWidget("LiveInputRaw"));
		UpdateLiveInputTester();
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateLiveInputTester()
	{
		if (!m_LiveInputReadable || !m_LiveInputRaw)
			return;

		HOTASDebugController controller = HOTASDebugController.GetInstance();
		int revision = controller.GetLiveInputRevision();
		if (revision == m_iLastLiveInputRevision)
			return;

		m_iLastLiveInputRevision = revision;
		m_LiveInputReadable.SetText(controller.GetLiveInputReadable());
		m_LiveInputRaw.SetText(controller.GetLiveInputRaw());
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
