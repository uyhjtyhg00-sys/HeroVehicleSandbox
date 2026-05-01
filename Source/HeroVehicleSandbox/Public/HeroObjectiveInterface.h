#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HeroTypes.h"
#include "HeroObjectiveInterface.generated.h"

UINTERFACE(BlueprintType)
class HEROVEHICLESANDBOX_API UHeroObjectiveInterface : public UInterface
{
    GENERATED_BODY()
};

class HEROVEHICLESANDBOX_API IHeroObjectiveInterface
{
    GENERATED_BODY()

public:
    virtual FVector GetObjectiveLocationForTeam(EHeroTeam Team) const = 0;
    virtual EHeroObjectiveState GetObjectiveState() const = 0;
    virtual EHeroTeam GetOwningTeam() const = 0;
};
