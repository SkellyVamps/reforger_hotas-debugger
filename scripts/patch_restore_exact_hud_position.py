from pathlib import Path


def replace_once(text: str, old: str, new: str, label: str) -> str:
    if old not in text:
        raise RuntimeError(f"Missing expected block: {label}")
    return text.replace(old, new, 1)


# -----------------------------------------------------------------------------
# HOTASDebugger.c — restore normalized exact positioning while preserving the
# newer axis-label/config-selection work.
# -----------------------------------------------------------------------------
controller_path = Path("Scripts/Game/HOTASDebugger/HOTASDebugger.c")
controller = controller_path.read_text(encoding="utf-8")

controller = replace_once(
    controller,
    '\tprotected string m_sHudPosition = "bottom_center";\n',
    '\t// Normalized top-left travel position. 0 = left/top edge, 1 = right/bottom edge.\n'
    '\t// The available travel range keeps the HUD fully on-screen at any resolution/scale.\n'
    '\tprotected float m_fHudPositionX = 0.5;\n'
    '\tprotected float m_fHudPositionY = 0.95;\n',
    "HUD position fields",
)

old_get_position = '''\tprotected void GetHudPosition(WorkspaceWidget workspace, int width, int height, out int left, out int top)\n\t{\n\t\tint marginX = Math.Round(48 * m_fHudScale);\n\t\tint marginY = Math.Round(54 * m_fHudScale);\n\t\tint screenWidth = workspace.GetWidth();\n\t\tint screenHeight = workspace.GetHeight();\n\n\t\tleft = (screenWidth - width) / 2;\n\t\ttop = screenHeight - height - marginY;\n\n\t\tif (m_sHudPosition == "top_left")\n\t\t{\n\t\t\tleft = marginX;\n\t\t\ttop = marginY;\n\t\t}\n\t\telse if (m_sHudPosition == "top_center")\n\t\t{\n\t\t\tleft = (screenWidth - width) / 2;\n\t\t\ttop = marginY;\n\t\t}\n\t\telse if (m_sHudPosition == "top_right")\n\t\t{\n\t\t\tleft = screenWidth - width - marginX;\n\t\t\ttop = marginY;\n\t\t}\n\t\telse if (m_sHudPosition == "center_left")\n\t\t{\n\t\t\tleft = marginX;\n\t\t\ttop = (screenHeight - height) / 2;\n\t\t}\n\t\telse if (m_sHudPosition == "center")\n\t\t{\n\t\t\tleft = (screenWidth - width) / 2;\n\t\t\ttop = (screenHeight - height) / 2;\n\t\t}\n\t\telse if (m_sHudPosition == "center_right")\n\t\t{\n\t\t\tleft = screenWidth - width - marginX;\n\t\t\ttop = (screenHeight - height) / 2;\n\t\t}\n\t\telse if (m_sHudPosition == "bottom_left")\n\t\t{\n\t\t\tleft = marginX;\n\t\t\ttop = screenHeight - height - marginY;\n\t\t}\n\t\telse if (m_sHudPosition == "bottom_right")\n\t\t{\n\t\t\tleft = screenWidth - width - marginX;\n\t\t\ttop = screenHeight - height - marginY;\n\t\t}\n\t}\n'''
new_get_position = '''\tprotected void GetHudPosition(WorkspaceWidget workspace, int width, int height, out int left, out int top)\n\t{\n\t\tint screenWidth = workspace.GetWidth();\n\t\tint screenHeight = workspace.GetHeight();\n\t\tint travelX = Math.Max(0, screenWidth - width);\n\t\tint travelY = Math.Max(0, screenHeight - height);\n\n\t\tleft = Math.Round(travelX * m_fHudPositionX);\n\t\ttop = Math.Round(travelY * m_fHudPositionY);\n\t}\n'''
controller = replace_once(controller, old_get_position, new_get_position, "GetHudPosition")

controller = replace_once(
    controller,
    '\t\t\t\tdefaults.WriteLine("# position: top_left, top_center, top_right, center_left, center, center_right, bottom_left, bottom_center, bottom_right");\n\t\t\t\tdefaults.WriteLine("position=bottom_center");\n',
    '\t\t\t\tdefaults.WriteLine("# Normalized exact HUD placement inside the usable screen area.");\n'
    '\t\t\t\tdefaults.WriteLine("position_x=0.5");\n'
    '\t\t\t\tdefaults.WriteLine("position_y=0.95");\n',
    "default position settings",
)

