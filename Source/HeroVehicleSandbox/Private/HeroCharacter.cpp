#include "HeroCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "HeroHealthComponent.h"
#include "HeroSettingsSaveGame.h"
#include "HeroTeamComponent.h"
#include "HeroVehicleModeComponent.h"
#include "HeroWeaponComponent.h"
#include "Kismet/GameplayStatics.h"

// 기능: 1인칭 영웅 캐릭터의 기본 컴포넌트, 이동값, 카메라 상태를 초기화한다.
AHeroCharacter::AHeroCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

    FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
    FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
    FirstPersonCamera->SetRelativeLocation(FVector(0.0f, 0.0f, StandingEyeHeight));
    FirstPersonCamera->bUsePawnControlRotation = true;
    FirstPersonCamera->FieldOfView = 103.0f;

    WeaponComponent = CreateDefaultSubobject<UHeroWeaponComponent>(TEXT("HeroWeaponComponent"));
    HealthComponent = CreateDefaultSubobject<UHeroHealthComponent>(TEXT("HeroHealthComponent"));
    VehicleModeComponent = CreateDefaultSubobject<UHeroVehicleModeComponent>(TEXT("HeroVehicleModeComponent"));
    TeamComponent = CreateDefaultSubobject<UHeroTeamComponent>(TEXT("HeroTeamComponent"));

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = true;
    bUseControllerRotationRoll = false;

    ConfigureCharacterMovement();
}

// 기능: 저장된 조준 설정을 불러오고 체력/모드 변경 이벤트를 연결한다.
void AHeroCharacter::BeginPlay()
{
    Super::BeginPlay();
    LoadAimSettings();

    if (HealthComponent)
    {
        HealthComponent->OnDeath.AddDynamic(this, &AHeroCharacter::HandleDeath);
    }

    if (VehicleModeComponent)
    {
        VehicleModeComponent->OnPlayerModeChanged.AddDynamic(this, &AHeroCharacter::ApplyModeToComponents);
        ApplyModeToComponents(VehicleModeComponent->GetPlayerMode());
    }
}

// 기능: 매 프레임 카메라 높이와 조준 FOV를 부드럽게 보간한다.
void AHeroCharacter::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    UpdateCameraHeight(DeltaSeconds);
    UpdateCameraFov(DeltaSeconds);
}

// 기능: Legacy Input 축/액션을 이동, 시점, 점프, 앉기, 사격, 조준, 재장전에 연결한다.
void AHeroCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (!PlayerInputComponent)
    {
        return;
    }

    PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AHeroCharacter::MoveForward);
    PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AHeroCharacter::MoveRight);
    PlayerInputComponent->BindAxis(TEXT("Turn"), this, &AHeroCharacter::Turn);
    PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &AHeroCharacter::LookUp);

    PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &ACharacter::Jump);
    PlayerInputComponent->BindAction(TEXT("Jump"), IE_Released, this, &ACharacter::StopJumping);
    PlayerInputComponent->BindAction(TEXT("Crouch"), IE_Pressed, this, &AHeroCharacter::StartHeroCrouch);
    PlayerInputComponent->BindAction(TEXT("Crouch"), IE_Released, this, &AHeroCharacter::StopHeroCrouch);
    PlayerInputComponent->BindAction(TEXT("Fire"), IE_Pressed, this, &AHeroCharacter::StartFire);
    PlayerInputComponent->BindAction(TEXT("Fire"), IE_Released, this, &AHeroCharacter::StopFire);
    PlayerInputComponent->BindAction(TEXT("Aim"), IE_Pressed, this, &AHeroCharacter::StartAim);
    PlayerInputComponent->BindAction(TEXT("Aim"), IE_Released, this, &AHeroCharacter::StopAim);
    PlayerInputComponent->BindAction(TEXT("Reload"), IE_Pressed, this, &AHeroCharacter::Reload);
    PlayerInputComponent->BindAction(TEXT("ToggleVehicleMode"), IE_Pressed, this, &AHeroCharacter::ToggleVehicleMode);
    PlayerInputComponent->BindAction(TEXT("Interact"), IE_Pressed, this, &AHeroCharacter::Interact);
}

