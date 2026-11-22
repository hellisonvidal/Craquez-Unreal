// Copyright 2024 Craquez. All Rights Reserved.

#include "GOAP/FootballGOAP.h"
#include "AIController.h"
#include "BallPhysics/FootballBallComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Navigation/PathFollowingComponent.h"
#include "Player/FootballPlayerCharacter.h"
#include "Tactical/FootballTacticalComponent.h"
#include "Team/FootballTeam.h"
#include "Team/FootballTeamManager.h"

// FGOAPWorldState
bool FGOAPWorldState::operator==(const FGOAPWorldState& Other) const
{
    return bHasBall == Other.bHasBall && bBallInAir == Other.bBallInAir &&
           bBallInPossession == Other.bBallInPossession && bBallNearGoal == Other.bBallNearGoal &&
           bBallInDefensiveThird == Other.bBallInDefensiveThird &&
           bBallInAttackingThird == Other.bBallInAttackingThird && bNearBall == Other.bNearBall &&
           bNearGoal == Other.bNearGoal && bMarked == Other.bMarked && bHasSupport == Other.bHasSupport &&
           bCanShoot == Other.bCanShoot && bCanPass == Other.bCanPass && bCanIntercept == Other.bCanIntercept &&
           bCanTackle == Other.bCanTackle && bFatigued == Other.bFatigued && bInPosition == Other.bInPosition &&
           bTeamHasPossession == Other.bTeamHasPossession && bTeamAttacking == Other.bTeamAttacking &&
           bTeamDefending == Other.bTeamDefending && bCounterAttackAvailable == Other.bCounterAttackAvailable &&
           bSetPieceActive == Other.bSetPieceActive;
}

bool FGOAPWorldState::operator!=(const FGOAPWorldState& Other) const
{
    return !(*this == Other);
}

bool FGOAPWorldState::Satisfies(const FGOAPWorldState& Goal) const
{
    // Check if all true values in Goal are also true in this state
    if (Goal.bHasBall && !bHasBall)
        return false;
    if (Goal.bBallInAir && !bBallInAir)
        return false;
    if (Goal.bBallInPossession && !bBallInPossession)
        return false;
    if (Goal.bBallNearGoal && !bBallNearGoal)
        return false;
    if (Goal.bBallInDefensiveThird && !bBallInDefensiveThird)
        return false;
    if (Goal.bBallInAttackingThird && !bBallInAttackingThird)
        return false;
    if (Goal.bNearBall && !bNearBall)
        return false;
    if (Goal.bNearGoal && !bNearGoal)
        return false;
    if (Goal.bMarked && !bMarked)
        return false;
    if (Goal.bHasSupport && !bHasSupport)
        return false;
    if (Goal.bCanShoot && !bCanShoot)
        return false;
    if (Goal.bCanPass && !bCanPass)
        return false;
    if (Goal.bCanIntercept && !bCanIntercept)
        return false;
    if (Goal.bCanTackle && !bCanTackle)
        return false;
    if (Goal.bFatigued && !bFatigued)
        return false;
    if (Goal.bInPosition && !bInPosition)
        return false;
    if (Goal.bTeamHasPossession && !bTeamHasPossession)
        return false;
    if (Goal.bTeamAttacking && !bTeamAttacking)
        return false;
    if (Goal.bTeamDefending && !bTeamDefending)
        return false;
    if (Goal.bCounterAttackAvailable && !bCounterAttackAvailable)
        return false;
    if (Goal.bSetPieceActive && !bSetPieceActive)
        return false;

    return true;
}

FGOAPWorldState FGOAPWorldState::Combine(const FGOAPWorldState& Other) const
{
    FGOAPWorldState Result = *this;

    // Combine with OR logic for positive states
    Result.bHasBall = bHasBall || Other.bHasBall;
    Result.bBallInAir = bBallInAir || Other.bBallInAir;
    Result.bBallInPossession = bBallInPossession || Other.bBallInPossession;
    Result.bBallNearGoal = bBallNearGoal || Other.bBallNearGoal;
    Result.bBallInDefensiveThird = bBallInDefensiveThird || Other.bBallInDefensiveThird;
    Result.bBallInAttackingThird = bBallInAttackingThird || Other.bBallInAttackingThird;
    Result.bNearBall = bNearBall || Other.bNearBall;
    Result.bNearGoal = bNearGoal || Other.bNearGoal;
    Result.bMarked = bMarked || Other.bMarked;
    Result.bHasSupport = bHasSupport || Other.bHasSupport;
    Result.bCanShoot = bCanShoot || Other.bCanShoot;
    Result.bCanPass = bCanPass || Other.bCanPass;
    Result.bCanIntercept = bCanIntercept || Other.bCanIntercept;
    Result.bCanTackle = bCanTackle || Other.bCanTackle;
    Result.bFatigued = bFatigued || Other.bFatigued;
    Result.bInPosition = bInPosition || Other.bInPosition;
    Result.bTeamHasPossession = bTeamHasPossession || Other.bTeamHasPossession;
    Result.bTeamAttacking = bTeamAttacking || Other.bTeamAttacking;
    Result.bTeamDefending = bTeamDefending || Other.bTeamDefending;
    Result.bCounterAttackAvailable = bCounterAttackAvailable || Other.bCounterAttackAvailable;
    Result.bSetPieceActive = bSetPieceActive || Other.bSetPieceActive;

    return Result;
}