controller = replace_once(
    controller,
    '\t\tstring line;\n\t\twhile (file.ReadLine(line) >= 0)\n',
    '\t\tstring line;\n'
    '\t\tstring legacyPosition;\n'
    '\t\tbool loadedPositionX;\n'
    '\t\tbool loadedPositionY;\n'
    '\t\twhile (file.ReadLine(line) >= 0)\n',
    "position migration locals",
)

controller = replace_once(
    controller,
    '\t\t\telse if (key == "position")\n\t\t\t\tm_sHudPosition = value;\n',
    '\t\t\telse if (key == "position")\n'
    '\t\t\t\tlegacyPosition = value;\n'
    '\t\t\telse if (key == "position_x")\n'
    '\t\t\t{\n'
    '\t\t\t\tm_fHudPositionX = Math.Clamp(value.ToFloat(0.5), 0.0, 1.0);\n'
    '\t\t\t\tloadedPositionX = true;\n'
    '\t\t\t}\n'
    '\t\t\telse if (key == "position_y")\n'
    '\t\t\t{\n'
    '\t\t\t\tm_fHudPositionY = Math.Clamp(value.ToFloat(0.95), 0.0, 1.0);\n'
    '\t\t\t\tloadedPositionY = true;\n'
    '\t\t\t}\n',
    "position settings parser",
)

old_after_load = '''\t\tfile.Close();\n\n\t\tPrint(string.Format("[HOTAS Debugger] HUD settings: position=%1 scale=%2 fade=%3/%4 background=%5 opacity=%6", m_sHudPosition, m_fHudScale, m_iFadeDelayMs, m_iFadeDurationMs, m_bBackgroundEnabled, m_fBackgroundOpacity), LogLevel.NORMAL);\n'''
new_after_load = '''\t\tfile.Close();\n\n\t\tif (!loadedPositionX || !loadedPositionY)\n\t\t{\n\t\t\tfloat legacyX;\n\t\t\tfloat legacyY;\n\t\t\tResolveLegacyHudPosition(legacyPosition, legacyX, legacyY);\n\t\t\tif (!loadedPositionX)\n\t\t\t\tm_fHudPositionX = legacyX;\n\t\t\tif (!loadedPositionY)\n\t\t\t\tm_fHudPositionY = legacyY;\n\t\t}\n\n\t\tPrint(string.Format("[HOTAS Debugger] HUD settings: position=%1/%2 scale=%3 fade=%4/%5 background=%6 opacity=%7", m_fHudPositionX, m_fHudPositionY, m_fHudScale, m_iFadeDelayMs, m_iFadeDurationMs, m_bBackgroundEnabled, m_fBackgroundOpacity), LogLevel.NORMAL);\n'''
controller = replace_once(controller, old_after_load, new_after_load, "post-load position migration")

resolve_legacy = '''\n\tprotected void ResolveLegacyHudPosition(string position, out float x, out float y)\n\t{\n\t\tx = 0.5;\n\t\ty = 0.95;\n\n\t\tif (position == "top_left") { x = 0.05; y = 0.05; }\n\t\telse if (position == "top_center") { x = 0.5; y = 0.05; }\n\t\telse if (position == "top_right") { x = 0.95; y = 0.05; }\n\t\telse if (position == "center_left") { x = 0.05; y = 0.5; }\n\t\telse if (position == "center") { x = 0.5; y = 0.5; }\n\t\telse if (position == "center_right") { x = 0.95; y = 0.5; }\n\t\telse if (position == "bottom_left") { x = 0.05; y = 0.95; }\n\t\telse if (position == "bottom_right") { x = 0.95; y = 0.95; }\n\t}\n'''
controller = replace_once(
    controller,
    '\n\tprotected int BoolToInt(bool value)\n',
    resolve_legacy + '\n\tprotected int BoolToInt(bool value)\n',
    "legacy position resolver insertion",
)

