// Copyright 2024 Craquez. All Rights Reserved.

#include "BehaviorTree/FootballBTTask.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BTTask_MoveTo.h"
#include "Player/FootballPlayerCharacter.h"
#include "Team/FootballTeam.h"
#include "Team/FootballTeamManager.h"
#include "BallPhysics/FootballBallComponent.h"
#include "Tactical/FootballTacticalComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "NavigationSystem.h"

UFootballBTTask::UFootballBTTask()
{
    NodeName = "Football BT Task";
}

AFootballPlayerCharacter* UFootballBTTask::GetFootballCharacter(UBehaviorTreeComponent& OwnerComp) const
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (AIController)
    {
        return Cast<AFootballPlayerCharacter>(AIController->GetPawn());
    }
    return nullptr;
}

UFootballBallComponent* UFootballBTTask::GetBall(UBehaviorTreeComponent& OwnerComp) const
{
    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
    if (Blackboard && BallKey.SelectedKeyName.IsValid())
    {
        return Cast<UFootballBallComponent>(Blackboard->GetValueAsObject(BallKey.SelectedKeyName));
    }
    return nullptr;
}

AFootballTeam* UFootballBTTask::GetTeam(UBehaviorTreeComponent& OwnerComp) const
{
    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
    if (Blackboard && TeamKey.SelectedKeyName.IsValid())
    {
        return Cast<AFootballTeam>(Blackboard->GetValueAsObject(TeamKey.SelectedKeyName));
    }
    return nullptr;
}

AFootballTeam* UFootballBTTask::GetOpponentTeam(UBehaviorTreeComponent& OwnerComp) const
{
    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
    if (Blackboard && OpponentTeamKey.SelectedKeyName.IsValid())
    {
        return Cast<AFootballTeam>(Blackboard->GetValueAsObject(OpponentTeamKey.SelectedKeyName));
    }
    return nullptr;
}

// UBTTask_MoveToBall
UBTTask_MoveToBall::UBTTask_MoveToBall()
{
    NodeName = "Move To Ball";
}

EBTNodeResult::Type UBTTask_MoveToBall::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AFootballPlayerCharacter* Character = GetFootballCharacter(OwnerComp);
    UFootballBallComponent* Ball = GetBall(OwnerComp);

    if (!Character || !Ball)
    {
        return EBTNodeResult::Failed;
    }

    FVector BallLocation = Ball->GetComponentLocation();

    // Set movement mode
    Character->SetMovementMode(MovementMode);

    // Use MoveTo task
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (AIController)
    {
        FAIMoveRequest MoveRequest;
        MoveRequest.SetGoalLocation(BallLocation);
        MoveRequest.SetAcceptanceRadius(AcceptanceRadius);
        MoveRequest.SetStopOnOverlap(bStopOnOverlap);

        FNavPathSharedPtr Path;
        AIController->MoveTo(MoveRequest, &Path);

        return EBTNodeResult::InProgress;
    }

    return EBTNodeResult::Failed;
}

void UBTTask_MoveToBall::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    AFootballPlayerCharacter* Character = GetFootballCharacter(OwnerComp);
    UFootballBallComponent* Ball = GetBall(OwnerComp);

    if (!Character || !Ball)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    float Distance = FVector::Dist(Character->GetActorLocation(), Ball->GetComponentLocation());

    if (Distance <= AcceptanceRadius)
    {
        // Check if we can possess the ball
        if (Ball->GetBallState() == EFootballBallState::Free || Ball->GetBallState() == EFootballBallState::Rolling)
        {
            Character->RequestPossession(Ball);
            FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        }
        else if (Ball->GetBallState() == EFootballBallState::Possessed)
        {
            // Ball possessed by someone else, switch to intercept/press
            FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        }
    }
}

// UBTTask_InterceptBall
UBTTask_InterceptBall::UBTTask_InterceptBall()
{
    NodeName = "Intercept Ball";
}

