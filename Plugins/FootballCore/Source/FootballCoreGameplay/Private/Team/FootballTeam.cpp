// Copyright 2024 Craquez. All Rights Reserved.

#include "Team/FootballTeam.h"
#include "Player/FootballPlayerCharacter.h"
#include "Tactical/FootballTacticalComponent.h"
#include "BallPhysics/FootballBallComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"

AFootballTeam::AFootballTeam()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PostPhysics;

    TacticalComponent = CreateDefaultSubobject<UFootballTacticalComponent>(TEXT("TacticalComponent"));

    // Default formation positions (relative to team center)
    // 4-3-3 Formation
    TArray<FVector> Formation433;
    Formation433.Add(FVector(-4000, 0, 0));        // GK
    Formation433.Add(FVector(-2500, -2000, 0));    // LB
    Formation433.Add(FVector(-2500, 0, 0));        // CB1
    Formation433.Add(FVector(-2500, 2000, 0));     // CB2
    Formation433.Add(FVector(-2500, 4000, 0));     // RB
    Formation433.Add(FVector(-500, -2500, 0));     // CDM
    Formation433.Add(FVector(-500, 2500, 0));      // CM
    Formation433.Add(FVector(1500, 0, 0));         // CAM
    Formation433.Add(FVector(3500, -3000, 0));     // LW
    Formation433.Add(FVector(3500, 3000, 0));      // RW
    Formation433.Add(FVector(4500, 0, 0));         // ST
    FormationPositions.Add(433, Formation433);

    // 4-4-2 Formation
    TArray<FVector> Formation442;
    Formation442.Add(FVector(-4000, 0, 0));        // GK
    Formation442.Add(FVector(-2500, -2000, 0));    // LB
    Formation442.Add(FVector(-2500, 0, 0));        // CB1
    Formation442.Add(FVector(-2500, 2000, 0));     // CB2
    Formation442.Add(FVector(-2500, 4000, 0));     // RB
    Formation442.Add(FVector(-500, -3000, 0));     // LM
    Formation442.Add(FVector(-500, -1000, 0));     // CM1
    Formation442.Add(FVector(-500, 1000, 0));      // CM2
    Formation442.Add(FVector(-500, 3000, 0));      // RM
    Formation442.Add(FVector(3000, -1500, 0));     // ST1
    Formation442.Add(FVector(3000, 1500, 0));      // ST2
    FormationPositions.Add(442, Formation442);

    // 3-5-2 Formation
    TArray<FVector> Formation352;
    Formation352.Add(FVector(-4000, 0, 0));        // GK
    Formation352.Add(FVector(-2500, -1500, 0));    // CB1
    Formation352.Add(FVector(-2500, 1500, 0));     // CB2
    Formation352.Add(FVector(-2500, 0, 0));        // CB3
    Formation352.Add(FVector(-500, -3000, 0));     // LWB
    Formation352.Add(FVector(-500, -1000, 0));     // CM1
    Formation352.Add(FVector(-500, 1000, 0));      // CM2
    Formation352.Add(FVector(-500, 3000, 0));      // RWB
    Formation352.Add(FVector(1500, 0, 0));         // CAM
    Formation352.Add(FVector(3500, -2000, 0));     // ST1
    Formation352.Add(FVector(3500, 2000, 0));      // ST2
    FormationPositions.Add(352, Formation352);
}

void AFootballTeam::BeginPlay()
{
    Super::BeginPlay();

    // Initialize tactical component
    if (TacticalComponent)
    {
        TacticalComponent->SetTeam(this);
        TacticalComponent->Initialize(Tactics);
    }
}

void AFootballTeam::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdatePossessionTime(DeltaTime);
    UpdatePlayerPositions(DeltaTime);
    UpdateTeamShape(DeltaTime);
}

void AFootballTeam::InitializeTeam(EFootballTeamSide::Type InSide, const FString& InTeamName, const FFootballTacticalInstructions& InTactics)
{
    TeamSide = InSide;
    TeamName = InTeamName;
    Tactics = InTactics;

    if (TacticalComponent)
    {
        TacticalComponent->Initialize(Tactics);
    }
}