controller = replace_once(
    controller,
    '\t\tfile.WriteLine(string.Format("position=%1", m_sHudPosition));\n',
    '\t\tfile.WriteLine(string.Format("position_x=%1", m_fHudPositionX));\n'
    '\t\tfile.WriteLine(string.Format("position_y=%1", m_fHudPositionY));\n',
    "saved normalized position",
)

start = controller.index('\tprotected int GetHudPositionIndex()\n')
end = controller.index('\tint GetSettingOptionCount(int index)\n', start)
compat_position_block = '''\tvoid GetHudPositionNormalized(out float x, out float y)\n\t{\n\t\tx = m_fHudPositionX;\n\t\ty = m_fHudPositionY;\n\t}\n\n\tvoid SetHudPositionNormalized(float x, float y)\n\t{\n\t\tm_fHudPositionX = Math.Clamp(x, 0.0, 1.0);\n\t\tm_fHudPositionY = Math.Clamp(y, 0.0, 1.0);\n\t\tSaveHudSettings();\n\t\tif (m_bInitialized)\n\t\t\tRebuildHud();\n\t}\n\n\t// Compatibility helpers for the old nine-position selector. The native settings\n\t// tab now uses the draggable preview, but keeping these avoids breaking callers.\n\tprotected int GetHudPositionIndex()\n\t{\n\t\tint column = 1;\n\t\tint row = 1;\n\t\tif (m_fHudPositionX < 0.33) column = 0;\n\t\telse if (m_fHudPositionX > 0.66) column = 2;\n\t\tif (m_fHudPositionY < 0.33) row = 0;\n\t\telse if (m_fHudPositionY > 0.66) row = 2;\n\t\treturn row * 3 + column;\n\t}\n\n\tprotected void SetHudPositionIndex(int index)\n\t{\n\t\tindex = Math.ClampInt(index, 0, 8);\n\t\tint column = index % 3;\n\t\tint row = index / 3;\n\t\tif (column == 0) m_fHudPositionX = 0.05;\n\t\telse if (column == 1) m_fHudPositionX = 0.5;\n\t\telse m_fHudPositionX = 0.95;\n\t\tif (row == 0) m_fHudPositionY = 0.05;\n\t\telse if (row == 1) m_fHudPositionY = 0.5;\n\t\telse m_fHudPositionY = 0.95;\n\t}\n\n'''
controller = controller[:start] + compat_position_block + controller[end:]
controller_path.write_text(controller, encoding="utf-8")


# -----------------------------------------------------------------------------
# HOTASSettingsTab.c — restore the draggable screen preview and end-stop arrows,
# while keeping sliders, detected-axis edit boxes, and Free Look labels.
# -----------------------------------------------------------------------------
tab_path = Path("Scripts/Game/HOTASDebugger/HOTASSettingsTab.c")
tab = tab_path.read_text(encoding="utf-8")

tab = replace_once(
    tab,
    '\tprotected bool m_bLoading;\n',
    '\tprotected bool m_bLoading;\n'
    '\tprotected Widget m_PreviewHost;\n'
    '\tprotected Widget m_PreviewSquare;\n'
    '\tprotected Widget m_PreviewSquareBackground;\n'
    '\tprotected Widget m_ScreenPreview;\n'
    '\tprotected Widget m_ScreenPreviewBackground;\n'
    '\tprotected Widget m_HudPositionPreview;\n'
    '\tprotected ref HOTASHudPositionDragHandler m_HudDragHandler;\n'
    '\tprotected bool m_bDraggingHudPosition;\n'
    '\tprotected float m_fHudDragOffsetX;\n'
    '\tprotected float m_fHudDragOffsetY;\n'
    '\tprotected float m_fPreviewPositionX = 0.5;\n'
    '\tprotected float m_fPreviewPositionY = 0.95;\n',
    "preview fields",
)

tab = replace_once(
    tab,
    '\t\tSetupAxisLabelEditors();\n\t\tSetupFreelookLabelEditors();\n\t\tm_bLoading = false;\n\t}\n',
    '\t\tSetupAxisLabelEditors();\n'
    '\t\tSetupFreelookLabelEditors();\n'
    '\t\tSetupHudPositionPreview();\n'
    '\t\tSyncHudPositionPreviewFromController();\n'
    '\t\tm_bLoading = false;\n'
    '\t\tGetGame().GetCallqueue().CallLater(RefreshAllSpinBoxArrows, 0, false);\n'
    '\t\tGetGame().GetCallqueue().CallLater(UpdateHudPositionPreview, 0, false);\n'
    '\t}\n',
    "OnTabCreate preview setup",
)

