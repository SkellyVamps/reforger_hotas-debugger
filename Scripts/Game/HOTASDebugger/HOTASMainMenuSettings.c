//------------------------------------------------------------------------------------------------
// Make the HOTAS settings page fully usable from the main menu before a play session starts.
// This prepares only the settings/input-binding state. It does not create the HUD, register
// gameplay listeners, or automatically open the Settings menu.
modded class HOTASDebugController
{
	//------------------------------------------------------------------------------------------------
	void PrepareForSettings()
	{
		// InputManager is available from the main menu, but the normal controller does not
		// acquire it until Initialize() runs after entering a play session. The settings page
		// needs a user binding earlier so config/axis detection works before joining a game.
		if (!m_InputManager)
			m_InputManager = GetGame().GetInputManager();

		if (m_InputManager && !m_InputBinding)
			m_InputBinding = m_InputManager.CreateUserBinding();

		// HUD presentation and labels are profile data and can safely be loaded/saved from
		// the main menu. Refresh axis assignments when an input binding is available.
		LoadHudSettings();
		if (m_InputBinding)
			RefreshAssignedAxesFromBindings();
	}
}

//------------------------------------------------------------------------------------------------
// Prepare controller/settings state before the existing HOTAS tab creates or refreshes its
// controls. The tab remains user-opened through Settings > HOTAS; nothing is opened automatically.
modded class HOTASSettingsSubMenu
{
	//------------------------------------------------------------------------------------------------
	override void OnTabCreate(Widget menuRoot, ResourceName buttonsLayout, int index)
	{
		HOTASDebugController.GetInstance().PrepareForSettings();
		super.OnTabCreate(menuRoot, buttonsLayout, index);
	}

	//------------------------------------------------------------------------------------------------
	override void OnTabShow()
	{
		HOTASDebugController.GetInstance().PrepareForSettings();
		super.OnTabShow();
	}
}
