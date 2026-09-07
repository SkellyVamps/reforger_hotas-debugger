//------------------------------------------------------------------------------------------------
// Keep arrows visible on the custom HOTAS settings controls.
//
// The stock settings UI keeps its paging controls visually present while our previous helper
// disabled arrow widgets at the ends of a list. Depending on the WLib spinbox style that could
// make the arrow disappear entirely. Leave the paging widgets enabled/visible and let the
// spinbox component decide whether the selection itself can change.
modded class HOTASSettingsSubMenu
{
	//------------------------------------------------------------------------------------------------
	override protected void RefreshSpinBoxArrows(SCR_SpinBoxComponent control, int selected, int optionCount)
	{
		if (!control || optionCount <= 0)
			return;

		Widget root = control.GetRootWidget();
		if (!root)
			return;

		KeepHotasSpinBoxArrowVisible(root.FindAnyWidget("ButtonLeft"));
		KeepHotasSpinBoxArrowVisible(root.FindAnyWidget("ButtonRight"));
	}

	//------------------------------------------------------------------------------------------------
	protected void KeepHotasSpinBoxArrowVisible(Widget arrowWidget)
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