tab = replace_once(
    tab,
    '\t\tSyncAxisLabelEditors();\n\t\tSyncFreelookLabelEditors();\n\t\tm_bLoading = false;\n\t}\n\n\t//------------------------------------------------------------------------------------------------\n\tprotected SCR_SpinBoxComponent FindSpinBox',
    '\t\tSyncAxisLabelEditors();\n'
    '\t\tSyncFreelookLabelEditors();\n'
    '\t\tSyncHudPositionPreviewFromController();\n'
    '\t\tm_bLoading = false;\n'
    '\t\tGetGame().GetCallqueue().CallLater(RefreshAllSpinBoxArrows, 0, false);\n\n'
    '\t\tGetGame().GetCallqueue().Remove(UpdateHudPositionPreview);\n'
    '\t\tGetGame().GetCallqueue().CallLater(UpdateHudPositionPreview, 0, false);\n'
    '\t\tGetGame().GetCallqueue().CallLater(UpdateHudPositionPreview, 250, true);\n'
    '\t}\n\n'
    '\t//------------------------------------------------------------------------------------------------\n'
    '\toverride void OnTabHide()\n'
    '\t{\n'
    '\t\tGetGame().GetCallqueue().Remove(UpdateHudPositionPreview);\n'
    '\t\tGetGame().GetCallqueue().Remove(UpdateHudPositionDrag);\n'
    '\t\tif (m_bDraggingHudPosition)\n'
    '\t\t\tEndHudPositionDrag();\n'
    '\t\tsuper.OnTabHide();\n'
    '\t}\n\n'
    '\t//------------------------------------------------------------------------------------------------\n'
    '\tprotected SCR_SpinBoxComponent FindSpinBox',
    "OnTabShow/Hide preview lifecycle",
)

tab = replace_once(
    tab,
    '\t\tm_HotasConfig.SetCurrentItem(selected, false, false, false);\n\t}\n',
    '\t\tm_HotasConfig.SetCurrentItem(selected, false, false, false);\n'
    '\t\tRefreshSpinBoxArrows(m_HotasConfig, selected, m_UserConfigs.Count() + 1);\n'
    '\t}\n',
    "HOTAS selector arrow sync",
)

tab = replace_once(
    tab,
    '\t\tInputBinding binding = keybindModule.GetInputBindings();\n\t\tif (!binding)\n\t\t\treturn;\n\n\t\tif (index <= 0)\n',
    '\t\tInputBinding binding = keybindModule.GetInputBindings();\n'
    '\t\tif (!binding)\n'
    '\t\t\treturn;\n\n'
    '\t\tRefreshSpinBoxArrows(m_HotasConfig, index, m_UserConfigs.Count() + 1);\n'
    '\t\tGetGame().GetCallqueue().CallLater(RefreshAllSpinBoxArrows, 0, false);\n\n'
    '\t\tif (index <= 0)\n',
    "HOTAS selector change arrow clamp",
)

