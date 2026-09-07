//------------------------------------------------------------------------------------------------
// Keep the cockpit preview inside one clipped viewport and hide texture-edge seams caused by
// fractional DPI scaling. The screenshot is allowed to bleed a couple of physical pixels past the
// viewport, then HUDScreenPreview clips it back to the exact visible rectangle.
modded class HOTASSettingsSubMenu
{
	//------------------------------------------------------------------------------------------------
	override protected void UpdateHudPositionPreview()
	{
		super.UpdateHudPositionPreview();
		ApplyPreviewViewportFix();
	}

	//------------------------------------------------------------------------------------------------
	protected void ApplyPreviewViewportFix()
	{
		if (!m_ScreenPreview || !m_ScreenPreviewBackground)
			return;

		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace)
			return;

		// Treat HUDScreenPreview as the single viewport for the cockpit image, visible HUD sample,
		// and invisible drag target. Anything outside this exact rectangle is clipped.
		m_ScreenPreview.SetFlags(WidgetFlags.CLIPCHILDREN);
		m_ScreenPreview.ClearFlags(WidgetFlags.INHERIT_CLIPPING);

		// The old square background was useful for the original diagram, but with a real screenshot it
		// can show through as a grey strip when DPI rounding leaves a fraction of a pixel uncovered.
		if (m_PreviewSquareBackground)
			m_PreviewSquareBackground.SetOpacity(0.0);

		float previewWidthPx;
		float previewHeightPx;
		m_ScreenPreview.GetScreenSize(previewWidthPx, previewHeightPx);
		if (previewWidthPx <= 0 || previewHeightPx <= 0)
			return;

		float previewWidth = workspace.DPIUnscale(previewWidthPx);
		float previewHeight = workspace.DPIUnscale(previewHeightPx);

		// Slightly overscan only the cockpit screenshot. The parent viewport clips the excess, which
		// prevents the texture's right/bottom sampling edge from becoming visible when stretched.
		float bleed = workspace.DPIUnscale(2.0);
		FrameSlot.SetPos(m_ScreenPreviewBackground, -bleed, -bleed);
		FrameSlot.SetSize(
			m_ScreenPreviewBackground,
			previewWidth + bleed * 2.0,
			previewHeight + bleed * 2.0
		);

		ImageWidget previewImage = ImageWidget.Cast(m_ScreenPreviewBackground);
		if (!previewImage)
			return;

		// Dynamic textures replace the placeholder image at runtime. Keep the ImageWidget's internal
		// content size synchronized with the real texture instead of the 1024x1024 placeholder size.
		int imageWidth;
		int imageHeight;
		previewImage.GetImageSize(0, imageWidth, imageHeight);
		if (imageWidth > 0 && imageHeight > 0)
			previewImage.SetSize(imageWidth, imageHeight);

		previewImage.SetColor(Color.White);
		previewImage.SetOpacity(1.0);
	}
}
