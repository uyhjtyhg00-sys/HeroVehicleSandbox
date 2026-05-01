#pragma once

#include "CoreMinimal.h"
#include "HeroCharacter.h"
#include "HeroBotCharacter.generated.h"

UCLASS()
class HEROVEHICLESANDBOX_API AHeroBotCharacter : public AHeroCharacter
{
    GENERATED_BODY()

public:
    AHeroBotCharacter();

    UFUNCTION(BlueprintCallable, Category="Hero|AI")
    EHeroBotRole GetBotRole() const;

private:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|AI", meta=(AllowPrivateAccess="true"))
    EHeroBotRole BotRole = EHeroBotRole::Damage;
};