preview_helpers = r'''
	//------------------------------------------------------------------------------------------------
	protected void SetupHudPositionPreview()
	{
		m_PreviewHost = m_wRoot.FindAnyWidget("HUDPreviewHost");
		m_PreviewSquare = m_wRoot.FindAnyWidget("HUDPreviewSquare");
		m_PreviewSquareBackground = m_wRoot.FindAnyWidget("HUDPreviewSquareBackground");
		m_ScreenPreview = m_wRoot.FindAnyWidget("HUDScreenPreview");
		m_ScreenPreviewBackground = m_wRoot.FindAnyWidget("HUDScreenPreviewBackground");
		m_HudPositionPreview = m_wRoot.FindAnyWidget("HUDPositionPreview");

		if (m_HudPositionPreview)
		{
			m_HudDragHandler = new HOTASHudPositionDragHandler(this);
			m_HudPositionPreview.AddHandler(m_HudDragHandler);
		}
	}

	protected void SyncHudPositionPreviewFromController()
	{
		HOTASDebugController.GetInstance().GetHudPositionNormalized(m_fPreviewPositionX, m_fPreviewPositionY);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateHudPositionPreview()
	{
		if (!m_PreviewHost || !m_PreviewSquare || !m_PreviewSquareBackground || !m_ScreenPreview || !m_ScreenPreviewBackground || !m_HudPositionPreview)
			return;

		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace)
			return;

		float hostWidthPx;
		float hostHeightPx;
		m_PreviewHost.GetScreenSize(hostWidthPx, hostHeightPx);
		float hostWidth = workspace.DPIUnscale(hostWidthPx);
		float hostHeight = workspace.DPIUnscale(hostHeightPx);
		if (hostWidth <= 1 || hostHeight <= 1)
			return;

		float squareSize = Math.Min(hostWidth, hostHeight) - 24;
		if (squareSize <= 32)
			return;

		float squareLeft = (hostWidth - squareSize) * 0.5;
		float squareTop = (hostHeight - squareSize) * 0.5;
		FrameSlot.SetPos(m_PreviewSquare, squareLeft, squareTop);
		FrameSlot.SetSize(m_PreviewSquare, squareSize, squareSize);
		FrameSlot.SetPos(m_PreviewSquareBackground, 0, 0);
		FrameSlot.SetSize(m_PreviewSquareBackground, squareSize, squareSize);

		float screenWidth = workspace.GetWidth();
		float screenHeight = workspace.GetHeight();
		if (screenWidth <= 0 || screenHeight <= 0)
			return;

		float inset = 24;
		float available = squareSize - inset * 2;
		if (available <= 1)
			return;

		float screenAspect = screenWidth / screenHeight;
		float previewWidth = available;
		float previewHeight = available / screenAspect;
		if (previewHeight > available)
		{
			previewHeight = available;
			previewWidth = available * screenAspect;
		}

		float screenLeft = (squareSize - previewWidth) * 0.5;
		float screenTop = (squareSize - previewHeight) * 0.5;
		FrameSlot.SetPos(m_ScreenPreview, screenLeft, screenTop);
		FrameSlot.SetSize(m_ScreenPreview, previewWidth, previewHeight);
		FrameSlot.SetPos(m_ScreenPreviewBackground, 0, 0);
		FrameSlot.SetSize(m_ScreenPreviewBackground, previewWidth, previewHeight);

		HOTASDebugController controller = HOTASDebugController.GetInstance();
		float hudScale = 0.6 + (controller.GetHudScalePercent() / 100.0) * 1.4;
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

		if (previewHudWidth < 8)
			previewHudWidth = 8;
		if (previewHudHeight < 5)
			previewHudHeight = 5;

		FrameSlot.SetPos(m_HudPositionPreview, previewHudLeft, previewHudTop);
		FrameSlot.SetSize(m_HudPositionPreview, previewHudWidth, previewHudHeight);
		if (controller.GetSettingOptionIndex(0) == 0)
			m_HudPositionPreview.SetOpacity(0.3);
		else
			m_HudPositionPreview.SetOpacity(0.9);
	}

	//------------------------------------------------------------------------------------------------
	void BeginHudPositionDrag()
	{
		if (!m_HudPositionPreview || !m_ScreenPreview)
			return;

		int mouseX;
		int mouseY;
		WidgetManager.GetMousePos(mouseX, mouseY);
		float boxX;
		float boxY;
		m_HudPositionPreview.GetScreenPos(boxX, boxY);
		m_fHudDragOffsetX = mouseX - boxX;
		m_fHudDragOffsetY = mouseY - boxY;

		m_bDraggingHudPosition = true;
		GetGame().GetCallqueue().Remove(UpdateHudPositionDrag);
		GetGame().GetCallqueue().CallLater(UpdateHudPositionDrag, 16, true);
		UpdateHudPositionDrag();
	}

	//------------------------------------------------------------------------------------------------
	void EndHudPositionDrag()
	{
		if (!m_bDraggingHudPosition)
			return;

		m_bDraggingHudPosition = false;
		GetGame().GetCallqueue().Remove(UpdateHudPositionDrag);
		HOTASDebugController.GetInstance().SetHudPositionNormalized(m_fPreviewPositionX, m_fPreviewPositionY);
		UpdateHudPositionPreview();
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateHudPositionDrag()
	{
		if (!m_bDraggingHudPosition || !m_HudPositionPreview || !m_ScreenPreview)
			return;

		int mouseX;
		int mouseY;
		WidgetManager.GetMousePos(mouseX, mouseY);
		float screenX;
		float screenY;
		float screenWidth;
		float screenHeight;
		float boxWidth;
		float boxHeight;
		m_ScreenPreview.GetScreenPos(screenX, screenY);
		m_ScreenPreview.GetScreenSize(screenWidth, screenHeight);
		m_HudPositionPreview.GetScreenSize(boxWidth, boxHeight);

		float travelX = screenWidth - boxWidth;
		float travelY = screenHeight - boxHeight;
		if (travelX <= 0 || travelY <= 0)
			return;

		m_fPreviewPositionX = Math.Clamp((mouseX - screenX - m_fHudDragOffsetX) / travelX, 0.0, 1.0);
		m_fPreviewPositionY = Math.Clamp((mouseY - screenY - m_fHudDragOffsetY) / travelY, 0.0, 1.0);
		UpdateHudPositionPreview();
	}

	//------------------------------------------------------------------------------------------------
	protected void RefreshAllSpinBoxArrows()
	{
		if (m_HotasConfig)
			RefreshSpinBoxArrows(m_HotasConfig, m_HotasConfig.GetCurrentIndex(), m_UserConfigs.Count() + 1);

		HOTASDebugController controller = HOTASDebugController.GetInstance();
		for (int i = 0; i < m_HudControls.Count(); i++)
		{
			SCR_SpinBoxComponent control = m_HudControls[i];
			if (!control)
				continue;
			RefreshSpinBoxArrows(control, control.GetCurrentIndex(), controller.GetSettingOptionCount(i));
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void RefreshSpinBoxArrows(SCR_SpinBoxComponent control, int selected, int optionCount)
	{
		if (!control || optionCount <= 0)
			return;

		Widget root = control.GetRootWidget();
		if (!root)
			return;

		RefreshArrowButton(root.FindAnyWidget("ButtonLeft"), selected > 0);
		RefreshArrowButton(root.FindAnyWidget("ButtonRight"), selected < optionCount - 1);
	}

	//------------------------------------------------------------------------------------------------
	protected void RefreshArrowButton(Widget buttonWidget, bool enabled)
	{
		if (!buttonWidget)
			return;

		SCR_PagingButtonComponent button = SCR_PagingButtonComponent.Cast(buttonWidget.FindHandler(SCR_PagingButtonComponent));
		if (button)
			button.SetEnabled(enabled, false);
		else
			buttonWidget.SetEnabled(enabled);

		Widget background = buttonWidget.FindAnyWidget("BackgroundImage");
		if (background)
			background.SetVisible(true);
		buttonWidget.SetOpacity(enabled ? 1.0 : 0.35);
	}

'''
tab = replace_once(
    tab,
    '\t//------------------------------------------------------------------------------------------------\n\tprotected void SetupHudControls()\n',
    preview_helpers + '\t//------------------------------------------------------------------------------------------------\n\tprotected void SetupHudControls()\n',
    "preview/arrow helper insertion",
)

