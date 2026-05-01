#include "LegacyVehicle/RaceVehicleCore.h"

#include <algorithm>
#include <cmath>
#include <cstring>

namespace
{
    constexpr float Gravity = 9.80665f;
    constexpr float Pi = 3.14159265358979323846f;
    constexpr float RadToDeg = 180.0f / Pi;
    constexpr float DegToRad = Pi / 180.0f;

    struct FWheelBasis2D
    {
        FRaceCoreVector3 Forward;
        FRaceCoreVector3 Right;
    };

    static double Dot2D(const FRaceCoreVector3& A, const FRaceCoreVector3& B)
    {
        return A.X * B.X + A.Y * B.Y;
    }

    static double Cross2D(const FRaceCoreVector3& A, const FRaceCoreVector3& B)
    {
        return A.X * B.Y - A.Y * B.X;
    }

    static FRaceCoreVector3 Normalize2D(const FRaceCoreVector3& V)
    {
        const double Size = std::sqrt(V.X * V.X + V.Y * V.Y);
        if (Size <= 1.0e-6)
        {
            return FRaceCoreVector3(1.0, 0.0, 0.0);
        }

        return FRaceCoreVector3(V.X / Size, V.Y / Size, 0.0);
    }

    static FRaceCoreVector3 Rotate2D(const FRaceCoreVector3& V, const float AngleRad)
    {
        const double C = std::cos(AngleRad);
        const double S = std::sin(AngleRad);
        return FRaceCoreVector3(V.X * C - V.Y * S, V.X * S + V.Y * C, 0.0);
    }

    static FRaceCoreVector3 YawCrossRadius(const float AngularVelocity, const FRaceCoreVector3& Radius)
    {
        return FRaceCoreVector3(
            -static_cast<double>(AngularVelocity) * Radius.Y,
             static_cast<double>(AngularVelocity) * Radius.X,
             0.0);
    }

    static float ClampAbs(float Value, float Limit)
    {
        Limit = std::max(0.0f, Limit);
        return std::max(-Limit, std::min(Value, Limit));
    }

    static float SaturatingTireLateralForce(
        const float SlipAngleRad,
        const float CorneringStiffness,
        const float NormalLoadN,
        const float Mu,
        const float PeakSlipAngleRad,
        const float SlidingFrictionRatio,
        const float LongitudinalForceAbsN)
    {
        const float FrictionCircleLimit = std::max(0.0f, Mu * NormalLoadN);
        const float AvailableLateral = std::sqrt(std::max(0.0f, FrictionCircleLimit * FrictionCircleLimit - LongitudinalForceAbsN * LongitudinalForceAbsN));

        if (AvailableLateral <= 1.0f)
        {
            return 0.0f;
        }

        const float RawForce = -CorneringStiffness * SlipAngleRad;
        const float AbsSlip = std::abs(SlipAngleRad);
        const float SafePeak = std::max(0.5f * DegToRad, PeakSlipAngleRad);

        if (AbsSlip <= SafePeak)
        {
            return ClampAbs(RawForce, AvailableLateral);
        }

        const float Excess = (AbsSlip - SafePeak) / SafePeak;
        const float Falloff = 1.0f / (1.0f + Excess * 0.46f);
        const float SlideLimit = AvailableLateral * std::max(0.35f, SlidingFrictionRatio * Falloff);

        return ClampAbs(RawForce, SlideLimit);
    }

    static float ComputeBrakeForceForWheelSpeed(const float BrakeInput, const float MaxBrakeForce, const float WheelLongitudinalSpeed)
    {
        if (BrakeInput <= 0.0f)
        {
            return 0.0f;
        }

        const float Sign = std::abs(WheelLongitudinalSpeed) < 0.08f
            ? 0.0f
            : (WheelLongitudinalSpeed < 0.0f ? 1.0f : -1.0f);

        return Sign * BrakeInput * MaxBrakeForce;
    }

    static FRaceCoreVector3 ForceFromBasis(const FWheelBasis2D& Basis, const float LongitudinalN, const float LateralN)
    {
        return Basis.Forward * static_cast<double>(LongitudinalN)
            + Basis.Right * static_cast<double>(LateralN);
    }
}

FRaceCoreVector3::FRaceCoreVector3(const double InX, const double InY, const double InZ)
    : X(InX), Y(InY), Z(InZ)
{
}

FRaceCoreVector3 FRaceCoreVector3::operator+(const FRaceCoreVector3& Other) const
{
    return FRaceCoreVector3(X + Other.X, Y + Other.Y, Z + Other.Z);
}

FRaceCoreVector3 FRaceCoreVector3::operator-(const FRaceCoreVector3& Other) const
{
    return FRaceCoreVector3(X - Other.X, Y - Other.Y, Z - Other.Z);
}

FRaceCoreVector3 FRaceCoreVector3::operator*(const double Scalar) const
{
    return FRaceCoreVector3(X * Scalar, Y * Scalar, Z * Scalar);
}

FRaceCoreVector3& FRaceCoreVector3::operator+=(const FRaceCoreVector3& Other)
{
    X += Other.X;
    Y += Other.Y;
    Z += Other.Z;
    return *this;
}

double FRaceCoreVector3::Size2D() const
{
    return std::sqrt(X * X + Y * Y);
}

FRaceVehicleCore::FRaceVehicleCore()
{
    std::memset(&CurrentInput, 0, sizeof(CurrentInput));
    CurrentInput.Header.SequenceId = 0;
    RaceBridge::FinalizeVehicleInputPacket(CurrentInput);
    ResetState();
}

void FRaceVehicleCore::Initialize(const FRaceVehicleParameters& InParameters)
{
    Parameters = InParameters;

    Parameters.MassKg = std::max(1.0f, Parameters.MassKg);
    Parameters.WheelBaseMeters = std::max(1.0f, Parameters.WheelBaseMeters);
    Parameters.FrontAxleDistanceMeters = std::max(0.25f, Parameters.FrontAxleDistanceMeters);
    Parameters.RearAxleDistanceMeters = std::max(0.25f, Parameters.RearAxleDistanceMeters);
    Parameters.TrackWidthMeters = std::max(0.5f, Parameters.TrackWidthMeters);
    Parameters.CenterOfMassHeightMeters = std::max(0.05f, Parameters.CenterOfMassHeightMeters);
    Parameters.YawInertiaKgM2 = std::max(1.0f, Parameters.YawInertiaKgM2);
    Parameters.TargetRideHeightMeters = std::max(0.02f, Parameters.TargetRideHeightMeters);

    const float AxleSum = Parameters.FrontAxleDistanceMeters + Parameters.RearAxleDistanceMeters;
    if (std::abs(AxleSum - Parameters.WheelBaseMeters) > 0.05f)
    {
        const float FrontRatio = Parameters.FrontAxleDistanceMeters / std::max(0.01f, AxleSum);
        Parameters.FrontAxleDistanceMeters = Parameters.WheelBaseMeters * FrontRatio;
        Parameters.RearAxleDistanceMeters = Parameters.WheelBaseMeters - Parameters.FrontAxleDistanceMeters;
    }

    State.EngineRpm = Parameters.IdleRpm;
    State.CurrentGear = std::max(1, State.CurrentGear);
    State.MaxUsableGear = 7;

    if (State.PositionMeters.Z < Parameters.TargetRideHeightMeters * 0.75)
    {
        State.PositionMeters.Z = Parameters.TargetRideHeightMeters;
        State.RideHeightMeters = Parameters.TargetRideHeightMeters;
        State.VerticalVelocityMetersPerSecond = 0.0f;
    }
}