// FGOAPAction
FGOAPAction::FGOAPAction(FName InName, FGameplayTag InTag, EGOAPActionCost::Type InCost)
    : Name(InName), ActionTag(InTag), Cost(InCost)
{
}

// FGOAPGoal
FGOAPGoal::FGOAPGoal(FName InName, FGameplayTag InTag, float InPriority)
    : Name(InName), GoalTag(InTag), Priority(InPriority)
{
}

// UFootballGOAPPlanner
UFootballGOAPPlanner::UFootballGOAPPlanner() {}

FGOAPPlan UFootballGOAPPlanner::Plan(const FGOAPWorldState& CurrentState, const TArray<FGOAPGoal>& Goals,
                                     const TArray<FGOAPAction>& AvailableActions)
{
    return FindBestPlan(CurrentState, Goals, AvailableActions);
}

FGOAPPlan UFootballGOAPPlanner::FindBestPlan(const FGOAPWorldState& CurrentState, const TArray<FGOAPGoal>& Goals,
                                             const TArray<FGOAPAction>& AvailableActions)
{
    FGOAPPlan BestPlan;
    BestPlan.bValid = false;
    BestPlan.TotalCost = FLT_MAX;

    // Sort goals by priority
    TArray<FGOAPGoal> SortedGoals = Goals;
    SortedGoals.Sort([](const FGOAPGoal& A, const FGOAPGoal& B) { return A.Priority > B.Priority; });

    // Try each goal in priority order
    for (const FGOAPGoal& Goal : SortedGoals)
    {
        FGOAPPlan Plan = FindPlanForGoal(CurrentState, Goal, AvailableActions);
        if (Plan.bValid && Plan.TotalCost < BestPlan.TotalCost)
        {
            BestPlan = Plan;
        }
    }

    return BestPlan;
}

FGOAPPlan UFootballGOAPPlanner::FindPlanForGoal(const FGOAPWorldState& StartState, const FGOAPGoal& Goal,
                                                const TArray<FGOAPAction>& AvailableActions)
{
    FGOAPPlan Result;
    Result.GoalName = Goal.Name;
    Result.bValid = false;

    // A* search using min-heap TArray
    struct FLocalPlanNode
    {
        FGOAPWorldState State;
        TArray<FGOAPAction> Actions;
        float Cost = 0.0f;
        float Heuristic = 0.0f;
        int32 Depth = 0;

        float TotalCost() const { return Cost + Heuristic; }
    };

    auto NodePredicate = [](const FLocalPlanNode& A, const FLocalPlanNode& B) { return A.TotalCost() < B.TotalCost(); };

    TArray<FLocalPlanNode> OpenSet;
    TSet<FGOAPWorldState> ClosedSet;

    // Initial node
    FLocalPlanNode StartNode;
    StartNode.State = StartState;
    StartNode.Actions = {};
    StartNode.Cost = 0.0f;
    StartNode.Heuristic = CalculateHeuristic(StartState, Goal.DesiredState);
    StartNode.Depth = 0;

    OpenSet.HeapPush(StartNode, NodePredicate);

    const int32 MaxIterations = 1000;
    int32 Iterations = 0;

    while (OpenSet.Num() > 0 && Iterations < MaxIterations)
    {
        Iterations++;
        FLocalPlanNode Current;
        OpenSet.HeapPop(Current, NodePredicate);

        // Check if goal satisfied
        if (Current.State.Satisfies(Goal.DesiredState))
        {
            Result.Actions = Current.Actions;
            Result.TotalCost = Current.Cost;
            Result.bValid = true;
            Result.CurrentActionIndex = 0;
            return Result;
        }

        // Skip if already visited
        if (ClosedSet.Contains(Current.State))
        {
            continue;
        }

        ClosedSet.Add(Current.State);

        // Skip if too deep
        if (Current.Depth >= 10)
        {
            continue;
        }

        // Get applicable actions
        TArray<FGOAPAction> ApplicableActions = GetApplicableActions(Current.State, AvailableActions);

        for (const FGOAPAction& Action : ApplicableActions)
        {
            FLocalPlanNode NextNode;
            NextNode.State = ApplyAction(Action, Current.State);
            NextNode.Actions = Current.Actions;
            NextNode.Actions.Add(Action);
            NextNode.Cost = Current.Cost + static_cast<float>(Action.Cost.GetValue());
            NextNode.Heuristic = CalculateHeuristic(NextNode.State, Goal.DesiredState);
            NextNode.Depth = Current.Depth + 1;

            OpenSet.HeapPush(NextNode, NodePredicate);
        }
    }

    return Result;
}

