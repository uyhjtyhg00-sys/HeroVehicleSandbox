
@echo off
setlocal
powershell -ExecutionPolicy Bypass -File "%~dp0ApplyInputSensitivitySettingsFix_v2_AndBuild.ps1" -ProjectRoot "D:\UnrealProject\HeroVehicleSandbox" -UERoot "D:\UE_5.7"
pause
