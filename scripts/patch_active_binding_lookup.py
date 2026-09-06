from pathlib import Path

path = Path('Scripts/Game/HOTASDebugger/HOTASDebugger.c')
src = path.read_text(encoding='utf-8')
start = src.index('\tprotected string GetJoystickBindings(string actionName)')
end = src.index('\n\tprotected string MakeReadableActionName', start)
new = '''\tprotected string GetJoystickBindings(string actionName)
\t{
\t\tif (!m_InputManager)
\t\t\treturn "InputManager unavailable";

\t\tstring joystickBindings;

\t\t// Query the active runtime ActionManager first. The separate InputBinding object can
\t\t// report zero bindings even while a custom joystick config is actively driving actions.
\t\tfor (int bindIndex = 0; bindIndex < 16; bindIndex++)
\t\t{
\t\t\tref array<string> keyStack = {};
\t\t\tref array<BaseContainer> filterStack = {};
\t\t\tbool found = m_InputManager.GetActionKeybinding(
\t\t\t\tactionName,
\t\t\t\tkeyStack,
\t\t\t\tfilterStack,
\t\t\t\tEInputDeviceType.JOYSTICK,
\t\t\t\tstring.Empty,
\t\t\t\tbindIndex
\t\t\t);

\t\t\tif (!found)
\t\t\t\tbreak;

\t\t\tforeach (string binding : keyStack)
\t\t\t{
\t\t\t\tif (!joystickBindings.IsEmpty())
\t\t\t\t\tjoystickBindings += " / ";

\t\t\t\tjoystickBindings += binding;
\t\t\t}
\t\t}

\t\tif (!joystickBindings.IsEmpty())
\t\t\treturn joystickBindings;

\t\t// Fallback for actions where the runtime manager does not expose an indexed binding.
\t\tref array<string> keyStackFallback = {};
\t\tref array<BaseContainer> filterStackFallback = {};
\t\tif (m_InputManager.GetActionKeybinding(actionName, keyStackFallback, filterStackFallback, EInputDeviceType.JOYSTICK, string.Empty, -1))
\t\t{
\t\t\tforeach (string binding : keyStackFallback)
\t\t\t{
\t\t\t\tif (!joystickBindings.IsEmpty())
\t\t\t\t\tjoystickBindings += " / ";

\t\t\t\tjoystickBindings += binding;
\t\t\t}
\t\t}

\t\tif (!joystickBindings.IsEmpty())
\t\t\treturn joystickBindings;

\t\t// Final compatibility fallback to InputBinding.
\t\tif (m_InputBinding)
\t\t{
\t\t\tref array<string> bindings = {};
\t\t\tif (m_InputBinding.GetBindings(actionName, bindings, EInputDeviceType.JOYSTICK, string.Empty, false))
\t\t\t{
\t\t\t\tforeach (string binding : bindings)
\t\t\t\t{
\t\t\t\t\tif (!joystickBindings.IsEmpty())
\t\t\t\t\t\tjoystickBindings += " / ";

\t\t\t\t\tjoystickBindings += binding;
\t\t\t\t}
\t\t\t}
\t\t}

\t\tif (joystickBindings.IsEmpty())
\t\t\treturn "Action fired, but active joystick binding lookup returned nothing";

\t\treturn joystickBindings;
\t}
'''
path.write_text(src[:start] + new + src[end:], encoding='utf-8')
