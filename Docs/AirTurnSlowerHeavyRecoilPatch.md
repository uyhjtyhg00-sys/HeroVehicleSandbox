# Air Turn Slower / Heavy Recoil Patch

- Jump height is intentionally unchanged from the previous accepted value.
- Mid-air direction changing is slowed further:
  - AirControl = 0.18
  - AirControlBoostMultiplier = 0.20
  - AirControlBoostVelocityThreshold = 65
- Horizontal speed is still preserved:
  - FallingLateralFriction = 0
  - BrakingDecelerationFalling = 0
- Recoil is intentionally strong:
  - PitchKick = 0.280
  - YawKick = +/-0.030
- Recoil direction remains owned by `AHeroCharacter::ApplyViewKick`.
