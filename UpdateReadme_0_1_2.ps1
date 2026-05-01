
param(
    [string]$ProjectRoot = "D:\UnrealProject\HeroVehicleSandbox"
)

$ErrorActionPreference = "Stop"

$ReadmePath = Join-Path $ProjectRoot "README.md"

if (!(Test-Path $ReadmePath)) {
    throw "README.md not found: $ReadmePath"
}

$Backup = "$ReadmePath.release_012.bak"
if (!(Test-Path $Backup)) {
    Copy-Item $ReadmePath $Backup
}

$Text = Get-Content $ReadmePath -Raw -Encoding UTF8

$VersionSection = @'
## Version

Current release: 0.1.2

0.1.2 is the combat UX and input-fix release. It keeps the RaceCore vehicle baseline intact while stabilizing the first-person sandbox controls, Overwatch-style mouse sensitivity, mouse Y behavior, weapon recoil direction, aim zoom, HUD readability, and settings input workflow.
'@

$ReleaseHistorySection = @'
## Release History

### v0.1.2 - Combat UX / Mouse / Zoom / HUD Fix

- Fixed Sandbox mode input by restoring the legacy input classes used by `BindAxis` / `BindAction`.
- Fixed Overwatch-style mouse sensitivity using yaw `0.0066`.
- Fixed Mouse Y direction behavior.
- Separated mouse inversion from weapon recoil.
- Fixed recoil direction so weapon recoil always kicks upward regardless of invert setting.
- Added working aim/zoom FOV transition.
- Hid the distracting top debug status banner.
- Improved settings UI with direct numeric input fields.
- Added function-purpose comments to patched gameplay/UI code.
- Added `Docs/FunctionCommentPolicy.md` for future patch consistency.

### v0.1.1 - First Person Startup / HUD / Menu Fix

- Fixed first-person startup possession flow.
- Improved HUD crosshair visibility.
- Fixed menu button availability.
- Improved game input mode transition from menu to play.

### v0.1.0 - Initial HeroVehicleSandbox Foundation

- Created UE5.7 C++ HeroVehicleSandbox project.
- Added first-person hero character foundation.
- Added weapon component, projectile, HUD, bot, and objective groundwork.
- Imported legacy vehicle physics baseline from RaceCore.
'@

if ($Text -match '(?s)## Version\s+Current release:.*?(?=\r?\n## |\z)') {
    $Text = [regex]::Replace($Text, '(?s)## Version\s+Current release:.*?(?=\r?\n## |\z)', $VersionSection.TrimEnd())
}
elseif ($Text -match '(?s)# .+?\r?\n') {
    $Text = [regex]::Replace($Text, '(?s)(# .+?\r?\n)', "`$1`r`n$VersionSection`r`n", 1)
}
else {
    $Text = "$VersionSection`r`n`r`n$Text"
}

if ($Text -match '(?s)## Release History\s+.*?(?=\r?\n## |\z)') {
    $Text = [regex]::Replace($Text, '(?s)## Release History\s+.*?(?=\r?\n## |\z)', $ReleaseHistorySection.TrimEnd())
}
else {
    $Text = $Text.TrimEnd() + "`r`n`r`n" + $ReleaseHistorySection + "`r`n"
}

$Utf8NoBom = New-Object System.Text.UTF8Encoding($false)
[System.IO.File]::WriteAllText($ReadmePath, $Text, $Utf8NoBom)

Write-Host "README.md updated for release 0.1.2."
Write-Host "Backup created at:"
Write-Host "  $Backup"
Write-Host ""
Write-Host "Next commands:"
Write-Host "  git -C `"$ProjectRoot`" add README.md"
Write-Host "  git -C `"$ProjectRoot`" commit -m `"Update release history for 0.1.2`""
