# Arma Reforger HOTAS Debugger

Client-side diagnostic HUD for Arma Reforger HOTAS configurations.

The debugger listens to Reforger input **actions**, then asks the engine which inputs are currently bound to the action and shows the result on-screen. It is intended to help diagnose HOTAS configs made with the expanded configurator.

## Initial goals

- Show the action that Reforger actually received.
- Show the current joystick binding(s) attached to that action.
- Show the action value when useful for analog controls.
- Keep the HUD local to the client; no gameplay state is replicated or modified.
- Cover the helicopter, turret, vehicle, WCS, and fixed-wing actions supported by the configurator.
- Make repeated inputs easy to spot when one physical press is being interpreted more than once.

Example HUD output:

```text
HOTAS INPUT DEBUG
Input: joystick1:button8
Action: TurretNextWeapon
Value: 1.00
```

If one action currently has multiple joystick bindings, they are shown together because Reforger's action callback identifies the action that fired, while the binding API exposes the bindings configured for that action.

## Source layout

```text
Scripts/Game/HOTASDebugger/
  HOTASDebugger.c
```

## Importing into Arma Reforger Tools

1. Install **Arma Reforger Tools** from Steam.
2. Create a new addon project in Workbench named `ReforgerHOTASDebugger`.
3. Copy this repository's `Scripts` folder into the addon root.
4. Open the project in Workbench.
5. Compile Game scripts and check the Script Editor output for errors.
6. Launch a test world or scenario and verify the overlay appears when one of the watched actions is pressed.

A Workbench-created `.gproj` is intentionally not committed yet because Workbench generates the project GUID and addon metadata. Once the addon is created locally, its generated project file can be committed to this repository.

## How detection works

The script uses the public Reforger input API:

- `GetGame().GetInputManager()`
- `ActionManager.AddActionListener(...)`
- `InputBinding.GetBindings(...)`

The first version listens for the configured action list rather than modifying or replacing the game's input configuration.

## Current limitations

- The engine callback tells us **which action fired**, not which one of several bindings on that action was the exact physical source. The HUD therefore displays the joystick bindings currently assigned to that action.
- Analog-axis handling will be expanded after the first in-Workbench compile/test pass so continuous values do not flood the HUD.
- This is a client diagnostic addon, but server/workshop policies can still determine whether a client may join with an addon loaded.

## License

MIT
