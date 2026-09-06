from pathlib import Path


def replace_once(text, old, new, label):
    count = text.count(old)
    if count != 1:
        raise RuntimeError(f"{label}: expected exactly 1 match, found {count}")
    return text.replace(old, new, 1)


# -----------------------------------------------------------------------------
# Runtime HUD settings: replace the nine-position enum with normalized exact X/Y.
# -----------------------------------------------------------------------------
path = Path("Scripts/Game/HOTASDebugger/HOTASDebugger.c")
src = path.read_text(encoding="utf-8")

src = replace_once(
    src,
    '\tprotected string m_sHudPosition = "bottom_center";\n\tprotected float m_fHudScale = 1.0;',
    '\t// Normalized top-left travel position. 0 = left/top edge, 1 = right/bottom edge.\n\t// Storing the position against the available travel range keeps the HUD fully on-screen\n\t// while preserving placement across resolutions and HUD scales.\n\tprotected float m_fHudPositionX = 0.5;\n\tprotected float m_fHudPositionY = 0.95;\n\tprotected float m_fHudScale = 1.0;',
    "HUD position fields",
)

old_get_position = '''\tprotected void GetHudPosition(WorkspaceWidget workspace, int width, int height, out int left, out int top)
\t{
\t\tint marginX = Math.Round(48 * m_fHudScale);
\t\tint marginY = Math.Round(54 * m_fHudScale);
\t\tint screenWidth = workspace.GetWidth();
\t\tint screenHeight = workspace.GetHeight();

\t\tleft = (screenWidth - width) / 2;
\t\ttop = screenHeight - height - marginY;

\t\tif (m_sHudPosition == "top_left")
\t\t{
\t\t\tleft = marginX;
\t\t\ttop = marginY;
\t\t}
\t\telse if (m_sHudPosition == "top_center")
\t\t{
\t\t\tleft = (screenWidth - width) / 2;
\t\t\ttop = marginY;
\t\t}
\t\telse if (m_sHudPosition == "top_right")
\t\t{
\t\t\tleft = screenWidth - width - marginX;
\t\t\ttop = marginY;
\t\t}
\t\telse if (m_sHudPosition == "center_left")
\t\t{
\t\t\tleft = marginX;
\t\t\ttop = (screenHeight - height) / 2;
\t\t}
\t\telse if (m_sHudPosition == "center")
\t\t{
\t\t\tleft = (screenWidth - width) / 2;
\t\t\ttop = (screenHeight - height) / 2;
\t\t}
\t\telse if (m_sHudPosition == "center_right")
\t\t{
\t\t\tleft = screenWidth - width - marginX;
\t\t\ttop = (screenHeight - height) / 2;
\t\t}
\t\telse if (m_sHudPosition == "bottom_left")
\t\t{
\t\t\tleft = marginX;
\t\t\ttop = screenHeight - height - marginY;
\t\t}
\t\telse if (m_sHudPosition == "bottom_right")
\t\t{
\t\t\tleft = screenWidth - width - marginX;
\t\t\ttop = screenHeight - height - marginY;
\t\t}
\t}
'''
new_get_position = '''\tprotected void GetHudPosition(WorkspaceWidget workspace, int width, int height, out int left, out int top)
\t{
\t\tint screenWidth = workspace.GetWidth();
\t\tint screenHeight = workspace.GetHeight();
\t\tint travelX = Math.Max(0, screenWidth - width);
\t\tint travelY = Math.Max(0, screenHeight - height);

\t\tleft = Math.Round(travelX * m_fHudPositionX);
\t\ttop = Math.Round(travelY * m_fHudPositionY);
\t}
'''
src = replace_once(src, old_get_position, new_get_position, "GetHudPosition")

src = replace_once(
    src,
    '\t\t\t\tdefaults.WriteLine("# position: top_left, top_center, top_right, center_left, center, center_right, bottom_left, bottom_center, bottom_right");\n\t\t\t\tdefaults.WriteLine("position=bottom_center");',
    '\t\t\t\tdefaults.WriteLine("# Normalized exact HUD placement inside the usable screen area.");\n\t\t\t\tdefaults.WriteLine("position_x=0.5");\n\t\t\t\tdefaults.WriteLine("position_y=0.95");',
    "default position settings",
)

