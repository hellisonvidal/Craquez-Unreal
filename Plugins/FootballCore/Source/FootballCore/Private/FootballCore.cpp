// Copyright 2024 Craquez. All Rights Reserved.

#include "FootballCore.h"
#include "Modules/ModuleManager.h"
#include "FootballCoreTypes.h"

DEFINE_LOG_CATEGORY(LogFootballCore);

void FFootballCoreModule::StartupModule()
{
    UE_LOG(LogFootballCore, Log, TEXT("FootballCore Module Started"));

    // Initialize gameplay tags
    FFootballTags::Game_Active = FGameplayTag::RequestGameplayTag(FName("Game.Active"), false);
    FFootballTags::Game_Paused = FGameplayTag::RequestGameplayTag(FName("Game.Paused"), false);
    FFootballTags::Game_HalfTime = FGameplayTag::RequestGameplayTag(FName("Game.HalfTime"), false);
    FFootballTags::Game_Finished = FGameplayTag::RequestGameplayTag(FName("Game.Finished"), false);
    FFootballTags::Game_SetPiece = FGameplayTag::RequestGameplayTag(FName("Game.SetPiece"), false);

    FFootballTags::Ball_Free = FGameplayTag::RequestGameplayTag(FName("Ball.Free"), false);
    FFootballTags::Ball_Possessed = FGameplayTag::RequestGameplayTag(FName("Ball.Possessed"), false);
    FFootballTags::Ball_Flying = FGameplayTag::RequestGameplayTag(FName("Ball.Flying"), false);
    FFootballTags::Ball_Rolling = FGameplayTag::RequestGameplayTag(FName("Ball.Rolling"), false);
    FFootballTags::Ball_OutOfPlay = FGameplayTag::RequestGameplayTag(FName("Ball.OutOfPlay"), false);
    FFootballTags::Ball_Goal = FGameplayTag::RequestGameplayTag(FName("Ball.Goal"), false);

    FFootballTags::Player_Idle = FGameplayTag::RequestGameplayTag(FName("Player.Idle"), false);
    FFootballTags::Player_Running = FGameplayTag::RequestGameplayTag(FName("Player.Running"), false);
    FFootballTags::Player_Sprinting = FGameplayTag::RequestGameplayTag(FName("Player.Sprinting"), false);
    FFootballTags::Player_Dribbling = FGameplayTag::RequestGameplayTag(FName("Player.Dribbling"), false);
    FFootballTags::Player_Passing = FGameplayTag::RequestGameplayTag(FName("Player.Passing"), false);
    FFootballTags::Player_Shooting = FGameplayTag::RequestGameplayTag(FName("Player.Shooting"), false);
    FFootballTags::Player_Tackling = FGameplayTag::RequestGameplayTag(FName("Player.Tackling"), false);
    FFootballTags::Player_Intercepting = FGameplayTag::RequestGameplayTag(FName("Player.Intercepting"), false);
    FFootballTags::Player_Goalkeeping = FGameplayTag::RequestGameplayTag(FName("Player.Goalkeeping"), false);
    FFootballTags::Player_Celebrating = FGameplayTag::RequestGameplayTag(FName("Player.Celebrating"), false);
    FFootballTags::Player_Injured = FGameplayTag::RequestGameplayTag(FName("Player.Injured"), false);
    FFootballTags::Player_Fatigued = FGameplayTag::RequestGameplayTag(FName("Player.Fatigued"), false);

    FFootballTags::Team_Attacking = FGameplayTag::RequestGameplayTag(FName("Team.Attacking"), false);
    FFootballTags::Team_Defending = FGameplayTag::RequestGameplayTag(FName("Team.Defending"), false);
    FFootballTags::Team_Pressing = FGameplayTag::RequestGameplayTag(FName("Team.Pressing"), false);
    FFootballTags::Team_CounterAttacking = FGameplayTag::RequestGameplayTag(FName("Team.CounterAttacking"), false);
    FFootballTags::Team_Possession = FGameplayTag::RequestGameplayTag(FName("Team.Possession"), false);
    FFootballTags::Team_SetPieceAttack = FGameplayTag::RequestGameplayTag(FName("Team.SetPieceAttack"), false);
    FFootballTags::Team_SetPieceDefense = FGameplayTag::RequestGameplayTag(FName("Team.SetPieceDefense"), false);

    FFootballTags::Action_Pass_Short = FGameplayTag::RequestGameplayTag(FName("Action.Pass.Short"), false);
    FFootballTags::Action_Pass_Long = FGameplayTag::RequestGameplayTag(FName("Action.Pass.Long"), false);
    FFootballTags::Action_Pass_Through = FGameplayTag::RequestGameplayTag(FName("Action.Pass.Through"), false);
    FFootballTags::Action_Pass_Cross = FGameplayTag::RequestGameplayTag(FName("Action.Pass.Cross"), false);
    FFootballTags::Action_Shoot_Power = FGameplayTag::RequestGameplayTag(FName("Action.Shoot.Power"), false);
    FFootballTags::Action_Shoot_Placement = FGameplayTag::RequestGameplayTag(FName("Action.Shoot.Placement"), false);
    FFootballTags::Action_Shoot_Chip = FGameplayTag::RequestGameplayTag(FName("Action.Shoot.Chip"), false);
    FFootballTags::Action_Shoot_Volley = FGameplayTag::RequestGameplayTag(FName("Action.Shoot.Volley"), false);
    FFootballTags::Action_Dribble = FGameplayTag::RequestGameplayTag(FName("Action.Dribble"), false);
    FFootballTags::Action_SkillMove = FGameplayTag::RequestGameplayTag(FName("Action.SkillMove"), false);
    FFootballTags::Action_Tackle_Standing = FGameplayTag::RequestGameplayTag(FName("Action.Tackle.Standing"), false);
    FFootballTags::Action_Tackle_Sliding = FGameplayTag::RequestGameplayTag(FName("Action.Tackle.Sliding"), false);
    FFootballTags::Action_Intercept = FGameplayTag::RequestGameplayTag(FName("Action.Intercept"), false);
    FFootballTags::Action_Block = FGameplayTag::RequestGameplayTag(FName("Action.Block"), false);
    FFootballTags::Action_Clearance = FGameplayTag::RequestGameplayTag(FName("Action.Clearance"), false);
    FFootballTags::Action_Header = FGameplayTag::RequestGameplayTag(FName("Action.Header"), false);
    FFootballTags::Action_Goalkeeper_Save = FGameplayTag::RequestGameplayTag(FName("Action.Goalkeeper.Save"), false);
    FFootballTags::Action_Goalkeeper_Catch = FGameplayTag::RequestGameplayTag(FName("Action.Goalkeeper.Catch"), false);
    FFootballTags::Action_Goalkeeper_Punch = FGameplayTag::RequestGameplayTag(FName("Action.Goalkeeper.Punch"), false);
    FFootballTags::Action_Goalkeeper_Distribute_Throw = FGameplayTag::RequestGameplayTag(FName("Action.Goalkeeper.Distribute.Throw"), false);
    FFootballTags::Action_Goalkeeper_Distribute_Kick = FGameplayTag::RequestGameplayTag(FName("Action.Goalkeeper.Distribute.Kick"), false);

    FFootballTags::Body_LeftFoot = FGameplayTag::RequestGameplayTag(FName("Body.LeftFoot"), false);
    FFootballTags::Body_RightFoot = FGameplayTag::RequestGameplayTag(FName("Body.RightFoot"), false);
    FFootballTags::Body_Head = FGameplayTag::RequestGameplayTag(FName("Body.Head"), false);
    FFootballTags::Body_Chest = FGameplayTag::RequestGameplayTag(FName("Body.Chest"), false);
    FFootballTags::Body_LeftThigh = FGameplayTag::RequestGameplayTag(FName("Body.LeftThigh"), false);
    FFootballTags::Body_RightThigh = FGameplayTag::RequestGameplayTag(FName("Body.RightThigh"), false);
    FFootballTags::Body_LeftShoulder = FGameplayTag::RequestGameplayTag(FName("Body.LeftShoulder"), false);
    FFootballTags::Body_RightShoulder = FGameplayTag::RequestGameplayTag(FName("Body.RightShoulder"), false);
}

