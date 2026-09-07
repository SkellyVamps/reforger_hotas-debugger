//------------------------------------------------------------------------------------------------
// Keep the large cockpit preview while mapping normalized HUD coordinates to the exact usable
// preview area. The normalized position represents travel of the HUD's top-left corner from
// 0..1 after accounting for the HUD's own size, matching HOTASDebugController.GetHudPosition().
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

		// This is the same coordinate model as the real HUD:
		// left = (screenWidth - hudWidth) * normalizedX
		// top  = (screenHeight - hudHeight) * normalizedY
		float travelX = Math.Max(0.0, previewWidth - hudWidth);
		float travelY = Math.Max(0.0, previewHeight - hudHeight);
		float previewLeft = travelX * Math.Clamp(m_fPreviewPositionX, 0.0, 1.0);
		float previewTop = travelY * Math.Clamp(m_fPreviewPositionY, 0.0, 1.0);

		FrameSlot.SetPos(m_HudPositionPreview, previewLeft, previewTop);

		// The visible sample is a sibling of the transparent drag target. Give it the exact
		// same local position and size instead of deriving its location from screen-space/DPI
		// coordinates. This keeps all four edges pixel-consistent with the actual HUD mapping.
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