bool UFootballGOAPPlanner::CanExecuteAction(const FGOAPAction& Action, const FGOAPWorldState& CurrentState) const
{
    // Check preconditions
    if (Action.Preconditions.bHasBall && !CurrentState.bHasBall)
        return false;
    if (Action.Preconditions.bBallInAir && !CurrentState.bBallInAir)
        return false;
    if (Action.Preconditions.bBallInPossession && !CurrentState.bBallInPossession)
        return false;
    if (Action.Preconditions.bBallNearGoal && !CurrentState.bBallNearGoal)
        return false;
    if (Action.Preconditions.bBallInDefensiveThird && !CurrentState.bBallInDefensiveThird)
        return false;
    if (Action.Preconditions.bBallInAttackingThird && !CurrentState.bBallInAttackingThird)
        return false;
    if (Action.Preconditions.bNearBall && !CurrentState.bNearBall)
        return false;
    if (Action.Preconditions.bNearGoal && !CurrentState.bNearGoal)
        return false;
    if (Action.Preconditions.bMarked && !CurrentState.bMarked)
        return false;
    if (Action.Preconditions.bHasSupport && !CurrentState.bHasSupport)
        return false;
    if (Action.Preconditions.bCanShoot && !CurrentState.bCanShoot)
        return false;
    if (Action.Preconditions.bCanPass && !CurrentState.bCanPass)
        return false;
    if (Action.Preconditions.bCanIntercept && !CurrentState.bCanIntercept)
        return false;
    if (Action.Preconditions.bCanTackle && !CurrentState.bCanTackle)
        return false;
    if (Action.Preconditions.bFatigued && !CurrentState.bFatigued)
        return false;
    if (Action.Preconditions.bInPosition && !CurrentState.bInPosition)
        return false;
    if (Action.Preconditions.bTeamHasPossession && !CurrentState.bTeamHasPossession)
        return false;
    if (Action.Preconditions.bTeamAttacking && !CurrentState.bTeamAttacking)
        return false;
    if (Action.Preconditions.bTeamDefending && !CurrentState.bTeamDefending)
        return false;
    if (Action.Preconditions.bCounterAttackAvailable && !CurrentState.bCounterAttackAvailable)
        return false;
    if (Action.Preconditions.bSetPieceActive && !CurrentState.bSetPieceActive)
        return false;

    return true;
}

FGOAPWorldState UFootballGOAPPlanner::ApplyAction(const FGOAPAction& Action, const FGOAPWorldState& CurrentState) const
{
    FGOAPWorldState NewState = CurrentState;

    // Apply effects (set to true)
    if (Action.Effects.bHasBall)
        NewState.bHasBall = true;
    if (Action.Effects.bBallInAir)
        NewState.bBallInAir = true;
    if (Action.Effects.bBallInPossession)
        NewState.bBallInPossession = true;
    if (Action.Effects.bBallNearGoal)
        NewState.bBallNearGoal = true;
    if (Action.Effects.bBallInDefensiveThird)
        NewState.bBallInDefensiveThird = true;
    if (Action.Effects.bBallInAttackingThird)
        NewState.bBallInAttackingThird = true;
    if (Action.Effects.bNearBall)
        NewState.bNearBall = true;
    if (Action.Effects.bNearGoal)
        NewState.bNearGoal = true;
    if (Action.Effects.bMarked)
        NewState.bMarked = true;
    if (Action.Effects.bHasSupport)
        NewState.bHasSupport = true;
    if (Action.Effects.bCanShoot)
        NewState.bCanShoot = true;
    if (Action.Effects.bCanPass)
        NewState.bCanPass = true;
    if (Action.Effects.bCanIntercept)
        NewState.bCanIntercept = true;
    if (Action.Effects.bCanTackle)
        NewState.bCanTackle = true;
    if (Action.Effects.bFatigued)
        NewState.bFatigued = true;
    if (Action.Effects.bInPosition)
        NewState.bInPosition = true;
    if (Action.Effects.bTeamHasPossession)
        NewState.bTeamHasPossession = true;
    if (Action.Effects.bTeamAttacking)
        NewState.bTeamAttacking = true;
    if (Action.Effects.bTeamDefending)
        NewState.bTeamDefending = true;
    if (Action.Effects.bCounterAttackAvailable)
        NewState.bCounterAttackAvailable = true;
    if (Action.Effects.bSetPieceActive)
        NewState.bSetPieceActive = true;

    // Note: We don't handle negative effects (setting to false) in this simple
    // version A full implementation would track which effects are "unset"
    // operations

    return NewState;
}