void FRaceVehicleCore::ResetState(const double XMeters, const double YMeters, const double ZMeters, const float InYawRadians)
{
    const double InitialZ = static_cast<double>(Parameters.GroundProbeHeightMeters + Parameters.TargetRideHeightMeters);

    State.PositionMeters = FRaceCoreVector3(XMeters, YMeters, InitialZ);
    State.VelocityMetersPerSecond = FRaceCoreVector3(0.0, 0.0, 0.0);
    State.YawRadians = InYawRadians;
    State.YawAngularVelocityRadPerSecond = 0.0f;

    State.RollRadians = 0.0f;
    State.PitchRadians = 0.0f;
    State.RollAngularVelocityRadPerSecond = 0.0f;
    State.PitchAngularVelocityRadPerSecond = 0.0f;

    State.RideHeightMeters = Parameters.TargetRideHeightMeters;
    State.VerticalVelocityMetersPerSecond = 0.0f;

    State.EngineRpm = Parameters.IdleRpm;
    State.ForwardSpeedMetersPerSecond = 0.0f;

    State.VehicleHealth = 100.0f;
    State.EngineDamage01 = 0.0f;
    State.SteeringDamage01 = 0.0f;
    State.TransmissionDamage01 = 0.0f;
    State.SteeringPullBias = 0.0f;
    State.ImpactShake = 0.0f;
    State.DamageFront01 = 0.0f;
    State.DamageRear01 = 0.0f;
    State.DamageLeft01 = 0.0f;
    State.DamageRight01 = 0.0f;
    State.MaxUsableGear = 7;
    State.EngineDisabled = 0;
    State.bPositionLocked = 0;
    State.LockedPositionMeters = State.PositionMeters;

    State.CurrentGear = 1;
    State.LapIndex = 0;
    State.CheckpointIndex = 0;
    State.Grounded = 1;

    LastLongitudinalAcceleration = 0.0f;
    LastLateralAcceleration = 0.0f;
    GearShiftCooldownSeconds = 0.0f;

    Telemetry = FRaceVehicleDebugTelemetry{};
    Telemetry.RideHeightMeters = Parameters.TargetRideHeightMeters;
}

bool FRaceVehicleCore::ValidateInputPacket(const FRaceVehicleInputPacket& Packet) const
{
    if (!RaceBridge::ValidateVehicleInputPacket(Packet))
    {
        return false;
    }

    return Packet.Throttle >= -0.001f && Packet.Throttle <= 1.001f
        && Packet.Brake >= -0.001f && Packet.Brake <= 1.001f
        && Packet.Steering >= -1.001f && Packet.Steering <= 1.001f
        && Packet.ClientDeltaSeconds >= 0.0f && Packet.ClientDeltaSeconds <= 0.25f;
}

void FRaceVehicleCore::ApplyInputPacket(const FRaceVehicleInputPacket& Packet)
{
    if (!ValidateInputPacket(Packet))
    {
        return;
    }

    CurrentInput = Packet;
    NormalizeInput();
}

void FRaceVehicleCore::ApplyCollisionImpact(
    const FRaceCoreVector3& ImpactPointMeters,
    const FRaceCoreVector3& ImpactNormal,
    const float ImpactMagnitude,
    float Restitution,
    float Absorption,
    float SurfaceFriction)
{
    State.bPositionLocked = 0;

    FRaceCoreVector3 Normal = Normalize2D(ImpactNormal);
    const float EffectiveRestitution = ClampFloat(Restitution < 0.0f ? Parameters.CollisionRestitution : Restitution, 0.0f, 0.95f);
    const float EffectiveAbsorption = ClampFloat(Absorption < 0.0f ? Parameters.CollisionAbsorption : Absorption, 0.0f, 0.95f);
    const float EffectiveFriction = ClampFloat(SurfaceFriction < 0.0f ? Parameters.CollisionSurfaceFriction : SurfaceFriction, 0.0f, 0.98f);

    const double NormalSpeed = Dot2D(State.VelocityMetersPerSecond, Normal);
    const float IntoSurfaceSpeed = static_cast<float>(std::max(0.0, -NormalSpeed));

    if (IntoSurfaceSpeed > 0.02f)
    {
        const FRaceCoreVector3 NormalVelocity = Normal * NormalSpeed;
        const FRaceCoreVector3 TangentialVelocity = State.VelocityMetersPerSecond - NormalVelocity;

        const float ImpactSeverity = ClampFloat(ImpactMagnitude / 14000.0f, 0.0f, 1.0f);
        const FRaceCoreVector3 ReflectedNormalVelocity = Normal * (static_cast<double>(IntoSurfaceSpeed) * EffectiveRestitution);
        const FRaceCoreVector3 DampenedTangentialVelocity = TangentialVelocity * std::max(0.0f, 1.0f - EffectiveFriction * (0.25f + ImpactSeverity * 0.65f));

        State.VelocityMetersPerSecond = (ReflectedNormalVelocity + DampenedTangentialVelocity) * std::max(0.0f, 1.0f - EffectiveAbsorption * ImpactSeverity);

        const FRaceCoreVector3 Radius = ImpactPointMeters - State.PositionMeters;
        const FRaceCoreVector3 Impulse = Normal * (static_cast<double>(Parameters.MassKg) * IntoSurfaceSpeed * (1.0f + EffectiveRestitution));
        const float AngularImpulse = static_cast<float>(Cross2D(Radius, Impulse));
        const float DeltaYawRate = ClampFloat(
            AngularImpulse / std::max(1.0f, Parameters.YawInertiaKgM2) * Parameters.CollisionYawImpulseScale,
            -7.0f,
            7.0f);

        State.YawAngularVelocityRadPerSecond += DeltaYawRate;
    }

    const float Damage = std::max(0.0f, ImpactMagnitude - Parameters.CollisionDamageThreshold) * Parameters.CollisionDamageScale;
    if (Damage > 0.0f)
    {
        State.VehicleHealth = ClampFloat(State.VehicleHealth - Damage, 0.0f, 100.0f);

        const FRaceCoreVector3 BodyForward(std::cos(State.YawRadians), std::sin(State.YawRadians), 0.0);
        const FRaceCoreVector3 BodyRight(-std::sin(State.YawRadians), std::cos(State.YawRadians), 0.0);
        const FRaceCoreVector3 LocalImpact = ImpactPointMeters - State.PositionMeters;

        const float LocalForward = static_cast<float>(Dot2D(LocalImpact, BodyForward));
        const float LocalRight = static_cast<float>(Dot2D(LocalImpact, BodyRight));
        const float Damage01 = ClampFloat(Damage / 35.0f, 0.0f, 1.0f);

        if (std::abs(LocalForward) >= std::abs(LocalRight))
        {
            if (LocalForward >= 0.0f)
            {
                State.DamageFront01 = ClampFloat(State.DamageFront01 + Damage01, 0.0f, 1.0f);
                State.EngineDamage01 = ClampFloat(State.EngineDamage01 + Damage01 * 0.85f, 0.0f, 1.0f);
            }
            else
            {
                State.DamageRear01 = ClampFloat(State.DamageRear01 + Damage01, 0.0f, 1.0f);
                State.TransmissionDamage01 = ClampFloat(State.TransmissionDamage01 + Damage01 * 0.65f, 0.0f, 1.0f);
            }
        }
        else if (LocalRight >= 0.0f)
        {
            State.DamageRight01 = ClampFloat(State.DamageRight01 + Damage01, 0.0f, 1.0f);
            State.SteeringDamage01 = ClampFloat(State.SteeringDamage01 + Damage01 * 0.75f, 0.0f, 1.0f);
            State.SteeringPullBias = ClampFloat(State.SteeringPullBias - Damage01 * 0.24f, -0.45f, 0.45f);
        }
        else
        {
            State.DamageLeft01 = ClampFloat(State.DamageLeft01 + Damage01, 0.0f, 1.0f);
            State.SteeringDamage01 = ClampFloat(State.SteeringDamage01 + Damage01 * 0.75f, 0.0f, 1.0f);
            State.SteeringPullBias = ClampFloat(State.SteeringPullBias + Damage01 * 0.24f, -0.45f, 0.45f);
        }

        State.ImpactShake = std::max(State.ImpactShake, ClampFloat(Damage / 22.0f, 0.0f, 1.0f));
    }

    Telemetry.LastImpactMagnitude = ImpactMagnitude;
    UpdateDamagePerformanceState(0.0f);
}

