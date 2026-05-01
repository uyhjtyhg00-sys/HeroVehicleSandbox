# Recoil Direction / Jump Height Fix

- Recoil direction fixed by changing AHeroCharacter::ApplyViewKick to add positive pitch in this project's camera convention.
- Mouse invert still affects only manual LookUp input, not weapon recoil.
- Jump height raised with JumpZVelocity = 760.
- GravityScale remains 2.05 so the jump is higher but not floaty.
- Air control remains full, with no falling lateral braking.
