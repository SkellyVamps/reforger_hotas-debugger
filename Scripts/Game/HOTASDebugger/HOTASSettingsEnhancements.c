//------------------------------------------------------------------------------------------------
// HOTAS settings-menu extensions that do not belong in the core HUD/controller implementation.
// Keep UI-only behavior here so HOTASSettingsTab.c can remain focused on the base settings tab.
modded class HOTASSettingsSubMenu
{
	protected Widget m_ResetButtonsRoot;
	protected SCR_ButtonTextComponent m_ResetHudButton;
	protected SCR_ButtonTextComponent m_ResetLabelsButton;

	protected Widget m_PreviewExtrasRoot;
	protected SCR_ButtonTextComponent m_PreviewDayButton;
	protected SCR_ButtonTextComponent m_PreviewNightButton;
	protected RichTextWidget m_LiveInputReadable;
	protected TextWidget m_LiveInputRaw;
	protected int m_iLastLiveInputRevision = -1;

	protected ImageWidget m_ScreenPreviewImage;
	protected Widget m_PreviewHudVisualRoot;
	protected Widget m_PreviewHudBackground;
	protected RichTextWidget m_PreviewHudInput;
	protected RichTextWidget m_PreviewHudSeparator;
	protected RichTextWidget m_PreviewHudAction;
	protected bool m_bPreviewNight;
	protected ResourceName m_CurrentPreviewTexture;

	//------------------------------------------------------------------------------------------------
	override void OnTabCreate(Widget menuRoot, ResourceName buttonsLayout, int index)
	{
		super.OnTabCreate(menuRoot, buttonsLayout, index);
		SetupResetButtons();
		SetupLiveInputTester();
		SetupEnhancedHudPreview();
	}

	//------------------------------------------------------------------------------------------------
	override void OnTabShow()
	{
		super.OnTabShow();

		HOTASDebugController.GetInstance().SetLiveInputTesterActive(true);
		m_iLastLiveInputRevision = -1;
		UpdateLiveInputTester();
		GetGame().GetCallqueue().Remove(UpdateLiveInputTester);
		GetGame().GetCallqueue().CallLater(UpdateLiveInputTester, 100, true);
		UpdateHudPositionPreview();
	}

	//------------------------------------------------------------------------------------------------
	override void OnTabHide()
	{
		HOTASDebugController.GetInstance().SetLiveInputTesterActive(false);
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
	protected void SetupEnhancedHudPreview()
	{
		Widget titleWidget = m_wRoot.FindAnyWidget("TitleHUDPreview");
		if (titleWidget)
		{
			SCR_LabelComponent titleLabel = SCR_LabelComponent.Cast(titleWidget.FindHandler(SCR_LabelComponent));
			if (titleLabel)
				titleLabel.SetText("HUD Position - Drag Example HUD");
		}

		m_ScreenPreviewImage = ImageWidget.Cast(m_wRoot.FindAnyWidget("HUDScreenPreviewBackground"));
		if (m_ScreenPreviewImage)
			m_ScreenPreviewImage.SetColor(Color.White);

		Widget oldPreviewFill = m_wRoot.FindAnyWidget("HUDPositionPreviewFill");
		if (oldPreviewFill)
			oldPreviewFill.SetVisible(false);

		if (m_HudPositionPreview)
		{
			m_PreviewHudVisualRoot = GetGame().GetWorkspace().CreateWidgets(
				"{B87F439E5C1A02D6}UI/layouts/HUD/HOTAS/HOTASHudPreviewSample.layout",
				m_HudPositionPreview
			);

			if (m_PreviewHudVisualRoot)
			{
				LayoutSlot.SetHorizontalAlign(m_PreviewHudVisualRoot, LayoutHorizontalAlign.Stretch);
				LayoutSlot.SetVerticalAlign(m_PreviewHudVisualRoot, LayoutVerticalAlign.Stretch);

				m_PreviewHudBackground = m_PreviewHudVisualRoot.FindAnyWidget("PreviewHudBackground");
				m_PreviewHudInput = RichTextWidget.Cast(m_PreviewHudVisualRoot.FindAnyWidget("PreviewHudInput"));
				m_PreviewHudSeparator = RichTextWidget.Cast(m_PreviewHudVisualRoot.FindAnyWidget("PreviewHudSeparator"));
				m_PreviewHudAction = RichTextWidget.Cast(m_PreviewHudVisualRoot.FindAnyWidget("PreviewHudAction"));
			}
		}

		if (m_PreviewExtrasRoot)
		{
			m_PreviewDayButton = SCR_ButtonTextComponent.GetButtonText("PreviewDay", m_PreviewExtrasRoot);
			if (m_PreviewDayButton)
				m_PreviewDayButton.m_OnClicked.Insert(OnPreviewDayClicked);

			m_PreviewNightButton = SCR_ButtonTextComponent.GetButtonText("PreviewNight", m_PreviewExtrasRoot);
			if (m_PreviewNightButton)
				m_PreviewNightButton.m_OnClicked.Insert(OnPreviewNightClicked);
		}

		UpdatePreviewLightingButtons();
		UpdateHudPositionPreview();
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
	protected void OnPreviewDayClicked()
	{
		m_bPreviewNight = false;
		UpdatePreviewLightingButtons();
		UpdateHudPositionPreview();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPreviewNightClicked()
	{
		m_bPreviewNight = true;
		UpdatePreviewLightingButtons();
		UpdateHudPositionPreview();
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdatePreviewLightingButtons()
	{
		if (m_PreviewDayButton)
			m_PreviewDayButton.SetToggled(!m_bPreviewNight, false, false);
		if (m_PreviewNightButton)
			m_PreviewNightButton.SetToggled(m_bPreviewNight, false, false);
	}

	//------------------------------------------------------------------------------------------------
	override protected void UpdateHudPositionPreview()
	{
		super.UpdateHudPositionPreview();

		if (!m_HudPositionPreview)
			return;

		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace)
			return;

		float screenWidth = workspace.GetWidth();
		float screenHeight = workspace.GetHeight();
		if (screenWidth <= 0 || screenHeight <= 0)
			return;

		ExpandPreviewToHost(workspace, screenWidth, screenHeight);

		if (m_ScreenPreviewImage)
		{
			ResourceName previewTexture = GetBestPreviewTexture(screenWidth, screenHeight);
			if (previewTexture != m_CurrentPreviewTexture)
			{
				m_ScreenPreviewImage.LoadImageTexture(0, previewTexture);
				m_CurrentPreviewTexture = previewTexture;
			}
			m_ScreenPreviewImage.SetColor(Color.White);
			m_ScreenPreviewImage.SetOpacity(1.0);
		}

		UpdatePreviewHudAppearance(workspace);
	}

	//------------------------------------------------------------------------------------------------
	// The base settings tab intentionally used a centered square as a generic position diagram.
	// With real cockpit screenshots available, use almost the entire preview host instead. This
	// makes the reference image roughly twice as large on widescreen displays while preserving
	// the player's actual screen aspect ratio and normalized HUD position.
	protected void ExpandPreviewToHost(WorkspaceWidget workspace, float screenWidth, float screenHeight)
	{
		if (!m_PreviewHost || !m_PreviewSquare || !m_PreviewSquareBackground || !m_ScreenPreview || !m_ScreenPreviewBackground || !m_HudPositionPreview)
			return;

		float hostWidthPx;
		float hostHeightPx;
		m_PreviewHost.GetScreenSize(hostWidthPx, hostHeightPx);
		float hostWidth = workspace.DPIUnscale(hostWidthPx);
		float hostHeight = workspace.DPIUnscale(hostHeightPx);
		if (hostWidth <= 32 || hostHeight <= 32)
			return;

		float outerMargin = 6;
		float containerWidth = hostWidth - outerMargin * 2;
		float containerHeight = hostHeight - outerMargin * 2;
		FrameSlot.SetPos(m_PreviewSquare, outerMargin, outerMargin);
		FrameSlot.SetSize(m_PreviewSquare, containerWidth, containerHeight);
		FrameSlot.SetPos(m_PreviewSquareBackground, 0, 0);
		FrameSlot.SetSize(m_PreviewSquareBackground, containerWidth, containerHeight);

		float inset = 8;
		float availableWidth = containerWidth - inset * 2;
		float availableHeight = containerHeight - inset * 2;
		if (availableWidth <= 1 || availableHeight <= 1)
			return;

		float screenAspect = screenWidth / screenHeight;
		float previewWidth = availableWidth;
		float previewHeight = previewWidth / screenAspect;
		if (previewHeight > availableHeight)
		{
			previewHeight = availableHeight;
			previewWidth = previewHeight * screenAspect;
		}

		float screenLeft = (containerWidth - previewWidth) * 0.5;
		float screenTop = (containerHeight - previewHeight) * 0.5;
		FrameSlot.SetPos(m_ScreenPreview, screenLeft, screenTop);
		FrameSlot.SetSize(m_ScreenPreview, previewWidth, previewHeight);
		FrameSlot.SetPos(m_ScreenPreviewBackground, 0, 0);
		FrameSlot.SetSize(m_ScreenPreviewBackground, previewWidth, previewHeight);

		HOTASDebugController controller = HOTASDebugController.GetInstance();
		float hudScale = controller.GetHudScaleMultiplier();
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

		if (previewHudWidth < 20)
			previewHudWidth = 20;
		if (previewHudHeight < 12)
			previewHudHeight = 12;

		FrameSlot.SetPos(m_HudPositionPreview, previewHudLeft, previewHudTop);
		FrameSlot.SetSize(m_HudPositionPreview, previewHudWidth, previewHudHeight);
	}

	//------------------------------------------------------------------------------------------------
	protected ResourceName GetBestPreviewTexture(float screenWidth, float screenHeight)
	{
		int selectedReference = 0;
		float bestScore = GetPreviewResolutionScore(screenWidth, screenHeight, 1920.0, 1080.0);

		float score = GetPreviewResolutionScore(screenWidth, screenHeight, 2560.0, 1080.0);
		if (score < bestScore)
		{
			bestScore = score;
			selectedReference = 1;
		}

		score = GetPreviewResolutionScore(screenWidth, screenHeight, 3440.0, 1440.0);
		if (score < bestScore)
			selectedReference = 2;

		if (selectedReference == 0)
		{
			if (m_bPreviewNight)
				return "{2FB1972DA5654A69}UI/Textures/HOTASPreview/HUDPreview_1920x1080-night_UI.edds";
			return "{595F20E54C4F8D61}UI/Textures/HOTASPreview/HUDPreview_1920x1080-day_UI.edds";
		}

		if (selectedReference == 1)
		{
			if (m_bPreviewNight)
				return "{D569406EAADAA5B7}UI/Textures/HOTASPreview/HUDPreview_2560x1080-night_UI.edds";
			return "{424C5CD12385F91A}UI/Textures/HOTASPreview/HUDPreview_2560x1080-day_UI.edds";
		}

		if (m_bPreviewNight)
			return "{940895E72C9B439D}UI/Textures/HOTASPreview/HUDPreview_3440x1440-night_UI.edds";
		return "{0322A239C30882A6}UI/Textures/HOTASPreview/HUDPreview_3440x1440-day_UI.edds";
	}

	//------------------------------------------------------------------------------------------------
	protected float GetPreviewResolutionScore(float screenWidth, float screenHeight, float referenceWidth, float referenceHeight)
	{
		float screenAspect = screenWidth / screenHeight;
		float referenceAspect = referenceWidth / referenceHeight;
		float aspectScore = Math.AbsFloat(screenAspect - referenceAspect) * 100.0;
		float widthScore = Math.AbsFloat(screenWidth - referenceWidth) / Math.Max(screenWidth, 1.0);
		float heightScore = Math.AbsFloat(screenHeight - referenceHeight) / Math.Max(screenHeight, 1.0);
		return aspectScore + widthScore + heightScore;
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdatePreviewHudAppearance(WorkspaceWidget workspace)
	{
		HOTASDebugController controller = HOTASDebugController.GetInstance();

		if (m_PreviewHudBackground)
		{
			if (controller.IsHudBackgroundEnabled())
				m_PreviewHudBackground.SetOpacity(controller.GetHudBackgroundOpacity());
			else
				m_PreviewHudBackground.SetOpacity(0.0);
		}

		string inputLabel = controller.GetAxisCustomLabel(0);
		if (inputLabel.IsEmpty())
			inputLabel = "Roll";

		if (m_PreviewHudInput)
			m_PreviewHudInput.SetText(inputLabel + " +");
		if (m_PreviewHudSeparator)
			m_PreviewHudSeparator.SetText("|");
		if (m_PreviewHudAction)
			m_PreviewHudAction.SetText("Cyclic Right");

		float previewWidthPx;
		float previewHeightPx;
		m_HudPositionPreview.GetScreenSize(previewWidthPx, previewHeightPx);
		float previewHeight = workspace.DPIUnscale(previewHeightPx);
		int previewFontSize = Math.Round(previewHeight * (26.0 / 70.0));
		if (previewFontSize < 6)
			previewFontSize = 6;

		if (m_PreviewHudInput)
			m_PreviewHudInput.SetExactFontSize(previewFontSize);
		if (m_PreviewHudSeparator)
			m_PreviewHudSeparator.SetExactFontSize(previewFontSize);
		if (m_PreviewHudAction)
			m_PreviewHudAction.SetExactFontSize(previewFontSize);

		if (controller.IsHudEnabled())
			m_HudPositionPreview.SetOpacity(1.0);
		else
			m_HudPositionPreview.SetOpacity(0.35);
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