FRaceVehicleInputPacket FRaceVehicleCore::BuildInputPacketFromValues(
    const uint64 SequenceId,
    const uint32 PlayerId,
    const float Throttle,
    const float Brake,
    const float Steering,
    const uint8 Handbrake,
    const int8 GearRequest,
    const uint16 InputFlags,
    const float ClientDeltaSeconds,
    const uint32 InputFrame) const
{
    FRaceVehicleInputPacket Packet{};
    Packet.Header.SequenceId = SequenceId;
    Packet.Throttle = ClampFloat(Throttle, 0.0f, 1.0f);
    Packet.Brake = ClampFloat(Brake, 0.0f, 1.0f);
    Packet.Steering = ClampFloat(Steering, -1.0f, 1.0f);
    Packet.Handbrake = Handbrake != 0 ? 1 : 0;
    Packet.GearRequest = GearRequest;
    Packet.InputFlags = InputFlags;
    Packet.PlayerId = PlayerId;
    Packet.ClientDeltaSeconds = ClampFloat(ClientDeltaSeconds, 0.0f, 0.25f);
    Packet.InputFrame = InputFrame;

    RaceBridge::FinalizeVehicleInputPacket(Packet);
    return Packet;
}

void FRaceVehicleCore::NormalizeInput()
{
    CurrentInput.Throttle = ClampFloat(CurrentInput.Throttle, 0.0f, 1.0f);
    CurrentInput.Brake = ClampFloat(CurrentInput.Brake, 0.0f, 1.0f);
    CurrentInput.Steering = ClampFloat(CurrentInput.Steering, -1.0f, 1.0f);
    CurrentInput.Handbrake = CurrentInput.Handbrake != 0 ? 1 : 0;
    CurrentInput.ClientDeltaSeconds = ClampFloat(CurrentInput.ClientDeltaSeconds, 0.0f, 0.25f);

    if (State.EngineDisabled != 0)
    {
        CurrentInput.Throttle = 0.0f;
    }

    if (HasWakeInput())
    {
        State.bPositionLocked = 0;
    }

    RaceBridge::FinalizeVehicleInputPacket(CurrentInput);
}

void FRaceVehicleCore::SimulateFixed(const float FixedDeltaSeconds)
{
    if (FixedDeltaSeconds <= 0.0f || FixedDeltaSeconds > 0.1f)
    {
        return;
    }

    if (MaintainPositionLockIfNeeded())
    {
        return;
    }

    UpdateDamagePerformanceState(FixedDeltaSeconds);
    UpdateTransmission(FixedDeltaSeconds);
    SimulateForceBased(FixedDeltaSeconds);
    ApplyPostIntegrationDamping(FixedDeltaSeconds);
    ApplyFullStopIfNeeded();

    State.ImpactShake = ClampFloat(State.ImpactShake - FixedDeltaSeconds * 2.4f, 0.0f, 1.0f);
}

void FRaceVehicleCore::UpdateTransmission(const float FixedDeltaSeconds)
{
    GearShiftCooldownSeconds = std::max(0.0f, GearShiftCooldownSeconds - FixedDeltaSeconds);

    const int32 MaxGear = GetDamageLimitedMaxGear();

    if (Parameters.TransmissionMode == ERaceTransmissionMode::Manual || Parameters.TransmissionMode == ERaceTransmissionMode::SemiAutomatic)
    {
        if (CurrentInput.GearRequest >= 1 && CurrentInput.GearRequest <= MaxGear && GearShiftCooldownSeconds <= 0.0f)
        {
            State.CurrentGear = CurrentInput.GearRequest;
            GearShiftCooldownSeconds = GetShiftDelaySeconds();
        }

        State.CurrentGear = std::max(1, std::min(State.CurrentGear, MaxGear));
        return;
    }

    if (GearShiftCooldownSeconds > 0.0f)
    {
        return;
    }

    if (State.EngineRpm > Parameters.RedlineRpm * 0.92f && State.CurrentGear < MaxGear)
    {
        ++State.CurrentGear;
        GearShiftCooldownSeconds = GetShiftDelaySeconds();
    }
    else if (State.EngineRpm < Parameters.IdleRpm * 1.65f && State.CurrentGear > 1)
    {
        --State.CurrentGear;
        GearShiftCooldownSeconds = GetShiftDelaySeconds();
    }

    State.CurrentGear = std::max(1, std::min(State.CurrentGear, MaxGear));
}

