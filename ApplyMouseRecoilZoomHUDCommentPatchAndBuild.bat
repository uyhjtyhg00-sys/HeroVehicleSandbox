@echo off
setlocal EnableExtensions

set "PROJECT_ROOT=D:\UnrealProject\HeroVehicleSandbox"
set "UE_ROOT=D:\UE_5.7"
set "PROJECT=%PROJECT_ROOT%\HeroVehicleSandbox.uproject"
set "LOG=%PROJECT_ROOT%\Build_HeroVehicleSandbox_MouseZoomFix.log"

echo.
echo ================================================================
echo Applying HeroVehicleSandbox mouse / recoil / zoom / HUD cleanup patch
echo ================================================================
echo.

echo [1/4] Closing Unreal Editor and Live Coding...
taskkill /IM LiveCodingConsole.exe /F >nul 2>nul
taskkill /IM UnrealEditor.exe /F >nul 2>nul
taskkill /IM UnrealEditor-Cmd.exe /F >nul 2>nul
timeout /t 2 /nobreak >nul

echo [2/4] Cleaning generated folders...
if exist "%PROJECT_ROOT%\Intermediate" rmdir /s /q "%PROJECT_ROOT%\Intermediate"
if exist "%PROJECT_ROOT%\Binaries" rmdir /s /q "%PROJECT_ROOT%\Binaries"

echo [3/4] Building HeroVehicleSandboxEditor...
"%UE_ROOT%\Engine\Build\BatchFiles\Build.bat" HeroVehicleSandboxEditor Win64 Development -Project="%PROJECT%" -WaitMutex -architecture=x64 > "%LOG%" 2>&1

type "%LOG%"

findstr /C:"Result: Succeeded" "%LOG%" >nul 2>nul
if errorlevel 1 (
    echo.
    echo [FAILED] Build failed. Check log:
    echo "%LOG%"
    pause
    exit /b 1
)

echo.
echo [4/4] Build succeeded.
echo.
echo Test checklist:
echo - Sandbox WASD/mouse input works.
echo - Mouse Y is normal when invert is OFF.
echo - Mouse Y reverses only when invert is ON.
echo - Recoil always kicks upward regardless of invert setting.
echo - RMB zoom changes FOV.
echo - Top SANDBOX/NO TEAM/READY banner is gone.
echo.
pause
exit /b 0