src = replace_once(
    src,
    '\t\tstring line;\n\t\twhile (file.ReadLine(line) >= 0)',
    '\t\tstring line;\n\t\tstring legacyPosition;\n\t\tbool loadedPositionX;\n\t\tbool loadedPositionY;\n\t\twhile (file.ReadLine(line) >= 0)',
    "position migration locals",
)

src = replace_once(
    src,
    '\t\t\telse if (key == "position")\n\t\t\t\tm_sHudPosition = value;\n\t\t\telse if (key == "scale")',
    '\t\t\telse if (key == "position")\n\t\t\t\tlegacyPosition = value;\n\t\t\telse if (key == "position_x")\n\t\t\t{\n\t\t\t\tm_fHudPositionX = Math.Clamp(value.ToFloat(0.5), 0.0, 1.0);\n\t\t\t\tloadedPositionX = true;\n\t\t\t}\n\t\t\telse if (key == "position_y")\n\t\t\t{\n\t\t\t\tm_fHudPositionY = Math.Clamp(value.ToFloat(0.95), 0.0, 1.0);\n\t\t\t\tloadedPositionY = true;\n\t\t\t}\n\t\t\telse if (key == "scale")',
    "position parser",
)

src = replace_once(
    src,
    '\t\tfile.Close();\n\n\t\tPrint(string.Format("[HOTAS Debugger] HUD settings: position=%1 scale=%2 fade=%3/%4 background=%5 opacity=%6", m_sHudPosition, m_fHudScale, m_iFadeDelayMs, m_iFadeDurationMs, m_bBackgroundEnabled, m_fBackgroundOpacity), LogLevel.NORMAL);',
    '\t\tfile.Close();\n\n\t\tif (!loadedPositionX || !loadedPositionY)\n\t\t{\n\t\t\tfloat legacyX;\n\t\t\tfloat legacyY;\n\t\t\tResolveLegacyHudPosition(legacyPosition, legacyX, legacyY);\n\t\t\tif (!loadedPositionX)\n\t\t\t\tm_fHudPositionX = legacyX;\n\t\t\tif (!loadedPositionY)\n\t\t\t\tm_fHudPositionY = legacyY;\n\t\t}\n\n\t\tPrint(string.Format("[HOTAS Debugger] HUD settings: position=%1/%2 scale=%3 fade=%4/%5 background=%6 opacity=%7", m_fHudPositionX, m_fHudPositionY, m_fHudScale, m_iFadeDelayMs, m_iFadeDurationMs, m_bBackgroundEnabled, m_fBackgroundOpacity), LogLevel.NORMAL);',
    "position migration and log",
)

src = replace_once(
    src,
    '\tprotected int BoolToInt(bool value)\n',
    '''\tprotected void ResolveLegacyHudPosition(string position, out float x, out float y)
\t{
\t\tx = 0.5;
\t\ty = 0.95;

\t\tif (position == "top_left") { x = 0.05; y = 0.05; }
\t\telse if (position == "top_center") { x = 0.5; y = 0.05; }
\t\telse if (position == "top_right") { x = 0.95; y = 0.05; }
\t\telse if (position == "center_left") { x = 0.05; y = 0.5; }
\t\telse if (position == "center") { x = 0.5; y = 0.5; }
\t\telse if (position == "center_right") { x = 0.95; y = 0.5; }
\t\telse if (position == "bottom_left") { x = 0.05; y = 0.95; }
\t\telse if (position == "bottom_right") { x = 0.95; y = 0.95; }
\t}

\tprotected int BoolToInt(bool value)
''',
    "legacy position helper",
)

src = replace_once(
    src,
    '\t\tfile.WriteLine(string.Format("position=%1", m_sHudPosition));',
    '\t\tfile.WriteLine(string.Format("position_x=%1", m_fHudPositionX));\n\t\tfile.WriteLine(string.Format("position_y=%1", m_fHudPositionY));',
    "save exact position",
)