void FRaceVehicleCore::SimulateForceBased(const float FixedDeltaSeconds)
{
    const float Mass = std::max(1.0f, Parameters.MassKg);
    const float Speed = static_cast<float>(State.VelocityMetersPerSecond.Size2D());
    const float SpeedKmh = Speed * 3.6f;

    const FRaceCoreVector3 BodyForward(std::cos(State.YawRadians), std::sin(State.YawRadians), 0.0);
    const FRaceCoreVector3 BodyRight(-std::sin(State.YawRadians), std::cos(State.YawRadians), 0.0);

    State.ForwardSpeedMetersPerSecond = static_cast<float>(Dot2D(State.VelocityMetersPerSecond, BodyForward));

    const bool bWantsNoMotion =
        CurrentInput.Throttle <= 0.001f &&
        CurrentInput.Brake > 0.15f &&
        Speed < Parameters.StaticFrictionStopSpeed &&
        std::abs(State.YawAngularVelocityRadPerSecond) < Parameters.StaticFrictionAngularSpeed;

    if (bWantsNoMotion)
    {
        State.VelocityMetersPerSecond = FRaceCoreVector3(0.0, 0.0, 0.0);
        State.ForwardSpeedMetersPerSecond = 0.0f;
        State.YawAngularVelocityRadPerSecond = 0.0f;
        State.EngineRpm = Parameters.IdleRpm;
        LastLongitudinalAcceleration = 0.0f;
        LastLateralAcceleration = 0.0f;
        UpdateRideHeight(FixedDeltaSeconds);
        UpdateSuspensionTilt(FixedDeltaSeconds, 0.0f, 0.0f);
        UpdateTelemetryFromDynamics(0.0f, 0.0f, 0.0f, 0.0f, Mass * Gravity * 0.5f, Mass * Gravity * 0.5f, 0.0f, 0.0f, 0.0f, 0.0f);
        return;
    }

    const float SteerAngle = ComputeSteerAngleRadians(Speed);

    const FWheelBasis2D FrontBasis{ Rotate2D(BodyForward, SteerAngle), Rotate2D(BodyRight, SteerAngle) };
    const FWheelBasis2D RearBasis{ BodyForward, BodyRight };

    const FRaceCoreVector3 FrontRadius = BodyForward * Parameters.FrontAxleDistanceMeters;
    const FRaceCoreVector3 RearRadius = BodyForward * -Parameters.RearAxleDistanceMeters;

    const FRaceCoreVector3 FrontVelocity = State.VelocityMetersPerSecond + YawCrossRadius(State.YawAngularVelocityRadPerSecond, FrontRadius);
    const FRaceCoreVector3 RearVelocity = State.VelocityMetersPerSecond + YawCrossRadius(State.YawAngularVelocityRadPerSecond, RearRadius);

    const float FrontLongSpeed = static_cast<float>(Dot2D(FrontVelocity, FrontBasis.Forward));
    const float FrontLatSpeed = static_cast<float>(Dot2D(FrontVelocity, FrontBasis.Right));
    const float RearLongSpeed = static_cast<float>(Dot2D(RearVelocity, RearBasis.Forward));
    const float RearLatSpeed = static_cast<float>(Dot2D(RearVelocity, RearBasis.Right));

    const float SlipDenomFront = std::max(0.65f, std::abs(FrontLongSpeed));
    const float SlipDenomRear = std::max(0.65f, std::abs(RearLongSpeed));

    const float FrontSlipAngle = std::atan2(FrontLatSpeed, SlipDenomFront);
    const float RearSlipAngle = std::atan2(RearLatSpeed, SlipDenomRear);

    const float StaticFrontLoad = Mass * Gravity * (Parameters.RearAxleDistanceMeters / Parameters.WheelBaseMeters);
    const float StaticRearLoad = Mass * Gravity * (Parameters.FrontAxleDistanceMeters / Parameters.WheelBaseMeters);

    const float Downforce = Parameters.DownforceCoefficient * Speed * Speed;
    const float FrontDownforce = Downforce * (StaticFrontLoad / (Mass * Gravity));
    const float RearDownforce = Downforce - FrontDownforce;

    const float LongitudinalTransfer = Mass * LastLongitudinalAcceleration * Parameters.CenterOfMassHeightMeters / Parameters.WheelBaseMeters;

    float FrontLoad = StaticFrontLoad + FrontDownforce - LongitudinalTransfer;
    float RearLoad = StaticRearLoad + RearDownforce + LongitudinalTransfer;

    FrontLoad = std::max(StaticFrontLoad * 0.20f, FrontLoad);
    RearLoad = std::max(StaticRearLoad * 0.12f, RearLoad);

    const float LateralTransferRatio = ClampFloat(
        std::abs(LastLateralAcceleration) * Parameters.CenterOfMassHeightMeters /
        std::max(0.1f, Gravity * Parameters.TrackWidthMeters),
        0.0f,
        0.70f);

    const float RollGripLoss = ClampFloat(std::abs(State.RollRadians) / std::max(0.01f, Parameters.MaxVisualRollDegrees * DegToRad), 0.0f, 1.0f)
        * Parameters.RollGripSensitivity;

    FrontLoad *= (1.0f - LateralTransferRatio * 0.14f - RollGripLoss * 0.08f);
    RearLoad *= (1.0f - LateralTransferRatio * 0.24f - RollGripLoss * 0.15f);

    const float BrakeFrontBias = 0.66f;
    const float FrontBrakeN = ComputeBrakeForceForWheelSpeed(CurrentInput.Brake, Parameters.MaxBrakeForceN * BrakeFrontBias, FrontLongSpeed);
    const float RearBrakeServiceN = ComputeBrakeForceForWheelSpeed(CurrentInput.Brake, Parameters.MaxBrakeForceN * (1.0f - BrakeFrontBias), RearLongSpeed);
    const float RearHandbrakeN = ComputeBrakeForceForWheelSpeed(static_cast<float>(CurrentInput.Handbrake), Parameters.MaxHandbrakeForceN, RearLongSpeed);

    const float EngineForceN = ComputeEngineForce();
    const float EngineBrakingN = ComputeEngineBrakingForce();

    float FrontFx = FrontBrakeN;
    float RearFx = EngineForceN + RearBrakeServiceN + RearHandbrakeN + EngineBrakingN;

    const float FrontLongLimit = Parameters.LongitudinalMu * FrontLoad;
    const float RearLongMuScale = CurrentInput.Handbrake != 0 ? Parameters.HandbrakeRearLongitudinalMuScale : 1.0f;
    const float RearLongLimit = Parameters.LongitudinalMu * RearLongMuScale * RearLoad;

    FrontFx = ClampAbs(FrontFx, FrontLongLimit);
    RearFx = ClampAbs(RearFx, RearLongLimit);

    const float RearLateralMuScale = CurrentInput.Handbrake != 0 ? Parameters.HandbrakeRearLateralMuScale : 1.0f;

    float FrontMu = Parameters.FrontLateralMu;
    const float RearMu = Parameters.RearLateralMu * RearLateralMuScale;

    const float HighSpeedAlpha = ClampFloat((SpeedKmh - 80.0f) / 155.0f, 0.0f, 1.0f);
    FrontMu *= (1.0f - HighSpeedAlpha * Parameters.HighSpeedUndersteerStrength);

    const float LowSpeedAssistAlpha = ClampFloat(1.0f - (SpeedKmh / std::max(1.0f, Parameters.LowSpeedYawAssistMaxKmh)), 0.0f, 1.0f);
    const float AssistedFrontStiffness = Parameters.FrontCorneringStiffness * (1.0f + LowSpeedAssistAlpha * 0.62f);

    const float PeakSlipRad = Parameters.PeakSlipAngleDegrees * DegToRad;

    const float FrontFy = SaturatingTireLateralForce(
        FrontSlipAngle,
        AssistedFrontStiffness,
        FrontLoad,
        FrontMu,
        PeakSlipRad,
        Parameters.SlidingFrictionRatio,
        std::abs(FrontFx));

    const float RearFy = SaturatingTireLateralForce(
        RearSlipAngle,
        Parameters.RearCorneringStiffness,
        RearLoad,
        RearMu,
        PeakSlipRad,
        CurrentInput.Handbrake != 0 ? 0.50f : Parameters.SlidingFrictionRatio,
        std::abs(RearFx));

    const FRaceCoreVector3 FrontForce = ForceFromBasis(FrontBasis, FrontFx, FrontFy);
    const FRaceCoreVector3 RearForce = ForceFromBasis(RearBasis, RearFx, RearFy);

    FRaceCoreVector3 TotalForce = FrontForce + RearForce;

    Telemetry.RollingResistanceForceN = 0.0f;

    if (Speed > 0.05f)
    {
        const FRaceCoreVector3 VelocityDir = Normalize2D(State.VelocityMetersPerSecond);
        const float DragN = Parameters.DragCoefficient * Speed * Speed;
        const float RollingN =
            Parameters.RollingResistanceN +
            Parameters.RollingResistanceSpeedCoefficient * Speed +
            Parameters.RollingResistanceQuadraticCoefficient * Speed * Speed;

        TotalForce += VelocityDir * -static_cast<double>(DragN + RollingN);

        Telemetry.DragForceN = DragN;
        Telemetry.RollingResistanceForceN = RollingN;
    }
    else
    {
        Telemetry.DragForceN = 0.0f;
    }

    float YawTorque = static_cast<float>(Cross2D(FrontRadius, FrontForce) + Cross2D(RearRadius, RearForce));
    YawTorque += ComputeLowSpeedYawAssist(Speed, FixedDeltaSeconds);

    const float DriftYawTorque = ComputeDriftYawTorque(Speed, RearSlipAngle, PeakSlipRad);
    YawTorque += DriftYawTorque;

    const FRaceCoreVector3 Acceleration = TotalForce * (1.0 / static_cast<double>(Mass));
    const float LongitudinalAcceleration = static_cast<float>(Dot2D(Acceleration, BodyForward));
    const float LateralAcceleration = static_cast<float>(Dot2D(Acceleration, BodyRight));

    State.VelocityMetersPerSecond += Acceleration * FixedDeltaSeconds;

    const float YawAngularAcceleration = YawTorque / Parameters.YawInertiaKgM2;
    State.YawAngularVelocityRadPerSecond += YawAngularAcceleration * FixedDeltaSeconds;

    const float SteeringAbs = std::abs(CurrentInput.Steering);
    const float SpeedStability = ClampFloat(SpeedKmh / 180.0f, 0.0f, 1.0f);
    const float ActiveDampingScale =
        1.0f +
        (1.0f - SteeringAbs) * Parameters.ExtraReturnToCenterYawDamping +
        SpeedStability * 0.18f;

    State.YawAngularVelocityRadPerSecond *= std::max(0.0f, 1.0f - Parameters.YawDamping * ActiveDampingScale * FixedDeltaSeconds);

    if (Speed < Parameters.LowSpeedSteeringCutoff && CurrentInput.Throttle <= 0.001f)
    {
        State.YawAngularVelocityRadPerSecond *= 0.86f;
    }

    State.YawRadians += State.YawAngularVelocityRadPerSecond * FixedDeltaSeconds;

    State.PositionMeters.X += State.VelocityMetersPerSecond.X * FixedDeltaSeconds;
    State.PositionMeters.Y += State.VelocityMetersPerSecond.Y * FixedDeltaSeconds;
    State.VelocityMetersPerSecond.Z = 0.0;

    UpdateRideHeight(FixedDeltaSeconds);
    UpdateSuspensionTilt(FixedDeltaSeconds, LongitudinalAcceleration, LateralAcceleration);

    const FRaceCoreVector3 NewBodyForward(std::cos(State.YawRadians), std::sin(State.YawRadians), 0.0);
    State.ForwardSpeedMetersPerSecond = static_cast<float>(Dot2D(State.VelocityMetersPerSecond, NewBodyForward));

    const float WheelRpm = ComputeWheelRpmFromSpeed(std::abs(State.ForwardSpeedMetersPerSecond));
    const int32 GearIndex = std::max(1, std::min(State.CurrentGear, 7));
    State.EngineRpm = ClampFloat(
        WheelRpm * Parameters.GearRatios[GearIndex] * Parameters.FinalDriveRatio,
        Parameters.IdleRpm,
        Parameters.RedlineRpm);

    if (State.EngineDisabled != 0)
    {
        State.EngineRpm = 0.0f;
    }

    LastLongitudinalAcceleration = LongitudinalAcceleration;
    LastLateralAcceleration = LateralAcceleration;

    Telemetry.EngineForceN = EngineForceN;
    Telemetry.EngineBrakingForceN = std::abs(EngineBrakingN);
    Telemetry.BrakeForceN = std::abs(FrontBrakeN) + std::abs(RearBrakeServiceN) + std::abs(RearHandbrakeN);
    Telemetry.DownforceN = Downforce;

    UpdateTelemetryFromDynamics(
        FrontSlipAngle,
        RearSlipAngle,
        FrontFy,
        RearFy,
        FrontLoad,
        RearLoad,
        YawTorque,
        DriftYawTorque,
        LongitudinalAcceleration,
        LateralAcceleration);
}

