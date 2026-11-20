// Copyright 2024 Craquez. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FootballCoreTypes.h"
#include "FootballTeamManager.generated.h"

class AFootballTeam;
class AFootballPlayerCharacter;
class UFootballBallComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPossessionChanged, EFootballTeamSide::Type, NewPossessionTeam,
                                             AFootballPlayerCharacter*, Player);

/**
 * Football Team Manager - Central coordinator for match state:
 * - Ball possession tracking
 * - Team coordination
 * - Match statistics
 * - Referee decisions
 * - Deterministic state for networking
 */
UCLASS(Blueprintable, BlueprintType)
class FOOTBALLCOREGAMEPLAY_API AFootballTeamManager : public AActor
{
    GENERATED_BODY()

public:
    AFootballTeamManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Team Setup
    UFUNCTION(BlueprintCallable, Category = "Football|TeamManager")
    void SetTeams(AFootballTeam* Home, AFootballTeam* Away);

    UFUNCTION(BlueprintCallable, Category = "Football|TeamManager")
    void SetBall(UFootballBallComponent* InBall);

    // Ball & Possession
    UFUNCTION(BlueprintPure, Category = "Football|TeamManager")
    UFootballBallComponent* GetBall() const { return Ball; }

    UFUNCTION(BlueprintPure, Category = "Football|TeamManager")
    EFootballTeamSide::Type GetPossessionTeam() const { return PossessionTeam; }

    UFUNCTION(BlueprintPure, Category = "Football|TeamManager")
    AFootballPlayerCharacter* GetPossessionPlayer() const { return PossessionPlayer; }

    UFUNCTION(BlueprintCallable, Category = "Football|TeamManager")
    void SetPossession(EFootballTeamSide::Type Team, AFootballPlayerCharacter* Player);

    UFUNCTION(BlueprintCallable, Category = "Football|TeamManager")
    void ClearPossession();

    // Teams
    UFUNCTION(BlueprintPure, Category = "Football|TeamManager")
    AFootballTeam* GetHomeTeam() const { return HomeTeam; }

    UFUNCTION(BlueprintPure, Category = "Football|TeamManager")
    AFootballTeam* GetAwayTeam() const { return AwayTeam; }

    UFUNCTION(BlueprintPure, Category = "Football|TeamManager")
    AFootballTeam* GetTeam(EFootballTeamSide::Type Side) const;

    // Match Statistics
    UFUNCTION(BlueprintCallable, Category = "Football|Statistics")
    void RecordShot(EFootballTeamSide::Type Team, bool bOnTarget);

    UFUNCTION(BlueprintCallable, Category = "Football|Statistics")
    void RecordPass(EFootballTeamSide::Type Team, bool bCompleted);

    UFUNCTION(BlueprintCallable, Category = "Football|Statistics")
    void RecordTackle(EFootballTeamSide::Type Team, bool bSuccessful);

    UFUNCTION(BlueprintCallable, Category = "Football|Statistics")
    void RecordInterception(EFootballTeamSide::Type Team);

    UFUNCTION(BlueprintCallable, Category = "Football|Statistics")
    void RecordSave(EFootballTeamSide::Type Team);

    UFUNCTION(BlueprintCallable, Category = "Football|Statistics")
    void RecordFoul(EFootballTeamSide::Type Team);

    UFUNCTION(BlueprintCallable, Category = "Football|Statistics")
    void RecordCard(EFootballTeamSide::Type Team, bool bYellow);

    UFUNCTION(BlueprintPure, Category = "Football|Statistics")
    FFootballMatchStats GetMatchStats() const { return MatchStats; }

    UFUNCTION(BlueprintPure, Category = "Football|Statistics")
    float GetPossessionPercent(EFootballTeamSide::Type Team) const;

    // Referee
    UFUNCTION(BlueprintCallable, Category = "Football|Referee")
    void CheckAdvantage(EFootballTeamSide::Type FouledTeam, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Football|Referee")
    void PlayAdvantage(float Duration);

    UFUNCTION(BlueprintCallable, Category = "Football|Referee")
    void CallFoul(EFootballTeamSide::Type OffendingTeam, FVector Location, FGameplayTag FoulType);

    // Deterministic State
    UFUNCTION(BlueprintCallable, Category = "Football|Network")
    void CaptureDeterministicState();

    UFUNCTION(BlueprintCallable, Category = "Football|Network")
    void RestoreDeterministicState();

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Football|Events")
    FOnPossessionChanged OnPossessionChanged;

protected:
    // Teams
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|TeamManager",
              meta = (AllowPrivateAccess = "true"))
    TObjectPtr<AFootballTeam> HomeTeam = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|TeamManager",
              meta = (AllowPrivateAccess = "true"))
    TObjectPtr<AFootballTeam> AwayTeam = nullptr;

    // Ball
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|TeamManager",
              meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UFootballBallComponent> Ball = nullptr;

    // Possession
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|TeamManager",
              meta = (AllowPrivateAccess = "true"))
    TEnumAsByte<EFootballTeamSide::Type> PossessionTeam = EFootballTeamSide::Neutral;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|TeamManager",
              meta = (AllowPrivateAccess = "true"))
    TObjectPtr<AFootballPlayerCharacter> PossessionPlayer = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|TeamManager",
              meta = (AllowPrivateAccess = "true"))
    float PossessionStartTime = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|TeamManager",
              meta = (AllowPrivateAccess = "true"))
    TMap<TEnumAsByte<EFootballTeamSide::Type>, float> TeamPossessionTime;

    // Match Stats
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Statistics",
              meta = (AllowPrivateAccess = "true"))
    FFootballMatchStats MatchStats;

    // Referee
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Referee", meta = (AllowPrivateAccess = "true"))
    bool bAdvantagePlaying = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Referee", meta = (AllowPrivateAccess = "true"))
    TEnumAsByte<EFootballTeamSide::Type> AdvantageTeam = EFootballTeamSide::Neutral;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Referee", meta = (AllowPrivateAccess = "true"))
    float AdvantageEndTime = 0.0f;

    // Deterministic State History
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Network", meta = (AllowPrivateAccess = "true"))
    TArray<FFootballFixedVector> BallPositionHistory;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Network", meta = (AllowPrivateAccess = "true"))
    TArray<FFootballFixedVector> BallVelocityHistory;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Network", meta = (AllowPrivateAccess = "true"))
    int32 HistoryFrame = 0;

private:
    UFUNCTION()
    void OnBallKicked(const FVector& Impulse, const FVector& Spin, AActor* KickInstigator);

    UFUNCTION()
    void OnBallCollision(const FHitResult& HitResult, AActor* OtherActor);

    UFUNCTION()
    void OnBallStateChanged(EFootballBallState::Type NewState);

    void UpdatePossession(float DeltaTime);
    void UpdateBallTracking(float DeltaTime);
    void UpdateAdvantage(float DeltaTime);
    EFootballTeamSide::Type DeterminePossessionTeam() const;
};