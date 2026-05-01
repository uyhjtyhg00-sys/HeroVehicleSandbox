#include "HeroDriveableVehiclePawn.h"

#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "HeroCharacter.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
    constexpr float CmPerMeter = 100.0f;
    constexpr float MetersPerCm = 0.01f;
    constexpr float RadToDeg = 57.29577951308232f;
    constexpr float DegToRad = 0.017453292519943295f;
}

// 기능: 차량 Pawn 기본 컴포넌트와 RaceVehicleCore 기본값을 준비한다.
AHeroDriveableVehiclePawn::AHeroDriveableVehiclePawn()
{
    PrimaryActorTick.bCanEverTick = true;
    bUseControllerRotationYaw = false;

    VehicleRoot = CreateDefaultSubobject<USceneComponent>(TEXT("VehicleRoot"));
    SetRootComponent(VehicleRoot);

    VehicleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VehicleMesh"));
    VehicleMesh->SetupAttachment(VehicleRoot);
    VehicleMesh->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
    VehicleMesh->SetGenerateOverlapEvents(false);
    VehicleMesh->SetNotifyRigidBodyCollision(true);
    VehicleMesh->SetRelativeScale3D(FVector(2.7f, 1.35f, 0.55f));
    VehicleMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 65.0f));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (CubeMeshFinder.Succeeded())
    {
        VehicleMesh->SetStaticMesh(CubeMeshFinder.Object);
    }

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("VehicleCameraBoom"));
    CameraBoom->SetupAttachment(VehicleRoot);
    CameraBoom->TargetArmLength = 520.0f;
    CameraBoom->SocketOffset = FVector(0.0f, 0.0f, 135.0f);
    CameraBoom->bUsePawnControlRotation = true;
    CameraBoom->bInheritPitch = true;
    CameraBoom->bInheritYaw = true;
    CameraBoom->bInheritRoll = false;
    CameraBoom->bEnableCameraLag = false;
    CameraBoom->CameraLagSpeed = 18.0f;

    VehicleCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("VehicleCamera"));
    VehicleCamera->SetupAttachment(CameraBoom);
    VehicleCamera->bUsePawnControlRotation = false;
    VehicleCamera->FieldOfView = 96.0f;

    AutoPossessAI = EAutoPossessAI::Disabled;
}

// 기능: 월드 배치/스폰 직후 현재 Actor Transform을 순수 C++ 차량 코어 상태로 동기화한다.
void AHeroDriveableVehiclePawn::BeginPlay()
{
    Super::BeginPlay();

    FRaceVehicleParameters Parameters;
    Parameters.TargetRideHeightMeters = 0.20f;
    Parameters.GroundProbeHeightMeters = GetActorLocation().Z * MetersPerCm;
    VehicleCore.Initialize(Parameters);
    ResetCoreFromActorTransform();
}

// 기능: 탑승 중 입력을 RaceVehicleCore에 전달하고 차량 물리를 고정 스텝으로 진행한다.
void AHeroDriveableVehiclePawn::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    // Function: Vehicle physics keeps simulating after exit, so the car coasts naturally instead of freezing.
    if (!HasDriver())
    {
        ForwardInput = 0.0f;
        SteeringInput = 0.0f;
        bHandbrakeInput = 0;
    }

    PushInputToVehicleCore(DeltaSeconds);
    StepVehicleCore(DeltaSeconds);
    ApplyVehicleCoreTransform();
}

// 기능: 차량 조작 입력(W/S/A/D/Space/Q/E)을 Pawn에 직접 바인딩한다.
void AHeroDriveableVehiclePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // Function: Bind real vehicle drive controls and the shared G interaction key for exit.
    PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AHeroDriveableVehiclePawn::MoveForward);
    PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AHeroDriveableVehiclePawn::MoveRight);
    PlayerInputComponent->BindAxis(TEXT("Turn"), this, &AHeroDriveableVehiclePawn::Turn);
    PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &AHeroDriveableVehiclePawn::LookUp);
    PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &AHeroDriveableVehiclePawn::StartHandbrake);
    PlayerInputComponent->BindAction(TEXT("Jump"), IE_Released, this, &AHeroDriveableVehiclePawn::StopHandbrake);
    PlayerInputComponent->BindAction(TEXT("Interact"), IE_Pressed, this, &AHeroDriveableVehiclePawn::ExitVehicle);
}