settings_start = src.index('\n\tvoid ReloadHudSettings()')
settings_end = src.index('\n\tprotected void ShowHud()', settings_start)
new_settings_api = r'''
	void ReloadHudSettings()
	{
		LoadHudSettings();
	}

	void GetHudPositionNormalized(out float x, out float y)
	{
		x = m_fHudPositionX;
		y = m_fHudPositionY;
	}

	void SetHudPositionNormalized(float x, float y)
	{
		m_fHudPositionX = Math.Clamp(x, 0.0, 1.0);
		m_fHudPositionY = Math.Clamp(y, 0.0, 1.0);
		SaveHudSettings();
		if (m_bInitialized)
			RebuildHud();
	}

	int GetSettingsCount()
	{
		return 11;
	}

	string GetSettingLabel(int index)
	{
		switch (index)
		{
			case 0: return "HUD Enabled";
			case 1: return "Scale";
			case 2: return "Fade Delay";
			case 3: return "Fade Duration";
			case 4: return "Background";
			case 5: return "Background Opacity";
			case 6: return "Roll Axis";
			case 7: return "Pitch Axis";
			case 8: return "Throttle Axis";
			case 9: return "Yaw Axis";
			case 10: return "Debug Mode";
		}
		return "Unknown";
	}

	int GetSettingOptionCount(int index)
	{
		switch (index)
		{
			case 0: return 2;
			case 1: return 16;
			case 2: return 101;
			case 3: return 101;
			case 4: return 2;
			case 5: return 21;
			case 6:
			case 7:
			case 8:
			case 9: return 65;
			case 10: return 2;
		}
		return 0;
	}

	int GetSettingOptionIndex(int index)
	{
		switch (index)
		{
			case 0:
				if (m_bHudEnabled) return 1;
				return 0;
			case 1: return Math.ClampInt(Math.Round((m_fHudScale - 0.5) * 10.0), 0, 15);
			case 2: return Math.ClampInt(Math.Round(m_iFadeDelayMs / 100.0), 0, 100);
			case 3: return Math.ClampInt(Math.Round(m_iFadeDurationMs / 50.0), 0, 100);
			case 4:
				if (m_bBackgroundEnabled) return 1;
				return 0;
			case 5: return Math.ClampInt(Math.Round(m_fBackgroundOpacity * 20.0), 0, 20);
			case 6: return Math.ClampInt(m_iRollAxis + 1, 0, 64);
			case 7: return Math.ClampInt(m_iPitchAxis + 1, 0, 64);
			case 8: return Math.ClampInt(m_iThrottleAxis + 1, 0, 64);
			case 9: return Math.ClampInt(m_iYawAxis + 1, 0, 64);
			case 10:
				if (m_bDebugMode) return 1;
				return 0;
		}
		return 0;
	}

	string GetSettingOptionLabel(int index, int optionIndex)
	{
		optionIndex = Math.ClampInt(optionIndex, 0, Math.Max(0, GetSettingOptionCount(index) - 1));
		switch (index)
		{
			case 0:
			case 4:
			case 10:
				if (optionIndex > 0) return "On";
				return "Off";
			case 1: return string.Format("%1x", (0.5 + optionIndex * 0.1).ToString(1));
			case 2: return string.Format("%1 ms", optionIndex * 100);
			case 3: return string.Format("%1 ms", optionIndex * 50);
			case 5: return string.Format("%1%", optionIndex * 5);
			case 6:
			case 7:
			case 8:
			case 9:
				if (optionIndex == 0) return "Disabled";
				return string.Format("Axis %1", optionIndex);
		}
		return "";
	}

	void SetSettingOptionIndex(int index, int optionIndex)
	{
		int count = GetSettingOptionCount(index);
		if (count <= 0)
			return;
		optionIndex = Math.ClampInt(optionIndex, 0, count - 1);

		switch (index)
		{
			case 0: m_bHudEnabled = optionIndex != 0; break;
			case 1: m_fHudScale = 0.5 + optionIndex * 0.1; break;
			case 2: m_iFadeDelayMs = optionIndex * 100; break;
			case 3: m_iFadeDurationMs = optionIndex * 50; break;
			case 4: m_bBackgroundEnabled = optionIndex != 0; break;
			case 5: m_fBackgroundOpacity = optionIndex * 0.05; break;
			case 6: m_iRollAxis = optionIndex - 1; break;
			case 7: m_iPitchAxis = optionIndex - 1; break;
			case 8: m_iThrottleAxis = optionIndex - 1; break;
			case 9: m_iYawAxis = optionIndex - 1; break;
			case 10: m_bDebugMode = optionIndex != 0; break;
			default: return;
		}

		SaveHudSettings();
		if (m_bInitialized)
			RebuildHud();
	}
'''
src = src[:settings_start] + "\n" + new_settings_api.strip("\n") + src[settings_end:]

