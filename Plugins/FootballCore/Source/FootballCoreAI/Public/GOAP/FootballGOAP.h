// Copyright 2024 Craquez. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FootballCoreTypes.h"
#include "FootballGOAP.generated.h"

class AFootballPlayerCharacter;
class UFootballBallComponent;
class AFootballTeam;
class UFootballTacticalComponent;

UENUM(BlueprintType)
namespace EGOAPActionCost
{
    enum Type
    {
        Low = 1,
        Medium = 5,
        High = 10,
        VeryHigh = 20
    };
}

/**
 * GOAP World State - Represents the world state for planning
 */
USTRUCT(BlueprintType)
struct FGOAPWorldState
{
    GENERATED_BODY()

    // Ball state
    UPROPERTY()
    bool bHasBall = false;

    UPROPERTY()
    bool bBallInAir = false;

    UPROPERTY()
    bool bBallInPossession = false;

    UPROPERTY()
    bool bBallNearGoal = false;

    UPROPERTY()
    bool bBallInDefensiveThird = false;

    UPROPERTY()
    bool bBallInAttackingThird = false;

    // Player state
    UPROPERTY()
    bool bNearBall = false;

    UPROPERTY()
    bool bNearGoal = false;

    UPROPERTY()
    bool bMarked = false;

    UPROPERTY()
    bool bHasSupport = false;

    UPROPERTY()
    bool bCanShoot = false;

    UPROPERTY()
    bool bCanPass = false;

    UPROPERTY()
    bool bCanIntercept = false;

    UPROPERTY()
    bool bCanTackle = false;

    UPROPERTY()
    bool bFatigued = false;

    UPROPERTY()
    bool bInPosition = false;

    // Team state
    UPROPERTY()
    bool bTeamHasPossession = false;

    UPROPERTY()
    bool bTeamAttacking = false;

    UPROPERTY()
    bool bTeamDefending = false;

    UPROPERTY()
    bool bCounterAttackAvailable = false;

    UPROPERTY()
    bool bSetPieceActive = false;

    // Comparison
    bool operator==(const FGOAPWorldState& Other) const;
    bool operator!=(const FGOAPWorldState& Other) const;

    // Check if this state satisfies another (for goal checking)
    bool Satisfies(const FGOAPWorldState& Goal) const;

    // Combine states
    FGOAPWorldState Combine(const FGOAPWorldState& Other) const;
};

/**
 * GOAP Action - Base class for all actions
 */
USTRUCT(BlueprintType)
struct FGOAPAction
{
    GENERATED_BODY()

    UPROPERTY()
    FName Name;

    UPROPERTY()
    FGameplayTag ActionTag;

    UPROPERTY()
    EGOAPActionCost::Type Cost = EGOAPActionCost::Medium;

    UPROPERTY()
    FGOAPWorldState Preconditions;

    UPROPERTY()
    FGOAPWorldState Effects;

    UPROPERTY()
    float Duration = 1.0f;

    UPROPERTY()
    bool bRequiresBall = false;

    UPROPERTY()
    bool bInterruptible = true;

    FGOAPAction() {}
    FGOAPAction(FName InName, FGameplayTag InTag, EGOAPActionCost::Type InCost);
};

/**
 * GOAP Goal - Desired world state
 */
USTRUCT(BlueprintType)
struct FGOAPGoal
{
    GENERATED_BODY()

    UPROPERTY()
    FName Name;

    UPROPERTY()
    FGameplayTag GoalTag;

    UPROPERTY()
    FGOAPWorldState DesiredState;

    UPROPERTY()
    float Priority = 1.0f;

    UPROPERTY()
    float MinConfidence = 0.5f;

    FGOAPGoal() {}
    FGOAPGoal(FName InName, FGameplayTag InTag, float InPriority);
};

/**
 * GOAP Plan - Sequence of actions to achieve a goal
 */
USTRUCT(BlueprintType)
struct FGOAPPlan
{
    GENERATED_BODY()

    UPROPERTY()
    TArray<FGOAPAction> Actions;

    UPROPERTY()
    float TotalCost = 0.0f;

    UPROPERTY()
    FName GoalName;

    UPROPERTY()
    bool bValid = false;

    UPROPERTY()
    int32 CurrentActionIndex = 0;