void AFootballTeam::SetTeamManager(AFootballTeamManager* InManager)
{
    TeamManager = InManager;
}

void AFootballTeam::AddPlayer(AFootballPlayerCharacter* Player, EFootballPlayerRole::Type Role)
{
    if (!Player || Players.Contains(Player))
    {
        return;
    }

    Players.Add(Player);
    PlayerByRole.Add(Role, Player);
    Player->SetTeam(this);
    Player->SetPlayerRole(Role);

    // Assign special roles
    if (Role == EFootballPlayerRole::Goalkeeper)
    {
        Goalkeeper = Player;
    }

    // Set captain (first player added with high leadership)
    if (!Captain && Player->GetAttributes().Leadership > 80.0f)
    {
        Captain = Player;
    }
}

void AFootballTeam::RemovePlayer(AFootballPlayerCharacter* Player)
{
    if (!Player)
    {
        return;
    }

    Players.Remove(Player);
    for (auto It = PlayerByRole.CreateIterator(); It; ++It)
    {
        if (It.Value() == Player)
        {
            It.RemoveCurrent();
            break;
        }
    }

    if (Captain == Player)
    {
        Captain = nullptr;
    }
    if (Goalkeeper == Player)
    {
        Goalkeeper = nullptr;
    }

    Player->SetTeam(nullptr);
}

void AFootballTeam::SetFormation(int32 FormationID)
{
    if (FormationPositions.Contains(FormationID))
    {
        CurrentFormation = FormationID;
        // Reposition players
        ApplyTacticalInstructions();
    }
}

void AFootballTeam::SetTactics(const FFootballTacticalInstructions& NewTactics)
{
    Tactics = NewTactics;
    if (TacticalComponent)
    {
        TacticalComponent->UpdateTactics(Tactics);
    }
}

void AFootballTeam::ApplyTactic(FGameplayTag TacticTag, float Value)
{
    if (TacticalComponent)
    {
        TacticalComponent->SetInstruction(TacticTag, Value);
    }
    OnTacticChanged.Broadcast(TacticTag);
}

AFootballPlayerCharacter* AFootballTeam::GetPlayerByRole(EFootballPlayerRole::Type Role) const
{
    if (PlayerByRole.Contains(Role))
    {
        return PlayerByRole[Role];
    }
    return nullptr;
}

TArray<AFootballPlayerCharacter*> AFootballTeam::GetPlayersByRole(EFootballPlayerRole::Type Role) const
{
    TArray<AFootballPlayerCharacter*> Result;
    for (AFootballPlayerCharacter* Player : Players)
    {
        if (Player->GetPlayerRole() == Role)
        {
            Result.Add(Player);
        }
    }
    return Result;
}

AFootballPlayerCharacter* AFootballTeam::GetClosestPlayerToBall() const
{
    if (!TeamManager)
    {
        return nullptr;
    }

    UFootballBallComponent* Ball = TeamManager->GetBall();
    if (!Ball)
    {
        return nullptr;
    }

    FVector BallLocation = Ball->GetComponentLocation();
    float MinDist = FLT_MAX;
    AFootballPlayerCharacter* ClosestPlayer = nullptr;

    for (AFootballPlayerCharacter* Player : Players)
    {
        if (Player && Player->GetController())
        {
            float Dist = FVector::Dist(Player->GetActorLocation(), BallLocation);
            if (Dist < MinDist)
            {
                MinDist = Dist;
                ClosestPlayer = Player;
            }
        }
    }

    return ClosestPlayer;
}

AFootballPlayerCharacter* AFootballTeam::GetPlayerWithBall() const
{
    for (AFootballPlayerCharacter* Player : Players)
    {
        if (Player && Player->HasBallPossession())
        {
            return Player;
        }
    }
    return nullptr;
}

void AFootballTeam::SetTeamInstruction(FGameplayTag Instruction, float Value)
{
    if (TacticalComponent)
    {
        TacticalComponent->SetInstruction(Instruction, Value);
    }
}

