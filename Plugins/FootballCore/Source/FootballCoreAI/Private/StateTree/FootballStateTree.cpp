// Copyright 2024 Craquez. All Rights Reserved.

#include "StateTree/FootballStateTree.h"
#include "AIController.h"
#include "BallPhysics/FootballBallComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h"
#include "Player/FootballPlayerCharacter.h"
#include "StateTreeExecutionContext.h"
#include "Tactical/FootballTacticalComponent.h"
#include "Team/FootballTeam.h"
#include "Team/FootballTeamManager.h"

// Helper to get context data
#define GET_CONTEXT(Context) FFootballStateTreeContext& Ctx = FootballContext

// UFootballStateTreeEvaluator
void UFootballStateTreeEvaluator::Tick(FStateTreeExecutionContext& Context, const float DeltaTime)
{
    Super::Tick(Context, DeltaTime);
}

// UBallStateEvaluator
void UBallStateEvaluator::Tick(FStateTreeExecutionContext& Context, const float DeltaTime)
{
    Super::Tick(Context, DeltaTime);
    GET_CONTEXT(Context);

    if (Ctx.Ball)
    {
        Ctx.BallState = Ctx.Ball->GetBallState();
        Ctx.BallPosition = Ctx.Ball->GetComponentLocation();
        Ctx.BallVelocity = Ctx.Ball->GetVelocity();
        Ctx.BallPossessor = Ctx.Ball->GetPossessingPlayer();

        if (Ctx.Character)
        {
            Ctx.DistanceToBall = FVector::Dist(Ctx.Character->GetActorLocation(), Ctx.BallPosition);
            Ctx.bHasBall = (Ctx.BallPossessor == Ctx.Character);
        }
    }
}

// UTacticalEvaluator
void UTacticalEvaluator::Tick(FStateTreeExecutionContext& Context, const float DeltaTime)
{
    Super::Tick(Context, DeltaTime);
    GET_CONTEXT(Context);

    if (Ctx.Team)
    {
        Ctx.bTeamHasPossession = Ctx.Team->HasPossession();
        Ctx.TeamPossessionTime = Ctx.Team->GetPossessionTime();

        if (Ctx.Tactical)
        {
            Ctx.PressingIntensity =
                Ctx.Tactical->GetInstruction(FGameplayTag::RequestGameplayTag("Tactic.PressingIntensity", false));
            Ctx.DefensiveLine =
                Ctx.Tactical->GetInstruction(FGameplayTag::RequestGameplayTag("Tactic.DefensiveLine", false));
        }

        if (Ctx.Character)
        {
            Ctx.PlayerPosition = Ctx.Character->GetActorLocation();
            Ctx.PlayerVelocity = Ctx.Character->GetVelocity();
            Ctx.Stamina = Ctx.Character->GetStaminaPercent();
            Ctx.bIsFatigued = Ctx.Character->IsFatigued();
            Ctx.PlayerRole = Ctx.Character->GetPlayerRole();

            // Distance to goal
            EFootballTeamSide::Type Side = Ctx.Character->GetTeamSide();
            if (Side == EFootballTeamSide::Home)
            {
                Ctx.DistanceToGoal = FVector::Dist(Ctx.PlayerPosition, FVector(5200, 0, 0));
            }
            else
            {
                Ctx.DistanceToGoal = FVector::Dist(Ctx.PlayerPosition, FVector(-5200, 0, 0));
            }
        }
    }
}

