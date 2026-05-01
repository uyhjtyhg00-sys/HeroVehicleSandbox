#pragma once

#include "SharedTypes.h"

struct FRaceCoreVector3
{
    double X = 0.0;
    double Y = 0.0;
    double Z = 0.0;

    FRaceCoreVector3() = default;
    FRaceCoreVector3(double InX, double InY, double InZ);

    FRaceCoreVector3 operator+(const FRaceCoreVector3& Other) const;
    FRaceCoreVector3 operator-(const FRaceCoreVector3& Other) const;
    FRaceCoreVector3 operator*(double Scalar) const;
    FRaceCoreVector3& operator+=(const FRaceCoreVector3& Other);

    double Size2D() const;
};

struct FRaceVehicleParameters
{
    float MassKg = 1120.0f;

    // Longitudinal force sources.
    float MaxEngineForceN = 11800.0f;
    float MaxBrakeForceN = 15500.0f;
    float MaxHandbrakeForceN = 9600.0f;

    // Stronger dissipation to stop unrealistic coasting.
    float DragCoefficient = 1.55f;
    float RollingResistanceN = 300.0f;
    float RollingResistanceSpeedCoefficient = 36.0f;
    float RollingResistanceQuadraticCoefficient = 0.85f;
    float EngineBrakingCoefficient = 1.15f;
    float DownforceCoefficient = 10.5f;

    // Basic vehicle geometry, meters.
    float WheelBaseMeters = 2.50f;
    float FrontAxleDistanceMeters = 1.08f;
    float RearAxleDistanceMeters = 1.42f;
    float TrackWidthMeters = 1.68f;
    float CenterOfMassHeightMeters = 0.46f;

    // Snappier race-car yaw dynamics.
    float YawInertiaKgM2 = 760.0f;
    float YawDamping = 3.65f;
    float ExtraReturnToCenterYawDamping = 0.72f;
    float LowSpeedYawAssistGain = 2.15f;
    float LowSpeedYawAssistMaxKmh = 45.0f;

    // Drift yaw moment.
    float HandbrakeYawKickTorqueNm = 9500.0f;
    float HandbrakeYawKickSpeedThreshold = 4.0f;
    float RearSlipYawTorqueScale = 4200.0f;
    float MaxDriftYawTorqueNm = 14000.0f;

    // Collision response.
    float CollisionRestitution = 0.26f;
    float CollisionAbsorption = 0.42f;
    float CollisionSurfaceFriction = 0.58f;
    float CollisionYawImpulseScale = 1.0f;
    float CollisionDamageThreshold = 2300.0f;
    float CollisionDamageScale = 0.0125f;

    // Steering and tire model.
    float MaxSteerAngleDegrees = 40.0f;
    float LowSpeedMaxSteerAngleDegrees = 48.0f;
    float SteeringSpeedSensitivity = 0.86f;
    float FrontCorneringStiffness = 165000.0f;
    float RearCorneringStiffness = 96000.0f;
    float PeakSlipAngleDegrees = 10.2f;
    float SlidingFrictionRatio = 0.84f;
    float HighSpeedUndersteerStrength = 0.36f;

    // Grip.
    float FrontLateralMu = 1.42f;
    float RearLateralMu = 1.22f;
    float LongitudinalMu = 1.15f;
    float HandbrakeRearLateralMuScale = 0.16f;
    float HandbrakeRearLongitudinalMuScale = 0.72f;

    // Stability helpers.
    float FullStopSpeedKmh = 0.5f;
    float StaticFrictionStopSpeed = 0.18f;
    float StaticFrictionAngularSpeed = 0.05f;
    float LowSpeedSteeringCutoff = 0.38f;
    float LateralVelocityDamping = 1.05f;
    float NoThrottleExtraDamping = 0.30f;

    // Anti-clip ride height and virtual suspension.
    float TargetRideHeightMeters = 0.20f;
    float SuspensionSpringStrength = 92.0f;
    float SuspensionDamping = 15.0f;
    float GroundProbeHeightMeters = 0.0f;