path.write_text(src, encoding="utf-8")


# -----------------------------------------------------------------------------
# Settings tab: draggable preview + stable disabled-arrow visuals.
# -----------------------------------------------------------------------------
path = Path("Scripts/Game/HOTASDebugger/HOTASSettingsTab.c")
src = path.read_text(encoding="utf-8")

src = replace_once(
    src,
    '\tprotected Widget m_HudPositionPreview;\n',
    '''\tprotected Widget m_HudPositionPreview;
\tprotected ref HOTASHudPositionDragHandler m_HudDragHandler;
\tprotected bool m_bDraggingHudPosition;
\tprotected float m_fHudDragOffsetX;
\tprotected float m_fHudDragOffsetY;
\tprotected float m_fPreviewPositionX = 0.5;
\tprotected float m_fPreviewPositionY = 0.95;
''',
    "settings drag fields",
)

src = replace_once(
    src,
    '\t\tSetupHudControls();\n\t\tSetupHudPositionPreview();\n\t\tm_bLoading = false;',
    '\t\tSetupHudControls();\n\t\tSetupHudPositionPreview();\n\t\tSyncHudPositionPreviewFromController();\n\t\tm_bLoading = false;\n\t\tGetGame().GetCallqueue().CallLater(RefreshAllSpinBoxArrows, 0, false);',
    "tab create preview sync",
)

src = replace_once(
    src,
    '\t\tSyncHotasConfigSelector();\n\t\tSyncHudControls();\n\t\tm_bLoading = false;',
    '\t\tSyncHotasConfigSelector();\n\t\tSyncHudControls();\n\t\tSyncHudPositionPreviewFromController();\n\t\tm_bLoading = false;\n\t\tGetGame().GetCallqueue().CallLater(RefreshAllSpinBoxArrows, 0, false);',
    "tab show preview sync",
)

src = replace_once(
    src,
    '\t\tGetGame().GetCallqueue().Remove(UpdateHudPositionPreview);\n\t\tsuper.OnTabHide();',
    '\t\tGetGame().GetCallqueue().Remove(UpdateHudPositionPreview);\n\t\tGetGame().GetCallqueue().Remove(UpdateHudPositionDrag);\n\t\tif (m_bDraggingHudPosition)\n\t\t\tEndHudPositionDrag();\n\t\tsuper.OnTabHide();',
    "tab hide drag cleanup",
)

src = replace_once(
    src,
    '''\tprotected void SetupHudPositionPreview()
\t{
\t\tm_PreviewHost = m_wRoot.FindAnyWidget("HUDPreviewHost");
\t\tm_PreviewSquare = m_wRoot.FindAnyWidget("HUDPreviewSquare");
\t\tm_PreviewSquareBackground = m_wRoot.FindAnyWidget("HUDPreviewSquareBackground");
\t\tm_ScreenPreview = m_wRoot.FindAnyWidget("HUDScreenPreview");
\t\tm_ScreenPreviewBackground = m_wRoot.FindAnyWidget("HUDScreenPreviewBackground");
\t\tm_HudPositionPreview = m_wRoot.FindAnyWidget("HUDPositionPreview");
\t}
''',
    '''\tprotected void SetupHudPositionPreview()
\t{
\t\tm_PreviewHost = m_wRoot.FindAnyWidget("HUDPreviewHost");
\t\tm_PreviewSquare = m_wRoot.FindAnyWidget("HUDPreviewSquare");
\t\tm_PreviewSquareBackground = m_wRoot.FindAnyWidget("HUDPreviewSquareBackground");
\t\tm_ScreenPreview = m_wRoot.FindAnyWidget("HUDScreenPreview");
\t\tm_ScreenPreviewBackground = m_wRoot.FindAnyWidget("HUDScreenPreviewBackground");
\t\tm_HudPositionPreview = m_wRoot.FindAnyWidget("HUDPositionPreview");

\t\tif (m_HudPositionPreview)
\t\t{
\t\t\tm_HudDragHandler = new HOTASHudPositionDragHandler(this);
\t\t\tm_HudPositionPreview.AddHandler(m_HudDragHandler);
\t\t}
\t}

\tprotected void SyncHudPositionPreviewFromController()
\t{
\t\tHOTASDebugController.GetInstance().GetHudPositionNormalized(m_fPreviewPositionX, m_fPreviewPositionY);
\t}
''',
    "setup draggable preview",
)