// UInterceptionEvaluator - KEY FOR FIXING PRESS -> INTERCEPT TRANSITION
void UInterceptionEvaluator::Tick(FStateTreeExecutionContext& Context, const float DeltaTime)
{
    Super::Tick(Context, DeltaTime);
    GET_CONTEXT(Context);

    // Reset interception data
    Ctx.bCanIntercept = false;
    Ctx.InterceptConfidence = 0.0f;
    Ctx.PredictedInterceptPoint = FVector::ZeroVector;
    Ctx.TimeToIntercept = 0.0f;

    if (!Ctx.Ball || !Ctx.Character || !Ctx.Team)
    {
        return;
    }

    // Only evaluate interception when ball is in the air or rolling free
    if (Ctx.BallState != EFootballBallState::Flying && Ctx.BallState != EFootballBallState::Rolling)
    {
        return;
    }

    // Ball must not be possessed
    if (Ctx.BallState == EFootballBallState::Possessed)
    {
        return;
    }

    // Calculate interception using tactical component
    if (Ctx.Tactical)
    {
        FVector InterceptPoint = Ctx.Tactical->CalculateInterceptPoint(Ctx.Ball, Ctx.Character);
        float DistToIntercept = FVector::Dist(Ctx.Character->GetActorLocation(), InterceptPoint);

        if (DistToIntercept <= MaxInterceptDistance)
        {
            // Predict ball trajectory
            FVector BallPos = Ctx.BallPosition;
            FVector BallVel = Ctx.BallVelocity;
            float BallSpeed = BallVel.Size();

            if (BallSpeed > 100.0f) // Ball moving fast enough
            {
                // Time for ball to reach intercept point
                float BallTimeToPoint = FVector::Dist(BallPos, InterceptPoint) / BallSpeed;

                // Time for player to reach intercept point
                float PlayerSpeed = Ctx.Character->GetCharacterMovement()->MaxWalkSpeed;
                float PlayerTimeToPoint = PlayerSpeed > KINDA_SMALL_NUMBER ? (DistToIntercept / PlayerSpeed) : FLT_MAX;

                // Player must arrive before or at same time as ball
                float TimeDifference = BallTimeToPoint - PlayerTimeToPoint;

                if (TimeDifference >= -0.2f) // Small buffer for reaction time
                {
                    // Calculate confidence based on multiple factors
                    float DistanceFactor = 1.0f - FMath::Clamp(DistToIntercept / MaxInterceptDistance, 0.0f, 1.0f);
                    float TimeFactor = FMath::Clamp(TimeDifference / 1.0f, 0.0f, 1.0f);
                    float SpeedFactor = FMath::Clamp(PlayerSpeed / 700.0f, 0.0f, 1.0f);
                    float StaminaFactor = FMath::Clamp(Ctx.Stamina / 100.0f, 0.0f, 1.0f);
                    float ReactionFactor = FMath::Clamp(Ctx.Character->GetAttributes().Reactions / 100.0f, 0.0f, 1.0f);

                    // Role factor - defenders and midfielders better at intercepting
                    float RoleFactor = 0.5f;
                    switch (Ctx.PlayerRole)
                    {
                    case EFootballPlayerRole::CenterBack:
                    case EFootballPlayerRole::DefensiveMidfielder:
                        RoleFactor = 1.0f;
                        break;
                    case EFootballPlayerRole::CentralMidfielder:
                    case EFootballPlayerRole::LeftBack:
                    case EFootballPlayerRole::RightBack:
                        RoleFactor = 0.9f;
                        break;
                    case EFootballPlayerRole::AttackingMidfielder:
                        RoleFactor = 0.7f;
                        break;
                    default:
                        RoleFactor = 0.5f;
                        break;
                    }

                    // Tactical instruction factor
                    float InterceptionInstruction =
                        Ctx.Tactical->GetInstruction(FGameplayTag::RequestGameplayTag("Tactic.Interceptions", false));
                    float TacticalFactor = InterceptionInstruction / 100.0f;

                    // Combined confidence
                    Ctx.InterceptConfidence =
                        (DistanceFactor * 0.25f + TimeFactor * 0.25f + SpeedFactor * 0.15f + StaminaFactor * 0.10f +
                         RoleFactor * 0.15f + TacticalFactor * 0.05f + ReactionFactor * 0.05f);

                    Ctx.bCanIntercept = Ctx.InterceptConfidence >= MinInterceptConfidence;
                    Ctx.PredictedInterceptPoint = InterceptPoint;
                    Ctx.TimeToIntercept = PlayerTimeToPoint;
                }
            }
        }
    }
}

// UFootballStateTreeTask
EStateTreeRunStatus UFootballStateTreeTask::EnterState(FStateTreeExecutionContext& Context,
                                                       const FStateTreeTransitionResult& Transition)
{
    return Super::EnterState(Context, Transition);
}

EStateTreeRunStatus UFootballStateTreeTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime)
{
    return Super::Tick(Context, DeltaTime);
}

void UFootballStateTreeTask::ExitState(FStateTreeExecutionContext& Context,
                                       const FStateTreeTransitionResult& Transition)
{
    Super::ExitState(Context, Transition);
}

// UIdleTask
EStateTreeRunStatus UIdleTask::EnterState(FStateTreeExecutionContext& Context,
                                          const FStateTreeTransitionResult& Transition)
{
    GET_CONTEXT(Context);

    if (Ctx.Character)
    {
        Ctx.Character->SetMovementMode(EFootballMovementMode::Walking);
        Ctx.Character->SetCurrentActionState(FFootballTags::Player_Idle);
    }

    return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus UIdleTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime)
{
    return EStateTreeRunStatus::Running;
}

