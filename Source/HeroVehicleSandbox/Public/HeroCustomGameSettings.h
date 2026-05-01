#pragma once

#include "CoreMinimal.h"
#include "HeroTypes.h"
#include "HeroCustomGameSettings.generated.h"

USTRUCT(BlueprintType)
struct FHeroCustomGameSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Custom Game")
    EHeroGameModeType GameModeType = EHeroGameModeType::Sandbox;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Custom Game")
    int32 TeamSize = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Custom Game")
    float MatchTimeLimitSeconds = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Custom Game")
    float RespawnDelaySeconds = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Custom Game")
    bool bAllowVehicleMode = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Custom Game")
    bool bAllowHeroAbilities = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Custom Game")
    bool bAllowFriendlyFire = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Custom Game")
    float HumanDamageMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Custom Game")
    float VehicleDamageMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Custom Game")
    float HealingMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Custom Game")
    float MovementSpeedMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Custom Game")
    bool bEnableBots = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Custom Game")
    int32 BotCountTeamA = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Custom Game")
    int32 BotCountTeamB = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Custom Game")
    float BotAccuracy = 0.55f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Custom Game")
    float BotReactionTimeSeconds = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Custom Game")
    float BotAimInterpSpeed = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Custom Game")
    float BotPreferredCombatDistanceMeters = 18.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Custom Game")
    float BotStrafeIntervalSeconds = 0.45f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Custom Game")
    float BotStrafeStrength = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Custom Game")
    float HealingPerShot = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Custom Game")
    float HealRangeMeters = 35.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Custom Game")
    float AllyLowHealthThreshold = 0.65f;
};
