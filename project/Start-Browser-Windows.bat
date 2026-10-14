@echo off
setlocal
cd /d "%~dp0"
where py >nul 2>&1
if not errorlevel 1 (
    py -3 Start-Browser.py
    goto done
)
where python >nul 2>&1
if not errorlevel 1 (
    python Start-Browser.py
    goto done
)
echo Python 3 is required. Install Python 3, then run this file again.
:done
pause
