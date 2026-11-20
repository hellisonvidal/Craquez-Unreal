// Copyright 2024 Craquez. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "FootballCoreTypes.generated.h"

// Forward declarations
class UFootballBallComponent;
class AFootballPlayerCharacter;
class AFootballTeam;
class UFootballTacticalComponent;

/**
 * Gameplay Tags for Football
 */
namespace FFootballTags
{
// Game States
FOOTBALLCORE_API extern FGameplayTag Game_Active;
FOOTBALLCORE_API extern FGameplayTag Game_Paused;
FOOTBALLCORE_API extern FGameplayTag Game_HalfTime;
FOOTBALLCORE_API extern FGameplayTag Game_Finished;
FOOTBALLCORE_API extern FGameplayTag Game_SetPiece;

// Ball States
FOOTBALLCORE_API extern FGameplayTag Ball_Free;
FOOTBALLCORE_API extern FGameplayTag Ball_Possessed;
FOOTBALLCORE_API extern FGameplayTag Ball_Flying;
FOOTBALLCORE_API extern FGameplayTag Ball_Rolling;
FOOTBALLCORE_API extern FGameplayTag Ball_OutOfPlay;
FOOTBALLCORE_API extern FGameplayTag Ball_Goal;

// Player States
FOOTBALLCORE_API extern FGameplayTag Player_Idle;
FOOTBALLCORE_API extern FGameplayTag Player_Running;
FOOTBALLCORE_API extern FGameplayTag Player_Sprinting;
FOOTBALLCORE_API extern FGameplayTag Player_Dribbling;
FOOTBALLCORE_API extern FGameplayTag Player_Passing;
FOOTBALLCORE_API extern FGameplayTag Player_Shooting;
FOOTBALLCORE_API extern FGameplayTag Player_Tackling;
FOOTBALLCORE_API extern FGameplayTag Player_Intercepting;
FOOTBALLCORE_API extern FGameplayTag Player_Goalkeeping;
FOOTBALLCORE_API extern FGameplayTag Player_Celebrating;
FOOTBALLCORE_API extern FGameplayTag Player_Injured;
FOOTBALLCORE_API extern FGameplayTag Player_Fatigued;

// Team States
FOOTBALLCORE_API extern FGameplayTag Team_Attacking;
FOOTBALLCORE_API extern FGameplayTag Team_Defending;
FOOTBALLCORE_API extern FGameplayTag Team_Pressing;
FOOTBALLCORE_API extern FGameplayTag Team_CounterAttacking;
FOOTBALLCORE_API extern FGameplayTag Team_Possession;
FOOTBALLCORE_API extern FGameplayTag Team_SetPieceAttack;
FOOTBALLCORE_API extern FGameplayTag Team_SetPieceDefense;

// Actions
FOOTBALLCORE_API extern FGameplayTag Action_Pass_Short;
FOOTBALLCORE_API extern FGameplayTag Action_Pass_Long;
FOOTBALLCORE_API extern FGameplayTag Action_Pass_Through;
FOOTBALLCORE_API extern FGameplayTag Action_Pass_Cross;
FOOTBALLCORE_API extern FGameplayTag Action_Shoot_Power;
FOOTBALLCORE_API extern FGameplayTag Action_Shoot_Placement;
FOOTBALLCORE_API extern FGameplayTag Action_Shoot_Chip;
FOOTBALLCORE_API extern FGameplayTag Action_Shoot_Volley;
FOOTBALLCORE_API extern FGameplayTag Action_Dribble;
FOOTBALLCORE_API extern FGameplayTag Action_SkillMove;
FOOTBALLCORE_API extern FGameplayTag Action_Tackle_Standing;
FOOTBALLCORE_API extern FGameplayTag Action_Tackle_Sliding;
FOOTBALLCORE_API extern FGameplayTag Action_Intercept;
FOOTBALLCORE_API extern FGameplayTag Action_Block;
FOOTBALLCORE_API extern FGameplayTag Action_Clearance;
FOOTBALLCORE_API extern FGameplayTag Action_Header;
FOOTBALLCORE_API extern FGameplayTag Action_Goalkeeper_Save;
FOOTBALLCORE_API extern FGameplayTag Action_Goalkeeper_Catch;
FOOTBALLCORE_API extern FGameplayTag Action_Goalkeeper_Punch;
FOOTBALLCORE_API extern FGameplayTag Action_Goalkeeper_Distribute_Throw;
FOOTBALLCORE_API extern FGameplayTag Action_Goalkeeper_Distribute_Kick;

// Body Parts
FOOTBALLCORE_API extern FGameplayTag Body_LeftFoot;
FOOTBALLCORE_API extern FGameplayTag Body_RightFoot;
FOOTBALLCORE_API extern FGameplayTag Body_Head;
FOOTBALLCORE_API extern FGameplayTag Body_Chest;
FOOTBALLCORE_API extern FGameplayTag Body_LeftThigh;
FOOTBALLCORE_API extern FGameplayTag Body_RightThigh;
FOOTBALLCORE_API extern FGameplayTag Body_LeftShoulder;
FOOTBALLCORE_API extern FGameplayTag Body_RightShoulder;
} // namespace FFootballTags

