// Copyright 2024 Craquez. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AI/StateTree.h"
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
    UPROPERTY()
    TObjectPtr<AFootballPlayerCharacter> Character = nullptr;

    UPROPERTY()
    TObjectPtr<UFootballBallComponent> Ball = nullptr;

    UPROPERTY()
    TObjectPtr<AFootballTeam> Team = nullptr;

    UPROPERTY()
    TObjectPtr<AFootballTeam> OpponentTeam = nullptr;

    UPROPERTY()
    TObjectPtr<UFootballTacticalComponent> Tactical = nullptr;

    // Ball State
    UPROPERTY()
    EFootballBallState::Type BallState = EFootballBallState::Free;

    UPROPERTY()
    FVector BallPosition = FVector::ZeroVector;

    UPROPERTY()
    FVector BallVelocity = FVector::ZeroVector;

    UPROPERTY()
    AActor* BallPossessor = nullptr;

    // Player State
    UPROPERTY()
    EFootballPlayerRole::Type PlayerRole = EFootballPlayerRole::CentralMidfielder;

    UPROPERTY()
    FVector PlayerPosition = FVector::ZeroVector;

    UPROPERTY()
    FVector PlayerVelocity = FVector::ZeroVector;

    UPROPERTY()
    float Stamina = 100.0f;

    UPROPERTY()
    bool bHasBall = false;

    UPROPERTY()
    bool bIsFatigued = false;

    // Team State
    UPROPERTY()
    bool bTeamHasPossession = false;

    UPROPERTY()
    float TeamPossessionTime = 0.0f;

    UPROPERTY()
    EFootballMatchPhase::Type MatchPhase = EFootballMatchPhase::OpenPlay;

    // Tactical
    UPROPERTY()
    float PressingIntensity = 50.0f;

    UPROPERTY()
    float DefensiveLine = 50.0f;

    UPROPERTY()
    float InterceptionThreshold = 0.7f;

    // Distances
    UPROPERTY()
    float DistanceToBall = 0.0f;

    UPROPERTY()
    float DistanceToGoal = 0.0f;

    UPROPERTY()
    float DistanceToNearestOpponent = 0.0f;

    UPROPERTY()
    float DistanceToNearestTeammate = 0.0f;

    // Interception
    UPROPERTY()
    FVector PredictedInterceptPoint = FVector::ZeroVector;

    UPROPERTY()
    float TimeToIntercept = 0.0f;

    UPROPERTY()
    bool bCanIntercept = false;

    UPROPERTY()
    float InterceptConfidence = 0.0f;
};

/**
 * State Tree Evaluators
 */
UCLASS()
class FOOTBALLCOREAI_API UFootballStateTreeEvaluator : public UStateTreeEvaluatorBase
{
    GENERATED_BODY()

public:
    virtual void Evaluate(FStateTreeExecutionContext& Context, float DeltaTime) const override;
};

/**
 * Ball State Evaluator
 */
UCLASS()
class FOOTBALLCOREAI_API UBallStateEvaluator : public UFootballStateTreeEvaluator
{
    GENERATED_BODY()

public:
    virtual void Evaluate(FStateTreeExecutionContext& Context, float DeltaTime) const override;
};

/**
 * Tactical Evaluator
 */
UCLASS()
class FOOTBALLCOREAI_API UTacticalEvaluator : public UFootballStateTreeEvaluator
{
    GENERATED_BODY()

public:
    virtual void Evaluate(FStateTreeExecutionContext& Context, float DeltaTime) const override;
};

/**
 * Interception Evaluator - Key for Press -> Intercept transition
 */
UCLASS()
class FOOTBALLCOREAI_API UInterceptionEvaluator : public UFootballStateTreeEvaluator
{
    GENERATED_BODY()

public:
    virtual void Evaluate(FStateTreeExecutionContext& Context, float DeltaTime) const override;

    UPROPERTY(EditAnywhere, Category = "Interception")
    float MaxInterceptDistance = 5000.0f;

    UPROPERTY(EditAnywhere, Category = "Interception")
    float MaxPredictionTime = 3.0f;

