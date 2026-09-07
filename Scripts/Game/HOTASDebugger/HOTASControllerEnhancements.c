//------------------------------------------------------------------------------------------------
// Controller-side helpers used by the native HOTAS settings page.
modded class HOTASDebugController
{
	protected int m_iLiveInputRevision;
	protected string m_sLiveInputReadable = "Waiting for HOTAS input...";
	protected string m_sLiveInputRaw = "Press or move any bound HOTAS control. A vehicle is not required.";

	protected bool m_bLiveInputTesterActive;
	protected ref array<string> m_LiveTestActions = {};
	protected ref array<string> m_LiveTestBindingIndex = {};
	protected ref array<string> m_LiveTestBindingActionIndex = {};

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
	// The normal HUD still uses the base context filtering. The settings tester is separate:
	// while the HOTAS tab is visible it listens to every registered action and then looks up
	// every action sharing the same joystick binding, similar to the configurator's live test.
	void SetLiveInputTesterActive(bool active)
	{
		if (m_bLiveInputTesterActive == active)
			return;

		m_bLiveInputTesterActive = active;
		if (!m_InputManager)
			return;

		if (active)
		{
			BuildLiveInputBindingIndex();
			RegisterLiveInputListeners();
		}
		else
		{
			UnregisterLiveInputListeners();
			m_LiveTestBindingIndex.Clear();
			m_LiveTestBindingActionIndex.Clear();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void BuildLiveInputBindingIndex()
	{
		m_LiveTestBindingIndex.Clear();
		m_LiveTestBindingActionIndex.Clear();

		if (!m_InputManager)
			return;

		int actionCount = m_InputManager.GetActionCount();
		for (int i = 0; i < actionCount; i++)
		{
			string actionName = m_InputManager.GetActionName(i);
			if (actionName.IsEmpty())
				continue;

			string bindingsText = GetJoystickBindings(actionName);
			if (bindingsText == "Non-Joystick Input" || bindingsText == "InputManager unavailable")
				continue;

			ref array<string> bindings = {};
			bindingsText.Split(" / ", bindings, true);
			foreach (string binding : bindings)
			{
				binding = binding.Trim();
				if (binding.IndexOf(":button") < 0 && binding.IndexOf(":axis") < 0)
					continue;

				m_LiveTestBindingIndex.Insert(binding);
				m_LiveTestBindingActionIndex.Insert(actionName);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void RegisterLiveInputListeners()
	{
		m_LiveTestActions.Clear();
		if (!m_InputManager)
			return;

		int actionCount = m_InputManager.GetActionCount();
		for (int i = 0; i < actionCount; i++)
		{
			string actionName = m_InputManager.GetActionName(i);
			if (actionName.IsEmpty())
				continue;

			m_InputManager.AddActionListener(actionName, EActionTrigger.VALUE, OnLiveInputActionValue);
			m_LiveTestActions.Insert(actionName);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void UnregisterLiveInputListeners()
	{
		if (m_InputManager)
		{
			foreach (string actionName : m_LiveTestActions)
				m_InputManager.RemoveActionListener(actionName, EActionTrigger.VALUE, OnLiveInputActionValue);
		}

		m_LiveTestActions.Clear();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnLiveInputActionValue(float value = 0.0, EActionTrigger reason = 0, string actionName = string.Empty)
	{
		if (!m_bLiveInputTesterActive || !m_InputManager)
			return;
		if (m_InputManager.GetLastUsedInputDevice() != EInputDeviceType.JOYSTICK)
			return;
		if (value > -0.001 && value < 0.001)
			return;

		CaptureLiveInput(value, actionName);
	}

	//------------------------------------------------------------------------------------------------
	// Keep the existing watched-action callback feeding the tester as a fallback. The normal
	// HUD processing remains in super.OnActionTriggered() and therefore keeps vehicle/context rules.
	override protected void OnActionTriggered(float value = 0.0, EActionTrigger reason = 0, string actionName = string.Empty)
	{
		if (m_bLiveInputTesterActive)
			CaptureLiveInput(value, actionName);
		super.OnActionTriggered(value, reason, actionName);
	}

	//------------------------------------------------------------------------------------------------
	protected string GetPrimaryLiveInputBinding(string bindingsText, float value, string actionName)
	{
		if (UsesDirectionalValueListener(actionName))
			bindingsText = GetDirectionalBindingForValue(bindingsText, value);

		ref array<string> bindings = {};
		bindingsText.Split(" / ", bindings, true);
		foreach (string binding : bindings)
		{
			binding = binding.Trim();
			if (binding.IndexOf(":button") >= 0 || binding.IndexOf(":axis") >= 0)
				return binding;
		}

		return string.Empty;
	}

	//------------------------------------------------------------------------------------------------
	protected bool BindingHasPositiveDirection(string binding)
	{
		return binding.EndsWith("+");
	}

	//------------------------------------------------------------------------------------------------
	protected bool BindingHasNegativeDirection(string binding)
	{
		return binding.EndsWith("-");
	}

	//------------------------------------------------------------------------------------------------
	protected bool LiveBindingsMatch(string triggeredBinding, string candidateBinding, float value)
	{
		if (triggeredBinding == candidateBinding)
			return true;

		bool triggeredAxis = triggeredBinding.IndexOf(":axis") >= 0;
		bool candidateAxis = candidateBinding.IndexOf(":axis") >= 0;
		if (!triggeredAxis || !candidateAxis)
			return false;

		if (NormalizeAxisBinding(triggeredBinding) != NormalizeAxisBinding(candidateBinding))
			return false;

		bool triggeredPositive = BindingHasPositiveDirection(triggeredBinding);
		bool triggeredNegative = BindingHasNegativeDirection(triggeredBinding);
		bool candidatePositive = BindingHasPositiveDirection(candidateBinding);
		bool candidateNegative = BindingHasNegativeDirection(candidateBinding);

		// A full-range axis binding represents the same physical axis as either directional
		// binding. When the trigger itself is full-range, use its current sign to avoid also
		// reporting the opposite directional action.
		if (!candidatePositive && !candidateNegative)
			return true;

		if (triggeredPositive)
			return candidatePositive;
		if (triggeredNegative)
			return candidateNegative;

		if (value < 0.0)
			return candidateNegative;
		return candidatePositive;
	}

	//------------------------------------------------------------------------------------------------
	protected string GetAllActionsForLiveBinding(string triggeredBinding, float value)
	{
		ref array<string> matchedActions = {};

		for (int i = 0; i < m_LiveTestBindingIndex.Count(); i++)
		{
			if (!LiveBindingsMatch(triggeredBinding, m_LiveTestBindingIndex[i], value))
				continue;

			string actionName = m_LiveTestBindingActionIndex[i];
			if (!matchedActions.Contains(actionName))
				matchedActions.Insert(actionName);
		}

		string result;
		foreach (string matchedAction : matchedActions)
		{
			if (!result.IsEmpty())
				result += " / ";
			result += matchedAction;
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	protected void CaptureLiveInput(float value, string actionName)
	{
		if (!m_bLiveInputTesterActive || actionName.IsEmpty())
			return;

		bool directionalValueAction = UsesDirectionalValueListener(actionName);
		if (directionalValueAction && value > -0.001 && value < 0.001)
			return;

		string bindingsText = GetJoystickBindings(actionName);
		if (bindingsText == "Non-Joystick Input" || bindingsText == "InputManager unavailable")
			return;

		string liveBinding = GetPrimaryLiveInputBinding(bindingsText, value, actionName);
		if (liveBinding.IsEmpty())
			return;

		string boundActions = GetAllActionsForLiveBinding(liveBinding, value);
		if (boundActions.IsEmpty())
			boundActions = actionName;

		m_sLiveInputReadable = MakeReadableBinding(liveBinding, actionName);
		m_sLiveInputRaw = string.Format("Bound actions: %1\nRaw input: %2  |  value=%3", boundActions, liveBinding, value.ToString(2));
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
