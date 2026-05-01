@echo off
setlocal

set "PROJECT_ROOT=D:\UnrealProject\HeroVehicleSandbox"
set "UE_ROOT=D:\UE_5.7"
set "PROJECT=%PROJECT_ROOT%\HeroVehicleSandbox.uproject"
set "LOG=%PROJECT_ROOT%\Build_HeroVehicleSandboxEditor_AirTurnSlowerHeavyRecoil.log"

echo Building HeroVehicleSandboxEditor...
"%UE_ROOT%\Engine\Build\BatchFiles\Build.bat" HeroVehicleSandboxEditor Win64 Development -Project="%PROJECT%" -WaitMutex -architecture=x64 > "%LOG%" 2>&1

type "%LOG%"
pause
