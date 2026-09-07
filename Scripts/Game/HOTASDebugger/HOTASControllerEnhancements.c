//------------------------------------------------------------------------------------------------
// Controller-side helpers used by the native HOTAS settings page.
modded class HOTASDebugController
{
	//------------------------------------------------------------------------------------------------
	// When no settings file exists yet, let the base controller create its normal defaults,
	// then apply the player-facing label defaults used by the current settings UI.
	// Existing files are left untouched so user-customized labels are never overwritten.
	override protected void LoadHudSettings()
	{
		bool settingsFileMissing = !FileIO.FileExists("$profile:HOTASHudSettings.txt");
		super.LoadHudSettings();

		if (!settingsFileMissing)
			return;

		m_sRollAxisLabel = "Roll";
		m_sPitchAxisLabel = "Pitch";
		m_sThrottleAxisLabel = "Throttle";
		m_sYawAxisLabel = "Yaw";
		m_sFreelookUpLabel = "Thumb Up";
		m_sFreelookDownLabel = "Thumb Down";
		m_sFreelookRightLabel = "Thumb Right";
		m_sFreelookLeftLabel = "Thumb Left";

		SaveHudSettings();
	}

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
		m_sFreelookUpLabel = "Thumb Up";
		m_sFreelookDownLabel = "Thumb Down";
		m_sFreelookRightLabel = "Thumb Right";
		m_sFreelookLeftLabel = "Thumb Left";

		SaveHudSettings();
	}

	//------------------------------------------------------------------------------------------------
	// Return the same player-facing binding text that the real HUD would use for a
	// forward cyclic/pitch input. This lets the settings preview use the current axis
	// assignment and custom Pitch label rather than a hard-coded placeholder.
	string GetPitchForwardPreviewInput()
	{
		string bindingsText = GetJoystickBindings("HelicopterCyclicForward");
		if (bindingsText != "Non-Joystick Input" && bindingsText != "InputManager unavailable")
		{
			string readable = MakeReadableBinding(bindingsText, "HelicopterCyclicForward");
			if (!readable.IsEmpty())
				return readable;
		}

		string pitchLabel = m_sPitchAxisLabel;
		if (pitchLabel.IsEmpty())
			pitchLabel = "Pitch";

		// Forward pitch is normally the negative half of the configured pitch axis.
		// This is only a fallback for cases where the runtime binding cannot be queried.
		return pitchLabel + " -";
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
