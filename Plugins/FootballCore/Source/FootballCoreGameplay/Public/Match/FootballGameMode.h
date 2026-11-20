// Copyright 2024 Craquez. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FootballCoreTypes.h"
#include "FootballGameMode.generated.h"

class AFootballTeam;
class AFootballTeamManager;
class UFootballBallComponent;
class AFootballPlayerCharacter;
class UFootballMatchState;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnMatchEvent, FGameplayTag, EventTag, AActor*, Instigator, AActor*,
                                               Target);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGoalScored, EFootballTeamSide::Type, ScoringTeam,
                                             AFootballPlayerCharacter*, Scorer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchPhaseChanged, EFootballMatchPhase::Type, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchTimeChanged, float, TimeRemaining);

/**
 * Football Game Mode - Main match controller handling:
 * - Match phases (kickoff, open play, set pieces, half-time)
 * - Score and time management
 * - Team management
 * - Ball spawning and reset
 * - Rules enforcement (offsides, fouls, etc.)
 * - Match state replication
 */
UCLASS(Blueprintable, BlueprintType)
class FOOTBALLCOREGAMEPLAY_API AFootballGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AFootballGameMode();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // Match Control
    UFUNCTION(BlueprintCallable, Category = "Football|Match")
    void StartMatch();

    UFUNCTION(BlueprintCallable, Category = "Football|Match")
    void PauseMatch();

    UFUNCTION(BlueprintCallable, Category = "Football|Match")
    void ResumeMatch();

    UFUNCTION(BlueprintCallable, Category = "Football|Match")
    void EndMatch();

    UFUNCTION(BlueprintCallable, Category = "Football|Match")
    void RestartMatch();

    // Team Management
    UFUNCTION(BlueprintCallable, Category = "Football|Team")
    void SetHomeTeam(AFootballTeam* Team);

    UFUNCTION(BlueprintCallable, Category = "Football|Team")
    void SetAwayTeam(AFootballTeam* Team);

    UFUNCTION(BlueprintCallable, Category = "Football|Team")
    AFootballTeam* GetHomeTeam() const { return HomeTeam; }

    UFUNCTION(BlueprintCallable, Category = "Football|Team")
    AFootballTeam* GetAwayTeam() const { return AwayTeam; }

    // Ball
    UFUNCTION(BlueprintCallable, Category = "Football|Ball")
    UFootballBallComponent* GetBall() const;

    UFUNCTION(BlueprintCallable, Category = "Football|Ball")
    void SpawnBall(FVector Location = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Football|Ball")
    void ResetBall(EFootballMatchPhase::Type RestartType);

    // Scoring
    UFUNCTION(BlueprintCallable, Category = "Football|Scoring")
    void RegisterGoal(EFootballTeamSide::Type ScoringTeam, AFootballPlayerCharacter* Scorer);

    UFUNCTION(BlueprintPure, Category = "Football|Scoring")
    int32 GetHomeScore() const { return HomeScore; }

    UFUNCTION(BlueprintPure, Category = "Football|Scoring")
    int32 GetAwayScore() const { return AwayScore; }

    // Match Time
    UFUNCTION(BlueprintPure, Category = "Football|Time")
    float GetMatchTimeRemaining() const;

    UFUNCTION(BlueprintPure, Category = "Football|Time")
    float GetMatchTimeElapsed() const;

    UFUNCTION(BlueprintPure, Category = "Football|Time")
    int32 GetCurrentMinute() const;

    UFUNCTION(BlueprintPure, Category = "Football|Time")
    bool IsHalfTime() const { return CurrentPhase == EFootballMatchPhase::HalfTime; }

    UFUNCTION(BlueprintPure, Category = "Football|Time")
    bool IsFullTime() const { return CurrentPhase == EFootballMatchPhase::FullTime; }

    // Match Phase
    UFUNCTION(BlueprintCallable, Category = "Football|Match")
    void SetMatchPhase(EFootballMatchPhase::Type NewPhase);

    UFUNCTION(BlueprintPure, Category = "Football|Match")
    EFootballMatchPhase::Type GetCurrentPhase() const { return CurrentPhase; }

    // Rules
    UFUNCTION(BlueprintCallable, Category = "Football|Rules")
    void CheckOffside(AFootballPlayerCharacter* Attacker);

    UFUNCTION(BlueprintCallable, Category = "Football|Rules")
    void RegisterFoul(EFootballTeamSide::Type OffendingTeam, AFootballPlayerCharacter* Offender, FVector Location,
                      FGameplayTag FoulType);

    UFUNCTION(BlueprintCallable, Category = "Football|Rules")
    void AwardFreeKick(EFootballTeamSide::Type Team, FVector Location, FGameplayTag FoulType);

    UFUNCTION(BlueprintCallable, Category = "Football|Rules")
    void AwardPenalty(EFootballTeamSide::Type Team, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Football|Rules")
    void AwardCorner(EFootballTeamSide::Type Team);

    UFUNCTION(BlueprintCallable, Category = "Football|Rules")
    void AwardGoalKick(EFootballTeamSide::Type Team);

    UFUNCTION(BlueprintCallable, Category = "Football|Rules")
    void AwardThrowIn(EFootballTeamSide::Type Team, FVector Location);

    // Substitutions
    UFUNCTION(BlueprintCallable, Category = "Football|Team")
    bool ProcessSubstitution(EFootballTeamSide::Type Team, AFootballPlayerCharacter* PlayerOut,
                             AFootballPlayerCharacter* PlayerIn);

    // Game Mode Type
    UFUNCTION(BlueprintCallable, Category = "Football|Match")
    void SetGameMode(EFootballGameMode::Type Mode);

    UFUNCTION(BlueprintPure, Category = "Football|Match")
    EFootballGameMode::Type GetGameMode() const { return GameModeType; }

    // Player Spawning
    UFUNCTION(BlueprintCallable, Category = "Football|Players")
    void SpawnPlayers();

    UFUNCTION(BlueprintCallable, Category = "Football|Players")
    AFootballPlayerCharacter* SpawnPlayer(EFootballTeamSide::Type Team, EFootballPlayerRole::Type InRole,
                                          FVector Location, FRotator Rotation);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Football|Events")
    FOnMatchEvent OnMatchEvent;

    UPROPERTY(BlueprintAssignable, Category = "Football|Events")
    FOnGoalScored OnGoalScored;

    UPROPERTY(BlueprintAssignable, Category = "Football|Events")
    FOnMatchPhaseChanged OnMatchPhaseChanged;

    UPROPERTY(BlueprintAssignable, Category = "Football|Events")
    FOnMatchTimeChanged OnMatchTimeChanged;

protected:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Match", meta = (AllowPrivateAccess = "true"))
    TEnumAsByte<EFootballGameMode::Type> GameModeType = EFootballGameMode::TrainingMatch_11v11;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Match", meta = (AllowPrivateAccess = "true"))
    float MatchDuration = 90.0f; // minutes

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Match", meta = (AllowPrivateAccess = "true"))
    float HalfDuration = 45.0f; // minutes

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Match", meta = (AllowPrivateAccess = "true"))
    bool bAllowSubstitutions = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Match", meta = (AllowPrivateAccess = "true"))
    int32 MaxSubstitutions = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Match", meta = (AllowPrivateAccess = "true"))
    TSubclassOf<UFootballBallComponent> BallComponentClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Match", meta = (AllowPrivateAccess = "true"))
    TSubclassOf<AFootballPlayerCharacter> PlayerCharacterClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Match", meta = (AllowPrivateAccess = "true"))
    TSubclassOf<AFootballTeam> TeamClass;

    // Teams
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Team", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<AFootballTeam> HomeTeam = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Team", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<AFootballTeam> AwayTeam = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Team", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<AFootballTeamManager> TeamManager = nullptr;

    // Ball
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Ball", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UFootballBallComponent> BallComponent = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Ball", meta = (AllowPrivateAccess = "true"))
    FVector BallSpawnLocation = FVector::ZeroVector;

    // Match State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Match", meta = (AllowPrivateAccess = "true"))
    TEnumAsByte<EFootballMatchPhase::Type> CurrentPhase = EFootballMatchPhase::KickOff;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Match", meta = (AllowPrivateAccess = "true"))
    float MatchTimeElapsed = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Match", meta = (AllowPrivateAccess = "true"))
    bool bIsMatchActive = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Match", meta = (AllowPrivateAccess = "true"))
    bool bIsPaused = false;

    // Score
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Scoring", meta = (AllowPrivateAccess = "true"))
    int32 HomeScore = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Scoring", meta = (AllowPrivateAccess = "true"))
    int32 AwayScore = 0;

    // Stats
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Match", meta = (AllowPrivateAccess = "true"))
    FFootballMatchStats MatchStats;

    // Set Piece State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Rules", meta = (AllowPrivateAccess = "true"))
    TEnumAsByte<EFootballTeamSide::Type> LastTouchTeam = EFootballTeamSide::Neutral;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Rules", meta = (AllowPrivateAccess = "true"))
    AFootballPlayerCharacter* LastTouchPlayer = nullptr;

    // Substitutions
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Team", meta = (AllowPrivateAccess = "true"))
    int32 HomeSubsUsed = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Team", meta = (AllowPrivateAccess = "true"))
    int32 AwaySubsUsed = 0;

private:
    void UpdateMatchTime(float DeltaTime);
    void HandlePhaseTransitions();
    void HandleKickOff();
    void HandleHalfTime();
    void HandleFullTime();
    void ProcessBallOutOfPlay();
    void UpdateBallOwnership();
    void CheckGoalScored();
    FVector GetKickOffPosition() const;
    FVector GetGoalKickPosition(EFootballTeamSide::Type Team) const;
    FVector GetCornerPosition(EFootballTeamSide::Type Team) const;
    void InitializeTeams();
    void SetupDefaultTactics();
    void SpawnTeamPlayers(AFootballTeam* Team, EFootballTeamSide::Type Side);
};