// 기능: UE 기본 crouch 시작 이벤트를 유지해 앉기 상태와 충돌 높이를 정상 처리한다.
void AHeroCharacter::OnStartCrouch(const float HalfHeightAdjust, const float ScaledHalfHeightAdjust)
{
    Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
}

// 기능: UE 기본 crouch 종료 이벤트를 유지해 서기 상태와 충돌 높이를 정상 복원한다.
void AHeroCharacter::OnEndCrouch(const float HalfHeightAdjust, const float ScaledHalfHeightAdjust)
{
    Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
}

// 기능: 무기 조준/발사 기준이 되는 1인칭 카메라 컴포넌트를 반환한다.
UCameraComponent* AHeroCharacter::GetFirstPersonCamera() const
{
    return FirstPersonCamera;
}

// 기능: 캐릭터의 무기 컴포넌트를 반환한다.
UHeroWeaponComponent* AHeroCharacter::GetWeaponComponent() const
{
    return WeaponComponent;
}

// 기능: 체력/사망 처리를 담당하는 체력 컴포넌트를 반환한다.
UHeroHealthComponent* AHeroCharacter::GetHealthComponent() const
{
    return HealthComponent;
}

// 기능: 영웅/차량 모드 전환 상태를 담당하는 모드 컴포넌트를 반환한다.
UHeroVehicleModeComponent* AHeroCharacter::GetVehicleModeComponent() const
{
    return VehicleModeComponent;
}

// 기능: 팀 정보를 담당하는 팀 컴포넌트를 반환한다.
UHeroTeamComponent* AHeroCharacter::GetTeamComponent() const
{
    return TeamComponent;
}

// 기능: 감도, FOV, 마우스 반전 설정을 캐릭터에 즉시 적용한다.
void AHeroCharacter::ApplyAimSettings(const FHeroAimSettings& NewSettings)
{
    AimSettings = NewSettings;
    AimSettings.Clamp();

    if (FirstPersonCamera && (!WeaponComponent || !WeaponComponent->IsAiming()))
    {
        FirstPersonCamera->SetFieldOfView(AimSettings.FirstPersonFovDegrees);
    }
}

// 기능: 현재 적용 중인 조준/감도 설정을 반환한다.
const FHeroAimSettings& AHeroCharacter::GetAimSettings() const
{
    return AimSettings;
}

// 기능: 현재 캐릭터가 앉기 상태인지 반환한다.
bool AHeroCharacter::IsHeroCrouching() const
{
    return bIsCrouched;
}

// 기능: 전후 이동 입력을 처리하며 공중에서는 입력 효율을 낮춘다.
void AHeroCharacter::MoveForward(const float Value)
{
    if (Value != 0.0f && (!HealthComponent || !HealthComponent->IsDead()))
    {
        AddMovementInput(GetActorForwardVector(), Value * GetAirMovementScale());
    }
}

// 기능: 좌우 이동 입력을 처리하며 공중에서는 입력 효율을 낮춘다.
void AHeroCharacter::MoveRight(const float Value)
{
    if (Value != 0.0f && (!HealthComponent || !HealthComponent->IsDead()))
    {
        AddMovementInput(GetActorRightVector(), Value * GetAirMovementScale());
    }
}

// 기능: 오버워치식 yaw 0.0066 감도 공식으로 좌우 시점을 회전한다.
void AHeroCharacter::Turn(const float Value)
{
    if (Controller && Value != 0.0f && (!HealthComponent || !HealthComponent->IsDead()))
    {
        const bool bVehicle = VehicleModeComponent && VehicleModeComponent->GetPlayerMode() == EHeroPlayerMode::Vehicle;
        const bool bScoped = WeaponComponent && WeaponComponent->IsAiming();

        FRotator ControlRotation = Controller->GetControlRotation();
        ControlRotation.Yaw += Value * AimSettings.GetFinalDegreesPerCount(bScoped, bVehicle);
        Controller->SetControlRotation(ControlRotation);
    }
}

