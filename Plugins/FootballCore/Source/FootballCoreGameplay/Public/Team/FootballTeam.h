// Copyright 2024 Craquez. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "FootballCoreTypes.h"
#include "FootballTeam.generated.h"

class AFootballPlayerCharacter;
class AFootballTeamManager;
class UFootballTacticalComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTeamPossessionChanged, EFootballTeamSide::Type, PossessingTeam);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTeamGoalScored, EFootballTeamSide::Type, ScoringTeam, AFootballPlayerCharacter*, Scorer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTeamTacticChanged, FGameplayTag, TacticTag);

/**
 * Football Team - Manages a team of 11 players, tactics, and team state
 */
UCLASS(Blueprintable, BlueprintType)
class FOOTBALLCOREGAMEPLAY_API AFootballTeam : public AActor
{
    GENERATED_BODY()

public:
    AFootballTeam();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Team Setup
    UFUNCTION(BlueprintCallable, Category = "Football|Team")
    void InitializeTeam(EFootballTeamSide::Type InSide, const FString& TeamName, const FFootballTacticalInstructions& InTactics);

    UFUNCTION(BlueprintCallable, Category = "Football|Team")
    void SetTeamManager(AFootballTeamManager* InManager);

    UFUNCTION(BlueprintCallable, Category = "Football|Team")
    void AddPlayer(AFootballPlayerCharacter* Player, EFootballPlayerRole::Type Role);

    UFUNCTION(BlueprintCallable, Category = "Football|Team")
    void RemovePlayer(AFootballPlayerCharacter* Player);

    UFUNCTION(BlueprintCallable, Category = "Football|Team")
    void SetFormation(int32 FormationID);

    UFUNCTION(BlueprintCallable, Category = "Football|Team")
    void SetTactics(const FFootballTacticalInstructions& NewTactics);

    UFUNCTION(BlueprintCallable, Category = "Football|Team")
    void ApplyTactic(FGameplayTag TacticTag, float Value);

    // Player Management
    UFUNCTION(BlueprintCallable, Category = "Football|Team")
    AFootballPlayerCharacter* GetPlayerByRole(EFootballPlayerRole::Type Role) const;

    UFUNCTION(BlueprintCallable, Category = "Football|Team")
    TArray<AFootballPlayerCharacter*> GetPlayersByRole(EFootballPlayerRole::Type Role) const;

    UFUNCTION(BlueprintCallable, Category = "Football|Team")
    AFootballPlayerCharacter* GetClosestPlayerToBall() const;

    UFUNCTION(BlueprintCallable, Category = "Football|Team")
    AFootballPlayerCharacter* GetPlayerWithBall() const;

    UFUNCTION(BlueprintCallable, Category = "Football|Team")
    TArray<AFootballPlayerCharacter*> GetAllPlayers() const { return Players; }

    UFUNCTION(BlueprintCallable, Category = "Football|Team")
    int32 GetPlayerCount() const { return Players.Num(); }

    // Tactical
    UFUNCTION(BlueprintCallable, Category = "Football|Tactical")
    void SetTeamInstruction(FGameplayTag Instruction, float Value);

    UFUNCTION(BlueprintPure, Category = "Football|Tactical")
    float GetTeamInstruction(FGameplayTag Instruction) const;

    UFUNCTION(BlueprintCallable, Category = "Football|Tactical")
    void SetDefensiveLine(float Value); // 0-100

    UFUNCTION(BlueprintCallable, Category = "Football|Tactical")
    void SetEngagementLine(float Value); // 0-100

    UFUNCTION(BlueprintCallable, Category = "Football|Tactical")
    void SetPressingIntensity(float Value); // 0-100

    UFUNCTION(BlueprintCallable, Category = "Football|Tactical")
    void SetOffsideTrap(bool bEnabled);

    // Match State
    UFUNCTION(BlueprintCallable, Category = "Football|Match")
    void OnKickOff(bool bWeHaveBall);

    UFUNCTION(BlueprintCallable, Category = "Football|Match")
    void OnPossessionGained();

    UFUNCTION(BlueprintCallable, Category = "Football|Match")
    void OnPossessionLost();

    UFUNCTION(BlueprintCallable, Category = "Football|Match")
    void OnGoalScored(AFootballPlayerCharacter* Scorer);

    UFUNCTION(BlueprintCallable, Category = "Football|Match")
    void OnGoalConceded();

    UFUNCTION(BlueprintCallable, Category = "Football|Match")
    void OnSetPiece(EFootballMatchPhase::Type SetPieceType, bool bAttacking);

    UFUNCTION(BlueprintCallable, Category = "Football|Match")
    void OnHalfTime();