float AFootballTeam::GetTeamInstruction(FGameplayTag Instruction) const
{
    return TacticalComponent ? TacticalComponent->GetInstruction(Instruction) : 0.0f;
}

void AFootballTeam::SetDefensiveLine(float Value)
{
    Tactics.DefensiveLine = FMath::Clamp(Value, 0.0f, 100.0f);
    ApplyTactic(FGameplayTag::RequestGameplayTag("Tactic.DefensiveLine"), Tactics.DefensiveLine);
}

void AFootballTeam::SetEngagementLine(float Value)
{
    Tactics.EngagementLine = FMath::Clamp(Value, 0.0f, 100.0f);
    ApplyTactic(FGameplayTag::RequestGameplayTag("Tactic.EngagementLine"), Tactics.EngagementLine);
}

void AFootballTeam::SetPressingIntensity(float Value)
{
    Tactics.PressingIntensity = FMath::Clamp(Value, 0.0f, 100.0f);
    ApplyTactic(FGameplayTag::RequestGameplayTag("Tactic.PressingIntensity"), Tactics.PressingIntensity);
}

void AFootballTeam::SetOffsideTrap(bool bEnabled)
{
    Tactics.OffsideTrap = bEnabled ? 100.0f : 0.0f;
    ApplyTactic(FGameplayTag::RequestGameplayTag("Tactic.OffsideTrap"), Tactics.OffsideTrap);
}

void AFootballTeam::OnKickOff(bool bWeHaveBall)
{
    bHasPossession = bWeHaveBall;
    ApplyTacticalInstructions();

    if (bWeHaveBall)
    {
        OnPossessionGained();
    }
    else
    {
        OnPossessionLost();
    }
}

void AFootballTeam::OnPossessionGained()
{
    bHasPossession = true;
    OnPossessionChanged.Broadcast(TeamSide);
    ApplyTactic(FGameplayTag::RequestGameplayTag("Team.Attacking"), 1.0f);
}

void AFootballTeam::OnPossessionLost()
{
    bHasPossession = false;
    OnPossessionChanged.Broadcast(TeamSide == EFootballTeamSide::Home ? EFootballTeamSide::Away : EFootballTeamSide::Home);
    ApplyTactic(FGameplayTag::RequestGameplayTag("Team.Defending"), 1.0f);
}

void AFootballTeam::OnGoalScored(AFootballPlayerCharacter* Scorer)
{
    GoalsScored++;
    OnGoalScored.Broadcast(TeamSide, Scorer);
}

void AFootballTeam::OnGoalConceded()
{
    GoalsConceded++;
}

void AFootballTeam::OnSetPiece(EFootballMatchPhase::Type SetPieceType, bool bAttacking)
{
    if (bAttacking)
    {
        ApplyTactic(FGameplayTag::RequestGameplayTag("Team.SetPieceAttack"), 1.0f);
    }
    else
    {
        ApplyTactic(FGameplayTag::RequestGameplayTag("Team.SetPieceDefense"), 1.0f);
    }
}

void AFootballTeam::OnHalfTime()
{
    // Reset first half stats, apply halftime tactics
}

void AFootballTeam::OnFullTime()
{
    // Match ended
}

bool AFootballTeam::RequestSubstitution(AFootballPlayerCharacter* PlayerOut, AFootballPlayerCharacter* PlayerIn)
{
    if (!PlayerOut || !PlayerIn || !Players.Contains(PlayerOut))
    {
        return false;
    }

    EFootballPlayerRole::Type Role = PlayerOut->GetPlayerRole();
    RemovePlayer(PlayerOut);
    AddPlayer(PlayerIn, Role);

    return true;
}

void AFootballTeam::UpdatePossessionTime(float DeltaTime)
{
    if (bHasPossession)
    {
        PossessionTime += DeltaTime;
    }
}