EBTNodeResult::Type UBTTask_InterceptBall::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AFootballPlayerCharacter* Character = GetFootballCharacter(OwnerComp);
    UFootballBallComponent* Ball = GetBall(OwnerComp);
    AFootballTeam* Team = GetTeam(OwnerComp);

    if (!Character || !Ball || !Team)
    {
        return EBTNodeResult::Failed;
    }

    if (Ball->GetBallState() != EFootballBallState::Flying && Ball->GetBallState() != EFootballBallState::Rolling)
    {
        return EBTNodeResult::Failed;
    }

    // Calculate intercept point
    UFootballTacticalComponent* Tactical = Team->GetTacticalComponent();
    if (Tactical)
    {
        FVector InterceptPoint = Tactical->CalculateInterceptPoint(Ball, Character);
        float DistToIntercept = FVector::Dist(Character->GetActorLocation(), InterceptPoint);

        if (DistToIntercept <= MaxInterceptDistance)
        {
            Character->SetMovementMode(EFootballMovementMode::Sprinting);
            Character->Intercept();

            AAIController* AIController = OwnerComp.GetAIOwner();
            if (AIController)
            {
                FAIMoveRequest MoveRequest;
                MoveRequest.SetGoalLocation(InterceptPoint);
                MoveRequest.SetAcceptanceRadius(100.0f);
                AIController->MoveTo(MoveRequest);

                return EBTNodeResult::InProgress;
            }
        }
    }

    return EBTNodeResult::Failed;
}

void UBTTask_InterceptBall::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    AFootballPlayerCharacter* Character = GetFootballCharacter(OwnerComp);
    UFootballBallComponent* Ball = GetBall(OwnerComp);

    if (!Character || !Ball)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    // Check if ball is now possessable
    if (Ball->GetBallState() == EFootballBallState::Free || Ball->GetBallState() == EFootballBallState::Rolling)
    {
        float Distance = FVector::Dist(Character->GetActorLocation(), Ball->GetComponentLocation());
        if (Distance <= 200.0f)
        {
            Character->RequestPossession(Ball);
            FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        }
    }
    else if (Ball->GetBallState() == EFootballBallState::Possessed)
    {
        // Someone else got it
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
    }
}

// UBTTask_PassToTeammate
UBTTask_PassToTeammate::UBTTask_PassToTeammate()
{
    NodeName = "Pass To Teammate";
}

EBTNodeResult::Type UBTTask_PassToTeammate::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AFootballPlayerCharacter* Character = GetFootballCharacter(OwnerComp);
    AFootballTeam* Team = GetTeam(OwnerComp);

    if (!Character || !Team || !Character->HasBallPossession())
    {
        return EBTNodeResult::Failed;
    }

    // Find best pass target using tactical component
    UFootballTacticalComponent* Tactical = Team->GetTacticalComponent();
    if (Tactical)
    {
        AFootballPlayerCharacter* Target = Tactical->FindBestPassTarget(Character);
        if (Target)
        {
            float Distance = FVector::Dist(Character->GetActorLocation(), Target->GetActorLocation());

            if (Distance >= MinPassDistance && Distance <= MaxPassDistance)
            {
                // Check passing lane if required
                if (bRequireOpenLane)
                {
                    // Would check for opponents in passing lane
                }

                // Determine pass type based on distance
                FGameplayTag ActualPassType = PassType;
                if (Distance > 2500.0f)
                {
                    ActualPassType = FFootballTags::Action_Pass_Long;
                }
                else if (Distance > 1500.0f)
                {
                    ActualPassType = FFootballTags::Action_Pass_Through;
                }

                Character->PassBall(Target, 0.8f, ActualPassType);

                // Set blackboard target
                UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
                if (Blackboard && TargetPlayerKey.SelectedKeyName.IsValid())
                {
                    Blackboard->SetValueAsObject(TargetPlayerKey.SelectedKeyName, Target);
                }

                return EBTNodeResult::Succeeded;
            }
        }
    }

    return EBTNodeResult::Failed;
}

// UBTTask_ShootAtGoal
UBTTask_ShootAtGoal::UBTTask_ShootAtGoal()
{
    NodeName = "Shoot At Goal";
}