void FRaceVehicleCore::UpdateSuspensionTilt(const float FixedDeltaSeconds, const float LongitudinalAcceleration, const float LateralAcceleration)
{
    const float LateralG = LateralAcceleration / Gravity;
    const float LongitudinalG = LongitudinalAcceleration / Gravity;

    const float MaxRollRad = Parameters.MaxVisualRollDegrees * DegToRad;
    const float MaxPitchRad = Parameters.MaxVisualPitchDegrees * DegToRad;

    const float TargetRoll = ClampFloat(-LateralG * Parameters.RollFromLateralG * DegToRad, -MaxRollRad, MaxRollRad);
    const float TargetPitch = ClampFloat(-LongitudinalG * Parameters.PitchFromLongitudinalG * DegToRad, -MaxPitchRad, MaxPitchRad);

    const float RollAccel = (TargetRoll - State.RollRadians) * Parameters.RollSpring
        - State.RollAngularVelocityRadPerSecond * Parameters.RollDamping;

    const float PitchAccel = (TargetPitch - State.PitchRadians) * Parameters.PitchSpring
        - State.PitchAngularVelocityRadPerSecond * Parameters.PitchDamping;

    State.RollAngularVelocityRadPerSecond += RollAccel * FixedDeltaSeconds;
    State.PitchAngularVelocityRadPerSecond += PitchAccel * FixedDeltaSeconds;

    State.RollRadians += State.RollAngularVelocityRadPerSecond * FixedDeltaSeconds;
    State.PitchRadians += State.PitchAngularVelocityRadPerSecond * FixedDeltaSeconds;

    State.RollRadians = ClampFloat(State.RollRadians, -MaxRollRad * 1.25f, MaxRollRad * 1.25f);
    State.PitchRadians = ClampFloat(State.PitchRadians, -MaxPitchRad * 1.25f, MaxPitchRad * 1.25f);
}

