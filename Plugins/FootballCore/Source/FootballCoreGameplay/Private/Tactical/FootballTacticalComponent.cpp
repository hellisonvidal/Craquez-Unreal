// Copyright 2024 Craquez. All Rights Reserved.

#include "Tactical/FootballTacticalComponent.h"
#include "Algo/Sort.h"
#include "Team/FootballTeam.h"
#include "Team/FootballTeamManager.h"
#include "Player/FootballPlayerCharacter.h"
#include "BallPhysics/FootballBallComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"

UFootballTacticalComponent::UFootballTacticalComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

void UFootballTacticalComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UFootballTacticalComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                               FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!Team)
    {
        return;
    }

    UpdatePlayerMovement(DeltaTime);

    // Apply tactical behaviors based on instructions
    float PressingIntensity = GetInstruction(FGameplayTag::RequestGameplayTag("Tactic.PressingIntensity"));
    if (PressingIntensity > 50.0f)
    {
        ApplyPressing(DeltaTime);
    }

    float OffsideTrap = GetInstruction(FGameplayTag::RequestGameplayTag("Tactic.OffsideTrap"));
    if (OffsideTrap > 50.0f)
    {
        ApplyOffsideTrap(DeltaTime);
    }

    float CounterAttack = GetInstruction(FGameplayTag::RequestGameplayTag("Tactic.CounterAttack"));
    if (CounterAttack > 50.0f && !Team->HasPossession())
    {
        ApplyCounterAttack(DeltaTime);
    }
}

void UFootballTacticalComponent::SetTeam(AFootballTeam* InTeam)
{
    Team = InTeam;
}

void UFootballTacticalComponent::Initialize(const FFootballTacticalInstructions& InTactics)
{
    CurrentTactics = InTactics;

    // Convert tactics to instructions map
    Instructions.Add(FGameplayTag::RequestGameplayTag("Tactic.DefensiveLine"), InTactics.DefensiveLine);
    Instructions.Add(FGameplayTag::RequestGameplayTag("Tactic.EngagementLine"), InTactics.EngagementLine);
    Instructions.Add(FGameplayTag::RequestGameplayTag("Tactic.Width"), InTactics.Width);
    Instructions.Add(FGameplayTag::RequestGameplayTag("Tactic.BuildUpPlay"), InTactics.BuildUpPlay);
    Instructions.Add(FGameplayTag::RequestGameplayTag("Tactic.ChanceCreation"), InTactics.ChanceCreation);
    Instructions.Add(FGameplayTag::RequestGameplayTag("Tactic.PassingDirectness"), InTactics.PassingDirectness);
    Instructions.Add(FGameplayTag::RequestGameplayTag("Tactic.CrossingFrequency"), InTactics.CrossingFrequency);
    Instructions.Add(FGameplayTag::RequestGameplayTag("Tactic.ShootOnSight"), InTactics.ShootOnSight);
    Instructions.Add(FGameplayTag::RequestGameplayTag("Tactic.Tempo"), InTactics.Tempo);
    Instructions.Add(FGameplayTag::RequestGameplayTag("Tactic.CreativeFreedom"), InTactics.CreativeFreedom);
    Instructions.Add(FGameplayTag::RequestGameplayTag("Tactic.PressingIntensity"), InTactics.PressingIntensity);
    Instructions.Add(FGameplayTag::RequestGameplayTag("Tactic.DefensiveShape"), InTactics.DefensiveShape);
    Instructions.Add(FGameplayTag::RequestGameplayTag("Tactic.Interceptions"), InTactics.Interceptions);
    Instructions.Add(FGameplayTag::RequestGameplayTag("Tactic.TacklingAggression"), InTactics.TacklingAggression);
    Instructions.Add(FGameplayTag::RequestGameplayTag("Tactic.OffsideTrap"), InTactics.OffsideTrap);
    Instructions.Add(FGameplayTag::RequestGameplayTag("Tactic.GK_RushOut"), InTactics.GK_RushOut);
    Instructions.Add(FGameplayTag::RequestGameplayTag("Tactic.GK_Distribution"), InTactics.GK_Distribution);
    Instructions.Add(FGameplayTag::RequestGameplayTag("Tactic.CounterAttack"),
                     InTactics.BuildUpPlay < 30.0f ? 80.0f : 20.0f);
}