    // Visual/physics suspension tilt model.
    float MaxVisualRollDegrees = 9.5f;
    float MaxVisualPitchDegrees = 5.5f;
    float RollFromLateralG = 6.2f;
    float PitchFromLongitudinalG = 3.8f;
    float RollSpring = 42.0f;
    float RollDamping = 8.0f;
    float PitchSpring = 46.0f;
    float PitchDamping = 9.5f;
    float RollGripSensitivity = 0.38f;

    // Damage performance degradation.
    float EngineDamageStartsAtHealth = 50.0f;
    float MinimumEnginePowerScale = 0.18f;
    float SteeringDamageStartsAtHealth = 72.0f;
    float MinimumSteeringScale = 0.45f;
    float TransmissionDamageStartsAtHealth = 58.0f;
    float BaseShiftDelaySeconds = 0.06f;
    float DamagedShiftDelaySeconds = 0.38f;

    // Engine.
    float WheelRadiusMeters = 0.34f;
    float IdleRpm = 900.0f;
    float RedlineRpm = 7600.0f;
    float GearRatios[8] = { 0.0f, 3.35f, 2.28f, 1.72f, 1.34f, 1.08f, 0.88f, 0.72f };
    float FinalDriveRatio = 3.62f;

    ERaceTransmissionMode TransmissionMode = ERaceTransmissionMode::Automatic;
    ERaceVehicleClass VehicleClass = ERaceVehicleClass::Sports;
};

struct FRaceVehicleRuntimeState
{
    FRaceCoreVector3 PositionMeters;
    FRaceCoreVector3 VelocityMetersPerSecond;

    float YawRadians = 0.0f;
    float YawAngularVelocityRadPerSecond = 0.0f;

    float RollRadians = 0.0f;
    float PitchRadians = 0.0f;
    float RollAngularVelocityRadPerSecond = 0.0f;
    float PitchAngularVelocityRadPerSecond = 0.0f;

    float RideHeightMeters = 0.20f;
    float VerticalVelocityMetersPerSecond = 0.0f;

    float EngineRpm = 900.0f;
    float ForwardSpeedMetersPerSecond = 0.0f;

    float VehicleHealth = 100.0f;
    float EngineDamage01 = 0.0f;
    float SteeringDamage01 = 0.0f;
    float TransmissionDamage01 = 0.0f;
    float SteeringPullBias = 0.0f;
    float ImpactShake = 0.0f;

    float DamageFront01 = 0.0f;
    float DamageRear01 = 0.0f;
    float DamageLeft01 = 0.0f;
    float DamageRight01 = 0.0f;

    FRaceCoreVector3 LockedPositionMeters;

    int32 CurrentGear = 1;
    int32 LapIndex = 0;
    int32 CheckpointIndex = 0;
    int32 MaxUsableGear = 7;

    uint8 Grounded = 1;
    uint8 EngineDisabled = 0;
    uint8 bPositionLocked = 0;
};

struct FRaceVehicleDebugTelemetry
{
    float EngineForceN = 0.0f;
    float EngineBrakingForceN = 0.0f;
    float BrakeForceN = 0.0f;
    float DragForceN = 0.0f;
    float RollingResistanceForceN = 0.0f;
    float DownforceN = 0.0f;

    float FrontLateralForceN = 0.0f;
    float RearLateralForceN = 0.0f;
    float FrontSlipAngleDeg = 0.0f;
    float RearSlipAngleDeg = 0.0f;
    float FrontNormalLoadN = 0.0f;
    float RearNormalLoadN = 0.0f;
    float YawTorqueNm = 0.0f;
    float DriftYawTorqueNm = 0.0f;
    float YawAngularVelocity = 0.0f;

    float RollRadians = 0.0f;
    float PitchRadians = 0.0f;
    float RollDegrees = 0.0f;
    float PitchDegrees = 0.0f;
    float RideHeightMeters = 0.20f;
    float VerticalVelocityMetersPerSecond = 0.0f;
    float LateralG = 0.0f;
    float LongitudinalG = 0.0f;

    float VehicleHealth = 100.0f;
    float EngineDamage01 = 0.0f;
    float SteeringDamage01 = 0.0f;
    float TransmissionDamage01 = 0.0f;
    float SteeringPullBias = 0.0f;
    float DamageFront01 = 0.0f;
    float DamageRear01 = 0.0f;
    float DamageLeft01 = 0.0f;
    float DamageRight01 = 0.0f;
    float LastImpactMagnitude = 0.0f;
    float ImpactShake = 0.0f;
    uint8 EngineDisabled = 0;
    uint8 bPositionLocked = 0;

