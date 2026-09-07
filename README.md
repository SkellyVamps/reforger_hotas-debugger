# Arma Reforger HOTAS Debugger

Client-side HOTAS input HUD and diagnostic utility for Arma Reforger.

The mod listens to Reforger input **actions**, resolves the active joystick bindings for those actions, and displays a compact readable HUD. It also adds a native **HOTAS** tab to the Reforger Settings menu for configuring and testing the overlay.

## Features

- Native **Settings > HOTAS** configuration tab
- Automatic discovery of the active HOTAS/custom input configuration
- Compact fading input HUD
- Exact draggable HUD positioning instead of fixed screen anchors
- HUD scale, fade timing, background visibility, and background opacity controls
- Automatic Roll, Pitch, Throttle, and Yaw axis detection from the active input configuration
- Editable player-facing labels for Roll, Pitch, Throttle, and Yaw
- Direction-specific Free Look labels for thumbsticks, hats, ministicks, or other controls
- Detailed debug mode with raw action, binding, and value information
- Live input tester in the HOTAS settings page
- Day/night HUD-position preview with resolution/aspect-aware cockpit backgrounds
- Reset controls for HUD settings and custom labels

## Normal HUD

Normal mode converts Reforger binding strings into readable input names. Examples:

```text
BUTTON 27   |   Next Weapon
ROLL -      |   Cyclic Left
Thumbstick Right   |   Free Look Right
```

Buttons are shown using 1-based numbering for readability. Known flight axes use the custom labels configured in Settings. Unknown axes fall back to `AXIS N+/-` rather than being mislabeled.

The HUD appears immediately when a watched HOTAS action fires, remains visible for the configured delay, and then fades smoothly.

## HOTAS Settings

Open the normal Reforger Settings menu and select the **HOTAS** tab.

### Input Profile

**HOTAS Config** selects one of the joystick/custom input configurations found in the player's Reforger profile. Axis assignments are refreshed from the selected configuration.

### HOTAS Input HUD

The HUD controls include:

- **HUD Enabled**
- **Scale**
- **Fade Delay**
- **Fade Duration**
- **Background**
- **Background Opacity**

HUD placement is controlled from the preview pane. Drag the example HUD to any point inside the previewed screen. Position is stored as normalized X/Y coordinates so it remains valid across resolutions and HUD scales.

The preview uses the closest supplied reference aspect ratio for the current display and provides a **Day / Night** selector. The example HUD reflects the configured scale, background visibility, background opacity, and custom axis label so users can estimate its final appearance before returning to gameplay.

### Axis Labels

The mod discovers the physical bindings used for:

- Roll
- Pitch
- Throttle
- Yaw

Each row shows the detected axis and provides an editable display label. These labels affect presentation only; they do not modify the actual Reforger binding.

### Free Look Labels

Free Look Up, Down, Right, and Left are tracked separately so a direction can be named after the physical control that produced it. New settings files use these defaults:

```text
Thumb Up
Thumb Down
Thumb Right
Thumb Left
```

The values remain fully editable in the HOTAS settings page. Leaving a Free Look label blank keeps the normal raw `AXIS N+/-` display.

### Reset Controls

The settings page provides separate reset actions for HUD presentation settings and custom labels. Resetting labels restores the default Roll/Pitch/Throttle/Yaw and Thumb direction names without moving the HUD. Resetting the HUD does not change the active HOTAS input configuration.

### Live Input Tester

The live tester appears below the HUD-position preview. While a supported HOTAS action is received, it shows the same readable input/action pair used by the HUD together with raw diagnostic information. This is intended for verifying button numbers, axis directions, custom labels, and action detection without guessing which control Reforger resolved.

## Settings File

The UI persists its values to:

```text
$profile:HOTASHudSettings.txt
```

This is a local per-profile settings file and is ignored by the repository. If the file is missing, the mod generates it automatically with defaults equivalent to:

```text
hud_enabled=1
debug_mode=0
position_x=0.5
position_y=0.95
scale=1.0
fade_delay_ms=1800
fade_duration_ms=350
background=1
background_opacity=0.55
roll_label=Roll
pitch_label=Pitch
throttle_label=Throttle
yaw_label=Yaw
freelook_up_label=Thumb Up
freelook_down_label=Thumb Down
freelook_right_label=Thumb Right
freelook_left_label=Thumb Left
```

Normal use should be done through the in-game HOTAS settings page. The text file mainly remains useful for troubleshooting and migration from older versions.

Legacy files that still contain a fixed `position=top_left`, `bottom_center`, etc. entry are converted to normalized placement when loaded.

## Debug Mode

Debug mode displays detailed input information including:

```text
Input: joystick0:button26
Action: Next Weapon
Raw action: CharacterNextWeapon
Value: 1.00
```

This is useful when adding support for a new controller, aircraft mod, or action name.

## How Detection Works

The script registers listeners for supported Reforger actions. When an action fires it:

1. Determines the current HOTAS context (turret, helicopter, or fixed-wing).
2. Queries the active runtime input manager for joystick bindings.
3. Falls back to the user `InputBinding` object when required.
4. Converts the resolved binding into a readable label.
5. Displays the readable input and action in the HUD and live tester.

The mod is client-side and does not change gameplay state or alter the physical joystick binding simply to display a custom name.

## Project Layout

```text
Scripts/Game/HOTASDebugger/
  HOTASDebugger.c
  HOTASSettingsTab.c

UI/layouts/HUD/HOTAS/
  HOTASInputHUD.layout

UI/layouts/Menus/SettingsSubMenus/
  HOTASSettings.layout

UI/Textures/HOTASPreview/
  preview reference images
```

## Development / Workbench

1. Install **Arma Reforger Tools** from Steam.
2. Open the `ReforgerHOTASDebugger` addon project in Workbench.
3. Pull this repository into the addon root.
4. Allow Workbench to import/update registered resources when required.
5. Run **Script Validation** and resolve any Game-script errors.
6. Launch a test scenario and verify the HOTAS Settings tab and HUD.

## Current Limitation

The engine action callback identifies the action that fired, but an action can have multiple bindings. The mod queries the joystick binding(s) currently assigned to that action and, where the action exposes direction through its value, selects the appropriate directional binding. This is more reliable than assuming a specific physical HOTAS layout, but unusual multi-binding configurations can still be ambiguous.

## License

MIT
