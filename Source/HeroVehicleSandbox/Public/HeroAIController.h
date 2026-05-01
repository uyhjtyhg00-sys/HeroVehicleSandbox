#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "HeroTypes.h"
#include "HeroAIController.generated.h"

class AHeroCharacter;

UCLASS()
class HEROVEHICLESANDBOX_API AHeroAIController : public AAIController
{
    GENERATED_BODY()

public:
    AHeroAIController();
    virtual void Tick(float DeltaSeconds) override;
    virtual void OnPossess(APawn* InPawn) override;

private:
    bool IsDead() const;
    void UpdateKnownEnemies();
    void UpdateKnownAllies();
    bool TryHealAlly();
    bool TryAttackEnemy(float DeltaSeconds);
    bool TryMoveToObjective();
    void PerformIdleStrafe(float DeltaSeconds);
    void UpdateStrafe(float DeltaSeconds);
    AHeroCharacter* GetHeroPawn() const;
    EHeroTeam GetMyTeam() const;

private:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|AI", meta=(AllowPrivateAccess="true"))
    float BotAccuracy = 0.55f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|AI", meta=(AllowPrivateAccess="true"))
    float BotReactionTimeSeconds = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|AI", meta=(AllowPrivateAccess="true"))
    float BotAimInterpSpeed = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|AI", meta=(AllowPrivateAccess="true"))
    float BotPreferredCombatDistanceMeters = 18.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|AI", meta=(AllowPrivateAccess="true"))
    float BotStrafeIntervalSeconds = 0.45f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|AI", meta=(AllowPrivateAccess="true"))
    float BotStrafeStrength = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|AI", meta=(AllowPrivateAccess="true"))
    float HealingPerShot = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|AI", meta=(AllowPrivateAccess="true"))
    float HealRangeMeters = 35.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|AI", meta=(AllowPrivateAccess="true"))
    float AllyLowHealthThreshold = 0.65f;

    UPROPERTY(Transient)
    TObjectPtr<AHeroCharacter> CurrentEnemy;

    UPROPERTY(Transient)
    TObjectPtr<AHeroCharacter> CurrentAllyToHeal;

    float ReactionTimer = 0.0f;
    float CurrentStrafeDirection = 1.0f;
    float StrafeTimer = 0.0f;
    float LastDamagedTimeSeconds = -999.0f;
};
