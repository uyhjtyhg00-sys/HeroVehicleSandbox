#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HeroCustomGameSettings.h"
#include "HeroGameModeBase.generated.h"

class AHeroBotCharacter;
class AHeroControlPointActor;
class AHeroPayloadActor;

UCLASS()
class HEROVEHICLESANDBOX_API AHeroGameModeBase : public AGameModeBase
{
    GENERATED_BODY()

public:
    AHeroGameModeBase();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    UFUNCTION(BlueprintCallable, Category="Hero|Custom Game")
    void ApplyCustomGameSettings(const FHeroCustomGameSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category="Hero|Custom Game")
    const FHeroCustomGameSettings& GetCustomGameSettings() const;

    UFUNCTION(BlueprintCallable, Category="Hero|Custom Game")
    void StartSandboxMode();

    UFUNCTION(BlueprintCallable, Category="Hero|Custom Game")
    void StartCombatTestMode();

    UFUNCTION(BlueprintCallable, Category="Hero|Custom Game")
    void StartCustomGameMode(EHeroGameModeType ModeType, int32 EnemyBotCount);

    UFUNCTION(BlueprintCallable, Category="Hero|AI")
    void SpawnTestBots(int32 TeamBCount, int32 TeamACount = 0);

    UFUNCTION(BlueprintCallable, Category="Hero|AI")
    void ClearSpawnedBots();

    UFUNCTION(BlueprintCallable, Category="Hero|AI")
    int32 GetAliveBotCount() const;

    UFUNCTION(BlueprintCallable, Category="Hero|AI")
    int32 GetTotalSpawnedBotCount() const;

    UFUNCTION(BlueprintCallable, Category="Hero|Objective")
    AActor* GetPrimaryObjectiveForTeam(EHeroTeam Team) const;

    UFUNCTION(BlueprintCallable, Category="Hero|Objective")
    EHeroObjectiveState GetPrimaryObjectiveState() const;

    UFUNCTION(BlueprintCallable, Category="Hero|Objective")
    float GetPrimaryObjectiveProgress01() const;

    UFUNCTION(BlueprintCallable, Category="Hero|Match")
    EHeroMatchState GetHeroMatchState() const;

private:
    void CachePrimaryObjectives();
    void SpawnBot(EHeroTeam Team, EHeroBotRole BotRole, const FVector& Location, const FRotator& Rotation);
    FVector GetBotSpawnLocation(int32 Index, EHeroTeam Team) const;

private:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Custom Game", meta=(AllowPrivateAccess="true"))
    FHeroCustomGameSettings CustomGameSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|AI", meta=(AllowPrivateAccess="true"))
    TSubclassOf<AHeroBotCharacter> BotCharacterClass;

    UPROPERTY(BlueprintReadOnly, Category="Hero|Match", meta=(AllowPrivateAccess="true"))
    EHeroMatchState HeroMatchState = EHeroMatchState::Boot;

    UPROPERTY(Transient)
    TObjectPtr<AHeroControlPointActor> PrimaryControlPoint;

    UPROPERTY(Transient)
    TObjectPtr<AHeroPayloadActor> PrimaryPayload;

    UPROPERTY(Transient)
    TArray<TWeakObjectPtr<AHeroBotCharacter>> SpawnedBots;
};

