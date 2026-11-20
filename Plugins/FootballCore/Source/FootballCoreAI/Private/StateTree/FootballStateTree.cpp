// Copyright 2024 Craquez. All Rights Reserved.

#include "StateTree/FootballStateTree.h"
#include "AI/StateTreeExecutionContext.h"
#include "BallPhysics/FootballBallComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "NavigationSystem.h"
#include "Player/FootballPlayerCharacter.h"
#include "Tactical/FootballTacticalComponent.h"
#include "Team/FootballTeam.h"
#include "Team/FootballTeamManager.h"

// Helper to get context data
#define GET_CONTEXT(Context)                                                   \
  FFootballStateTreeContext &Ctx =                                             \
      Context.GetContext<FFootballStateTreeContext>()

// UFootballStateTreeEvaluator
void UFootballStateTreeEvaluator::Evaluate(FStateTreeExecutionContext &Context,
                                           float DeltaTime) const {
  // Base evaluator does nothing
}

// UBallStateEvaluator
void UBallStateEvaluator::Evaluate(FStateTreeExecutionContext &Context,
                                   float DeltaTime) const {
  GET_CONTEXT(Context);

  if (Ctx.Ball) {
    Ctx.BallState = Ctx.Ball->GetBallState();
    Ctx.BallPosition = Ctx.Ball->GetComponentLocation();
    Ctx.BallVelocity = Ctx.Ball->GetVelocity();
    Ctx.BallPossessor = Ctx.Ball->GetPossessingPlayer();

    if (Ctx.Character) {
      Ctx.DistanceToBall =
          FVector::Dist(Ctx.Character->GetActorLocation(), Ctx.BallPosition);
      Ctx.bHasBall = (Ctx.BallPossessor == Ctx.Character);
    }
  }
}

// UTacticalEvaluator
void UTacticalEvaluator::Evaluate(FStateTreeExecutionContext &Context,
                                  float DeltaTime) const {
  GET_CONTEXT(Context);

  if (Ctx.Team) {
    Ctx.bTeamHasPossession = Ctx.Team->HasPossession();
    Ctx.TeamPossessionTime = Ctx.Team->GetPossessionTime();

    if (Ctx.Tactical) {
      Ctx.PressingIntensity = Ctx.Tactical->GetInstruction(
          FGameplayTag::RequestGameplayTag("Tactic.PressingIntensity"));
      Ctx.DefensiveLine = Ctx.Tactical->GetInstruction(
          FGameplayTag::RequestGameplayTag("Tactic.DefensiveLine"));
    }

    if (Ctx.Character) {
      Ctx.PlayerPosition = Ctx.Character->GetActorLocation();
      Ctx.PlayerVelocity = Ctx.Character->GetVelocity();
      Ctx.Stamina = Ctx.Character->GetStaminaPercent();
      Ctx.bIsFatigued = Ctx.Character->IsFatigued();
      Ctx.PlayerRole = Ctx.Character->GetPlayerRole();

      // Distance to goal
      EFootballTeamSide::Type Side = Ctx.Character->GetTeamSide();
      if (Side == EFootballTeamSide::Home) {
        Ctx.DistanceToGoal =
            FVector::Dist(Ctx.PlayerPosition, FVector(5200, 0, 0));
      } else {
        Ctx.DistanceToGoal =
            FVector::Dist(Ctx.PlayerPosition, FVector(-5200, 0, 0));
      }
    }
  }
}

