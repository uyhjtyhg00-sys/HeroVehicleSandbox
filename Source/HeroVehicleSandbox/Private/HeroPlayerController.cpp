#include "HeroPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Framework/Application/SlateApplication.h"
#include "Engine/GameViewportClient.h"
#include "Engine/Engine.h"
#include "HeroCharacter.h"
#include "HeroGameModeBase.h"
#include "HeroHUDWidget.h"
#include "HeroHealthComponent.h"
#include "HeroMainMenuWidget.h"
#include "HeroSettingsWidget.h"
#include "HeroTeamComponent.h"
#include "HeroVehicleModeComponent.h"
#include "HeroWeaponComponent.h"
#include "InputCoreTypes.h"

// 기능: 기본 HUD/메뉴 상태와 기본 영웅 클래스 정보를 초기화한다.
AHeroPlayerController::AHeroPlayerController()
{
    bShowMouseCursor = true;
    HeroCharacterClass = AHeroCharacter::StaticClass();
}

// 기능: 시작 시 영웅 Pawn을 확보하고 메인 메뉴를 표시한다.
void AHeroPlayerController::BeginPlay()
{
    Super::BeginPlay();
    EnsureHeroPawnPossessed();
    ShowMainMenu();
}

// 기능: 매 프레임 현재 Pawn 상태를 HUD에 반영한다.
void AHeroPlayerController::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    UpdateHUDFromPawn();
}

// 기능: ESC/PauseMenu 입력을 설정 메뉴 토글에 연결한다.
void AHeroPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    if (InputComponent)
    {
// 기능: ESC 입력으로 설정 메뉴를 열거나 닫는다.
        InputComponent->BindAction(TEXT("PauseMenu"), IE_Pressed, this, &AHeroPlayerController::TogglePauseMenu);
    }
}

// 기능: 게임 HUD를 숨기고 메인 메뉴 UI와 UI 입력 모드를 표시한다.
void AHeroPlayerController::ShowMainMenu()
{
    ClearMenu();
    if (HeroHUD && HeroHUD->IsInViewport())
    {
        HeroHUD->RemoveFromParent();
    }

    UClass* WidgetClass = MainMenuWidgetClass ? MainMenuWidgetClass.Get() : UHeroMainMenuWidget::StaticClass();
    CurrentMenu = CreateWidget<UHeroMainMenuWidget>(this, WidgetClass);
    if (CurrentMenu)
    {
        CurrentMenu->AddToViewport(20);
    }
    SetMenuInputMode();
}