    UFUNCTION(BlueprintCallable, Category = "Football|Match")
    void OnFullTime();

    // Substitutions
    UFUNCTION(BlueprintCallable, Category = "Football|Team")
    bool RequestSubstitution(AFootballPlayerCharacter* PlayerOut, AFootballPlayerCharacter* PlayerIn);

    // Getters
    UFUNCTION(BlueprintPure, Category = "Football|Team")
    EFootballTeamSide::Type GetTeamSide() const { return TeamSide; }

    UFUNCTION(BlueprintPure, Category = "Football|Team")
    FString GetTeamName() const { return TeamName; }

    UFUNCTION(BlueprintPure, Category = "Football|Team")
    FFootballTacticalInstructions GetTactics() const { return Tactics; }

    UFUNCTION(BlueprintPure, Category = "Football|Team")
    bool HasPossession() const { return bHasPossession; }

    UFUNCTION(BlueprintPure, Category = "Football|Team")
    int32 GetGoals() const { return GoalsScored; }

    UFUNCTION(BlueprintPure, Category = "Football|Team")
    int32 GetShots() const { return Shots; }

    UFUNCTION(BlueprintPure, Category = "Football|Team")
    int32 GetShotsOnTarget() const { return ShotsOnTarget; }

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Football|Events")
    FOnTeamPossessionChanged OnPossessionChanged;

    UPROPERTY(BlueprintAssignable, Category = "Football|Events")
    FOnTeamGoalScored OnGoalScored;

    UPROPERTY(BlueprintAssignable, Category = "Football|Events")
    FOnTeamTacticChanged OnTacticChanged;

protected:
    // Team Identity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Team", meta = (AllowPrivateAccess = "true"))
    EFootballTeamSide::Type TeamSide = EFootballTeamSide::Home;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Team", meta = (AllowPrivateAccess = "true"))
    FString TeamName = TEXT("Team");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Team", meta = (AllowPrivateAccess = "true"))
    FColor TeamColor = FColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Team", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UTexture2D> TeamCrest = nullptr;

    // Players
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Team", meta = (AllowPrivateAccess = "true"))
    TArray<TObjectPtr<AFootballPlayerCharacter>> Players;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Team", meta = (AllowPrivateAccess = "true"))
    TMap<EFootballPlayerRole::Type, TObjectPtr<AFootballPlayerCharacter>> PlayerByRole;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Team", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<AFootballPlayerCharacter> Captain = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Team", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<AFootballPlayerCharacter> Goalkeeper = nullptr;

    // Tactics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Tactical", meta = (AllowPrivateAccess = "true"))
    FFootballTacticalInstructions Tactics;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Tactical", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UFootballTacticalComponent> TacticalComponent;

    // Match State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Match", meta = (AllowPrivateAccess = "true"))
    bool bHasPossession = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Match", meta = (AllowPrivateAccess = "true"))
    float PossessionTime = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Match", meta = (AllowPrivateAccess = "true"))
    int32 GoalsScored = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Match", meta = (AllowPrivateAccess = "true"))
    int32 GoalsConceded = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Match", meta = (AllowPrivateAccess = "true"))
    int32 Shots = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Match", meta = (AllowPrivateAccess = "true"))
    int32 ShotsOnTarget = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Match", meta = (AllowPrivateAccess = "true"))
    int32 PassesCompleted = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Match", meta = (AllowPrivateAccess = "true"))
    int32 PassesAttempted = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Match", meta = (AllowPrivateAccess = "true"))
    int32 FoulsCommitted = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Match", meta = (AllowPrivateAccess = "true"))
    int32 YellowCards = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Match", meta = (AllowPrivateAccess = "true"))
    int32 RedCards = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Match", meta = (AllowPrivateAccess = "true"))
    int32 Corners = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Match", meta = (AllowPrivateAccess = "true"))
    int32 Offsides = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Match", meta = (AllowPrivateAccess = "true"))
    int32 Saves = 0;

    // Formation
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Formation", meta = (AllowPrivateAccess = "true"))
    int32 CurrentFormation = 433;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Formation", meta = (AllowPrivateAccess = "true"))
    TMap<int32, TArray<FVector>> FormationPositions; // FormationID -> Relative positions

    // Manager
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Team", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<AFootballTeamManager> TeamManager;

private:
    void UpdatePossessionTime(float DeltaTime);
    void UpdatePlayerPositions(float DeltaTime);
    void ApplyTacticalInstructions();
    FVector CalculateFormationPosition(EFootballPlayerRole::Type Role) const;
    void UpdateTeamShape(float DeltaTime);
};