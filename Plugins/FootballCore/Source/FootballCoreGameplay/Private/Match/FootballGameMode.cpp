// Copyright 2024 Craquez. All Rights Reserved.

#include "Match/FootballGameMode.h"
#include "Team/FootballTeam.h"
#include "Team/FootballTeamManager.h"
#include "Player/FootballPlayerCharacter.h"
#include "BallPhysics/FootballBallComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "Engine/World.h"

AFootballGameMode::AFootballGameMode()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PrePhysics;

    // Default classes
    DefaultPawnClass = AFootballPlayerCharacter::StaticClass();
    PlayerControllerClass = APlayerController::StaticClass();
}

void AFootballGameMode::BeginPlay()
{
    Super::BeginPlay();

    InitializeTeams();
    SetupDefaultTactics();
    SpawnBall(BallSpawnLocation);
    SpawnPlayers();

    // Start match after a brief delay
    FTimerHandle StartTimer;
    GetWorldTimerManager().SetTimer(StartTimer, this, &AFootballGameMode::StartMatch, 3.0f, false);
}

void AFootballGameMode::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bIsMatchActive || bIsPaused)
    {
        return;
    }

    UpdateMatchTime(DeltaTime);
    HandlePhaseTransitions();
    UpdateBallOwnership();
    CheckGoalScored();
}

void AFootballGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
}

void AFootballGameMode::StartMatch()
{
    bIsMatchActive = true;
    bIsPaused = false;
    CurrentPhase = EFootballMatchPhase::KickOff;
    MatchTimeElapsed = 0.0f;

    // Determine kickoff team (home team starts)
    if (HomeTeam && AwayTeam)
    {
        HomeTeam->OnKickOff(true);
        AwayTeam->OnKickOff(false);
    }

    OnMatchPhaseChanged.Broadcast(CurrentPhase);
    OnMatchEvent.Broadcast(FGameplayTag::RequestGameplayTag("Match.Start"), nullptr, nullptr);
}

void AFootballGameMode::PauseMatch()
{
    bIsPaused = true;
    CurrentPhase = EFootballMatchPhase::HalfTime; // Or a paused phase
    OnMatchEvent.Broadcast(FGameplayTag::RequestGameplayTag("Match.Paused"), nullptr, nullptr);
}

void AFootballGameMode::ResumeMatch()
{
    bIsPaused = false;
    CurrentPhase = EFootballMatchPhase::OpenPlay;
    OnMatchEvent.Broadcast(FGameplayTag::RequestGameplayTag("Match.Resumed"), nullptr, nullptr);
}

void AFootballGameMode::EndMatch()
{
    bIsMatchActive = false;
    CurrentPhase = EFootballMatchPhase::FullTime;
    OnMatchPhaseChanged.Broadcast(CurrentPhase);
    OnMatchEvent.Broadcast(FGameplayTag::RequestGameplayTag("Match.End"), nullptr, nullptr);
}

void AFootballGameMode::RestartMatch()
{
    HomeScore = 0;
    AwayScore = 0;
    MatchTimeElapsed = 0.0f;
    HomeSubsUsed = 0;
    AwaySubsUsed = 0;
    CurrentPhase = EFootballMatchPhase::KickOff;

    // Reset ball
    ResetBall(EFootballMatchPhase::KickOff);

    // Reset teams
    if (HomeTeam) HomeTeam->OnKickOff(true);
    if (AwayTeam) AwayTeam->OnKickOff(false);

    StartMatch();
}

void AFootballGameMode::SetHomeTeam(AFootballTeam* Team)
{
    HomeTeam = Team;
    if (Team)
    {
        Team->InitializeTeam(EFootballTeamSide::Home, TEXT("Home"), FFootballTacticalInstructions());
    }
}

void AFootballGameMode::SetAwayTeam(AFootballTeam* Team)
{
    AwayTeam = Team;
    if (Team)
    {
        Team->InitializeTeam(EFootballTeamSide::Away, TEXT("Away"), FFootballTacticalInstructions());
    }
}

UFootballBallComponent* AFootballGameMode::GetBall() const
{
    return BallComponent;
}