void UFootballTacticalComponent::UpdateTactics(const FFootballTacticalInstructions& InTactics)
{
    Initialize(InTactics);
}

void UFootballTacticalComponent::SetInstruction(FGameplayTag Instruction, float Value)
{
    float OldValue = Instructions.Contains(Instruction) ? Instructions[Instruction] : 0.0f;
    Instructions.Add(Instruction, FMath::Clamp(Value, 0.0f, 100.0f));

    if (FMath::Abs(OldValue - Value) > 0.1f)
    {
        OnInstructionChanged.Broadcast(Instruction, Value);
    }
}

float UFootballTacticalComponent::GetInstruction(FGameplayTag Instruction) const
{
    return Instructions.Contains(Instruction) ? Instructions[Instruction] : 50.0f;
}

void UFootballTacticalComponent::SetPlayerTargetPosition(AFootballPlayerCharacter* Player, FVector TargetPosition)
{
    if (Player)
    {
        PlayerTargetPositions.Add(Player, TargetPosition);
    }
}

void UFootballTacticalComponent::UpdatePlayerPosition(AFootballPlayerCharacter* Player, float DeltaTime)
{
    if (!Player || !PlayerTargetPositions.Contains(Player))
    {
        return;
    }

    FVector TargetPos = PlayerTargetPositions[Player];
    FVector CurrentPos = Player->GetActorLocation();

    // Calculate direction and distance
    FVector ToTarget = TargetPos - CurrentPos;
    float Distance = ToTarget.Size();

    if (Distance > 100.0f) // 1 meter threshold
    {
        FVector Direction = ToTarget.GetSafeNormal();

        // Determine speed based on role and situation
        float BaseSpeed = 400.0f; // cm/s
        EFootballPlayerRole::Type Role = Player->GetPlayerRole();

        // Adjust speed based on role
        switch (Role)
        {
        case EFootballPlayerRole::Goalkeeper:
            BaseSpeed = 300.0f;
            break;
        case EFootballPlayerRole::CenterBack:
        case EFootballPlayerRole::LeftBack:
        case EFootballPlayerRole::RightBack:
            BaseSpeed = 350.0f;
            break;
        case EFootballPlayerRole::Striker:
        case EFootballPlayerRole::LeftWinger:
        case EFootballPlayerRole::RightWinger:
            BaseSpeed = 500.0f;
            break;
        default:
            BaseSpeed = 400.0f;
            break;
        }

        // Apply player attributes
        BaseSpeed *= Player->GetAttributes().SprintSpeed / 100.0f;

        // Move towards target
        FVector Velocity = Direction * BaseSpeed;
        Player->GetCharacterMovement()->Velocity = Velocity;
        Player->GetCharacterMovement()->MaxWalkSpeed = BaseSpeed;
    }
    else
    {
        // Close enough, stop
        Player->GetCharacterMovement()->Velocity = FVector::ZeroVector;
    }
}

FVector UFootballTacticalComponent::CalculateDefensivePosition(AFootballPlayerCharacter* Player) const
{
    if (!Team || !Player)
    {
        return FVector::ZeroVector;
    }

    UFootballBallComponent* Ball = nullptr;
    if (AFootballTeamManager* Manager = Cast<AFootballTeamManager>(Team->GetTeamManager()))
    {
        Ball = Manager->GetBall();
    }

    FVector BallPos = Ball ? Ball->GetComponentLocation() : Team->GetActorLocation();
    FVector TeamCenter = Team->GetActorLocation();

    EFootballPlayerRole::Type Role = Player->GetPlayerRole();
    float DefensiveLine = GetInstruction(FGameplayTag::RequestGameplayTag("Tactic.DefensiveLine"));
    float DefensiveShape = GetInstruction(FGameplayTag::RequestGameplayTag("Tactic.DefensiveShape"));

    // Base position from formation
    FVector BasePos = Team->CalculateFormationPosition(Role);

    // Adjust for defensive line
    float LineOffset = (DefensiveLine - 50.0f) * 40.0f;
    if (Team->GetTeamSide() == EFootballTeamSide::Away)
    {
        LineOffset = -LineOffset;
    }

    // Defensive players drop deeper
    if (Role == EFootballPlayerRole::CenterBack || Role == EFootballPlayerRole::LeftBack ||
        Role == EFootballPlayerRole::RightBack || Role == EFootballPlayerRole::DefensiveMidfielder)
    {
        BasePos.X += LineOffset * 0.8f;
    }
    else if (Role == EFootballPlayerRole::Goalkeeper)
    {
        BasePos.X += LineOffset * 0.3f;
    }

    // Zonal vs man-to-man
    if (DefensiveShape > 50.0f) // Zonal
    {
        // Stay in zone
        return BasePos;
    }
    else // Man-to-man
    {
        // Find opponent to mark
        // This would require access to opponent team
        return BasePos;
    }
}