EBTNodeResult::Type UBTTask_ShootAtGoal::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AFootballPlayerCharacter* Character = GetFootballCharacter(OwnerComp);
    AFootballTeam* OpponentTeam = GetOpponentTeam(OwnerComp);

    if (!Character || !OpponentTeam || !Character->HasBallPossession())
    {
        return EBTNodeResult::Failed;
    }

    // Find goal position
    AFootballPlayerCharacter* OpponentGK = OpponentTeam->GetPlayerByRole(EFootballPlayerRole::Goalkeeper);
    FVector GoalCenter;

    if (OpponentGK)
    {
        GoalCenter = OpponentGK->GetActorLocation();
        GoalCenter.Z = 100.0f;
    }
    else
    {
        // Default goal position based on team side
        EFootballTeamSide::Type OurSide = Character->GetTeamSide();
        if (OurSide == EFootballTeamSide::Home)
        {
            GoalCenter = FVector(5200, 0, 100); // Away goal
        }
        else
        {
            GoalCenter = FVector(-5200, 0, 100); // Home goal
        }
    }

    float Distance = FVector::Dist(Character->GetActorLocation(), GoalCenter);

    if (Distance < MinShootDistance || Distance > MaxShootDistance)
    {
        return EBTNodeResult::Failed;
    }

    // Add variance for placement shots
    FVector TargetLocation = GoalCenter;
    if (bPreferPlacement || (ShotType == FFootballTags::Action_Shoot_Placement))
    {
        // Aim for corners
        float Variance = FMath::FRandRange(-GoalTargetVariance, GoalTargetVariance);
        TargetLocation.Y += Variance;

        // Prefer far post
        FVector ToGoal = (GoalCenter - Character->GetActorLocation()).GetSafeNormal();
        FVector Right = FVector::CrossProduct(ToGoal, FVector::UpVector);
        TargetLocation += Right * GoalTargetVariance * 0.5f;
    }

    FGameplayTag ActualShotType = ShotType;
    if (bPreferPlacement || Distance < 1500.0f)
    {
        ActualShotType = FFootballTags::Action_Shoot_Placement;
    }
    else if (Distance > 2500.0f)
    {
        ActualShotType = FFootballTags::Action_Shoot_Power;
    }

    Character->ShootAtGoal(TargetLocation, 0.9f, bPreferPlacement ? 0.5f : 0.0f, ActualShotType);

    return EBTNodeResult::Succeeded;
}

// UBTTask_TackleOpponent
UBTTask_TackleOpponent::UBTTask_TackleOpponent()
{
    NodeName = "Tackle Opponent";
}

EBTNodeResult::Type UBTTask_TackleOpponent::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AFootballPlayerCharacter* Character = GetFootballCharacter(OwnerComp);
    UFootballBallComponent* Ball = GetBall(OwnerComp);

    if (!Character || !Ball)
    {
        return EBTNodeResult::Failed;
    }

    // Check if opponent has ball nearby
    if (Ball->GetBallState() == EFootballBallState::Possessed)
    {
        AActor* Possessor = Ball->GetPossessingPlayer();
        if (Possessor && Possessor != Character)
        {
            float Distance = FVector::Dist(Character->GetActorLocation(), Possessor->GetActorLocation());

            if (Distance <= TackleRange)
            {
                // Calculate success chance based on attributes
                float TackleAttr = Character->GetAttributes().Tackling; // Would need this attribute
                float SuccessChance = FMath::Clamp(TackleAttr / 100.0f, 0.2f, 0.9f);

                if (SuccessChance >= MinTackleSuccessChance)
                {
                    Character->Tackle(TackleType);
                    return EBTNodeResult::Succeeded;
                }
            }
        }
    }

    return EBTNodeResult::Failed;
}

// UBTTask_MarkOpponent
UBTTask_MarkOpponent::UBTTask_MarkOpponent()
{
    NodeName = "Mark Opponent";
}