void FRaceVehicleCore::UpdateRideHeight(const float FixedDeltaSeconds)
{
    const float TargetZ = Parameters.GroundProbeHeightMeters + Parameters.TargetRideHeightMeters;
    const float CurrentZ = static_cast<float>(State.PositionMeters.Z);
    const float Error = TargetZ - CurrentZ;

    const float SpringAccel =
        Error * Parameters.SuspensionSpringStrength
        - State.VerticalVelocityMetersPerSecond * Parameters.SuspensionDamping;

    State.VerticalVelocityMetersPerSecond += SpringAccel * FixedDeltaSeconds;
    State.PositionMeters.Z += State.VerticalVelocityMetersPerSecond * FixedDeltaSeconds;

    const float MinZ = Parameters.GroundProbeHeightMeters + Parameters.TargetRideHeightMeters * 0.75f;
    if (State.PositionMeters.Z < MinZ)
    {
        State.PositionMeters.Z = MinZ;
        State.VerticalVelocityMetersPerSecond = std::max(0.0f, State.VerticalVelocityMetersPerSecond);
    }

    const float MaxReasonableZ = Parameters.GroundProbeHeightMeters + Parameters.TargetRideHeightMeters * 3.0f;
    if (State.PositionMeters.Z > MaxReasonableZ)
    {
        State.PositionMeters.Z = MaxReasonableZ;
        State.VerticalVelocityMetersPerSecond = std::min(0.0f, State.VerticalVelocityMetersPerSecond);
    }

    State.RideHeightMeters = static_cast<float>(State.PositionMeters.Z - Parameters.GroundProbeHeightMeters);
}

void FRaceVehicleCore::UpdateDamagePerformanceState(const float FixedDeltaSeconds)
{
    State.VehicleHealth = ClampFloat(State.VehicleHealth, 0.0f, 100.0f);

    if (State.VehicleHealth <= 0.0f)
    {
        State.EngineDisabled = 1;
        State.EngineDamage01 = 1.0f;
        State.SteeringDamage01 = std::max(State.SteeringDamage01, 0.85f);
        State.TransmissionDamage01 = std::max(State.TransmissionDamage01, 0.85f);
        State.MaxUsableGear = 1;
        return;
    }

    const float EngineHealthDamage = State.VehicleHealth < Parameters.EngineDamageStartsAtHealth
        ? 1.0f - State.VehicleHealth / std::max(1.0f, Parameters.EngineDamageStartsAtHealth)
        : 0.0f;

    const float SteeringHealthDamage = State.VehicleHealth < Parameters.SteeringDamageStartsAtHealth
        ? 1.0f - State.VehicleHealth / std::max(1.0f, Parameters.SteeringDamageStartsAtHealth)
        : 0.0f;

    const float TransmissionHealthDamage = State.VehicleHealth < Parameters.TransmissionDamageStartsAtHealth
        ? 1.0f - State.VehicleHealth / std::max(1.0f, Parameters.TransmissionDamageStartsAtHealth)
        : 0.0f;

    State.EngineDamage01 = ClampFloat(std::max(State.EngineDamage01, EngineHealthDamage), 0.0f, 1.0f);
    State.SteeringDamage01 = ClampFloat(std::max(State.SteeringDamage01, SteeringHealthDamage), 0.0f, 1.0f);
    State.TransmissionDamage01 = ClampFloat(std::max(State.TransmissionDamage01, TransmissionHealthDamage), 0.0f, 1.0f);
    State.MaxUsableGear = GetDamageLimitedMaxGear();

    if (FixedDeltaSeconds > 0.0f)
    {
        State.SteeringPullBias *= std::max(0.0f, 1.0f - FixedDeltaSeconds * 0.008f);
    }
}

void FRaceVehicleCore::ApplyFullStopIfNeeded()
{
    const float SpeedKmh = static_cast<float>(State.VelocityMetersPerSecond.Size2D()) * 3.6f;
    const bool bNoDriverInput =
        CurrentInput.Throttle <= 0.001f &&
        CurrentInput.Brake <= 0.001f &&
        std::abs(CurrentInput.Steering) <= 0.01f &&
        CurrentInput.Handbrake == 0;

    if (bNoDriverInput &&
        SpeedKmh <= Parameters.FullStopSpeedKmh &&
        std::abs(State.YawAngularVelocityRadPerSecond) <= 0.04f)
    {
        State.VelocityMetersPerSecond = FRaceCoreVector3(0.0, 0.0, 0.0);
        State.ForwardSpeedMetersPerSecond = 0.0f;
        State.YawAngularVelocityRadPerSecond = 0.0f;
        State.VerticalVelocityMetersPerSecond = 0.0f;
        State.EngineRpm = State.EngineDisabled != 0 ? 0.0f : Parameters.IdleRpm;

        State.PositionMeters.Z = Parameters.GroundProbeHeightMeters + Parameters.TargetRideHeightMeters;
        State.RideHeightMeters = Parameters.TargetRideHeightMeters;
        State.LockedPositionMeters = State.PositionMeters;
        State.bPositionLocked = 1;
    }
}

bool FRaceVehicleCore::MaintainPositionLockIfNeeded()
{
    if (State.bPositionLocked == 0)
    {
        return false;
    }

    if (HasWakeInput())
    {
        State.bPositionLocked = 0;
        return false;
    }

    State.PositionMeters = State.LockedPositionMeters;
    State.PositionMeters.Z = Parameters.GroundProbeHeightMeters + Parameters.TargetRideHeightMeters;
    State.RideHeightMeters = Parameters.TargetRideHeightMeters;
    State.VelocityMetersPerSecond = FRaceCoreVector3(0.0, 0.0, 0.0);
    State.ForwardSpeedMetersPerSecond = 0.0f;
    State.YawAngularVelocityRadPerSecond = 0.0f;
    State.VerticalVelocityMetersPerSecond = 0.0f;
    State.EngineRpm = State.EngineDisabled != 0 ? 0.0f : Parameters.IdleRpm;

    UpdateTelemetryFromDynamics(0.0f, 0.0f, 0.0f, 0.0f, Parameters.MassKg * Gravity * 0.5f, Parameters.MassKg * Gravity * 0.5f, 0.0f, 0.0f, 0.0f, 0.0f);
    return true;
}

bool FRaceVehicleCore::HasWakeInput() const
{
    return CurrentInput.Throttle > 0.02f ||
        CurrentInput.Brake > 0.02f ||
        std::abs(CurrentInput.Steering) > 0.03f ||
        CurrentInput.Handbrake != 0;
}

void FRaceVehicleCore::ApplyPostIntegrationDamping(const float FixedDeltaSeconds)
{
    const FRaceCoreVector3 BodyForward(std::cos(State.YawRadians), std::sin(State.YawRadians), 0.0);
    const FRaceCoreVector3 BodyRight(-std::sin(State.YawRadians), std::cos(State.YawRadians), 0.0);

    const double ForwardSpeed = Dot2D(State.VelocityMetersPerSecond, BodyForward);
    const double LateralSpeed = Dot2D(State.VelocityMetersPerSecond, BodyRight);

    const float SteeringAbs = std::abs(CurrentInput.Steering);
    const float SteeringDampingScale = ClampFloat(1.0f - SteeringAbs * 0.55f, 0.32f, 1.0f);
    const float NoThrottleDamping = CurrentInput.Throttle <= 0.001f ? Parameters.NoThrottleExtraDamping : 0.0f;
    const float HandbrakeDampingScale = CurrentInput.Handbrake != 0 ? 0.18f : 1.0f;

    const float Damping = (Parameters.LateralVelocityDamping + NoThrottleDamping) * SteeringDampingScale * HandbrakeDampingScale;
    const double NewLateralSpeed = LateralSpeed * std::exp(-static_cast<double>(Damping) * FixedDeltaSeconds);

    State.VelocityMetersPerSecond = BodyForward * ForwardSpeed + BodyRight * NewLateralSpeed;
}