FVector UFootballTacticalComponent::CalculateAttackingPosition(AFootballPlayerCharacter* Player) const
{
    if (!Team || !Player)
    {
        return FVector::ZeroVector;
    }

    UFootballBallComponent* Ball = nullptr;
    if (AFootballTeamManager* Manager = Cast<AFootballTeamManager>(Team->GetTeamManager()))
    {
        Ball = Manager->GetBall();
    }

    FVector BallPos = Ball ? Ball->GetComponentLocation() : Team->GetActorLocation();
    EFootballPlayerRole::Type Role = Player->GetPlayerRole();

    // Base position from formation
    FVector BasePos = Team->CalculateFormationPosition(Role);

    // Adjust for engagement line
    float EngagementLine = GetInstruction(FGameplayTag::RequestGameplayTag("Tactic.EngagementLine"));
    float LineOffset = (EngagementLine - 50.0f) * 40.0f;
    if (Team->GetTeamSide() == EFootballTeamSide::Away)
    {
        LineOffset = -LineOffset;
    }

    // Attacking players push higher
    if (Role == EFootballPlayerRole::Striker || Role == EFootballPlayerRole::CenterForward ||
        Role == EFootballPlayerRole::LeftWinger || Role == EFootballPlayerRole::RightWinger ||
        Role == EFootballPlayerRole::AttackingMidfielder)
    {
        BasePos.X += LineOffset;
    }
    else if (Role == EFootballPlayerRole::CentralMidfielder)
    {
        BasePos.X += LineOffset * 0.5f;
    }

    // Width adjustment
    float Width = GetInstruction(FGameplayTag::RequestGameplayTag("Tactic.Width"));
    if (Role == EFootballPlayerRole::LeftWinger || Role == EFootballPlayerRole::LeftBack)
    {
        BasePos.Y *= (Width / 50.0f);
    }
    else if (Role == EFootballPlayerRole::RightWinger || Role == EFootballPlayerRole::RightBack)
    {
        BasePos.Y *= (Width / 50.0f);
    }

    // Support positioning - move toward ball if nearby
    if (Ball)
    {
        float DistToBall = FVector::Dist(Player->GetActorLocation(), BallPos);
        if (DistToBall < 3000.0f) // 30m
        {
            FVector ToBall = (BallPos - Player->GetActorLocation()).GetSafeNormal();
            BasePos += ToBall * 500.0f; // Move 5m toward ball
        }
    }

    return BasePos;
}

FVector UFootballTacticalComponent::CalculateSupportPosition(AFootballPlayerCharacter* Player) const
{
    if (!Team || !Player)
    {
        return FVector::ZeroVector;
    }

    AFootballPlayerCharacter* BallCarrier = Team->GetPlayerWithBall();
    if (!BallCarrier)
    {
        return CalculateAttackingPosition(Player);
    }

    FVector CarrierPos = BallCarrier->GetActorLocation();
    FVector PlayerPos = Player->GetActorLocation();
    FVector ToCarrier = CarrierPos - PlayerPos;
    float Dist = ToCarrier.Size();

    // Support positions based on role
    EFootballPlayerRole::Type Role = Player->GetPlayerRole();
    FVector SupportPos = PlayerPos;

    if (Dist < 2000.0f) // Close support
    {
        // Move to side/behind carrier for pass option
        FVector Right = FVector::CrossProduct(ToCarrier.GetSafeNormal(), FVector::UpVector);

        if (Role == EFootballPlayerRole::CentralMidfielder || Role == EFootballPlayerRole::AttackingMidfielder)
        {
            SupportPos = CarrierPos + Right * 1000.0f - ToCarrier.GetSafeNormal() * 500.0f;
        }
        else if (Role == EFootballPlayerRole::LeftWinger || Role == EFootballPlayerRole::RightWinger)
        {
            SupportPos = CarrierPos + Right * 1500.0f;
        }
        else if (Role == EFootballPlayerRole::Striker || Role == EFootballPlayerRole::CenterForward)
        {
            SupportPos = CarrierPos + ToCarrier.GetSafeNormal() * 1000.0f; // Ahead for through ball
        }
    }

    return SupportPos;
}

