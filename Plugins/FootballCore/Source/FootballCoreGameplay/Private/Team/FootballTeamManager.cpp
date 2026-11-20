// Copyright 2024 Craquez. All Rights Reserved.

#include "Team/FootballTeamManager.h"
#include "Team/FootballTeam.h"
#include "Player/FootballPlayerCharacter.h"
#include "BallPhysics/FootballBallComponent.h"
#include "Kismet/GameplayStatics.h"

AFootballTeamManager::AFootballTeamManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PrePhysics;

    TeamPossessionTime.Add(EFootballTeamSide::Home, 0.0f);
    TeamPossessionTime.Add(EFootballTeamSide::Away, 0.0f);
    TeamPossessionTime.Add(EFootballTeamSide::Neutral, 0.0f);
}

void AFootballTeamManager::BeginPlay()
{
    Super::BeginPlay();
}

void AFootballTeamManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdatePossession(DeltaTime);
    UpdateBallTracking(DeltaTime);
    UpdateAdvantage(DeltaTime);
}

void AFootballTeamManager::SetTeams(AFootballTeam* Home, AFootballTeam* Away)
{
    HomeTeam = Home;
    AwayTeam = Away;
}

void AFootballTeamManager::SetBall(UFootballBallComponent* InBall)
{
    Ball = InBall;

    if (Ball)
    {
        Ball->OnBallKicked.AddDynamic(this, &AFootballTeamManager::OnBallKicked);
        Ball->OnBallCollision.AddDynamic(this, &AFootballTeamManager::OnBallCollision);
        Ball->OnBallStateChanged.AddDynamic(this, &AFootballTeamManager::OnBallStateChanged);
    }
}

void AFootballTeamManager::SetPossession(EFootballTeamSide::Type Team, AFootballPlayerCharacter* Player)
{
    EFootballTeamSide::Type OldTeam = PossessionTeam;
    PossessionTeam = Team;
    PossessionPlayer = Player;
    PossessionStartTime = 0.0f;

    if (OldTeam != Team && OldTeam != EFootballTeamSide::Neutral)
    {
        OnPossessionChanged.Broadcast(Team, Player);
    }

    // Notify teams
    if (Team == EFootballTeamSide::Home && HomeTeam)
    {
        HomeTeam->OnPossessionGained();
        if (AwayTeam) AwayTeam->OnPossessionLost();
    }
    else if (Team == EFootballTeamSide::Away && AwayTeam)
    {
        AwayTeam->OnPossessionGained();
        if (HomeTeam) HomeTeam->OnPossessionLost();
    }
}

void AFootballTeamManager::ClearPossession()
{
    PossessionTeam = EFootballTeamSide::Neutral;
    PossessionPlayer = nullptr;
}

AFootballTeam* AFootballTeamManager::GetTeam(EFootballTeamSide::Type Side) const
{
    switch (Side)
    {
        case EFootballTeamSide::Home: return HomeTeam;
        case EFootballTeamSide::Away: return AwayTeam;
        default: return nullptr;
    }
}

void AFootballTeamManager::RecordShot(EFootballTeamSide::Type Team, bool bOnTarget)
{
    if (Team == EFootballTeamSide::Home)
    {
        MatchStats.HomeShots++;
        if (bOnTarget) MatchStats.HomeShotsOnTarget++;
    }
    else
    {
        MatchStats.AwayShots++;
        if (bOnTarget) MatchStats.AwayShotsOnTarget++;
    }
}

void AFootballTeamManager::RecordPass(EFootballTeamSide::Type Team, bool bCompleted)
{
    if (Team == EFootballTeamSide::Home)
    {
        MatchStats.HomePasses++;
        if (bCompleted) MatchStats.HomePassAccuracy++;
    }
    else
    {
        MatchStats.AwayPasses++;
        if (bCompleted) MatchStats.AwayPassAccuracy++;
    }
}

void AFootballTeamManager::RecordTackle(EFootballTeamSide::Type Team, bool bSuccessful)
{
    // Track tackle stats
}