// UMoveToFormationTask
EStateTreeRunStatus UMoveToFormationTask::EnterState(FStateTreeExecutionContext& Context,
                                                     const FStateTreeTransitionResult& Transition)
{
    GET_CONTEXT(Context);

    if (Ctx.Character && Ctx.Tactical)
    {
        Ctx.Character->SetMovementMode(EFootballMovementMode::Jogging);
        Ctx.Character->SetCurrentActionState(FFootballTags::Player_Running);

        FVector FormationPos = Ctx.Tactical->CalculateDefensivePosition(Ctx.Character);
        Ctx.Tactical->SetPlayerTargetPosition(Ctx.Character, FormationPos);
    }

    return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus UMoveToFormationTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime)
{
    GET_CONTEXT(Context);

    if (Ctx.Character && Ctx.Tactical)
    {
        FVector FormationPos = Ctx.Tactical->CalculateDefensivePosition(Ctx.Character);
        Ctx.Tactical->SetPlayerTargetPosition(Ctx.Character, FormationPos);
        Ctx.Tactical->UpdatePlayerPosition(Ctx.Character, DeltaTime);

        float Distance = FVector::Dist(Ctx.Character->GetActorLocation(), FormationPos);
        if (Distance <= PositionTolerance)
        {
            return EStateTreeRunStatus::Succeeded;
        }
    }

    return EStateTreeRunStatus::Running;
}

// UPressTask - FIXED: Proper transition logic to Intercept
EStateTreeRunStatus UPressTask::EnterState(FStateTreeExecutionContext& Context,
                                           const FStateTreeTransitionResult& Transition)
{
    GET_CONTEXT(Context);

    if (Ctx.Character)
    {
        Ctx.Character->SetMovementMode(EFootballMovementMode::Jockeying);
        Ctx.Character->SetCurrentActionState(FFootballTags::Player_Intercepting);
    }

    return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus UPressTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime)
{
    GET_CONTEXT(Context);

    if (!Ctx.Character || !Ctx.Ball || !Ctx.Team || !Ctx.Tactical)
    {
        return EStateTreeRunStatus::Failed;
    }

    // Ball must be possessed by opponent
    if (Ctx.BallState != EFootballBallState::Possessed)
    {
        return EStateTreeRunStatus::Failed;
    }

    AActor* BallCarrier = Ctx.BallPossessor;
    if (!BallCarrier || BallCarrier == Ctx.Character)
    {
        return EStateTreeRunStatus::Failed;
    }

    // Calculate press position
    FVector CarrierPos = BallCarrier->GetActorLocation();
    FVector CarrierForward = BallCarrier->GetActorForwardVector();

    // Press from angle
    FVector PressDirection =
        CarrierForward.RotateAngleAxis(bForceWeakFoot ? PressAngle : -PressAngle, FVector::UpVector);
    FVector PressPos = CarrierPos + PressDirection * PressDistance;

    // Move to press position
    AAIController* AIController = Ctx.Character->GetController<AAIController>();
    if (AIController)
    {
        FAIMoveRequest MoveRequest;
        MoveRequest.SetGoalLocation(PressPos);
        MoveRequest.SetAcceptanceRadius(100.0f);
        AIController->MoveTo(MoveRequest);
    }

    // Check if close enough to attempt intercept
    float Distance = FVector::Dist(Ctx.Character->GetActorLocation(), CarrierPos);
    if (Distance <= 250.0f)
    {
        Ctx.Character->Intercept();
    }

    return EStateTreeRunStatus::Running;
}

// UInterceptTask - Properly handles interception execution
EStateTreeRunStatus UInterceptTask::EnterState(FStateTreeExecutionContext& Context,
                                               const FStateTreeTransitionResult& Transition)
{
    GET_CONTEXT(Context);

    if (Ctx.Character)
    {
        Ctx.Character->SetMovementMode(EFootballMovementMode::Sprinting);
        Ctx.Character->SetCurrentActionState(FFootballTags::Player_Intercepting);
        Ctx.Character->Intercept();
    }

    return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus UInterceptTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime)
{
    GET_CONTEXT(Context);

    if (!Ctx.Character || !Ctx.Ball || !Ctx.Tactical)
    {
        return EStateTreeRunStatus::Failed;
    }

    // Ball must be free/rolling for interception
    if (Ctx.BallState != EFootballBallState::Flying && Ctx.BallState != EFootballBallState::Rolling)
    {
        return EStateTreeRunStatus::Failed;
    }

    // Move to predicted intercept point
    if (Ctx.bCanIntercept && !Ctx.PredictedInterceptPoint.IsZero())
    {
        AAIController* AIController = Ctx.Character->GetController<AAIController>();
        if (AIController)
        {
            FAIMoveRequest MoveRequest;
            MoveRequest.SetGoalLocation(Ctx.PredictedInterceptPoint);
            MoveRequest.SetAcceptanceRadius(100.0f);
            AIController->MoveTo(MoveRequest);
        }

        // Check if we've reached intercept point
        float Distance = FVector::Dist(Ctx.Character->GetActorLocation(), Ctx.PredictedInterceptPoint);
        if (Distance <= 150.0f)
        {
            if (Ctx.Ball->GetBallState() == EFootballBallState::Free ||
                Ctx.Ball->GetBallState() == EFootballBallState::Rolling)
            {
                Ctx.Character->RequestPossession(Ctx.Ball);
                return EStateTreeRunStatus::Succeeded;
            }
        }
    }
    else
    {
        return EStateTreeRunStatus::Failed;
    }

    return EStateTreeRunStatus::Running;
}

