//------------------------------------------------------------------------------------------------
// Ensure custom HOTAS HUD labels are persisted even when the user leaves an edit box
// without explicitly confirming it with Enter. SCR_EditBoxComponent fires m_OnChanged
// with the same (component, text) arguments used by m_OnConfirm.
modded class HOTASSettingsSubMenu
{
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