void AFootballTeamManager::RecordInterception(EFootballTeamSide::Type Team)
{
    // Track interception stats
}

void AFootballTeamManager::RecordSave(EFootballTeamSide::Type Team)
{
    if (Team == EFootballTeamSide::Home)
    {
        MatchStats.HomeSaves++;
    }
    else
    {
        MatchStats.AwaySaves++;
    }
}

void AFootballTeamManager::RecordFoul(EFootballTeamSide::Type Team)
{
    if (Team == EFootballTeamSide::Home)
    {
        MatchStats.HomeFouls++;
    }
    else
    {
        MatchStats.AwayFouls++;
    }
}

void AFootballTeamManager::RecordCard(EFootballTeamSide::Type Team, bool bYellow)
{
    if (Team == EFootballTeamSide::Home)
    {
        if (bYellow) MatchStats.HomeYellowCards++;
        else MatchStats.HomeRedCards++;
    }
    else
    {
        if (bYellow) MatchStats.AwayYellowCards++;
        else MatchStats.AwayRedCards++;
    }
}

float AFootballTeamManager::GetPossessionPercent(EFootballTeamSide::Type Team) const
{
    float Total = TeamPossessionTime[EFootballTeamSide::Home] + TeamPossessionTime[EFootballTeamSide::Away] + TeamPossessionTime[EFootballTeamSide::Neutral];
    if (Total <= 0.0f)
    {
        return 0.0f;
    }
    return (TeamPossessionTime[Team] / Total) * 100.0f;
}

void AFootballTeamManager::CheckAdvantage(EFootballTeamSide::Type FouledTeam, FVector Location)
{
    // Check if fouled team has a promising attack
    AFootballTeam* Team = GetTeam(FouledTeam);
    if (!Team)
    {
        return;
    }

    // Simple advantage logic: if team has ball in attacking third
    AFootballPlayerCharacter* BallCarrier = Team->GetPlayerWithBall();
    if (BallCarrier)
    {
        FVector BallPos = BallCarrier->GetActorLocation();
        float AttackDirection = (FouledTeam == EFootballTeamSide::Home) ? 1.0f : -1.0f;

        // Check if in attacking third
        if (AttackDirection * BallPos.X > 2000.0f) // Past halfway line
        {
            PlayAdvantage(3.0f); // 3 seconds advantage
        }
    }
}

void AFootballTeamManager::PlayAdvantage(float Duration)
{
    bAdvantagePlaying = true;
    AdvantageTeam = DeterminePossessionTeam();
    AdvantageEndTime = GetWorld()->GetTimeSeconds() + Duration;
}

void AFootballTeamManager::CallFoul(EFootballTeamSide::Type OffendingTeam, FVector Location, FGameplayTag FoulType)
{
    bAdvantagePlaying = false;

    // Would notify game mode to award free kick/penalty
    RecordFoul(OffendingTeam);
}

void AFootballTeamManager::CaptureDeterministicState()
{
    if (!Ball)
    {
        return;
    }

    FFootballFixedVector Pos, Vel, AngVel;
    int32 Frame;
    Ball->GetDeterministicState(Pos, Vel, AngVel, Frame);

    BallPositionHistory.Add(Pos);
    BallVelocityHistory.Add(Vel);
    HistoryFrame = Frame;

    // Limit history size
    const int32 MaxHistory = 120; // 2 seconds at 60Hz
    if (BallPositionHistory.Num() > MaxHistory)
    {
        BallPositionHistory.RemoveAt(0);
        BallVelocityHistory.RemoveAt(0);
    }
}

void AFootballTeamManager::RestoreDeterministicState()
{
    if (!Ball || BallPositionHistory.Num() == 0)
    {
        return;
    }

    // Restore to last known good state
    int32 LastIndex = BallPositionHistory.Num() - 1;
    FFootballFixedVector Pos = BallPositionHistory[LastIndex];
    FFootballFixedVector Vel = BallVelocityHistory[LastIndex];

    Ball->SetDeterministicState(Pos, Vel, FFootballFixedVector(), HistoryFrame);
}

