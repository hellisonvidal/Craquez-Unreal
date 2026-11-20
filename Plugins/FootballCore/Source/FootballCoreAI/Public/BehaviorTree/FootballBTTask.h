// Copyright 2024 Craquez. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "FootballCoreTypes.h"
#include "FootballBTTask.generated.h"

class AFootballPlayerCharacter;
class UFootballBallComponent;
class AFootballTeam;

/**
 * Base class for football behavior tree tasks
 */
UCLASS(Abstract, Blueprintable)
class FOOTBALLCOREAI_API UFootballBTTask : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UFootballBTTask();

protected:
    // Helper functions
    AFootballPlayerCharacter* GetFootballCharacter(UBehaviorTreeComponent& OwnerComp) const;
    UFootballBallComponent* GetBall(UBehaviorTreeComponent& OwnerComp) const;
    AFootballTeam* GetTeam(UBehaviorTreeComponent& OwnerComp) const;
    AFootballTeam* GetOpponentTeam(UBehaviorTreeComponent& OwnerComp) const;

    // Blackboard keys
    UPROPERTY(EditAnywhere, Category = "Football|Blackboard")
    FBlackboardKeySelector BallKey;

    UPROPERTY(EditAnywhere, Category = "Football|Blackboard")
    FBlackboardKeySelector TargetPlayerKey;

    UPROPERTY(EditAnywhere, Category = "Football|Blackboard")
    FBlackboardKeySelector TargetLocationKey;

    UPROPERTY(EditAnywhere, Category = "Football|Blackboard")
    FBlackboardKeySelector TeamKey;

    UPROPERTY(EditAnywhere, Category = "Football|Blackboard")
    FBlackboardKeySelector OpponentTeamKey;

    UPROPERTY(EditAnywhere, Category = "Football|Blackboard")
    FBlackboardKeySelector ActionStateKey;

    UPROPERTY(EditAnywhere, Category = "Football|Blackboard")
    FBlackboardKeySelector StaminaKey;

    UPROPERTY(EditAnywhere, Category = "Football|Blackboard")
    FBlackboardKeySelector FormationPositionKey;
};

/**
 * Task: Move to ball
 */
UCLASS(Blueprintable)
class FOOTBALLCOREAI_API UBTTask_MoveToBall : public UFootballBTTask
{
    GENERATED_BODY()

public:
    UBTTask_MoveToBall();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
    UPROPERTY(EditAnywhere, Category = "Football|MoveToBall")
    float AcceptanceRadius = 150.0f;

    UPROPERTY(EditAnywhere, Category = "Football|MoveToBall")
    bool bStopOnOverlap = true;

    UPROPERTY(EditAnywhere, Category = "Football|MoveToBall")
    EFootballMovementMode::Type MovementMode = EFootballMovementMode::Jogging;
};

/**
 * Task: Intercept ball
 */
UCLASS(Blueprintable)
class FOOTBALLCOREAI_API UBTTask_InterceptBall : public UFootballBTTask
{
    GENERATED_BODY()

public:
    UBTTask_InterceptBall();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
    UPROPERTY(EditAnywhere, Category = "Football|Intercept")
    float PredictionTime = 2.0f;

    UPROPERTY(EditAnywhere, Category = "Football|Intercept")
    float MaxInterceptDistance = 5000.0f;

    UPROPERTY(EditAnywhere, Category = "Football|Intercept")
    bool bPredictBallPhysics = true;
};

/**
 * Task: Pass to teammate
 */
UCLASS(Blueprintable)
class FOOTBALLCOREAI_API UBTTask_PassToTeammate : public UFootballBTTask
{
    GENERATED_BODY()

public:
    UBTTask_PassToTeammate();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
    UPROPERTY(EditAnywhere, Category = "Football|Pass")
    FGameplayTag PassType = FFootballTags::Action_Pass_Short;

    UPROPERTY(EditAnywhere, Category = "Football|Pass")
    float MinPassDistance = 500.0f;

    UPROPERTY(EditAnywhere, Category = "Football|Pass")
    float MaxPassDistance = 4000.0f;

    UPROPERTY(EditAnywhere, Category = "Football|Pass")
    bool bRequireOpenLane = true;
};

/**
 * Task: Shoot at goal
 */
UCLASS(Blueprintable)
class FOOTBALLCOREAI_API UBTTask_ShootAtGoal : public UFootballBTTask
{
    GENERATED_BODY()

public:
    UBTTask_ShootAtGoal();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
    UPROPERTY(EditAnywhere, Category = "Football|Shoot")
    FGameplayTag ShotType = FFootballTags::Action_Shoot_Power;

    UPROPERTY(EditAnywhere, Category = "Football|Shoot")
    float MinShootDistance = 1000.0f;

