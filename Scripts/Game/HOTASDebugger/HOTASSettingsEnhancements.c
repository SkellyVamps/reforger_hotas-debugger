//------------------------------------------------------------------------------------------------
// HOTAS settings-menu extensions that do not belong in the core HUD/controller implementation.
// Keep UI-only behavior here so HOTASSettingsTab.c can remain focused on the base settings tab.
modded class HOTASSettingsSubMenu
{
	// Persist editable labels as the text changes. OnConfirm remains registered by the
	// base tab as a fallback for keyboard/controller confirmation.
	override protected void SetupAxisLabelEditors()
	{
		super.SetupAxisLabelEditors();

		foreach (SCR_EditBoxComponent editor : m_AxisLabelEditors)
		{
			if (editor)
				editor.m_OnChanged.Insert(OnAxisLabelConfirmed);
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void SetupFreelookLabelEditors()
	{
		super.SetupFreelookLabelEditors();

		foreach (SCR_EditBoxComponent editor : m_FreelookLabelEditors)
		{
			if (editor)
				editor.m_OnChanged.Insert(OnFreelookLabelConfirmed);
		}
	}
}
