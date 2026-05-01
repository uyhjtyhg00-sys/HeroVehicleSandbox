#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "LegacyVehicle/RaceVehicleCore.h"
#include "HeroDriveableVehiclePawn.generated.h"

class AHeroCharacter;
class UCameraComponent;
class USceneComponent;
class USpringArmComponent;
class UStaticMeshComponent;

UCLASS()
class HEROVEHICLESANDBOX_API AHeroDriveableVehiclePawn : public APawn
{
    GENERATED_BODY()

public:
    // 기능: 차량 Pawn 기본 컴포넌트와 RaceVehicleCore 기본값을 준비한다.
    AHeroDriveableVehiclePawn();

    // 기능: 월드 배치/스폰 직후 현재 Actor Transform을 순수 C++ 차량 코어 상태로 동기화한다.
    virtual void BeginPlay() override;

    // 기능: 탑승 중 입력을 RaceVehicleCore에 전달하고 차량 물리를 고정 스텝으로 진행한다.
    virtual void Tick(float DeltaSeconds) override;

    // 기능: 차량 조작 입력(W/S/A/D/Space/Q/E)을 Pawn에 직접 바인딩한다.
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

    // 기능: HeroCharacter를 숨기고 이 차량을 컨트롤러가 Possess하게 만들어 실제 탑승을 수행한다.
    UFUNCTION(BlueprintCallable, Category="Hero|Vehicle")
    bool EnterVehicle(AHeroCharacter* DriverCharacter, AController* DriverController);

    // 기능: 차량에서 내려 숨겨둔 HeroCharacter를 차량 옆에 복구하고 다시 Possess한다.
    UFUNCTION(BlueprintCallable, Category="Hero|Vehicle")
    void ExitVehicle();

    // 기능: 현재 플레이어가 이 차량을 조종 중인지 반환한다.
    UFUNCTION(BlueprintCallable, Category="Hero|Vehicle")
    bool HasDriver() const;

    // 기능: 이 차량에 연결된 원래 HeroCharacter를 반환한다.
    UFUNCTION(BlueprintCallable, Category="Hero|Vehicle")
    AHeroCharacter* GetStoredDriver() const;

    // 기능: RaceVehicleCore의 현재 전진 속도를 km/h 단위로 반환한다.
    UFUNCTION(BlueprintCallable, Category="Hero|Vehicle")
    float GetSpeedKmh() const;

    // 기능: RaceVehicleCore의 현재 차량 체력을 반환한다.
    UFUNCTION(BlueprintCallable, Category="Hero|Vehicle")
    float GetVehicleHealth() const;

    // 기능: HUD 표시에 사용할 차량 최대 체력을 반환한다.
    UFUNCTION(BlueprintCallable, Category="Hero|Vehicle")
    float GetMaxVehicleHealth() const;

    // 기능: RaceVehicleCore의 현재 기어를 반환한다.
    UFUNCTION(BlueprintCallable, Category="Hero|Vehicle")
    int32 GetCurrentGear() const;

    // 기능: RaceVehicleCore의 현재 엔진 RPM을 반환한다.
    UFUNCTION(BlueprintCallable, Category="Hero|Vehicle")
    float GetEngineRpm() const;

    // 기능: HUD/디버그 연동용 RaceVehicleCore 텔레메트리를 반환한다.
    const FRaceVehicleDebugTelemetry& GetVehicleTelemetry() const;

protected:
    // 기능: W/S 입력을 가속/브레이크 축으로 저장한다.
    void MoveForward(float Value);

    // 기능: A/D 입력을 조향 축으로 저장한다.
    void MoveRight(float Value);

    // 기능: 차량 탑승 중 마우스 X 입력으로 차량 카메라를 좌우 회전시킨다.
    void Turn(float Value);

    // 기능: 차량 탑승 중 마우스 Y 입력으로 차량 카메라를 상하 회전시킨다.
    void LookUp(float Value);

    // 기능: Space 입력을 핸드브레이크 ON 상태로 만든다.
    void StartHandbrake();

    // 기능: Space 입력 해제 시 핸드브레이크 OFF 상태로 만든다.
    void StopHandbrake();

    // 기능: Q/E 입력으로 차량에서 내린다.
    void RequestExitVehicle();

private:
    // 기능: Actor Transform(cm/degree)을 RaceVehicleCore 상태(m/radian)로 초기화한다.
    void ResetCoreFromActorTransform();

    // 기능: 저장된 입력 축을 FRaceVehicleInputPacket으로 변환해 RaceVehicleCore에 적용한다.
    void PushInputToVehicleCore(float DeltaSeconds);

    // 기능: 누적 시간을 고정 스텝으로 쪼개 RaceVehicleCore::SimulateFixed를 호출한다.
    void StepVehicleCore(float DeltaSeconds);

    // 기능: RaceVehicleCore 위치/회전 결과를 UE Actor Transform에 반영하고 충돌을 처리한다.
    void ApplyVehicleCoreTransform();

    // 기능: UE 충돌 결과를 RaceVehicleCore::ApplyCollisionImpact 인터페이스로 전달한다.
    void HandleVehicleHit(const FHitResult& Hit);

    // 기능: 하차 시 HeroCharacter가 튀어나올 안전한 위치를 계산한다.
    FVector ComputeExitLocation() const;

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Hero|Vehicle", meta=(AllowPrivateAccess="true"))
    TObjectPtr<USceneComponent> VehicleRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Hero|Vehicle", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UStaticMeshComponent> VehicleMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Hero|Vehicle", meta=(AllowPrivateAccess="true"))
    TObjectPtr<USpringArmComponent> CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Hero|Vehicle", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UCameraComponent> VehicleCamera;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Vehicle", meta=(AllowPrivateAccess="true"))
    float FixedPhysicsStepSeconds = 1.0f / 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Vehicle", meta=(AllowPrivateAccess="true"))
    float MaxPhysicsAccumulationSeconds = 0.18f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Vehicle", meta=(AllowPrivateAccess="true"))
    float ExitSideOffsetCm = 170.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Vehicle", meta=(AllowPrivateAccess="true"))
    float ExitUpOffsetCm = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Vehicle|Camera", meta=(AllowPrivateAccess="true"))
    float VehicleCameraDefaultPitchDegrees = -8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Vehicle|Camera", meta=(AllowPrivateAccess="true"))
    float VehicleCameraMinPitchDegrees = -42.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Vehicle|Camera", meta=(AllowPrivateAccess="true"))
    float VehicleCameraMaxPitchDegrees = 24.0f;

    FRaceVehicleCore VehicleCore;
    uint64 InputSequence = 1;
    uint32 InputFrame = 0;
    float PhysicsAccumulator = 0.0f;
    float ForwardInput = 0.0f;
    float SteeringInput = 0.0f;
    uint8 bHandbrakeInput = 0;

    UPROPERTY(Transient)
    TWeakObjectPtr<AHeroCharacter> StoredDriver;
};
