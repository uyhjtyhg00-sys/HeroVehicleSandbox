#pragma once

#include "CoreTypes.h"
#include <cstddef>
#include <cstring>

namespace RaceBridge
{
    constexpr uint32 Magic = 0x45434152u; // ASCII little-endian "RACE"
    constexpr uint16 Version = 1;
    constexpr uint32 HeaderSize = 24;

    constexpr uint32 LobbyConfigPacketSize = 144;
    constexpr uint32 VehicleInputPacketSize = 56;
    constexpr uint32 VehicleStatePacketSize = 88;
    constexpr uint32 TrackConfigPacketSize = 64;

    constexpr uint32 FixedNameBytes = 32;
    constexpr uint32 FixedVehicleIdBytes = 32;

    inline bool IsLittleEndian()
    {
        const uint16 Value = 1;
        return *reinterpret_cast<const uint8*>(&Value) == 1;
    }

    inline uint32 ComputeChecksum(const void* Data, const SIZE_T Length)
    {
        const uint8* Bytes = reinterpret_cast<const uint8*>(Data);
        uint32 Hash = 2166136261u;

        for (SIZE_T Index = 0; Index < Length; ++Index)
        {
            Hash ^= static_cast<uint32>(Bytes[Index]);
            Hash *= 16777619u;
        }

        return Hash;
    }
}

enum class ERacePacketType : uint16
{
    None = 0,
    LobbyConfig = 1,
    VehicleInput = 2,
    VehicleState = 3,
    TrackConfig = 4
};

enum class ERaceTransmissionMode : uint8
{
    Automatic = 0,
    Manual = 1,
    SemiAutomatic = 2
};

enum class ERaceVehicleClass : uint8
{
    Compact = 0,
    Sports = 1,
    Formula = 2,
    Truck = 3
};

enum class ERaceInputFlags : uint16
{
    None = 0,
    Handbrake = 1 << 0,
    Boost = 1 << 1,
    ResetVehicle = 1 << 2
};

#pragma pack(push, 8)

struct alignas(8) FRacePacketHeader
{
    uint32 Magic;
    uint16 Version;
    uint16 PacketType;
    uint32 PacketSize;
    uint32 HeaderSize;
    uint64 SequenceId;
};

struct alignas(8) FRaceLobbyConfigPacket
{
    FRacePacketHeader Header;

    uint32 TrackSeed;
    uint32 SegmentCount;
    uint32 CheckpointCount;
    uint32 LapCount;

    float TrackWidthMeters;
    float MinTrackRadiusMeters;
    float MaxTrackRadiusMeters;
    float BankingAmount;
    float AiDifficulty;

    uint8 TransmissionMode;
    uint8 VehicleClass;
    uint8 EnableAbs;
    uint8 EnableTcs;
    uint8 MaxPlayers;
    uint8 Reserved0[7];

    uint8 PlayerNameUtf8[RaceBridge::FixedNameBytes];
    uint8 VehicleIdUtf8[RaceBridge::FixedVehicleIdBytes];

    uint32 PayloadCrc32;
    uint32 Reserved1;
};

struct alignas(8) FRaceVehicleInputPacket
{
    FRacePacketHeader Header;

    float Throttle;
    float Brake;
    float Steering;

    uint8 Handbrake;
    int8 GearRequest;
    uint16 InputFlags;

    uint32 PlayerId;
    float ClientDeltaSeconds;
    uint32 InputFrame;

    uint32 PayloadCrc32;
};

struct alignas(8) FRaceVehicleStatePacket
{
    FRacePacketHeader Header;

    float PositionX;
    float PositionY;
    float PositionZ;

    float VelocityX;
    float VelocityY;
    float VelocityZ;

    float YawRadians;
    float ForwardSpeed;
    float EngineRpm;

    int32 CurrentGear;
    int32 LapIndex;
    int32 CheckpointIndex;

    uint8 Grounded;
    uint8 TransmissionMode;
    uint8 VehicleClass;
    uint8 Reserved0[5];

    uint32 PlayerId;
    uint32 PayloadCrc32;
};

