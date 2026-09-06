from pathlib import Path

path = Path('Scripts/Game/HOTASDebugger/HOTASDebugger.c')
src = path.read_text(encoding='utf-8')

old = '''\n#ifdef WORKBENCH\n\t\tLoadWorkbenchTestConfig();\n#endif\n'''
assert old in src, 'Workbench forced-config call not found'
src = src.replace(old, '\n', 1)

old = '''\n\t\t\tm_InputManager.RemoveActionListener("HOTASSettingsToggle", EActionTrigger.DOWN, OnSettingsToggle);\n'''
assert old in src, 'F10 shutdown listener not found'
src = src.replace(old, '\n', 1)

start = src.index('\n#ifdef WORKBENCH\n\tprotected void LoadWorkbenchTestConfig()')
end = src.index('\n#endif\n', start) + len('\n#endif\n')
src = src[:start] + '\n' + src[end:]

old = '''\n\t\tm_InputManager.AddActionListener("HOTASSettingsToggle", EActionTrigger.DOWN, OnSettingsToggle);\n'''
assert old in src, 'F10 register listener not found'
src = src.replace(old, '\n', 1)

start = src.index('\n\tprotected void OnSettingsToggle(')
end = src.index('\n\tprotected void CreateHud()', start)
src = src[:start] + '\n' + src[end:]

start = src.index('\n\tint GetSettingsCount()')
end = src.index('\n\tprotected void ShowHud()', start)
new_settings_api = r'''
	void ReloadHudSettings()
	{
		LoadHudSettings();
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
			case 2: return 16;
			case 3: return 101;
			case 4: return 101;
			case 5: return 2;
			case 6: return 21;
			case 7:
			case 8:
			case 9:
			case 10: return 65;
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
			case 2: return Math.ClampInt(Math.Round((m_fHudScale - 0.5) * 10.0), 0, 15);
			case 3: return Math.ClampInt(Math.Round(m_iFadeDelayMs / 100.0), 0, 100);
			case 4: return Math.ClampInt(Math.Round(m_iFadeDurationMs / 50.0), 0, 100);
			case 5:
				if (m_bBackgroundEnabled) return 1;
				return 0;
			case 6: return Math.ClampInt(Math.Round(m_fBackgroundOpacity * 20.0), 0, 20);
			case 7: return Math.ClampInt(m_iRollAxis + 1, 0, 64);
			case 8: return Math.ClampInt(m_iPitchAxis + 1, 0, 64);
			case 9: return Math.ClampInt(m_iThrottleAxis + 1, 0, 64);
			case 10: return Math.ClampInt(m_iYawAxis + 1, 0, 64);
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
			case 2: return string.Format("%1x", (0.5 + optionIndex * 0.1).ToString(1));
			case 3: return string.Format("%1 ms", optionIndex * 100);
			case 4: return string.Format("%1 ms", optionIndex * 50);
			case 6: return string.Format("%1%", optionIndex * 5);
			case 7:
			case 8:
			case 9:
			case 10:
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
			case 1: SetHudPositionIndex(optionIndex); break;
			case 2: m_fHudScale = 0.5 + optionIndex * 0.1; break;
			case 3: m_iFadeDelayMs = optionIndex * 100; break;
			case 4: m_iFadeDurationMs = optionIndex * 50; break;
			case 5: m_bBackgroundEnabled = optionIndex != 0; break;
			case 6: m_fBackgroundOpacity = optionIndex * 0.05; break;
			case 7: m_iRollAxis = optionIndex - 1; break;
			case 8: m_iPitchAxis = optionIndex - 1; break;
			case 9: m_iThrottleAxis = optionIndex - 1; break;
			case 10: m_iYawAxis = optionIndex - 1; break;
			case 11: m_bDebugMode = optionIndex != 0; break;
			default: return;
		}

		SaveHudSettings();
		if (m_bInitialized)
			RebuildHud();
	}
'''
src = src[:start] + '\n' + new_settings_api + src[end:]

assert 'LoadWorkbenchTestConfig' not in src
assert 'HOTASSettingsToggle' not in src
assert 'OnSettingsToggle' not in src
assert 'GetSettingOptionCount' in src
assert 'SetSettingOptionIndex' in src

path.write_text(src, encoding='utf-8')