void AFootballGameMode::SpawnBall(FVector Location)
{
    if (BallComponentClass)
    {
        AActor* BallActor = GetWorld()->SpawnActor<AActor>(BallComponentClass->GetDefaultObject()->GetClass(), Location, FRotator::ZeroRotator);
        if (BallActor)
        {
            BallComponent = BallActor->FindComponentByClass<UFootballBallComponent>();
            if (BallComponent)
            {
                BallComponent->InitializeBall(FFootballBallPhysicsParams());
            }
        }
    }
}

void AFootballGameMode::ResetBall(EFootballMatchPhase::Type RestartType)
{
    if (!BallComponent)
    {
        return;
    }

    FVector ResetLocation = FVector::ZeroVector;

    switch (RestartType)
    {
        case EFootballMatchPhase::KickOff:
            ResetLocation = GetKickOffPosition();
            break;
        case EFootballMatchPhase::SetPiece_GoalKick:
            ResetLocation = GetGoalKickPosition(LastTouchTeam == EFootballTeamSide::Home ? EFootballTeamSide::Away : EFootballTeamSide::Home);
            break;
        case EFootballMatchPhase::SetPiece_Corner:
            ResetLocation = GetCornerPosition(LastTouchTeam == EFootballTeamSide::Home ? EFootballTeamSide::Away : EFootballTeamSide::Home);
            break;
        case EFootballMatchPhase::SetPiece_FreeKick:
            ResetLocation = BallComponent->GetComponentLocation(); // Stay at foul location
            break;
        case EFootballMatchPhase::SetPiece_ThrowIn:
            ResetLocation = BallComponent->GetComponentLocation();
            break;
        case EFootballMatchPhase::SetPiece_Penalty:
            ResetLocation = FVector(5000, 0, 100); // Penalty spot
            break;
        default:
            ResetLocation = GetKickOffPosition();
            break;
    }

    BallComponent->ResetBall(ResetLocation);

    // Set possession based on restart type
    if (RestartType == EFootballMatchPhase::KickOff)
    {
        if (HomeTeam) HomeTeam->OnKickOff(true);
        if (AwayTeam) AwayTeam->OnKickOff(false);
    }
}

void AFootballGameMode::RegisterGoal(EFootballTeamSide::Type ScoringTeam, AFootballPlayerCharacter* Scorer)
{
    if (ScoringTeam == EFootballTeamSide::Home)
    {
        HomeScore++;
        if (HomeTeam) HomeTeam->OnGoalScored(Scorer);
        if (AwayTeam) AwayTeam->OnGoalConceded();
    }
    else
    {
        AwayScore++;
        if (AwayTeam) AwayTeam->OnGoalScored(Scorer);
        if (HomeTeam) HomeTeam->OnGoalConceded();
    }

    OnGoalScored.Broadcast(ScoringTeam, Scorer);
    OnMatchEvent.Broadcast(FGameplayTag::RequestGameplayTag("Match.Goal"), Scorer, nullptr);

    // Restart with kickoff for conceding team
    FTimerHandle RestartTimer;
    GetWorldTimerManager().SetTimer(RestartTimer, [this, ScoringTeam]()
    {
        ResetBall(EFootballMatchPhase::KickOff);
        if (ScoringTeam == EFootballTeamSide::Home)
        {
            if (AwayTeam) AwayTeam->OnKickOff(true);
            if (HomeTeam) HomeTeam->OnKickOff(false);
        }
        else
        {
            if (HomeTeam) HomeTeam->OnKickOff(true);
            if (AwayTeam) AwayTeam->OnKickOff(false);
        }
    }, 5.0f, false);
}

void AFootballGameMode::SetMatchPhase(EFootballMatchPhase::Type NewPhase)
{
    if (CurrentPhase != NewPhase)
    {
        CurrentPhase = NewPhase;
        OnMatchPhaseChanged.Broadcast(NewPhase);
    }
}