EBTNodeResult::Type UBTTask_MarkOpponent::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AFootballPlayerCharacter* Character = GetFootballCharacter(OwnerComp);
    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();

    if (!Character || !Blackboard || !TargetPlayerKey.SelectedKeyName.IsValid())
    {
        return EBTNodeResult::Failed;
    }

    AFootballPlayerCharacter* Target = Cast<AFootballPlayerCharacter>(Blackboard->GetValueAsObject(TargetPlayerKey.SelectedKeyName));
    if (!Target)
    {
        return EBTNodeResult::Failed;
    }

    Character->SetMovementMode(EFootballMovementMode::Jockeying);

    return EBTNodeResult::InProgress;
}

void UBTTask_MarkOpponent::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    AFootballPlayerCharacter* Character = GetFootballCharacter(OwnerComp);
    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();

    if (!Character || !Blackboard)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    AFootballPlayerCharacter* Target = Cast<AFootballPlayerCharacter>(Blackboard->GetValueAsObject(TargetPlayerKey.SelectedKeyName));
    UFootballBallComponent* Ball = GetBall(OwnerComp);

    if (!Target || !Ball)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    // Calculate marking position
    FVector TargetPos = Target->GetActorLocation();
    FVector BallPos = Ball->GetComponentLocation();

    FVector MarkPos = TargetPos;

    if (bGoalSide)
    {
        // Position between target and goal
        EFootballTeamSide::Type OurSide = Character->GetTeamSide();
        FVector GoalDirection = (OurSide == EFootballTeamSide::Home) ? FVector(1, 0, 0) : FVector(-1, 0, 0);
        FVector ToGoal = (TargetPos + GoalDirection * 10000.0f) - TargetPos;
        MarkPos = TargetPos + ToGoal.GetSafeNormal() * MarkDistance;
    }

    if (bBallSide)
    {
        // Also stay ball-side
        FVector ToBall = (BallPos - TargetPos).GetSafeNormal();
        MarkPos = TargetPos + ToBall * MarkDistance;
    }

    // Move to marking position
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (AIController)
    {
        FAIMoveRequest MoveRequest;
        MoveRequest.SetGoalLocation(MarkPos);
        MoveRequest.SetAcceptanceRadius(150.0f);
        AIController->MoveTo(MoveRequest);
    }
}

// UBTTask_MoveToFormationPosition
UBTTask_MoveToFormationPosition::UBTTask_MoveToFormationPosition()
{
    NodeName = "Move To Formation Position";
}

EBTNodeResult::Type UBTTask_MoveToFormationPosition::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AFootballPlayerCharacter* Character = GetFootballCharacter(OwnerComp);
    AFootballTeam* Team = GetTeam(OwnerComp);

    if (!Character || !Team)
    {
        return EBTNodeResult::Failed;
    }

    // Get formation position from tactical component
    UFootballTacticalComponent* Tactical = Team->GetTacticalComponent();
    if (Tactical)
    {
        FVector FormationPos = Tactical->CalculateDefensivePosition(Character);

        // Adjust for ball if enabled
        if (bAdjustForBall)
        {
            UFootballBallComponent* Ball = GetBall(OwnerComp);
            if (Ball)
            {
                float DistToBall = FVector::Dist(Character->GetActorLocation(), Ball->GetComponentLocation());
                if (DistToBall < BallInfluenceRadius)
                {
                    // Move slightly toward ball
                    FVector ToBall = (Ball->GetComponentLocation() - Character->GetActorLocation()).GetSafeNormal();
                    FormationPos += ToBall * 500.0f;
                }
            }
        }

        AAIController* AIController = OwnerComp.GetAIOwner();
        if (AIController)
        {
            FAIMoveRequest MoveRequest;
            MoveRequest.SetGoalLocation(FormationPos);
            MoveRequest.SetAcceptanceRadius(PositionTolerance);
            AIController->MoveTo(MoveRequest);

            return EBTNodeResult::InProgress;
        }
    }

    return EBTNodeResult::Failed;
}

void UBTTask_MoveToFormationPosition::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    AFootballPlayerCharacter* Character = GetFootballCharacter(OwnerComp);
    AFootballTeam* Team = GetTeam(OwnerComp);

    if (!Character || !Team)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    UFootballTacticalComponent* Tactical = Team->GetTacticalComponent();
    if (Tactical)
    {
        FVector FormationPos = Tactical->CalculateDefensivePosition(Character);
        float Distance = FVector::Dist(Character->GetActorLocation(), FormationPos);

        if (Distance <= PositionTolerance)
        {
            FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        }
    }
}

