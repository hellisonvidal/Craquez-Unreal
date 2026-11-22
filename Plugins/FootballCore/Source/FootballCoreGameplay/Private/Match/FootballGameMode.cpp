// Copyright 2024 Craquez. All Rights Reserved.

#include "Match/FootballGameMode.h"
#include "BallPhysics/FootballBallActor.h"
#include "BallPhysics/FootballBallComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyLight.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "GameFramework/HUD.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Player/FootballPlayerCharacter.h"
#include "Tactical/FootballTacticalComponent.h"
#include "Team/FootballTeam.h"
#include "Team/FootballTeamManager.h"
#include "UObject/ConstructorHelpers.h"

AFootballGameMode::AFootballGameMode()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PrePhysics;

    // DefaultPawnClass = nullptr prevents AGameModeBase from spawning an extra 23rd neutral pawn at PlayerStart
    DefaultPawnClass = nullptr;
    PlayerControllerClass = APlayerController::StaticClass();
    PlayerCharacterClass = AFootballPlayerCharacter::StaticClass();
    TeamClass = AFootballTeam::StaticClass();
    BallComponentClass = UFootballBallComponent::StaticClass();

    // Reference Stadium & Pitch assets directly so the Cooker always packages them and C++ can ensure they exist
    static ConstructorHelpers::FObjectFinder<UStaticMesh> PitchMeshFinder(
        TEXT("/Game/Assets/Pitch/SM_CraquezPitch.SM_CraquezPitch"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> PitchMatFinder(
        TEXT("/Game/Materials/M_CraquezPitch.M_CraquezPitch"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> GrassMatFinder(
        TEXT("/Game/Materials/M_PitchGrassFallback.M_PitchGrassFallback"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> GoalMatFinder(
        TEXT("/Game/Materials/M_Shorts_Away.M_Shorts_Away"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderFinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));

    if (PitchMeshFinder.Succeeded())
        PitchStaticMeshAsset = PitchMeshFinder.Object;
    if (PitchMatFinder.Succeeded())
        PitchMaterialAsset = PitchMatFinder.Object;
    if (GrassMatFinder.Succeeded())
        GrassFallbackMaterialAsset = GrassMatFinder.Object;
    if (GoalMatFinder.Succeeded())
        GoalPostMaterialAsset = GoalMatFinder.Object;
    if (CubeFinder.Succeeded())
        BasicCubeMeshAsset = CubeFinder.Object;
    if (CylinderFinder.Succeeded())
        BasicCylinderMeshAsset = CylinderFinder.Object;
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

    // Ensure full-color lighting, collision floor at Z=0, SM_CraquezPitch (grass aligned at Z=0), and 3D goalposts
    // exist
    EnsureStadiumEnvironment();

    PitchSurfaceZ = 0.0f;
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

void AFootballGameMode::EnsureStadiumEnvironment()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    // 1. Ensure bright directional sunlight
    TArray<AActor*> FoundLights;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundLights);
    if (FoundLights.Num() == 0)
    {
        if (ADirectionalLight* Sun =
                World->SpawnActor<ADirectionalLight>(ADirectionalLight::StaticClass(), FVector(0.0f, 0.0f, 2500.0f),
                                                     FRotator(-48.0f, -35.0f, 0.0f), SpawnParams))
        {
            if (UDirectionalLightComponent* LC = Cast<UDirectionalLightComponent>(Sun->GetLightComponent()))
            {
                LC->SetMobility(EComponentMobility::Movable);
                LC->SetIntensity(8.0f);
                LC->SetLightColor(FLinearColor(1.0f, 0.97f, 0.91f, 1.0f));
                LC->bAtmosphereSunLight = true;
            }
        }
    }

    // 2. Ensure SkyLight so shadows are filled with natural blue sky light (never black)
    TArray<AActor*> FoundSkyLights;
    UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), FoundSkyLights);
    if (FoundSkyLights.Num() == 0)
    {
        if (ASkyLight* Sky = World->SpawnActor<ASkyLight>(ASkyLight::StaticClass(), FVector(0.0f, 0.0f, 2600.0f),
                                                          FRotator::ZeroRotator, SpawnParams))
        {
            if (USkyLightComponent* SLC = Sky->GetLightComponent())
            {
                SLC->SetMobility(EComponentMobility::Movable);
                SLC->SetIntensity(2.0f);
                SLC->bRealTimeCapture = true;
                SLC->RecaptureSky();
            }
        }
    }

    // 3. Ensure SkyAtmosphere
    TArray<AActor*> FoundAtmospheres;
    UGameplayStatics::GetAllActorsOfClass(World, ASkyAtmosphere::StaticClass(), FoundAtmospheres);
    if (FoundAtmospheres.Num() == 0)
    {
        World->SpawnActor<ASkyAtmosphere>(ASkyAtmosphere::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator,
                                          SpawnParams);
    }

    // 4. Ensure ExponentialHeightFog
    TArray<AActor*> FoundFogs;
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundFogs);
    if (FoundFogs.Num() == 0)
    {
        World->SpawnActor<AExponentialHeightFog>(AExponentialHeightFog::StaticClass(), FVector(0.0f, 0.0f, 200.0f),
                                                 FRotator::ZeroRotator, SpawnParams);
    }

    // 5. Always spawn a rock-solid Box-Collision Floor with top surface at World Z = 0.0f (guarantees collision on
    // Android & PC)
    if (BasicCubeMeshAsset)
    {
        if (AStaticMeshActor* FloorActor = World->SpawnActor<AStaticMeshActor>(
                AStaticMeshActor::StaticClass(), FVector(0.0f, 0.0f, -50.0f), FRotator::ZeroRotator, SpawnParams))
        {
            UStaticMeshComponent* SMC = FloorActor->GetStaticMeshComponent();
            SMC->SetMobility(EComponentMobility::Movable);
            SMC->SetStaticMesh(BasicCubeMeshAsset);
            if (GrassFallbackMaterialAsset)
            {
                SMC->SetMaterial(0, GrassFallbackMaterialAsset);
            }
            SMC->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            SMC->SetCollisionObjectType(ECC_WorldStatic);
            SMC->SetCollisionResponseToAllChannels(ECR_Block);
            FloorActor->SetActorScale3D(FVector(250.0f, 250.0f, 1.0f));
        }
    }

    // 6. Ensure SM_CraquezPitch is present and aligned so its grass surface sits at World Z = 0.0f with uniform
    // scale 60.0f
    if (PitchStaticMeshAsset)
    {
        TArray<AActor*> StaticMeshes;
        UGameplayStatics::GetAllActorsOfClass(World, AStaticMeshActor::StaticClass(), StaticMeshes);
        AStaticMeshActor* ExistingPitch = nullptr;
        for (AActor* Act : StaticMeshes)
        {
            if (AStaticMeshActor* SMA = Cast<AStaticMeshActor>(Act))
            {
                if (SMA->GetStaticMeshComponent() &&
                    SMA->GetStaticMeshComponent()->GetStaticMesh() == PitchStaticMeshAsset)
                {
                    ExistingPitch = SMA;
                    break;
                }
            }
        }

        // Unscaled SM_CraquezPitch flat grass surface is at Z_UE = -17.02.
        // At uniform scale 60.0f, placing actor at Z = +17.02 * 60.0 = +1021.2f puts grass surface at World Z = 0.0f!
        const FVector PitchLoc(9.0f, -22.0f, 1021.5f);
        const FVector PitchScale(60.0f, 60.0f, 60.0f);

        if (ExistingPitch)
        {
            ExistingPitch->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
            ExistingPitch->SetActorLocationAndRotation(PitchLoc, FRotator::ZeroRotator);
            ExistingPitch->SetActorScale3D(PitchScale);
            if (PitchMaterialAsset)
            {
                ExistingPitch->GetStaticMeshComponent()->SetMaterial(0, PitchMaterialAsset);
            }
        }
        else if (AStaticMeshActor* NewPitch = World->SpawnActor<AStaticMeshActor>(
                     AStaticMeshActor::StaticClass(), PitchLoc, FRotator::ZeroRotator, SpawnParams))
        {
            UStaticMeshComponent* SMC = NewPitch->GetStaticMeshComponent();
            SMC->SetMobility(EComponentMobility::Movable);
            SMC->SetStaticMesh(PitchStaticMeshAsset);
            if (PitchMaterialAsset)
            {
                SMC->SetMaterial(0, PitchMaterialAsset);
            }
            SMC->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            NewPitch->SetActorScale3D(PitchScale);
        }
    }

    // 7. Spawn 3D White Goalposts at both ends of the field (X = -3800 and X = +3800, Y = +/-366, Z = 0..244)
    if (BasicCylinderMeshAsset)
    {
        auto SpawnPostPiece = [&](const FVector& Loc, const FRotator& Rot, const FVector& Scale)
        {
            if (AStaticMeshActor* Piece =
                    World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Loc, Rot, SpawnParams))
            {
                UStaticMeshComponent* SMC = Piece->GetStaticMeshComponent();
                SMC->SetMobility(EComponentMobility::Movable);
                SMC->SetStaticMesh(BasicCylinderMeshAsset);
                if (GoalPostMaterialAsset)
                {
                    SMC->SetMaterial(0, GoalPostMaterialAsset);
                }
                SMC->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                Piece->SetActorScale3D(Scale);
            }
        };

        for (float Sign : {-1.0f, 1.0f})
        {
            const float GX = Sign * 3800.0f;
            const float BackX = Sign * 3940.0f;
            // Left & Right front goalposts
            SpawnPostPiece(FVector(GX, -366.0f, 122.0f), FRotator::ZeroRotator, FVector(0.16f, 0.16f, 2.44f));
            SpawnPostPiece(FVector(GX, 366.0f, 122.0f), FRotator::ZeroRotator, FVector(0.16f, 0.16f, 2.44f));
            // Front crossbar
            SpawnPostPiece(FVector(GX, 0.0f, 244.0f), FRotator(0.0f, 0.0f, 90.0f), FVector(0.16f, 0.16f, 7.40f));
            // Back goal frame bars
            SpawnPostPiece(FVector(BackX, -366.0f, 122.0f), FRotator::ZeroRotator, FVector(0.10f, 0.10f, 2.44f));
            SpawnPostPiece(FVector(BackX, 366.0f, 122.0f), FRotator::ZeroRotator, FVector(0.10f, 0.10f, 2.44f));
            SpawnPostPiece(FVector(BackX, 0.0f, 244.0f), FRotator(0.0f, 0.0f, 90.0f), FVector(0.10f, 0.10f, 7.40f));
        }
    }
}

