//------------------------------------------------------------------------------------------------
// Joystick capture compatibility layer.
//
// The first implementation created a second temporary InputBinding. In the main-menu settings
// context that binding could enter CAPTURING but never receive joystick input. Reforger's own
// Controls UI captures through SCR_SettingsManagerKeybindModule's active InputBinding, so use the
// same engine-owned binding for capture. The captured result is copied into HOTAS_Config.conf,
// then the temporary user binding is reset before the managed config is re-applied.
modded class HOTASBindingsSubMenu
{
	protected string m_sCaptureRuntimeAction;
	protected int m_iCapturePollTicks;

	//------------------------------------------------------------------------------------------------
	protected bool IsDirectionalCapture(HOTASBindingDefinition definition)
	{
		if (!definition)
			return false;

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
	override protected void OnBindInput()
	{
		if (m_bCapturing || !m_Definitions.IsIndexValid(m_iSelectedAction))
			return;

		SCR_SettingsManagerKeybindModule keybindModule = GetKeybindModule();
		if (!keybindModule)
		{
			SetEditorStatus("Keybind module is unavailable.");
			return;
		}

		InputBinding settingsBinding = keybindModule.GetInputBindings();
		if (!settingsBinding)
		{
			SetEditorStatus("Reforger's active input binding is unavailable.");
			return;
		}

		HOTASBindingDefinition definition = m_Definitions[m_iSelectedAction];
		if (!settingsBinding.FindAction(definition.m_sCaptureAction))
		{
			SetEditorStatus(string.Format("%1 is not available in the currently loaded game/mod set.", definition.m_sCaptureAction));
			return;
		}

		// Reforger's native keybind UI captures on the SettingsManager binding. A second binding can
		// remain CAPTURING forever in the main menu, so deliberately share the native binding here.
		m_CaptureBinding = settingsBinding;
		m_sCaptureRuntimeAction = definition.m_sCaptureAction;
		m_iCapturePollTicks = 0;

		if (m_CaptureBinding.GetCaptureState() == EInputBindingCaptureState.CAPTURING)
			m_CaptureBinding.CancelCapture();

		EInputBindingAxleCapture axleCapture = EInputBindingAxleCapture.ANY;
		if (definition.m_bFullAxis)
			axleCapture = EInputBindingAxleCapture.FULL_AXLE;
		else if (IsDirectionalCapture(definition))
			axleCapture = EInputBindingAxleCapture.HALF_AXLE;

		m_CaptureBinding.StartCapture(
			definition.m_sCaptureAction,
			EInputDeviceType.JOYSTICK,
			string.Empty,
			false,
			axleCapture
		);

		if (m_CaptureBinding.GetCaptureState() != EInputBindingCaptureState.CAPTURING)
		{
			m_CaptureBinding = null;
			m_sCaptureRuntimeAction = string.Empty;
			SetEditorStatus("Joystick capture could not be started by Reforger.");
			Print("[HOTAS Bindings] StartCapture returned without entering CAPTURING.", LogLevel.WARNING);
			return;
		}

		SetCaptureControls(true);
		SetEditorStatus(string.Format("Listening for joystick/HOTAS input for %1... Press Esc to cancel.", definition.m_sDisplayName));

		GetGame().GetCallqueue().Remove(PollCapture);
		GetGame().GetCallqueue().CallLater(PollCapture, 50, true);
	}

	//------------------------------------------------------------------------------------------------
	override protected void PollCapture()
	{
		if (!m_bCapturing || !m_CaptureBinding)
			return;

		if (m_CaptureBinding.GetCaptureState() == EInputBindingCaptureState.CAPTURING)
		{
			m_iCapturePollTicks++;
			if (m_iCapturePollTicks == 40)
				Print("[HOTAS Bindings] Capture is still waiting after 2 seconds.");
			else if (m_iCapturePollTicks == 100)
				Print("[HOTAS Bindings] Capture is still waiting after 5 seconds.", LogLevel.WARNING);
			return;
		}

		GetGame().GetCallqueue().Remove(PollCapture);

		if (!m_Definitions.IsIndexValid(m_iSelectedAction))
		{
			m_CaptureBinding = null;
			m_sCaptureRuntimeAction = string.Empty;
			SetCaptureControls(false);
			return;
		}

		HOTASBindingDefinition definition = m_Definitions[m_iSelectedAction];
		array<string> capturedBindings = {};
		m_CaptureBinding.GetBindings(
			definition.m_sCaptureAction,
			capturedBindings,
			EInputDeviceType.JOYSTICK,
			string.Empty,
			false
		);

		string previousManagedBinding;
		if (m_Bindings.IsIndexValid(m_iSelectedAction))
			previousManagedBinding = m_Bindings[m_iSelectedAction];

		string capturedBinding;
		foreach (string candidate : capturedBindings)
		{
			if (candidate.IndexOf("joystick") < 0)
				continue;

			if (capturedBinding.IsEmpty())
				capturedBinding = candidate;

			// Prefer the value that differs from the currently managed binding when replacing one.
			if (candidate != previousManagedBinding)
			{
				capturedBinding = candidate;
				break;
			}
		}

		// StartCapture temporarily creates a normal user binding on the SettingsManager binding.
		// Remove that temporary override before SelectJoystickPresetPath() saves anything. The only
		// persistent assignment we want is the regenerated mod-owned HOTAS_Config.conf.
		m_CaptureBinding.ResetDefault(definition.m_sCaptureAction, EInputDeviceType.JOYSTICK, string.Empty);
		m_CaptureBinding = null;
		m_sCaptureRuntimeAction = string.Empty;
		SetCaptureControls(false);

		if (capturedBinding.IsEmpty())
		{
			ActivateManagedConfig();
			RefreshCurrentBinding();
			SetEditorStatus("Capture ended, but Reforger did not return a joystick binding.");
			Print("[HOTAS Bindings] Capture ended without a joystick binding.", LogLevel.WARNING);
			return;
		}

		m_Bindings[m_iSelectedAction] = capturedBinding;
		WriteManagedConfig();
		ActivateManagedConfig();
		RefreshCurrentBinding();
		SetEditorStatus(string.Format("Bound %1 to %2.", definition.m_sDisplayName, capturedBinding));
		Print(string.Format("[HOTAS Bindings] Captured %1 for %2", capturedBinding, definition.m_sDisplayName));
	}

	//------------------------------------------------------------------------------------------------
	override protected void StopCapture(bool cancelCapture)
	{
		GetGame().GetCallqueue().Remove(PollCapture);

		bool restoreManagedConfig = false;
		if (m_CaptureBinding)
		{
			EInputBindingCaptureState captureState = m_CaptureBinding.GetCaptureState();
			if (cancelCapture && captureState == EInputBindingCaptureState.CAPTURING)
			{
				m_CaptureBinding.CancelCapture();
			}
			else if (cancelCapture && captureState == EInputBindingCaptureState.IDLE && !m_sCaptureRuntimeAction.IsEmpty())
			{
				// Input may have landed in the same frame the player pressed Cancel. Remove the temporary
				// user binding rather than allowing it to leak into normal Reforger controls.
				m_CaptureBinding.ResetDefault(m_sCaptureRuntimeAction, EInputDeviceType.JOYSTICK, string.Empty);
				restoreManagedConfig = true;
			}
		}

		m_CaptureBinding = null;
		m_sCaptureRuntimeAction = string.Empty;
		m_iCapturePollTicks = 0;
		SetCaptureControls(false);

		if (restoreManagedConfig)
			ActivateManagedConfig();
	}
}