void AFootballTeamManager::OnBallKicked(const FVector& Impulse, const FVector& Spin, AActor* Instigator)
{
    if (AFootballPlayerCharacter* Player = Cast<AFootballPlayerCharacter>(Instigator))
    {
        SetPossession(Player->GetTeamSide(), Player);
    }
}

void AFootballTeamManager::OnBallCollision(const FHitResult& HitResult, AActor* OtherActor)
{
    if (AFootballPlayerCharacter* Player = Cast<AFootballPlayerCharacter>(OtherActor))
    {
        // Ball touched by player
        SetPossession(Player->GetTeamSide(), Player);
    }
}

void AFootballTeamManager::OnBallStateChanged(EFootballBallState::Type NewState)
{
    // Track ball state changes
}

void AFootballTeamManager::UpdatePossession(float DeltaTime)
{
    if (!Ball)
    {
        return;
    }

    // Update possession time
    if (PossessionTeam != EFootballTeamSide::Neutral)
    {
        PossessionStartTime += DeltaTime;
        TeamPossessionTime[PossessionTeam] += DeltaTime;
    }
    else
    {
        TeamPossessionTime[EFootballTeamSide::Neutral] += DeltaTime;
    }

    // Check for possession change
    EFootballTeamSide::Type NewPossession = DeterminePossessionTeam();
    if (NewPossession != PossessionTeam && NewPossession != EFootballTeamSide::Neutral)
    {
        AFootballPlayerCharacter* Player = nullptr;
        if (NewPossession == EFootballTeamSide::Home && HomeTeam)
        {
            Player = HomeTeam->GetPlayerWithBall();
        }
        else if (NewPossession == EFootballTeamSide::Away && AwayTeam)
        {
            Player = AwayTeam->GetPlayerWithBall();
        }

        if (Player)
        {
            SetPossession(NewPossession, Player);
        }
    }
}

EFootballTeamSide::Type AFootballTeamManager::DeterminePossessionTeam() const
{
    if (!Ball)
    {
        return EFootballTeamSide::Neutral;
    }

    if (Ball->GetBallState() == EFootballBallState::Possessed)
    {
        AActor* Possessor = Ball->GetPossessingPlayer();
        if (AFootballPlayerCharacter* Player = Cast<AFootballPlayerCharacter>(Possessor))
        {
            return Player->GetTeamSide();
        }
    }

    // Check closest player to ball
    float HomeDist = FLT_MAX;
    float AwayDist = FLT_MAX;

    if (HomeTeam)
    {
        AFootballPlayerCharacter* Closest = HomeTeam->GetClosestPlayerToBall();
        if (Closest)
        {
            HomeDist = FVector::Dist(Closest->GetActorLocation(), Ball->GetComponentLocation());
        }
    }

    if (AwayTeam)
    {
        AFootballPlayerCharacter* Closest = AwayTeam->GetClosestPlayerToBall();
        if (Closest)
        {
            AwayDist = FVector::Dist(Closest->GetActorLocation(), Ball->GetComponentLocation());
        }
    }

    const float PossessionThreshold = 200.0f; // 2 meters

    if (HomeDist < PossessionThreshold && HomeDist < AwayDist)
    {
        return EFootballTeamSide::Home;
    }
    else if (AwayDist < PossessionThreshold && AwayDist < HomeDist)
    {
        return EFootballTeamSide::Away;
    }

    return EFootballTeamSide::Neutral;
}

void AFootballTeamManager::UpdateBallTracking(float DeltaTime)
{
    if (!Ball)
    {
        return;
    }

    // Capture deterministic state every frame
    CaptureDeterministicState();
}

void AFootballTeamManager::UpdateAdvantage(float DeltaTime)
{
    if (bAdvantagePlaying)
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime >= AdvantageEndTime)
        {
            bAdvantagePlaying = false;
            // Call the foul that was being advantaged
            // This would need to store the foul info
        }
    }
}