FGOAPWorldState UFootballGOAPPlanner::EvaluateWorldState(AFootballPlayerCharacter* Character,
                                                         UFootballBallComponent* Ball, AFootballTeam* Team,
                                                         AFootballTeam* OpponentTeam) const
{
    FGOAPWorldState State;

    if (!Character || !Ball || !Team)
    {
        return State;
    }

    // Ball state
    State.bHasBall = Character->HasBallPossession();
    State.bBallInAir = Ball->GetBallState() == EFootballBallState::Flying;
    State.bBallInPossession = Ball->GetBallState() == EFootballBallState::Possessed;

    float DistToBall = FVector::Dist(Character->GetActorLocation(), Ball->GetComponentLocation());
    State.bNearBall = DistToBall < 300.0f;

    // Ball position relative to field
    FVector BallPos = Ball->GetComponentLocation();
    EFootballTeamSide::Type Side = Character->GetTeamSide();

    if (Side == EFootballTeamSide::Home)
    {
        State.bBallInAttackingThird = BallPos.X > 2000.0f;
        State.bBallInDefensiveThird = BallPos.X < -2000.0f;
        State.bBallNearGoal = BallPos.X > 4500.0f;
        State.bNearGoal = Character->GetActorLocation().X > 4500.0f;
    }
    else
    {
        State.bBallInAttackingThird = BallPos.X < -2000.0f;
        State.bBallInDefensiveThird = BallPos.X > 2000.0f;
        State.bBallNearGoal = BallPos.X < -4500.0f;
        State.bNearGoal = Character->GetActorLocation().X < -4500.0f;
    }

    // Player state
    State.bMarked = false;     // Would check if marked by opponent
    State.bHasSupport = false; // Would check for nearby teammates
    State.bCanShoot = State.bNearGoal && State.bHasBall;
    State.bCanPass = State.bHasBall && !State.bFatigued;
    State.bCanIntercept = false; // Would use interception evaluator
    State.bCanTackle = !State.bHasBall && State.bNearBall;
    State.bFatigued = Character->IsFatigued();
    State.bInPosition = true; // Would check formation position

    // Team state
    State.bTeamHasPossession = Team->HasPossession();
    State.bTeamAttacking = State.bTeamHasPossession;
    State.bTeamDefending = !State.bTeamHasPossession;
    State.bCounterAttackAvailable = !State.bTeamHasPossession && State.bBallInDefensiveThird;
    State.bSetPieceActive = false; // Would check match phase

    return State;
}