tab = replace_once(
    tab,
    '\t\t\tcontrol.SetCurrentItem(controller.GetSettingOptionIndex(i), false, false, false);\n\t\t\tcontrol.m_OnChanged.Insert(OnHudSettingChanged);\n',
    '\t\t\tint currentIndex = controller.GetSettingOptionIndex(i);\n'
    '\t\t\tcontrol.SetCurrentItem(currentIndex, false, false, false);\n'
    '\t\t\tRefreshSpinBoxArrows(control, currentIndex, optionCount);\n'
    '\t\t\tcontrol.m_OnChanged.Insert(OnHudSettingChanged);\n',
    "initial HUD arrow state",
)

tab = replace_once(
    tab,
    '\t\t\tif (control)\n\t\t\t\tcontrol.SetCurrentItem(controller.GetSettingOptionIndex(i), false, false, false);\n',
    '\t\t\tif (control)\n'
    '\t\t\t{\n'
    '\t\t\t\tint currentIndex = controller.GetSettingOptionIndex(i);\n'
    '\t\t\t\tcontrol.SetCurrentItem(currentIndex, false, false, false);\n'
    '\t\t\t\tRefreshSpinBoxArrows(control, currentIndex, controller.GetSettingOptionCount(i));\n'
    '\t\t\t}\n',
    "synced HUD arrow state",
)

