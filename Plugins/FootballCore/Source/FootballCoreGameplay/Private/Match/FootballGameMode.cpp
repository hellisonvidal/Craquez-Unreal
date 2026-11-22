// Copyright 2024 Craquez. All Rights Reserved.

#include "Match/FootballGameMode.h"
#include "BallPhysics/FootballBallActor.h"
#include "BallPhysics/FootballBallComponent.h"
#include "Engine/World.h"
#include "GameFramework/HUD.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Player/FootballPlayerCharacter.h"
#include "Tactical/FootballTacticalComponent.h"
#include "Team/FootballTeam.h"
#include "Team/FootballTeamManager.h"

AFootballGameMode::AFootballGameMode()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PrePhysics;

    // Default classes so match, players, teams, ball, and HUD work out-of-the-box
    DefaultPawnClass = AFootballPlayerCharacter::StaticClass();
    PlayerControllerClass = APlayerController::StaticClass();
    PlayerCharacterClass = AFootballPlayerCharacter::StaticClass();
    TeamClass = AFootballTeam::StaticClass();
    BallComponentClass = UFootballBallComponent::StaticClass();
}

void AFootballGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
    Super::InitGame(MapName, Options, ErrorMessage);

    if (UClass* LoadedHUDClass =
            StaticLoadClass(AHUD::StaticClass(), nullptr, TEXT("/Script/FootballCoreUI.FootballHUD")))
    {
        HUDClass = LoadedHUDClass;
    }
}

void AFootballGameMode::BeginPlay()
{
    Super::BeginPlay();

    // Auto-detect exact turf surface height from the imported SM_CraquezPitch mesh
    PitchSurfaceZ = DetectPitchSurfaceZ(0.0f, 0.0f);
    BallSpawnLocation = FVector(0.0f, 0.0f, PitchSurfaceZ + 22.0f);

    SpawnBall(BallSpawnLocation);
    InitializeTeams();
    SetupDefaultTactics();
    SpawnPlayers();

    // Enable click/touch events on PlayerController for Android HUD hit boxes
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
        PC->bShowMouseCursor = true;
        PC->bEnableClickEvents = true;
        PC->bEnableTouchEvents = true;

        // Possess the Home Striker so the action camera immediately frames the full-color match
        if (HomeTeam)
        {
            AFootballPlayerCharacter* ControlledPlayer = HomeTeam->GetPlayerByRole(EFootballPlayerRole::Striker);
            if (!ControlledPlayer && HomeTeam->GetAllPlayers().Num() > 0)
            {
                ControlledPlayer = HomeTeam->GetAllPlayers().Last();
            }
            if (ControlledPlayer)
            {
                PC->Possess(ControlledPlayer);
            }
        }
    }

    // Start match right away so the field is alive
    StartMatch();
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
    UpdateSimpleMatchAI(DeltaTime);
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
    if (HomeTeam)
        HomeTeam->OnKickOff(true);
    if (AwayTeam)
        AwayTeam->OnKickOff(false);

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
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AFootballBallActor* BallActor = GetWorld()->SpawnActor<AFootballBallActor>(
        AFootballBallActor::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);
    if (BallActor)
    {
        BallComponent = BallActor->GetBallComponent();
        if (BallComponent)
        {
            BallComponent->InitializeBall(FFootballBallPhysicsParams());
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
        ResetLocation = GetGoalKickPosition(LastTouchTeam == EFootballTeamSide::Home ? EFootballTeamSide::Away
                                                                                     : EFootballTeamSide::Home);
        break;
    case EFootballMatchPhase::SetPiece_Corner:
        ResetLocation = GetCornerPosition(LastTouchTeam == EFootballTeamSide::Home ? EFootballTeamSide::Away
                                                                                   : EFootballTeamSide::Home);
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
        if (HomeTeam)
            HomeTeam->OnKickOff(true);
        if (AwayTeam)
            AwayTeam->OnKickOff(false);
    }
}

