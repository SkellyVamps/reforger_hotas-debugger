//------------------------------------------------------------------------------------------------
// Preserve the structural IDs and explicit filter blocks from imported HOTAS configs.
//
// Imported configs are read-only. Their supported Action/InputSource structure is copied into the
// mod-owned HOTAS_Config.conf so we do not replace valid InputSourceSum/InputSourceValue/filter IDs
// with placeholder values. New bindings receive stable random 16-character resource IDs the first
// time they are generated.
modded class HOTASBindingsSubMenu
{
	protected static const int HOTAS_CAPTURE_DEVICE_COUNT = 4;
	protected static const int HOTAS_CAPTURE_BUTTON_COUNT = 128;
	protected static const int HOTAS_CAPTURE_AXIS_COUNT = 16;

	protected ref array<string> m_BindingActionTypes = {};
	protected ref array<string> m_BindingSourceSumIds = {};
	protected ref array<string> m_BindingSourceValueIds = {};
	protected ref array<string> m_BindingFilterTypes = {};
	protected ref array<string> m_BindingFilterIds = {};
	protected ref array<string> m_BindingFilterHoldDurations = {};
	protected ref array<string> m_BindingFilterMultipliers = {};
	protected ref array<bool> m_BindingImportedStructure = {};

	//------------------------------------------------------------------------------------------------
	override protected void BuildDefinitions()
	{
		super.BuildDefinitions();
		EnsureStructureStorage();
	}

	//------------------------------------------------------------------------------------------------
	protected void EnsureStructureStorage()
	{
		while (m_BindingSourceSumIds.Count() < m_Definitions.Count())
		{
			m_BindingActionTypes.Insert(string.Empty);
			m_BindingSourceSumIds.Insert(string.Empty);
			m_BindingSourceValueIds.Insert(string.Empty);
			m_BindingFilterTypes.Insert(string.Empty);
			m_BindingFilterIds.Insert(string.Empty);
			m_BindingFilterHoldDurations.Insert(string.Empty);
			m_BindingFilterMultipliers.Insert(string.Empty);
			m_BindingImportedStructure.Insert(false);
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void ClearBindings()
	{
		super.ClearBindings();
		EnsureStructureStorage();

		for (int i = 0; i < m_Definitions.Count(); i++)
		{
			m_BindingActionTypes[i] = string.Empty;
			m_BindingSourceSumIds[i] = string.Empty;
			m_BindingSourceValueIds[i] = string.Empty;
			m_BindingFilterTypes[i] = string.Empty;
			m_BindingFilterIds[i] = string.Empty;
			m_BindingFilterHoldDurations[i] = string.Empty;
			m_BindingFilterMultipliers[i] = string.Empty;
			m_BindingImportedStructure[i] = false;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected string ExtractQuotedValue(string line, string prefix)
	{
		int token = line.IndexOf(prefix);
		if (token < 0)
			return string.Empty;

		int valueStart = token + prefix.Length();
		if (valueStart >= line.Length())
			return string.Empty;

		string tail = line.Substring(valueStart, line.Length() - valueStart);
		int valueEnd = tail.IndexOf("\"");
		if (valueEnd < 0)
			return string.Empty;

		return tail.Substring(0, valueEnd);
	}

	//------------------------------------------------------------------------------------------------
	protected string CompactValueAfter(string line, string prefix)
	{
		int token = line.IndexOf(prefix);
		if (token < 0)
			return string.Empty;

		int valueStart = token + prefix.Length();
		if (valueStart >= line.Length())
			return string.Empty;

		string value = line.Substring(valueStart, line.Length() - valueStart);
		value.Replace(" ", string.Empty);
		value.Replace("\t", string.Empty);
		return value;
	}

	//------------------------------------------------------------------------------------------------
	override protected int LoadConfigIntoBindings(string path, bool managedFile)
	{
		FileHandle file = FileIO.OpenFile(path, FileMode.READ);
		if (!file)
			return 0;

		EnsureStructureStorage();
		if (!managedFile)
			ClearBindings();

		string currentAction;
		string currentActionType;
		string currentFilterPreset;
		string currentSourceSumId;
		string currentSourceValueId;
		int pendingDefinitionIndex = -1;
		string line;
		int importedCount;

		while (file.ReadLine(line) > 0)
		{
			int actionToken = line.IndexOf("Action ");
			int braceToken = line.IndexOf("{");
			if (actionToken >= 0 && braceToken > actionToken)
			{
				currentAction = line.Substring(actionToken + 7, braceToken - actionToken - 7);
				currentAction.Replace(" ", string.Empty);
				currentAction.Replace("\t", string.Empty);
				currentActionType = string.Empty;
				currentFilterPreset = string.Empty;
				currentSourceSumId = string.Empty;
				currentSourceValueId = string.Empty;
				pendingDefinitionIndex = -1;
				continue;
			}

			if (!currentAction.IsEmpty() && line.IndexOf("Type ") >= 0)
			{
				currentActionType = CompactValueAfter(line, "Type ");
				continue;
			}

			if (line.IndexOf("InputSource InputSourceSum \"") >= 0)
			{
				currentSourceSumId = ExtractQuotedValue(line, "InputSource InputSourceSum \"");
				continue;
			}

			if (line.IndexOf("InputSourceValue \"") >= 0)
			{
				currentSourceValueId = ExtractQuotedValue(line, "InputSourceValue \"");
				currentFilterPreset = string.Empty;
				pendingDefinitionIndex = -1;
				continue;
			}

			if (line.IndexOf("FilterPreset \"") >= 0)
			{
				currentFilterPreset = ExtractQuotedValue(line, "FilterPreset \"");
				continue;
			}

			if (line.IndexOf("Input \"") >= 0 && !currentAction.IsEmpty())
			{
				string inputValue = ExtractQuotedValue(line, "Input \"");
				if (inputValue.IndexOf("joystick") < 0)
					continue;

				int definitionIndex = FindDefinitionForImport(currentAction, currentFilterPreset);
				if (definitionIndex < 0)
					continue;

				m_Bindings[definitionIndex] = inputValue;
				m_BindingActionTypes[definitionIndex] = currentActionType;
				m_BindingSourceSumIds[definitionIndex] = currentSourceSumId;
				m_BindingSourceValueIds[definitionIndex] = currentSourceValueId;
				m_BindingImportedStructure[definitionIndex] = true;
				pendingDefinitionIndex = definitionIndex;
				importedCount++;
				continue;
			}

			if (pendingDefinitionIndex >= 0 && line.IndexOf("Filter InputFilter") >= 0)
			{
				int filterStart = line.IndexOf("Filter ") + 7;
				int quoteStart = line.IndexOf("\"");
				if (quoteStart > filterStart)
				{
					string filterType = line.Substring(filterStart, quoteStart - filterStart);
					filterType.Replace(" ", string.Empty);
					filterType.Replace("\t", string.Empty);
					m_BindingFilterTypes[pendingDefinitionIndex] = filterType;
					m_BindingFilterIds[pendingDefinitionIndex] = ExtractQuotedValue(line, filterType + " \"");
				}
				continue;
			}

			if (pendingDefinitionIndex >= 0 && line.IndexOf("HoldDuration ") >= 0)
			{
				m_BindingFilterHoldDurations[pendingDefinitionIndex] = CompactValueAfter(line, "HoldDuration ");
				continue;
			}

			if (pendingDefinitionIndex >= 0 && line.IndexOf("Multiplier ") >= 0)
			{
				m_BindingFilterMultipliers[pendingDefinitionIndex] = CompactValueAfter(line, "Multiplier ");
				continue;
			}
		}

		file.Close();
		return importedCount;
	}

	//------------------------------------------------------------------------------------------------
	protected string NewResourceId()
	{
		const string HEX = "0123456789ABCDEF";
		string result;
		for (int i = 0; i < 16; i++)
			result += HEX.Substring(Math.RandomInt(0, 16), 1);

		return result;
	}

	//------------------------------------------------------------------------------------------------
	protected string GetOrCreateSourceSumId(int definitionIndex)
	{
		EnsureStructureStorage();
		if (!m_Definitions.IsIndexValid(definitionIndex))
			return NewResourceId();

		string configAction = m_Definitions[definitionIndex].m_sConfigAction;
		string sourceSumId;

		for (int i = 0; i < m_Definitions.Count(); i++)
		{
			if (m_Definitions[i].m_sConfigAction != configAction)
				continue;
			if (!m_BindingSourceSumIds[i].IsEmpty())
			{
				sourceSumId = m_BindingSourceSumIds[i];
				break;
			}
		}

		if (sourceSumId.IsEmpty())
			sourceSumId = NewResourceId();

		for (int j = 0; j < m_Definitions.Count(); j++)
		{
			if (m_Definitions[j].m_sConfigAction == configAction && m_BindingSourceSumIds[j].IsEmpty())
				m_BindingSourceSumIds[j] = sourceSumId;
		}

		return sourceSumId;
	}

	//------------------------------------------------------------------------------------------------
	protected string GetOrCreateSourceValueId(int definitionIndex)
	{
		EnsureStructureStorage();
		if (!m_BindingSourceValueIds.IsIndexValid(definitionIndex))
			return NewResourceId();

		if (m_BindingSourceValueIds[definitionIndex].IsEmpty())
			m_BindingSourceValueIds[definitionIndex] = NewResourceId();

		return m_BindingSourceValueIds[definitionIndex];
	}

	//------------------------------------------------------------------------------------------------
	protected string GetActionTypeForConfigAction(string configAction)
	{
		for (int i = 0; i < m_Definitions.Count(); i++)
		{
			if (m_Definitions[i].m_sConfigAction == configAction && !m_BindingActionTypes[i].IsEmpty())
				return m_BindingActionTypes[i];
		}

		return string.Empty;
	}

	//------------------------------------------------------------------------------------------------
	protected void EnsureFilterStructure(int definitionIndex)
	{
		EnsureStructureStorage();
		if (!m_Definitions.IsIndexValid(definitionIndex))
			return;

		if (m_BindingImportedStructure[definitionIndex])
			return;

		if (!m_BindingFilterTypes[definitionIndex].IsEmpty())
			return;

		HOTASBindingDefinition definition = m_Definitions[definitionIndex];
		string filterType;
		string holdDuration;
		string multiplier;

		if (definition.m_sConfigAction == "CharacterNextWeapon")
			filterType = "InputFilterSingleClick";
		else if (definition.m_sConfigAction == "TurretNextWeapon")
		{
			filterType = "InputFilterHoldOnce";
			holdDuration = "25";
		}
		else if (definition.m_sConfigAction == "SelectAction" || definition.m_sConfigAction == "HelicopterSightZeroing")
		{
			filterType = "InputFilterRepeat";
			if (definition.m_sFilterPreset == "next" || definition.m_sFilterPreset == "down")
				multiplier = "-1";
		}
		else if (NeedsNegativeMultiplier(definition))
		{
			filterType = "InputFilterValue";
			multiplier = "-1";
		}
		else if (definition.m_sFilterPreset == "toggle")
			filterType = "InputFilterDown";
		else if (definition.m_sFilterPreset == "hold" && (definition.m_sConfigAction.Contains("Engine") || definition.m_sConfigAction.Contains("ADS")))
		{
			filterType = "InputFilterHold";
			if (definition.m_sConfigAction.Contains("ADSHold"))
				holdDuration = "-1";
		}
		else if (definition.m_sConfigAction.Contains("Reset"))
			filterType = "InputFilterSingleClick";
		else if (definition.m_sConfigAction == "HelicopterSightDeploy" || definition.m_sConfigAction == "VehicleDoorToggle")
			filterType = "InputFilterClick";
		else if (definition.m_sConfigAction.Contains("EngineStop"))
			filterType = "InputFilterHoldOnce";

		if (filterType.IsEmpty())
			return;

		m_BindingFilterTypes[definitionIndex] = filterType;
		m_BindingFilterIds[definitionIndex] = NewResourceId();
		m_BindingFilterHoldDurations[definitionIndex] = holdDuration;
		m_BindingFilterMultipliers[definitionIndex] = multiplier;
	}

	//------------------------------------------------------------------------------------------------
	protected void WriteStoredFilter(FileHandle file, int definitionIndex)
	{
		EnsureFilterStructure(definitionIndex);
		if (!m_BindingFilterTypes.IsIndexValid(definitionIndex))
			return;

		string filterType = m_BindingFilterTypes[definitionIndex];
		if (filterType.IsEmpty())
			return;

		string filterId = m_BindingFilterIds[definitionIndex];
		if (filterId.IsEmpty())
		{
			filterId = NewResourceId();
			m_BindingFilterIds[definitionIndex] = filterId;
		}

		file.WriteLine(string.Format("      Filter %1 \"%2\" {", filterType, filterId));
		if (!m_BindingFilterHoldDurations[definitionIndex].IsEmpty())
			file.WriteLine(string.Format("       HoldDuration %1", m_BindingFilterHoldDurations[definitionIndex]));
		if (!m_BindingFilterMultipliers[definitionIndex].IsEmpty())
			file.WriteLine(string.Format("       Multiplier %1", m_BindingFilterMultipliers[definitionIndex]));
		file.WriteLine("      }");
	}

	//------------------------------------------------------------------------------------------------
	protected string CaptureButtonActionName(int slot, int buttonIndex)
	{
		return string.Format("HOTASCapture_J%1_B%2", slot, buttonIndex);
	}

	//------------------------------------------------------------------------------------------------
	protected string CaptureAxisActionName(int slot, int axisIndex, bool positive)
	{
		if (positive)
			return string.Format("HOTASCapture_J%1_A%2_P", slot, axisIndex);
		return string.Format("HOTASCapture_J%1_A%2_N", slot, axisIndex);
	}

	//------------------------------------------------------------------------------------------------
	protected void WriteCaptureProbeAction(FileHandle file, string actionName, string inputName, int probeIndex)
	{
		file.WriteLine(string.Format("  Action %1 {", actionName));
		file.WriteLine(string.Format("   InputSource InputSourceSum \"%1\" {", MakeGuidPrefix("D", probeIndex)));
		file.WriteLine("    Sources {");
		file.WriteLine(string.Format("     InputSourceValue \"%1\" {", MakeGuidPrefix("E", probeIndex)));
		file.WriteLine(string.Format("      Input \"%1\"", inputName));
		file.WriteLine("     }");
		file.WriteLine("    }");
		file.WriteLine("   }");
		file.WriteLine("  }");
	}

	//------------------------------------------------------------------------------------------------
	protected void WriteCaptureProbeActions(FileHandle file)
	{
		file.WriteLine("  // HOTAS Debugger private capture probes. They are only activated while the binding editor is listening.");

		for (int slot = 0; slot < HOTAS_CAPTURE_DEVICE_COUNT; slot++)
		{
			for (int buttonIndex = 0; buttonIndex < HOTAS_CAPTURE_BUTTON_COUNT; buttonIndex++)
			{
				int buttonProbeIndex = 100000 + slot * 1000 + buttonIndex;
				WriteCaptureProbeAction(
					file,
					CaptureButtonActionName(slot, buttonIndex),
					string.Format("joystick%1:button%2", slot, buttonIndex),
					buttonProbeIndex
				);
			}

			for (int axisIndex = 0; axisIndex < HOTAS_CAPTURE_AXIS_COUNT; axisIndex++)
			{
				int axisProbeBase = 200000 + slot * 1000 + axisIndex * 2;
				WriteCaptureProbeAction(
					file,
					CaptureAxisActionName(slot, axisIndex, true),
					string.Format("joystick%1:axis%2+", slot, axisIndex),
					axisProbeBase
				);
				WriteCaptureProbeAction(
					file,
					CaptureAxisActionName(slot, axisIndex, false),
					string.Format("joystick%1:axis%2-", slot, axisIndex),
					axisProbeBase + 1
				);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected bool WriteManagedConfig()
	{
		EnsureStructureStorage();
		FileIO.MakeDirectory(CONFIG_DIRECTORY);
		FileHandle file = FileIO.OpenFile(MANAGED_CONFIG_PATH, FileMode.WRITE);
		if (!file)
		{
			SetEditorStatus("ERROR: Could not write HOTAS_Config.conf.");
			return false;
		}

		file.WriteLine(MANAGED_SIGNATURE);
		file.WriteLine("// This file is fully regenerated by Reforger HOTAS Debugger.");
		file.WriteLine("// Other custom input configs are read-only to this mod.");
		file.WriteLine("ActionManager {");
		file.WriteLine(" Actions {");

		for (int i = 0; i < m_Definitions.Count(); i++)
		{
			if (!IsFirstDefinitionForConfigAction(i))
				continue;

			HOTASBindingDefinition firstDefinition = m_Definitions[i];
			string configAction = firstDefinition.m_sConfigAction;
			if (!HasBindingForConfigAction(configAction))
				continue;

			string sourceSumId = GetOrCreateSourceSumId(i);
			string actionType = GetActionTypeForConfigAction(configAction);

			file.WriteLine(string.Format("  Action %1 {", configAction));
			if (!actionType.IsEmpty())
				file.WriteLine(string.Format("   Type %1", actionType));
			else if (configAction == "SelectAction" || configAction == "HelicopterSightZeroing")
				file.WriteLine("   Type AnalogRelative");

			file.WriteLine(string.Format("   InputSource InputSourceSum \"%1\" {", sourceSumId));
			file.WriteLine("    Sources {");

			for (int j = 0; j < m_Definitions.Count(); j++)
			{
				HOTASBindingDefinition definition = m_Definitions[j];
				if (definition.m_sConfigAction != configAction || m_Bindings[j].IsEmpty())
					continue;

				string inputValue = m_Bindings[j];
				if (definition.m_bFullAxis && inputValue.Length() > 0)
				{
					string lastCharacter = inputValue.Substring(inputValue.Length() - 1, 1);
					if (lastCharacter == "+" || lastCharacter == "-")
						inputValue = inputValue.Substring(0, inputValue.Length() - 1);
				}

				file.WriteLine(string.Format("     InputSourceValue \"%1\" {", GetOrCreateSourceValueId(j)));
				file.WriteLine(string.Format("      FilterPreset \"%1\"", definition.m_sFilterPreset));
				file.WriteLine(string.Format("      Input \"%1\"", inputValue));
				WriteStoredFilter(file, j);
				file.WriteLine("     }");
			}

			file.WriteLine("    }");
			file.WriteLine("   }");
			file.WriteLine("  }");
		}

		WriteCaptureProbeActions(file);
		file.WriteLine(" }");
		file.WriteLine("}");
		file.Close();
		return true;
	}
}