// USupportTask
EStateTreeRunStatus USupportTask::EnterState(FStateTreeExecutionContext& Context,
                                             const FStateTreeTransitionResult& Transition)
{
    GET_CONTEXT(Context);

    if (Ctx.Character)
    {
        Ctx.Character->SetMovementMode(EFootballMovementMode::Jogging);
        Ctx.Character->SetCurrentActionState(FFootballTags::Player_Running);
    }

    return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus USupportTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime)
{
    GET_CONTEXT(Context);

    if (Ctx.Character && Ctx.Team && Ctx.Tactical)
    {
        AFootballPlayerCharacter* BallCarrier = Ctx.Team->GetPlayerWithBall();
        if (BallCarrier && BallCarrier != Ctx.Character)
        {
            FVector SupportPos = Ctx.Tactical->CalculateSupportPosition(Ctx.Character);
            Ctx.Tactical->SetPlayerTargetPosition(Ctx.Character, SupportPos);
            Ctx.Tactical->UpdatePlayerPosition(Ctx.Character, DeltaTime);
        }
    }

    return EStateTreeRunStatus::Running;
}

// UMarkTask
EStateTreeRunStatus UMarkTask::EnterState(FStateTreeExecutionContext& Context,
                                          const FStateTreeTransitionResult& Transition)
{
    GET_CONTEXT(Context);

    if (Ctx.Character)
    {
        Ctx.Character->SetMovementMode(EFootballMovementMode::Jockeying);
        Ctx.Character->SetCurrentActionState(FFootballTags::Player_Intercepting);
    }

    return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus UMarkTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime)
{
    GET_CONTEXT(Context);

    if (!Ctx.Character || !Ctx.Team || !Ctx.Ball)
    {
        return EStateTreeRunStatus::Failed;
    }

    AFootballPlayerCharacter* Target = nullptr;
    float MinDist = FLT_MAX;

    if (Ctx.OpponentTeam)
    {
        for (AFootballPlayerCharacter* Opponent : Ctx.OpponentTeam->GetAllPlayers())
        {
            if (Opponent)
            {
                float Dist = FVector::Dist(Ctx.Character->GetActorLocation(), Opponent->GetActorLocation());
                if (Dist < MinDist)
                {
                    MinDist = Dist;
                    Target = Opponent;
                }
            }
        }
    }

    if (Target)
    {
        FVector TargetPos = Target->GetActorLocation();
        FVector BallPos = Ctx.BallPosition;
        FVector MarkPos = TargetPos;

        if (bGoalSide)
        {
            EFootballTeamSide::Type OurSide = Ctx.Character->GetTeamSide();
            FVector GoalDirection = (OurSide == EFootballTeamSide::Home) ? FVector(1, 0, 0) : FVector(-1, 0, 0);
            FVector ToGoal = (TargetPos + GoalDirection * 10000.0f) - TargetPos;
            MarkPos = TargetPos + ToGoal.GetSafeNormal() * MarkDistance;
        }

        if (bBallSide)
        {
            FVector ToBall = (BallPos - TargetPos).GetSafeNormal();
            MarkPos = TargetPos + ToBall * MarkDistance;
        }

        AAIController* AIController = Ctx.Character->GetController<AAIController>();
        if (AIController)
        {
            FAIMoveRequest MoveRequest;
            MoveRequest.SetGoalLocation(MarkPos);
            MoveRequest.SetAcceptanceRadius(150.0f);
            AIController->MoveTo(MoveRequest);
        }
    }

    return EStateTreeRunStatus::Running;
}