void AFootballGameMode::RegisterGoal(EFootballTeamSide::Type ScoringTeam, AFootballPlayerCharacter* Scorer)
{
    if (ScoringTeam == EFootballTeamSide::Home)
    {
        HomeScore++;
        if (HomeTeam)
            HomeTeam->OnGoalScored(Scorer);
        if (AwayTeam)
            AwayTeam->OnGoalConceded();
    }
    else
    {
        AwayScore++;
        if (AwayTeam)
            AwayTeam->OnGoalScored(Scorer);
        if (HomeTeam)
            HomeTeam->OnGoalConceded();
    }

    OnGoalScored.Broadcast(ScoringTeam, Scorer);
    OnMatchEvent.Broadcast(FGameplayTag::RequestGameplayTag("Match.Goal"), Scorer, nullptr);

    // Restart with kickoff for conceding team
    FTimerHandle RestartTimer;
    GetWorldTimerManager().SetTimer(
        RestartTimer,
        [this, ScoringTeam]()
        {
            ResetBall(EFootballMatchPhase::KickOff);
            if (ScoringTeam == EFootballTeamSide::Home)
            {
                if (AwayTeam)
                    AwayTeam->OnKickOff(true);
                if (HomeTeam)
                    HomeTeam->OnKickOff(false);
            }
            else
            {
                if (HomeTeam)
                    HomeTeam->OnKickOff(true);
                if (AwayTeam)
                    AwayTeam->OnKickOff(false);
            }
        },
        5.0f, false);
}

float AFootballGameMode::GetMatchTimeRemaining() const
{
    const float TotalMatchSeconds = MatchDuration * 60.0f;
    return FMath::Max(0.0f, TotalMatchSeconds - MatchTimeElapsed);
}

float AFootballGameMode::GetMatchTimeElapsed() const
{
    return MatchTimeElapsed;
}

int32 AFootballGameMode::GetCurrentMinute() const
{
    return FMath::FloorToInt(MatchTimeElapsed / 60.0f);
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
        AwardFreeKick(DefenderTeam->GetTeamSide(), Attacker->GetActorLocation(),
                      FGameplayTag::RequestGameplayTag("Foul.Offside"));
        OnMatchEvent.Broadcast(FGameplayTag::RequestGameplayTag("Foul.Offside"), Attacker, nullptr);
    }
}

void AFootballGameMode::RegisterFoul(EFootballTeamSide::Type OffendingTeam, AFootballPlayerCharacter* Offender,
                                     FVector Location, FGameplayTag FoulType)
{
    // Update stats
    if (OffendingTeam == EFootballTeamSide::Home)
    {
        if (HomeTeam)
            HomeTeam->AddFoul();
    }
    else
    {
        if (AwayTeam)
            AwayTeam->AddFoul();
    }

    // Determine sanction
    if (FoulType == FGameplayTag::RequestGameplayTag("Foul.YellowCard"))
    {
        if (OffendingTeam == EFootballTeamSide::Home)
        {
            if (HomeTeam)
                HomeTeam->AddYellowCard();
        }
        else
        {
            if (AwayTeam)
                AwayTeam->AddYellowCard();
        }
    }
    else if (FoulType == FGameplayTag::RequestGameplayTag("Foul.RedCard"))
    {
        if (OffendingTeam == EFootballTeamSide::Home)
        {
            if (HomeTeam)
                HomeTeam->AddRedCard();
        }
        else
        {
            if (AwayTeam)
                AwayTeam->AddRedCard();
        }
        // Send off player
        if (Offender)
        {
            Offender->Destroy();
        }
    }

    // Award free kick or penalty
    EFootballTeamSide::Type VictimTeam =
        (OffendingTeam == EFootballTeamSide::Home) ? EFootballTeamSide::Away : EFootballTeamSide::Home;

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
        if (AwayTeam)
            AwayTeam->OnSetPiece(EFootballMatchPhase::SetPiece_FreeKick, false);
    }
    else if (AwayTeam)
    {
        AwayTeam->OnSetPiece(EFootballMatchPhase::SetPiece_FreeKick, true);
        if (HomeTeam)
            HomeTeam->OnSetPiece(EFootballMatchPhase::SetPiece_FreeKick, false);
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
        if (AwayTeam)
            AwayTeam->OnSetPiece(EFootballMatchPhase::SetPiece_Penalty, false);
    }
    else if (AwayTeam)
    {
        AwayTeam->OnSetPiece(EFootballMatchPhase::SetPiece_Penalty, true);
        if (HomeTeam)
            HomeTeam->OnSetPiece(EFootballMatchPhase::SetPiece_Penalty, false);
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
        HomeTeam->AddCorner();
        if (AwayTeam)
            AwayTeam->OnSetPiece(EFootballMatchPhase::SetPiece_Corner, false);
    }
    else if (AwayTeam)
    {
        AwayTeam->OnSetPiece(EFootballMatchPhase::SetPiece_Corner, true);
        AwayTeam->AddCorner();
        if (HomeTeam)
            HomeTeam->OnSetPiece(EFootballMatchPhase::SetPiece_Corner, false);
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
        if (AwayTeam)
            AwayTeam->OnSetPiece(EFootballMatchPhase::SetPiece_GoalKick, false);
    }
    else if (AwayTeam)
    {
        AwayTeam->OnSetPiece(EFootballMatchPhase::SetPiece_GoalKick, true);
        if (HomeTeam)
            HomeTeam->OnSetPiece(EFootballMatchPhase::SetPiece_GoalKick, false);
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
        if (AwayTeam)
            AwayTeam->OnSetPiece(EFootballMatchPhase::SetPiece_ThrowIn, false);
    }
    else if (AwayTeam)
    {
        AwayTeam->OnSetPiece(EFootballMatchPhase::SetPiece_ThrowIn, true);
        if (HomeTeam)
            HomeTeam->OnSetPiece(EFootballMatchPhase::SetPiece_ThrowIn, false);
    }

    ResetBall(EFootballMatchPhase::SetPiece_ThrowIn);
    OnMatchEvent.Broadcast(FGameplayTag::RequestGameplayTag("SetPiece.ThrowIn"), nullptr, nullptr);
}