void FFootballCoreModule::ShutdownModule()
{
    UE_LOG(LogFootballCore, Log, TEXT("FootballCore Module Shutdown"));
}

// FFootballFixedVector implementation
FFootballFixedVector FFootballFixedVector::FromVector(const FVector& Vec, float Precision)
{
    return FFootballFixedVector(
        FMath::RoundToInt(Vec.X * Precision),
        FMath::RoundToInt(Vec.Y * Precision),
        FMath::RoundToInt(Vec.Z * Precision)
    );
}

FVector FFootballFixedVector::ToVector(float Precision) const
{
    return FVector(X / Precision, Y / Precision, Z / Precision);
}

FFootballFixedVector FFootballFixedVector::operator+(const FFootballFixedVector& Other) const
{
    return FFootballFixedVector(X + Other.X, Y + Other.Y, Z + Other.Z);
}

FFootballFixedVector FFootballFixedVector::operator-(const FFootballFixedVector& Other) const
{
    return FFootballFixedVector(X - Other.X, Y - Other.Y, Z - Other.Z);
}

FFootballFixedVector FFootballFixedVector::operator*(int32 Scalar) const
{
    return FFootballFixedVector(X * Scalar, Y * Scalar, Z * Scalar);
}

bool FFootballFixedVector::operator==(const FFootballFixedVector& Other) const
{
    return X == Other.X && Y == Other.Y && Z == Other.Z;
}

bool FFootballFixedVector::operator!=(const FFootballFixedVector& Other) const
{
    return !(*this == Other);
}

// FFootballFixedRotator implementation
FFootballFixedRotator FFootballFixedRotator::FromRotator(const FRotator& Rot, float Precision)
{
    return FFootballFixedRotator(
        FMath::RoundToInt(Rot.Pitch * Precision),
        FMath::RoundToInt(Rot.Yaw * Precision),
        FMath::RoundToInt(Rot.Roll * Precision)
    );
}

FRotator FFootballFixedRotator::ToRotator(float Precision) const
{
    return FRotator(Pitch / Precision, Yaw / Precision, Roll / Precision);
}

IMPLEMENT_MODULE(FFootballCoreModule, FootballCore)