/**
 * Football specific enums
 */
UENUM(BlueprintType)
namespace EFootballGameMode
{
enum Type
{
    TrainingMatch_11v11 UMETA(DisplayName = "Training Match 11v11"),
    OneVOne_StrikerVsKeeper UMETA(DisplayName = "1v1 Striker vs Keeper"),
    PenaltyShootout UMETA(DisplayName = "Penalty Shootout"),
    FreeKick_Practice UMETA(DisplayName = "Free Kick Practice"),
    Custom UMETA(DisplayName = "Custom")
};
}

UENUM(BlueprintType)
namespace EFootballBallState
{
enum Type
{
    Free UMETA(DisplayName = "Free"),
    Possessed UMETA(DisplayName = "Possessed"),
    Flying UMETA(DisplayName = "Flying"),
    Rolling UMETA(DisplayName = "Rolling"),
    OutOfPlay UMETA(DisplayName = "Out of Play"),
    Goal UMETA(DisplayName = "Goal")
};
}

UENUM(BlueprintType)
namespace EFootballPlayerRole
{
enum Type
{
    Goalkeeper UMETA(DisplayName = "Goalkeeper"),
    CenterBack UMETA(DisplayName = "Center Back"),
    LeftBack UMETA(DisplayName = "Left Back"),
    RightBack UMETA(DisplayName = "Right Back"),
    DefensiveMidfielder UMETA(DisplayName = "Defensive Midfielder"),
    CentralMidfielder UMETA(DisplayName = "Central Midfielder"),
    AttackingMidfielder UMETA(DisplayName = "Attacking Midfielder"),
    LeftWinger UMETA(DisplayName = "Left Winger"),
    RightWinger UMETA(DisplayName = "Right Winger"),
    Striker UMETA(DisplayName = "Striker"),
    CenterForward UMETA(DisplayName = "Center Forward"),
    Substitute UMETA(DisplayName = "Substitute")
};
}

UENUM(BlueprintType)
namespace EFootballTeamSide
{
enum Type
{
    Home UMETA(DisplayName = "Home"),
    Away UMETA(DisplayName = "Away"),
    Neutral UMETA(DisplayName = "Neutral")
};
}

UENUM(BlueprintType)
namespace EFootballMatchPhase
{
enum Type
{
    KickOff UMETA(DisplayName = "Kick Off"),
    OpenPlay UMETA(DisplayName = "Open Play"),
    SetPiece_Corner UMETA(DisplayName = "Corner"),
    SetPiece_FreeKick UMETA(DisplayName = "Free Kick"),
    SetPiece_Penalty UMETA(DisplayName = "Penalty"),
    SetPiece_ThrowIn UMETA(DisplayName = "Throw In"),
    SetPiece_GoalKick UMETA(DisplayName = "Goal Kick"),
    HalfTime UMETA(DisplayName = "Half Time"),
    FullTime UMETA(DisplayName = "Full Time")
};
}