bool AFootballGameMode::ProcessSubstitution(EFootballTeamSide::Type Team, AFootballPlayerCharacter* PlayerOut,
                                            AFootballPlayerCharacter* PlayerIn)
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
    SpawnPlayers();
    ResetBall(EFootballMatchPhase::KickOff);
}

void AFootballGameMode::ToggleTrainingOrDuelMode()
{
    if (GameModeType == EFootballGameMode::TrainingMatch_11v11)
    {
        SetGameMode(EFootballGameMode::OneVOne_StrikerVsKeeper);
    }
    else
    {
        SetGameMode(EFootballGameMode::TrainingMatch_11v11);
    }
}

void AFootballGameMode::SpawnPlayers()
{
    if (!PlayerCharacterClass || !TeamClass)
    {
        return;
    }

    // Clear existing players when spawning/switching modes
    auto ClearTeamPlayers = [](AFootballTeam* Team)
    {
        if (!Team)
        {
            return;
        }
        TArray<AFootballPlayerCharacter*> Existing = Team->GetAllPlayers();
        for (AFootballPlayerCharacter* P : Existing)
        {
            if (P)
            {
                Team->RemovePlayer(P);
                P->Destroy();
            }
        }
    };
    ClearTeamPlayers(HomeTeam);
    ClearTeamPlayers(AwayTeam);

    if (GameModeType == EFootballGameMode::OneVOne_StrikerVsKeeper)
    {
        // 1v1 Striker vs Keeper duel
        if (HomeTeam)
        {
            float Z = DetectPitchSurfaceZ(-900.0f, 0.0f) + 96.0f;
            AFootballPlayerCharacter* Striker = SpawnPlayer(EFootballTeamSide::Home, EFootballPlayerRole::Striker,
                                                            FVector(-900.0f, 0.0f, Z), FRotator(0.0f, 0.0f, 0.0f));
            if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
            {
                if (Striker)
                {
                    PC->Possess(Striker);
                }
            }
        }
        if (AwayTeam)
        {
            float Z = DetectPitchSurfaceZ(3200.0f, 0.0f) + 96.0f;
            SpawnPlayer(EFootballTeamSide::Away, EFootballPlayerRole::Goalkeeper, FVector(3200.0f, 0.0f, Z),
                        FRotator(0.0f, 180.0f, 0.0f));
        }
        return;
    }

    // Full 11v11 Training Match
    if (HomeTeam)
    {
        SpawnTeamPlayers(HomeTeam, EFootballTeamSide::Home);
    }
    if (AwayTeam)
    {
        SpawnTeamPlayers(AwayTeam, EFootballTeamSide::Away);
    }
}

