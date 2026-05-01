#pragma once

#include "CoreMinimal.h"
#include "HeroTypes.generated.h"

UENUM(BlueprintType)
enum class EHeroPlayerMode : uint8
{
    Human UMETA(DisplayName="Human"),
    Vehicle UMETA(DisplayName="Vehicle"),
    Dead UMETA(DisplayName="Dead"),
    Spectator UMETA(DisplayName="Spectator")
};

UENUM(BlueprintType)
enum class EHeroMatchState : uint8
{
    Boot UMETA(DisplayName="Boot"),
    Sandbox UMETA(DisplayName="Sandbox"),
    CombatTest UMETA(DisplayName="Combat Test"),
    Paused UMETA(DisplayName="Paused")
};

UENUM(BlueprintType)
enum class EHeroWeaponFireMode : uint8
{
    HitScan UMETA(DisplayName="HitScan"),
    Projectile UMETA(DisplayName="Projectile")
};

UENUM(BlueprintType)
enum class EHeroGameModeType : uint8
{
    Sandbox UMETA(DisplayName="Sandbox"),
    Escort UMETA(DisplayName="Escort"),
    Control UMETA(DisplayName="Control"),
    TeamDeathmatch UMETA(DisplayName="Team Deathmatch")
};

UENUM(BlueprintType)
enum class EHeroTeam : uint8
{
    None UMETA(DisplayName="None"),
    TeamA UMETA(DisplayName="Team A"),
    TeamB UMETA(DisplayName="Team B")
};

UENUM(BlueprintType)
enum class EHeroObjectiveState : uint8
{
    Inactive UMETA(DisplayName="Inactive"),
    Active UMETA(DisplayName="Active"),
    Contested UMETA(DisplayName="Contested"),
    Captured UMETA(DisplayName="Captured"),
    Completed UMETA(DisplayName="Completed")
};

UENUM(BlueprintType)
enum class EHeroBotRole : uint8
{
    Damage UMETA(DisplayName="Damage"),
    Support UMETA(DisplayName="Support"),
    Tank UMETA(DisplayName="Tank")
};

USTRUCT(BlueprintType)
struct FHeroWeaponConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Weapon")
    FText WeaponName = FText::FromString(TEXT("Pulse Rifle"));

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Weapon")
    EHeroWeaponFireMode FireMode = EHeroWeaponFireMode::HitScan;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Weapon")
    bool bIsHitScan = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Weapon")
    float Damage = 34.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Weapon")
    float FireRateRoundsPerMinute = 620.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Weapon")
    float ReloadSeconds = 1.65f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Weapon")
    float SpreadDegrees = 0.45f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Weapon")
    float AimSpreadMultiplier = 0.35f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Weapon")
    float RangeMeters = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Weapon")
    float ProjectileSpeedMetersPerSecond = 55.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Weapon")
    float ProjectileGravityScale = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Weapon")
    float ExplosionRadiusMeters = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Weapon")
    int32 MagazineSize = 30;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Weapon")
    int32 ReserveAmmo = 120;
};

USTRUCT(BlueprintType)
struct FHeroAbilitySlot
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Ability")
    FName AbilityId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Ability")
    float CooldownSeconds = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Ability")
    bool bUnlocked = false;
};

USTRUCT(BlueprintType)
struct FHeroAimSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Settings")
    float MouseSensitivity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Settings")
    float ScopedSensitivityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Settings")
    float VehicleLookSensitivityMultiplier = 0.85f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Settings")
    float FirstPersonFovDegrees = 103.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Settings")
    bool bInvertMouseY = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Settings")
    float ReferenceMouseDpi = 1600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Settings")
    float TargetCmPer360 = 86.6f;

    float GetBaseDegreesPerCount() const
    {
        const float CountsPerCm = ReferenceMouseDpi / 2.54f;
        const float CountsPer360 = TargetCmPer360 * CountsPerCm;
        return CountsPer360 > KINDA_SMALL_NUMBER ? 360.0f / CountsPer360 : 0.0066f;
    }

    float GetFinalDegreesPerCount(const bool bScoped, const bool bVehicle) const
    {
        float Result = GetBaseDegreesPerCount() * MouseSensitivity;
        Result *= bScoped ? ScopedSensitivityMultiplier : 1.0f;
        Result *= bVehicle ? VehicleLookSensitivityMultiplier : 1.0f;
        return Result;
    }

    void Clamp()
    {
        MouseSensitivity = FMath::Clamp(MouseSensitivity, 0.1f, 20.0f);
        ScopedSensitivityMultiplier = FMath::Clamp(ScopedSensitivityMultiplier, 0.1f, 2.0f);
        VehicleLookSensitivityMultiplier = FMath::Clamp(VehicleLookSensitivityMultiplier, 0.1f, 2.0f);
        FirstPersonFovDegrees = FMath::Clamp(FirstPersonFovDegrees, 80.0f, 120.0f);
    }
};