// 기능: 영웅 Pawn을 보장하고 HUD를 표시한 뒤 게임 입력 모드로 전환한다.
void AHeroPlayerController::ShowHUD()
{
    EnsureHeroPawnPossessed();
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

// 기능: 설정 메뉴를 표시하고 UI 입력 모드로 전환한다.
void AHeroPlayerController::ShowSettingsMenu()
{
    bSettingsOpenedFromGame = HeroHUD && HeroHUD->IsInViewport();
    ClearMenu();
    UClass* WidgetClass = SettingsWidgetClass ? SettingsWidgetClass.Get() : UHeroSettingsWidget::StaticClass();
    CurrentMenu = CreateWidget<UHeroSettingsWidget>(this, WidgetClass);
    if (CurrentMenu)
    {
        CurrentMenu->AddToViewport(30);
    }
    SetMenuInputMode();
}

// 기능: 설정 메뉴를 닫고 호출 위치에 따라 HUD 또는 메인 메뉴로 돌아간다.
void AHeroPlayerController::ReturnFromSettings()
{
    ClearMenu();
    if (bSettingsOpenedFromGame)
    {
        ShowHUD();
    }
    else
    {
        ShowMainMenu();
    }
}

// 기능: ESC 입력으로 설정 메뉴를 열거나 닫는다.
void AHeroPlayerController::TogglePauseMenu()
{
    if (CurrentMenu)
    {
        ReturnFromSettings();
        return;
    }

    ShowSettingsMenu();
}

// 기능: 샌드박스 모드 진입 시 Pawn possess와 입력 활성화를 보장한다.
void AHeroPlayerController::StartSandboxFromMenu()
{
    EnsureHeroPawnPossessed();

    if (APawn* ControlledPawn = GetPawn())
    {
        ControlledPawn->EnableInput(this);
    }

    ShowHUD();
}

// 기능: 플레이어 팀을 지정하고 전투 테스트 모드를 시작한다.
void AHeroPlayerController::StartCombatTestFromMenu()
{
    EnsureHeroPawnPossessed();
    AssignPlayerTeam(EHeroTeam::TeamA);
    if (AHeroGameModeBase* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AHeroGameModeBase>() : nullptr)
    {
        GM->StartCombatTestMode();
    }
    ShowHUD();
}

// 기능: 사용자 지정 게임 설정을 생성해 커스텀 게임을 시작한다.
void AHeroPlayerController::StartCustomGameFromMenu()
{
    EnsureHeroPawnPossessed();
    AssignPlayerTeam(EHeroTeam::TeamA);
    if (AHeroGameModeBase* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AHeroGameModeBase>() : nullptr)
    {
        GM->StartCustomGameMode(EHeroGameModeType::Control, 6);
    }
    ShowHUD();
}

// 기능: 현재 HUD 인스턴스를 반환한다.
UHeroHUDWidget* AHeroPlayerController::GetHeroHUD() const
{
    return HeroHUD;
}

// 기능: 현재 열려 있는 메뉴 위젯을 제거한다.
void AHeroPlayerController::ClearMenu()
{
    if (CurrentMenu)
    {
        CurrentMenu->RemoveFromParent();
        CurrentMenu = nullptr;
    }
}

// 기능: 현재 컨트롤 중인 Pawn이 없거나 영웅이 아니면 AHeroCharacter를 스폰/possess한다.
AHeroCharacter* AHeroPlayerController::EnsureHeroPawnPossessed()
{
    if (AHeroCharacter* CurrentHero = Cast<AHeroCharacter>(GetPawn()))
    {
        SetViewTarget(CurrentHero);
        return CurrentHero;
    }

    UWorld* World = GetWorld();
    UClass* PawnClass = HeroCharacterClass ? HeroCharacterClass.Get() : AHeroCharacter::StaticClass();
    if (!World || !PawnClass)
    {
        return nullptr;
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
        return nullptr;
    }

    Possess(NewHero);
    SetControlRotation(FRotator(0.0f, SpawnRotation.Yaw, 0.0f));
    SetViewTarget(NewHero);

    if (PreviousPawn && PreviousPawn != NewHero)
    {
        PreviousPawn->Destroy();
    }

    return NewHero;
}

// 기능: 플레이어 캐릭터의 팀 컴포넌트에 팀 값을 적용한다.
void AHeroPlayerController::AssignPlayerTeam(const EHeroTeam Team)
{
    if (AHeroCharacter* Hero = Cast<AHeroCharacter>(GetPawn()))
    {
        if (UHeroTeamComponent* TeamComponent = Hero->GetTeamComponent())
        {
            TeamComponent->SetTeam(Team);
        }
        if (UHeroHealthComponent* Health = Hero->GetHealthComponent())
        {
            Health->ResetHealth();
        }
    }
}

// 기능: 메뉴 조작용 UI 입력 모드와 마우스 커서를 활성화한다.
void AHeroPlayerController::SetMenuInputMode()
{
    bShowMouseCursor = true;
    bEnableClickEvents = true;
    bEnableMouseOverEvents = true;
    SetIgnoreMoveInput(true);
    SetIgnoreLookInput(true);
    FInputModeUIOnly InputMode;
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    SetInputMode(InputMode);
}

// 기능: FPS 조작용 게임 입력 모드, 마우스 캡처, 이동/시점 입력을 활성화한다.
void AHeroPlayerController::SetGameInputMode()
{
    EnsureHeroPawnPossessed();

    bShowMouseCursor = false;
    bEnableClickEvents = false;
    bEnableMouseOverEvents = false;

    SetIgnoreMoveInput(false);
    SetIgnoreLookInput(false);
    ResetIgnoreMoveInput();
    ResetIgnoreLookInput();

    FInputModeGameOnly InputMode;
    InputMode.SetConsumeCaptureMouseDown(false);
    SetInputMode(InputMode);

    if (GEngine && GEngine->GameViewport)
    {
        GEngine->GameViewport->SetMouseCaptureMode(EMouseCaptureMode::CapturePermanently_IncludingInitialMouseDown);
        GEngine->GameViewport->SetMouseLockMode(EMouseLockMode::LockAlways);
    }

    if (FSlateApplication::IsInitialized())
    {
        FSlateApplication::Get().SetAllUserFocusToGameViewport(EFocusCause::SetDirectly);
    }

    FlushPressedKeys();
}

// 기능: 체력, 무기, 탄약, 모드, 목표 상태를 HUD로 전달한다.
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
        HeroHUD->SetWeaponHeat(Weapon->GetWeaponHeat01());
    }

    if (const AHeroGameModeBase* HeroGM = GetWorld() ? GetWorld()->GetAuthGameMode<AHeroGameModeBase>() : nullptr)
    {
        HeroHUD->SetObjectiveStatus(
            HeroGM->GetCustomGameSettings().GameModeType,
            Team ? Team->GetTeam() : EHeroTeam::None,
            HeroGM->GetPrimaryObjectiveState(),
            HeroGM->GetPrimaryObjectiveProgress01(),
            HeroGM->GetPrimaryObjectiveState() == EHeroObjectiveState::Contested);
        HeroHUD->SetBotStatus(HeroGM->GetAliveBotCount(), HeroGM->GetTotalSpawnedBotCount());
    }
}

// 기능: 설정 메뉴가 열려 있으면 닫고, 닫혀 있으면 연다.
void AHeroPlayerController::ToggleSettingsMenu()
{
    if (CurrentMenu)
    {
        ClearMenu();
        ShowHUD();
        return;
    }

    ShowSettingsMenu();
}