TArray<FGOAPAction> UFootballGOAPPlanner::GetDefaultActions() const
{
    TArray<FGOAPAction> Actions;

    // Pass Action
    FGOAPAction PassAction(TEXT("Pass"), FFootballTags::Action_Pass_Short, EGOAPActionCost::Low);
    PassAction.Preconditions.bHasBall = true;
    PassAction.Preconditions.bCanPass = true;
    PassAction.Effects.bBallInPossession = false;
    PassAction.Effects.bHasBall = false;
    PassAction.Effects.bTeamHasPossession = true;
    Actions.Add(PassAction);

    // Long Pass Action
    FGOAPAction LongPassAction(TEXT("LongPass"), FFootballTags::Action_Pass_Long, EGOAPActionCost::Medium);
    LongPassAction.Preconditions.bHasBall = true;
    LongPassAction.Preconditions.bCanPass = true;
    LongPassAction.Preconditions.bBallInDefensiveThird = true;
    LongPassAction.Effects.bBallInPossession = false;
    LongPassAction.Effects.bHasBall = false;
    LongPassAction.Effects.bBallInAttackingThird = true;
    LongPassAction.Effects.bTeamHasPossession = true;
    Actions.Add(LongPassAction);

    // Shoot Action
    FGOAPAction ShootAction(TEXT("Shoot"), FFootballTags::Action_Shoot_Power, EGOAPActionCost::Medium);
    ShootAction.Preconditions.bHasBall = true;
    ShootAction.Preconditions.bCanShoot = true;
    ShootAction.Preconditions.bBallNearGoal = true;
    ShootAction.Effects.bBallInPossession = false;
    ShootAction.Effects.bHasBall = false;
    ShootAction.Effects.bBallInAir = true;
    Actions.Add(ShootAction);

    // Dribble Action
    FGOAPAction DribbleAction(TEXT("Dribble"), FFootballTags::Action_Dribble, EGOAPActionCost::Low);
    DribbleAction.Preconditions.bHasBall = true;
    DribbleAction.Effects.bNearGoal = true;
    DribbleAction.Effects.bBallInAttackingThird = true;
    Actions.Add(DribbleAction);

    // Intercept Action
    FGOAPAction InterceptAction(TEXT("Intercept"), FFootballTags::Action_Intercept, EGOAPActionCost::Medium);
    InterceptAction.Preconditions.bCanIntercept = true;
    InterceptAction.Preconditions.bBallInAir = true;
    InterceptAction.Effects.bHasBall = true;
    InterceptAction.Effects.bBallInPossession = true;
    InterceptAction.Effects.bTeamHasPossession = true;
    Actions.Add(InterceptAction);

    // Tackle Action
    FGOAPAction TackleAction(TEXT("Tackle"), FFootballTags::Action_Tackle_Standing, EGOAPActionCost::High);
    TackleAction.Preconditions.bCanTackle = true;
    TackleAction.Preconditions.bNearBall = true;
    TackleAction.Effects.bHasBall = true;
    TackleAction.Effects.bBallInPossession = true;
    TackleAction.Effects.bTeamHasPossession = true;
    Actions.Add(TackleAction);

    // Move to Ball Action
    FGOAPAction MoveToBallAction(TEXT("MoveToBall"), FGameplayTag::RequestGameplayTag("Action.MoveToBall"),
                                 EGOAPActionCost::Low);
    MoveToBallAction.Preconditions.bNearBall = false;
    MoveToBallAction.Preconditions.bBallInPossession = false;
    MoveToBallAction.Effects.bNearBall = true;
    Actions.Add(MoveToBallAction);

    // Move to Position Action
    FGOAPAction MoveToPositionAction(TEXT("MoveToPosition"), FGameplayTag::RequestGameplayTag("Action.MoveToPosition"),
                                     EGOAPActionCost::Low);
    MoveToPositionAction.Preconditions.bInPosition = false;
    MoveToPositionAction.Effects.bInPosition = true;
    Actions.Add(MoveToPositionAction);

    // Press Action
    FGOAPAction PressAction(TEXT("Press"), FGameplayTag::RequestGameplayTag("Action.Press"), EGOAPActionCost::Low);
    PressAction.Preconditions.bTeamDefending = true;
    PressAction.Preconditions.bBallInPossession = true;
    PressAction.Preconditions.bNearBall = false;
    PressAction.Effects.bCanIntercept = true;
    PressAction.Effects.bCanTackle = true;
    Actions.Add(PressAction);

    // Support Action
    FGOAPAction SupportAction(TEXT("Support"), FGameplayTag::RequestGameplayTag("Action.Support"),
                              EGOAPActionCost::Low);
    SupportAction.Preconditions.bTeamAttacking = true;
    SupportAction.Preconditions.bHasBall = false;
    SupportAction.Effects.bHasSupport = true;
    Actions.Add(SupportAction);

    // Mark Action
    FGOAPAction MarkAction(TEXT("Mark"), FGameplayTag::RequestGameplayTag("Action.Mark"), EGOAPActionCost::Low);
    MarkAction.Preconditions.bTeamDefending = true;
    MarkAction.Preconditions.bHasBall = false;
    MarkAction.Effects.bMarked = true;
    Actions.Add(MarkAction);

    // Counter Attack Action
    FGOAPAction CounterAttackAction(TEXT("CounterAttack"), FGameplayTag::RequestGameplayTag("Action.CounterAttack"),
                                    EGOAPActionCost::Medium);
    CounterAttackAction.Preconditions.bCounterAttackAvailable = true;
    CounterAttackAction.Preconditions.bHasBall = true;
    CounterAttackAction.Effects.bBallInAttackingThird = true;
    CounterAttackAction.Effects.bNearGoal = true;
    Actions.Add(CounterAttackAction);

    return Actions;
}