tab = replace_once(
    tab,
    '\t\tarray<string> widgetNames = { "RollAxis", "PitchAxis", "ThrottleAxis", "YawAxis" };\n\t\tHOTASDebugController controller = HOTASDebugController.GetInstance();\n',
    '\t\tarray<string> widgetNames = { "RollAxis", "PitchAxis", "ThrottleAxis", "YawAxis" };\n'
    '\t\tarray<string> placeholders = { "e.g. Roll", "e.g. Pitch", "e.g. Throttle", "e.g. Yaw" };\n'
    '\t\tHOTASDebugController controller = HOTASDebugController.GetInstance();\n',
    "axis example placeholders",
)

tab = replace_once(
    tab,
    '\t\t\teditor.SetPlaceholderText("Custom HUD label");\n',
    '\t\t\teditor.SetPlaceholderText(placeholders[i]);\n',
    "axis placeholder assignment",
)

tab = replace_once(
    tab,
    '\t\t\tHOTASDebugController.GetInstance().SetSettingOptionIndex(i, optionIndex);\n\t\t\treturn;\n',
    '\t\t\tHOTASDebugController controller = HOTASDebugController.GetInstance();\n'
    '\t\t\tcontroller.SetSettingOptionIndex(i, optionIndex);\n'
    '\t\t\tRefreshSpinBoxArrows(component, optionIndex, controller.GetSettingOptionCount(i));\n'
    '\t\t\tGetGame().GetCallqueue().CallLater(RefreshAllSpinBoxArrows, 0, false);\n'
    '\t\t\tUpdateHudPositionPreview();\n'
    '\t\t\treturn;\n',
    "HUD setting change arrows/preview",
)

tab = replace_once(
    tab,
    '\t\tHOTASDebugController.GetInstance().SetHudScalePercent(value);\n\t}\n',
    '\t\tHOTASDebugController.GetInstance().SetHudScalePercent(value);\n'
    '\t\tUpdateHudPositionPreview();\n'
    '\t}\n',
    "scale preview update",
)

drag_handler = r'''

//------------------------------------------------------------------------------------------------
// Mouse handler for the orange HUD preview bar.
class HOTASHudPositionDragHandler : ScriptedWidgetEventHandler
{
	protected HOTASSettingsSubMenu m_Owner;

	void HOTASHudPositionDragHandler(HOTASSettingsSubMenu owner)
	{
		m_Owner = owner;
	}

	override bool OnMouseButtonDown(Widget w, int x, int y, int button)
	{
		if (button != 0 || !m_Owner)
			return false;
		m_Owner.BeginHudPositionDrag();
		return true;
	}

	override bool OnMouseButtonUp(Widget w, int x, int y, int button)
	{
		if (button != 0 || !m_Owner)
			return false;
		m_Owner.EndHudPositionDrag();
		return true;
	}
}
'''
tab = replace_once(
    tab,
    '\n//------------------------------------------------------------------------------------------------\n// Add HOTAS as a normal peer of the game\'s Video / Audio / Interface settings tabs.\n',
    drag_handler + '\n//------------------------------------------------------------------------------------------------\n// Add HOTAS as a normal peer of the game\'s Video / Audio / Interface settings tabs.\n',
    "drag handler insertion",
)
tab_path.write_text(tab, encoding="utf-8")


# -----------------------------------------------------------------------------
# Settings layout — remove the nine-position spinbox, restore the preview pane,
# and explicitly disable cycling on the remaining spin boxes.
# -----------------------------------------------------------------------------
layout_path = Path("UI/layouts/Menus/SettingsSubMenus/HOTASSettings.layout")
layout = layout_path.read_text(encoding="utf-8")

