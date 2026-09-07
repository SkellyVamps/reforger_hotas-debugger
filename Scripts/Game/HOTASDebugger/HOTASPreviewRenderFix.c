//------------------------------------------------------------------------------------------------
// Final preview rendering fixes: keep the draggable ButtonWidget as an invisible hit target,
// render the HUD sample as a sibling so it is not tinted/hidden by the button, and constrain
// the Day/Night controls to a normal settings-row height.
modded class HOTASSettingsSubMenu
{
	//------------------------------------------------------------------------------------------------
	override protected void SetupPreviewControls()
	{
		super.SetupPreviewControls();

		if (!m_PreviewExtrasRoot)
			return;

		Widget buttonRow = m_PreviewExtrasRoot.FindAnyWidget("PreviewLightingButtons");
		if (buttonRow)
			LayoutSlot.SetSizeMode(buttonRow, LayoutSizeMode.Auto);

		LimitPreviewLightingButtonHeight(m_PreviewDayButton);
		LimitPreviewLightingButtonHeight(m_PreviewNightButton);
	}

	//------------------------------------------------------------------------------------------------
	protected void LimitPreviewLightingButtonHeight(SCR_ButtonTextComponent button)
	{
		if (!button)
			return;

		SizeLayoutWidget sizeWidget = FindFirstSizeLayout(button.GetRootWidget());
		if (!sizeWidget)
			return;

		sizeWidget.EnableHeightOverride(true);
		sizeWidget.SetHeightOverride(76);
	}

	//------------------------------------------------------------------------------------------------
	protected SizeLayoutWidget FindFirstSizeLayout(Widget root)
	{
		if (!root)
			return null;

		SizeLayoutWidget sizeWidget = SizeLayoutWidget.Cast(root);
		if (sizeWidget)
			return sizeWidget;

		Widget child = root.GetChildren();
		while (child)
		{
			sizeWidget = FindFirstSizeLayout(child);
			if (sizeWidget)
				return sizeWidget;

			child = child.GetSibling();
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	override protected void SetupEnhancedHudPreview()
	{
		super.SetupEnhancedHudPreview();

		if (!m_HudPositionPreview || !m_ScreenPreview)
			return;

		// The previous implementation created the sample inside the transparent drag button.
		// Parent tinting/opacity can make all of those children disappear. Remove that instance
		// and recreate the exact same sample as a sibling inside the screen preview instead.
		if (m_PreviewHudVisualRoot)
			m_PreviewHudVisualRoot.RemoveFromHierarchy();

		m_PreviewHudVisualRoot = GetGame().GetWorkspace().CreateWidgets(
			"{B87F439E5C1A02D6}UI/layouts/HUD/HOTAS/HOTASHudPreviewSample.layout",
			m_ScreenPreview
		);
		if (!m_PreviewHudVisualRoot)
			return;

		m_PreviewHudVisualRoot.SetIsColorInherited(false);
		m_PreviewHudVisualRoot.SetZOrder(10);
		m_HudPositionPreview.SetZOrder(20);

		m_PreviewHudBackground = m_PreviewHudVisualRoot.FindAnyWidget("PreviewHudBackground");
		m_PreviewHudContent = m_PreviewHudVisualRoot.FindAnyWidget("PreviewHudContent");
		m_PreviewHudInput = RichTextWidget.Cast(m_PreviewHudVisualRoot.FindAnyWidget("PreviewHudInput"));
		m_PreviewHudSeparator = RichTextWidget.Cast(m_PreviewHudVisualRoot.FindAnyWidget("PreviewHudSeparator"));
		m_PreviewHudAction = RichTextWidget.Cast(m_PreviewHudVisualRoot.FindAnyWidget("PreviewHudAction"));

		WorkspaceWidget workspace = GetGame().GetWorkspace();
		SyncPreviewHudVisualToDragTarget(workspace);
		UpdatePreviewHudAppearance(workspace);
	}

	//------------------------------------------------------------------------------------------------
	override protected void UpdateHudPositionPreview()
	{
		super.UpdateHudPositionPreview();

		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace)
			return;

		SyncPreviewHudVisualToDragTarget(workspace);
		UpdatePreviewHudAppearance(workspace);
	}

	//------------------------------------------------------------------------------------------------
	protected void SyncPreviewHudVisualToDragTarget(WorkspaceWidget workspace)
	{
		if (!workspace || !m_PreviewHudVisualRoot || !m_HudPositionPreview || !m_ScreenPreview)
			return;

		float targetX;
		float targetY;
		float screenX;
		float screenY;
		float targetWidth;
		float targetHeight;
		m_HudPositionPreview.GetScreenPos(targetX, targetY);
		m_ScreenPreview.GetScreenPos(screenX, screenY);
		m_HudPositionPreview.GetScreenSize(targetWidth, targetHeight);

		FrameSlot.SetPos(
			m_PreviewHudVisualRoot,
			workspace.DPIUnscale(targetX - screenX),
			workspace.DPIUnscale(targetY - screenY)
		);
		FrameSlot.SetSize(
			m_PreviewHudVisualRoot,
			workspace.DPIUnscale(targetWidth),
			workspace.DPIUnscale(targetHeight)
		);

		// Keep the transparent hit target above the visual sample so dragging works anywhere
		// on the rendered HUD without the sample intercepting the mouse.
		m_PreviewHudVisualRoot.SetZOrder(10);
		m_HudPositionPreview.SetZOrder(20);
		m_HudPositionPreview.SetColor(Color.FromInt(0x00000000));
		m_HudPositionPreview.SetOpacity(1.0);
	}

	//------------------------------------------------------------------------------------------------
	override protected void UpdatePreviewHudAppearance(WorkspaceWidget workspace)
	{
		if (!workspace || !m_PreviewHudVisualRoot)
			return;

		HOTASDebugController controller = HOTASDebugController.GetInstance();

		// Match HOTASInputHUD.layout: the text remains fully opaque while only the dark
		// background uses the configured Background Opacity value.
		if (m_PreviewHudBackground)
		{
			m_PreviewHudBackground.SetColor(Color.FromInt(0xFF010202));
			if (controller.IsHudBackgroundEnabled())
				m_PreviewHudBackground.SetOpacity(controller.GetHudBackgroundOpacity());
			else
				m_PreviewHudBackground.SetOpacity(0.0);
		}

		if (m_PreviewHudInput)
			m_PreviewHudInput.SetText(controller.GetPitchForwardPreviewInput());
		if (m_PreviewHudSeparator)
			m_PreviewHudSeparator.SetText("|");
		if (m_PreviewHudAction)
			m_PreviewHudAction.SetText("Cyclic Forward");

		float previewWidthPx;
		float previewHeightPx;
		m_PreviewHudVisualRoot.GetScreenSize(previewWidthPx, previewHeightPx);
		float previewHeight = workspace.DPIUnscale(previewHeightPx);
		float previewScale = previewHeight / 70.0;
		int previewFontSize = Math.Round(26.0 * previewScale);
		if (previewFontSize < 6)
			previewFontSize = 6;

		if (m_PreviewHudInput)
			m_PreviewHudInput.SetExactFontSize(previewFontSize);
		if (m_PreviewHudSeparator)
			m_PreviewHudSeparator.SetExactFontSize(previewFontSize);
		if (m_PreviewHudAction)
			m_PreviewHudAction.SetExactFontSize(previewFontSize);

		float contentPadding = Math.Max(1.0, 15.0 * previewScale);
		float separatorPadding = Math.Max(1.0, 5.0 * previewScale);
		if (m_PreviewHudContent)
			LayoutSlot.SetPadding(m_PreviewHudContent, contentPadding, contentPadding, contentPadding, contentPadding);
		if (m_PreviewHudSeparator)
			LayoutSlot.SetPadding(m_PreviewHudSeparator, separatorPadding, 0, separatorPadding, 0);

		if (controller.IsHudEnabled())
			m_PreviewHudVisualRoot.SetOpacity(1.0);
		else
			m_PreviewHudVisualRoot.SetOpacity(0.35);

		// The hit target itself stays invisible and fully active; its sibling above is what
		// the player sees.
		m_HudPositionPreview.SetColor(Color.FromInt(0x00000000));
		m_HudPositionPreview.SetOpacity(1.0);
	}
}