TArray<FGOAPGoal> UFootballGOAPPlanner::GetDefaultGoals() const
{
    TArray<FGOAPGoal> Goals;

    // Score Goal - Highest priority
    FGOAPGoal ScoreGoal(TEXT("Score"), FGameplayTag::RequestGameplayTag("Goal.Score"), 10.0f);
    ScoreGoal.DesiredState.bBallNearGoal = true;
    ScoreGoal.DesiredState.bBallInAir = true;
    ScoreGoal.DesiredState.bHasBall = false; // Ball in net
    Goals.Add(ScoreGoal);

    // Create Chance
    FGOAPGoal CreateChanceGoal(TEXT("CreateChance"), FGameplayTag::RequestGameplayTag("Goal.CreateChance"), 8.0f);
    CreateChanceGoal.DesiredState.bBallInAttackingThird = true;
    CreateChanceGoal.DesiredState.bHasSupport = true;
    CreateChanceGoal.DesiredState.bCanShoot = true;
    Goals.Add(CreateChanceGoal);

    // Maintain Possession
    FGOAPGoal PossessionGoal(TEXT("Possession"), FGameplayTag::RequestGameplayTag("Goal.Possession"), 6.0f);
    PossessionGoal.DesiredState.bTeamHasPossession = true;
    PossessionGoal.DesiredState.bHasBall = true;
    Goals.Add(PossessionGoal);

    // Regain Possession
    FGOAPGoal RegainGoal(TEXT("RegainPossession"), FGameplayTag::RequestGameplayTag("Goal.RegainPossession"), 7.0f);
    RegainGoal.DesiredState.bTeamHasPossession = true;
    RegainGoal.DesiredState.bHasBall = true;
    RegainGoal.DesiredState.bCanIntercept = true;
    Goals.Add(RegainGoal);

    // Defend Goal
    FGOAPGoal DefendGoal(TEXT("Defend"), FGameplayTag::RequestGameplayTag("Goal.Defend"), 9.0f);
    DefendGoal.DesiredState.bTeamDefending = true;
    DefendGoal.DesiredState.bBallInDefensiveThird = false;
    DefendGoal.DesiredState.bMarked = true;
    Goals.Add(DefendGoal);

    // Hold Position
    FGOAPGoal HoldPositionGoal(TEXT("HoldPosition"), FGameplayTag::RequestGameplayTag("Goal.HoldPosition"), 4.0f);
    HoldPositionGoal.DesiredState.bInPosition = true;
    Goals.Add(HoldPositionGoal);

    // Counter Attack
    FGOAPGoal CounterGoal(TEXT("CounterAttack"), FGameplayTag::RequestGameplayTag("Goal.CounterAttack"), 8.0f);
    CounterGoal.DesiredState.bCounterAttackAvailable = true;
    CounterGoal.DesiredState.bBallInAttackingThird = true;
    Goals.Add(CounterGoal);

    return Goals;
}

float UFootballGOAPPlanner::CalculateHeuristic(const FGOAPWorldState& Current, const FGOAPWorldState& Goal) const
{
    // Count unsatisfied conditions
    float Cost = 0.0f;

    if (Goal.bHasBall && !Current.bHasBall)
        Cost += 1.0f;
    if (Goal.bBallInAir && !Current.bBallInAir)
        Cost += 1.0f;
    if (Goal.bBallInPossession && !Current.bBallInPossession)
        Cost += 1.0f;
    if (Goal.bBallNearGoal && !Current.bBallNearGoal)
        Cost += 2.0f;
    if (Goal.bBallInDefensiveThird && !Current.bBallInDefensiveThird)
        Cost += 1.0f;
    if (Goal.bBallInAttackingThird && !Current.bBallInAttackingThird)
        Cost += 1.0f;
    if (Goal.bNearBall && !Current.bNearBall)
        Cost += 1.0f;
    if (Goal.bNearGoal && !Current.bNearGoal)
        Cost += 2.0f;
    if (Goal.bMarked && !Current.bMarked)
        Cost += 1.0f;
    if (Goal.bHasSupport && !Current.bHasSupport)
        Cost += 1.0f;
    if (Goal.bCanShoot && !Current.bCanShoot)
        Cost += 2.0f;
    if (Goal.bCanPass && !Current.bCanPass)
        Cost += 1.0f;
    if (Goal.bCanIntercept && !Current.bCanIntercept)
        Cost += 1.0f;
    if (Goal.bCanTackle && !Current.bCanTackle)
        Cost += 1.0f;
    if (Goal.bFatigued && !Current.bFatigued)
        Cost += 1.0f;
    if (Goal.bInPosition && !Current.bInPosition)
        Cost += 1.0f;
    if (Goal.bTeamHasPossession && !Current.bTeamHasPossession)
        Cost += 2.0f;
    if (Goal.bTeamAttacking && !Current.bTeamAttacking)
        Cost += 1.0f;
    if (Goal.bTeamDefending && !Current.bTeamDefending)
        Cost += 1.0f;
    if (Goal.bCounterAttackAvailable && !Current.bCounterAttackAvailable)
        Cost += 2.0f;
    if (Goal.bSetPieceActive && !Current.bSetPieceActive)
        Cost += 1.0f;

    return Cost;
}

