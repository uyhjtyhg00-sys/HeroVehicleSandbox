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
    // 기능: 1인칭 영웅 캐릭터의 기본 컴포넌트, 이동값, 카메라 상태를 초기화한다.
    AHeroCharacter();

    // 기능: 저장된 조준 설정을 불러오고 체력/모드 변경 이벤트를 연결한다.
    virtual void BeginPlay() override;

    // 기능: 매 프레임 카메라 높이와 조준 FOV를 부드럽게 보간한다.
    virtual void Tick(float DeltaSeconds) override;

    // 기능: Legacy Input 축/액션을 이동, 시점, 점프, 앉기, 사격, 조준, 재장전에 연결한다.
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

    // 기능: UE 기본 crouch 시작 이벤트를 유지해 앉기 상태와 충돌 높이를 정상 처리한다.
    virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

    // 기능: UE 기본 crouch 종료 이벤트를 유지해 서기 상태와 충돌 높이를 정상 복원한다.
    virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

    // 기능: 무기 조준/발사 기준이 되는 1인칭 카메라 컴포넌트를 반환한다.
    UCameraComponent* GetFirstPersonCamera() const;

    // 기능: 캐릭터의 무기 컴포넌트를 반환한다.
    UHeroWeaponComponent* GetWeaponComponent() const;

    // 기능: 체력/사망 처리를 담당하는 체력 컴포넌트를 반환한다.
    UHeroHealthComponent* GetHealthComponent() const;

    // 기능: 영웅/차량 모드 전환 상태를 담당하는 모드 컴포넌트를 반환한다.
    UHeroVehicleModeComponent* GetVehicleModeComponent() const;

    // 기능: 팀 정보를 담당하는 팀 컴포넌트를 반환한다.
    UHeroTeamComponent* GetTeamComponent() const;

    // 기능: 감도, FOV, 마우스 반전 설정을 캐릭터에 즉시 적용한다.
    UFUNCTION(BlueprintCallable, Category="Hero|Settings")
    void ApplyAimSettings(const FHeroAimSettings& NewSettings);

    // 기능: 현재 적용 중인 조준/감도 설정을 반환한다.
    UFUNCTION(BlueprintCallable, Category="Hero|Settings")
    const FHeroAimSettings& GetAimSettings() const;

    // 기능: 현재 캐릭터가 앉기 상태인지 반환한다.
    UFUNCTION(BlueprintCallable, Category="Hero|Movement")
    bool IsHeroCrouching() const;

    // 기능: 총기 반동/피격 흔들림처럼 마우스 반전과 무관한 카메라 킥을 적용한다.
    UFUNCTION(BlueprintCallable, Category="Hero|Camera")
    void ApplyViewKick(float PitchKickDegrees, float YawKickDegrees = 0.0f);

protected:
    // 기능: 전후 이동 입력을 처리하며 공중에서는 입력 효율을 낮춘다.
    void MoveForward(float Value);

    // 기능: 좌우 이동 입력을 처리하며 공중에서는 입력 효율을 낮춘다.
    void MoveRight(float Value);

    // 기능: 오버워치식 yaw 0.0066 감도 공식으로 좌우 시점을 회전한다.
    void Turn(float Value);

    // 기능: 오버워치식 yaw 0.0066 감도 공식으로 상하 시점을 회전하고 마우스 반전 옵션은 수동 입력에만 적용한다.
    void LookUp(float Value);

    // 기능: 현재 무기의 발사를 시작한다.
    void StartFire();

    // 기능: 현재 무기의 발사를 중단한다.
    void StopFire();

    // 기능: 우클릭 조준 상태를 켠다.
    void StartAim();

    // 기능: 우클릭 조준 상태를 끈다.
    void StopAim();

    // 기능: 현재 무기를 재장전한다.
    void Reload();

    // 기능: 앉기 입력 시작 시 crouch를 적용한다.
    void StartHeroCrouch();

    // 기능: 앉기 입력 종료 시 crouch를 해제한다.
    void StopHeroCrouch();

    // 기능: 영웅/차량 모드를 토글한다.
    void ToggleVehicleMode();

    // 기능: 목표물/차량/장치 상호작용용 예약 입력을 처리한다.
    void Interact();

    // 기능: 사망 시 발사와 차량 모드, crouch 상태를 정리한다.
    UFUNCTION()
    void HandleDeath();

private:
    // 기능: 지상 이동, 공중 제어, 점프, crouch 가능 여부를 초기 설정한다.
    void ConfigureCharacterMovement();

    // 기능: 저장 슬롯에서 감도/FOV/마우스 반전 설정을 불러온다.
    void LoadAimSettings();

    // 기능: 앉기/서기 상태에 따라 카메라 높이를 부드럽게 보간한다.
    void UpdateCameraHeight(float DeltaSeconds);

    // 기능: 우클릭 조준 상태에 따라 카메라 FOV를 부드럽게 보간한다.
    void UpdateCameraFov(float DeltaSeconds);

    // 기능: 공중 이동 시 입력 효율을 낮춰 오버워치식 제한된 공중 제어감을 만든다.
    float GetAirMovementScale() const;

    // 기능: 영웅/차량 모드 변화에 맞춰 무기 모드와 이동속도를 갱신한다.
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Movement", meta=(AllowPrivateAccess="true"))
    float CrouchWalkSpeed = 330.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Movement", meta=(AllowPrivateAccess="true"))
    float StandingEyeHeight = 74.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Movement", meta=(AllowPrivateAccess="true"))
    float HeroCrouchedEyeHeight = 48.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Movement", meta=(AllowPrivateAccess="true"))
    float CameraHeightInterpSpeed = 16.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Camera", meta=(AllowPrivateAccess="true"))
    float ScopedFovMultiplier = 0.72f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Camera", meta=(AllowPrivateAccess="true"))
    float FovInterpSpeed = 18.0f;

    // Overwatch-like air control: the player can still correct direction while airborne,
    // but the strength is intentionally lower than grounded input.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Movement", meta=(AllowPrivateAccess="true"))
    float AirMovementInputScale = 0.46f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Settings", meta=(AllowPrivateAccess="true"))
    FHeroAimSettings AimSettings;
};