void UFootballTacticalComponent::MaintainTeamShape(float DeltaTime)
{
    if (!Team)
    {
        return;
    }

    float Compactness = CalculateTeamCompactness();
    float TargetCompactness = 60.0f; // Ideal compactness

    // Adjust player positions to maintain compactness
    if (Compactness > TargetCompactness + 10.0f)
    {
        // Team too spread out - pull players toward center
        FVector Center = CalculateTeamCenter();
        for (AFootballPlayerCharacter* Player : Team->GetAllPlayers())
        {
            if (Player && !Player->HasBallPossession())
            {
                FVector ToCenter = (Center - Player->GetActorLocation()).GetSafeNormal();
                FVector Adjustment = ToCenter * (Compactness - TargetCompactness) * 10.0f;
                PlayerTargetPositions.Add(Player, Player->GetActorLocation() + Adjustment);
            }
        }
    }
    else if (Compactness < TargetCompactness - 10.0f)
    {
        // Team too compact - spread out
        FVector Center = CalculateTeamCenter();
        for (AFootballPlayerCharacter* Player : Team->GetAllPlayers())
        {
            if (Player && !Player->HasBallPossession())
            {
                FVector FromCenter = (Player->GetActorLocation() - Center).GetSafeNormal();
                FVector Adjustment = FromCenter * (TargetCompactness - Compactness) * 10.0f;
                PlayerTargetPositions.Add(Player, Player->GetActorLocation() + Adjustment);
            }
        }
    }
}

float UFootballTacticalComponent::CalculateTeamCompactness() const
{
    if (!Team)
    {
        return 0.0f;
    }

    TArray<AFootballPlayerCharacter*> Players = Team->GetAllPlayers();
    if (Players.Num() < 2)
    {
        return 0.0f;
    }

    float TotalDist = 0.0f;
    int32 Count = 0;

    for (int32 i = 0; i < Players.Num(); i++)
    {
        for (int32 j = i + 1; j < Players.Num(); j++)
        {
            if (Players[i] && Players[j])
            {
                TotalDist += FVector::Dist(Players[i]->GetActorLocation(), Players[j]->GetActorLocation());
                Count++;
            }
        }
    }

    return Count > 0 ? TotalDist / Count : 0.0f;
}

FVector UFootballTacticalComponent::CalculateTeamCenter() const
{
    if (!Team)
    {
        return FVector::ZeroVector;
    }

    TArray<AFootballPlayerCharacter*> Players = Team->GetAllPlayers();
    if (Players.Num() == 0)
    {
        return Team->GetActorLocation();
    }

    FVector Sum = FVector::ZeroVector;
    int32 ValidCount = 0;

    for (AFootballPlayerCharacter* Player : Players)
    {
        if (Player)
        {
            Sum += Player->GetActorLocation();
            ValidCount++;
        }
    }

    return ValidCount > 0 ? Sum / ValidCount : Team->GetActorLocation();
}

float UFootballTacticalComponent::CalculateDefensiveLine() const
{
    if (!Team)
    {
        return 0.0f;
    }

    float FurthestBack = FLT_MAX;
    for (AFootballPlayerCharacter* Player : Team->GetAllPlayers())
    {
        if (Player && Player->GetPlayerRole() != EFootballPlayerRole::Goalkeeper)
        {
            float X = Player->GetActorLocation().X;
            if (Team->GetTeamSide() == EFootballTeamSide::Home)
            {
                FurthestBack = FMath::Min(FurthestBack, X);
            }
            else
            {
                FurthestBack = FMath::Max(FurthestBack, X);
            }
        }
    }

    return FurthestBack;
}

