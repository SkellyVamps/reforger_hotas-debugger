//------------------------------------------------------------------------------------------------
// Joystick/HOTAS capture that does not depend on InputBinding.StartCapture().
//
// Reforger currently leaves joystick InputBinding capture waiting indefinitely in both the main
// menu and an in-game settings menu. HOTAS_Config.conf therefore contains private probe actions for
// joystick buttons and half-axes. While this page is listening, only those private actions are
// activated and polled. The detected physical input string is then written into the mod-owned
// HOTAS_Config.conf through the normal managed-config regeneration path.
modded class HOTASBindingsSubMenu
{
	protected static const int CAPTURE_DEVICE_COUNT = 4;
	protected static const int CAPTURE_BUTTON_COUNT = 128;
	protected static const int CAPTURE_AXIS_COUNT = 16;
	protected static const float CAPTURE_BUTTON_THRESHOLD = 0.5;
	protected static const float CAPTURE_AXIS_DELTA_THRESHOLD = 0.25;

	protected ref array<float> m_CaptureAxisBaseline = {};
	protected bool m_bCaptureBaselineReady;
	protected bool m_bCaptureProbeWarmupDone;
	protected int m_iCapturePollTicks;
	protected int m_iLastActivatedProbeCount;

	//------------------------------------------------------------------------------------------------
	protected bool IsAxisCaptureDefinition(HOTASBindingDefinition definition)
	{
		if (!definition)
			return false;

		if (definition.m_bFullAxis)
			return true;

		string preset = definition.m_sFilterPreset;
		if (preset == "up")
			return true;
		if (preset == "down")
			return true;
		if (preset == "left")
			return true;
		if (preset == "right")
			return true;
		if (preset == "forward")
			return true;
		if (preset == "back")
			return true;
		if (preset == "previous")
			return true;
		if (preset == "next")
			return true;

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected string ProbeButtonActionName(int slot, int buttonIndex)
	{
		return string.Format("HOTASCapture_J%1_B%2", slot, buttonIndex);
	}

	//------------------------------------------------------------------------------------------------
	protected string ProbeAxisActionName(int slot, int axisIndex, bool positive)
	{
		if (positive)
			return string.Format("HOTASCapture_J%1_A%2_P", slot, axisIndex);

		return string.Format("HOTASCapture_J%1_A%2_N", slot, axisIndex);
	}

	//------------------------------------------------------------------------------------------------
	protected int ProbeAxisBaselineIndex(int slot, int axisIndex, bool positive)
	{
		int signIndex = 0;
		if (positive)
			signIndex = 1;

		return (slot * CAPTURE_AXIS_COUNT + axisIndex) * 2 + signIndex;
	}

	//------------------------------------------------------------------------------------------------
	protected void ResetCaptureBaseline()
	{
		m_CaptureAxisBaseline.Clear();
		int baselineCount = CAPTURE_DEVICE_COUNT * CAPTURE_AXIS_COUNT * 2;
		for (int i = 0; i < baselineCount; i++)
			m_CaptureAxisBaseline.Insert(0.0);

		m_bCaptureBaselineReady = false;
		m_bCaptureProbeWarmupDone = false;
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsJoystickSlotConnected(InputManager inputManager, int slot)
	{
		if (!inputManager)
			return false;

		JoystickDeviceHandler joystickHandler = inputManager.GetJoystickDeviceHandler();
		if (!joystickHandler)
			return false;

		return joystickHandler.IsJoystickConnected(slot);
	}

	//------------------------------------------------------------------------------------------------
	protected int CountConnectedJoysticks(InputManager inputManager)
	{
		int connectedCount;
		for (int slot = 0; slot < CAPTURE_DEVICE_COUNT; slot++)
		{
			if (IsJoystickSlotConnected(inputManager, slot))
				connectedCount++;
		}

		return connectedCount;
	}

	//------------------------------------------------------------------------------------------------
	protected int ActivateCaptureProbes(InputManager inputManager, bool includeAxes)
	{
		if (!inputManager)
			return 0;

		int activatedCount;
		for (int slot = 0; slot < CAPTURE_DEVICE_COUNT; slot++)
		{
			if (!IsJoystickSlotConnected(inputManager, slot))
				continue;

			for (int buttonIndex = 0; buttonIndex < CAPTURE_BUTTON_COUNT; buttonIndex++)
			{
				if (inputManager.ActivateAction(ProbeButtonActionName(slot, buttonIndex), 300))
					activatedCount++;
			}

			if (!includeAxes)
				continue;

			for (int axisIndex = 0; axisIndex < CAPTURE_AXIS_COUNT; axisIndex++)
			{
				if (inputManager.ActivateAction(ProbeAxisActionName(slot, axisIndex, true), 300))
					activatedCount++;
				if (inputManager.ActivateAction(ProbeAxisActionName(slot, axisIndex, false), 300))
					activatedCount++;
			}
		}

		return activatedCount;
	}

	//------------------------------------------------------------------------------------------------
	protected void CaptureAxisBaseline(InputManager inputManager)
	{
		if (!inputManager)
			return;

		for (int slot = 0; slot < CAPTURE_DEVICE_COUNT; slot++)
		{
			if (!IsJoystickSlotConnected(inputManager, slot))
				continue;

			for (int axisIndex = 0; axisIndex < CAPTURE_AXIS_COUNT; axisIndex++)
			{
				int negativeIndex = ProbeAxisBaselineIndex(slot, axisIndex, false);
				int positiveIndex = ProbeAxisBaselineIndex(slot, axisIndex, true);

				if (m_CaptureAxisBaseline.IsIndexValid(negativeIndex))
					m_CaptureAxisBaseline[negativeIndex] = inputManager.GetActionValue(ProbeAxisActionName(slot, axisIndex, false));
				if (m_CaptureAxisBaseline.IsIndexValid(positiveIndex))
					m_CaptureAxisBaseline[positiveIndex] = inputManager.GetActionValue(ProbeAxisActionName(slot, axisIndex, true));
			}
		}

		m_bCaptureBaselineReady = true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool DetectPressedButton(InputManager inputManager, out string binding)
	{
		binding = string.Empty;
		if (!inputManager)
			return false;

		for (int slot = 0; slot < CAPTURE_DEVICE_COUNT; slot++)
		{
			if (!IsJoystickSlotConnected(inputManager, slot))
				continue;

			for (int buttonIndex = 0; buttonIndex < CAPTURE_BUTTON_COUNT; buttonIndex++)
			{
				string actionName = ProbeButtonActionName(slot, buttonIndex);
				if (inputManager.GetActionValue(actionName) < CAPTURE_BUTTON_THRESHOLD)
					continue;

				binding = string.Format("joystick%1:button%2", slot, buttonIndex);
				return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected bool DetectMovedAxis(InputManager inputManager, bool fullAxis, out string binding)
	{
		binding = string.Empty;
		if (!inputManager || !m_bCaptureBaselineReady)
			return false;

		float largestDelta;
		int detectedSlot = -1;
		int detectedAxis = -1;
		bool detectedPositive;

		for (int slot = 0; slot < CAPTURE_DEVICE_COUNT; slot++)
		{
			if (!IsJoystickSlotConnected(inputManager, slot))
				continue;

			for (int axisIndex = 0; axisIndex < CAPTURE_AXIS_COUNT; axisIndex++)
			{
				int negativeIndex = ProbeAxisBaselineIndex(slot, axisIndex, false);
				int positiveIndex = ProbeAxisBaselineIndex(slot, axisIndex, true);
				if (!m_CaptureAxisBaseline.IsIndexValid(negativeIndex) || !m_CaptureAxisBaseline.IsIndexValid(positiveIndex))
					continue;

				float negativeValue = inputManager.GetActionValue(ProbeAxisActionName(slot, axisIndex, false));
				float positiveValue = inputManager.GetActionValue(ProbeAxisActionName(slot, axisIndex, true));
				float negativeDelta = Math.AbsFloat(negativeValue - m_CaptureAxisBaseline[negativeIndex]);
				float positiveDelta = Math.AbsFloat(positiveValue - m_CaptureAxisBaseline[positiveIndex]);

				if (negativeDelta > largestDelta)
				{
					largestDelta = negativeDelta;
					detectedSlot = slot;
					detectedAxis = axisIndex;
					detectedPositive = false;
				}

				if (positiveDelta > largestDelta)
				{
					largestDelta = positiveDelta;
					detectedSlot = slot;
					detectedAxis = axisIndex;
					detectedPositive = true;
				}
			}
		}

		if (detectedSlot < 0 || detectedAxis < 0 || largestDelta < CAPTURE_AXIS_DELTA_THRESHOLD)
			return false;

		if (fullAxis)
		{
			binding = string.Format("joystick%1:axis%2", detectedSlot, detectedAxis);
			return true;
		}

		if (detectedPositive)
			binding = string.Format("joystick%1:axis%2+", detectedSlot, detectedAxis);
		else
			binding = string.Format("joystick%1:axis%2-", detectedSlot, detectedAxis);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnBindInput()
	{
		if (m_bCapturing || !m_Definitions.IsIndexValid(m_iSelectedAction))
			return;

		InputManager inputManager = GetGame().GetInputManager();
		if (!inputManager)
		{
			SetEditorStatus("InputManager is unavailable.");
			return;
		}

		int connectedCount = CountConnectedJoysticks(inputManager);
		if (connectedCount <= 0)
		{
			SetEditorStatus("No joystick/HOTAS device is currently detected by Reforger.");
			Print("[HOTAS Bindings] Capture aborted because Reforger reports no connected joystick slots.", LogLevel.WARNING);
			return;
		}

		// Make sure the private probe actions exist in the active managed config before polling them.
		if (!WriteManagedConfig())
			return;

		ActivateManagedConfig();

		m_CaptureBinding = null;
		m_iCapturePollTicks = 0;
		m_iLastActivatedProbeCount = 0;
		ResetCaptureBaseline();
		SetCaptureControls(true);

		HOTASBindingDefinition definition = m_Definitions[m_iSelectedAction];
		SetEditorStatus(string.Format("Preparing HOTAS capture for %1...", definition.m_sDisplayName));
		Print(string.Format("[HOTAS Bindings] Preparing private capture probes for %1; connected joystick slots=%2", definition.m_sDisplayName, connectedCount));

		GetGame().GetCallqueue().Remove(BeginProbeCapture);
		GetGame().GetCallqueue().Remove(PollCapture);
		GetGame().GetCallqueue().CallLater(BeginProbeCapture, 200, false);
	}

	//------------------------------------------------------------------------------------------------
	protected void BeginProbeCapture()
	{
		if (!m_bCapturing || !m_Definitions.IsIndexValid(m_iSelectedAction))
			return;

		InputManager inputManager = GetGame().GetInputManager();
		if (!inputManager)
		{
			StopCapture(false);
			SetEditorStatus("InputManager became unavailable while preparing HOTAS capture.");
			return;
		}

		HOTASBindingDefinition definition = m_Definitions[m_iSelectedAction];
		bool includeAxes = IsAxisCaptureDefinition(definition);
		m_iLastActivatedProbeCount = ActivateCaptureProbes(inputManager, includeAxes);
		if (m_iLastActivatedProbeCount <= 0)
		{
			StopCapture(false);
			SetEditorStatus("HOTAS capture probes were not loaded from HOTAS_Config.conf. Check the console log.");
			Print("[HOTAS Bindings] No private capture probe action could be activated after HOTAS_Config.conf was reloaded.", LogLevel.ERROR);
			return;
		}

		SetEditorStatus(string.Format("Listening for joystick/HOTAS input for %1... Press Esc to cancel.", definition.m_sDisplayName));
		Print(string.Format("[HOTAS Bindings] Private capture active for %1; probe actions activated=%2", definition.m_sDisplayName, m_iLastActivatedProbeCount));

		GetGame().GetCallqueue().CallLater(PollCapture, 100, true);
	}

	//------------------------------------------------------------------------------------------------
	override protected void PollCapture()
	{
		if (!m_bCapturing || !m_Definitions.IsIndexValid(m_iSelectedAction))
			return;

		InputManager inputManager = GetGame().GetInputManager();
		if (!inputManager)
			return;

		HOTASBindingDefinition definition = m_Definitions[m_iSelectedAction];
		bool includeAxes = IsAxisCaptureDefinition(definition);
		m_iLastActivatedProbeCount = ActivateCaptureProbes(inputManager, includeAxes);
		m_iCapturePollTicks++;

		// Give the newly activated probe actions a frame to receive their current device values before
		// using them as an axis baseline. This prevents a throttle already away from center from being
		// mistaken for movement immediately after capture starts.
		if (!m_bCaptureProbeWarmupDone)
		{
			m_bCaptureProbeWarmupDone = true;
			return;
		}

		if (includeAxes && !m_bCaptureBaselineReady)
		{
			CaptureAxisBaseline(inputManager);
			return;
		}

		string capturedBinding;
		bool captured = DetectPressedButton(inputManager, capturedBinding);
		if (!captured && includeAxes)
			captured = DetectMovedAxis(inputManager, definition.m_bFullAxis, capturedBinding);

		if (!captured)
		{
			if (m_iCapturePollTicks == 20)
				Print(string.Format("[HOTAS Bindings] Probe capture is still waiting after about 2 seconds; activated probes=%1", m_iLastActivatedProbeCount));
			else if (m_iCapturePollTicks == 50)
				Print(string.Format("[HOTAS Bindings] Probe capture is still waiting after about 5 seconds; activated probes=%1", m_iLastActivatedProbeCount), LogLevel.WARNING);
			return;
		}

		GetGame().GetCallqueue().Remove(PollCapture);
		m_Bindings[m_iSelectedAction] = capturedBinding;
		m_bCapturing = false;
		SetCaptureControls(false);

		WriteManagedConfig();
		ActivateManagedConfig();
		RefreshCurrentBinding();
		SetEditorStatus(string.Format("Bound %1 to %2.", definition.m_sDisplayName, capturedBinding));
		Print(string.Format("[HOTAS Bindings] Captured %1 for %2 using private probe actions.", capturedBinding, definition.m_sDisplayName));
	}

	//------------------------------------------------------------------------------------------------
	override protected void StopCapture(bool cancelCapture)
	{
		GetGame().GetCallqueue().Remove(BeginProbeCapture);
		GetGame().GetCallqueue().Remove(PollCapture);
		m_CaptureBinding = null;
		m_iCapturePollTicks = 0;
		m_iLastActivatedProbeCount = 0;
		m_bCaptureProbeWarmupDone = false;
		m_bCaptureBaselineReady = false;
		SetCaptureControls(false);
	}
}