void FRaceVehicleCore::UpdateTelemetryFromDynamics(
    const float FrontSlipAngleRad,
    const float RearSlipAngleRad,
    const float FrontLateralForceN,
    const float RearLateralForceN,
    const float FrontNormalLoadN,
    const float RearNormalLoadN,
    const float YawTorqueNm,
    const float DriftYawTorqueNm,
    const float LongitudinalAcceleration,
    const float LateralAcceleration)
{
    const FRaceCoreVector3 BodyRight(-std::sin(State.YawRadians), std::cos(State.YawRadians), 0.0);
    const float BodyLateralSpeed = static_cast<float>(Dot2D(State.VelocityMetersPerSecond, BodyRight));

    Telemetry.FrontLateralForceN = FrontLateralForceN;
    Telemetry.RearLateralForceN = RearLateralForceN;
    Telemetry.FrontSlipAngleDeg = FrontSlipAngleRad * RadToDeg;
    Telemetry.RearSlipAngleDeg = RearSlipAngleRad * RadToDeg;
    Telemetry.FrontNormalLoadN = FrontNormalLoadN;
    Telemetry.RearNormalLoadN = RearNormalLoadN;
    Telemetry.YawTorqueNm = YawTorqueNm;
    Telemetry.DriftYawTorqueNm = DriftYawTorqueNm;
    Telemetry.YawAngularVelocity = State.YawAngularVelocityRadPerSecond;

    Telemetry.RollRadians = State.RollRadians;
    Telemetry.PitchRadians = State.PitchRadians;
    Telemetry.RollDegrees = State.RollRadians * RadToDeg;
    Telemetry.PitchDegrees = State.PitchRadians * RadToDeg;
    Telemetry.RideHeightMeters = State.RideHeightMeters;
    Telemetry.VerticalVelocityMetersPerSecond = State.VerticalVelocityMetersPerSecond;
    Telemetry.LateralG = LateralAcceleration / Gravity;
    Telemetry.LongitudinalG = LongitudinalAcceleration / Gravity;

    Telemetry.VehicleHealth = State.VehicleHealth;
    Telemetry.EngineDamage01 = State.EngineDamage01;
    Telemetry.SteeringDamage01 = State.SteeringDamage01;
    Telemetry.TransmissionDamage01 = State.TransmissionDamage01;
    Telemetry.SteeringPullBias = State.SteeringPullBias;
    Telemetry.DamageFront01 = State.DamageFront01;
    Telemetry.DamageRear01 = State.DamageRear01;
    Telemetry.DamageLeft01 = State.DamageLeft01;
    Telemetry.DamageRight01 = State.DamageRight01;
    Telemetry.ImpactShake = State.ImpactShake;
    Telemetry.EngineDisabled = State.EngineDisabled;
    Telemetry.bPositionLocked = State.bPositionLocked;

    Telemetry.LateralSpeed = std::abs(BodyLateralSpeed);
    Telemetry.NormalizedSpeed = ClampFloat(std::abs(State.ForwardSpeedMetersPerSecond) / 90.0f, 0.0f, 1.0f);

    const float FrontSlipScore = std::abs(FrontSlipAngleRad) / std::max(0.01f, Parameters.PeakSlipAngleDegrees * DegToRad);
    const float RearSlipScore = std::abs(RearSlipAngleRad) / std::max(0.01f, Parameters.PeakSlipAngleDegrees * DegToRad);
    const float HandbrakeBonus = CurrentInput.Handbrake != 0 ? 0.42f : 0.0f;
    const float LatAccelScore = std::abs(LateralAcceleration) / (Gravity * 0.95f);

    Telemetry.SkidAmount = ClampFloat(
        0.22f * FrontSlipScore +
        0.55f * RearSlipScore +
        0.23f * LatAccelScore +
        HandbrakeBonus,
        0.0f,
        1.0f);

    Telemetry.EnginePitch = State.EngineDisabled != 0 ? 0.0f : ClampFloat(State.EngineRpm / 3500.0f, 0.6f, 2.2f);
}

float FRaceVehicleCore::ComputeEngineForce() const
{
    if (State.EngineDisabled != 0 || CurrentInput.Throttle <= 0.0f)
    {
        return 0.0f;
    }

    const int32 GearIndex = std::max(1, std::min(State.CurrentGear, 7));
    const float GearTorqueScale = ClampFloat(Parameters.GearRatios[GearIndex] / Parameters.GearRatios[1], 0.25f, 1.15f);
    const float RpmPowerScale = 1.0f - ClampFloat((State.EngineRpm - Parameters.IdleRpm) / (Parameters.RedlineRpm - Parameters.IdleRpm), 0.0f, 1.0f) * 0.20f;

    return CurrentInput.Throttle * Parameters.MaxEngineForceN * GearTorqueScale * RpmPowerScale * GetEnginePowerScale();
}

float FRaceVehicleCore::ComputeEngineBrakingForce() const
{
    if (CurrentInput.Throttle > 0.001f || std::abs(State.ForwardSpeedMetersPerSecond) < 0.05f)
    {
        return 0.0f;
    }

    const int32 GearIndex = std::max(1, std::min(State.CurrentGear, 7));
    const float GearRatio = Parameters.GearRatios[GearIndex] * Parameters.FinalDriveRatio;
    const float RpmFactor = ClampFloat((State.EngineRpm - Parameters.IdleRpm) / std::max(1.0f, Parameters.RedlineRpm - Parameters.IdleRpm), 0.0f, 1.0f);
    const float GearFactor = ClampFloat(GearRatio / (Parameters.GearRatios[1] * Parameters.FinalDriveRatio), 0.25f, 1.0f);
    const float SpeedFactor = ClampFloat(std::abs(State.ForwardSpeedMetersPerSecond) / 22.0f, 0.35f, 1.35f);

    const float EngineBrakeN = Parameters.EngineBrakingCoefficient * Parameters.MaxEngineForceN * (0.35f + RpmFactor * 1.15f) * GearFactor * SpeedFactor;
    return -SignNonZero(State.ForwardSpeedMetersPerSecond) * EngineBrakeN;
}

float FRaceVehicleCore::ComputeWheelRpmFromSpeed(const float ForwardSpeedMetersPerSecond) const
{
    const float Circumference = 2.0f * Pi * Parameters.WheelRadiusMeters;
    if (Circumference <= 0.001f)
    {
        return Parameters.IdleRpm;
    }

    return (ForwardSpeedMetersPerSecond / Circumference) * 60.0f;
}

float FRaceVehicleCore::ComputeSteerAngleRadians(const float SpeedAbs) const
{
    const float SpeedKmh = SpeedAbs * 3.6f;
    const float LowSpeedAlpha = ClampFloat(1.0f - SpeedKmh / std::max(1.0f, Parameters.LowSpeedYawAssistMaxKmh), 0.0f, 1.0f);

    const float MaxSteerDeg = Parameters.MaxSteerAngleDegrees +
        (Parameters.LowSpeedMaxSteerAngleDegrees - Parameters.MaxSteerAngleDegrees) * LowSpeedAlpha;

    const float MaxSteerRad = MaxSteerDeg * DegToRad;

    const float SpeedAlpha = ClampFloat(SpeedAbs / 68.0f, 0.0f, 1.0f);
    const float Sensitivity = 1.0f / (1.0f + SpeedAlpha * Parameters.SteeringSpeedSensitivity * 3.0f);

    const float LowSpeedScale = ClampFloat(SpeedAbs / std::max(0.1f, Parameters.LowSpeedSteeringCutoff), 0.42f, 1.0f);
    const float DamagedSteer = ClampFloat(CurrentInput.Steering + State.SteeringPullBias, -1.0f, 1.0f) * GetSteeringScale();

    return DamagedSteer * MaxSteerRad * Sensitivity * LowSpeedScale;
}

