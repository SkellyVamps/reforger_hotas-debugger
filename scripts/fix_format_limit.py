from pathlib import Path

path = Path('Scripts/Game/HOTASDebugger/HOTASDebugger.c')
src = path.read_text(encoding='utf-8')
old = '''\t\tPrint(string.Format("[HOTAS Debugger] HUD settings: position=%1 scale=%2 fade=%3/%4 background=%5 opacity=%6 axes R/P/T/Y=%7/%8/%9/%10", m_sHudPosition, m_fHudScale, m_iFadeDelayMs, m_iFadeDurationMs, m_bBackgroundEnabled, m_fBackgroundOpacity, m_iRollAxis, m_iPitchAxis, m_iThrottleAxis, m_iYawAxis), LogLevel.NORMAL);'''
new = '''\t\tPrint(string.Format("[HOTAS Debugger] HUD settings: position=%1 scale=%2 fade=%3/%4 background=%5 opacity=%6", m_sHudPosition, m_fHudScale, m_iFadeDelayMs, m_iFadeDurationMs, m_bBackgroundEnabled, m_fBackgroundOpacity), LogLevel.NORMAL);\n\t\tPrint(string.Format("[HOTAS Debugger] Axis mapping: roll=%1 pitch=%2 throttle=%3 yaw=%4", m_iRollAxis, m_iPitchAxis, m_iThrottleAxis, m_iYawAxis), LogLevel.NORMAL);'''
if old not in src:
    raise SystemExit('target Format call not found')
path.write_text(src.replace(old, new, 1), encoding='utf-8')
