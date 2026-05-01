
@echo off
setlocal
powershell -ExecutionPolicy Bypass -File "%~dp0ApplyBuildFix_SettingsHUDIncludeAndBuild.ps1" -ProjectRoot "D:\UnrealProject\HeroVehicleSandbox" -UERoot "D:\UE_5.7"
pause