float UFootballTacticalComponent::CalculateOffsideLine() const
{
    if (!Team)
    {
        return 0.0f;
    }

    float SecondLastDefender = FLT_MAX;
    TArray<float> DefenderPositions;

    for (AFootballPlayerCharacter* Player : Team->GetAllPlayers())
    {
        if (Player && Player->GetPlayerRole() != EFootballPlayerRole::Goalkeeper)
        {
            DefenderPositions.Add(Player->GetActorLocation().X);
        }
    }

    DefenderPositions.Sort();

    if (Team->GetTeamSide() == EFootballTeamSide::Home)
    {
        // Home team attacks toward positive X
        // Offside line is second-last defender (closest to goal)
        if (DefenderPositions.Num() >= 2)
        {
            SecondLastDefender = DefenderPositions[1];
        }
    }
    else
    {
        // Away team attacks toward negative X
        if (DefenderPositions.Num() >= 2)
        {
            SecondLastDefender = DefenderPositions[DefenderPositions.Num() - 2];
        }
    }

    return SecondLastDefender;
}

TArray<AFootballPlayerCharacter*> UFootballTacticalComponent::GetPlayersInZone(FVector ZoneCenter,
                                                                               float ZoneRadius) const
{
    TArray<AFootballPlayerCharacter*> Result;
    if (!Team)
    {
        return Result;
    }

    for (AFootballPlayerCharacter* Player : Team->GetAllPlayers())
    {
        if (Player)
        {
            float Dist = FVector::Dist(Player->GetActorLocation(), ZoneCenter);
            if (Dist <= ZoneRadius)
            {
                Result.Add(Player);
            }
        }
    }

    return Result;
}

int32 UFootballTacticalComponent::CountPlayersInArea(FVector Min, FVector Max) const
{
    int32 Count = 0;
    if (!Team)
    {
        return Count;
    }

    FBox Area(Min, Max);
    for (AFootballPlayerCharacter* Player : Team->GetAllPlayers())
    {
        if (Player && Area.IsInside(Player->GetActorLocation()))
        {
            Count++;
        }
    }

    return Count;
}

bool UFootballTacticalComponent::IsPlayerMarked(AFootballPlayerCharacter* Player) const
{
    // This would require access to opponent team
    // For now, return false
    return false;
}

AFootballPlayerCharacter* UFootballTacticalComponent::FindBestPassTarget(AFootballPlayerCharacter* Passer) const
{
    if (!Team || !Passer)
    {
        return nullptr;
    }

    AFootballPlayerCharacter* BestTarget = nullptr;
    float BestScore = -1.0f;

    for (AFootballPlayerCharacter* Player : Team->GetAllPlayers())
    {
        if (Player && Player != Passer && !Player->HasBallPossession())
        {
            float Score = CalculatePassScore(Passer, Player);
            if (Score > BestScore)
            {
                BestScore = Score;
                BestTarget = Player;
            }
        }
    }

    return BestTarget;
}

