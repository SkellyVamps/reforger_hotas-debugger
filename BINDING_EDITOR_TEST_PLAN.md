# HOTAS Binding Editor test plan

Test branch: `feature/hotas-bindings-working`

This branch is intentionally not merged into `main` until the feature is tested in Workbench and the retail game.

## Safety boundary

The binding editor may write only:

- `$profile:.save/settings/customInputConfigs/HOTAS_Config.conf`

Existing HUD/debugger settings continue to use the mod-owned `$profile:HOTASHudSettings.txt` file.

Every other `.conf` in `customInputConfigs` is treated as a read-only import source. Importing a config reads its supported action/input pairs and writes a fresh `HOTAS_Config.conf`; the source file is not modified.

Every managed-config save rewrites the entire `HOTAS_Config.conf`, so cleared or removed bindings cannot remain in old Action blocks.

## Morning test

1. Check out/pull `feature/hotas-bindings-working` and restart Workbench.
2. Run Script Validation before opening the game preview.
3. From the main menu, open Settings and confirm both `HOTAS` and `HOTAS Bindings` are present before loading a scenario.
4. Open `HOTAS Bindings`. Confirm `HOTAS_Config.conf` is created in `.save/settings/customInputConfigs`.
5. Select a helicopter action, press `Bind Input`, then move/press the intended joystick control. Confirm the displayed binding updates.
6. Bind a button above button31 to confirm the editor is not limited to the old browser Gamepad API range.
7. Bind at least one axis direction and confirm the generated `.conf` contains the correct `joystickN:axisN+/-` input.
8. Test Previous Action and Next Action. Confirm the generated `SelectAction` block uses `Type AnalogRelative`, `InputFilterRepeat`, and `Multiplier -1` for Next.
9. Clear one binding. Open `HOTAS_Config.conf` and confirm the removed binding/action source is no longer present after regeneration.
10. Use `Import Existing Config` to import a known working external `.conf`. Compare the source file before/after and confirm it was not changed.
11. Confirm the imported bindings appear in the action selector and are written into `HOTAS_Config.conf`.
12. Enter a helicopter and verify the managed config works in actual gameplay, including context-menu Previous/Next.
13. Restart Reforger and confirm `HOTAS_Config.conf` remains selectable/active and the HOTAS HUD labels/settings are unchanged.

## Do not merge yet

Keep this branch separate until Workbench script validation passes and the generated config has been tested in the retail game.