void AFootballGameMode::SpawnTeamPlayers(AFootballTeam* Team, EFootballTeamSide::Type Side)
{
    if (!Team)
    {
        return;
    }

    static const EFootballPlayerRole::Type Roles11[11] = {
        EFootballPlayerRole::Goalkeeper,        EFootballPlayerRole::LeftBack,
        EFootballPlayerRole::CenterBack,        EFootballPlayerRole::CenterBack,
        EFootballPlayerRole::RightBack,         EFootballPlayerRole::DefensiveMidfielder,
        EFootballPlayerRole::CentralMidfielder, EFootballPlayerRole::AttackingMidfielder,
        EFootballPlayerRole::LeftWinger,        EFootballPlayerRole::Striker,
        EFootballPlayerRole::RightWinger,
    };

    // Compact balanced 4-3-3 layout fitting cleanly on the training ground pitch
    static const FVector HomeOffsets[11] = {
        FVector(-3400.0f, 0.0f, 0.0f),     // GK
        FVector(-2300.0f, -1500.0f, 0.0f), // LB
        FVector(-2400.0f, -500.0f, 0.0f),  // CB1
        FVector(-2400.0f, 500.0f, 0.0f),   // CB2
        FVector(-2300.0f, 1500.0f, 0.0f),  // RB
        FVector(-1400.0f, 0.0f, 0.0f),     // CDM
        FVector(-1000.0f, -900.0f, 0.0f),  // CM
        FVector(-1000.0f, 900.0f, 0.0f),   // CAM
        FVector(-450.0f, -1400.0f, 0.0f),  // LW
        FVector(-280.0f, 0.0f, 0.0f),      // ST
        FVector(-450.0f, 1400.0f, 0.0f),   // RW
    };

    FRotator SpawnRot = (Side == EFootballTeamSide::Home) ? FRotator(0.0f, 0.0f, 0.0f) : FRotator(0.0f, 180.0f, 0.0f);

    for (int32 i = 0; i < 11; ++i)
    {
        FVector Pos = HomeOffsets[i];
        if (Side == EFootballTeamSide::Away)
        {
            Pos.X = -Pos.X;
            Pos.Y = -Pos.Y;
        }
        Pos.Z = DetectPitchSurfaceZ(Pos.X, Pos.Y) + 96.0f;
        SpawnPlayer(Side, Roles11[i], Pos, SpawnRot);
    }
}

float AFootballGameMode::DetectPitchSurfaceZ(float X, float Y) const
{
    if (UWorld* World = GetWorld())
    {
        FHitResult Hit;
        FCollisionQueryParams Params;
        Params.bTraceComplex = true;
        if (World->LineTraceSingleByChannel(Hit, FVector(X, Y, 5000.0f), FVector(X, Y, -5000.0f), ECC_WorldStatic,
                                            Params))
        {
            return Hit.ImpactPoint.Z;
        }
    }
    return PitchSurfaceZ;
}