float UFootballTacticalComponent::CalculatePassScore(AFootballPlayerCharacter* Passer,
                                                     AFootballPlayerCharacter* Receiver) const
{
    if (!Passer || !Receiver)
    {
        return 0.0f;
    }

    FVector PassVector = Receiver->GetActorLocation() - Passer->GetActorLocation();
    float Distance = PassVector.Size();
    float MaxPassDist = 4000.0f; // 40m

    // Distance factor (closer is easier, but too close is wasteful)
    float DistanceFactor = 1.0f - FMath::Clamp(Distance / MaxPassDist, 0.0f, 1.0f);
    if (Distance < 500.0f)
        DistanceFactor *= 0.5f;

    // Receiver space factor (how much space they have)
    float SpaceFactor = 1.0f;
    int32 OpponentsNearby = 0; // Would need opponent team
    SpaceFactor = 1.0f - FMath::Clamp(OpponentsNearby * 0.2f, 0.0f, 0.8f);

    // Receiver attribute factor
    float AttributeFactor = Receiver->GetAttributes().BallControl / 100.0f;

    // Forward pass bonus
    float ForwardBonus = 0.0f;
    FVector AttackDirection = (Team->GetTeamSide() == EFootballTeamSide::Home) ? FVector(1, 0, 0) : FVector(-1, 0, 0);
    float ForwardDot = FVector::DotProduct(PassVector.GetSafeNormal(), AttackDirection);
    if (ForwardDot > 0)
    {
        ForwardBonus = ForwardDot * 0.3f;
    }

    // Role bonus
    float RoleBonus = 0.0f;
    EFootballPlayerRole::Type Role = Receiver->GetPlayerRole();
    if (Role == EFootballPlayerRole::Striker || Role == EFootballPlayerRole::CenterForward)
    {
        RoleBonus = 0.2f;
    }
    else if (Role == EFootballPlayerRole::LeftWinger || Role == EFootballPlayerRole::RightWinger)
    {
        RoleBonus = 0.15f;
    }

    return DistanceFactor * 0.4f + SpaceFactor * 0.3f + AttributeFactor * 0.2f + ForwardBonus + RoleBonus;
}

AFootballPlayerCharacter* UFootballTacticalComponent::FindBestInterceptTarget() const
{
    // Would need ball and opponent team
    return nullptr;
}

FVector UFootballTacticalComponent::CalculateInterceptPoint(UFootballBallComponent* Ball,
                                                            AFootballPlayerCharacter* Interceptor) const
{
    if (!Ball || !Interceptor)
    {
        return FVector::ZeroVector;
    }

    FVector BallPos = Ball->GetComponentLocation();
    FVector BallVel = Ball->GetVelocity();
    FVector PlayerPos = Interceptor->GetActorLocation();
    float PlayerSpeed = Interceptor->GetCharacterMovement()->MaxWalkSpeed;

    // Predict ball position over time and find intercept point
    float BestTime = 0.0f;
    float MinDist = FLT_MAX;

    for (float t = 0.1f; t < 5.0f; t += 0.1f)
    {
        FVector PredictedBallPos = BallPos + BallVel * t * 100.0f; // cm
        float DistToBall = FVector::Dist(PlayerPos, PredictedBallPos);
        float PlayerReachDist = PlayerSpeed * t;

        if (PlayerReachDist >= DistToBall && DistToBall < MinDist)
        {
            MinDist = DistToBall;
            BestTime = t;
        }
    }

    return BallPos + BallVel * BestTime * 100.0f;
}

void UFootballTacticalComponent::SetupCornerRoutine(bool bAttacking)
{
    // Setup corner kick positions
    TArray<FVector> Positions;

    if (bAttacking)
    {
        // Attacking corner positions
        Positions.Add(FVector(5000, -2000, 0)); // Near post runner
        Positions.Add(FVector(5000, 0, 0));     // Penalty spot
        Positions.Add(FVector(5000, 2000, 0));  // Far post
        Positions.Add(FVector(4000, -3000, 0)); // Edge of box
        Positions.Add(FVector(4000, 3000, 0));  // Edge of box far
    }
    else
    {
        // Defending corner positions
        Positions.Add(FVector(-5000, -1500, 0)); // Near post
        Positions.Add(FVector(-5000, 0, 0));     // Goal line
        Positions.Add(FVector(-5000, 1500, 0));  // Far post
        Positions.Add(FVector(-4000, -3000, 0)); // Edge of box
        Positions.Add(FVector(-4000, 3000, 0));  // Edge of box
    }

    SetPiecePositions.Add(bAttacking ? EFootballMatchPhase::SetPiece_Corner : EFootballMatchPhase::SetPiece_Corner,
                          Positions);
}

void UFootballTacticalComponent::SetupFreeKickRoutine(FVector BallPosition, bool bAttacking)
{
    TArray<FVector> Positions;

    if (bAttacking)
    {
        // Attacking free kick
        Positions.Add(BallPosition + FVector(0, -2000, 0)); // Wall side
        Positions.Add(BallPosition + FVector(1000, 0, 0));  // Target area
        Positions.Add(BallPosition + FVector(0, 2000, 0));  // Far side
    }
    else
    {
        // Defending free kick - wall
        FVector WallPos = BallPosition;
        WallPos.X += (Team->GetTeamSide() == EFootballTeamSide::Home) ? -915 : 915; // 9.15m

        for (int32 i = 0; i < 4; i++)
        {
            Positions.Add(WallPos + FVector(0, -1500 + i * 1000, 0));
        }
    }

    SetPiecePositions.Add(EFootballMatchPhase::SetPiece_FreeKick, Positions);
}

