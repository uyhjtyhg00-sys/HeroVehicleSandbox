@echo off
setlocal

set "PROJECT_ROOT=D:\UnrealProject\HeroVehicleSandbox"
set "UE_ROOT=D:\UE_5.7"
set "PROJECT=%PROJECT_ROOT%\HeroVehicleSandbox.uproject"
set "LOG=%PROJECT_ROOT%\Build_HeroVehicleSandboxEditor.log"

echo Building HeroVehicleSandboxEditor...
echo.

if exist "%PROJECT_ROOT%\Intermediate" rmdir /s /q "%PROJECT_ROOT%\Intermediate"
if exist "%PROJECT_ROOT%\Binaries" rmdir /s /q "%PROJECT_ROOT%\Binaries"

"%UE_ROOT%\Engine\Build\BatchFiles\Build.bat" HeroVehicleSandboxEditor Win64 Development -Project="%PROJECT%" -architecture=x64 > "%LOG%" 2>&1

type "%LOG%"

echo.
echo Done.
pause