// UBTTask_PressBallCarrier
UBTTask_PressBallCarrier::UBTTask_PressBallCarrier()
{
    NodeName = "Press Ball Carrier";
}

EBTNodeResult::Type UBTTask_PressBallCarrier::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AFootballPlayerCharacter* Character = GetFootballCharacter(OwnerComp);
    UFootballBallComponent* Ball = GetBall(OwnerComp);

    if (!Character || !Ball || Ball->GetBallState() != EFootballBallState::Possessed)
    {
        return EBTNodeResult::Failed;
    }

    AActor* BallCarrier = Ball->GetPossessingPlayer();
    if (!BallCarrier || BallCarrier == Character)
    {
        return EBTNodeResult::Failed;
    }

    Character->SetMovementMode(EFootballMovementMode::Jockeying);

    return EBTNodeResult::InProgress;
}

void UBTTask_PressBallCarrier::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    AFootballPlayerCharacter* Character = GetFootballCharacter(OwnerComp);
    UFootballBallComponent* Ball = GetBall(OwnerComp);

    if (!Character || !Ball)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    AActor* BallCarrier = Ball->GetPossessingPlayer();
    if (!BallCarrier)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    // Position to press - angle from ball carrier's forward
    FVector CarrierPos = BallCarrier->GetActorLocation();
    FVector CarrierForward = BallCarrier->GetActorForwardVector();

    // Press from the side/front at an angle
    float AngleRad = FMath::DegreesToRadians(PressAngle);
    FVector PressDirection = CarrierForward.RotateAngleAxis(bForceWeakFoot ? 90.0f : -90.0f, FVector::UpVector);

    FVector PressPos = CarrierPos + PressDirection * PressDistance;

    AAIController* AIController = OwnerComp.GetAIOwner();
    if (AIController)
    {
        FAIMoveRequest MoveRequest;
        MoveRequest.SetGoalLocation(PressPos);
        MoveRequest.SetAcceptanceRadius(100.0f);
        AIController->MoveTo(MoveRequest);
    }

    // Check if close enough to tackle/intercept
    float Distance = FVector::Dist(Character->GetActorLocation(), CarrierPos);
    if (Distance <= 200.0f)
    {
        Character->Intercept();
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
    }
}

// UBTTask_SupportBallCarrier
UBTTask_SupportBallCarrier::UBTTask_SupportBallCarrier()
{
    NodeName = "Support Ball Carrier";
}

EBTNodeResult::Type UBTTask_SupportBallCarrier::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AFootballPlayerCharacter* Character = GetFootballCharacter(OwnerComp);
    AFootballTeam* Team = GetTeam(OwnerComp);

    if (!Character || !Team)
    {
        return EBTNodeResult::Failed;
    }

    AFootballPlayerCharacter* BallCarrier = Team->GetPlayerWithBall();
    if (!BallCarrier || BallCarrier == Character)
    {
        return EBTNodeResult::Failed;
    }

    Character->SetMovementMode(EFootballMovementMode::Jogging);

    return EBTNodeResult::InProgress;
}

void UBTTask_SupportBallCarrier::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    AFootballPlayerCharacter* Character = GetFootballCharacter(OwnerComp);
    AFootballTeam* Team = GetTeam(OwnerComp);

    if (!Character || !Team)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    AFootballPlayerCharacter* BallCarrier = Team->GetPlayerWithBall();
    if (!BallCarrier)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    // Calculate support position
    UFootballTacticalComponent* Tactical = Team->GetTacticalComponent();
    if (Tactical)
    {
        FVector SupportPos = Tactical->CalculateSupportPosition(Character);

        AAIController* AIController = OwnerComp.GetAIOwner();
        if (AIController)
        {
            FAIMoveRequest MoveRequest;
            MoveRequest.SetGoalLocation(SupportPos);
            MoveRequest.SetAcceptanceRadius(200.0f);
            AIController->MoveTo(MoveRequest);
        }
    }
}