// UAttackTask
EStateTreeRunStatus UAttackTask::EnterState(FStateTreeExecutionContext& Context,
                                            const FStateTreeTransitionResult& Transition)
{
    GET_CONTEXT(Context);

    if (Ctx.Character)
    {
        Ctx.Character->SetMovementMode(EFootballMovementMode::Sprinting);
        Ctx.Character->SetCurrentActionState(FFootballTags::Player_Sprinting);
    }

    return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus UAttackTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime)
{
    GET_CONTEXT(Context);

    if (Ctx.Character && Ctx.Team && Ctx.Tactical)
    {
        FVector AttackPos = Ctx.Tactical->CalculateAttackingPosition(Ctx.Character);
        Ctx.Tactical->SetPlayerTargetPosition(Ctx.Character, AttackPos);
        Ctx.Tactical->UpdatePlayerPosition(Ctx.Character, DeltaTime);
    }

    return EStateTreeRunStatus::Running;
}

// UDefendTask
EStateTreeRunStatus UDefendTask::EnterState(FStateTreeExecutionContext& Context,
                                            const FStateTreeTransitionResult& Transition)
{
    GET_CONTEXT(Context);

    if (Ctx.Character)
    {
        Ctx.Character->SetMovementMode(EFootballMovementMode::Jogging);
        Ctx.Character->SetCurrentActionState(FFootballTags::Player_Running);
    }

    return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus UDefendTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime)
{
    GET_CONTEXT(Context);

    if (Ctx.Character && Ctx.Team && Ctx.Tactical)
    {
        FVector DefendPos = Ctx.Tactical->CalculateDefensivePosition(Ctx.Character);
        Ctx.Tactical->SetPlayerTargetPosition(Ctx.Character, DefendPos);
        Ctx.Tactical->UpdatePlayerPosition(Ctx.Character, DeltaTime);
    }

    return EStateTreeRunStatus::Running;
}

// UGoalkeeperTask
EStateTreeRunStatus UGoalkeeperTask::EnterState(FStateTreeExecutionContext& Context,
                                                const FStateTreeTransitionResult& Transition)
{
    GET_CONTEXT(Context);

    if (Ctx.Character)
    {
        Ctx.Character->SetMovementMode(EFootballMovementMode::Walking);
        Ctx.Character->SetCurrentActionState(FFootballTags::Player_Goalkeeping);
    }

    return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus UGoalkeeperTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime)
{
    GET_CONTEXT(Context);

    if (!Ctx.Character || !Ctx.Ball || Ctx.Character->GetPlayerRole() != EFootballPlayerRole::Goalkeeper)
    {
        return EStateTreeRunStatus::Failed;
    }

    // Position on goal line based on ball position
    FVector BallPos = Ctx.BallPosition;
    FVector GKPos = Ctx.Character->GetActorLocation();

    EFootballTeamSide::Type Side = Ctx.Character->GetTeamSide();
    float GoalLineX = (Side == EFootballTeamSide::Home) ? -5000.0f : 5000.0f;

    // Follow ball laterally
    float TargetY = FMath::Clamp(BallPos.Y, -3000.0f, 3000.0f);
    FVector TargetPos = FVector(GoalLineX, TargetY, 100.0f);

    AAIController* AIController = Ctx.Character->GetController<AAIController>();
    if (AIController)
    {
        FAIMoveRequest MoveRequest;
        MoveRequest.SetGoalLocation(TargetPos);
        MoveRequest.SetAcceptanceRadius(200.0f);
        AIController->MoveTo(MoveRequest);
    }

    // Check for shots to save
    if (Ctx.BallState == EFootballBallState::Flying)
    {
        FVector BallVel = Ctx.BallVelocity;
        FVector ToBall = BallPos - GKPos;
        float Distance = ToBall.Size();

        if (Distance < 1000.0f && FVector::DotProduct(BallVel.GetSafeNormal(), ToBall.GetSafeNormal()) > 0.7f)
        {
            Ctx.Character->GoalkeeperDive((BallPos - GKPos).GetSafeNormal());
        }
    }

    return EStateTreeRunStatus::Running;
}

// USetPieceTask
EStateTreeRunStatus USetPieceTask::EnterState(FStateTreeExecutionContext& Context,
                                              const FStateTreeTransitionResult& Transition)
{
    GET_CONTEXT(Context);

    if (Ctx.Team && Ctx.Tactical)
    {
        Ctx.Tactical->ExecuteSetPieceRoutine(SetPieceType);
    }

    return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus USetPieceTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime)
{
    GET_CONTEXT(Context);

    if (Ctx.Ball && Ctx.BallState == EFootballBallState::Flying)
    {
        return EStateTreeRunStatus::Succeeded;
    }

    return EStateTreeRunStatus::Running;
}