// 기능: HeroCharacter를 숨기고 이 차량을 컨트롤러가 Possess하게 만들어 실제 탑승을 수행한다.
bool AHeroDriveableVehiclePawn::EnterVehicle(AHeroCharacter* DriverCharacter, AController* DriverController)
{
    if (!DriverCharacter || !DriverController)
    {
        return false;
    }

    StoredDriver = DriverCharacter;
    DriverCharacter->SetActorHiddenInGame(true);
    DriverCharacter->SetActorEnableCollision(false);

    if (APlayerController* PlayerController = Cast<APlayerController>(DriverController))
    {
        DriverCharacter->DisableInput(PlayerController);
    }

    DriverController->Possess(this);
    SetOwner(DriverController);

    if (APlayerController* PlayerController = Cast<APlayerController>(DriverController))
    {
        PlayerController->SetViewTarget(this);
        PlayerController->SetControlRotation(FRotator(VehicleCameraDefaultPitchDegrees, GetActorRotation().Yaw, 0.0f));
        EnableInput(PlayerController);
    }

    ForwardInput = 0.0f;
    SteeringInput = 0.0f;
    bHandbrakeInput = 0;
    PhysicsAccumulator = 0.0f;
    ResetCoreFromActorTransform();
    return true;
}

// 기능: 차량에서 내려 숨겨둔 HeroCharacter를 차량 옆에 복구하고 다시 Possess한다.
void AHeroDriveableVehiclePawn::ExitVehicle()
{
    AController* CurrentController = Controller;
    AHeroCharacter* DriverCharacter = StoredDriver.Get();
    if (!CurrentController || !DriverCharacter)
    {
        return;
    }

    DriverCharacter->SetActorLocationAndRotation(ComputeExitLocation(), FRotator(0.0f, GetActorRotation().Yaw, 0.0f), false, nullptr, ETeleportType::TeleportPhysics);
    DriverCharacter->SetActorHiddenInGame(false);
    DriverCharacter->SetActorEnableCollision(true);
    CurrentController->Possess(DriverCharacter);
    if (APlayerController* PlayerController = Cast<APlayerController>(CurrentController))
    {
        PlayerController->SetViewTarget(DriverCharacter);
        PlayerController->SetControlRotation(FRotator(0.0f, GetActorRotation().Yaw, 0.0f));
        DriverCharacter->EnableInput(PlayerController);
    }

    ForwardInput = 0.0f;
    SteeringInput = 0.0f;
    bHandbrakeInput = 0;
    StoredDriver.Reset();
}

// 기능: 현재 플레이어가 이 차량을 조종 중인지 반환한다.
bool AHeroDriveableVehiclePawn::HasDriver() const
{
    return StoredDriver.IsValid() && Controller != nullptr;
}

// 기능: 이 차량에 연결된 원래 HeroCharacter를 반환한다.
AHeroCharacter* AHeroDriveableVehiclePawn::GetStoredDriver() const
{
    return StoredDriver.Get();
}

// 기능: RaceVehicleCore의 현재 전진 속도를 km/h 단위로 반환한다.
float AHeroDriveableVehiclePawn::GetSpeedKmh() const
{
    return FMath::Abs(VehicleCore.GetRuntimeState().ForwardSpeedMetersPerSecond) * 3.6f;
}

// 기능: RaceVehicleCore의 현재 차량 체력을 반환한다.
float AHeroDriveableVehiclePawn::GetVehicleHealth() const
{
    return VehicleCore.GetRuntimeState().VehicleHealth;
}

