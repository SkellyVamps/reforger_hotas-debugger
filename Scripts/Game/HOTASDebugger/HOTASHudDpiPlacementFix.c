//------------------------------------------------------------------------------------------------
// The workspace reports physical viewport pixels, while FrameSlot uses DPI-unscaled UI units.
// Keep HUD placement/size calculations in physical pixels so the settings preview and runtime
// use the same coordinate model, then convert only the final FrameSlot values.
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

		// These are intentional physical-pixel dimensions. The Scale setting controls them;
		// Workbench/game DPI scaling must not change the effective HUD size or position.
		int widthPx = Math.Round(700 * m_fHudScale);
		int heightPx = Math.Round(70 * m_fHudScale);
		int leftPx;
		int topPx;
		GetHudPosition(workspace, widthPx, heightPx, leftPx, topPx);

		// FrameSlot coordinates are UI/reference units, not physical viewport pixels.
		// Converting here makes (0,0) the true top-left and (1,1) the true bottom-right
		// of the HUD's usable travel area at any viewport size or UI DPI scale.
		float leftUi = workspace.DPIUnscale(leftPx);
		float topUi = workspace.DPIUnscale(topPx);
		float widthUi = workspace.DPIUnscale(widthPx);
		float heightUi = workspace.DPIUnscale(heightPx);
		FrameSlot.SetPos(m_HudRootWidget, leftUi, topUi);
		FrameSlot.SetSize(m_HudRootWidget, widthUi, heightUi);

		// Font/padding are defined in the same physical-pixel model so the settings preview
		// remains representative instead of the runtime HUD becoming larger at higher DPI.
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

		Print(string.Format(
			"[HOTAS Debugger] HUD viewport=%1x%2 px position=%3,%4 px size=%5x%6 px normalized=%7/%8",
			workspace.GetWidth(), workspace.GetHeight(), leftPx, topPx, widthPx, heightPx,
			m_fHudPositionX.ToString(3), m_fHudPositionY.ToString(3)
		), LogLevel.NORMAL);
	}
}
