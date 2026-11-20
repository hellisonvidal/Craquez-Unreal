// Copyright 2024 Craquez. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "StateTree.h"
#include "Blueprint/StateTreeEvaluatorBlueprintBase.h"
#include "Blueprint/StateTreeTaskBlueprintBase.h"
#include "FootballCoreTypes.h"
#include "FootballStateTree.generated.h"

class AFootballPlayerCharacter;
class UFootballBallComponent;
class AFootballTeam;
class UFootballTacticalComponent;

/**
 * Football State Tree - Tactical AI state machine with proper transitions
 * Fixes the Press -> Intercept transition issue by adding proper conditions
 */
USTRUCT(BlueprintType)
struct FFootballStateTreeContext
{
    GENERATED_BODY()

    // References
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    TObjectPtr<AFootballPlayerCharacter> Character = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    TObjectPtr<UFootballBallComponent> Ball = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    TObjectPtr<AFootballTeam> Team = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    TObjectPtr<AFootballTeam> OpponentTeam = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    TObjectPtr<UFootballTacticalComponent> Tactical = nullptr;

    // Ball State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    TEnumAsByte<EFootballBallState::Type> BallState = EFootballBallState::Free;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    FVector BallPosition = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    FVector BallVelocity = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    TObjectPtr<AActor> BallPossessor = nullptr;

    // Player State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    TEnumAsByte<EFootballPlayerRole::Type> PlayerRole = EFootballPlayerRole::CentralMidfielder;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    FVector PlayerPosition = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    FVector PlayerVelocity = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    float Stamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    bool bHasBall = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    bool bIsFatigued = false;

    // Team State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    bool bTeamHasPossession = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    float TeamPossessionTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    TEnumAsByte<EFootballMatchPhase::Type> MatchPhase = EFootballMatchPhase::OpenPlay;

    // Tactical
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    float PressingIntensity = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    float DefensiveLine = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    float InterceptionThreshold = 0.7f;

    // Distances
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    float DistanceToBall = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    float DistanceToGoal = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    float DistanceToNearestOpponent = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    float DistanceToNearestTeammate = 0.0f;

    // Interception
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    FVector PredictedInterceptPoint = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    float TimeToIntercept = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    bool bCanIntercept = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    float InterceptConfidence = 0.0f;
};

/**
 * State Tree Evaluators
 */
UCLASS()
class FOOTBALLCOREAI_API UFootballStateTreeEvaluator : public UStateTreeEvaluatorBlueprintBase
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    FFootballStateTreeContext FootballContext;

protected:
    virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) override;
};

/**
 * Ball State Evaluator
 */
UCLASS()
class FOOTBALLCOREAI_API UBallStateEvaluator : public UFootballStateTreeEvaluator
{
    GENERATED_BODY()

protected:
    virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) override;
};

/**
 * Tactical Evaluator
 */
UCLASS()
class FOOTBALLCOREAI_API UTacticalEvaluator : public UFootballStateTreeEvaluator
{
    GENERATED_BODY()

protected:
    virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) override;
};

/**
 * Interception Evaluator - Key for Press -> Intercept transition
 */
UCLASS()
class FOOTBALLCOREAI_API UInterceptionEvaluator : public UFootballStateTreeEvaluator
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "Interception")
    float MaxInterceptDistance = 5000.0f;

    UPROPERTY(EditAnywhere, Category = "Interception")
    float MaxPredictionTime = 3.0f;

    UPROPERTY(EditAnywhere, Category = "Interception")
    float MinInterceptConfidence = 0.6f;

protected:
    virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) override;
};

/**
 * State Tree Tasks
 */
UCLASS()
class FOOTBALLCOREAI_API UFootballStateTreeTask : public UStateTreeTaskBlueprintBase
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    FFootballStateTreeContext FootballContext;

protected:
    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
                                           const FStateTreeTransitionResult& Transition) override;
    virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) override;
    virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;
};

/**
 * Idle Task
 */
UCLASS()
class FOOTBALLCOREAI_API UIdleTask : public UFootballStateTreeTask
{
    GENERATED_BODY()

protected:
    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
                                           const FStateTreeTransitionResult& Transition) override;
    virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) override;
};

/**
 * Move To Formation Task
 */
UCLASS()
class FOOTBALLCOREAI_API UMoveToFormationTask : public UFootballStateTreeTask
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "Formation")
    float PositionTolerance = 200.0f;

protected:
    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
                                           const FStateTreeTransitionResult& Transition) override;
    virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) override;
};

/**
 * Press Task - Fixed transition to Intercept
 */
UCLASS()
class FOOTBALLCOREAI_API UPressTask : public UFootballStateTreeTask
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "Press")
    float PressDistance = 300.0f;

    UPROPERTY(EditAnywhere, Category = "Press")
    float PressAngle = 45.0f;

    UPROPERTY(EditAnywhere, Category = "Press")
    bool bForceWeakFoot = true;

protected:
    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
                                           const FStateTreeTransitionResult& Transition) override;
    virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) override;
};

/**
 * Intercept Task - Properly handles interception
 */
UCLASS()
class FOOTBALLCOREAI_API UInterceptTask : public UFootballStateTreeTask
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "Intercept")
    float PredictionTime = 2.0f;

    UPROPERTY(EditAnywhere, Category = "Intercept")
    float MaxInterceptDistance = 5000.0f;

protected:
    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
                                           const FStateTreeTransitionResult& Transition) override;
    virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) override;
};

/**
 * Support Task
 */
UCLASS()
class FOOTBALLCOREAI_API USupportTask : public UFootballStateTreeTask
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "Support")
    float SupportDistance = 1500.0f;

protected:
    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
                                           const FStateTreeTransitionResult& Transition) override;
    virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) override;
};

/**
 * Mark Task
 */
UCLASS()
class FOOTBALLCOREAI_API UMarkTask : public UFootballStateTreeTask
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "Mark")
    float MarkDistance = 200.0f;

    UPROPERTY(EditAnywhere, Category = "Mark")
    bool bGoalSide = true;

    UPROPERTY(EditAnywhere, Category = "Mark")
    bool bBallSide = true;

protected:
    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
                                           const FStateTreeTransitionResult& Transition) override;
    virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) override;
};

/**
 * Attack Task
 */
UCLASS()
class FOOTBALLCOREAI_API UAttackTask : public UFootballStateTreeTask
{
    GENERATED_BODY()

protected:
    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
                                           const FStateTreeTransitionResult& Transition) override;
    virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) override;
};

/**
 * Defend Task
 */
UCLASS()
class FOOTBALLCOREAI_API UDefendTask : public UFootballStateTreeTask
{
    GENERATED_BODY()

protected:
    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
                                           const FStateTreeTransitionResult& Transition) override;
    virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) override;
};

/**
 * Goalkeeper Task
 */
UCLASS()
class FOOTBALLCOREAI_API UGoalkeeperTask : public UFootballStateTreeTask
{
    GENERATED_BODY()

protected:
    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
                                           const FStateTreeTransitionResult& Transition) override;
    virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) override;
};

/**
 * Set Piece Task
 */
UCLASS()
class FOOTBALLCOREAI_API USetPieceTask : public UFootballStateTreeTask
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "SetPiece")
    TEnumAsByte<EFootballMatchPhase::Type> SetPieceType = EFootballMatchPhase::SetPiece_FreeKick;

protected:
    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
                                           const FStateTreeTransitionResult& Transition) override;
    virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) override;
};
