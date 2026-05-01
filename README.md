# HeroVehicleSandbox

HeroVehicleSandbox is a UE 5.7 C++ foundation project for a Hyper-Tactical Hero Vehicle Sandbox.

It is derived from the final RaceCore vehicle sandbox baseline, but it is a new repository and a new project. The original `D:\UnrealProject\RaceCore` project is not modified by this work.

## Design Goal

The game direction is a team-based hero vehicle shooter:

- Human Mode plays like a fast, precise hero FPS with Overwatch-style immediate movement.
- Vehicle Mode is not just transportation. It is a stronger combat form with mass, collision, damage, and vehicle weapon handling.
- RaceCore's `RaceVehicleCore` is preserved under `LegacyVehicle` as the future Vehicle Mode physics asset.
- This foundation intentionally avoids bringing RaceCore lap, checkpoint, countdown, race result, and lobby UI into the core FPS project.

## Current Implementation

- `AHeroGameModeBase` as the default game mode.
- `AHeroCharacter` as the first-person hero pawn.
- Human/Vehicle mode state with `UHeroVehicleModeComponent`.
- FOV 103 first-person camera.
- 1600 DPI / sensitivity 1.0 / 86.6 cm/360 mouse calculation structure.
- Configurable mouse sensitivity and FOV through settings save data and Settings UI API.
- Snappy FPS CharacterMovement values for immediate acceleration/deceleration.
- `UHeroWeaponComponent` with separate Human and Vehicle weapon configs.
- HitScan and Projectile weapon paths.
- Ammo, reserve ammo, reload, spread, fire rate, projectile speed, gravity, splash radius fields.
- `UHeroHealthComponent` for damage/healing/death.
- `AHeroProjectile` with direct and radius damage support.
- `UHeroHUDWidget`, `UHeroMainMenuWidget`, and `UHeroSettingsWidget` prototype UI classes.
- Objective framework for Sandbox, Escort, Control, and Team Deathmatch rules.
- `FHeroCustomGameSettings` for Overwatch-style custom game settings.
- Team system through `UHeroTeamComponent`.
- Objective interface, payload actor, and control point actor.
- Basic AI controller with objective movement, enemy attack, support healing structure, and AD strafing.
- RaceCore `RaceVehicleCore` copied into `Source/HeroVehicleSandbox/Public/LegacyVehicle` and `Private/LegacyVehicle`.

## Controls and Sensitivity

Default mouse configuration:

- DPI: 1600
- In-game sensitivity: 1.0
- Approximate cm/360: 86.6
- FOV: 103

Sensitivity is configurable in-game.

Calculation:

```cpp
CountsPerCm = ReferenceMouseDpi / 2.54f;
CountsPer360 = TargetCmPer360 * CountsPerCm;
BaseDegreesPerCount = 360.0f / CountsPer360;
FinalDegreesPerCount = BaseDegreesPerCount * InGameSensitivity;
```

Default values:

```text
ReferenceMouseDpi = 1600.0
TargetCmPer360 = 86.6
MouseSensitivity = 1.0
ScopedSensitivityMultiplier = 1.0
VehicleLookSensitivityMultiplier = 0.85
FirstPersonFovDegrees = 103.0
InvertMouseY = false
```

## Input

| Input | Human Mode | Vehicle Mode foundation |
|---|---|---|
| W/S | Move forward/back | Reserved for future vehicle movement handoff |
| A/D | Strafe | Reserved for future vehicle movement handoff |
| Mouse | Look | Vehicle look sensitivity multiplier |
| Left Mouse | Human primary fire | Vehicle primary fire |
| Right Mouse | Aim | Vehicle aim / alternate fire placeholder |
| R | Reload | Vehicle reload |
| Q | Toggle Vehicle Mode | Exit Vehicle Mode |
| E | Interact placeholder | Exit/interact placeholder |
| Space | Jump | Reserved |

## UI Direction