TArray<FGOAPAction> UFootballGOAPPlanner::GetApplicableActions(const FGOAPWorldState& State,
                                                               const TArray<FGOAPAction>& AllActions) const
{
    TArray<FGOAPAction> Result;

    for (const FGOAPAction& Action : AllActions)
    {
        if (CanExecuteAction(Action, State))
        {
            Result.Add(Action);
        }
    }

    return Result;
}

// UFootballGOAPComponent
UFootballGOAPComponent::UFootballGOAPComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;

    Planner = CreateDefaultSubobject<UFootballGOAPPlanner>(TEXT("GOAPPlanner"));
}

void UFootballGOAPComponent::BeginPlay()
{
    Super::BeginPlay();

    Character = Cast<AFootballPlayerCharacter>(GetOwner());
    if (Character)
    {
        Team = Character->GetTeam();
        if (AFootballTeamManager* Manager = Team ? Team->GetTeamManager() : nullptr)
        {
            Ball = Manager->GetBall();
            OpponentTeam =
                (Team->GetTeamSide() == EFootballTeamSide::Home) ? Manager->GetAwayTeam() : Manager->GetHomeTeam();
        }
        Tactical = Character->FindComponentByClass<UFootballTacticalComponent>();
    }

    // Load default goals and actions if not set
    if (Goals.Num() == 0)
    {
        Goals = Planner->GetDefaultGoals();
    }
}

void UFootballGOAPComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                           FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!Character || !Ball || !Team)
    {
        return;
    }

    TimeSinceLastPlan += DeltaTime;

    // Request new plan periodically
    if (TimeSinceLastPlan >= ReplanInterval || !IsPlanValid())
    {
        RequestPlan();
        TimeSinceLastPlan = 0.0f;
    }

    // Execute current plan
    if (bExecutingPlan && CurrentPlan.bValid && CurrentPlan.CurrentActionIndex < CurrentPlan.Actions.Num())
    {
        ExecuteCurrentAction(DeltaTime);
    }
}

void UFootballGOAPComponent::RequestPlan()
{
    if (!Character || !Ball || !Team || !Planner)
    {
        return;
    }

    FGOAPWorldState CurrentState = Planner->EvaluateWorldState(Character, Ball, Team, OpponentTeam);

    // Combine default actions with custom actions
    TArray<FGOAPAction> AllActions = Planner->GetDefaultActions();
    AllActions.Append(CustomActions);

    CurrentPlan = Planner->Plan(CurrentState, Goals, AllActions);

    if (CurrentPlan.bValid)
    {
        CurrentPlan.CurrentActionIndex = 0;
        bExecutingPlan = true;
        LastWorldState = CurrentState;
    }
    else
    {
        bExecutingPlan = false;
    }
}