UENUM(BlueprintType)
namespace EFootballMovementMode
{
enum Type : int
{
    Walking UMETA(DisplayName = "Walking"),
    Jogging UMETA(DisplayName = "Jogging"),
    Sprinting UMETA(DisplayName = "Sprinting"),
    Dribbling UMETA(DisplayName = "Dribbling"),
    Shielding UMETA(DisplayName = "Shielding"),
    Jockeying UMETA(DisplayName = "Jockeying"),
    Sliding UMETA(DisplayName = "Sliding"),
    Jumping UMETA(DisplayName = "Jumping"),
    Falling UMETA(DisplayName = "Falling")
};
}

/**
 * Deterministic fixed-point math for network replication
 */
USTRUCT(BlueprintType)
struct FOOTBALLCORE_API FFootballFixedVector
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Math")
    int32 X = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Math")
    int32 Y = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Math")
    int32 Z = 0;

    FFootballFixedVector() : X(0), Y(0), Z(0) {}
    FFootballFixedVector(int32 InX, int32 InY, int32 InZ) : X(InX), Y(InY), Z(InZ) {}

    static FFootballFixedVector FromVector(const FVector& Vec, float Precision = 100.0f);
    FVector ToVector(float Precision = 100.0f) const;

    FFootballFixedVector operator+(const FFootballFixedVector& Other) const;
    FFootballFixedVector operator-(const FFootballFixedVector& Other) const;
    FFootballFixedVector operator*(int32 Scalar) const;
    bool operator==(const FFootballFixedVector& Other) const;
    bool operator!=(const FFootballFixedVector& Other) const;
};

USTRUCT(BlueprintType)
struct FOOTBALLCORE_API FFootballFixedRotator
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Math")
    int32 Pitch = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Math")
    int32 Yaw = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Math")
    int32 Roll = 0;

    FFootballFixedRotator() : Pitch(0), Yaw(0), Roll(0) {}
    FFootballFixedRotator(int32 InPitch, int32 InYaw, int32 InRoll) : Pitch(InPitch), Yaw(InYaw), Roll(InRoll) {}

    static FFootballFixedRotator FromRotator(const FRotator& Rot, float Precision = 100.0f);
    FRotator ToRotator(float Precision = 100.0f) const;
};

/**
 * Ball Physics Parameters - Serializable for deterministic simulation
 */
USTRUCT(BlueprintType)
struct FFootballBallPhysicsParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball Physics")
    float Mass = 0.43f; // FIFA standard: 410-450g

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball Physics")
    float Radius = 0.11f; // FIFA standard: 21.65-22.29cm diameter

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball Physics")
    float AirDensity = 1.225f; // kg/m³ at sea level

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball Physics")
    float DragCoefficient = 0.25f; // Smooth sphere

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball Physics")
    float LiftCoefficient = 0.2f; // Magnus effect coefficient

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball Physics")
    float Restitution = 0.6f; // Bounce coefficient

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball Physics")
    float RollingFriction = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball Physics")
    float SlidingFriction = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball Physics")
    float SpinDecayRate = 0.98f; // Per second

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball Physics")
    float MaxSpinRate = 50.0f; // rad/s

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball Physics")
    float DeformationFactor = 0.1f; // Visual deformation on impact

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball Physics")
    bool bUseMagnusEffect = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball Physics")
    bool bUseDeformation = true;
};

/**
 * Player Physical Attributes
 */
USTRUCT(BlueprintType)
struct FFootballPlayerAttributes
{
    GENERATED_BODY()

    // Physical
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical")
    float Height = 1.80f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical")
    float Weight = 75.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical")
    float Acceleration = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical")
    float SprintSpeed = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical")
    float Agility = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical")
    float Balance = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical")
    float Stamina = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical")
    float Strength = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical")
    float Jumping = 70.0f;

    // Technical
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Technical")
    float BallControl = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Technical")
    float Dribbling = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Technical")
    float ShortPassing = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Technical")
    float LongPassing = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Technical")
    float Crossing = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Technical")
    float Finishing = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Technical")
    float ShotPower = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Technical")
    float LongShots = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Technical")
    float Volleys = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Technical")
    float Penalties = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Technical")
    float FreeKicks = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Technical")
    float Curve = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Technical")
    float Heading = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Technical")
    float Tackling = 70.0f;