// 기능: HUD 표시에 사용할 차량 최대 체력을 반환한다.
float AHeroDriveableVehiclePawn::GetMaxVehicleHealth() const
{
    return 100.0f;
}

// 기능: RaceVehicleCore의 현재 기어를 반환한다.
int32 AHeroDriveableVehiclePawn::GetCurrentGear() const
{
    return VehicleCore.GetRuntimeState().CurrentGear;
}

// 기능: RaceVehicleCore의 현재 엔진 RPM을 반환한다.
float AHeroDriveableVehiclePawn::GetEngineRpm() const
{
    return VehicleCore.GetRuntimeState().EngineRpm;
}

// 기능: HUD/디버그 연동용 RaceVehicleCore 텔레메트리를 반환한다.
const FRaceVehicleDebugTelemetry& AHeroDriveableVehiclePawn::GetVehicleTelemetry() const
{
    return VehicleCore.GetDebugTelemetry();
}

// 기능: W/S 입력을 가속/브레이크 축으로 저장한다.
void AHeroDriveableVehiclePawn::MoveForward(const float Value)
{
    ForwardInput = FMath::Clamp(Value, -1.0f, 1.0f);
}

// 기능: A/D 입력을 조향 축으로 저장한다.
void AHeroDriveableVehiclePawn::MoveRight(const float Value)
{
    SteeringInput = FMath::Clamp(Value, -1.0f, 1.0f);
}


// 기능: 차량 탑승 중 마우스 X 입력을 카메라 yaw에 적용한다. 차량 본체 yaw는 RaceVehicleCore가 담당한다.
void AHeroDriveableVehiclePawn::Turn(const float Value)
{
    if (!Controller || Value == 0.0f)
    {
        return;
    }

    const AHeroCharacter* DriverCharacter = StoredDriver.Get();
    const FHeroAimSettings AimSettings = DriverCharacter ? DriverCharacter->GetAimSettings() : FHeroAimSettings{};

    FRotator ControlRotation = Controller->GetControlRotation();
    ControlRotation.Yaw += Value * AimSettings.GetFinalDegreesPerCount(false, true);
    Controller->SetControlRotation(ControlRotation);
}

// 기능: 차량 탑승 중 마우스 Y 입력을 카메라 pitch에 적용한다. 마우스 반전 옵션은 수동 시점에만 적용된다.
void AHeroDriveableVehiclePawn::LookUp(const float Value)
{
    if (!Controller || Value == 0.0f)
    {
        return;
    }

    const AHeroCharacter* DriverCharacter = StoredDriver.Get();
    const FHeroAimSettings AimSettings = DriverCharacter ? DriverCharacter->GetAimSettings() : FHeroAimSettings{};
    const float UserInvertSign = AimSettings.bInvertMouseY ? -1.0f : 1.0f;

    FRotator ControlRotation = Controller->GetControlRotation();
    const float NewPitch = FMath::Clamp(
        FRotator::NormalizeAxis(ControlRotation.Pitch + Value * UserInvertSign * AimSettings.GetFinalDegreesPerCount(false, true)),
        VehicleCameraMinPitchDegrees,
        VehicleCameraMaxPitchDegrees);

    ControlRotation.Pitch = NewPitch;
    Controller->SetControlRotation(ControlRotation);
}

// 기능: Space 입력을 핸드브레이크 ON 상태로 만든다.
void AHeroDriveableVehiclePawn::StartHandbrake()
{
    bHandbrakeInput = 1;
}

// 기능: Space 입력 해제 시 핸드브레이크 OFF 상태로 만든다.
void AHeroDriveableVehiclePawn::StopHandbrake()
{
    bHandbrakeInput = 0;
}

// 기능: Q/E 입력으로 차량에서 내린다.
void AHeroDriveableVehiclePawn::RequestExitVehicle()
{
    ExitVehicle();
}