position_row = '''      ButtonWidgetClass "{8C52D9F7A31B6422}" : "{C9DF0E6590F6C388}UI/layouts/WidgetLibrary/SpinBox/WLib_SpinBox.layout" {\n       Name "HUDPosition"\n       Slot LayoutSlot "{8C52D9F7A31B6423}" { Padding 4 4 4 4 }\n       components { SCR_SpinBoxComponent "{5472C6CBC0640458}" { m_sLabel "Position" m_sLabelLayout "{F003823FF141983C}UI/layouts/Menus/SettingsMenu/CustomWidgets/SettingsLabel.layout" m_bUseLightArrows 1 m_bShowHints 0 } }\n      }\n'''
layout = replace_once(layout, position_row, "", "remove position spinbox")

# Explicitly keep every remaining selector from wrapping around.
layout = layout.replace('m_bShowHints 0 }', 'm_bShowHints 0 m_bCycleMode 0 }')
layout = replace_once(
    layout,
    '         m_bShowHints 0\n        }\n',
    '         m_bShowHints 0\n         m_bCycleMode 0\n        }\n',
    "HOTAS config cycle mode",
)

preview_layout = r'''  VerticalLayoutWidgetClass "{8C52D9F7A31B6500}" {
   Name "HUDPreviewPane"
   Slot LayoutSlot "{8C52D9F7A31B6501}" {
    Padding 36 72 24 24
    SizeMode Fill
    FillWeight 1
   }
   {
    VerticalLayoutWidgetClass "{8C52D9F7A31B6502}" : "{FEEEB639F2735BA1}UI/layouts/Menus/SettingsMenu/CustomWidgets/SettingsTitle.layout" {
     Name "TitleHUDPreview"
     Slot LayoutSlot "{8C52D9F7A31B6503}" {
     }
     components {
      SCR_LabelComponent "{58B30C1A8E56F0FF}" {
       m_sLabel "HUD Position - Drag Orange Bar"
       m_fPaddingTop 4
      }
     }
    }
    FrameWidgetClass "{8C52D9F7A31B6504}" {
     Name "HUDPreviewHost"
     Slot LayoutSlot "{8C52D9F7A31B6505}" {
      Padding 8 12 8 12
      SizeMode Fill
      FillWeight 1
     }
     {
      FrameWidgetClass "{8C52D9F7A31B6506}" {
       Name "HUDPreviewSquare"
       Slot FrameWidgetSlot "{8C52D9F7A31B6507}" {
        PositionX 0
        PositionY 0
        SizeX 400
        SizeY 400
        Alignment 0 0
       }
       {
        ImageWidgetClass "{8C52D9F7A31B6508}" {
         Name "HUDPreviewSquareBackground"
         Slot FrameWidgetSlot "{8C52D9F7A31B6509}" {
          PositionX 0
          PositionY 0
          SizeX 400
          SizeY 400
          Alignment 0 0
         }
         Color 0.08 0.08 0.08 0.72
         Size 1024 1024
        }
        FrameWidgetClass "{8C52D9F7A31B6510}" {
         Name "HUDScreenPreview"
         Slot FrameWidgetSlot "{8C52D9F7A31B6511}" {
          PositionX 20
          PositionY 80
          SizeX 360
          SizeY 240
          Alignment 0 0
         }
         {
          ImageWidgetClass "{8C52D9F7A31B6512}" {
           Name "HUDScreenPreviewBackground"
           Slot FrameWidgetSlot "{8C52D9F7A31B6513}" {
            PositionX 0
            PositionY 0
            SizeX 360
            SizeY 240
            Alignment 0 0
           }
           Color 0.012 0.016 0.021 1
           Size 1024 1024
          }
          ImageWidgetClass "{8C52D9F7A31B6514}" {
           Name "HUDPositionPreview"
           Slot FrameWidgetSlot "{8C52D9F7A31B6515}" {
            PositionX 120
            PositionY 205
            SizeX 120
            SizeY 16
            Alignment 0 0
           }
           Color 0.7605 0.3865 0.0802 1
           Size 1024 1024
          }
         }
        }
       }
      }
     }
    }
   }
  }
'''
end_marker = '  }\n }\n}\n'
if not layout.endswith(end_marker):
    raise RuntimeError("Unexpected settings layout ending")
layout = layout[:-len(end_marker)] + '  }\n' + preview_layout + ' }\n}\n'
layout_path.write_text(layout, encoding="utf-8")

print("Restored exact draggable HUD position, non-wrapping selectors, and axis examples")
