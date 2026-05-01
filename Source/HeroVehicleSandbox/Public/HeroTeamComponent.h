#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HeroTypes.h"
#include "HeroTeamComponent.generated.h"

UCLASS(ClassGroup=(Hero), meta=(BlueprintSpawnableComponent))
class HEROVEHICLESANDBOX_API UHeroTeamComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UHeroTeamComponent();

    UFUNCTION(BlueprintCallable, Category="Hero|Team")
    void SetTeam(EHeroTeam NewTeam);

    UFUNCTION(BlueprintCallable, Category="Hero|Team")
    EHeroTeam GetTeam() const;

    UFUNCTION(BlueprintCallable, Category="Hero|Team")
    bool IsEnemyTeam(EHeroTeam OtherTeam) const;

private:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Team", meta=(AllowPrivateAccess="true"))
    EHeroTeam Team = EHeroTeam::None;
};