    FGOAPPlan() : TotalCost(0.0f), bValid(false), CurrentActionIndex(0) {}
};

/**
 * GOAP Planner - Plans actions to achieve goals
 */
UCLASS(Blueprintable, BlueprintType)
class FOOTBALLCOREAI_API UFootballGOAPPlanner : public UObject
{
    GENERATED_BODY()

public:
    UFootballGOAPPlanner();

    // Planning
    UFUNCTION(BlueprintCallable, Category = "Football|GOAP")
    FGOAPPlan Plan(const FGOAPWorldState& CurrentState, const TArray<FGOAPGoal>& Goals, const TArray<FGOAPAction>& AvailableActions);

    UFUNCTION(BlueprintCallable, Category = "Football|GOAP")
    FGOAPPlan FindBestPlan(const FGOAPWorldState& CurrentState, const TArray<FGOAPGoal>& Goals, const TArray<FGOAPAction>& AvailableActions);

    // Action execution
    UFUNCTION(BlueprintCallable, Category = "Football|GOAP")
    bool CanExecuteAction(const FGOAPAction& Action, const FGOAPWorldState& CurrentState) const;

    UFUNCTION(BlueprintCallable, Category = "Football|GOAP")
    FGOAPWorldState ApplyAction(const FGOAPAction& Action, const FGOAPWorldState& CurrentState) const;

    // World state evaluation
    UFUNCTION(BlueprintCallable, Category = "Football|GOAP")
    FGOAPWorldState EvaluateWorldState(AFootballPlayerCharacter* Character, UFootballBallComponent* Ball, AFootballTeam* Team, AFootballTeam* OpponentTeam) const;

    // Default actions and goals
    UFUNCTION(BlueprintCallable, Category = "Football|GOAP")
    TArray<FGOAPAction> GetDefaultActions() const;

    UFUNCTION(BlueprintCallable, Category = "Football|GOAP")
    TArray<FGOAPGoal> GetDefaultGoals() const;

protected:
    // A* search for planning
    struct FPlanNode
    {
        FGOAPWorldState State;
        TArray<FGOAPAction> Actions;
        float Cost = 0.0f;
        float Heuristic = 0.0f;

        bool operator>(const FPlanNode& Other) const { return (Cost + Heuristic) > (Other.Cost + Other.Heuristic); }
    };

    float CalculateHeuristic(const FGOAPWorldState& Current, const FGOAPWorldState& Goal) const;
    TArray<FGOAPAction> GetApplicableActions(const FGOAPWorldState& State, const TArray<FGOAPAction>& AllActions) const;
};

/**
 * GOAP Component - Attaches to AI characters for planning
 */
UCLASS(ClassGroup = (Football), meta = (BlueprintSpawnableComponent))
class FOOTBALLCOREAI_API UFootballGOAPComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UFootballGOAPComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Planning
    UFUNCTION(BlueprintCallable, Category = "Football|GOAP")
    void RequestPlan();

    UFUNCTION(BlueprintCallable, Category = "Football|GOAP")
    void ExecuteCurrentAction(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Football|GOAP")
    void AbortCurrentPlan();

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|GOAP")
    float ReplanInterval = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|GOAP")
    float MaxPlanDepth = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|GOAP")
    TArray<FGOAPGoal> Goals;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|GOAP")
    TArray<FGOAPAction> CustomActions;

    // Current plan
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|GOAP")
    FGOAPPlan CurrentPlan;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|GOAP")
    bool bExecutingPlan = false;

    // References
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|GOAP")
    TObjectPtr<AFootballPlayerCharacter> Character;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|GOAP")
    TObjectPtr<UFootballBallComponent> Ball;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|GOAP")
    TObjectPtr<AFootballTeam> Team;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|GOAP")
    TObjectPtr<AFootballTeam> OpponentTeam;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|GOAP")
    TObjectPtr<UFootballTacticalComponent> Tactical;

    // Planner
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|GOAP")
    TObjectPtr<UFootballGOAPPlanner> Planner;

private:
    float TimeSinceLastPlan = 0.0f;
    FGOAPWorldState LastWorldState;

    void UpdateWorldState();
    void ExecuteAction(float DeltaTime);
    bool IsPlanValid() const;
    void OnActionCompleted(bool bSuccess);
};