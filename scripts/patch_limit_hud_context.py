from pathlib import Path

path = Path('Scripts/Game/HOTASDebugger/HOTASDebugger.c')
src = path.read_text(encoding='utf-8')

anchor = '\tprotected void OnActionTriggered(float value = 0.0, EActionTrigger reason = 0, string actionName = string.Empty)\n\t{\n\t\tif (actionName.IsEmpty())\n\t\t\treturn;'
if anchor not in src:
    raise SystemExit('OnActionTriggered anchor not found')

helper = '''\tprotected bool IsPlayerInAircraftOrTurret()\n\t{\n\t\tChimeraCharacter character = ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());\n\t\tif (!character)\n\t\t\treturn false;\n\n\t\tCompartmentAccessComponent compartmentAccess = character.GetCompartmentAccessComponent();\n\t\tif (!compartmentAccess || !compartmentAccess.IsInCompartment())\n\t\t\treturn false;\n\n\t\tBaseCompartmentSlot slot = compartmentAccess.GetCompartment();\n\t\tif (!slot)\n\t\t\treturn false;\n\n\t\t// Any turret seat is valid, whether it is vehicle-mounted or a static emplacement.\n\t\tif (TurretCompartmentSlot.Cast(slot))\n\t\t\treturn true;\n\n\t\t// Aircraft controls are only relevant from a pilot compartment.\n\t\tif (!PilotCompartmentSlot.Cast(slot))\n\t\t\treturn false;\n\n\t\tIEntity vehicle = compartmentAccess.GetVehicleCompartmentManagerOwner();\n\t\tif (!vehicle)\n\t\t\tvehicle = slot.GetOwner();\n\t\tif (!vehicle)\n\t\t\treturn false;\n\n\t\t// Vanilla helicopters expose a helicopter controller directly.\n\t\tif (vehicle.FindComponent(HelicopterControllerComponent))\n\t\t\treturn true;\n\n\t\t// Modded fixed-wing aircraft commonly use a pilot compartment with a custom\n\t\t// controller. Exclude known ground-vehicle controllers so those seats do not\n\t\t// activate the HOTAS HUD while still allowing custom aircraft implementations.\n\t\tif (vehicle.FindComponent(SCR_CarControllerComponent))\n\t\t\treturn false;\n\t\tif (vehicle.FindComponent(SCR_TrackedControllerComponent))\n\t\t\treturn false;\n\n\t\treturn true;\n\t}\n\n'''

replacement = helper + anchor.replace(
    '\t\tif (actionName.IsEmpty())\n\t\t\treturn;',
    '\t\tif (actionName.IsEmpty())\n\t\t\treturn;\n\n\t\t// Ignore watched actions completely unless the local player is currently\n\t\t// occupying an aircraft pilot seat or a turret seat.\n\t\tif (!IsPlayerInAircraftOrTurret())\n\t\t\treturn;',
)
src = src.replace(anchor, replacement, 1)
path.write_text(src, encoding='utf-8')