void AFootballGameMode::CheckOffside(AFootballPlayerCharacter* Attacker)
{
    if (!Attacker || !Attacker->GetTeam())
    {
        return;
    }

    AFootballTeam* AttackerTeam = Attacker->GetTeam();
    AFootballTeam* DefenderTeam = (AttackerTeam->GetTeamSide() == EFootballTeamSide::Home) ? AwayTeam : HomeTeam;

    if (!DefenderTeam)
    {
        return;
    }

    // Get offside line from defending team
    UFootballTacticalComponent* DefenderTactical = DefenderTeam->GetTacticalComponent();
    if (!DefenderTactical)
    {
        return;
    }

    float OffsideLine = DefenderTactical->CalculateOffsideLine();
    float AttackerX = Attacker->GetActorLocation().X;

    bool bIsOffside = false;
    if (AttackerTeam->GetTeamSide() == EFootballTeamSide::Home)
    {
        // Home attacks toward positive X
        bIsOffside = AttackerX > OffsideLine;
    }
    else
    {
        // Away attacks toward negative X
        bIsOffside = AttackerX < OffsideLine;
    }

    if (bIsOffside)
    {
        // Award free kick to defending team
        AwardFreeKick(DefenderTeam->GetTeamSide(), Attacker->GetActorLocation(), FGameplayTag::RequestGameplayTag("Foul.Offside"));
        OnMatchEvent.Broadcast(FGameplayTag::RequestGameplayTag("Foul.Offside"), Attacker, nullptr);
    }
}

void AFootballGameMode::RegisterFoul(EFootballTeamSide::Type OffendingTeam, AFootballPlayerCharacter* Offender, FVector Location, FGameplayTag FoulType)
{
    // Update stats
    if (OffendingTeam == EFootballTeamSide::Home)
    {
        if (HomeTeam) HomeTeam->FoulsCommitted++;
    }
    else
    {
        if (AwayTeam) AwayTeam->FoulsCommitted++;
    }

    // Determine sanction
    if (FoulType == FGameplayTag::RequestGameplayTag("Foul.YellowCard"))
    {
        if (OffendingTeam == EFootballTeamSide::Home)
        {
            HomeTeam->YellowCards++;
        }
        else
        {
            AwayTeam->YellowCards++;
        }
    }
    else if (FoulType == FGameplayTag::RequestGameplayTag("Foul.RedCard"))
    {
        if (OffendingTeam == EFootballTeamSide::Home)
        {
            HomeTeam->RedCards++;
        }
        else
        {
            AwayTeam->RedCards++;
        }
        // Send off player
        if (Offender)
        {
            Offender->Destroy();
        }
    }

    // Award free kick or penalty
    EFootballTeamSide::Type VictimTeam = (OffendingTeam == EFootballTeamSide::Home) ? EFootballTeamSide::Away : EFootballTeamSide::Home;

    // Check if in penalty area
    bool bInPenaltyArea = false; // Would check location against penalty area

    if (bInPenaltyArea)
    {
        AwardPenalty(VictimTeam, Location);
    }
    else
    {
        AwardFreeKick(VictimTeam, Location, FoulType);
    }

    OnMatchEvent.Broadcast(FGameplayTag::RequestGameplayTag("Foul.Committed"), Offender, nullptr);
}

void AFootballGameMode::AwardFreeKick(EFootballTeamSide::Type Team, FVector Location, FGameplayTag FoulType)
{
    SetMatchPhase(EFootballMatchPhase::SetPiece_FreeKick);
    LastTouchTeam = Team;

    if (Team == EFootballTeamSide::Home && HomeTeam)
    {
        HomeTeam->OnSetPiece(EFootballMatchPhase::SetPiece_FreeKick, true);
        if (AwayTeam) AwayTeam->OnSetPiece(EFootballMatchPhase::SetPiece_FreeKick, false);
    }
    else if (AwayTeam)
    {
        AwayTeam->OnSetPiece(EFootballMatchPhase::SetPiece_FreeKick, true);
        if (HomeTeam) HomeTeam->OnSetPiece(EFootballMatchPhase::SetPiece_FreeKick, false);
    }

    ResetBall(EFootballMatchPhase::SetPiece_FreeKick);
    OnMatchEvent.Broadcast(FGameplayTag::RequestGameplayTag("SetPiece.FreeKick"), nullptr, nullptr);
}