void AFootballTeam::UpdatePlayerPositions(float DeltaTime)
{
    if (!TeamManager)
    {
        return;
    }

    UFootballBallComponent* Ball = TeamManager->GetBall();
    if (!Ball)
    {
        return;
    }

    // Update each player's tactical position
    for (AFootballPlayerCharacter* Player : Players)
    {
        if (Player && Player->GetController() && !Player->HasBallPossession())
        {
            // Tactical component handles positioning
            if (TacticalComponent)
            {
                TacticalComponent->UpdatePlayerPosition(Player, DeltaTime);
            }
        }
    }
}

void AFootballTeam::ApplyTacticalInstructions()
{
    // Position players according to formation and tactics
    if (!FormationPositions.Contains(CurrentFormation))
    {
        return;
    }

    const TArray<FVector>& Positions = FormationPositions[CurrentFormation];
    FVector TeamCenter = GetActorLocation();

    // Adjust center based on tactics
    float DefensiveOffset = (Tactics.DefensiveLine - 50.0f) * 40.0f; // cm per unit
    float EngagementOffset = (Tactics.EngagementLine - 50.0f) * 40.0f;

    if (TeamSide == EFootballTeamSide::Away)
    {
        DefensiveOffset = -DefensiveOffset;
        EngagementOffset = -EngagementOffset;
    }

    TeamCenter.X += DefensiveOffset;

    int32 PlayerIndex = 0;
    for (AFootballPlayerCharacter* Player : Players)
    {
        if (Player && PlayerIndex < Positions.Num())
        {
            FVector TargetPos = TeamCenter + Positions[PlayerIndex];

            // Adjust for width
            TargetPos.Y *= (Tactics.Width / 50.0f);

            // Adjust for defensive/engagement line based on role
            EFootballPlayerRole::Type Role = Player->GetPlayerRole();
            if (Role == EFootballPlayerRole::Goalkeeper ||
                Role == EFootballPlayerRole::CenterBack ||
                Role == EFootballPlayerRole::LeftBack ||
                Role == EFootballPlayerRole::RightBack)
            {
                TargetPos.X += DefensiveOffset;
            }
            else if (Role == EFootballPlayerRole::Striker ||
                     Role == EFootballPlayerRole::CenterForward ||
                     Role == EFootballPlayerRole::LeftWinger ||
                     Role == EFootballPlayerRole::RightWinger)
            {
                TargetPos.X += EngagementOffset;
            }

            // Send to tactical component for smooth movement
            if (TacticalComponent)
            {
                TacticalComponent->SetPlayerTargetPosition(Player, TargetPos);
            }
        }
        PlayerIndex++;
    }
}

void AFootballTeam::UpdateTeamShape(float DeltaTime)
{
    // Maintain team compactness and shape
    if (TacticalComponent)
    {
        TacticalComponent->MaintainTeamShape(DeltaTime);
    }
}

FVector AFootballTeam::CalculateFormationPosition(EFootballPlayerRole::Type Role) const
{
    if (!FormationPositions.Contains(CurrentFormation))
    {
        return FVector::ZeroVector;
    }

    // Map role to formation index
    int32 Index = 0;
    switch (Role)
    {
        case EFootballPlayerRole::Goalkeeper: Index = 0; break;
        case EFootballPlayerRole::LeftBack: Index = 1; break;
        case EFootballPlayerRole::CenterBack: Index = 2; break;
        case EFootballPlayerRole::RightBack: Index = 3; break;
        case EFootballPlayerRole::DefensiveMidfielder: Index = 4; break;
        case EFootballPlayerRole::CentralMidfielder: Index = 5; break;
        case EFootballPlayerRole::AttackingMidfielder: Index = 6; break;
        case EFootballPlayerRole::LeftWinger: Index = 7; break;
        case EFootballPlayerRole::RightWinger: Index = 8; break;
        case EFootballPlayerRole::Striker: Index = 9; break;
        case EFootballPlayerRole::CenterForward: Index = 10; break;
        default: Index = 0; break;
    }

    const TArray<FVector>& Positions = FormationPositions[CurrentFormation];
    if (Index < Positions.Num())
    {
        return GetActorLocation() + Positions[Index];
    }

    return GetActorLocation();
}