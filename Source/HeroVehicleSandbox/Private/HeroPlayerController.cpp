#include "HeroPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "HeroCharacter.h"
#include "HeroGameModeBase.h"
#include "HeroHUDWidget.h"
#include "HeroHealthComponent.h"
#include "HeroMainMenuWidget.h"
#include "HeroSettingsWidget.h"
#include "HeroTeamComponent.h"
#include "HeroVehicleModeComponent.h"
#include "HeroWeaponComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"

AHeroPlayerController::AHeroPlayerController()
{
    bShowMouseCursor = true;
    HeroCharacterClass = AHeroCharacter::StaticClass();
}

void AHeroPlayerController::BeginPlay()
{
    Super::BeginPlay();
    EnsureHeroPawnPossessed();
    ShowMainMenu();
}

void AHeroPlayerController::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    UpdateHUDFromPawn();
}

void AHeroPlayerController::ShowMainMenu()
{
    ClearMenu();
    UClass* WidgetClass = MainMenuWidgetClass ? MainMenuWidgetClass.Get() : UHeroMainMenuWidget::StaticClass();
    CurrentMenu = CreateWidget<UHeroMainMenuWidget>(this, WidgetClass);
    if (CurrentMenu)
    {
        CurrentMenu->AddToViewport(20);
    }
    SetMenuInputMode();
}

void AHeroPlayerController::ShowHUD()
{
    ClearMenu();
    if (!HeroHUD)
    {
        UClass* WidgetClass = HUDWidgetClass ? HUDWidgetClass.Get() : UHeroHUDWidget::StaticClass();
        HeroHUD = CreateWidget<UHeroHUDWidget>(this, WidgetClass);
    }
    if (HeroHUD && !HeroHUD->IsInViewport())
    {
        HeroHUD->AddToViewport(5);
    }
    SetGameInputMode();
}

void AHeroPlayerController::ShowSettingsMenu()
{
    ClearMenu();
    UClass* WidgetClass = SettingsWidgetClass ? SettingsWidgetClass.Get() : UHeroSettingsWidget::StaticClass();
    CurrentMenu = CreateWidget<UHeroSettingsWidget>(this, WidgetClass);
    if (CurrentMenu)
    {
        CurrentMenu->AddToViewport(30);
    }
    SetMenuInputMode();
}

void AHeroPlayerController::StartSandboxFromMenu()
{
    EnsureHeroPawnPossessed();
    ShowHUD();
}

UHeroHUDWidget* AHeroPlayerController::GetHeroHUD() const
{
    return HeroHUD;
}

void AHeroPlayerController::ClearMenu()
{
    if (CurrentMenu)
    {
        CurrentMenu->RemoveFromParent();
        CurrentMenu = nullptr;
    }
}

void AHeroPlayerController::EnsureHeroPawnPossessed()
{
    if (AHeroCharacter* CurrentHero = Cast<AHeroCharacter>(GetPawn()))
    {
        SetViewTarget(CurrentHero);
        return;
    }

    UWorld* World = GetWorld();
    UClass* PawnClass = HeroCharacterClass ? HeroCharacterClass.Get() : AHeroCharacter::StaticClass();
    if (!World || !PawnClass)
    {
        return;
    }

    APawn* PreviousPawn = GetPawn();
    FVector HeroSpawnLocation = FVector(0.0f, 0.0f, 120.0f);
    FRotator SpawnRotation = FRotator::ZeroRotator;

    if (PreviousPawn)
    {
        HeroSpawnLocation = PreviousPawn->GetActorLocation();
        SpawnRotation = PreviousPawn->GetActorRotation();
    }
    else
    {
        FVector ViewLocation = FVector::ZeroVector;
        FRotator ViewRotation = FRotator::ZeroRotator;
        GetPlayerViewPoint(ViewLocation, ViewRotation);
        if (!ViewLocation.IsNearlyZero())
        {
            HeroSpawnLocation = ViewLocation;
            SpawnRotation = ViewRotation;
        }
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AHeroCharacter* NewHero = World->SpawnActor<AHeroCharacter>(PawnClass, HeroSpawnLocation, FRotator(0.0f, SpawnRotation.Yaw, 0.0f), SpawnParams);
    if (!NewHero)
    {
        return;
    }

    Possess(NewHero);
    SetControlRotation(FRotator(0.0f, SpawnRotation.Yaw, 0.0f));
    SetViewTarget(NewHero);

    if (PreviousPawn && PreviousPawn != NewHero)
    {
        PreviousPawn->Destroy();
    }
}

void AHeroPlayerController::SetMenuInputMode()
{
    bShowMouseCursor = true;
    SetIgnoreMoveInput(true);
    SetIgnoreLookInput(true);
    FInputModeUIOnly InputMode;
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    SetInputMode(InputMode);
}

void AHeroPlayerController::SetGameInputMode()
{
    bShowMouseCursor = false;
    bEnableClickEvents = false;
    bEnableMouseOverEvents = false;
    SetIgnoreMoveInput(false);
    SetIgnoreLookInput(false);
    FInputModeGameOnly InputMode;
    SetInputMode(InputMode);
    FlushPressedKeys();
}

void AHeroPlayerController::UpdateHUDFromPawn()
{
    if (!HeroHUD)
    {
        return;
    }

    AHeroCharacter* Hero = Cast<AHeroCharacter>(GetPawn());
    if (!Hero)
    {
        return;
    }

    UHeroHealthComponent* Health = Hero->GetHealthComponent();
    UHeroWeaponComponent* Weapon = Hero->GetWeaponComponent();
    UHeroVehicleModeComponent* VehicleMode = Hero->GetVehicleModeComponent();
    UHeroTeamComponent* Team = Hero->GetTeamComponent();

    if (VehicleMode)
    {
        HeroHUD->SetPlayerMode(VehicleMode->GetPlayerMode());
    }
    if (Health)
    {
        HeroHUD->SetHealth(Health->GetHealth(), Health->GetMaxHealth());
        HeroHUD->SetVehicleHealth(Health->GetHealth() * 1.8f, Health->GetMaxHealth() * 1.8f);
    }
    if (Weapon)
    {
        HeroHUD->SetWeaponName(Weapon->GetWeaponName());
        HeroHUD->SetAmmo(Weapon->GetAmmoInMagazine(), Weapon->GetReserveAmmo());
        HeroHUD->SetReloading(Weapon->IsReloading());
    }

    if (const AHeroGameModeBase* HeroGM = GetWorld() ? GetWorld()->GetAuthGameMode<AHeroGameModeBase>() : nullptr)
    {
        HeroHUD->SetObjectiveStatus(
            HeroGM->GetCustomGameSettings().GameModeType,
            Team ? Team->GetTeam() : EHeroTeam::None,
            HeroGM->GetPrimaryObjectiveState(),
            HeroGM->GetPrimaryObjectiveProgress01(),
            HeroGM->GetPrimaryObjectiveState() == EHeroObjectiveState::Contested);
    }
}