void AFootballGameMode::AwardPenalty(EFootballTeamSide::Type Team, FVector Location)
{
    SetMatchPhase(EFootballMatchPhase::SetPiece_Penalty);
    LastTouchTeam = Team;

    if (Team == EFootballTeamSide::Home && HomeTeam)
    {
        HomeTeam->OnSetPiece(EFootballMatchPhase::SetPiece_Penalty, true);
        if (AwayTeam) AwayTeam->OnSetPiece(EFootballMatchPhase::SetPiece_Penalty, false);
    }
    else if (AwayTeam)
    {
        AwayTeam->OnSetPiece(EFootballMatchPhase::SetPiece_Penalty, true);
        if (HomeTeam) HomeTeam->OnSetPiece(EFootballMatchPhase::SetPiece_Penalty, false);
    }

    ResetBall(EFootballMatchPhase::SetPiece_Penalty);
    OnMatchEvent.Broadcast(FGameplayTag::RequestGameplayTag("SetPiece.Penalty"), nullptr, nullptr);
}

void AFootballGameMode::AwardCorner(EFootballTeamSide::Type Team)
{
    SetMatchPhase(EFootballMatchPhase::SetPiece_Corner);
    LastTouchTeam = (Team == EFootballTeamSide::Home) ? EFootballTeamSide::Away : EFootballTeamSide::Home;

    if (Team == EFootballTeamSide::Home && HomeTeam)
    {
        HomeTeam->OnSetPiece(EFootballMatchPhase::SetPiece_Corner, true);
        HomeTeam->Corners++;
        if (AwayTeam) AwayTeam->OnSetPiece(EFootballMatchPhase::SetPiece_Corner, false);
    }
    else if (AwayTeam)
    {
        AwayTeam->OnSetPiece(EFootballMatchPhase::SetPiece_Corner, true);
        AwayTeam->Corners++;
        if (HomeTeam) HomeTeam->OnSetPiece(EFootballMatchPhase::SetPiece_Corner, false);
    }

    ResetBall(EFootballMatchPhase::SetPiece_Corner);
    OnMatchEvent.Broadcast(FGameplayTag::RequestGameplayTag("SetPiece.Corner"), nullptr, nullptr);
}

void AFootballGameMode::AwardGoalKick(EFootballTeamSide::Type Team)
{
    SetMatchPhase(EFootballMatchPhase::SetPiece_GoalKick);
    LastTouchTeam = (Team == EFootballTeamSide::Home) ? EFootballTeamSide::Away : EFootballTeamSide::Home;

    if (Team == EFootballTeamSide::Home && HomeTeam)
    {
        HomeTeam->OnSetPiece(EFootballMatchPhase::SetPiece_GoalKick, true);
        if (AwayTeam) AwayTeam->OnSetPiece(EFootballMatchPhase::SetPiece_GoalKick, false);
    }
    else if (AwayTeam)
    {
        AwayTeam->OnSetPiece(EFootballMatchPhase::SetPiece_GoalKick, true);
        if (HomeTeam) HomeTeam->OnSetPiece(EFootballMatchPhase::SetPiece_GoalKick, false);
    }

    ResetBall(EFootballMatchPhase::SetPiece_GoalKick);
    OnMatchEvent.Broadcast(FGameplayTag::RequestGameplayTag("SetPiece.GoalKick"), nullptr, nullptr);
}

void AFootballGameMode::AwardThrowIn(EFootballTeamSide::Type Team, FVector Location)
{
    SetMatchPhase(EFootballMatchPhase::SetPiece_ThrowIn);
    LastTouchTeam = (Team == EFootballTeamSide::Home) ? EFootballTeamSide::Away : EFootballTeamSide::Home;

    if (Team == EFootballTeamSide::Home && HomeTeam)
    {
        HomeTeam->OnSetPiece(EFootballMatchPhase::SetPiece_ThrowIn, true);
        if (AwayTeam) AwayTeam->OnSetPiece(EFootballMatchPhase::SetPiece_ThrowIn, false);
    }
    else if (AwayTeam)
    {
        AwayTeam->OnSetPiece(EFootballMatchPhase::SetPiece_ThrowIn, true);
        if (HomeTeam) HomeTeam->OnSetPiece(EFootballMatchPhase::SetPiece_ThrowIn, false);
    }

    ResetBall(EFootballMatchPhase::SetPiece_ThrowIn);
    OnMatchEvent.Broadcast(FGameplayTag::RequestGameplayTag("SetPiece.ThrowIn"), nullptr, nullptr);
}

