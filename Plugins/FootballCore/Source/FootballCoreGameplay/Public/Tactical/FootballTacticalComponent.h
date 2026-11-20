// Copyright 2024 Craquez. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "FootballCoreTypes.h"
#include "FootballTacticalComponent.generated.h"

class AFootballTeam;
class AFootballPlayerCharacter;
class UFootballBallComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTacticalInstructionChanged, FGameplayTag, Instruction, float, Value);

/**
 * Tactical Component - Handles team tactics, player positioning, and strategic decisions
 */
UCLASS(ClassGroup = (Football), meta = (BlueprintSpawnableComponent))
class FOOTBALLCOREGAMEPLAY_API UFootballTacticalComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UFootballTacticalComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

    // Initialization
    UFUNCTION(BlueprintCallable, Category = "Football|Tactical")
    void SetTeam(AFootballTeam* InTeam);

    UFUNCTION(BlueprintCallable, Category = "Football|Tactical")
    void Initialize(const FFootballTacticalInstructions& InTactics);

    UFUNCTION(BlueprintCallable, Category = "Football|Tactical")
    void UpdateTactics(const FFootballTacticalInstructions& InTactics);

    // Instructions
    UFUNCTION(BlueprintCallable, Category = "Football|Tactical")
    void SetInstruction(FGameplayTag Instruction, float Value);

    UFUNCTION(BlueprintPure, Category = "Football|Tactical")
    float GetInstruction(FGameplayTag Instruction) const;

    // Player Positioning
    UFUNCTION(BlueprintCallable, Category = "Football|Tactical")
    void SetPlayerTargetPosition(AFootballPlayerCharacter* Player, FVector TargetPosition);

    UFUNCTION(BlueprintCallable, Category = "Football|Tactical")
    void UpdatePlayerPosition(AFootballPlayerCharacter* Player, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Football|Tactical")
    FVector CalculateDefensivePosition(AFootballPlayerCharacter* Player) const;

    UFUNCTION(BlueprintCallable, Category = "Football|Tactical")
    FVector CalculateAttackingPosition(AFootballPlayerCharacter* Player) const;

    UFUNCTION(BlueprintCallable, Category = "Football|Tactical")
    FVector CalculateSupportPosition(AFootballPlayerCharacter* Player) const;

    // Team Shape
    UFUNCTION(BlueprintCallable, Category = "Football|Tactical")
    void MaintainTeamShape(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Football|Tactical")
    float CalculateTeamCompactness() const;

    UFUNCTION(BlueprintCallable, Category = "Football|Tactical")
    FVector CalculateTeamCenter() const;

    UFUNCTION(BlueprintCallable, Category = "Football|Tactical")
    float CalculateDefensiveLine() const;

    UFUNCTION(BlueprintCallable, Category = "Football|Tactical")
    float CalculateOffsideLine() const;

    // Tactical Analysis
    UFUNCTION(BlueprintCallable, Category = "Football|Tactical")
    TArray<AFootballPlayerCharacter*> GetPlayersInZone(FVector ZoneCenter, float ZoneRadius) const;

    UFUNCTION(BlueprintCallable, Category = "Football|Tactical")
    int32 CountPlayersInArea(FVector Min, FVector Max) const;

    UFUNCTION(BlueprintCallable, Category = "Football|Tactical")
    bool IsPlayerMarked(AFootballPlayerCharacter* Player) const;

    UFUNCTION(BlueprintCallable, Category = "Football|Tactical")
    AFootballPlayerCharacter* FindBestPassTarget(AFootballPlayerCharacter* Passer) const;

    UFUNCTION(BlueprintCallable, Category = "Football|Tactical")
    AFootballPlayerCharacter* FindBestInterceptTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Football|Tactical")
    FVector CalculateInterceptPoint(UFootballBallComponent* Ball, AFootballPlayerCharacter* Interceptor) const;

    // Set Pieces
    UFUNCTION(BlueprintCallable, Category = "Football|Tactical|SetPiece")
    void SetupCornerRoutine(bool bAttacking);

    UFUNCTION(BlueprintCallable, Category = "Football|Tactical|SetPiece")
    void SetupFreeKickRoutine(FVector BallPosition, bool bAttacking);

    UFUNCTION(BlueprintCallable, Category = "Football|Tactical|SetPiece")
    void SetupPenaltyRoutine(bool bAttacking);

    UFUNCTION(BlueprintCallable, Category = "Football|Tactical|SetPiece")
    void ExecuteSetPieceRoutine(EFootballMatchPhase::Type SetPieceType);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Football|Events")
    FOnTacticalInstructionChanged OnInstructionChanged;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Tactical", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<AFootballTeam> Team;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Tactical", meta = (AllowPrivateAccess = "true"))
    FFootballTacticalInstructions CurrentTactics;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Tactical", meta = (AllowPrivateAccess = "true"))
    TMap<FGameplayTag, float> Instructions;

    // Player target positions
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Tactical", meta = (AllowPrivateAccess = "true"))
    TMap<TObjectPtr<AFootballPlayerCharacter>, FVector> PlayerTargetPositions;

    // Set piece routines
    TMap<EFootballMatchPhase::Type, TArray<FVector>> SetPiecePositions;

    // Tactical zones
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Tactical|Zones",
              meta = (AllowPrivateAccess = "true"))
    TArray<FBox> DefensiveZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Tactical|Zones",
              meta = (AllowPrivateAccess = "true"))
    TArray<FBox> AttackingZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Tactical|Zones",
              meta = (AllowPrivateAccess = "true"))
    TArray<FBox> PressingZones;

private:
    void UpdatePlayerMovement(float DeltaTime);
    void ApplyPressing(float DeltaTime);
    void ApplyOffsideTrap(float DeltaTime);
    void ApplyCounterAttack(float DeltaTime);
    FVector CalculateIdealPosition(AFootballPlayerCharacter* Player) const;
    float GetPlayerRoleWeight(EFootballPlayerRole::Type Role, bool bAttacking) const;
    float CalculatePassScore(AFootballPlayerCharacter* Passer, AFootballPlayerCharacter* Receiver) const;
};