// UInterceptionEvaluator - KEY FOR FIXING PRESS -> INTERCEPT TRANSITION
void UInterceptionEvaluator::Evaluate(FStateTreeExecutionContext &Context,
                                      float DeltaTime) const {
  GET_CONTEXT(Context);

  // Reset interception data
  Ctx.bCanIntercept = false;
  Ctx.InterceptConfidence = 0.0f;
  Ctx.PredictedInterceptPoint = FVector::ZeroVector;
  Ctx.TimeToIntercept = 0.0f;

  if (!Ctx.Ball || !Ctx.Character || !Ctx.Team) {
    return;
  }

  // Only evaluate interception when ball is in the air or rolling free
  if (Ctx.BallState != EFootballBallState::Flying &&
      Ctx.BallState != EFootballBallState::Rolling) {
    return;
  }

  // Ball must not be possessed
  if (Ctx.BallState == EFootballBallState::Possessed) {
    return;
  }

  // Calculate interception using tactical component
  if (Ctx.Tactical) {
    FVector InterceptPoint =
        Ctx.Tactical->CalculateInterceptPoint(Ctx.Ball, Ctx.Character);
    float DistToIntercept =
        FVector::Dist(Ctx.Character->GetActorLocation(), InterceptPoint);

    if (DistToIntercept <= MaxInterceptDistance) {
      // Predict ball trajectory
      FVector BallPos = Ctx.BallPosition;
      FVector BallVel = Ctx.BallVelocity;
      float BallSpeed = BallVel.Size();

      if (BallSpeed > 100.0f) // Ball moving fast enough
      {
        // Time for ball to reach intercept point
        float BallTimeToPoint =
            FVector::Dist(BallPos, InterceptPoint) / BallSpeed;

        // Time for player to reach intercept point
        float PlayerSpeed = Ctx.Character->GetCharacterMovement()->MaxWalkSpeed;
        float PlayerTimeToPoint = DistToIntercept / PlayerSpeed;

        // Player must arrive before or at same time as ball
        float TimeDifference = BallTimeToPoint - PlayerTimeToPoint;

        if (TimeDifference >= -0.2f) // Small buffer for reaction time
        {
          // Calculate confidence based on multiple factors
          float DistanceFactor =
              1.0f -
              FMath::Clamp(DistToIntercept / MaxInterceptDistance, 0.0f, 1.0f);
          float TimeFactor =
              FMath::Clamp(TimeDifference / 1.0f, 0.0f,
                           1.0f); // 1 second advantage = full confidence
          float SpeedFactor = FMath::Clamp(PlayerSpeed / 700.0f, 0.0f,
                                           1.0f); // Normalized to max sprint
          float StaminaFactor = FMath::Clamp(Ctx.Stamina / 100.0f, 0.0f, 1.0f);
          float ReactionFactor =
              1.0f -
              FMath::Clamp(Ctx.Character->GetAttributes().Reactions / 100.0f,
                           0.0f, 1.0f); // Lower reactions = lower confidence

          // Role factor - defenders and midfielders better at intercepting
          float RoleFactor = 0.5f;
          switch (Ctx.PlayerRole) {
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
          float InterceptionInstruction = Ctx.Tactical->GetInstruction(
              FGameplayTag::RequestGameplayTag("Tactic.Interceptions"));
          float TacticalFactor = InterceptionInstruction / 100.0f;

          // Combined confidence
          Ctx.InterceptConfidence =
              (DistanceFactor * 0.25f + TimeFactor * 0.25f +
               SpeedFactor * 0.15f + StaminaFactor * 0.10f +
               RoleFactor * 0.15f + TacticalFactor * 0.10f);

          Ctx.bCanIntercept = Ctx.InterceptConfidence >= MinInterceptConfidence;
          Ctx.PredictedInterceptPoint = InterceptPoint;
          Ctx.TimeToIntercept = PlayerTimeToPoint;
        }
      }
    }
  }
}

// UFootballStateTreeTask
EStateTreeRunStatus UFootballStateTreeTask::EnterState(
    FStateTreeExecutionContext &Context,
    const FStateTreeTransitionResult &Transition) const {
  return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus
UFootballStateTreeTask::Tick(FStateTreeExecutionContext &Context,
                             float DeltaTime) const {
  return EStateTreeRunStatus::Running;
}

void UFootballStateTreeTask::ExitState(
    FStateTreeExecutionContext &Context,
    const FStateTreeTransitionResult &Transition) const {}

// UIdleTask
EStateTreeRunStatus
UIdleTask::EnterState(FStateTreeExecutionContext &Context,
                      const FStateTreeTransitionResult &Transition) const {
  GET_CONTEXT(Context);

  if (Ctx.Character) {
    Ctx.Character->SetMovementMode(EFootballMovementMode::Walking);
    Ctx.Character->CurrentActionState = FFootballTags::Player_Idle;
  }

  return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus UIdleTask::Tick(FStateTreeExecutionContext &Context,
                                    float DeltaTime) const {
  return EStateTreeRunStatus::Running;
}

// UMoveToFormationTask
EStateTreeRunStatus UMoveToFormationTask::EnterState(
    FStateTreeExecutionContext &Context,
    const FStateTreeTransitionResult &Transition) const {
  GET_CONTEXT(Context);

  if (Ctx.Character && Ctx.Tactical) {
    Ctx.Character->SetMovementMode(EFootballMovementMode::Jogging);
    Ctx.Character->CurrentActionState = FFootballTags::Player_Running;

    FVector FormationPos =
        Ctx.Tactical->CalculateDefensivePosition(Ctx.Character);
    Ctx.Tactical->SetPlayerTargetPosition(Ctx.Character, FormationPos);
  }

  return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus
UMoveToFormationTask::Tick(FStateTreeExecutionContext &Context,
                           float DeltaTime) const {
  GET_CONTEXT(Context);

  if (Ctx.Character && Ctx.Tactical) {
    FVector FormationPos =
        Ctx.Tactical->CalculateDefensivePosition(Ctx.Character);
    Ctx.Tactical->SetPlayerTargetPosition(Ctx.Character, FormationPos);
    Ctx.Tactical->UpdatePlayerPosition(Ctx.Character, DeltaTime);

    float Distance =
        FVector::Dist(Ctx.Character->GetActorLocation(), FormationPos);
    if (Distance <= PositionTolerance) {
      return EStateTreeRunStatus::Succeeded;
    }
  }

  return EStateTreeRunStatus::Running;
}

// UPressTask - FIXED: Proper transition logic to Intercept
EStateTreeRunStatus
UPressTask::EnterState(FStateTreeExecutionContext &Context,
                       const FStateTreeTransitionResult &Transition) const {
  GET_CONTEXT(Context);

  if (Ctx.Character) {
    Ctx.Character->SetMovementMode(EFootballMovementMode::Jockeying);
    Ctx.Character->CurrentActionState =
        FFootballTags::Player_Intercepting; // Using intercepting tag for
                                            // pressing
  }

  return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus UPressTask::Tick(FStateTreeExecutionContext &Context,
                                     float DeltaTime) const {
  GET_CONTEXT(Context);

  if (!Ctx.Character || !Ctx.Ball || !Ctx.Team || !Ctx.Tactical) {
    return EStateTreeRunStatus::Failed;
  }

  // Ball must be possessed by opponent
  if (Ctx.BallState != EFootballBallState::Possessed) {
    return EStateTreeRunStatus::Failed;
  }

  AActor *BallCarrier = Ctx.BallPossessor;
  if (!BallCarrier || BallCarrier == Ctx.Character) {
    return EStateTreeRunStatus::Failed;
  }

  // Calculate press position
  FVector CarrierPos = BallCarrier->GetActorLocation();
  FVector CarrierForward = BallCarrier->GetActorForwardVector();

  // Press from angle
  float AngleRad = FMath::DegreesToRadians(PressAngle);
  FVector PressDirection = CarrierForward.RotateAngleAxis(
      bForceWeakFoot ? 90.0f : -90.0f, FVector::UpVector);
  FVector PressPos = CarrierPos + PressDirection * PressDistance;

  // Move to press position
  AAIController *AIController = Ctx.Character->GetController<AAIController>();
  if (AIController) {
    FAIMoveRequest MoveRequest;
    MoveRequest.SetGoalLocation(PressPos);
    MoveRequest.SetAcceptanceRadius(100.0f);
    AIController->MoveTo(MoveRequest);
  }

  // Check if close enough to attempt intercept
  float Distance = FVector::Dist(Ctx.Character->GetActorLocation(), CarrierPos);
  if (Distance <= 250.0f) {
    // Try to intercept - this will trigger the transition to Intercept state
    Ctx.Character->Intercept();
  }

  // THE FIX: The transition to Intercept is now handled by the StateTree
  // transition conditions which check Ctx.bCanIntercept from the
  // InterceptionEvaluator This evaluator runs every tick and properly
  // calculates if interception is possible

  return EStateTreeRunStatus::Running;
}

// UInterceptTask - Properly handles interception execution
EStateTreeRunStatus
UInterceptTask::EnterState(FStateTreeExecutionContext &Context,
                           const FStateTreeTransitionResult &Transition) const {
  GET_CONTEXT(Context);

  if (Ctx.Character) {
    Ctx.Character->SetMovementMode(EFootballMovementMode::Sprinting);
    Ctx.Character->CurrentActionState = FFootballTags::Player_Intercepting;
    Ctx.Character->Intercept();
  }

  return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus UInterceptTask::Tick(FStateTreeExecutionContext &Context,
                                         float DeltaTime) const {
  GET_CONTEXT(Context);

  if (!Ctx.Character || !Ctx.Ball || !Ctx.Tactical) {
    return EStateTreeRunStatus::Failed;
  }

  // Ball must be free/rolling for interception
  if (Ctx.BallState != EFootballBallState::Flying &&
      Ctx.BallState != EFootballBallState::Rolling) {
    return EStateTreeRunStatus::Failed;
  }

  // Move to predicted intercept point
  if (Ctx.bCanIntercept && !Ctx.PredictedInterceptPoint.IsZero()) {
    AAIController *AIController = Ctx.Character->GetController<AAIController>();
    if (AIController) {
      FAIMoveRequest MoveRequest;
      MoveRequest.SetGoalLocation(Ctx.PredictedInterceptPoint);
      MoveRequest.SetAcceptanceRadius(100.0f);
      AIController->MoveTo(MoveRequest);
    }

    // Check if we've reached intercept point
    float Distance = FVector::Dist(Ctx.Character->GetActorLocation(),
                                   Ctx.PredictedInterceptPoint);
    if (Distance <= 150.0f) {
      // Attempt to possess ball
      if (Ctx.Ball->GetBallState() == EFootballBallState::Free ||
          Ctx.Ball->GetBallState() == EFootballBallState::Rolling) {
        Ctx.Character->RequestPossession(Ctx.Ball);
        return EStateTreeRunStatus::Succeeded;
      }
    }
  } else {
    // Lost interception opportunity
    return EStateTreeRunStatus::Failed;
  }

  return EStateTreeRunStatus::Running;
}

// USupportTask
EStateTreeRunStatus
USupportTask::EnterState(FStateTreeExecutionContext &Context,
                         const FStateTreeTransitionResult &Transition) const {
  GET_CONTEXT(Context);

  if (Ctx.Character) {
    Ctx.Character->SetMovementMode(EFootballMovementMode::Jogging);
    Ctx.Character->CurrentActionState = FFootballTags::Player_Running;
  }

  return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus USupportTask::Tick(FStateTreeExecutionContext &Context,
                                       float DeltaTime) const {
  GET_CONTEXT(Context);

  if (Ctx.Character && Ctx.Team && Ctx.Tactical) {
    AFootballPlayerCharacter *BallCarrier = Ctx.Team->GetPlayerWithBall();
    if (BallCarrier && BallCarrier != Ctx.Character) {
      FVector SupportPos =
          Ctx.Tactical->CalculateSupportPosition(Ctx.Character);
      Ctx.Tactical->SetPlayerTargetPosition(Ctx.Character, SupportPos);
      Ctx.Tactical->UpdatePlayerPosition(Ctx.Character, DeltaTime);
    }
  }

  return EStateTreeRunStatus::Running;
}

// UMarkTask
EStateTreeRunStatus
UMarkTask::EnterState(FStateTreeExecutionContext &Context,
                      const FStateTreeTransitionResult &Transition) const {
  GET_CONTEXT(Context);

  if (Ctx.Character) {
    Ctx.Character->SetMovementMode(EFootballMovementMode::Jockeying);
    Ctx.Character->CurrentActionState = FFootballTags::Player_Intercepting;
  }

  return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus UMarkTask::Tick(FStateTreeExecutionContext &Context,
                                    float DeltaTime) const {
  GET_CONTEXT(Context);

  if (!Ctx.Character || !Ctx.Team || !Ctx.Ball) {
    return EStateTreeRunStatus::Failed;
  }

  // Get mark target from blackboard (would be set by tactical component)
  // For now, find closest opponent
  AFootballPlayerCharacter *Target = nullptr;
  float MinDist = FLT_MAX;

  if (Ctx.OpponentTeam) {
    for (AFootballPlayerCharacter *Opponent :
         Ctx.OpponentTeam->GetAllPlayers()) {
      if (Opponent) {
        float Dist = FVector::Dist(Ctx.Character->GetActorLocation(),
                                   Opponent->GetActorLocation());
        if (Dist < MinDist) {
          MinDist = Dist;
          Target = Opponent;
        }
      }
    }
  }

  if (Target) {
    FVector TargetPos = Target->GetActorLocation();
    FVector BallPos = Ctx.BallPosition;
    FVector MarkPos = TargetPos;

    if (bGoalSide) {
      EFootballTeamSide::Type OurSide = Ctx.Character->GetTeamSide();
      FVector GoalDirection = (OurSide == EFootballTeamSide::Home)
                                  ? FVector(1, 0, 0)
                                  : FVector(-1, 0, 0);
      FVector ToGoal = (TargetPos + GoalDirection * 10000.0f) - TargetPos;
      MarkPos = TargetPos + ToGoal.GetSafeNormal() * MarkDistance;
    }

    if (bBallSide) {
      FVector ToBall = (BallPos - TargetPos).GetSafeNormal();
      MarkPos = TargetPos + ToBall * MarkDistance;
    }

    AAIController *AIController = Ctx.Character->GetController<AAIController>();
    if (AIController) {
      FAIMoveRequest MoveRequest;
      MoveRequest.SetGoalLocation(MarkPos);
      MoveRequest.SetAcceptanceRadius(150.0f);
      AIController->MoveTo(MoveRequest);
    }
  }

  return EStateTreeRunStatus::Running;
}

// UAttackTask
EStateTreeRunStatus
UAttackTask::EnterState(FStateTreeExecutionContext &Context,
                        const FStateTreeTransitionResult &Transition) const {
  GET_CONTEXT(Context);

  if (Ctx.Character) {
    Ctx.Character->SetMovementMode(EFootballMovementMode::Sprinting);
    Ctx.Character->CurrentActionState = FFootballTags::Player_Sprinting;
  }

  return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus UAttackTask::Tick(FStateTreeExecutionContext &Context,
                                      float DeltaTime) const {
  GET_CONTEXT(Context);

  if (Ctx.Character && Ctx.Team && Ctx.Tactical) {
    FVector AttackPos = Ctx.Tactical->CalculateAttackingPosition(Ctx.Character);
    Ctx.Tactical->SetPlayerTargetPosition(Ctx.Character, AttackPos);
    Ctx.Tactical->UpdatePlayerPosition(Ctx.Character, DeltaTime);
  }

  return EStateTreeRunStatus::Running;
}

// UDefendTask
EStateTreeRunStatus
UDefendTask::EnterState(FStateTreeExecutionContext &Context,
                        const FStateTreeTransitionResult &Transition) const {
  GET_CONTEXT(Context);

  if (Ctx.Character) {
    Ctx.Character->SetMovementMode(EFootballMovementMode::Jogging);
    Ctx.Character->CurrentActionState = FFootballTags::Player_Running;
  }

  return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus UDefendTask::Tick(FStateTreeExecutionContext &Context,
                                      float DeltaTime) const {
  GET_CONTEXT(Context);

  if (Ctx.Character && Ctx.Team && Ctx.Tactical) {
    FVector DefendPos = Ctx.Tactical->CalculateDefensivePosition(Ctx.Character);
    Ctx.Tactical->SetPlayerTargetPosition(Ctx.Character, DefendPos);
    Ctx.Tactical->UpdatePlayerPosition(Ctx.Character, DeltaTime);
  }

  return EStateTreeRunStatus::Running;
}

// UGoalkeeperTask
EStateTreeRunStatus UGoalkeeperTask::EnterState(
    FStateTreeExecutionContext &Context,
    const FStateTreeTransitionResult &Transition) const {
  GET_CONTEXT(Context);

  if (Ctx.Character) {
    Ctx.Character->SetMovementMode(EFootballMovementMode::Walking);
    Ctx.Character->CurrentActionState = FFootballTags::Player_Goalkeeping;
  }

  return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus UGoalkeeperTask::Tick(FStateTreeExecutionContext &Context,
                                          float DeltaTime) const {
  GET_CONTEXT(Context);

  if (!Ctx.Character || !Ctx.Ball ||
      Ctx.Character->GetPlayerRole() != EFootballPlayerRole::Goalkeeper) {
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

  AAIController *AIController = Ctx.Character->GetController<AAIController>();
  if (AIController) {
    FAIMoveRequest MoveRequest;
    MoveRequest.SetGoalLocation(TargetPos);
    MoveRequest.SetAcceptanceRadius(200.0f);
    AIController->MoveTo(MoveRequest);
  }

  // Check for shots to save
  if (Ctx.BallState == EFootballBallState::Flying) {
    FVector BallVel = Ctx.BallVelocity;
    FVector ToBall = BallPos - GKPos;
    float Distance = ToBall.Size();

    // If ball heading toward goal and close
    if (Distance < 1000.0f &&
        FVector::DotProduct(BallVel.GetSafeNormal(), ToBall.GetSafeNormal()) >
            0.7f) {
      // Trigger save - would transition to save state
      Ctx.Character->GoalkeeperDive((BallPos - GKPos).GetSafeNormal());
    }
  }

  return EStateTreeRunStatus::Running;
}

// USetPieceTask
EStateTreeRunStatus
USetPieceTask::EnterState(FStateTreeExecutionContext &Context,
                          const FStateTreeTransitionResult &Transition) const {
  GET_CONTEXT(Context);

  if (Ctx.Team && Ctx.Tactical) {
    bool bAttacking = Ctx.bTeamHasPossession;
    Ctx.Tactical->ExecuteSetPieceRoutine(SetPieceType);
  }

  return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus USetPieceTask::Tick(FStateTreeExecutionContext &Context,
                                        float DeltaTime) const {
  GET_CONTEXT(Context);

  // Wait for set piece to be taken
  if (Ctx.Ball && Ctx.BallState == EFootballBallState::Flying) {
    return EStateTreeRunStatus::Succeeded;
  }

  return EStateTreeRunStatus::Running;
}