void UFootballTacticalComponent::SetupPenaltyRoutine(bool bAttacking)
{
    TArray<FVector> Positions;

    if (bAttacking)
    {
        Positions.Add(FVector(5000, 0, 0)); // Penalty spot
    }
    else
    {
        Positions.Add(FVector(-5000, 0, 0)); // Goal line center
    }

    SetPiecePositions.Add(EFootballMatchPhase::SetPiece_Penalty, Positions);
}

void UFootballTacticalComponent::ExecuteSetPieceRoutine(EFootballMatchPhase::Type SetPieceType)
{
    if (!SetPiecePositions.Contains(SetPieceType))
    {
        return;
    }

    const TArray<FVector>& Positions = SetPiecePositions[SetPieceType];
    TArray<AFootballPlayerCharacter*> Players = Team->GetAllPlayers();

    for (int32 i = 0; i < FMath::Min(Positions.Num(), Players.Num()); i++)
    {
        if (Players[i] && !Players[i]->HasBallPossession())
        {
            SetPlayerTargetPosition(Players[i], Team->GetActorLocation() + Positions[i]);
        }
    }
}

void UFootballTacticalComponent::UpdatePlayerMovement(float DeltaTime)
{
    if (!Team)
    {
        return;
    }

    bool bHasBall = Team->HasPossession();

    for (AFootballPlayerCharacter* Player : Team->GetAllPlayers())
    {
        if (!Player || Player->HasBallPossession())
        {
            continue;
        }

        FVector TargetPos;

        if (bHasBall)
        {
            // Attacking positioning
            if (Player->GetPlayerRole() == EFootballPlayerRole::Striker ||
                Player->GetPlayerRole() == EFootballPlayerRole::CenterForward ||
                Player->GetPlayerRole() == EFootballPlayerRole::LeftWinger ||
                Player->GetPlayerRole() == EFootballPlayerRole::RightWinger)
            {
                TargetPos = CalculateAttackingPosition(Player);
            }
            else
            {
                TargetPos = CalculateSupportPosition(Player);
            }
        }
        else
        {
            // Defensive positioning
            TargetPos = CalculateDefensivePosition(Player);
        }

        SetPlayerTargetPosition(Player, TargetPos);
        UpdatePlayerPosition(Player, DeltaTime);
    }
}

void UFootballTacticalComponent::ApplyPressing(float DeltaTime)
{
    if (!Team || Team->HasPossession())
    {
        return;
    }

    UFootballBallComponent* Ball = nullptr;
    if (AFootballTeamManager* Manager = Cast<AFootballTeamManager>(Team->GetTeamManager()))
    {
        Ball = Manager->GetBall();
    }

    if (!Ball)
    {
        return;
    }

    float PressingIntensity = GetInstruction(FGameplayTag::RequestGameplayTag("Tactic.PressingIntensity"));
    float PressingRadius = 1500.0f + (PressingIntensity / 100.0f) * 3000.0f; // 15-45m

    // Find closest players to ball and make them press
    TArray<AFootballPlayerCharacter*> Players = Team->GetAllPlayers();
    Algo::Sort(Players,
               [Ball](AFootballPlayerCharacter* A, AFootballPlayerCharacter* B)
               {
                   if (!A || !B || !Ball)
                       return false;
                   return FVector::Dist(A->GetActorLocation(), Ball->GetComponentLocation()) <
                          FVector::Dist(B->GetActorLocation(), Ball->GetComponentLocation());
               });

    int32 PressingPlayers = FMath::RoundToInt(PressingIntensity / 100.0f * 4.0f); // 0-4 players press
    PressingPlayers = FMath::Clamp(PressingPlayers, 1, 4);

    for (int32 i = 0; i < PressingPlayers && i < Players.Num(); i++)
    {
        if (Players[i])
        {
            float Dist = FVector::Dist(Players[i]->GetActorLocation(), Ball->GetComponentLocation());
            if (Dist < PressingRadius)
            {
                Players[i]->Intercept();
                Players[i]->SetMovementMode(EFootballMovementMode::Jockeying);
            }
        }
    }
}