void AFootballGameMode::SpawnPlayers()
{
    if (!PlayerCharacterClass || !TeamClass)
    {
        return;
    }

    // Release ball possession before destroying old players
    if (BallComponent)
    {
        BallComponent->SetPossessingPlayer(nullptr);
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
                P->ReleasePossession();
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
        AFootballPlayerCharacter* Striker = nullptr;
        if (HomeTeam)
        {
            Striker = SpawnPlayer(EFootballTeamSide::Home, EFootballPlayerRole::Striker,
                                  FVector(-220.0f, 0.0f, PitchSurfaceZ + 92.0f), FRotator(0.0f, 0.0f, 0.0f));
        }
        if (AwayTeam)
        {
            SpawnPlayer(EFootballTeamSide::Away, EFootballPlayerRole::Goalkeeper,
                        FVector(3600.0f, 0.0f, PitchSurfaceZ + 92.0f), FRotator(0.0f, 180.0f, 0.0f));
        }
        if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
        {
            if (Striker)
            {
                PC->Possess(Striker);
            }
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

    // Always possess the Home Striker so the action camera and player controls are active immediately
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
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

    // Balanced 4-3-3 layout fitting inside the 7600x5400 cm playable pitch (goals at X = +/-3800)
    static const FVector HomeOffsets[11] = {
        FVector(-3620.0f, 0.0f, 0.0f),     // GK
        FVector(-2300.0f, -1550.0f, 0.0f), // LB
        FVector(-2450.0f, -550.0f, 0.0f),  // CB1
        FVector(-2450.0f, 550.0f, 0.0f),   // CB2
        FVector(-2300.0f, 1550.0f, 0.0f),  // RB
        FVector(-1400.0f, 0.0f, 0.0f),     // CDM
        FVector(-950.0f, -950.0f, 0.0f),   // CM
        FVector(-950.0f, 950.0f, 0.0f),    // CAM
        FVector(-380.0f, -1450.0f, 0.0f),  // LW
        FVector(-140.0f, 0.0f, 0.0f),      // ST (right next to center kickoff ball!)
        FVector(-380.0f, 1450.0f, 0.0f),   // RW
    };

    FRotator SpawnRot = (Side == EFootballTeamSide::Home) ? FRotator(0.0f, 0.0f, 0.0f) : FRotator(0.0f, 180.0f, 0.0f);

    for (int32 i = 0; i < 11; ++i)
    {
        FVector Pos = HomeOffsets[i];
        if (Side == EFootballTeamSide::Away)
        {
            Pos.X = -Pos.X;
            Pos.Y = -Pos.Y;
            if (i == 9) // Push Away striker slightly back on kickoff so Home striker gets first touch
            {
                Pos.X = 520.0f;
            }
        }
        Pos.Z = PitchSurfaceZ + 92.0f;
        SpawnPlayer(Side, Roles11[i], Pos, SpawnRot);
    }
}

float AFootballGameMode::DetectPitchSurfaceZ(float X, float Y) const
{
    return PitchSurfaceZ;
}

void AFootballGameMode::UpdateSimpleMatchAI(float DeltaTime)
{
    if (!BallComponent)
    {
        return;
    }

    const FVector BallLoc = BallComponent->GetComponentLocation();
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    APawn* ControlledPawn = PC ? PC->GetPawn() : nullptr;

    static const FVector BaseOffsets[11] = {
        FVector(-3620.0f, 0.0f, 0.0f),   FVector(-2300.0f, -1550.0f, 0.0f), FVector(-2450.0f, -550.0f, 0.0f),
        FVector(-2450.0f, 550.0f, 0.0f), FVector(-2300.0f, 1550.0f, 0.0f),  FVector(-1400.0f, 0.0f, 0.0f),
        FVector(-950.0f, -950.0f, 0.0f), FVector(-950.0f, 950.0f, 0.0f),    FVector(-380.0f, -1450.0f, 0.0f),
        FVector(-140.0f, 0.0f, 0.0f),    FVector(-380.0f, 1450.0f, 0.0f),
    };

    auto TickTeamAI = [&](AFootballTeam* Team, float OwnGoalX, EFootballTeamSide::Type Side)
    {
        if (!Team)
        {
            return;
        }

        // Find closest non-human-controlled outfield player to the ball
        AFootballPlayerCharacter* Chaser = nullptr;
        float BestDist = FLT_MAX;
        const TArray<AFootballPlayerCharacter*>& TeamPlayers = Team->GetAllPlayers();
        for (AFootballPlayerCharacter* P : TeamPlayers)
        {
            if (!P || P == ControlledPawn || P->GetPlayerRole() == EFootballPlayerRole::Goalkeeper)
            {
                continue;
            }
            const float D = FVector::Dist2D(P->GetActorLocation(), BallLoc);
            if (D < BestDist)
            {
                BestDist = D;
                Chaser = P;
            }
        }

        const float AttackDir = (Side == EFootballTeamSide::Home) ? 1.0f : -1.0f;
        const float BallShiftX = FMath::Clamp(BallLoc.X * 0.42f, -1100.0f, 1100.0f);

        for (int32 Idx = 0; Idx < TeamPlayers.Num(); ++Idx)
        {
            AFootballPlayerCharacter* Player = TeamPlayers[Idx];
            if (!Player || Player == ControlledPawn)
            {
                continue;
            }

            if (Player->GetPlayerRole() == EFootballPlayerRole::Goalkeeper)
            {
                if (Player->HasBallPossession())
                {
                    const FVector ClearTarget(-OwnGoalX * 0.5f, FMath::RandRange(-600.0f, 600.0f), 120.0f);
                    Player->ShootAtGoal(ClearTarget, 0.85f, 0.0f, FFootballTags::Action_Shoot_Power);
                }
                else
                {
                    const float TargetY = FMath::Clamp(BallLoc.Y * 0.65f, -310.0f, 310.0f);
                    const FVector GKTarget(OwnGoalX, TargetY, Player->GetActorLocation().Z);
                    const FVector Dir = GKTarget - Player->GetActorLocation();
                    if (Dir.Size2D() > 45.0f)
                    {
                        Player->AddMovementInput(Dir.GetSafeNormal2D(), 0.80f);
                    }
                    if (FVector::Dist2D(Player->GetActorLocation(), BallLoc) < 260.0f)
                    {
                        Player->TriggerTackle();
                    }
                }
            }
            else if (Player->HasBallPossession())
            {
                const FVector OpponentGoal(-OwnGoalX,
                                           FMath::Clamp(Player->GetActorLocation().Y * 0.3f, -220.0f, 220.0f), 110.0f);
                const FVector ToGoal = (OpponentGoal - Player->GetActorLocation()).GetSafeNormal2D();
                Player->AddMovementInput(ToGoal, 0.90f);
                if (FVector::Dist2D(Player->GetActorLocation(), OpponentGoal) < 1850.0f)
                {
                    Player->ShootAtGoal(OpponentGoal, 0.92f, 0.1f, FFootballTags::Action_Shoot_Power);
                }
            }
            else if (Player == Chaser)
            {
                const FVector ToBall = BallLoc - Player->GetActorLocation();
                const float Dist2D = ToBall.Size2D();
                if (Dist2D > 45.0f)
                {
                    Player->AddMovementInput(ToBall.GetSafeNormal2D(), 0.88f);
                }
                if (Dist2D < 190.0f && BallComponent->GetBallState() == EFootballBallState::Possessed)
                {
                    Player->TriggerTackle();
                }
            }
            else if (Idx < 11)
            {
                // Smooth dynamic formation movement for all supporting outfield players
                FVector FormTarget = BaseOffsets[Idx];
                if (Side == EFootballTeamSide::Away)
                {
                    FormTarget.X = -FormTarget.X;
                    FormTarget.Y = -FormTarget.Y;
                }
                FormTarget.X = FMath::Clamp(FormTarget.X + BallShiftX + AttackDir * 180.0f, -3300.0f, 3300.0f);
                FormTarget.Y = FMath::Clamp(FormTarget.Y + BallLoc.Y * 0.22f, -2400.0f, 2400.0f);

                const FVector ToForm = FormTarget - Player->GetActorLocation();
                if (ToForm.Size2D() > 140.0f)
                {
                    Player->AddMovementInput(ToForm.GetSafeNormal2D(), 0.65f);
                }
            }
        }
    };

    TickTeamAI(HomeTeam, -3620.0f, EFootballTeamSide::Home);
    TickTeamAI(AwayTeam, 3620.0f, EFootballTeamSide::Away);
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

    const FVector BallLoc = BallComponent->GetComponentLocation();
    if (FMath::Abs(BallLoc.Y) < 370.0f && BallLoc.Z < 255.0f)
    {
        if (BallLoc.X > 3780.0f)
        {
            RegisterGoal(EFootballTeamSide::Home, LastTouchPlayer);
            SpawnPlayers();
            ResetBall(EFootballMatchPhase::KickOff);
        }
        else if (BallLoc.X < -3780.0f)
        {
            RegisterGoal(EFootballTeamSide::Away, LastTouchPlayer);
            SpawnPlayers();
            ResetBall(EFootballMatchPhase::KickOff);
        }
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