// 기능: Actor Transform(cm/degree)을 RaceVehicleCore 상태(m/radian)로 초기화한다.
void AHeroDriveableVehiclePawn::ResetCoreFromActorTransform()
{
    const FVector Location = GetActorLocation();
    const float YawRadians = GetActorRotation().Yaw * DegToRad;
    VehicleCore.ResetState(Location.X * MetersPerCm, Location.Y * MetersPerCm, Location.Z * MetersPerCm, YawRadians);
}

// 기능: 저장된 입력 축을 FRaceVehicleInputPacket으로 변환해 RaceVehicleCore에 적용한다.
void AHeroDriveableVehiclePawn::PushInputToVehicleCore(const float DeltaSeconds)
{
    const float Throttle = FMath::Max(0.0f, ForwardInput);
    const float Brake = FMath::Max(0.0f, -ForwardInput);

    const FRaceVehicleInputPacket Packet = VehicleCore.BuildInputPacketFromValues(
        InputSequence++,
        0,
        Throttle,
        Brake,
        SteeringInput,
        bHandbrakeInput,
        0,
        0,
        FMath::Clamp(DeltaSeconds, 0.0f, 0.1f),
        InputFrame++);

    VehicleCore.ApplyInputPacket(Packet);
}

// 기능: 누적 시간을 고정 스텝으로 쪼개 RaceVehicleCore::SimulateFixed를 호출한다.
void AHeroDriveableVehiclePawn::StepVehicleCore(const float DeltaSeconds)
{
    PhysicsAccumulator = FMath::Min(PhysicsAccumulator + DeltaSeconds, MaxPhysicsAccumulationSeconds);
    const float Step = FMath::Max(1.0f / 240.0f, FixedPhysicsStepSeconds);

    while (PhysicsAccumulator >= Step)
    {
        VehicleCore.SimulateFixed(Step);
        PhysicsAccumulator -= Step;
    }
}

// 기능: RaceVehicleCore 위치/회전 결과를 UE Actor Transform에 반영하고 충돌을 처리한다.
void AHeroDriveableVehiclePawn::ApplyVehicleCoreTransform()
{
    const FRaceVehicleRuntimeState& State = VehicleCore.GetRuntimeState();
    const FVector TargetLocation(
        State.PositionMeters.X * CmPerMeter,
        State.PositionMeters.Y * CmPerMeter,
        State.PositionMeters.Z * CmPerMeter);
    const FRotator TargetRotation(0.0f, State.YawRadians * RadToDeg, 0.0f);

    FHitResult Hit;
    SetActorLocationAndRotation(TargetLocation, TargetRotation, true, &Hit, ETeleportType::None);
    if (Hit.bBlockingHit)
    {
        HandleVehicleHit(Hit);
    }
}

// 기능: UE 충돌 결과를 RaceVehicleCore::ApplyCollisionImpact 인터페이스로 전달한다.
void AHeroDriveableVehiclePawn::HandleVehicleHit(const FHitResult& Hit)
{
    const FRaceVehicleRuntimeState& State = VehicleCore.GetRuntimeState();
    const float Speed = static_cast<float>(State.VelocityMetersPerSecond.Size2D());
    const float ImpactMagnitude = FMath::Clamp(Speed * VehicleCore.GetParameters().MassKg, 0.0f, 80000.0f);

    const FVector ImpactPoint = Hit.ImpactPoint;
    const FVector ImpactNormal = Hit.ImpactNormal.GetSafeNormal();

    VehicleCore.ApplyCollisionImpact(
        FRaceCoreVector3(ImpactPoint.X * MetersPerCm, ImpactPoint.Y * MetersPerCm, ImpactPoint.Z * MetersPerCm),
        FRaceCoreVector3(ImpactNormal.X, ImpactNormal.Y, ImpactNormal.Z),
        ImpactMagnitude);

    ResetCoreFromActorTransform();
}

// 기능: 하차 시 HeroCharacter가 튀어나올 안전한 위치를 계산한다.
FVector AHeroDriveableVehiclePawn::ComputeExitLocation() const
{
    return GetActorLocation() + GetActorRightVector() * ExitSideOffsetCm + FVector(0.0f, 0.0f, ExitUpOffsetCm);
}