// UBTTask_GoalkeeperSave
UBTTask_GoalkeeperSave::UBTTask_GoalkeeperSave()
{
    NodeName = "Goalkeeper Save";
}

EBTNodeResult::Type UBTTask_GoalkeeperSave::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AFootballPlayerCharacter* Character = GetFootballCharacter(OwnerComp);
    UFootballBallComponent* Ball = GetBall(OwnerComp);

    if (!Character || !Ball || Character->GetPlayerRole() != EFootballPlayerRole::Goalkeeper)
    {
        return EBTNodeResult::Failed;
    }

    // Only react to shots on target
    if (Ball->GetBallState() != EFootballBallState::Flying)
    {
        return EBTNodeResult::Failed;
    }

    FVector BallPos = Ball->GetComponentLocation();
    FVector BallVel = Ball->GetVelocity();
    FVector GKPos = Character->GetActorLocation();

    // Predict if ball is heading toward goal
    FVector ToGoal = BallVel.GetSafeNormal();
    float DotToGoal = FVector::DotProduct(ToGoal, (GKPos - BallPos).GetSafeNormal());

    if (DotToGoal < 0.5f) // Not heading toward GK
    {
        return EBTNodeResult::Failed;
    }

    // Predict ball position at goal line
    float TimeToGoal = FVector::Dist(BallPos, GKPos) / BallVel.Size();
    FVector PredictedPos = BallPos + BallVel * TimeToGoal;

    float DistanceToSave = FVector::Dist(GKPos, PredictedPos);

    if (DistanceToSave <= DiveRange)
    {
        // React after reaction time
        FTimerHandle TimerHandle;
        Character->GetWorldTimerManager().SetTimer(TimerHandle, [Character, PredictedPos, this]()
        {
            FVector Direction = (PredictedPos - Character->GetActorLocation()).GetSafeNormal();
            Character->GoalkeeperDive(Direction);
        }, ReactionTime, false);

        return EBTNodeResult::Succeeded;
    }

    return EBTNodeResult::Failed;
}

// UBTTask_GoalkeeperDistribute
UBTTask_GoalkeeperDistribute::UBTTask_GoalkeeperDistribute()
{
    NodeName = "Goalkeeper Distribute";
}

EBTNodeResult::Type UBTTask_GoalkeeperDistribute::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AFootballPlayerCharacter* Character = GetFootballCharacter(OwnerComp);
    AFootballTeam* Team = GetTeam(OwnerComp);

    if (!Character || !Team || Character->GetPlayerRole() != EFootballPlayerRole::Goalkeeper || !Character->HasBallPossession())
    {
        return EBTNodeResult::Failed;
    }

    // Find best distribution target
    AFootballPlayerCharacter* Target = nullptr;
    float BestScore = -1.0f;

    for (AFootballPlayerCharacter* Player : Team->GetAllPlayers())
    {
        if (Player && Player != Character && Player->GetPlayerRole() != EFootballPlayerRole::Goalkeeper)
        {
            float Distance = FVector::Dist(Character->GetActorLocation(), Player->GetActorLocation());

            // Prefer counter-attack targets
            float Score = 0.0f;
            if (Distance > MinKickDistance)
            {
                Score += 0.5f;
            }

            EFootballPlayerRole::Type Role = Player->GetPlayerRole();
            if (Role == EFootballPlayerRole::Striker || Role == EFootballPlayerRole::LeftWinger || Role == EFootballPlayerRole::RightWinger)
            {
                Score += 0.3f;
            }

            if (Score > BestScore)
            {
                BestScore = Score;
                Target = Player;
            }
        }
    }

    if (Target)
    {
        float Distance = FVector::Dist(Character->GetActorLocation(), Target->GetActorLocation());

        if (bPreferThrow || Distance < 3000.0f)
        {
            Character->GoalkeeperDistributeThrow(Target);
        }
        else
        {
            FVector Direction = (Target->GetActorLocation() - Character->GetActorLocation()).GetSafeNormal();
            Character->GoalkeeperDistributeKick(Direction, 0.8f);
        }

        return EBTNodeResult::Succeeded;
    }

    return EBTNodeResult::Failed;
}