struct alignas(8) FRaceTrackConfigPacket
{
    FRacePacketHeader Header;

    uint32 TrackSeed;
    uint32 SegmentCount;
    uint32 CheckpointCount;
    uint32 Reserved0;

    float TrackWidthMeters;
    float MinTrackRadiusMeters;
    float MaxTrackRadiusMeters;
    float BankingAmount;
    float StartYawDegrees;

    uint32 PayloadCrc32;
};

#pragma pack(pop)

static_assert(sizeof(FRacePacketHeader) == RaceBridge::HeaderSize, "FRacePacketHeader size mismatch");
static_assert(sizeof(FRaceLobbyConfigPacket) == RaceBridge::LobbyConfigPacketSize, "FRaceLobbyConfigPacket size mismatch");
static_assert(sizeof(FRaceVehicleInputPacket) == RaceBridge::VehicleInputPacketSize, "FRaceVehicleInputPacket size mismatch");
static_assert(sizeof(FRaceVehicleStatePacket) == RaceBridge::VehicleStatePacketSize, "FRaceVehicleStatePacket size mismatch");
static_assert(sizeof(FRaceTrackConfigPacket) == RaceBridge::TrackConfigPacketSize, "FRaceTrackConfigPacket size mismatch");

static_assert(alignof(FRacePacketHeader) == 8, "FRacePacketHeader alignment mismatch");
static_assert(alignof(FRaceLobbyConfigPacket) == 8, "FRaceLobbyConfigPacket alignment mismatch");
static_assert(alignof(FRaceVehicleInputPacket) == 8, "FRaceVehicleInputPacket alignment mismatch");
static_assert(alignof(FRaceVehicleStatePacket) == 8, "FRaceVehicleStatePacket alignment mismatch");
static_assert(alignof(FRaceTrackConfigPacket) == 8, "FRaceTrackConfigPacket alignment mismatch");

namespace RaceBridge
{
    inline FRacePacketHeader MakeHeader(const ERacePacketType Type, const uint32 PacketSize, const uint64 SequenceId)
    {
        FRacePacketHeader Header{};
        Header.Magic = Magic;
        Header.Version = Version;
        Header.PacketType = static_cast<uint16>(Type);
        Header.PacketSize = PacketSize;
        Header.HeaderSize = HeaderSize;
        Header.SequenceId = SequenceId;
        return Header;
    }

    inline bool ValidateHeader(const FRacePacketHeader& Header, const ERacePacketType ExpectedType, const uint32 ExpectedSize)
    {
        return IsLittleEndian()
            && Header.Magic == Magic
            && Header.Version == Version
            && Header.PacketType == static_cast<uint16>(ExpectedType)
            && Header.PacketSize == ExpectedSize
            && Header.HeaderSize == HeaderSize;
    }

    inline void FinalizeLobbyPacket(FRaceLobbyConfigPacket& Packet)
    {
        Packet.Header = MakeHeader(ERacePacketType::LobbyConfig, LobbyConfigPacketSize, Packet.Header.SequenceId);
        Packet.PayloadCrc32 = ComputeChecksum(&Packet, offsetof(FRaceLobbyConfigPacket, PayloadCrc32));
    }

    inline void FinalizeVehicleInputPacket(FRaceVehicleInputPacket& Packet)
    {
        Packet.Header = MakeHeader(ERacePacketType::VehicleInput, VehicleInputPacketSize, Packet.Header.SequenceId);
        Packet.PayloadCrc32 = ComputeChecksum(&Packet, offsetof(FRaceVehicleInputPacket, PayloadCrc32));
    }

    inline void FinalizeVehicleStatePacket(FRaceVehicleStatePacket& Packet)
    {
        Packet.Header = MakeHeader(ERacePacketType::VehicleState, VehicleStatePacketSize, Packet.Header.SequenceId);
        Packet.PayloadCrc32 = ComputeChecksum(&Packet, offsetof(FRaceVehicleStatePacket, PayloadCrc32));
    }