bool AFootballGameMode::ProcessSubstitution(EFootballTeamSide::Type Team, AFootballPlayerCharacter* PlayerOut, AFootballPlayerCharacter* PlayerIn)
{
    if (!bAllowSubstitutions)
    {
        return false;
    }

    AFootballTeam* TargetTeam = (Team == EFootballTeamSide::Home) ? HomeTeam : AwayTeam;
    int32& SubsUsed = (Team == EFootballTeamSide::Home) ? HomeSubsUsed : AwaySubsUsed;

    if (!TargetTeam || SubsUsed >= MaxSubstitutions)
    {
        return false;
    }

    bool bSuccess = TargetTeam->RequestSubstitution(PlayerOut, PlayerIn);
    if (bSuccess)
    {
        SubsUsed++;
        OnMatchEvent.Broadcast(FGameplayTag::RequestGameplayTag("Match.Substitution"), PlayerOut, PlayerIn);
    }

    return bSuccess;
}

void AFootballGameMode::SetGameMode(EFootballGameMode::Type Mode)
{
    GameModeType = Mode;

    // Adjust team sizes based on mode
    switch (Mode)
    {
        case EFootballGameMode::OneVOne_StrikerVsKeeper:
            // Only spawn 1 striker and 1 keeper per team
            break;
        case EFootballGameMode::TrainingMatch_11v11:
        default:
            // Full 11v11
            break;
    }
}

void AFootballGameMode::SpawnPlayers()
{
    if (!PlayerCharacterClass || !TeamClass)
    {
        return;
    }

    // Spawn home team players
    if (HomeTeam)
    {
        SpawnTeamPlayers(HomeTeam, EFootballTeamSide::Home);
    }

    // Spawn away team players
    if (AwayTeam)
    {
        SpawnTeamPlayers(AwayTeam, EFootballTeamSide::Away);
    }
}

void AFootballGameMode::SpawnTeamPlayers(AFootballTeam* Team, EFootballTeamSide::Type Side)
{
    if (!Team || !FormationPositions.Contains(Team->CurrentFormation))
    {
        return;
    }

    const TArray<FVector>& Positions = FormationPositions[Team->CurrentFormation];
    FVector TeamCenter = Team->GetActorLocation();

    // Adjust for side
    if (Side == EFootballTeamSide::Away)
    {
        TeamCenter.X = -TeamCenter.X;
    }

    int32 PlayerIndex = 0;
    for (AFootballPlayerCharacter* Player : Team->GetAllPlayers())
    {
        if (Player && PlayerIndex < Positions.Num())
        {
            FVector SpawnPos = TeamCenter + Positions[PlayerIndex];

            if (Side == EFootballTeamSide::Away)
            {
                SpawnPos.X = -SpawnPos.X;
                SpawnPos.Y = -SpawnPos.Y;
            }

            FRotator SpawnRot = (Side == EFootballTeamSide::Home) ? FRotator(0, 0, 0) : FRotator(0, 180, 0);

            // Player already exists, just position it
            Player->SetActorLocationAndRotation(SpawnPos, SpawnRot);
            PlayerIndex++;
        }
    }
}