void UFootballTacticalComponent::ApplyOffsideTrap(float DeltaTime)
{
    if (!Team || Team->HasPossession())
    {
        return;
    }

    float OffsideTrapValue = GetInstruction(FGameplayTag::RequestGameplayTag("Tactic.OffsideTrap"));
    if (OffsideTrapValue < 50.0f)
    {
        return;
    }

    // Move defensive line up to catch attackers offside
    float DefensiveLine = CalculateDefensiveLine();
    float TargetLine = DefensiveLine;

    if (Team->GetTeamSide() == EFootballTeamSide::Home)
    {
        TargetLine += 500.0f; // Step up 5m
    }
    else
    {
        TargetLine -= 500.0f;
    }

    // Move defenders to target line
    for (AFootballPlayerCharacter* Player : Team->GetAllPlayers())
    {
        if (Player && (Player->GetPlayerRole() == EFootballPlayerRole::CenterBack ||
                       Player->GetPlayerRole() == EFootballPlayerRole::LeftBack ||
                       Player->GetPlayerRole() == EFootballPlayerRole::RightBack))
        {
            FVector TargetPos = Player->GetActorLocation();
            TargetPos.X = TargetLine;
            SetPlayerTargetPosition(Player, TargetPos);
        }
    }
}

void UFootballTacticalComponent::ApplyCounterAttack(float DeltaTime)
{
    if (!Team || Team->HasPossession())
    {
        return;
    }

    // Quick transition to attack when winning ball
    UFootballBallComponent* Ball = nullptr;
    if (AFootballTeamManager* Manager = Cast<AFootballTeamManager>(Team->GetTeamManager()))
    {
        Ball = Manager->GetBall();
    }

    if (!Ball || Ball->GetBallState() != EFootballBallState::Free)
    {
        return;
    }

    // Find player who won the ball
    AFootballPlayerCharacter* Interceptor = Team->GetClosestPlayerToBall();
    if (Interceptor && Interceptor->HasBallPossession())
    {
        // Launch counter attack - attackers sprint forward
        for (AFootballPlayerCharacter* Player : Team->GetAllPlayers())
        {
            if (Player && (Player->GetPlayerRole() == EFootballPlayerRole::Striker ||
                           Player->GetPlayerRole() == EFootballPlayerRole::LeftWinger ||
                           Player->GetPlayerRole() == EFootballPlayerRole::RightWinger))
            {
                Player->StartSprint();
                FVector TargetPos = CalculateAttackingPosition(Player);
                TargetPos.X += (Team->GetTeamSide() == EFootballTeamSide::Home) ? 2000.0f : -2000.0f;
                SetPlayerTargetPosition(Player, TargetPos);
            }
        }
    }
}

float UFootballTacticalComponent::GetPlayerRoleWeight(EFootballPlayerRole::Type Role, bool bAttacking) const
{
    switch (Role)
    {
    case EFootballPlayerRole::Goalkeeper:
        return bAttacking ? 0.1f : 1.0f;
    case EFootballPlayerRole::CenterBack:
    case EFootballPlayerRole::LeftBack:
    case EFootballPlayerRole::RightBack:
        return bAttacking ? 0.3f : 1.0f;
    case EFootballPlayerRole::DefensiveMidfielder:
        return bAttacking ? 0.5f : 0.9f;
    case EFootballPlayerRole::CentralMidfielder:
        return 0.8f;
    case EFootballPlayerRole::AttackingMidfielder:
        return bAttacking ? 1.0f : 0.4f;
    case EFootballPlayerRole::LeftWinger:
    case EFootballPlayerRole::RightWinger:
        return bAttacking ? 1.0f : 0.3f;
    case EFootballPlayerRole::Striker:
    case EFootballPlayerRole::CenterForward:
        return bAttacking ? 1.0f : 0.2f;
    default:
        return 0.5f;
    }
}