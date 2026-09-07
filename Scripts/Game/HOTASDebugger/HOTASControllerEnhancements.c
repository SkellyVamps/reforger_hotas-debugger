//------------------------------------------------------------------------------------------------
// Controller-side helpers used by the native HOTAS settings page.
modded class HOTASDebugController
{
	//------------------------------------------------------------------------------------------------
	void ResetHudPresentationSettings()
	{
		m_bHudEnabled = true;
		m_bDebugMode = false;
		m_fHudPositionX = 0.5;
		m_fHudPositionY = 0.95;
		m_fHudScale = 1.0;
		m_iFadeDelayMs = 1800;
		m_iFadeDurationMs = 350;
		m_bBackgroundEnabled = true;
		m_fBackgroundOpacity = 0.55;

		SaveHudSettings();
		if (m_bInitialized)
			RebuildHud();
	}

	//------------------------------------------------------------------------------------------------
	void ResetCustomLabels()
	{
		m_sRollAxisLabel = "Roll";
		m_sPitchAxisLabel = "Pitch";
		m_sThrottleAxisLabel = "Throttle";
		m_sYawAxisLabel = "Yaw";
		m_sFreelookUpLabel = string.Empty;
		m_sFreelookDownLabel = string.Empty;
		m_sFreelookRightLabel = string.Empty;
		m_sFreelookLeftLabel = string.Empty;

		SaveHudSettings();
	}

	// Preview-facing accessors. Keeping these here avoids duplicating controller state in UI code.
	float GetHudScaleMultiplier()
	{
		return m_fHudScale;
	}

	bool IsHudBackgroundEnabled()
	{
		return m_bBackgroundEnabled;
	}

	float GetHudBackgroundOpacity()
	{
		return m_fBackgroundOpacity;
	}

	bool IsHudEnabled()
	{
		return m_bHudEnabled;
	}
}