src = replace_once(
    src,
    '\t\tint positionIndex = controller.GetSettingOptionIndex(1);\n\t\tfloat hudScale = 0.5 + controller.GetSettingOptionIndex(2) * 0.1;',
    '\t\tfloat hudScale = 0.5 + controller.GetSettingOptionIndex(1) * 0.1;',
    "preview scale index",
)

old_preview_position = '''\t\tfloat marginX = 48 * hudScale;
\t\tfloat marginY = 54 * hudScale;
\t\tfloat hudLeft = (screenWidth - hudWidth) * 0.5;
\t\tfloat hudTop = screenHeight - hudHeight - marginY;

\t\tswitch (positionIndex)
\t\t{
\t\t\tcase 0: hudLeft = marginX; hudTop = marginY; break;
\t\t\tcase 1: hudLeft = (screenWidth - hudWidth) * 0.5; hudTop = marginY; break;
\t\t\tcase 2: hudLeft = screenWidth - hudWidth - marginX; hudTop = marginY; break;
\t\t\tcase 3: hudLeft = marginX; hudTop = (screenHeight - hudHeight) * 0.5; break;
\t\t\tcase 4: hudLeft = (screenWidth - hudWidth) * 0.5; hudTop = (screenHeight - hudHeight) * 0.5; break;
\t\t\tcase 5: hudLeft = screenWidth - hudWidth - marginX; hudTop = (screenHeight - hudHeight) * 0.5; break;
\t\t\tcase 6: hudLeft = marginX; hudTop = screenHeight - hudHeight - marginY; break;
\t\t\tcase 7: hudLeft = (screenWidth - hudWidth) * 0.5; hudTop = screenHeight - hudHeight - marginY; break;
\t\t\tcase 8: hudLeft = screenWidth - hudWidth - marginX; hudTop = screenHeight - hudHeight - marginY; break;
\t\t}
'''
new_preview_position = '''\t\tfloat travelX = Math.Max(0.0, screenWidth - hudWidth);
\t\tfloat travelY = Math.Max(0.0, screenHeight - hudHeight);
\t\tfloat hudLeft = travelX * m_fPreviewPositionX;
\t\tfloat hudTop = travelY * m_fPreviewPositionY;
'''
src = replace_once(src, old_preview_position, new_preview_position, "preview exact position")

src = replace_once(
    src,
    '''\t\tarray<string> widgetNames = {
\t\t\t"HUDEnabled",
\t\t\t"HUDPosition",
\t\t\t"HUDScale",''',
    '''\t\tarray<string> widgetNames = {
\t\t\t"HUDEnabled",
\t\t\t"HUDScale",''',
    "remove position control mapping",
)

# Add stable arrow refresh after each initial current-item set.
src = replace_once(
    src,
    '\t\t\tcontrol.SetCurrentItem(controller.GetSettingOptionIndex(i), false, false, false);\n\t\t\tcontrol.m_OnChanged.Insert(OnHudSettingChanged);',
    '\t\t\tint currentIndex = controller.GetSettingOptionIndex(i);\n\t\t\tcontrol.SetCurrentItem(currentIndex, false, false, false);\n\t\t\tRefreshSpinBoxArrows(control, currentIndex, optionCount);\n\t\t\tcontrol.m_OnChanged.Insert(OnHudSettingChanged);',
    "setup arrow refresh",
)

