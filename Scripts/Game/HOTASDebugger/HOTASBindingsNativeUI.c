//------------------------------------------------------------------------------------------------
// Native-style presentation layer for the managed HOTAS binding editor.
//
// The stock Reforger controls menu is used as the visual/interaction reference: category selector
// at the top, a scrollable action list on the left, and details/actions on the right. The stock
// SCR_KeybindRowComponent is intentionally not used because it writes normal player keybinds;
// this page must continue writing only the mod-owned HOTAS_Config.conf.
class HOTASBindingRowHandler : ScriptedWidgetEventHandler
{
	protected HOTASBindingsSubMenu m_Owner;
	protected int m_iDefinitionIndex;

	void HOTASBindingRowHandler(HOTASBindingsSubMenu owner, int definitionIndex)
	{
		m_Owner = owner;
		m_iDefinitionIndex = definitionIndex;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (button != 0 || !m_Owner)
			return false;

		m_Owner.SelectBindingActionFromRow(m_iDefinitionIndex);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		if (m_Owner)
			m_Owner.SelectBindingActionFromRow(m_iDefinitionIndex);
		return false;
	}
}

//------------------------------------------------------------------------------------------------
modded class HOTASBindingsSubMenu
{
	protected static const string HOTAS_ACTION_ROW_LAYOUT = "{B45D7E926AF13C80}UI/layouts/Menus/SettingsSubMenus/HOTASBindingActionRow.layout";

	protected SCR_SpinBoxComponent m_CategorySelector;
	protected VerticalLayoutWidget m_ActionRowsLayout;
	protected ScrollLayoutWidget m_ActionRowsScroll;
	protected TextWidget m_DescriptionHeader;
	protected RichTextWidget m_DescriptionText;

	protected ref array<string> m_CategoryNames = {};
	protected ref array<Widget> m_ActionRowWidgets = {};
	protected ref array<int> m_ActionRowDefinitionIndices = {};
	protected ref array<ref HOTASBindingRowHandler> m_ActionRowHandlers = {};

	//------------------------------------------------------------------------------------------------
	override void OnTabCreate(Widget menuRoot, ResourceName buttonsLayout, int index)
	{
		super.OnTabCreate(menuRoot, buttonsLayout, index);
		SetupNativeBindingPage();
	}

	//------------------------------------------------------------------------------------------------
	override void OnTabShow()
	{
		super.OnTabShow();

		if (!m_CategorySelector)
			SetupNativeBindingPage();
		else
		{
			ListActionsForCurrentCategory();
			RefreshCurrentBinding();
			KeepSpinBoxArrowsVisible(m_CategorySelector);
			KeepSpinBoxArrowsVisible(m_ImportSelector);
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void RefreshImportSelector()
	{
		super.RefreshImportSelector();
		KeepSpinBoxArrowsVisible(m_ImportSelector);
	}

	//------------------------------------------------------------------------------------------------
	override protected void RefreshCurrentBinding()
	{
		super.RefreshCurrentBinding();
		RefreshActionRowBindingText();
		RefreshActionRowSelection();
		UpdateBindingDescription();
	}

	//------------------------------------------------------------------------------------------------
	protected void SetupNativeBindingPage()
	{
		Widget categoryWidget = m_wRoot.FindAnyWidget("CategoriesBox");
		if (categoryWidget)
			m_CategorySelector = SCR_SpinBoxComponent.Cast(categoryWidget.FindHandler(SCR_SpinBoxComponent));

		m_ActionRowsLayout = VerticalLayoutWidget.Cast(m_wRoot.FindAnyWidget("ActionRowsContent"));
		m_ActionRowsScroll = ScrollLayoutWidget.Cast(m_wRoot.FindAnyWidget("ActionRowScroll"));
		m_DescriptionHeader = TextWidget.Cast(m_wRoot.FindAnyWidget("DescriptionHeader"));
		m_DescriptionText = RichTextWidget.Cast(m_wRoot.FindAnyWidget("Description"));

		BuildCategorySelector();
		ListActionsForCurrentCategory();
		KeepSpinBoxArrowsVisible(m_CategorySelector);
		KeepSpinBoxArrowsVisible(m_ImportSelector);

		if (m_CategorySelector)
			m_CategorySelector.m_OnChanged.Insert(OnHotasCategoryChanged);

		SetEditorStatus("Select an action row, then choose Bind Input or Clear Binding.");
		RefreshCurrentBinding();
	}

	//------------------------------------------------------------------------------------------------
	protected void BuildCategorySelector()
	{
		if (!m_CategorySelector)
			return;

		m_CategoryNames.Clear();
		foreach (HOTASBindingDefinition definition : m_Definitions)
		{
			if (!m_CategoryNames.Contains(definition.m_sCategory))
				m_CategoryNames.Insert(definition.m_sCategory);
		}

		m_CategorySelector.ClearAll();
		int categoryCount = m_CategoryNames.Count();
		for (int i = 0; i < categoryCount; i++)
			m_CategorySelector.AddItem(m_CategoryNames[i], i == categoryCount - 1);

		if (categoryCount > 0)
			m_CategorySelector.SetCurrentItem(0, false, false, false);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnHotasCategoryChanged(SCR_SpinBoxComponent component, int index)
	{
		ListActionsForCurrentCategory();
		KeepSpinBoxArrowsVisible(m_CategorySelector);
	}

	//------------------------------------------------------------------------------------------------
	protected void ListActionsForCurrentCategory()
	{
		if (!m_ActionRowsLayout || !m_CategorySelector || m_CategoryNames.IsEmpty())
			return;

		foreach (Widget row : m_ActionRowWidgets)
		{
			if (row)
				row.RemoveFromHierarchy();
		}

		m_ActionRowWidgets.Clear();
		m_ActionRowDefinitionIndices.Clear();
		m_ActionRowHandlers.Clear();

		int categoryIndex = m_CategorySelector.GetCurrentIndex();
		if (!m_CategoryNames.IsIndexValid(categoryIndex))
			categoryIndex = 0;

		string categoryName = m_CategoryNames[categoryIndex];
		for (int i = 0; i < m_Definitions.Count(); i++)
		{
			HOTASBindingDefinition definition = m_Definitions[i];
			if (definition.m_sCategory != categoryName)
				continue;

			Widget row = GetGame().GetWorkspace().CreateWidgets(HOTAS_ACTION_ROW_LAYOUT, m_ActionRowsLayout);
			if (!row)
				continue;

			TextWidget actionName = TextWidget.Cast(row.FindAnyWidget("ActionName"));
			if (actionName)
				actionName.SetText(definition.m_sDisplayName);

			TextWidget bindingValue = TextWidget.Cast(row.FindAnyWidget("BindingValue"));
			if (bindingValue)
				bindingValue.SetText(GetBindingDisplayText(i));

			HOTASBindingRowHandler rowHandler = new HOTASBindingRowHandler(this, i);
			row.AddHandler(rowHandler);

			m_ActionRowWidgets.Insert(row);
			m_ActionRowDefinitionIndices.Insert(i);
			m_ActionRowHandlers.Insert(rowHandler);
		}

		if (!m_ActionRowDefinitionIndices.IsEmpty())
			m_iSelectedAction = m_ActionRowDefinitionIndices[0];

		if (m_ActionRowsScroll)
			m_ActionRowsScroll.SetSliderPos(0, 0);

		RefreshActionRowBindingText();
		RefreshActionRowSelection();
		UpdateBindingDescription();
	}

	//------------------------------------------------------------------------------------------------
	string GetBindingDisplayText(int definitionIndex)
	{
		if (!m_Bindings.IsIndexValid(definitionIndex))
			return "Unassigned";

		string bindingText = m_Bindings[definitionIndex];
		if (bindingText.IsEmpty())
			return "Unassigned";

		return bindingText;
	}

	//------------------------------------------------------------------------------------------------
	void SelectBindingActionFromRow(int definitionIndex)
	{
		if (!m_Definitions.IsIndexValid(definitionIndex))
			return;

		m_iSelectedAction = definitionIndex;
		RefreshCurrentBinding();
	}

	//------------------------------------------------------------------------------------------------
	protected void RefreshActionRowBindingText()
	{
		for (int i = 0; i < m_ActionRowWidgets.Count(); i++)
		{
			if (!m_ActionRowDefinitionIndices.IsIndexValid(i))
				continue;

			Widget row = m_ActionRowWidgets[i];
			if (!row)
				continue;

			TextWidget bindingValue = TextWidget.Cast(row.FindAnyWidget("BindingValue"));
			if (bindingValue)
				bindingValue.SetText(GetBindingDisplayText(m_ActionRowDefinitionIndices[i]));
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void RefreshActionRowSelection()
	{
		for (int i = 0; i < m_ActionRowWidgets.Count(); i++)
		{
			if (!m_ActionRowDefinitionIndices.IsIndexValid(i))
				continue;

			Widget row = m_ActionRowWidgets[i];
			if (!row)
				continue;

			Widget selection = row.FindAnyWidget("RowSelection");
			if (!selection)
				continue;

			if (m_ActionRowDefinitionIndices[i] == m_iSelectedAction)
				selection.SetOpacity(1.0);
			else
				selection.SetOpacity(0.0);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateBindingDescription()
	{
		if (!m_Definitions.IsIndexValid(m_iSelectedAction))
		{
			if (m_DescriptionHeader)
				m_DescriptionHeader.SetText("HOTAS Bindings");
			if (m_DescriptionText)
				m_DescriptionText.SetText("Select an action from the list.");
			return;
		}

		HOTASBindingDefinition definition = m_Definitions[m_iSelectedAction];
		string bindingText = GetBindingDisplayText(m_iSelectedAction);
		string inputType = GetDefinitionInputType(definition);

		if (m_DescriptionHeader)
			m_DescriptionHeader.SetText(definition.m_sDisplayName);

		if (m_DescriptionText)
		{
			string descriptionText = string.Format(
				"Category: %1\nGame action: %2\nInput type: %3\nManaged binding: %4\n\nBind Input listens only for joystick/HOTAS input. Changes regenerate HOTAS_Config.conf; other custom .conf files are read-only import sources.",
				definition.m_sCategory,
				definition.m_sConfigAction,
				inputType,
				bindingText
			);

			if (definition.m_sCategory == "WCS Armament")
				descriptionText += "\n\nWCS actions are optional and can only be captured when the corresponding WCS actions are available in the currently loaded mod set.";

			m_DescriptionText.SetText(descriptionText);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected string GetDefinitionInputType(HOTASBindingDefinition definition)
	{
		if (!definition)
			return "Joystick input";

		if (definition.m_bFullAxis)
			return "Full analog axis";

		string preset = definition.m_sFilterPreset;
		if (preset == "up" || preset == "down" || preset == "left" || preset == "right" || preset == "forward" || preset == "back" || preset == "previous" || preset == "next")
			return "Directional axis or button";

		return "Button / digital input";
	}

	//------------------------------------------------------------------------------------------------
	protected void KeepSpinBoxArrowsVisible(SCR_SpinBoxComponent spinBox)
	{
		if (!spinBox)
			return;

		Widget root = spinBox.GetRootWidget();
		if (!root)
			return;

		KeepSpinBoxArrowVisible(root.FindAnyWidget("ButtonLeft"));
		KeepSpinBoxArrowVisible(root.FindAnyWidget("ButtonRight"));
	}

	//------------------------------------------------------------------------------------------------
	protected void KeepSpinBoxArrowVisible(Widget arrowWidget)
	{
		if (!arrowWidget)
			return;

		arrowWidget.SetVisible(true);
		arrowWidget.SetOpacity(1.0);
		arrowWidget.SetEnabled(true);

		SCR_PagingButtonComponent pagingButton = SCR_PagingButtonComponent.Cast(arrowWidget.FindHandler(SCR_PagingButtonComponent));
		if (pagingButton)
			pagingButton.SetEnabled(true, false);

		Widget background = arrowWidget.FindAnyWidget("BackgroundImage");
		if (background)
			background.SetVisible(true);
	}
}
