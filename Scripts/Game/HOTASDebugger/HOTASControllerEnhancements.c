//------------------------------------------------------------------------------------------------
// Controller-side helpers used by the native HOTAS settings page.
modded class HOTASDebugController
{
	protected int m_iLiveInputRevision;
	protected string m_sLiveInputReadable = "Waiting for HOTAS input...";
	protected string m_sLiveInputRaw = "Press or move a bound HOTAS control while in a supported vehicle.";

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
	// Capture the same readable binding/action pair used by the HUD before the base handler
	// checks whether the HUD itself is enabled. This keeps the tester useful with HUD Enabled = Off.
	override protected void OnActionTriggered(float value = 0.0, EActionTrigger reason = 0, string actionName = string.Empty)
	{
		CaptureLiveInput(value, actionName);
		super.OnActionTriggered(value, reason, actionName);
	}

	//------------------------------------------------------------------------------------------------
	protected void CaptureLiveInput(float value, string actionName)
	{
		if (actionName.IsEmpty())
			return;

		int hotasContext = GetPlayerHotasContext();
		if (hotasContext == HOTAS_CONTEXT_NONE)
			return;
		if (!IsActionAllowedForContext(actionName, hotasContext))
			return;

		bool directionalValueAction = UsesDirectionalValueListener(actionName);
		if (directionalValueAction && value > -0.001 && value < 0.001)
			return;

		string bindingsText = GetJoystickBindings(actionName);
		if (bindingsText == "Non-Joystick Input" || bindingsText == "InputManager unavailable")
			return;

		string readableAction = MakeReadableActionName(actionName);
		if (directionalValueAction)
		{
			bindingsText = GetDirectionalBindingForValue(bindingsText, value);
			readableAction = GetDirectionalActionName(actionName, value);
		}

		m_sLiveInputReadable = string.Format("%1  |  %2", MakeReadableBinding(bindingsText, actionName), readableAction);
		m_sLiveInputRaw = string.Format("%1  |  %2  |  value=%3", bindingsText, actionName, value.ToString(2));
		m_iLiveInputRevision++;
	}

	int GetLiveInputRevision()
	{
		return m_iLiveInputRevision;
	}

	string GetLiveInputReadable()
	{
		return m_sLiveInputReadable;
	}

	string GetLiveInputRaw()
	{
		return m_sLiveInputRaw;
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
