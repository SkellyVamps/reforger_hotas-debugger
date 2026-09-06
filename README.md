# Arma Reforger HOTAS Debugger

Client-side diagnostic HUD for Arma Reforger HOTAS configurations.

The debugger listens to Reforger input **actions**, asks the engine which joystick inputs are bound to the action, and shows a compact input HUD in normal mode while preserving the detailed diagnostic view for debug mode.

## Normal HUD

Normal mode uses readable input labels instead of raw Reforger binding strings. Examples:

```text
BUTTON 27   •   Next Weapon
AXIS 1-     •   Cyclic Left
BUTTON 26   •   Missile Ripple
```

Buttons and axes are shown as 1-based numbers for human readability.

The HUD fades in immediately on input, stays visible for the configured delay, then fades smoothly. A dark background panel can be enabled or disabled.

## HUD settings

On first run the mod creates:

```text
$profile:HOTASHudSettings.txt
```

Default contents:

```text
position=bottom_center
scale=1.0
fade_delay_ms=1800
fade_duration_ms=350
background=1
background_opacity=0.55
```

Supported positions:

```text
top_left
top_center
top_right
center_left
center
center_right
bottom_left
bottom_center
bottom_right
```

`scale` is clamped from `0.5` to `2.0`. `background_opacity` is clamped from `0.0` to `1.0`. Restart the play session after editing the file so the HUD reloads the settings.

## Debug mode

The existing detailed debug presentation remains available in code through `m_bDebugMode` and keeps the event counter, raw action name, action value, and raw binding output.

## Source layout

```text
Scripts/Game/HOTASDebugger/
  HOTASDebugger.c
```

## Importing into Arma Reforger Tools

1. Install **Arma Reforger Tools** from Steam.
2. Create/open the `ReforgerHOTASDebugger` addon project in Workbench.
3. Copy or pull this repository into the addon root.
4. Compile Game scripts and check the Script Editor output for errors.
5. Launch a test world or scenario and verify the overlay appears when one of the watched actions is pressed.

## How detection works

The script uses Reforger's input APIs to listen for actions and query their active joystick bindings. The HUD is client-side and does not modify gameplay state.

## Current limitations

- The engine callback identifies which action fired, not necessarily which one of several bindings on that action was the exact physical source. The HUD therefore displays the joystick binding(s) currently assigned to that action.
- Settings are currently edited through `HOTASHudSettings.txt`; an in-game settings screen can be added later.

## License

MIT
