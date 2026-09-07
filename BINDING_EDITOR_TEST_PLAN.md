# HOTAS Binding Editor test plan

Test branch: `feature/hotas-bindings-working`

This branch is intentionally not merged into `main` until the feature is tested in Workbench and the retail game.

## Safety boundary

The binding editor may write only:

- `$profile:.save/settings/customInputConfigs/HOTAS_Config.conf`

Existing HUD/debugger settings continue to use the mod-owned `$profile:HOTASHudSettings.txt` file.

Every other `.conf` in `customInputConfigs` is treated as a read-only import source. Importing a config reads its supported action/input pairs and writes a fresh `HOTAS_Config.conf`; the source file is not modified.

Every managed-config save rewrites the entire `HOTAS_Config.conf`, so cleared or removed bindings cannot remain in old Action blocks.

## Current UI direction

The binding page follows the stock Reforger Controls layout rather than using one large action spinbox:

- category selector across the top of the action pane
- scrollable action rows with the managed joystick binding shown in-row
- action details and Bind/Clear controls in the right-hand details pane
- read-only import and managed-config controls in the right-hand pane
- stock-style large paging arrows on custom spinboxes

The stock `SCR_KeybindRowComponent` is intentionally not used because it writes normal player bindings. The custom rows only update the in-memory managed model and regenerate `HOTAS_Config.conf`.

## Test sequence

1. Check out/pull `feature/hotas-bindings-working` and restart Workbench.
2. Run Script Validation before opening the game preview.
3. From the main menu, open Settings and confirm both `HOTAS` and `HOTAS Bindings` are present before loading a scenario.
4. Check every spinbox on the existing `HOTAS` page. Confirm both paging arrows remain visible at the first, middle, and last options.
5. Open `HOTAS Bindings`. Confirm the page resembles the stock Controls layout with a category selector, action rows, and a right-hand details pane.
6. Cycle through every HOTAS category and confirm the left/right category arrows remain visible.
7. Confirm `HOTAS_Config.conf` is created in `.save/settings/customInputConfigs`.
8. Select a helicopter action row, press `Bind Input`, then move/press the intended joystick control. Confirm the binding updates both in the row and in the details pane.
9. Bind a button above button31 to confirm the editor is not limited to the old browser Gamepad API range.
10. Bind at least one axis direction and confirm the generated `.conf` contains the correct `joystickN:axisN+/-` input.
11. Test Previous Action and Next Action. Confirm the generated `SelectAction` block uses `Type AnalogRelative`, `InputFilterRepeat`, and `Multiplier -1` for Next.
12. Clear one binding. Open `HOTAS_Config.conf` and confirm the removed binding/action source is no longer present after regeneration.
13. Use `Import Existing Config` to import a known working external `.conf`. Compare the source file before/after and confirm it was not changed.
14. Confirm the imported bindings appear in the appropriate action rows and are written into `HOTAS_Config.conf`.
15. Confirm the Import Existing Config spinbox keeps both arrows visible while changing selections.
16. Enter a helicopter and verify the managed config works in actual gameplay, including context-menu Previous/Next.
17. Restart Reforger and confirm `HOTAS_Config.conf` remains selectable/active and the HOTAS HUD labels/settings are unchanged.

## Do not merge yet

Keep this branch separate until Workbench script validation passes and the generated config has been tested in the retail game.