    UPROPERTY(EditAnywhere, Category = "Interception")
    float MinInterceptConfidence = 0.6f;
};

/**
 * State Tree Tasks
 */
UCLASS()
class FOOTBALLCOREAI_API UFootballStateTreeTask : public UStateTreeTaskBase
{
    GENERATED_BODY()

public:
    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
    virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, float DeltaTime) const override;
    virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};

/**
 * Idle Task
 */
UCLASS()
class FOOTBALLCOREAI_API UIdleTask : public UFootballStateTreeTask
{
    GENERATED_BODY()

public:
    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
    virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, float DeltaTime) const override;
};

/**
 * Move To Formation Task
 */
UCLASS()
class FOOTBALLCOREAI_API UMoveToFormationTask : public UFootballStateTreeTask
{
    GENERATED_BODY()

public:
    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
    virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, float DeltaTime) const override;

    UPROPERTY(EditAnywhere, Category = "Formation")
    float PositionTolerance = 200.0f;
};

/**
 * Press Task - Fixed transition to Intercept
 */
UCLASS()
class FOOTBALLCOREAI_API UPressTask : public UFootballStateTreeTask
{
    GENERATED_BODY()

public:
    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
    virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, float DeltaTime) const override;

    UPROPERTY(EditAnywhere, Category = "Press")
    float PressDistance = 300.0f;

    UPROPERTY(EditAnywhere, Category = "Press")
    float PressAngle = 45.0f;

    UPROPERTY(EditAnywhere, Category = "Press")
    bool bForceWeakFoot = true;
};

/**
 * Intercept Task - Properly handles interception
 */
UCLASS()
class FOOTBALLCOREAI_API UInterceptTask : public UFootballStateTreeTask
{
    GENERATED_BODY()

public:
    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
    virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, float DeltaTime) const override;

    UPROPERTY(EditAnywhere, Category = "Intercept")
    float PredictionTime = 2.0f;

    UPROPERTY(EditAnywhere, Category = "Intercept")
    float MaxInterceptDistance = 5000.0f;
};

/**
 * Support Task
 */
UCLASS()
class FOOTBALLCOREAI_API USupportTask : public UFootballStateTreeTask
{
    GENERATED_BODY()

public:
    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
    virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, float DeltaTime) const override;

    UPROPERTY(EditAnywhere, Category = "Support")
    float SupportDistance = 1500.0f;
};

/**
 * Mark Task
 */
UCLASS()
class FOOTBALLCOREAI_API UMarkTask : public UFootballStateTreeTask
{
    GENERATED_BODY()

public:
    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
    virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, float DeltaTime) const override;

    UPROPERTY(EditAnywhere, Category = "Mark")
    float MarkDistance = 200.0f;

    UPROPERTY(EditAnywhere, Category = "Mark")
    bool bGoalSide = true;

    UPROPERTY(EditAnywhere, Category = "Mark")
    bool bBallSide = true;
};

/**
 * Attack Task
 */
UCLASS()
class FOOTBALLCOREAI_API UAttackTask : public UFootballStateTreeTask
{
    GENERATED_BODY()

public:
    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
    virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, float DeltaTime) const override;
};

/**
 * Defend Task
 */
UCLASS()
class FOOTBALLCOREAI_API UDefendTask : public UFootballStateTreeTask
{
    GENERATED_BODY()

public:
    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
    virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, float DeltaTime) const override;
};

/**
 * Goalkeeper Task
 */
UCLASS()
class FOOTBALLCOREAI_API UGoalkeeperTask : public UFootballStateTreeTask
{
    GENERATED_BODY()

public:
    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
    virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, float DeltaTime) const override;
};

/**
 * Set Piece Task
 */
UCLASS()
class FOOTBALLCOREAI_API USetPieceTask : public UFootballStateTreeTask
{
    GENERATED_BODY()

public:
    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
    virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, float DeltaTime) const override;

    UPROPERTY(EditAnywhere, Category = "SetPiece")
    EFootballMatchPhase::Type SetPieceType = EFootballMatchPhase::SetPiece_FreeKick;
};