void UFootballGOAPComponent::ExecuteCurrentAction(float DeltaTime)
{
    if (CurrentPlan.CurrentActionIndex >= CurrentPlan.Actions.Num())
    {
        bExecutingPlan = false;
        return;
    }

    const FGOAPAction& CurrentAction = CurrentPlan.Actions[CurrentPlan.CurrentActionIndex];

    // Execute action via character
    bool bActionComplete = false;
    bool bActionSuccess = true;

    if (CurrentAction.ActionTag == FFootballTags::Action_Pass_Short ||
        CurrentAction.ActionTag == FFootballTags::Action_Pass_Long ||
        CurrentAction.ActionTag == FFootballTags::Action_Pass_Through)
    {
        // Find pass target
        if (Tactical)
        {
            AFootballPlayerCharacter* Target = Tactical->FindBestPassTarget(Character);
            if (Target)
            {
                Character->PassBall(Target, 0.8f, CurrentAction.ActionTag);
                bActionComplete = true;
            }
            else
            {
                bActionSuccess = false;
            }
        }
    }
    else if (CurrentAction.ActionTag == FFootballTags::Action_Shoot_Power ||
             CurrentAction.ActionTag == FFootballTags::Action_Shoot_Placement ||
             CurrentAction.ActionTag == FFootballTags::Action_Shoot_Chip)
    {
        // Shoot at goal
        EFootballTeamSide::Type Side = Character->GetTeamSide();
        FVector GoalCenter = (Side == EFootballTeamSide::Home) ? FVector(5200, 0, 100) : FVector(-5200, 0, 100);
        Character->ShootAtGoal(GoalCenter, 0.9f, 0.0f, CurrentAction.ActionTag);
        bActionComplete = true;
    }
    else if (CurrentAction.ActionTag == FFootballTags::Action_Dribble)
    {
        Character->SetMovementMode(EFootballMovementMode::Dribbling);
        bActionComplete = true; // Continuous action
    }
    else if (CurrentAction.ActionTag == FFootballTags::Action_Intercept)
    {
        Character->Intercept();
        bActionComplete = true;
    }
    else if (CurrentAction.ActionTag == FFootballTags::Action_Tackle_Standing ||
             CurrentAction.ActionTag == FFootballTags::Action_Tackle_Sliding)
    {
        Character->Tackle(CurrentAction.ActionTag);
        bActionComplete = true;
    }
    else if (CurrentAction.ActionTag == FGameplayTag::RequestGameplayTag("Action.MoveToBall"))
    {
        if (Ball && Ball->GetBallState() != EFootballBallState::Possessed)
        {
            AAIController* AIController = Character->GetController<AAIController>();
            if (AIController)
            {
                FAIMoveRequest MoveRequest;
                MoveRequest.SetGoalLocation(Ball->GetComponentLocation());
                MoveRequest.SetAcceptanceRadius(150.0f);
                AIController->MoveTo(MoveRequest);
            }
        }
        bActionComplete = true;
    }
    else if (CurrentAction.ActionTag == FGameplayTag::RequestGameplayTag("Action.MoveToPosition"))
    {
        if (Tactical)
        {
            FVector Pos = Tactical->CalculateDefensivePosition(Character);
            Tactical->SetPlayerTargetPosition(Character, Pos);
        }
        bActionComplete = true;
    }
    else if (CurrentAction.ActionTag == FGameplayTag::RequestGameplayTag("Action.Press"))
    {
        Character->Intercept(); // Use intercept for pressing
        bActionComplete = true;
    }
    else if (CurrentAction.ActionTag == FGameplayTag::RequestGameplayTag("Action.Support"))
    {
        if (Tactical)
        {
            FVector SupportPos = Tactical->CalculateSupportPosition(Character);
            Tactical->SetPlayerTargetPosition(Character, SupportPos);
        }
        bActionComplete = true;
    }
    else if (CurrentAction.ActionTag == FGameplayTag::RequestGameplayTag("Action.Mark"))
    {
        // Marking handled by tactical component
        bActionComplete = true;
    }
    else if (CurrentAction.ActionTag == FGameplayTag::RequestGameplayTag("Action.CounterAttack"))
    {
        Character->StartSprint();
        bActionComplete = true;
    }

    OnActionCompleted(bActionSuccess);

    if (bActionComplete && bActionSuccess)
    {
        CurrentPlan.CurrentActionIndex++;
    }
    else if (!bActionSuccess)
    {
        // Action failed, replan
        bExecutingPlan = false;
    }
}

void UFootballGOAPComponent::AbortCurrentPlan()
{
    bExecutingPlan = false;
    CurrentPlan.Actions.Empty();
    CurrentPlan.bValid = false;
}

bool UFootballGOAPComponent::IsPlanValid() const
{
    if (!CurrentPlan.bValid || CurrentPlan.Actions.Num() == 0)
    {
        return false;
    }

    // Check if current action is still applicable
    if (CurrentPlan.CurrentActionIndex < CurrentPlan.Actions.Num())
    {
        FGOAPWorldState CurrentState = Planner->EvaluateWorldState(Character, Ball, Team, OpponentTeam);
        const FGOAPAction& Action = CurrentPlan.Actions[CurrentPlan.CurrentActionIndex];

        if (!Planner->CanExecuteAction(Action, CurrentState))
        {
            return false;
        }
    }

    return true;
}

void UFootballGOAPComponent::OnActionCompleted(bool bSuccess)
{
    if (!bSuccess)
    {
        bExecutingPlan = false;
    }
}

void UFootballGOAPComponent::UpdateWorldState()
{
    LastWorldState = Planner->EvaluateWorldState(Character, Ball, Team, OpponentTeam);
}

void UFootballGOAPComponent::ExecuteAction(float DeltaTime)
{
    ExecuteCurrentAction(DeltaTime);
}