    inline void FinalizeTrackConfigPacket(FRaceTrackConfigPacket& Packet)
    {
        Packet.Header = MakeHeader(ERacePacketType::TrackConfig, TrackConfigPacketSize, Packet.Header.SequenceId);
        Packet.PayloadCrc32 = ComputeChecksum(&Packet, offsetof(FRaceTrackConfigPacket, PayloadCrc32));
    }

    inline bool ValidateLobbyPacket(const FRaceLobbyConfigPacket& Packet)
    {
        if (!ValidateHeader(Packet.Header, ERacePacketType::LobbyConfig, LobbyConfigPacketSize))
        {
            return false;
        }

        const uint32 Computed = ComputeChecksum(&Packet, offsetof(FRaceLobbyConfigPacket, PayloadCrc32));
        return Computed == Packet.PayloadCrc32;
    }

    inline bool ValidateVehicleInputPacket(const FRaceVehicleInputPacket& Packet)
    {
        if (!ValidateHeader(Packet.Header, ERacePacketType::VehicleInput, VehicleInputPacketSize))
        {
            return false;
        }

        const uint32 Computed = ComputeChecksum(&Packet, offsetof(FRaceVehicleInputPacket, PayloadCrc32));
        return Computed == Packet.PayloadCrc32;
    }

    inline bool ValidateVehicleStatePacket(const FRaceVehicleStatePacket& Packet)
    {
        if (!ValidateHeader(Packet.Header, ERacePacketType::VehicleState, VehicleStatePacketSize))
        {
            return false;
        }

        const uint32 Computed = ComputeChecksum(&Packet, offsetof(FRaceVehicleStatePacket, PayloadCrc32));
        return Computed == Packet.PayloadCrc32;
    }

    inline bool ValidateTrackConfigPacket(const FRaceTrackConfigPacket& Packet)
    {
        if (!ValidateHeader(Packet.Header, ERacePacketType::TrackConfig, TrackConfigPacketSize))
        {
            return false;
        }

        const uint32 Computed = ComputeChecksum(&Packet, offsetof(FRaceTrackConfigPacket, PayloadCrc32));
        return Computed == Packet.PayloadCrc32;
    }
}


// Full-game bridge extension. Kept POD/blittable and separate from UE reflection.
enum class ERaceGamePhase : uint8
{
    Boot = 0,
    MainMenu = 1,
    WaitingForLobby = 2,
    Countdown = 3,
    Racing = 4,
    Paused = 5,
    Finished = 6
};

#pragma pack(push, 8)
struct alignas(8) FRaceRaceResultPacket
{
    FRacePacketHeader Header;
    uint32 PlayerId;
    float FinishTimeSeconds;
    float BestLapSeconds;
    uint32 TotalLaps;
    uint8 Finished;
    uint8 Reserved0[7];
    uint32 PayloadCrc32;
    uint32 Reserved1;
};
#pragma pack(pop)

static_assert(sizeof(FRaceRaceResultPacket) == 56, "FRaceRaceResultPacket size mismatch");
static_assert(alignof(FRaceRaceResultPacket) == 8, "FRaceRaceResultPacket alignment mismatch");

namespace RaceBridge
{
    constexpr uint32 RaceResultPacketSize = 56;

    inline void FinalizeRaceResultPacket(FRaceRaceResultPacket& Packet)
    {
        Packet.Header = MakeHeader(ERacePacketType::None, RaceResultPacketSize, Packet.Header.SequenceId);
        Packet.Header.PacketType = 5;
        Packet.PayloadCrc32 = ComputeChecksum(&Packet, offsetof(FRaceRaceResultPacket, PayloadCrc32));
    }

    inline bool ValidateRaceResultPacket(const FRaceRaceResultPacket& Packet)
    {
        if (!IsLittleEndian()
            || Packet.Header.Magic != Magic
            || Packet.Header.Version != Version
            || Packet.Header.PacketType != 5
            || Packet.Header.PacketSize != RaceResultPacketSize
            || Packet.Header.HeaderSize != HeaderSize)
        {
            return false;
        }
        return ComputeChecksum(&Packet, offsetof(FRaceRaceResultPacket, PayloadCrc32)) == Packet.PayloadCrc32;
    }
}
