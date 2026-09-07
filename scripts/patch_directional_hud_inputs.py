from pathlib import Path

path = Path('Scripts/Game/HOTASDebugger/HOTASDebugger.c')
src = path.read_text(encoding='utf-8')

old = '''\tvoid Shutdown()\n\t{\n\t\tif (!m_bInitialized)\n\t\t\treturn;\n\n\t\tif (m_InputManager)\n\t\t{\n\t\t\tforeach (string actionName : m_WatchedActions)\n\t\t\t\tm_InputManager.RemoveActionListener(actionName, EActionTrigger.DOWN, OnActionTriggered);\n\n\t\t}\n'''
new = '''\tvoid Shutdown()\n\t{\n\t\tif (!m_bInitialized)\n\t\t\treturn;\n\n\t\tif (m_InputManager)\n\t\t{\n\t\t\tforeach (string actionName : m_WatchedActions)\n\t\t\t{\n\t\t\t\tif (UsesDirectionalValueListener(actionName))\n\t\t\t\t\tm_InputManager.RemoveActionListener(actionName, EActionTrigger.VALUE, OnActionTriggered);\n\t\t\t\telse\n\t\t\t\t\tm_InputManager.RemoveActionListener(actionName, EActionTrigger.DOWN, OnActionTriggered);\n\t\t\t}\n\t\t}\n'''
if old not in src:
    raise SystemExit('Shutdown listener block not found')
src = src.replace(old, new, 1)

old = '''\tprotected void RegisterListeners()\n\t{\n\t\tforeach (string actionName : m_WatchedActions)\n\t\t\tm_InputManager.AddActionListener(actionName, EActionTrigger.DOWN, OnActionTriggered);\n\n\t}\n'''
new = '''\tprotected bool UsesDirectionalValueListener(string actionName)\n\t{\n\t\treturn actionName == "SelectAction" || actionName == "HelicopterSightZeroing";\n\t}\n\n\tprotected void RegisterListeners()\n\t{\n\t\tforeach (string actionName : m_WatchedActions)\n\t\t{\n\t\t\t// These two actions are AnalogRelative. Their second binding emits a negative\n\t\t\t// value, so a DOWN listener only sees the positive/first direction. VALUE\n\t\t\t// lets the HUD identify both directions without changing the game binding.\n\t\t\tif (UsesDirectionalValueListener(actionName))\n\t\t\t\tm_InputManager.AddActionListener(actionName, EActionTrigger.VALUE, OnActionTriggered);\n\t\t\telse\n\t\t\t\tm_InputManager.AddActionListener(actionName, EActionTrigger.DOWN, OnActionTriggered);\n\t\t}\n\t}\n'''
if old not in src:
    raise SystemExit('RegisterListeners block not found')
src = src.replace(old, new, 1)

anchor = '''\tprotected void OnActionTriggered(float value = 0.0, EActionTrigger reason = 0, string actionName = string.Empty)\n\t{\n'''
helpers = '''\tprotected string GetDirectionalBindingForValue(string bindingsText, float value)\n\t{\n\t\tref array<string> bindings = {};\n\t\tbindingsText.Split(" / ", bindings, true);\n\t\tif (bindings.Count() <= 1)\n\t\t\treturn bindingsText;\n\n\t\t// The configurator writes previous/up first and next/down second. The second\n\t\t// binding carries Multiplier -1, so its runtime action value is negative.\n\t\tif (value < 0.0)\n\t\t\treturn bindings[1];\n\t\treturn bindings[0];\n\t}\n\n\tprotected string GetDirectionalActionName(string actionName, float value)\n\t{\n\t\tif (actionName == "SelectAction")\n\t\t{\n\t\t\tif (value < 0.0)\n\t\t\t\treturn "Next Action";\n\t\t\treturn "Previous Action";\n\t\t}\n\n\t\tif (actionName == "HelicopterSightZeroing")\n\t\t{\n\t\t\tif (value < 0.0)\n\t\t\t\treturn "Sight Zeroing Down";\n\t\t\treturn "Sight Zeroing Up";\n\t\t}\n\n\t\treturn MakeReadableActionName(actionName);\n\t}\n\n'''
if anchor not in src:
    raise SystemExit('OnActionTriggered anchor not found')
src = src.replace(anchor, helpers + anchor, 1)

old = '''\t\tif (!IsActionAllowedForContext(actionName, hotasContext))\n\t\t\treturn;\n\n\t\tm_iEventCounter++;\n\n\t\tstring bindingsText = GetJoystickBindings(actionName);\n\t\tstring readableAction = MakeReadableActionName(actionName);\n'''
new = '''\t\tif (!IsActionAllowedForContext(actionName, hotasContext))\n\t\t\treturn;\n\n\t\tbool directionalValueAction = UsesDirectionalValueListener(actionName);\n\t\tif (directionalValueAction && value > -0.001 && value < 0.001)\n\t\t\treturn;\n\n\t\tm_iEventCounter++;\n\n\t\tstring bindingsText = GetJoystickBindings(actionName);\n\t\tstring readableAction = MakeReadableActionName(actionName);\n\t\tif (directionalValueAction)\n\t\t{\n\t\t\tbindingsText = GetDirectionalBindingForValue(bindingsText, value);\n\t\t\treadableAction = GetDirectionalActionName(actionName, value);\n\t\t}\n'''
if old not in src:
    raise SystemExit('OnActionTriggered body block not found')
src = src.replace(old, new, 1)

replacements = {
    'case "FreelookUp": return "Look Up";': 'case "FreelookUp": return "Free Look Up";',
    'case "FreelookDown": return "Look Down";': 'case "FreelookDown": return "Free Look Down";',
    'case "FreelookLeft": return "Look Left";': 'case "FreelookLeft": return "Free Look Left";',
    'case "FreelookRight": return "Look Right";': 'case "FreelookRight": return "Free Look Right";',
}
for old_text, new_text in replacements.items():
    if old_text not in src:
        raise SystemExit(f'Free-look label not found: {old_text}')
    src = src.replace(old_text, new_text, 1)

path.write_text(src, encoding='utf-8')
