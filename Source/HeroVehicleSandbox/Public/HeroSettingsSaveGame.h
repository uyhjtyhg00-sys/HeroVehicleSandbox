#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "HeroTypes.h"
#include "HeroSettingsSaveGame.generated.h"

UCLASS()
class HEROVEHICLESANDBOX_API UHeroSettingsSaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Settings")
    FHeroAimSettings AimSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Settings")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Settings")
    bool bShowDamageNumbers = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Settings")
    bool bShowCrosshair = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Settings")
    bool bShowVehicleHints = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Settings")
    bool bAutoReload = true;
};