void AFootballGameMode::UpdateSimpleMatchAI(float DeltaTime)
{
    if (!BallComponent)
    {
        return;
    }

    FVector BallLoc = BallComponent->GetComponentLocation();
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    APawn* ControlledPawn = PC ? PC->GetPawn() : nullptr;

    auto TickTeamAI = [BallLoc, ControlledPawn](AFootballTeam* Team, float GoalX)
    {
        if (!Team)
        {
            return;
        }
        AFootballPlayerCharacter* Chaser = Team->GetClosestPlayerToBall();
        for (AFootballPlayerCharacter* Player : Team->GetAllPlayers())
        {
            if (!Player || Player == ControlledPawn)
            {
                continue;
            }

            if (Player->GetPlayerRole() == EFootballPlayerRole::Goalkeeper)
            {
                float TargetY = FMath::Clamp(BallLoc.Y, -600.0f, 600.0f);
                FVector GKTarget(GoalX, TargetY, Player->GetActorLocation().Z);
                FVector Dir = GKTarget - Player->GetActorLocation();
                if (Dir.Size2D() > 60.0f)
                {
                    Player->AddMovementInput(Dir.GetSafeNormal2D(), 0.75f);
                }
            }
            else if (Player == Chaser)
            {
                if (Player->HasBallPossession())
                {
                    FVector OpponentGoal(-GoalX, 0.0f, BallLoc.Z);
                    FVector ToGoal = (OpponentGoal - Player->GetActorLocation()).GetSafeNormal2D();
                    Player->AddMovementInput(ToGoal, 0.85f);
                    if (FVector::Dist2D(Player->GetActorLocation(), OpponentGoal) < 2200.0f)
                    {
                        Player->ShootAtGoal(OpponentGoal, 0.8f, 0.5f, FFootballTags::Action_Shoot_Power);
                    }
                }
                else
                {
                    FVector ToBall = BallLoc - Player->GetActorLocation();
                    if (ToBall.Size2D() > 80.0f)
                    {
                        Player->AddMovementInput(ToBall.GetSafeNormal2D(), 0.85f);
                    }
                }
            }
        }
    };

    TickTeamAI(HomeTeam, -3400.0f);
    TickTeamAI(AwayTeam, 3400.0f);
}

AFootballPlayerCharacter* AFootballGameMode::SpawnPlayer(EFootballTeamSide::Type TeamSide,
                                                         EFootballPlayerRole::Type InRole, FVector Location,
                                                         FRotator Rotation)
{
    if (!PlayerCharacterClass)
    {
        return nullptr;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AFootballPlayerCharacter* Player =
        GetWorld()->SpawnActor<AFootballPlayerCharacter>(PlayerCharacterClass, Location, Rotation, SpawnParams);
    if (Player)
    {
        Player->SetPlayerRole(InRole);
        AFootballTeam* Team = (TeamSide == EFootballTeamSide::Home) ? HomeTeam : AwayTeam;
        if (Team)
        {
            Team->AddPlayer(Player, InRole);
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
    if (MatchTimeElapsed >= HalfSeconds && CurrentPhase != EFootballMatchPhase::HalfTime &&
        CurrentPhase != EFootballMatchPhase::FullTime)
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

    if (HomeTeam)
        HomeTeam->OnHalfTime();
    if (AwayTeam)
        AwayTeam->OnHalfTime();

    OnMatchEvent.Broadcast(FGameplayTag::RequestGameplayTag("Match.HalfTime"), nullptr, nullptr);

    // Auto-resume after delay for second half
    FTimerHandle ResumeTimer;
    GetWorldTimerManager().SetTimer(
        ResumeTimer,
        [this]()
        {
            CurrentPhase = EFootballMatchPhase::KickOff;
            // Away team kicks off second half
            if (HomeTeam)
                HomeTeam->OnKickOff(false);
            if (AwayTeam)
                AwayTeam->OnKickOff(true);
            ResetBall(EFootballMatchPhase::KickOff);
            bIsMatchActive = true;
        },
        15.0f, false);
}

void AFootballGameMode::HandleFullTime()
{
    EndMatch();

    if (HomeTeam)
        HomeTeam->OnFullTime();
    if (AwayTeam)
        AwayTeam->OnFullTime();

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
            AwardThrowIn(LastTouchTeam == EFootballTeamSide::Home ? EFootballTeamSide::Away : EFootballTeamSide::Home,
                         BallLoc);
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
    return FVector(0.0f, 0.0f, PitchSurfaceZ + 22.0f); // Center spot on pitch surface
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
    HomeTeam =
        GetWorld()->SpawnActor<AFootballTeam>(TeamClass, FVector(-5000, 0, 0), FRotator::ZeroRotator, SpawnParams);
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
        if (HomeTeam)
            HomeTeam->SetTeamManager(TeamManager);
        if (AwayTeam)
            AwayTeam->SetTeamManager(TeamManager);
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