float FRaceVehicleCore::ComputeLowSpeedYawAssist(const float SpeedAbs, const float FixedDeltaSeconds) const
{
    (void)FixedDeltaSeconds;

    const float SpeedKmh = SpeedAbs * 3.6f;
    const float AssistAlpha = ClampFloat(1.0f - SpeedKmh / std::max(1.0f, Parameters.LowSpeedYawAssistMaxKmh), 0.0f, 1.0f);

    if (AssistAlpha <= 0.0f)
    {
        return 0.0f;
    }

    const float DamagedSteer = ClampFloat(CurrentInput.Steering + State.SteeringPullBias, -1.0f, 1.0f) * GetSteeringScale();
    if (std::abs(DamagedSteer) <= 0.001f)
    {
        return 0.0f;
    }

    const float SpeedFactor = ClampFloat(SpeedAbs / 12.0f, 0.38f, 1.0f);
    return DamagedSteer * AssistAlpha * SpeedFactor * Parameters.LowSpeedYawAssistGain * Parameters.YawInertiaKgM2;
}

float FRaceVehicleCore::ComputeDriftYawTorque(const float SpeedAbs, const float RearSlipAngleRad, const float PeakSlipRad) const
{
    const float DriftSpeedAlpha = ClampFloat(SpeedAbs / 24.0f, 0.0f, 1.0f);
    const float RearSlipScore = ClampFloat(std::abs(RearSlipAngleRad) / std::max(0.01f, PeakSlipRad), 0.0f, 2.0f);

    float DriftYawTorque = 0.0f;

    if (CurrentInput.Handbrake != 0 && SpeedAbs > Parameters.HandbrakeYawKickSpeedThreshold)
    {
        const float Direction = std::abs(CurrentInput.Steering) > 0.05f
            ? CurrentInput.Steering
            : SafeSign(State.YawAngularVelocityRadPerSecond, 0.02f);

        DriftYawTorque += Direction
            * Parameters.HandbrakeYawKickTorqueNm
            * DriftSpeedAlpha;
    }

    if (RearSlipScore > 1.0f && SpeedAbs > 5.0f)
    {
        const float SlipDirection = SafeSign(RearSlipAngleRad, 0.01f);
        DriftYawTorque += -SlipDirection
            * Parameters.RearSlipYawTorqueScale
            * (RearSlipScore - 1.0f)
            * DriftSpeedAlpha;
    }

    return ClampFloat(DriftYawTorque, -Parameters.MaxDriftYawTorqueNm, Parameters.MaxDriftYawTorqueNm);
}

float FRaceVehicleCore::GetEnginePowerScale() const
{
    if (State.EngineDisabled != 0)
    {
        return 0.0f;
    }

    if (State.VehicleHealth > Parameters.EngineDamageStartsAtHealth && State.EngineDamage01 <= 0.001f)
    {
        return 1.0f;
    }

    const float DamageScale = ClampFloat(State.EngineDamage01, 0.0f, 1.0f);
    return ClampFloat(1.0f - DamageScale * (1.0f - Parameters.MinimumEnginePowerScale), Parameters.MinimumEnginePowerScale, 1.0f);
}

float FRaceVehicleCore::GetSteeringScale() const
{
    const float DamageScale = ClampFloat(State.SteeringDamage01, 0.0f, 1.0f);
    return ClampFloat(1.0f - DamageScale * (1.0f - Parameters.MinimumSteeringScale), Parameters.MinimumSteeringScale, 1.0f);
}

int32 FRaceVehicleCore::GetDamageLimitedMaxGear() const
{
    if (State.EngineDisabled != 0)
    {
        return 1;
    }

    if (State.TransmissionDamage01 >= 0.82f)
    {
        return 3;
    }

    if (State.TransmissionDamage01 >= 0.55f)
    {
        return 4;
    }

    if (State.TransmissionDamage01 >= 0.32f)
    {
        return 5;
    }

    return 7;
}

float FRaceVehicleCore::GetShiftDelaySeconds() const
{
    return Parameters.BaseShiftDelaySeconds +
        (Parameters.DamagedShiftDelaySeconds - Parameters.BaseShiftDelaySeconds) *
        ClampFloat(State.TransmissionDamage01, 0.0f, 1.0f);
}

FRaceVehicleStatePacket FRaceVehicleCore::BuildStatePacket(const uint64 SequenceId, const uint32 PlayerId) const
{
    FRaceVehicleStatePacket Packet{};
    Packet.Header.SequenceId = SequenceId;

    Packet.PositionX = static_cast<float>(State.PositionMeters.X);
    Packet.PositionY = static_cast<float>(State.PositionMeters.Y);
    Packet.PositionZ = static_cast<float>(State.PositionMeters.Z);

    Packet.VelocityX = static_cast<float>(State.VelocityMetersPerSecond.X);
    Packet.VelocityY = static_cast<float>(State.VelocityMetersPerSecond.Y);
    Packet.VelocityZ = static_cast<float>(State.VelocityMetersPerSecond.Z);

    Packet.YawRadians = State.YawRadians;
    Packet.ForwardSpeed = State.ForwardSpeedMetersPerSecond;
    Packet.EngineRpm = State.EngineRpm;

    Packet.CurrentGear = State.CurrentGear;
    Packet.LapIndex = State.LapIndex;
    Packet.CheckpointIndex = State.CheckpointIndex;

    Packet.Grounded = State.Grounded;
    Packet.TransmissionMode = static_cast<uint8>(Parameters.TransmissionMode);
    Packet.VehicleClass = static_cast<uint8>(Parameters.VehicleClass);
    Packet.PlayerId = PlayerId;

    RaceBridge::FinalizeVehicleStatePacket(Packet);
    return Packet;
}

const FRaceVehicleRuntimeState& FRaceVehicleCore::GetRuntimeState() const
{
    return State;
}

const FRaceVehicleDebugTelemetry& FRaceVehicleCore::GetDebugTelemetry() const
{
    return Telemetry;
}

const FRaceVehicleParameters& FRaceVehicleCore::GetParameters() const
{
    return Parameters;
}

void FRaceVehicleCore::SetLapState(const int32 InLapIndex, const int32 InCheckpointIndex)
{
    State.LapIndex = InLapIndex;
    State.CheckpointIndex = InCheckpointIndex;
}

float FRaceVehicleCore::ClampFloat(const float Value, const float MinValue, const float MaxValue)
{
    return std::max(MinValue, std::min(Value, MaxValue));
}

float FRaceVehicleCore::SignNonZero(const float Value)
{
    return Value < 0.0f ? -1.0f : 1.0f;
}

float FRaceVehicleCore::SafeSign(const float Value, const float DeadZone)
{
    if (std::abs(Value) <= DeadZone)
    {
        return 0.0f;
    }

    return Value < 0.0f ? -1.0f : 1.0f;
}