// 기능: 오버워치식 yaw 0.0066 감도 공식으로 상하 시점을 회전하고 마우스 반전 옵션은 수동 입력에만 적용한다.
void AHeroCharacter::LookUp(const float Value)
{
    if (Controller && Value != 0.0f && (!HealthComponent || !HealthComponent->IsDead()))
    {
        const bool bVehicle = VehicleModeComponent && VehicleModeComponent->GetPlayerMode() == EHeroPlayerMode::Vehicle;
        const bool bScoped = WeaponComponent && WeaponComponent->IsAiming();
        const float UserInvertSign = AimSettings.bInvertMouseY ? -1.0f : 1.0f;

        FRotator ControlRotation = Controller->GetControlRotation();
        float NormalizedPitch = FRotator::NormalizeAxis(ControlRotation.Pitch);
        NormalizedPitch = FMath::Clamp(
            NormalizedPitch + Value * UserInvertSign * AimSettings.GetFinalDegreesPerCount(bScoped, bVehicle),
            -89.0f,
            89.0f);

        ControlRotation.Pitch = NormalizedPitch;
        Controller->SetControlRotation(ControlRotation);
    }
}

// 기능: 현재 무기의 발사를 시작한다.
void AHeroCharacter::StartFire()
{
    if (WeaponComponent)
    {
        WeaponComponent->StartFire(FirstPersonCamera);
    }
}

// 기능: 현재 무기의 발사를 중단한다.
void AHeroCharacter::StopFire()
{
    if (WeaponComponent)
    {
        WeaponComponent->StopFire();
    }
}

// 기능: 우클릭 조준 상태를 켠다.
void AHeroCharacter::StartAim()
{
    if (WeaponComponent)
    {
        WeaponComponent->SetAiming(true);
    }
}

// 기능: 우클릭 조준 상태를 끈다.
void AHeroCharacter::StopAim()
{
    if (WeaponComponent)
    {
        WeaponComponent->SetAiming(false);
    }
}

// 기능: 현재 무기를 재장전한다.
void AHeroCharacter::Reload()
{
    if (WeaponComponent)
    {
        WeaponComponent->Reload();
    }
}

// 기능: 앉기 입력 시작 시 crouch를 적용한다.
void AHeroCharacter::StartHeroCrouch()
{
    if (!HealthComponent || !HealthComponent->IsDead())
    {
        Crouch();
    }
}

// 기능: 앉기 입력 종료 시 crouch를 해제한다.
void AHeroCharacter::StopHeroCrouch()
{
    UnCrouch();
}

// 기능: 영웅/차량 모드를 토글한다.
void AHeroCharacter::ToggleVehicleMode()
{
    if (VehicleModeComponent)
    {
        VehicleModeComponent->ToggleVehicleMode();
    }
}

// 기능: 목표물/차량/장치 상호작용용 예약 입력을 처리한다.
void AHeroCharacter::Interact()
{
    UE_LOG(LogTemp, Display, TEXT("Interact: objective/use slot reserved. Vehicle entry and devices can attach here."));
}

// 기능: 사망 시 발사와 차량 모드, crouch 상태를 정리한다.
void AHeroCharacter::HandleDeath()
{
    if (WeaponComponent)
    {
        WeaponComponent->StopFire();
    }

    if (VehicleModeComponent)
    {
        VehicleModeComponent->ExitVehicleMode();
    }

    StopHeroCrouch();
}

// 기능: 지상 이동, 공중 제어, 점프, crouch 가능 여부를 초기 설정한다.
void AHeroCharacter::ConfigureCharacterMovement()
{
    UCharacterMovementComponent* Move = GetCharacterMovement();
    if (!Move)
    {
        return;
    }

    Move->MaxWalkSpeed = HumanWalkSpeed;
    Move->MaxWalkSpeedCrouched = CrouchWalkSpeed;
    Move->MaxAcceleration = 12000.0f;
    Move->BrakingDecelerationWalking = 12000.0f;
    Move->GroundFriction = 14.0f;
    Move->BrakingFrictionFactor = 2.0f;

    // Not locked in air: the player can steer while airborne, but less than on ground.
    Move->AirControl = 0.38f;
    Move->AirControlBoostMultiplier = 1.15f;
    Move->AirControlBoostVelocityThreshold = 220.0f;
    Move->FallingLateralFriction = 0.28f;

    Move->JumpZVelocity = 520.0f;
    Move->GravityScale = 1.08f;
    Move->bOrientRotationToMovement = false;
    Move->GetNavAgentPropertiesRef().bCanCrouch = true;
    GetCapsuleComponent()->SetCapsuleHalfHeight(96.0f);
}

