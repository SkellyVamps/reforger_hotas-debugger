# Changelog

All notable changes to Reforger HOTAS Debugger are documented here.

## 1.0.0 - Unreleased

### Added

- Compact fading HOTAS input HUD for supported helicopter, turret, shared vehicle, and fixed-wing/PFC actions.
- Native **Settings > HOTAS** configuration page.
- Automatic joystick binding and Roll/Pitch/Throttle/Yaw axis detection from the active input configuration.
- Editable axis labels and direction-specific Free Look labels.
- Exact draggable HUD placement using normalized X/Y coordinates.
- HUD scale, fade delay, fade duration, background visibility, and background opacity controls.
- Reset controls for HUD presentation settings and custom labels.
- Day/Night cockpit reference images selected by the closest display resolution/aspect ratio.
- Static Pitch Forward HUD example in the placement preview.
- DPI-correct runtime HUD sizing and placement across Workbench and game viewports.
- Debug mode showing raw action, binding, and input value information.

### Defaults

- Roll/Pitch/Throttle/Yaw labels default to `Roll`, `Pitch`, `Throttle`, and `Yaw`.
- Free Look labels default to `Thumb Up`, `Thumb Down`, `Thumb Right`, and `Thumb Left`.
- HUD position defaults to normalized `0.5 / 0.95`.
- HUD scale defaults to `1.0`.
- Background defaults to enabled at `0.55` opacity.

### Notes

- `$profile:HOTASHudSettings.txt` is generated locally and is intentionally not tracked by Git.
- Legacy fixed-position settings are migrated to normalized placement when loaded.