    // Mental
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mental")
    float Vision = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mental")
    float Composure = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mental")
    float Positioning = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mental")
    float Reactions = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mental")
    float Decisions = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mental")
    float Anticipation = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mental")
    float Determination = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mental")
    float Flair = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mental")
    float Leadership = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mental")
    float WorkRate = 70.0f;

    // Goalkeeping
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Goalkeeping")
    float GK_Diving = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Goalkeeping")
    float GK_Handling = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Goalkeeping")
    float GK_Kicking = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Goalkeeping")
    float GK_Positioning = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Goalkeeping")
    float GK_Reflexes = 70.0f;

    // Computed overall
    UPROPERTY(BlueprintReadOnly, Category = "Computed")
    float OverallRating = 70.0f;
};

/**
 * Tactical Instructions
 */
USTRUCT(BlueprintType)
struct FFootballTacticalInstructions
{
    GENERATED_BODY()

    // Team Shape
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team Shape")
    int32 FormationID = 433; // 4-3-3 default

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team Shape")
    float DefensiveLine = 50.0f; // 0-100

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team Shape")
    float EngagementLine = 50.0f; // 0-100

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team Shape")
    float Width = 50.0f; // 0-100

    // In Possession
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "In Possession")
    float BuildUpPlay = 50.0f; // 0=Counter, 100=Possession

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "In Possession")
    float ChanceCreation = 50.0f; // 0=Direct, 100=Patient

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "In Possession")
    float PassingDirectness = 50.0f; // 0=Short, 100=Long

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "In Possession")
    float CrossingFrequency = 50.0f; // 0=Rare, 100=Often

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "In Possession")
    float ShootOnSight = 50.0f; // 0=Work it, 100=Shoot

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "In Possession")
    float Tempo = 50.0f; // 0=Slow, 100=Fast

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "In Possession")
    float CreativeFreedom = 50.0f; // 0=Structured, 100=Free

    // Out of Possession
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Out of Possession")
    float PressingIntensity = 50.0f; // 0=Sit Back, 100=High Press

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Out of Possession")
    float DefensiveShape = 50.0f; // 0=Man-to-Man, 100=Zonal

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Out of Possession")
    float Interceptions = 50.0f; // 0=Stay Position, 100=Intercept

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Out of Possession")
    float TacklingAggression = 50.0f; // 0=Contain, 100=Aggressive

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Out of Possession")
    float OffsideTrap = 50.0f; // 0=Never, 100=Always

    // Goalkeeper
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Goalkeeper")
    float GK_RushOut = 50.0f; // 0=Stay, 100=Sweeper

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Goalkeeper")
    float GK_Distribution = 50.0f; // 0=Throw, 100=Kick Long
};

/**
 * Match Statistics
 */
USTRUCT(BlueprintType)
struct FFootballMatchStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Match Stats")
    int32 HomeGoals = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Match Stats")
    int32 AwayGoals = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Match Stats")
    int32 HomeShots = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Match Stats")
    int32 AwayShots = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Match Stats")
    int32 HomeShotsOnTarget = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Match Stats")
    int32 AwayShotsOnTarget = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Match Stats")
    float HomePossession = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Match Stats")
    float AwayPossession = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Match Stats")
    int32 HomePasses = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Match Stats")
    int32 AwayPasses = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Match Stats")
    int32 HomePassAccuracy = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Match Stats")
    int32 AwayPassAccuracy = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Match Stats")
    int32 HomeFouls = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Match Stats")
    int32 AwayFouls = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Match Stats")
    int32 HomeYellowCards = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Match Stats")
    int32 AwayYellowCards = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Match Stats")
    int32 HomeRedCards = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Match Stats")
    int32 AwayRedCards = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Match Stats")
    int32 HomeCorners = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Match Stats")
    int32 AwayCorners = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Match Stats")
    int32 HomeOffsides = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Match Stats")
    int32 AwayOffsides = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Match Stats")
    int32 HomeSaves = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Match Stats")
    int32 AwaySaves = 0;
};

/**
 * Input Action Data for Football
 */
USTRUCT(BlueprintType)
struct FFootballInputAction
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTag ActionTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Value = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector WorldDirection = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AActor* TargetActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 FrameNumber = 0; // For deterministic replay
};