// 기능: 저장 슬롯에서 감도/FOV/마우스 반전 설정을 불러온다.
void AHeroCharacter::LoadAimSettings()
{
    UHeroSettingsSaveGame* Save = Cast<UHeroSettingsSaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("HeroVehicleSettings"), 0));
    if (!Save)
    {
        Save = Cast<UHeroSettingsSaveGame>(UGameplayStatics::CreateSaveGameObject(UHeroSettingsSaveGame::StaticClass()));
        UGameplayStatics::SaveGameToSlot(Save, TEXT("HeroVehicleSettings"), 0);
    }

    if (Save)
    {
        ApplyAimSettings(Save->AimSettings);
    }
}

// 기능: 앉기/서기 상태에 따라 카메라 높이를 부드럽게 보간한다.
void AHeroCharacter::UpdateCameraHeight(const float DeltaSeconds)
{
    if (!FirstPersonCamera)
    {
        return;
    }

    const float TargetZ = bIsCrouched ? HeroCrouchedEyeHeight : StandingEyeHeight;
    const FVector Current = FirstPersonCamera->GetRelativeLocation();
    const float NewZ = FMath::FInterpTo(Current.Z, TargetZ, DeltaSeconds, CameraHeightInterpSpeed);
    FirstPersonCamera->SetRelativeLocation(FVector(Current.X, Current.Y, NewZ));
}

// 기능: 우클릭 조준 상태에 따라 카메라 FOV를 부드럽게 보간한다.
void AHeroCharacter::UpdateCameraFov(const float DeltaSeconds)
{
    if (!FirstPersonCamera)
    {
        return;
    }

    const bool bScoped = WeaponComponent && WeaponComponent->IsAiming();
    const float BaseFov = AimSettings.FirstPersonFovDegrees;
    const float TargetFov = bScoped ? FMath::Clamp(BaseFov * ScopedFovMultiplier, 50.0f, BaseFov) : BaseFov;
    const float NewFov = FMath::FInterpTo(FirstPersonCamera->FieldOfView, TargetFov, DeltaSeconds, FovInterpSpeed);
    FirstPersonCamera->SetFieldOfView(NewFov);
}

// 기능: 공중 이동 시 입력 효율을 낮춰 오버워치식 제한된 공중 제어감을 만든다.
float AHeroCharacter::GetAirMovementScale() const
{
    const UCharacterMovementComponent* Move = GetCharacterMovement();
    if (!Move || !Move->IsFalling())
    {
        return 1.0f;
    }

    return AirMovementInputScale;
}

// 기능: 영웅/차량 모드 변화에 맞춰 무기 모드와 이동속도를 갱신한다.
void AHeroCharacter::ApplyModeToComponents(const EHeroPlayerMode NewMode)
{
    if (WeaponComponent)
    {
        WeaponComponent->SetPlayerMode(NewMode);
    }

    if (UCharacterMovementComponent* Move = GetCharacterMovement())
    {
        Move->MaxWalkSpeed = NewMode == EHeroPlayerMode::Vehicle ? VehicleModeWalkSpeed : HumanWalkSpeed;
        Move->MaxWalkSpeedCrouched = NewMode == EHeroPlayerMode::Vehicle ? VehicleModeWalkSpeed * 0.72f : CrouchWalkSpeed;
    }
}

// 기능: 총기 반동/피격 흔들림처럼 마우스 반전과 무관한 카메라 킥을 적용한다.
void AHeroCharacter::ApplyViewKick(const float PitchKickDegrees, const float YawKickDegrees)
{
    if (!Controller || (HealthComponent && HealthComponent->IsDead()))
    {
        return;
    }

    // Recoil/camera impulse must be physical and must NOT depend on bInvertMouseY.
    // Positive PitchKickDegrees uses the same pitch direction as normal "look up".
    FRotator ControlRotation = Controller->GetControlRotation();
    float NormalizedPitch = FRotator::NormalizeAxis(ControlRotation.Pitch);

    NormalizedPitch = FMath::Clamp(
        NormalizedPitch + FMath::Abs(PitchKickDegrees),
        -89.0f,
        89.0f);

    ControlRotation.Pitch = NormalizedPitch;
    ControlRotation.Yaw += YawKickDegrees;
    Controller->SetControlRotation(ControlRotation);
}
