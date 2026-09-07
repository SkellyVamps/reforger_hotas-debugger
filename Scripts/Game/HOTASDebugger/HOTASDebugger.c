class HOTASDebugController
{
	protected static ref HOTASDebugController s_Instance;

	protected InputManager m_InputManager;
	protected ref InputBinding m_InputBinding;
	protected RichTextWidget m_DebugText;
	protected Widget m_HudBackground;
	protected Widget m_HudLayoutRoot;
	protected Widget m_HudRootWidget;
	protected RichTextWidget m_InputText;
	protected RichTextWidget m_SeparatorText;
	protected RichTextWidget m_ActionText;
	protected bool m_bUsingLayoutHud;
	protected ref array<string> m_WatchedActions = {};
	protected bool m_bInitialized;
	protected bool m_bDebugMode = false;
	protected bool m_bHudEnabled = true;
	protected int m_iEventCounter;

	protected static const int HOTAS_CONTEXT_NONE = 0;
	protected static const int HOTAS_CONTEXT_TURRET = 1;
	protected static const int HOTAS_CONTEXT_HELICOPTER = 2;
	protected static const int HOTAS_CONTEXT_FIXED_WING = 3;

	// Normal HUD user settings. Values are loaded from $profile:HOTASHudSettings.txt.
	protected string m_sHudPosition = "bottom_center";
	protected float m_fHudScale = 1.0;
	protected int m_iFadeDelayMs = 1800;
	protected int m_iFadeDurationMs = 350;
	protected bool m_bBackgroundEnabled = true;
	protected float m_fBackgroundOpacity = 0.55;
	protected float m_fFadeOpacity = 1.0;

	// Axis assignments are discovered from the currently active HOTAS config.
	// The normalized binding key preserves the joystick slot so matching still works
	// when two physical devices both expose (for example) axis0.
	protected string m_sRollAxisBinding;
	protected string m_sPitchAxisBinding;
	protected string m_sThrottleAxisBinding;
	protected string m_sYawAxisBinding;
	protected int m_iRollAxis = -1;
	protected int m_iPitchAxis = -1;
	protected int m_iThrottleAxis = -1;
	protected int m_iYawAxis = -1;

	// Player-facing labels shown in the HUD for the discovered flight axes.
	protected string m_sRollAxisLabel = "Roll";
	protected string m_sPitchAxisLabel = "Pitch";
	protected string m_sThrottleAxisLabel = "Throttle";
	protected string m_sYawAxisLabel = "Yaw";

	static HOTASDebugController GetInstance()
	{
		if (!s_Instance)
			s_Instance = new HOTASDebugController();

		return s_Instance;
	}

	void Initialize()
	{
		if (m_bInitialized)
		{
			Print("[HOTAS Debugger] Reinitializing for new play session", LogLevel.NORMAL);
			Shutdown();
		}

		m_InputManager = GetGame().GetInputManager();
		if (!m_InputManager)
		{
			Print("[HOTAS Debugger] InputManager is not available", LogLevel.ERROR);
			return;
		}

		m_InputBinding = m_InputManager.CreateUserBinding();


		BuildActionList();
		LoadHudSettings();
		RefreshAssignedAxesFromBindings();
		CreateHud();
		RegisterListeners();

		m_bInitialized = true;
		Print("[HOTAS Debugger] Initialized", LogLevel.NORMAL);
	}

	void Shutdown()
	{
		if (!m_bInitialized)
			return;

		if (m_InputManager)
		{
			foreach (string actionName : m_WatchedActions)
				m_InputManager.RemoveActionListener(actionName, EActionTrigger.DOWN, OnActionTriggered);

		}

		GetGame().GetCallqueue().Remove(StartFade);
		GetGame().GetCallqueue().Remove(FadeStep);

		DestroyHud();
		m_InputBinding = null;
		m_InputManager = null;
		m_bInitialized = false;
	}


	protected void RegisterListeners()
	{
		foreach (string actionName : m_WatchedActions)
			m_InputManager.AddActionListener(actionName, EActionTrigger.DOWN, OnActionTriggered);

	}

	protected void DestroyHud()
	{
		GetGame().GetCallqueue().Remove(StartFade);
		GetGame().GetCallqueue().Remove(FadeStep);

		if (m_HudLayoutRoot)
			m_HudLayoutRoot.RemoveFromHierarchy();
		else
		{
			if (m_DebugText)
				m_DebugText.RemoveFromHierarchy();
			if (m_HudBackground)
				m_HudBackground.RemoveFromHierarchy();
		}

		m_DebugText = null;
		m_HudBackground = null;
		m_HudLayoutRoot = null;
		m_HudRootWidget = null;
		m_InputText = null;
		m_SeparatorText = null;
		m_ActionText = null;
		m_bUsingLayoutHud = false;
	}

	protected void RebuildHud()
	{
		DestroyHud();
		CreateHud();
	}


	protected void CreateHud()
	{
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace)
		{
			Print("[HOTAS Debugger] Workspace is not available", LogLevel.ERROR);
			return;
		}

		if (!m_bHudEnabled)
			return;

		// Normal mode prefers the Workbench-editable layout. Until the named widgets are
		// added in Layout Editor, we safely fall back to the script-created HUD below.
		if (!m_bDebugMode && TryCreateLayoutHud(workspace))
			return;

		int left;
		int top;
		int width;
		int height;
		int flags = WidgetFlags.VISIBLE | WidgetFlags.IGNORE_CURSOR | WidgetFlags.NOFOCUS | WidgetFlags.NO_LOCALIZATION;

		if (m_bDebugMode)
		{
			left = 40;
			top = 120;
			width = 900;
			height = 180;
			flags |= WidgetFlags.WRAP_TEXT;
		}
		else
		{
			// Keep the HUD wide enough for readable labels, but always inside the current viewport.
			int horizontalPadding = Math.Round(24 * m_fHudScale);
			width = Math.Min(Math.Round(1360 * m_fHudScale), workspace.GetWidth() - horizontalPadding);
			height = Math.Round(72 * m_fHudScale);
			GetHudPosition(workspace, width, height, left, top);
			flags |= WidgetFlags.CENTER | WidgetFlags.VCENTER;

			if (m_bBackgroundEnabled)
			{
				// PanelWidget is only a container and does not draw a visible fill by itself.
				// A full ProgressBar gives us a reliable colorable rectangle for the HUD backdrop.
				m_HudBackground = workspace.CreateWidgetInWorkspace(
					WidgetType.ProgressBarWidgetTypeID,
					left,
					top,
					width,
					height,
					WidgetFlags.VISIBLE | WidgetFlags.IGNORE_CURSOR | WidgetFlags.NOFOCUS,
					Color.FromInt(0xFF101418),
					999
				);
				ProgressBarWidget backgroundBar = ProgressBarWidget.Cast(m_HudBackground);
				if (backgroundBar)
				{
					backgroundBar.SetMin(0.0);
					backgroundBar.SetMax(1.0);
					backgroundBar.SetCurrent(1.0);
				}
				if (m_HudBackground)
					m_HudBackground.SetOpacity(m_fBackgroundOpacity);
			}
		}

		Widget widget = workspace.CreateWidgetInWorkspace(
			WidgetType.RichTextWidgetTypeID,
			left,
			top,
			width,
			height,
			flags,
			Color.White,
			1000
		);

		m_DebugText = RichTextWidget.Cast(widget);
		if (!m_DebugText)
		{
			Print("[HOTAS Debugger] Could not create TextWidget", LogLevel.ERROR);
			return;
		}

		if (m_bDebugMode)
		{
			m_DebugText.SetExactFontSize(24);
			m_DebugText.SetOutline(2, 0xFF000000);
			m_DebugText.SetTextWrapping(true);
			m_DebugText.SetText("HOTAS INPUT DEBUG\nWaiting for a watched input action...");
		}
		else
		{
			m_DebugText.SetExactFontSize(Math.Round(26 * m_fHudScale));
			m_DebugText.SetOutline(Math.Max(1, Math.Round(2 * m_fHudScale)), 0xF0000000);
			m_DebugText.SetShadow(Math.Max(1, Math.Round(2 * m_fHudScale)), 0xC0000000, 1.0, 2, 2);
			m_DebugText.SetTextWrapping(false);
			m_DebugText.SetText("");
			m_DebugText.SetOpacity(0.0);
			if (m_HudBackground)
				m_HudBackground.SetOpacity(0.0);
		}

		m_DebugText.SetBold(true);
	}

	protected bool TryCreateLayoutHud(WorkspaceWidget workspace)
	{
		ResourceName hudLayout = "{25F3F1C1A41EA7E1}UI/layouts/HUD/HOTAS/HOTASInputHUD.layout";
		m_HudLayoutRoot = workspace.CreateWidgets(hudLayout);
		if (!m_HudLayoutRoot)
		{
			Print("[HOTAS Debugger] Could not load HOTASInputHUD.layout; using script HUD", LogLevel.WARNING);
			return false;
		}

		m_HudRootWidget = m_HudLayoutRoot.FindAnyWidget("HudRoot");
		m_InputText = RichTextWidget.Cast(m_HudLayoutRoot.FindAnyWidget("InputText"));
		m_SeparatorText = RichTextWidget.Cast(m_HudLayoutRoot.FindAnyWidget("SeparatorText"));
		m_ActionText = RichTextWidget.Cast(m_HudLayoutRoot.FindAnyWidget("ActionText"));
		m_HudBackground = m_HudLayoutRoot.FindAnyWidget("BackgroundImage");
		if (!m_HudBackground)
			m_HudBackground = m_HudLayoutRoot.FindAnyWidget("Background");

		if (!m_HudRootWidget || !m_InputText || !m_SeparatorText || !m_ActionText)
		{
			Print("[HOTAS Debugger] HOTASInputHUD.layout is present but needs named RichText widgets: InputText, SeparatorText, ActionText. Using script HUD until the layout is ready.", LogLevel.WARNING);
			m_HudLayoutRoot.RemoveFromHierarchy();
			m_HudLayoutRoot = null;
			m_HudRootWidget = null;
			m_InputText = null;
			m_SeparatorText = null;
			m_ActionText = null;
			m_HudBackground = null;
			return false;
		}

		m_bUsingLayoutHud = true;
		m_SeparatorText.SetText("|");
		ApplyLayoutHudPresentation();
		m_HudLayoutRoot.SetOpacity(0.0);

		Print("[HOTAS Debugger] Using Workbench-editable HOTASInputHUD.layout", LogLevel.NORMAL);
		return true;
	}

	protected void ApplyLayoutHudPresentation()
	{
		if (!m_HudRootWidget || !m_InputText || !m_SeparatorText || !m_ActionText)
			return;

		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace)
			return;

		int width = Math.Round(700 * m_fHudScale);
		int height = Math.Round(70 * m_fHudScale);
		int left;
		int top;
		GetHudPosition(workspace, width, height, left, top);

		FrameSlot.SetPos(m_HudRootWidget, left, top);
		FrameSlot.SetSize(m_HudRootWidget, width, height);

		int fontSize = Math.Round(26 * m_fHudScale);
		m_InputText.SetExactFontSize(fontSize);
		m_SeparatorText.SetExactFontSize(fontSize);
		m_ActionText.SetExactFontSize(fontSize);

		if (m_HudBackground)
		{
			if (m_bBackgroundEnabled)
				m_HudBackground.SetOpacity(m_fBackgroundOpacity);
			else
				m_HudBackground.SetOpacity(0.0);
		}
	}

	protected void GetHudPosition(WorkspaceWidget workspace, int width, int height, out int left, out int top)
	{
		int marginX = Math.Round(48 * m_fHudScale);
		int marginY = Math.Round(54 * m_fHudScale);
		int screenWidth = workspace.GetWidth();
		int screenHeight = workspace.GetHeight();

		left = (screenWidth - width) / 2;
		top = screenHeight - height - marginY;

		if (m_sHudPosition == "top_left")
		{
			left = marginX;
			top = marginY;
		}
		else if (m_sHudPosition == "top_center")
		{
			left = (screenWidth - width) / 2;
			top = marginY;
		}
		else if (m_sHudPosition == "top_right")
		{
			left = screenWidth - width - marginX;
			top = marginY;
		}
		else if (m_sHudPosition == "center_left")
		{
			left = marginX;
			top = (screenHeight - height) / 2;
		}
		else if (m_sHudPosition == "center")
		{
			left = (screenWidth - width) / 2;
			top = (screenHeight - height) / 2;
		}
		else if (m_sHudPosition == "center_right")
		{
			left = screenWidth - width - marginX;
			top = (screenHeight - height) / 2;
		}
		else if (m_sHudPosition == "bottom_left")
		{
			left = marginX;
			top = screenHeight - height - marginY;
		}
		else if (m_sHudPosition == "bottom_right")
		{
			left = screenWidth - width - marginX;
			top = screenHeight - height - marginY;
		}
	}

	protected void LoadHudSettings()
	{
		string settingsPath = "$profile:HOTASHudSettings.txt";
		if (!FileIO.FileExists(settingsPath))
		{
			FileHandle defaults = FileIO.OpenFile(settingsPath, FileMode.WRITE);
			if (defaults)
			{
				defaults.WriteLine("# HOTAS Input HUD settings");
				defaults.WriteLine("hud_enabled=1");
				defaults.WriteLine("debug_mode=0");
				defaults.WriteLine("# position: top_left, top_center, top_right, center_left, center, center_right, bottom_left, bottom_center, bottom_right");
				defaults.WriteLine("position=bottom_center");
				defaults.WriteLine("scale=1.0");
				defaults.WriteLine("fade_delay_ms=1800");
				defaults.WriteLine("fade_duration_ms=350");
				defaults.WriteLine("background=1");
				defaults.WriteLine("background_opacity=0.55");
				defaults.WriteLine("# Axis numbers are detected from the active HOTAS config. These are only display labels.");
				defaults.WriteLine("roll_label=Roll");
				defaults.WriteLine("pitch_label=Pitch");
				defaults.WriteLine("throttle_label=Throttle");
				defaults.WriteLine("yaw_label=Yaw");
				defaults.Close();
			}
		}

		FileHandle file = FileIO.OpenFile(settingsPath, FileMode.READ);
		if (!file)
			return;

		string line;
		while (file.ReadLine(line) >= 0)
		{
			line = line.Trim();
			if (line.IsEmpty() || line.StartsWith("#"))
				continue;

			ref array<string> parts = {};
			line.Split("=", parts, false);
			if (parts.Count() < 2)
				continue;

			string key = parts[0].Trim();
			string value = parts[1].Trim();
			if (key == "hud_enabled")
				m_bHudEnabled = value.ToInt(1) != 0;
			else if (key == "debug_mode")
				m_bDebugMode = value.ToInt(0) != 0;
			else if (key == "position")
				m_sHudPosition = value;
			else if (key == "scale")
				m_fHudScale = Math.Clamp(value.ToFloat(1.0), 0.6, 2.0);
			else if (key == "fade_delay_ms")
				m_iFadeDelayMs = Math.ClampInt(value.ToInt(1800), 0, 10000);
			else if (key == "fade_duration_ms")
				m_iFadeDurationMs = Math.ClampInt(value.ToInt(350), 0, 5000);
			else if (key == "background")
				m_bBackgroundEnabled = value.ToInt(1) != 0;
			else if (key == "background_opacity")
				m_fBackgroundOpacity = Math.Clamp(value.ToFloat(0.55), 0.0, 1.0);
			else if (key == "roll_label")
				m_sRollAxisLabel = value;
			else if (key == "pitch_label")
				m_sPitchAxisLabel = value;
			else if (key == "throttle_label")
				m_sThrottleAxisLabel = value;
			else if (key == "yaw_label")
				m_sYawAxisLabel = value;
		}
		file.Close();

		Print(string.Format("[HOTAS Debugger] HUD settings: position=%1 scale=%2 fade=%3/%4 background=%5 opacity=%6", m_sHudPosition, m_fHudScale, m_iFadeDelayMs, m_iFadeDurationMs, m_bBackgroundEnabled, m_fBackgroundOpacity), LogLevel.NORMAL);
		Print(string.Format("[HOTAS Debugger] Axis mapping: roll=%1 pitch=%2 throttle=%3 yaw=%4", m_iRollAxis, m_iPitchAxis, m_iThrottleAxis, m_iYawAxis), LogLevel.NORMAL);
	}

	protected int BoolToInt(bool value)
	{
		if (value)
			return 1;
		return 0;
	}

	protected void SaveHudSettings()
	{
		FileHandle file = FileIO.OpenFile("$profile:HOTASHudSettings.txt", FileMode.WRITE);
		if (!file)
			return;

		file.WriteLine("# HOTAS Input HUD settings");
		file.WriteLine(string.Format("hud_enabled=%1", BoolToInt(m_bHudEnabled)));
		file.WriteLine(string.Format("debug_mode=%1", BoolToInt(m_bDebugMode)));
		file.WriteLine(string.Format("position=%1", m_sHudPosition));
		file.WriteLine(string.Format("scale=%1", m_fHudScale));
		file.WriteLine(string.Format("fade_delay_ms=%1", m_iFadeDelayMs));
		file.WriteLine(string.Format("fade_duration_ms=%1", m_iFadeDurationMs));
		file.WriteLine(string.Format("background=%1", BoolToInt(m_bBackgroundEnabled)));
		file.WriteLine(string.Format("background_opacity=%1", m_fBackgroundOpacity));
		file.WriteLine("# Axis numbers are detected from the active HOTAS config. These are only display labels.");
		file.WriteLine(string.Format("roll_label=%1", m_sRollAxisLabel));
		file.WriteLine(string.Format("pitch_label=%1", m_sPitchAxisLabel));
		file.WriteLine(string.Format("throttle_label=%1", m_sThrottleAxisLabel));
		file.WriteLine(string.Format("yaw_label=%1", m_sYawAxisLabel));
		file.Close();
	}


	void ReloadHudSettings()
	{
		LoadHudSettings();
		RefreshAssignedAxesFromBindings();
	}

	void RefreshAssignedAxes()
	{
		RefreshAssignedAxesFromBindings();
	}

	int GetAxisAssignmentRaw(int axisIndex)
	{
		switch (axisIndex)
		{
			case 0: return m_iRollAxis;
			case 1: return m_iPitchAxis;
			case 2: return m_iThrottleAxis;
			case 3: return m_iYawAxis;
		}
		return -1;
	}

	string GetAxisAssignmentDisplayName(int axisIndex)
	{
		int rawAxis = GetAxisAssignmentRaw(axisIndex);
		if (rawAxis < 0)
			return "Unassigned";
		return string.Format("Axis %1", rawAxis + 1);
	}

	string GetAxisSettingRowLabel(int axisIndex)
	{
		string logicalName;
		switch (axisIndex)
		{
			case 0: logicalName = "Roll Axis"; break;
			case 1: logicalName = "Pitch Axis"; break;
			case 2: logicalName = "Throttle Axis"; break;
			case 3: logicalName = "Yaw Axis"; break;
			default: logicalName = "Axis"; break;
		}

		return string.Format("%1 - %2", logicalName, GetAxisAssignmentDisplayName(axisIndex));
	}

	string GetAxisCustomLabel(int axisIndex)
	{
		switch (axisIndex)
		{
			case 0: return m_sRollAxisLabel;
			case 1: return m_sPitchAxisLabel;
			case 2: return m_sThrottleAxisLabel;
			case 3: return m_sYawAxisLabel;
		}
		return string.Empty;
	}

	void SetAxisCustomLabel(int axisIndex, string value)
	{
		value = value.Trim();
		switch (axisIndex)
		{
			case 0: m_sRollAxisLabel = value; break;
			case 1: m_sPitchAxisLabel = value; break;
			case 2: m_sThrottleAxisLabel = value; break;
			case 3: m_sYawAxisLabel = value; break;
			default: return;
		}
		SaveHudSettings();
	}

	protected string NormalizeAxisBinding(string binding)
	{
		int axisPos = binding.IndexOf(":axis");
		if (axisPos < 0)
			return string.Empty;

		string normalized = binding;
		if (normalized.EndsWith("+") || normalized.EndsWith("-"))
			normalized = normalized.Substring(0, normalized.Length() - 1);
		return normalized;
	}

	protected int GetRawAxisFromBinding(string binding)
	{
		int axisPos = binding.IndexOf(":axis");
		if (axisPos < 0)
			return -1;

		string axisText = binding.Substring(axisPos + 5, binding.Length() - axisPos - 5);
		if (axisText.EndsWith("+") || axisText.EndsWith("-"))
			axisText = axisText.Substring(0, axisText.Length() - 1);
		return axisText.ToInt(-1);
	}

	protected string GetAxisBindingFromAction(string actionName)
	{
		string bindingsText = GetJoystickBindings(actionName);
		ref array<string> bindings = {};
		bindingsText.Split(" / ", bindings, true);
		foreach (string binding : bindings)
		{
			string normalized = NormalizeAxisBinding(binding);
			if (!normalized.IsEmpty())
				return normalized;
		}
		return string.Empty;
	}

	protected string ResolveAxisBindingFromActions(string actionA, string actionB, string actionC)
	{
		string binding = GetAxisBindingFromAction(actionA);
		if (!binding.IsEmpty())
			return binding;

		binding = GetAxisBindingFromAction(actionB);
		if (!binding.IsEmpty())
			return binding;

		return GetAxisBindingFromAction(actionC);
	}

	protected void RefreshAssignedAxesFromBindings()
	{
		m_sRollAxisBinding = ResolveAxisBindingFromActions("PFC_Roll", "HelicopterCyclicLeft", "HelicopterCyclicRight");
		m_sPitchAxisBinding = ResolveAxisBindingFromActions("PFC_Pitch", "HelicopterCyclicForward", "HelicopterCyclicBack");
		m_sThrottleAxisBinding = ResolveAxisBindingFromActions("PFC_ThrottleAxis", "HelicopterCollectiveIncrease", "HelicopterCollectiveDecrease");
		m_sYawAxisBinding = ResolveAxisBindingFromActions("PFC_Yaw", "HelicopterAntiTorqueLeft", "HelicopterAntiTorqueRight");

		m_iRollAxis = GetRawAxisFromBinding(m_sRollAxisBinding);
		m_iPitchAxis = GetRawAxisFromBinding(m_sPitchAxisBinding);
		m_iThrottleAxis = GetRawAxisFromBinding(m_sThrottleAxisBinding);
		m_iYawAxis = GetRawAxisFromBinding(m_sYawAxisBinding);

		Print(string.Format("[HOTAS Debugger] Config axis assignments: roll=%1 pitch=%2 throttle=%3 yaw=%4", GetAxisAssignmentDisplayName(0), GetAxisAssignmentDisplayName(1), GetAxisAssignmentDisplayName(2), GetAxisAssignmentDisplayName(3)), LogLevel.NORMAL);
	}

	// Settings-tab slider values are human-facing percentages. HUD scale maps
	// 0% -> 0.6x and 100% -> 2.0x, while opacity maps directly to 0..1.
	float GetHudScalePercent()
	{
		return Math.Clamp(((m_fHudScale - 0.6) / 1.4) * 100.0, 0.0, 100.0);
	}

	void SetHudScalePercent(float percent)
	{
		percent = Math.Clamp(percent, 0.0, 100.0);
		m_fHudScale = 0.6 + (percent / 100.0) * 1.4;
		SaveHudSettings();
		if (m_bInitialized)
			RebuildHud();
	}

	float GetBackgroundOpacityPercent()
	{
		return Math.Clamp(m_fBackgroundOpacity * 100.0, 0.0, 100.0);
	}

	void SetBackgroundOpacityPercent(float percent)
	{
		percent = Math.Clamp(percent, 0.0, 100.0);
		m_fBackgroundOpacity = percent / 100.0;
		SaveHudSettings();
		if (m_bInitialized)
			RebuildHud();
	}

	int GetSettingsCount()
	{
		return 12;
	}

	string GetSettingLabel(int index)
	{
		switch (index)
		{
			case 0: return "HUD Enabled";
			case 1: return "Position";
			case 2: return "Scale";
			case 3: return "Fade Delay";
			case 4: return "Fade Duration";
			case 5: return "Background";
			case 6: return "Background Opacity";
			case 7: return "Roll Axis";
			case 8: return "Pitch Axis";
			case 9: return "Throttle Axis";
			case 10: return "Yaw Axis";
			case 11: return "Debug Mode";
		}
		return "Unknown";
	}

	protected int GetHudPositionIndex()
	{
		switch (m_sHudPosition)
		{
			case "top_left": return 0;
			case "top_center": return 1;
			case "top_right": return 2;
			case "center_left": return 3;
			case "center": return 4;
			case "center_right": return 5;
			case "bottom_left": return 6;
			case "bottom_center": return 7;
			case "bottom_right": return 8;
		}
		return 7;
	}

	protected void SetHudPositionIndex(int index)
	{
		index = Math.ClampInt(index, 0, 8);
		switch (index)
		{
			case 0: m_sHudPosition = "top_left"; break;
			case 1: m_sHudPosition = "top_center"; break;
			case 2: m_sHudPosition = "top_right"; break;
			case 3: m_sHudPosition = "center_left"; break;
			case 4: m_sHudPosition = "center"; break;
			case 5: m_sHudPosition = "center_right"; break;
			case 6: m_sHudPosition = "bottom_left"; break;
			case 7: m_sHudPosition = "bottom_center"; break;
			case 8: m_sHudPosition = "bottom_right"; break;
		}
	}

	int GetSettingOptionCount(int index)
	{
		switch (index)
		{
			case 0: return 2;
			case 1: return 9;
			case 2: return 15;
			case 3: return 101;
			case 4: return 101;
			case 5: return 2;
			case 6: return 21;
			case 7:
			case 8:
			case 9:
			case 10: return 0;
			case 11: return 2;
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
			case 1: return GetHudPositionIndex();
			case 2: return Math.ClampInt(Math.Round((m_fHudScale - 0.6) * 10.0), 0, 14);
			case 3: return Math.ClampInt(Math.Round(m_iFadeDelayMs / 100.0), 0, 100);
			case 4: return Math.ClampInt(Math.Round(m_iFadeDurationMs / 50.0), 0, 100);
			case 5:
				if (m_bBackgroundEnabled) return 1;
				return 0;
			case 6: return Math.ClampInt(Math.Round(m_fBackgroundOpacity * 20.0), 0, 20);
			case 7:
			case 8:
			case 9:
			case 10: return 0;
			case 11:
				if (m_bDebugMode) return 1;
				return 0;
		}
		return 0;
	}

	protected string GetHudPositionOptionLabel(int index)
	{
		switch (index)
		{
			case 0: return "Top Left";
			case 1: return "Top Center";
			case 2: return "Top Right";
			case 3: return "Center Left";
			case 4: return "Center";
			case 5: return "Center Right";
			case 6: return "Bottom Left";
			case 7: return "Bottom Center";
			case 8: return "Bottom Right";
		}
		return "Bottom Center";
	}

	string GetSettingOptionLabel(int index, int optionIndex)
	{
		optionIndex = Math.ClampInt(optionIndex, 0, Math.Max(0, GetSettingOptionCount(index) - 1));
		switch (index)
		{
			case 0:
			case 5:
			case 11:
				if (optionIndex > 0) return "On";
				return "Off";
			case 1: return GetHudPositionOptionLabel(optionIndex);
			case 2: return string.Format("%1x", (0.6 + optionIndex * 0.1).ToString(1));
			case 3: return string.Format("%1 ms", optionIndex * 100);
			case 4: return string.Format("%1 ms", optionIndex * 50);
			case 6: return string.Format("%1%", optionIndex * 5);
			case 7:
			case 8:
			case 9:
			case 10:
				return GetAxisAssignmentDisplayName(index - 7);
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
			case 1: SetHudPositionIndex(optionIndex); break;
			case 2: m_fHudScale = 0.6 + optionIndex * 0.1; break;
			case 3: m_iFadeDelayMs = optionIndex * 100; break;
			case 4: m_iFadeDurationMs = optionIndex * 50; break;
			case 5: m_bBackgroundEnabled = optionIndex != 0; break;
			case 6: m_fBackgroundOpacity = optionIndex * 0.05; break;
			case 7:
			case 8:
			case 9:
			case 10: return;
			case 11: m_bDebugMode = optionIndex != 0; break;
			default: return;
		}

		SaveHudSettings();
		if (m_bInitialized)
			RebuildHud();
	}

	protected void ShowHud()
	{
		if (!m_bHudEnabled)
			return;
		if (m_bDebugMode)
			return;
		if (m_bUsingLayoutHud && !m_HudLayoutRoot)
			return;
		if (!m_bUsingLayoutHud && !m_DebugText)
			return;

		ScriptCallQueue queue = GetGame().GetCallqueue();
		queue.Remove(StartFade);
		queue.Remove(FadeStep);
		m_fFadeOpacity = 1.0;

		if (m_bUsingLayoutHud)
			m_HudLayoutRoot.SetOpacity(1.0);
		else
		{
			m_DebugText.SetOpacity(1.0);
			if (m_HudBackground)
				m_HudBackground.SetOpacity(m_fBackgroundOpacity);
		}

		queue.CallLater(StartFade, m_iFadeDelayMs, false);
	}

	protected void StartFade()
	{
		if (m_bDebugMode)
			return;
		if (m_bUsingLayoutHud && !m_HudLayoutRoot)
			return;
		if (!m_bUsingLayoutHud && !m_DebugText)
			return;

		if (m_iFadeDurationMs <= 0)
		{
			if (m_bUsingLayoutHud)
				m_HudLayoutRoot.SetOpacity(0.0);
			else
			{
				m_DebugText.SetOpacity(0.0);
				if (m_HudBackground)
					m_HudBackground.SetOpacity(0.0);
			}
			return;
		}

		m_fFadeOpacity = 1.0;
		GetGame().GetCallqueue().CallLater(FadeStep, 50, true);
	}

	protected void FadeStep()
	{
		if (m_bUsingLayoutHud && !m_HudLayoutRoot)
		{
			GetGame().GetCallqueue().Remove(FadeStep);
			return;
		}
		if (!m_bUsingLayoutHud && !m_DebugText)
		{
			GetGame().GetCallqueue().Remove(FadeStep);
			return;
		}

		m_fFadeOpacity -= 50.0 / m_iFadeDurationMs;
		if (m_fFadeOpacity <= 0.0)
		{
			m_fFadeOpacity = 0.0;
			GetGame().GetCallqueue().Remove(FadeStep);
		}

		if (m_bUsingLayoutHud)
			m_HudLayoutRoot.SetOpacity(m_fFadeOpacity);
		else
		{
			m_DebugText.SetOpacity(m_fFadeOpacity);
			if (m_HudBackground)
				m_HudBackground.SetOpacity(m_fBackgroundOpacity * m_fFadeOpacity);
		}
	}

	protected int GetPlayerHotasContext()
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		if (!character)
			return HOTAS_CONTEXT_NONE;

		CompartmentAccessComponent compartmentAccess = character.GetCompartmentAccessComponent();
		if (!compartmentAccess || !compartmentAccess.IsInCompartment())
			return HOTAS_CONTEXT_NONE;

		BaseCompartmentSlot slot = compartmentAccess.GetCompartment();
		if (!slot)
			return HOTAS_CONTEXT_NONE;

		if (TurretCompartmentSlot.Cast(slot))
			return HOTAS_CONTEXT_TURRET;

		if (!PilotCompartmentSlot.Cast(slot))
			return HOTAS_CONTEXT_NONE;

		IEntity vehicle = compartmentAccess.GetVehicleCompartmentManagerOwner();
		if (!vehicle)
			vehicle = slot.GetOwner();
		if (!vehicle)
			return HOTAS_CONTEXT_NONE;

		if (vehicle.FindComponent(HelicopterControllerComponent))
			return HOTAS_CONTEXT_HELICOPTER;

		if (vehicle.FindComponent(SCR_CarControllerComponent))
			return HOTAS_CONTEXT_NONE;
		if (vehicle.FindComponent(SCR_TrackedControllerComponent))
			return HOTAS_CONTEXT_NONE;

		// A pilot seat without a known ground-vehicle controller is treated as fixed-wing.
		// This keeps compatibility with PFC and other modded aircraft controllers.
		return HOTAS_CONTEXT_FIXED_WING;
	}

	protected bool IsSharedHotasAction(string actionName)
	{
		return actionName == "VehicleDoorToggle"
			|| actionName == "FocusToggle"
			|| actionName == "Freelook"
			|| actionName == "FreelookReset"
			|| actionName == "FreelookUp"
			|| actionName == "FreelookDown"
			|| actionName == "FreelookLeft"
			|| actionName == "FreelookRight"
			|| actionName == "VONDirectToggle"
			|| actionName == "VONChannel"
			|| actionName == "GadgetMap"
			|| actionName == "PerformAction"
			|| actionName == "SelectAction"
			|| actionName == "GetOut"
			|| actionName == "JumpOut";
	}

	protected bool IsAircraftWeaponAction(string actionName)
	{
		return actionName == "VehicleFire"
			|| actionName == "VehicleNextWeapon"
			|| actionName == "TurretFire"
			|| actionName == "TurretReload"
			|| actionName == "TurretNextWeapon"
			|| actionName == "TurretWeaponNextFireMode"
			|| actionName == "TurretWeaponNextRippleQuantity"
			|| actionName == "TurretADS"
			|| actionName == "TurretADSHold"
			|| actionName == "WeaponToggleSightsIllumination"
			|| actionName == "WeaponSwitchOptics";
	}

	protected bool IsAircraftWcsAction(string actionName)
	{
		if (!actionName.StartsWith("WCS_Armament_"))
			return false;

		// Ground-only smoke/stabilization actions should not appear while flying.
		if (actionName == "WCS_Armament_TurretStabilizationToggle")
			return false;
		if (actionName == "WCS_Armament_DeploySmoke")
			return false;
		if (actionName == "WCS_Armament_FireContinuousSmokeDispenser")
			return false;

		return true;
	}

	protected bool IsTurretWcsAction(string actionName)
	{
		if (!actionName.StartsWith("WCS_Armament_"))
			return false;

		// Countermeasure actions are aircraft-specific.
		if (actionName == "WCS_Armament_DeployFlares")
			return false;
		if (actionName == "WCS_Armament_DeployChaffs")
			return false;

		return true;
	}

	protected bool IsActionAllowedForContext(string actionName, int context)
	{
		if (IsSharedHotasAction(actionName))
			return true;

		if (context == HOTAS_CONTEXT_TURRET)
		{
			if (actionName.StartsWith("Turret"))
				return true;
			if (actionName.StartsWith("Weapon"))
				return true;
			if (actionName == "VehicleFire" || actionName == "VehicleNextWeapon")
				return true;
			return IsTurretWcsAction(actionName);
		}

		if (context == HOTAS_CONTEXT_HELICOPTER)
		{
			if (actionName.StartsWith("Helicopter"))
				return true;
			if (IsAircraftWeaponAction(actionName))
				return true;
			return IsAircraftWcsAction(actionName);
		}

		if (context == HOTAS_CONTEXT_FIXED_WING)
		{
			if (actionName.StartsWith("PFC_"))
				return true;
			if (IsAircraftWeaponAction(actionName))
				return true;
			return IsAircraftWcsAction(actionName);
		}

		return false;
	}

	protected void OnActionTriggered(float value = 0.0, EActionTrigger reason = 0, string actionName = string.Empty)
	{
		if (actionName.IsEmpty())
			return;

		if (!m_bHudEnabled)
			return;

		int hotasContext = GetPlayerHotasContext();
		if (hotasContext == HOTAS_CONTEXT_NONE)
			return;
		if (!IsActionAllowedForContext(actionName, hotasContext))
			return;

		m_iEventCounter++;

		string bindingsText = GetJoystickBindings(actionName);
		string readableAction = MakeReadableActionName(actionName);
		if (m_bDebugMode)
		{
			string output = string.Format(
				"HOTAS INPUT DEBUG  #%1\nInput: %2\nAction: %3\nRaw action: %4\nValue: %5",
				m_iEventCounter,
				bindingsText,
				readableAction,
				actionName,
				value.ToString(2)
			);
			if (m_DebugText)
				m_DebugText.SetText(output);
		}
		else if (m_bUsingLayoutHud)
		{
			m_InputText.SetText(MakeReadableBinding(bindingsText));
			m_SeparatorText.SetText("|");
			m_ActionText.SetText(readableAction);
			ShowHud();
		}
		else if (m_DebugText)
		{
			string output = string.Format("<color rgba=\"226,167,80,255\">%1</color> | <color rgba=\"255,255,255,255\">%2</color>", MakeReadableBinding(bindingsText), readableAction);
			m_DebugText.SetText(output);
			ShowHud();
		}

		Print(string.Format("[HOTAS Debugger] %1 | %2 | value=%3", actionName, bindingsText, value), LogLevel.NORMAL);
	}

	protected string GetJoystickBindings(string actionName)
	{
		if (!m_InputManager)
			return "InputManager unavailable";

		string joystickBindings;

		// Query the active runtime ActionManager first. The separate InputBinding object can
		// report zero bindings even while a custom joystick config is actively driving actions.
		for (int bindIndex = 0; bindIndex < 16; bindIndex++)
		{
			ref array<string> keyStack = {};
			ref array<BaseContainer> filterStack = {};
			bool found = m_InputManager.GetActionKeybinding(
				actionName,
				keyStack,
				filterStack,
				EInputDeviceType.JOYSTICK,
				string.Empty,
				bindIndex
			);

			if (!found)
				break;

			foreach (string binding : keyStack)
			{
				if (!joystickBindings.IsEmpty())
					joystickBindings += " / ";

				joystickBindings += binding;
			}
		}

		if (!joystickBindings.IsEmpty())
			return joystickBindings;

		// Fallback for actions where the runtime manager does not expose an indexed binding.
		ref array<string> keyStackFallback = {};
		ref array<BaseContainer> filterStackFallback = {};
		if (m_InputManager.GetActionKeybinding(actionName, keyStackFallback, filterStackFallback, EInputDeviceType.JOYSTICK, string.Empty, -1))
		{
			foreach (string binding : keyStackFallback)
			{
				if (!joystickBindings.IsEmpty())
					joystickBindings += " / ";

				joystickBindings += binding;
			}
		}

		if (!joystickBindings.IsEmpty())
			return joystickBindings;

		// Final compatibility fallback to InputBinding.
		if (m_InputBinding)
		{
			ref array<string> bindings = {};
			if (m_InputBinding.GetBindings(actionName, bindings, EInputDeviceType.JOYSTICK, string.Empty, false))
			{
				foreach (string binding : bindings)
				{
					if (!joystickBindings.IsEmpty())
						joystickBindings += " / ";

					joystickBindings += binding;
				}
			}
		}

		if (joystickBindings.IsEmpty())
			return "Non-Joystick Input";

		return joystickBindings;
	}

	protected string MakeReadableBinding(string bindingsText)
	{
		ref array<string> bindings = {};
		bindingsText.Split(" / ", bindings, true);
		string result;

		foreach (string binding : bindings)
		{
			string readable = binding;
			int buttonPos = binding.IndexOf(":button");
			int axisPos = binding.IndexOf(":axis");
			if (buttonPos >= 0)
			{
				int number = binding.Substring(buttonPos + 7, binding.Length() - buttonPos - 7).ToInt() + 1;
				readable = string.Format("BUTTON %1", number);
			}
			else if (axisPos >= 0)
			{
				string axisText = binding.Substring(axisPos + 5, binding.Length() - axisPos - 5);
				string direction;
				if (axisText.EndsWith("+"))
					direction = "+";
				else if (axisText.EndsWith("-"))
					direction = "-";

				int rawAxis = axisText.ToInt();
				string normalizedBinding = NormalizeAxisBinding(binding);
				string axisName;
				if (!m_sRollAxisBinding.IsEmpty() && normalizedBinding == m_sRollAxisBinding)
					axisName = m_sRollAxisLabel;
				else if (!m_sPitchAxisBinding.IsEmpty() && normalizedBinding == m_sPitchAxisBinding)
					axisName = m_sPitchAxisLabel;
				else if (!m_sThrottleAxisBinding.IsEmpty() && normalizedBinding == m_sThrottleAxisBinding)
					axisName = m_sThrottleAxisLabel;
				else if (!m_sYawAxisBinding.IsEmpty() && normalizedBinding == m_sYawAxisBinding)
					axisName = m_sYawAxisLabel;

				if (!axisName.IsEmpty())
					readable = string.Format("%1 %2", axisName, direction);
				else
					readable = string.Format("AXIS %1%2", rawAxis + 1, direction);
			}

			if (!result.IsEmpty())
				result += " / ";
			result += readable;
		}

		return result;
	}

	protected string MakeReadableActionName(string actionName)
	{
		switch (actionName)
		{
			case "CharacterNextWeapon": return "Next Weapon";
			case "TurretNextWeapon": return "Next Weapon";
			case "TurretWeaponNextRippleQuantity": return "Missile Ripple";
			case "TurretWeaponNextFireMode": return "Next Fire Mode";
			case "TurretReload": return "Reload";
			case "TurretFire": return "Fire";
			case "HelicopterCyclicForward": return "Cyclic Forward";
			case "HelicopterCyclicBack": return "Cyclic Back";
			case "HelicopterCyclicLeft": return "Cyclic Left";
			case "HelicopterCyclicRight": return "Cyclic Right";
			case "HelicopterAntiTorqueLeft": return "Pedal Left";
			case "HelicopterAntiTorqueRight": return "Pedal Right";
			case "HelicopterCollectiveIncrease": return "Collective Up";
			case "HelicopterCollectiveDecrease": return "Collective Down";
			case "HelicopterWheelBrake": return "Wheel Brake";
			case "HelicopterWheelBrakePersistent": return "Parking Brake";
			case "HelicopterAutohoverToggle": return "Auto Hover";
			case "HelicopterLightsTaxiToggle": return "Taxi Lights";
			case "HelicopterLightsLandingToggle": return "Landing Lights";
			case "HelicopterEngineStart": return "Engine Start";
			case "HelicopterEngineStop": return "Engine Stop";
			case "HelicopterFire": return "Fire";
			case "HelicopterSightDeploy": return "Deploy Sight";
			case "HelicopterSightZeroing": return "Sight Zeroing";
			case "VehicleDoorToggle": return "Toggle Door";
			case "PerformAction": return "Use / Confirm";
			case "SelectAction": return "Select Action";
			case "GadgetMap": return "Map";
			case "Freelook": return "Freelook";
			case "FreelookReset": return "Center View";
			case "FreelookUp": return "Look Up";
			case "FreelookDown": return "Look Down";
			case "FreelookLeft": return "Look Left";
			case "FreelookRight": return "Look Right";
			case "FocusToggle": return "Focus";
			case "VONChannel": return "Voice Channel";
			case "VONDirectToggle": return "Direct Voice";
			case "PFC_Pitch": return "Pitch";
			case "PFC_Roll": return "Roll";
			case "PFC_Yaw": return "Yaw";
			case "PFC_ThrottleAxis": return "Throttle";
			case "PFC_GearToggle": return "Landing Gear";
			case "PFC_Flaps": return "Flaps";
			case "PFC_Airbrake": return "Airbrake";
		}

		string readable = actionName;
		readable.Replace("WCS_Armament_", "WCS ");
		readable.Replace("PFC_", "PFC ");
		readable.Replace("_", " ");
		return readable;
	}

	protected void BuildActionList()
	{
		m_WatchedActions.Clear();

		// Helicopter flight and systems
		m_WatchedActions.Insert("HelicopterCollectiveIncrease");
		m_WatchedActions.Insert("HelicopterCollectiveDecrease");
		m_WatchedActions.Insert("HelicopterAntiTorqueLeft");
		m_WatchedActions.Insert("HelicopterAntiTorqueRight");
		m_WatchedActions.Insert("HelicopterCyclicForward");
		m_WatchedActions.Insert("HelicopterCyclicBack");
		m_WatchedActions.Insert("HelicopterCyclicLeft");
		m_WatchedActions.Insert("HelicopterCyclicRight");
		m_WatchedActions.Insert("HelicopterWheelBrake");
		m_WatchedActions.Insert("HelicopterWheelBrakePersistent");
		m_WatchedActions.Insert("HelicopterAutohoverToggle");
		m_WatchedActions.Insert("HelicopterLightsTaxiToggle");
		m_WatchedActions.Insert("HelicopterLightsLandingToggle");
		m_WatchedActions.Insert("HelicopterEngineStart");
		m_WatchedActions.Insert("HelicopterEngineStop");
		m_WatchedActions.Insert("HelicopterFire");
		m_WatchedActions.Insert("HelicopterSightDeploy");
		m_WatchedActions.Insert("HelicopterSightZeroing");

		// Character / vehicle / turret actions used by the configurator
		m_WatchedActions.Insert("CharacterFire");
		m_WatchedActions.Insert("CharacterNextWeapon");
		m_WatchedActions.Insert("CharacterNextFireMode");
		m_WatchedActions.Insert("CharacterNextMuzzle");
		m_WatchedActions.Insert("VehicleFire");
		m_WatchedActions.Insert("VehicleNextWeapon");
		m_WatchedActions.Insert("VehicleDoorToggle");
		m_WatchedActions.Insert("TurretFire");
		m_WatchedActions.Insert("TurretReload");
		m_WatchedActions.Insert("TurretNextWeapon");
		m_WatchedActions.Insert("TurretWeaponNextFireMode");
		m_WatchedActions.Insert("TurretADS");
		m_WatchedActions.Insert("TurretADSHold");
		m_WatchedActions.Insert("TurretRotateLeft");
		m_WatchedActions.Insert("TurretRotateRight");
		m_WatchedActions.Insert("TurretAimUp");
		m_WatchedActions.Insert("TurretAimDown");
		m_WatchedActions.Insert("TurretAimLeft");
		m_WatchedActions.Insert("TurretAimRight");
		m_WatchedActions.Insert("WeaponToggleSightsIllumination");
		m_WatchedActions.Insert("WeaponSwitchOptics");
		m_WatchedActions.Insert("FocusToggle");
		m_WatchedActions.Insert("Freelook");
		m_WatchedActions.Insert("FreelookReset");
		m_WatchedActions.Insert("FreelookUp");
		m_WatchedActions.Insert("FreelookDown");
		m_WatchedActions.Insert("FreelookLeft");
		m_WatchedActions.Insert("FreelookRight");
		m_WatchedActions.Insert("VONDirectToggle");
		m_WatchedActions.Insert("VONChannel");
		m_WatchedActions.Insert("GadgetMap");
		m_WatchedActions.Insert("PerformAction");
		m_WatchedActions.Insert("SelectAction");
		m_WatchedActions.Insert("GetOut");
		m_WatchedActions.Insert("JumpOut");

		// WCS Armament
		m_WatchedActions.Insert("WCS_Armament_CycleWeapon");
		m_WatchedActions.Insert("WCS_Armament_DeployFlares");
		m_WatchedActions.Insert("WCS_Armament_DeployChaffs");
		m_WatchedActions.Insert("WCS_Armament_TurretStabilizationToggle");
		m_WatchedActions.Insert("WCS_Armament_VehicleAim");
		m_WatchedActions.Insert("WCS_Armament_CycleWeaponFireMode");
		m_WatchedActions.Insert("WCS_Armament_ActivateLock");
		m_WatchedActions.Insert("WCS_Armament_DeploySmoke");
		m_WatchedActions.Insert("WCS_Armament_RadarToggle");
		m_WatchedActions.Insert("WCS_Armament_FireContinuousSmokeDispenser");
		m_WatchedActions.Insert("TurretWeaponNextRippleQuantity");
		m_WatchedActions.Insert("WCS_Armament_ConfirmLock");

		// Fixed-wing / Propeller Flight Core
		m_WatchedActions.Insert("PFC_Pitch");
		m_WatchedActions.Insert("PFC_Roll");
		m_WatchedActions.Insert("PFC_Yaw");
		m_WatchedActions.Insert("PFC_ThrottleAxis");
		m_WatchedActions.Insert("PFC_ThrottleUp");
		m_WatchedActions.Insert("PFC_ThrottleDown");
		m_WatchedActions.Insert("PFC_GearToggle");
		m_WatchedActions.Insert("PFC_Flaps");
		m_WatchedActions.Insert("PFC_Airbrake");
		m_WatchedActions.Insert("PFC_WheelBrake");
		m_WatchedActions.Insert("PFC_WheelBrakePersistent");
		m_WatchedActions.Insert("PFC_EngineStart");
		m_WatchedActions.Insert("PFC_EngineStop");
		m_WatchedActions.Insert("PFC_TrimUp");
		m_WatchedActions.Insert("PFC_TrimDown");
		m_WatchedActions.Insert("PFC_TrimReset");
	}
}

modded class SCR_BaseGameMode
{
	override void OnGameModeStart()
	{
		super.OnGameModeStart();

		// The debugger has no authority-side gameplay logic. Initialize the local HUD/input listeners
		// on every machine where this addon is loaded; dedicated servers have no workspace and simply
		// fail the HUD creation harmlessly.
		GetGame().GetCallqueue().CallLater(HOTASDebugController.GetInstance().Initialize, 1000, false);
	}

	override void OnGameModeEnd(SCR_GameModeEndData endData)
	{
		HOTASDebugController.GetInstance().Shutdown();
		super.OnGameModeEnd(endData);
	}
}
