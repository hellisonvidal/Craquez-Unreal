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
    extern const FGameplayTag Game_Active;
    extern const FGameplayTag Game_Paused;
    extern const FGameplayTag Game_HalfTime;
    extern const FGameplayTag Game_Finished;
    extern const FGameplayTag Game_SetPiece;

    // Ball States
    extern const FGameplayTag Ball_Free;
    extern const FGameplayTag Ball_Possessed;
    extern const FGameplayTag Ball_Flying;
    extern const FGameplayTag Ball_Rolling;
    extern const FGameplayTag Ball_OutOfPlay;
    extern const FGameplayTag Ball_Goal;

    // Player States
    extern const FGameplayTag Player_Idle;
    extern const FGameplayTag Player_Running;
    extern const FGameplayTag Player_Sprinting;
    extern const FGameplayTag Player_Dribbling;
    extern const FGameplayTag Player_Passing;
    extern const FGameplayTag Player_Shooting;
    extern const FGameplayTag Player_Tackling;
    extern const FGameplayTag Player_Intercepting;
    extern const FGameplayTag Player_Goalkeeping;
    extern const FGameplayTag Player_Celebrating;
    extern const FGameplayTag Player_Injured;
    extern const FGameplayTag Player_Fatigued;

    // Team States
    extern const FGameplayTag Team_Attacking;
    extern const FGameplayTag Team_Defending;
    extern const FGameplayTag Team_Pressing;
    extern const FGameplayTag Team_CounterAttacking;
    extern const FGameplayTag Team_Possession;
    extern const FGameplayTag Team_SetPieceAttack;
    extern const FGameplayTag Team_SetPieceDefense;

    // Actions
    extern const FGameplayTag Action_Pass_Short;
    extern const FGameplayTag Action_Pass_Long;
    extern const FGameplayTag Action_Pass_Through;
    extern const FGameplayTag Action_Pass_Cross;
    extern const FGameplayTag Action_Shoot_Power;
    extern const FGameplayTag Action_Shoot_Placement;
    extern const FGameplayTag Action_Shoot_Chip;
    extern const FGameplayTag Action_Shoot_Volley;
    extern const FGameplayTag Action_Dribble;
    extern const FGameplayTag Action_SkillMove;
    extern const FGameplayTag Action_Tackle_Standing;
    extern const FGameplayTag Action_Tackle_Sliding;
    extern const FGameplayTag Action_Intercept;
    extern const FGameplayTag Action_Block;
    extern const FGameplayTag Action_Clearance;
    extern const FGameplayTag Action_Header;
    extern const FGameplayTag Action_Goalkeeper_Save;
    extern const FGameplayTag Action_Goalkeeper_Catch;
    extern const FGameplayTag Action_Goalkeeper_Punch;
    extern const FGameplayTag Action_Goalkeeper_Distribute_Throw;
    extern const FGameplayTag Action_Goalkeeper_Distribute_Kick;

    // Body Parts
    extern const FGameplayTag Body_LeftFoot;
    extern const FGameplayTag Body_RightFoot;
    extern const FGameplayTag Body_Head;
    extern const FGameplayTag Body_Chest;
    extern const FGameplayTag Body_LeftThigh;
    extern const FGameplayTag Body_RightThigh;
    extern const FGameplayTag Body_LeftShoulder;
    extern const FGameplayTag Body_RightShoulder;
}

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

/**
 * Deterministic fixed-point math for network replication
 */
struct FFootballFixedVector
{
    int32 X;
    int32 Y;
    int32 Z;

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

struct FFootballFixedRotator
{
    int32 Pitch;
    int32 Yaw;
    int32 Roll;

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