    UPROPERTY(EditAnywhere, Category = "Football|Shoot")
    float MaxShootDistance = 30000.0f;

    UPROPERTY(EditAnywhere, Category = "Football|Shoot")
    float GoalTargetVariance = 1000.0f; // cm

    UPROPERTY(EditAnywhere, Category = "Football|Shoot")
    bool bPreferPlacement = false;
};

/**
 * Task: Tackle opponent
 */
UCLASS(Blueprintable)
class FOOTBALLCOREAI_API UBTTask_TackleOpponent : public UFootballBTTask
{
    GENERATED_BODY()

public:
    UBTTask_TackleOpponent();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
    UPROPERTY(EditAnywhere, Category = "Football|Tackle")
    FGameplayTag TackleType = FFootballTags::Action_Tackle_Standing;

    UPROPERTY(EditAnywhere, Category = "Football|Tackle")
    float TackleRange = 200.0f;

    UPROPERTY(EditAnywhere, Category = "Football|Tackle")
    float MinTackleSuccessChance = 0.4f;
};

/**
 * Task: Mark opponent
 */
UCLASS(Blueprintable)
class FOOTBALLCOREAI_API UBTTask_MarkOpponent : public UFootballBTTask
{
    GENERATED_BODY()

public:
    UBTTask_MarkOpponent();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
    UPROPERTY(EditAnywhere, Category = "Football|Mark")
    float MarkDistance = 200.0f;

    UPROPERTY(EditAnywhere, Category = "Football|Mark")
    bool bGoalSide = true;

    UPROPERTY(EditAnywhere, Category = "Football|Mark")
    bool bBallSide = true;
};

/**
 * Task: Move to formation position
 */
UCLASS(Blueprintable)
class FOOTBALLCOREAI_API UBTTask_MoveToFormationPosition : public UFootballBTTask
{
    GENERATED_BODY()

public:
    UBTTask_MoveToFormationPosition();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
    UPROPERTY(EditAnywhere, Category = "Football|Formation")
    float PositionTolerance = 200.0f;

    UPROPERTY(EditAnywhere, Category = "Football|Formation")
    bool bAdjustForBall = true;

    UPROPERTY(EditAnywhere, Category = "Football|Formation")
    float BallInfluenceRadius = 3000.0f;
};

/**
 * Task: Press ball carrier
 */
UCLASS(Blueprintable)
class FOOTBALLCOREAI_API UBTTask_PressBallCarrier : public UFootballBTTask
{
    GENERATED_BODY()

public:
    UBTTask_PressBallCarrier();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
    UPROPERTY(EditAnywhere, Category = "Football|Press")
    float PressDistance = 300.0f;

    UPROPERTY(EditAnywhere, Category = "Football|Press")
    float PressAngle = 45.0f; // degrees from ball-carrier's forward

    UPROPERTY(EditAnywhere, Category = "Football|Press")
    bool bForceWeakFoot = true;
};

/**
 * Task: Support ball carrier
 */
UCLASS(Blueprintable)
class FOOTBALLCOREAI_API UBTTask_SupportBallCarrier : public UFootballBTTask
{
    GENERATED_BODY()

public:
    UBTTask_SupportBallCarrier();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
    UPROPERTY(EditAnywhere, Category = "Football|Support")
    float SupportDistance = 1500.0f;

    UPROPERTY(EditAnywhere, Category = "Football|Support")
    float SupportAngle = 45.0f;

    UPROPERTY(EditAnywhere, Category = "Football|Support")
    bool bProvidePassingOption = true;
};

/**
 * Task: Goalkeeper save
 */
UCLASS(Blueprintable)
class FOOTBALLCOREAI_API UBTTask_GoalkeeperSave : public UFootballBTTask
{
    GENERATED_BODY()

public:
    UBTTask_GoalkeeperSave();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
    UPROPERTY(EditAnywhere, Category = "Football|GK")
    float ReactionTime = 0.15f;

    UPROPERTY(EditAnywhere, Category = "Football|GK")
    float DiveRange = 300.0f;

    UPROPERTY(EditAnywhere, Category = "Football|GK")
    bool bPreferCatch = true;
};

/**
 * Task: Goalkeeper distribute
 */
UCLASS(Blueprintable)
class FOOTBALLCOREAI_API UBTTask_GoalkeeperDistribute : public UFootballBTTask
{
    GENERATED_BODY()

public:
    UBTTask_GoalkeeperDistribute();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
    UPROPERTY(EditAnywhere, Category = "Football|GK")
    bool bPreferThrow = false;

    UPROPERTY(EditAnywhere, Category = "Football|GK")
    float MinKickDistance = 4000.0f;
};