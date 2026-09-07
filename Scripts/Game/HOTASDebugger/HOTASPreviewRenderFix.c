//------------------------------------------------------------------------------------------------
// Final preview rendering fixes. Only override methods that exist on the original
// HOTASSettingsSubMenu class; helper methods added by another modded class cannot be
// overridden directly in Enfusion.
modded class HOTASSettingsSubMenu
{
	//------------------------------------------------------------------------------------------------
	override void OnTabCreate(Widget menuRoot, ResourceName buttonsLayout, int index)
	{
		super.OnTabCreate(menuRoot, buttonsLayout, index);
		CreateSiblingHudPreview();
	}

	//------------------------------------------------------------------------------------------------
	override protected void UpdateHudPositionPreview()
	{
		super.UpdateHudPositionPreview();

		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace)
			return;

		SyncSiblingHudPreview(workspace);
		ApplySiblingHudPreviewAppearance(workspace);
	}

	//------------------------------------------------------------------------------------------------
	protected void CreateSiblingHudPreview()
	{
		if (!m_HudPositionPreview || !m_ScreenPreview)
			return;

		// The settings enhancement initially creates the miniature HUD inside the drag
		// ButtonWidget. A transparent button also affects its children, so recreate the
		// miniature as a sibling inside HUDScreenPreview instead.
		if (m_PreviewHudVisualRoot)
			m_PreviewHudVisualRoot.RemoveFromHierarchy();

		m_PreviewHudVisualRoot = GetGame().GetWorkspace().CreateWidgets(
			"{B87F439E5C1A02D6}UI/layouts/HUD/HOTAS/HOTASHudPreviewSample.layout",
			m_ScreenPreview
		);
		if (!m_PreviewHudVisualRoot)
			return;

		m_PreviewHudVisualRoot.SetIsColorInherited(false);
		m_PreviewHudVisualRoot.SetVisible(true);
		m_PreviewHudVisualRoot.SetZOrder(10);

		m_PreviewHudBackground = m_PreviewHudVisualRoot.FindAnyWidget("PreviewHudBackground");
		m_PreviewHudContent = m_PreviewHudVisualRoot.FindAnyWidget("PreviewHudContent");
		m_PreviewHudInput = RichTextWidget.Cast(m_PreviewHudVisualRoot.FindAnyWidget("PreviewHudInput"));
		m_PreviewHudSeparator = RichTextWidget.Cast(m_PreviewHudVisualRoot.FindAnyWidget("PreviewHudSeparator"));
		m_PreviewHudAction = RichTextWidget.Cast(m_PreviewHudVisualRoot.FindAnyWidget("PreviewHudAction"));

		// Keep the existing ButtonWidget purely as the mouse/drag hit target above the
		// visible sample.
		m_HudPositionPreview.SetColor(Color.FromInt(0x00000000));
		m_HudPositionPreview.SetOpacity(1.0);
		m_HudPositionPreview.SetZOrder(20);

		WorkspaceWidget workspace = GetGame().GetWorkspace();
		SyncSiblingHudPreview(workspace);
		ApplySiblingHudPreviewAppearance(workspace);
	}

	//------------------------------------------------------------------------------------------------
	protected void SyncSiblingHudPreview(WorkspaceWidget workspace)
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

		m_PreviewHudVisualRoot.SetVisible(true);
		m_PreviewHudVisualRoot.SetZOrder(10);
		m_HudPositionPreview.SetColor(Color.FromInt(0x00000000));
		m_HudPositionPreview.SetOpacity(1.0);
		m_HudPositionPreview.SetZOrder(20);
	}

	//------------------------------------------------------------------------------------------------
	protected void ApplySiblingHudPreviewAppearance(WorkspaceWidget workspace)
	{
		if (!workspace || !m_PreviewHudVisualRoot)
			return;

		HOTASDebugController controller = HOTASDebugController.GetInstance();

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

		// Never use the drag target's opacity to represent the HUD; it remains an invisible,
		// fully active hit box while the sibling carries all visual opacity.
		m_HudPositionPreview.SetColor(Color.FromInt(0x00000000));
		m_HudPositionPreview.SetOpacity(1.0);
	}
}
