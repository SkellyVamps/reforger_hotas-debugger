//------------------------------------------------------------------------------------------------
// Runtime and settings-preview presentation logic that keeps HUD placement consistent across
// viewport sizes and DPI scales.

//------------------------------------------------------------------------------------------------
// The workspace reports physical viewport pixels, while FrameSlot uses DPI-unscaled UI units.
// Keep runtime HUD placement/size calculations in physical pixels, then convert only the final
// FrameSlot values.
modded class HOTASDebugController
{
	//------------------------------------------------------------------------------------------------
	override protected void ApplyLayoutHudPresentation()
	{
		if (!m_HudRootWidget || !m_InputText || !m_SeparatorText || !m_ActionText)
			return;

		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace)
			return;

		int widthPx = Math.Round(700 * m_fHudScale);
		int heightPx = Math.Round(70 * m_fHudScale);
		int leftPx;
		int topPx;
		GetHudPosition(workspace, widthPx, heightPx, leftPx, topPx);

		float leftUi = workspace.DPIUnscale(leftPx);
		float topUi = workspace.DPIUnscale(topPx);
		float widthUi = workspace.DPIUnscale(widthPx);
		float heightUi = workspace.DPIUnscale(heightPx);
		FrameSlot.SetPos(m_HudRootWidget, leftUi, topUi);
		FrameSlot.SetSize(m_HudRootWidget, widthUi, heightUi);

		int fontSizeUi = Math.Max(1, Math.Round(workspace.DPIUnscale(26 * m_fHudScale)));
		m_InputText.SetExactFontSize(fontSizeUi);
		m_SeparatorText.SetExactFontSize(fontSizeUi);
		m_ActionText.SetExactFontSize(fontSizeUi);

		Widget content = m_HudLayoutRoot.FindAnyWidget("Content");
		if (content)
		{
			float paddingUi = workspace.DPIUnscale(15 * m_fHudScale);
			LayoutSlot.SetPadding(content, paddingUi, paddingUi, paddingUi, paddingUi);
		}

		float separatorPaddingUi = workspace.DPIUnscale(5 * m_fHudScale);
		LayoutSlot.SetPadding(m_SeparatorText, separatorPaddingUi, 0, separatorPaddingUi, 0);

		if (m_HudBackground)
		{
			if (m_bBackgroundEnabled)
				m_HudBackground.SetOpacity(m_fBackgroundOpacity);
			else
				m_HudBackground.SetOpacity(0.0);
		}
	}
}

//------------------------------------------------------------------------------------------------
// Keep the large cockpit preview while mapping normalized HUD coordinates to the exact usable
// preview area. Normalized X/Y represent the HUD top-left corner's travel after accounting for
// the HUD's own size, matching HOTASDebugController.GetHudPosition().
modded class HOTASSettingsSubMenu
{
	//------------------------------------------------------------------------------------------------
	override protected void UpdateHudPositionPreview()
	{
		super.UpdateHudPositionPreview();

		if (!m_HudPositionPreview || !m_ScreenPreview)
			return;

		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace)
			return;

		float previewWidthPx;
		float previewHeightPx;
		float hudWidthPx;
		float hudHeightPx;
		m_ScreenPreview.GetScreenSize(previewWidthPx, previewHeightPx);
		m_HudPositionPreview.GetScreenSize(hudWidthPx, hudHeightPx);

		float previewWidth = workspace.DPIUnscale(previewWidthPx);
		float previewHeight = workspace.DPIUnscale(previewHeightPx);
		float hudWidth = workspace.DPIUnscale(hudWidthPx);
		float hudHeight = workspace.DPIUnscale(hudHeightPx);
		if (previewWidth <= 0 || previewHeight <= 0 || hudWidth <= 0 || hudHeight <= 0)
			return;

		float travelX = Math.Max(0.0, previewWidth - hudWidth);
		float travelY = Math.Max(0.0, previewHeight - hudHeight);
		float previewLeft = travelX * Math.Clamp(m_fPreviewPositionX, 0.0, 1.0);
		float previewTop = travelY * Math.Clamp(m_fPreviewPositionY, 0.0, 1.0);

		FrameSlot.SetPos(m_HudPositionPreview, previewLeft, previewTop);

		Widget visual = m_ScreenPreview.FindAnyWidget("HUDPreviewVisual");
		if (visual)
		{
			FrameSlot.SetPos(visual, previewLeft, previewTop);
			FrameSlot.SetSize(visual, hudWidth, hudHeight);
			visual.SetZOrder(10);
		}

		m_HudPositionPreview.SetZOrder(20);
		m_HudPositionPreview.SetColor(Color.FromInt(0x00000000));
		m_HudPositionPreview.SetOpacity(1.0);
	}
}
