# Air Turn / Recoil / Jump Tuning

- Recoil strength increased.
- Recoil direction remains owned by `AHeroCharacter::ApplyViewKick`, so mouse inversion does not affect it.
- Jump height reduced about 15% from the previous 760 tuning to 650.
- Air speed is preserved: falling friction and falling braking remain zero.
- Mid-air direction change is slowed by lowering AirControl to 0.42.