The UI is designed around fast hero-shooter readability:

- Human HUD
- Vehicle HUD
- Crosshair
- Health
- Ammo
- Reload state
- Weapon name
- Mode indicator
- Settings menu
- Objective state and progress

Human HUD is intended to show crosshair, health, ammo, reload state, weapon name, HUMAN mode, Q vehicle prompt, damage direction placeholder, and ability slot placeholders.

Vehicle HUD is intended to show vehicle crosshair, vehicle health, speed, vehicle weapon ammo/heat, reload state, VEHICLE mode, exit vehicle prompt, damage/impact warning, and locked vehicle ability placeholders.

Main Menu target items:

- Play Sandbox
- Combat Test
- Settings
- Credits
- Quit

Settings target categories:

- Controls
- Video
- Audio
- Gameplay

Mouse Sensitivity and FOV are implemented as settings-backed values. Other settings have placeholder UI/API structure for expansion.

## Game Mode Framework

Supported mode types at the framework level:

- Sandbox
- Escort
- Control
- TeamDeathmatch

`FHeroCustomGameSettings` controls team size, match time, respawn delay, vehicle mode allowance, hero ability allowance, friendly fire, damage multipliers, healing multiplier, movement speed multiplier, bot counts, bot accuracy, and bot reaction time.

Escort foundation:

- `AHeroPayloadActor`
- Payload progress
- Attacker/defender teams
- Contested state
- Optional rollback
- Future spline path support

Control foundation:

- `AHeroControlPointActor`
- Capture radius
- Team A/B capture progress
- Owning team
- Contested state

AI foundation:

- `AHeroAIController`
- `AHeroBotCharacter`
- Objective movement
- Enemy acquisition and firing
- Support healing structure
- Combat and idle AD strafing

## Vehicle Mode Combat

Vehicle Mode has a separate weapon config from Human Mode.

Current foundation:

- `HumanPrimaryWeapon`
- `VehiclePrimaryWeapon`
- shared `UHeroWeaponComponent` processing
- Vehicle fire through Left Mouse when mode is VEHICLE
- Vehicle reload through R when mode is VEHICLE
- Vehicle ability slots reserved but not implemented

Vehicle skills such as Boost, Shield, Missile Barrage, Ram, Smoke, or EMP are intentionally not implemented in this first foundation. The API leaves space for them without forcing premature logic.

## Legacy RaceCore Vehicle Physics

Copied files:

```text
Source/HeroVehicleSandbox/Public/LegacyVehicle/SharedTypes.h
Source/HeroVehicleSandbox/Public/LegacyVehicle/RaceVehicleCore.h
Source/HeroVehicleSandbox/Private/LegacyVehicle/RaceVehicleCore.cpp
```

The `RaceVehicleCore` name is intentionally preserved. It will be reused later as the core physics asset for Vehicle Mode instead of being renamed or simplified during the FPS foundation pass.

## Build

```powershell
D:\UE_5.7\Engine\Build\BatchFiles\Build.bat HeroVehicleSandboxEditor Win64 Development -Project="D:\UnrealProject\HeroVehicleSandbox\HeroVehicleSandbox.uproject" -architecture=x64
```

Or:

```bat
ApplyHeroVehicleSandboxAndBuild.bat
```

## Git

Initial commit message:

```text
Initialize HeroVehicleSandbox from RaceCore vehicle baseline
```

Initial tag:

```text
MVP-hero-fps-foundation
```

## Next Work

- Add a real first playable map.
- Add target dummy actors and team spawn points.
- Connect HUD updates to polished UMG art.
- Implement real Vehicle Pawn possession using `RaceVehicleCore`.
- Add basic projectile visual meshes and muzzle transforms.
- Expand custom game settings UI.
- Add Escort payload spline authoring tools.
- Add Control point round scoring.
- Improve bot pathing, roles, healing, and combat selection.
- Add hero abilities after the weapon and Vehicle Mode foundation is stable.