AFootballPlayerCharacter* AFootballGameMode::SpawnPlayer(EFootballTeamSide::Type TeamSide, EFootballPlayerRole::Type Role, FVector Location, FRotator Rotation)
{
    if (!PlayerCharacterClass)
    {
        return nullptr;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AFootballPlayerCharacter* Player = GetWorld()->SpawnActor<AFootballPlayerCharacter>(PlayerCharacterClass, Location, Rotation, SpawnParams);
    if (Player)
    {
        Player->SetPlayerRole(Role);
        AFootballTeam* Team = (TeamSide == EFootballTeamSide::Home) ? HomeTeam : AwayTeam;
        if (Team)
        {
            Team->AddPlayer(Player, Role);
        }
    }

    return Player;
}

void AFootballGameMode::UpdateMatchTime(float DeltaTime)
{
    MatchTimeElapsed += DeltaTime;

    float TotalMatchSeconds = MatchDuration * 60.0f;
    float TimeRemaining = TotalMatchSeconds - MatchTimeElapsed;

    OnMatchTimeChanged.Broadcast(TimeRemaining);

    // Check for half time
    float HalfSeconds = HalfDuration * 60.0f;
    if (MatchTimeElapsed >= HalfSeconds && CurrentPhase != EFootballMatchPhase::HalfTime && CurrentPhase != EFootballMatchPhase::FullTime)
    {
        HandleHalfTime();
    }

    // Check for full time
    if (MatchTimeElapsed >= TotalMatchSeconds)
    {
        HandleFullTime();
    }
}

void AFootballGameMode::HandlePhaseTransitions()
{
    // Handle transitions between match phases
    if (CurrentPhase == EFootballMatchPhase::KickOff)
    {
        // Wait for kickoff to be taken
        if (BallComponent && BallComponent->GetBallState() == EFootballBallState::Flying)
        {
            SetMatchPhase(EFootballMatchPhase::OpenPlay);
        }
    }
    else if (CurrentPhase == EFootballMatchPhase::SetPiece_FreeKick ||
             CurrentPhase == EFootballMatchPhase::SetPiece_Corner ||
             CurrentPhase == EFootballMatchPhase::SetPiece_Penalty ||
             CurrentPhase == EFootballMatchPhase::SetPiece_ThrowIn ||
             CurrentPhase == EFootballMatchPhase::SetPiece_GoalKick)
    {
        // Wait for set piece to be taken
        if (BallComponent && BallComponent->GetBallState() == EFootballBallState::Flying)
        {
            SetMatchPhase(EFootballMatchPhase::OpenPlay);
        }
    }
}

void AFootballGameMode::HandleKickOff()
{
    // Kickoff logic
}

void AFootballGameMode::HandleHalfTime()
{
    SetMatchPhase(EFootballMatchPhase::HalfTime);
    bIsMatchActive = false;

    if (HomeTeam) HomeTeam->OnHalfTime();
    if (AwayTeam) AwayTeam->OnHalfTime();

    OnMatchEvent.Broadcast(FGameplayTag::RequestGameplayTag("Match.HalfTime"), nullptr, nullptr);

    // Auto-resume after delay for second half
    FTimerHandle ResumeTimer;
    GetWorldTimerManager().SetTimer(ResumeTimer, [this]()
    {
        CurrentPhase = EFootballMatchPhase::KickOff;
        // Away team kicks off second half
        if (HomeTeam) HomeTeam->OnKickOff(false);
        if (AwayTeam) AwayTeam->OnKickOff(true);
        ResetBall(EFootballMatchPhase::KickOff);
        bIsMatchActive = true;
    }, 15.0f, false);
}

void AFootballGameMode::HandleFullTime()
{
    EndMatch();

    if (HomeTeam) HomeTeam->OnFullTime();
    if (AwayTeam) AwayTeam->OnFullTime();

    OnMatchEvent.Broadcast(FGameplayTag::RequestGameplayTag("Match.FullTime"), nullptr, nullptr);
}

void AFootballGameMode::ProcessBallOutOfPlay()
{
    if (!BallComponent)
    {
        return;
    }

    EFootballBallState::Type BallState = BallComponent->GetBallState();

    if (BallState == EFootballBallState::OutOfPlay)
    {
        // Determine restart type based on last touch and location
        FVector BallLoc = BallComponent->GetComponentLocation();

        // Simplified - would check actual field boundaries
        if (FMath::Abs(BallLoc.Y) > 6000.0f) // Touchline
        {
            AwardThrowIn(LastTouchTeam == EFootballTeamSide::Home ? EFootballTeamSide::Away : EFootballTeamSide::Home, BallLoc);
        }
        else if (BallLoc.X > 5200.0f) // Goal line home side
        {
            if (LastTouchTeam == EFootballTeamSide::Home)
            {
                AwardCorner(EFootballTeamSide::Away);
            }
            else
            {
                AwardGoalKick(EFootballTeamSide::Home);
            }
        }
        else if (BallLoc.X < -5200.0f) // Goal line away side
        {
            if (LastTouchTeam == EFootballTeamSide::Away)
            {
                AwardCorner(EFootballTeamSide::Home);
            }
            else
            {
                AwardGoalKick(EFootballTeamSide::Away);
            }
        }
    }
}

void AFootballGameMode::UpdateBallOwnership()
{
    if (!BallComponent)
    {
        return;
    }

    // Track last touch team
    if (BallComponent->GetBallState() == EFootballBallState::Possessed)
    {
        AActor* Possessor = BallComponent->GetPossessingPlayer();
        if (AFootballPlayerCharacter* Player = Cast<AFootballPlayerCharacter>(Possessor))
        {
            LastTouchPlayer = Player;
            LastTouchTeam = Player->GetTeamSide();
        }
    }
}

void AFootballGameMode::CheckGoalScored()
{
    if (!BallComponent)
    {
        return;
    }

    if (BallComponent->GetBallState() == EFootballBallState::Goal)
    {
        // Goal already registered via ball component event
    }
}

FVector AFootballGameMode::GetKickOffPosition() const
{
    return FVector(0, 0, 100); // Center spot
}

FVector AFootballGameMode::GetGoalKickPosition(EFootballTeamSide::Type Team) const
{
    if (Team == EFootballTeamSide::Home)
    {
        return FVector(-5000, 0, 100); // Home goal area
    }
    else
    {
        return FVector(5000, 0, 100); // Away goal area
    }
}

FVector AFootballGameMode::GetCornerPosition(EFootballTeamSide::Type Team) const
{
    if (Team == EFootballTeamSide::Home)
    {
        return FVector(5000, 5000, 100); // Away corner (home attacking)
    }
    else
    {
        return FVector(-5000, -5000, 100); // Home corner (away attacking)
    }
}

void AFootballGameMode::InitializeTeams()
{
    if (!TeamClass)
    {
        return;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    // Spawn home team
    HomeTeam = GetWorld()->SpawnActor<AFootballTeam>(TeamClass, FVector(-5000, 0, 0), FRotator::ZeroRotator, SpawnParams);
    if (HomeTeam)
    {
        HomeTeam->InitializeTeam(EFootballTeamSide::Home, TEXT("Craquez FC"), FFootballTacticalInstructions());
    }

    // Spawn away team
    AwayTeam = GetWorld()->SpawnActor<AFootballTeam>(TeamClass, FVector(5000, 0, 0), FRotator(0, 180, 0), SpawnParams);
    if (AwayTeam)
    {
        AwayTeam->InitializeTeam(EFootballTeamSide::Away, TEXT("Opponents"), FFootballTacticalInstructions());
    }

    // Spawn team manager
    TeamManager = GetWorld()->SpawnActor<AFootballTeamManager>(AFootballTeamManager::StaticClass());
    if (TeamManager)
    {
        TeamManager->SetTeams(HomeTeam, AwayTeam);
        TeamManager->SetBall(BallComponent);
        if (HomeTeam) HomeTeam->SetTeamManager(TeamManager);
        if (AwayTeam) AwayTeam->SetTeamManager(TeamManager);
    }
}

void AFootballGameMode::SetupDefaultTactics()
{
    // Setup default tactical instructions for both teams
    if (HomeTeam)
    {
        HomeTeam->SetTactics(FFootballTacticalInstructions());
    }
    if (AwayTeam)
    {
        FFootballTacticalInstructions AwayTactics;
        AwayTactics.PressingIntensity = 40.0f; // Slightly less pressing
        AwayTeam->SetTactics(AwayTactics);
    }
}