    float LateralSpeed = 0.0f;
    float NormalizedSpeed = 0.0f;
    float SkidAmount = 0.0f;
    float EnginePitch = 1.0f;
};

class FRaceVehicleCore
{
public:
    FRaceVehicleCore();

    void Initialize(const FRaceVehicleParameters& InParameters);
    void ResetState(double XMeters = 0.0, double YMeters = 0.0, double ZMeters = 0.0, float YawRadians = 0.0f);

    bool ValidateInputPacket(const FRaceVehicleInputPacket& Packet) const;
    void ApplyInputPacket(const FRaceVehicleInputPacket& Packet);

    void ApplyCollisionImpact(
        const FRaceCoreVector3& ImpactPointMeters,
        const FRaceCoreVector3& ImpactNormal,
        float ImpactMagnitude,
        float Restitution = -1.0f,
        float Absorption = -1.0f,
        float SurfaceFriction = -1.0f);

    FRaceVehicleInputPacket BuildInputPacketFromValues(
        uint64 SequenceId,
        uint32 PlayerId,
        float Throttle,
        float Brake,
        float Steering,
        uint8 Handbrake,
        int8 GearRequest,
        uint16 InputFlags,
        float ClientDeltaSeconds,
        uint32 InputFrame) const;

    void SimulateFixed(float FixedDeltaSeconds);

    FRaceVehicleStatePacket BuildStatePacket(uint64 SequenceId, uint32 PlayerId) const;

    const FRaceVehicleRuntimeState& GetRuntimeState() const;
    const FRaceVehicleDebugTelemetry& GetDebugTelemetry() const;
    const FRaceVehicleParameters& GetParameters() const;

    void SetLapState(int32 InLapIndex, int32 InCheckpointIndex);

private:
    void NormalizeInput();
    void UpdateTransmission(float FixedDeltaSeconds);

    void SimulateForceBased(float FixedDeltaSeconds);
    void UpdateSuspensionTilt(float FixedDeltaSeconds, float LongitudinalAcceleration, float LateralAcceleration);
    void UpdateRideHeight(float FixedDeltaSeconds);
    void UpdateDamagePerformanceState(float FixedDeltaSeconds);
    void ApplyFullStopIfNeeded();
    bool MaintainPositionLockIfNeeded();
    bool HasWakeInput() const;
    void ApplyPostIntegrationDamping(float FixedDeltaSeconds);

    void UpdateTelemetryFromDynamics(
        float FrontSlipAngleRad,
        float RearSlipAngleRad,
        float FrontLateralForceN,
        float RearLateralForceN,
        float FrontNormalLoadN,
        float RearNormalLoadN,
        float YawTorqueNm,
        float DriftYawTorqueNm,
        float LongitudinalAcceleration,
        float LateralAcceleration);

    float ComputeEngineForce() const;
    float ComputeEngineBrakingForce() const;
    float ComputeWheelRpmFromSpeed(float ForwardSpeedMetersPerSecond) const;
    float ComputeSteerAngleRadians(float SpeedAbs) const;
    float ComputeLowSpeedYawAssist(float SpeedAbs, float FixedDeltaSeconds) const;
    float ComputeDriftYawTorque(float SpeedAbs, float RearSlipAngleRad, float PeakSlipRad) const;

    float GetEnginePowerScale() const;
    float GetSteeringScale() const;
    int32 GetDamageLimitedMaxGear() const;
    float GetShiftDelaySeconds() const;

    static float ClampFloat(float Value, float MinValue, float MaxValue);
    static float SignNonZero(float Value);
    static float SafeSign(float Value, float DeadZone = 0.05f);

private:
    FRaceVehicleParameters Parameters;
    FRaceVehicleRuntimeState State;
    FRaceVehicleDebugTelemetry Telemetry;

    FRaceVehicleInputPacket CurrentInput{};

    float LastLongitudinalAcceleration = 0.0f;
    float LastLateralAcceleration = 0.0f;
    float GearShiftCooldownSeconds = 0.0f;
};
