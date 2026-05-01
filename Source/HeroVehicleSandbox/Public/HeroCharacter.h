#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "HeroTypes.h"
#include "HeroCharacter.generated.h"

class UCameraComponent;
class UHeroHealthComponent;
class UHeroWeaponComponent;
class UHeroVehicleModeComponent;
class UHeroTeamComponent;

UCLASS()
class HEROVEHICLESANDBOX_API AHeroCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AHeroCharacter();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

    UCameraComponent* GetFirstPersonCamera() const;
    UHeroWeaponComponent* GetWeaponComponent() const;
    UHeroHealthComponent* GetHealthComponent() const;
    UHeroVehicleModeComponent* GetVehicleModeComponent() const;
    UHeroTeamComponent* GetTeamComponent() const;

    UFUNCTION(BlueprintCallable, Category="Hero|Settings")
    void ApplyAimSettings(const FHeroAimSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category="Hero|Settings")
    const FHeroAimSettings& GetAimSettings() const;

protected:
    void MoveForward(float Value);
    void MoveRight(float Value);
    void Turn(float Value);
    void LookUp(float Value);
    void StartFire();
    void StopFire();
    void StartAim();
    void StopAim();
    void Reload();
    void ToggleVehicleMode();
    void Interact();

    UFUNCTION()
    void HandleDeath();

private:
    void ConfigureCharacterMovement();
    void LoadAimSettings();

    UFUNCTION()
    void ApplyModeToComponents(EHeroPlayerMode NewMode);

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Hero|Camera", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UCameraComponent> FirstPersonCamera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Hero|Components", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UHeroWeaponComponent> WeaponComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Hero|Components", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UHeroHealthComponent> HealthComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Hero|Components", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UHeroVehicleModeComponent> VehicleModeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Hero|Components", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UHeroTeamComponent> TeamComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Movement", meta=(AllowPrivateAccess="true"))
    float HumanWalkSpeed = 550.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Movement", meta=(AllowPrivateAccess="true"))
    float VehicleModeWalkSpeed = 950.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Settings", meta=(AllowPrivateAccess="true"))
    FHeroAimSettings AimSettings;
};