src = replace_once(
    src,
    '''\t\t\tSCR_SpinBoxComponent control = m_HudControls[i];
\t\t\tif (control)
\t\t\t\tcontrol.SetCurrentItem(controller.GetSettingOptionIndex(i), false, false, false);''',
    '''\t\t\tSCR_SpinBoxComponent control = m_HudControls[i];
\t\t\tif (control)
\t\t\t{
\t\t\t\tint currentIndex = controller.GetSettingOptionIndex(i);
\t\t\t\tcontrol.SetCurrentItem(currentIndex, false, false, false);
\t\t\t\tRefreshSpinBoxArrows(control, currentIndex, controller.GetSettingOptionCount(i));
\t\t\t}''',
    "sync arrow refresh",
)

src = replace_once(
    src,
    '''\t\t\tHOTASDebugController.GetInstance().SetSettingOptionIndex(i, optionIndex);
\t\t\tUpdateHudPositionPreview();
\t\t\treturn;''',
    '''\t\t\tHOTASDebugController controller = HOTASDebugController.GetInstance();
\t\t\tcontroller.SetSettingOptionIndex(i, optionIndex);
\t\t\tRefreshSpinBoxArrows(component, optionIndex, controller.GetSettingOptionCount(i));
\t\t\tGetGame().GetCallqueue().CallLater(RefreshAllSpinBoxArrows, 0, false);
\t\t\tUpdateHudPositionPreview();
\t\t\treturn;''',
    "changed arrow refresh",
)

# Refresh config selector arrows after syncing and after changing the selected config.
src = replace_once(
    src,
    '\t\tm_HotasConfig.SetCurrentItem(selected, false, false, false);\n\t}',
    '\t\tm_HotasConfig.SetCurrentItem(selected, false, false, false);\n\t\tRefreshSpinBoxArrows(m_HotasConfig, selected, m_UserConfigs.Count() + 1);\n\t}',
    "config sync arrows",
)

# Insert drag and arrow helper methods immediately before SetupHudControls.
insert_marker = '\n\t//------------------------------------------------------------------------------------------------\n\tprotected void SetupHudControls()'
insert_at = src.index(insert_marker)
helpers = r'''

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

		// SCR_PagingButtonComponent hides BackgroundImage when disabled. For settings
		// selectors we want the normal disabled/grey arrow instead of a disappearing one.
		Widget background = buttonWidget.FindAnyWidget("BackgroundImage");
		if (background)
			background.SetVisible(true);

		buttonWidget.SetOpacity(enabled ? 1.0 : 0.35);
	}
'''
src = src[:insert_at] + helpers + src[insert_at:]

# Add a dedicated lightweight mouse handler before the SettingsSuperMenu override.
menu_marker = '\n//------------------------------------------------------------------------------------------------\n// Add HOTAS as a normal peer'
menu_at = src.index(menu_marker)
drag_handler = r'''

//------------------------------------------------------------------------------------------------
// Mouse handler for the orange HUD preview bar. Keeping this separate prevents the
// settings submenu's root handler from being rebound when the preview widget is hooked.
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
src = src[:menu_at] + drag_handler + src[menu_at:]

path.write_text(src, encoding="utf-8")


# -----------------------------------------------------------------------------
# Layout: remove the obsolete discrete Position spinbox and clarify the preview.
# -----------------------------------------------------------------------------
path = Path("UI/layouts/Menus/SettingsSubMenus/HOTASSettings.layout")
src = path.read_text(encoding="utf-8")

position_row = '''      ButtonWidgetClass "{8C52D9F7A31B6422}" : "{C9DF0E6590F6C388}UI/layouts/WidgetLibrary/SpinBox/WLib_SpinBox.layout" {
       Name "HUDPosition"
       Slot LayoutSlot "{8C52D9F7A31B6423}" { Padding 4 4 4 4 }
       components { SCR_SpinBoxComponent "{5472C6CBC0640458}" { m_sLabel "Position" m_sLabelLayout "{F003823FF141983C}UI/layouts/Menus/SettingsMenu/CustomWidgets/SettingsLabel.layout" m_bUseLightArrows 1 m_bShowHints 0 } }
      }
'''
src = replace_once(src, position_row, "", "remove Position row")
src = replace_once(src, 'm_sLabel "HUD Position Preview"', 'm_sLabel "HUD Position - Drag Orange Bar"', "preview title")
path.write_text(src, encoding="utf-8")
