class HOTASSettingsMenu : MenuBase
{
	protected InputManager m_InputManager;
	protected RichTextWidget m_Title;
	protected RichTextWidget m_Body;
	protected RichTextWidget m_Help;
	protected int m_iSelectedSetting;

	override void OnMenuOpen()
	{
		super.OnMenuOpen();

		Widget root = GetRootWidget();
		if (root)
		{
			m_Title = RichTextWidget.Cast(root.FindAnyWidget("SettingsTitle"));
			m_Body = RichTextWidget.Cast(root.FindAnyWidget("SettingsBody"));
			m_Help = RichTextWidget.Cast(root.FindAnyWidget("SettingsHelp"));
		}

		m_InputManager = GetGame().GetInputManager();
		if (m_InputManager)
		{
			m_InputManager.AddActionListener("MenuUp", EActionTrigger.DOWN, OnMenuUp);
			m_InputManager.AddActionListener("MenuDown", EActionTrigger.DOWN, OnMenuDown);
			m_InputManager.AddActionListener("MenuLeft", EActionTrigger.DOWN, OnMenuLeft);
			m_InputManager.AddActionListener("MenuRight", EActionTrigger.DOWN, OnMenuRight);
			m_InputManager.AddActionListener("MenuSelect", EActionTrigger.DOWN, OnMenuSelect);
			m_InputManager.AddActionListener("MenuBack", EActionTrigger.DOWN, OnMenuBack);
		}

		if (m_Title)
			m_Title.SetText("HOTAS INPUT HUD SETTINGS");
		if (m_Help)
			m_Help.SetText("UP / DOWN: Select     LEFT / RIGHT: Change     ENTER: Toggle / Advance     ESC or F10: Close");

		RefreshSettingsText();
	}

	override void OnMenuClose()
	{
		if (m_InputManager)
		{
			m_InputManager.RemoveActionListener("MenuUp", EActionTrigger.DOWN, OnMenuUp);
			m_InputManager.RemoveActionListener("MenuDown", EActionTrigger.DOWN, OnMenuDown);
			m_InputManager.RemoveActionListener("MenuLeft", EActionTrigger.DOWN, OnMenuLeft);
			m_InputManager.RemoveActionListener("MenuRight", EActionTrigger.DOWN, OnMenuRight);
			m_InputManager.RemoveActionListener("MenuSelect", EActionTrigger.DOWN, OnMenuSelect);
			m_InputManager.RemoveActionListener("MenuBack", EActionTrigger.DOWN, OnMenuBack);
		}

		m_InputManager = null;
		super.OnMenuClose();
	}

	override void OnMenuUpdate(float tDelta)
	{
		super.OnMenuUpdate(tDelta);
		InputManager inputManager = GetGame().GetInputManager();
		if (inputManager)
			inputManager.ActivateContext("MenuContext");
	}

	protected void OnMenuUp(float value = 0.0, EActionTrigger reason = 0, string actionName = string.Empty)
	{
		int count = HOTASDebugController.GetInstance().GetSettingsCount();
		if (count <= 0)
			return;

		m_iSelectedSetting--;
		if (m_iSelectedSetting < 0)
			m_iSelectedSetting = count - 1;
		RefreshSettingsText();
	}

	protected void OnMenuDown(float value = 0.0, EActionTrigger reason = 0, string actionName = string.Empty)
	{
		int count = HOTASDebugController.GetInstance().GetSettingsCount();
		if (count <= 0)
			return;

		m_iSelectedSetting++;
		if (m_iSelectedSetting >= count)
			m_iSelectedSetting = 0;
		RefreshSettingsText();
	}

	protected void OnMenuLeft(float value = 0.0, EActionTrigger reason = 0, string actionName = string.Empty)
	{
		HOTASDebugController.GetInstance().AdjustSetting(m_iSelectedSetting, -1);
		RefreshSettingsText();
	}

	protected void OnMenuRight(float value = 0.0, EActionTrigger reason = 0, string actionName = string.Empty)
	{
		HOTASDebugController.GetInstance().AdjustSetting(m_iSelectedSetting, 1);
		RefreshSettingsText();
	}

	protected void OnMenuSelect(float value = 0.0, EActionTrigger reason = 0, string actionName = string.Empty)
	{
		HOTASDebugController.GetInstance().AdjustSetting(m_iSelectedSetting, 1);
		RefreshSettingsText();
	}

	protected void OnMenuBack(float value = 0.0, EActionTrigger reason = 0, string actionName = string.Empty)
	{
		GetManager().CloseMenu(this);
	}

	protected void RefreshSettingsText()
	{
		if (!m_Body)
			return;

		HOTASDebugController controller = HOTASDebugController.GetInstance();
		int count = controller.GetSettingsCount();
		string text;

		for (int i = 0; i < count; i++)
		{
			if (i == m_iSelectedSetting)
				text += "<color rgba=\"226,167,80,255\">> ";
			else
				text += "<color rgba=\"220,220,220,255\">  ";

			text += controller.GetSettingLabel(i);
			text += ":  ";
			text += controller.GetSettingValue(i);
			text += "</color>\n";
		}

		m_Body.SetText(text);
	}
}

modded enum ChimeraMenuPreset
{
	HOTASSettingsMenu
}
