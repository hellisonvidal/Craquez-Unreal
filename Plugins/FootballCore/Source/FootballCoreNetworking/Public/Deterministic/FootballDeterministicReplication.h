// Copyright 2024 Craquez. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"
#include "FootballCoreTypes.h"
#include "FootballDeterministicReplication.generated.h"

class UFootballBallComponent;
class AFootballPlayerCharacter;
class AFootballTeam;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnStateCorrected, int32, FrameNumber, FVector, CorrectedPosition, FVector, CorrectedVelocity);

/**
 * Deterministic Replication Component - Ensures deterministic physics simulation across network
 * Uses fixed-point math and frame-accurate synchronization
 */
UCLASS(ClassGroup = (Football), meta = (BlueprintSpawnableComponent))
class FOOTBALLCORENETWORKING_API UFootballDeterministicReplicationComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UFootballDeterministicReplicationComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Football|Network|Deterministic")
    void SetDeterministicMode(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Football|Network|Deterministic")
    void SetFixedTimeStep(float TimeStep);

    UFUNCTION(BlueprintCallable, Category = "Football|Network|Deterministic")
    void SetMaxPredictionFrames(int32 Frames);

    // State Management
    UFUNCTION(BlueprintCallable, Category = "Football|Network|Deterministic")
    void CaptureState(int32 FrameNumber);

    UFUNCTION(BlueprintCallable, Category = "Football|Network|Deterministic")
    void ApplyState(int32 FrameNumber);

    UFUNCTION(BlueprintCallable, Category = "Football|Network|Deterministic")
    void RollbackToFrame(int32 FrameNumber);

    UFUNCTION(BlueprintCallable, Category = "Football|Network|Deterministic")
    void Server_SendState(int32 FrameNumber, const FFootballFixedVector& Position, const FFootballFixedVector& Velocity, const FFootballFixedRotator& Rotation);

    UFUNCTION(BlueprintCallable, Category = "Football|Network|Deterministic")
    void Client_ReceiveState(int32 FrameNumber, const FFootballFixedVector& Position, const FFootballFixedVector& Velocity, const FFootballFixedRotator& Rotation);

    // Ball-specific
    UFUNCTION(BlueprintCallable, Category = "Football|Network|Deterministic")
    void SetBallComponent(UFootballBallComponent* InBallComponent);

    UFUNCTION(BlueprintCallable, Category = "Football|Network|Deterministic")
    void ReplicateBallState(int32 FrameNumber);

    // Player-specific
    UFUNCTION(BlueprintCallable, Category = "Football|Network|Deterministic")
    void ReplicatePlayerState(AFootballPlayerCharacter* Player, int32 FrameNumber);

    // Interpolation
    UFUNCTION(BlueprintCallable, Category = "Football|Network|Deterministic")
    void SetInterpolationEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Football|Network|Deterministic")
    void SetInterpolationTime(float Time);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Football|Network|Events")
    FOnStateCorrected OnStateCorrected;

protected:
    // Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Network|Deterministic", meta = (AllowPrivateAccess = "true"))
    bool bDeterministicMode = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Network|Deterministic", meta = (AllowPrivateAccess = "true"))
    float FixedTimeStep = 1.0f / 60.0f; // 60Hz

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Network|Deterministic", meta = (AllowPrivateAccess = "true"))
    int32 MaxPredictionFrames = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Network|Deterministic", meta = (AllowPrivateAccess = "true"))
    int32 StateHistorySize = 120; // 2 seconds at 60Hz

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Network|Deterministic", meta = (AllowPrivateAccess = "true"))
    float PositionTolerance = 10.0f; // cm

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Network|Deterministic", meta = (AllowPrivateAccess = "true"))
    float VelocityTolerance = 50.0f; // cm/s

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Network|Deterministic", meta = (AllowPrivateAccess = "true"))
    bool bInterpolate = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Network|Deterministic", meta = (AllowPrivateAccess = "true"))
    float InterpolationTime = 0.1f; // 100ms interpolation buffer

    // State History
    UPROPERTY()
    TArray<FFootballDeterministicState> StateHistory;

    UPROPERTY()
    int32 CurrentFrame = 0;

    UPROPERTY()
    int32 ServerFrame = 0;

    UPROPERTY()
    int32 LastAcknowledgedFrame = 0;

    // Components
    UPROPERTY()
    TObjectPtr<UFootballBallComponent> BallComponent = nullptr;

    UPROPERTY()
    TObjectPtr<AFootballPlayerCharacter> PlayerCharacter = nullptr;

    // Interpolation
    UPROPERTY()
    FFootballDeterministicState InterpolationTarget;

    UPROPERTY()
    FFootballDeterministicState InterpolationStart;

    UPROPERTY()
    float InterpolationAlpha = 0.0f;

    // Network
    UPROPERTY()
    float LastStateSendTime = 0.0f;

    UPROPERTY()
    float StateSendRate = 1.0f / 30.0f; // 30Hz state updates

private:
    void UpdateInterpolation(float DeltaTime);
    void CheckStateDivergence();
    void CorrectState(int32 FrameNumber);
    void SendStateToServer();
    void ReceiveStateFromServer();
    bool StatesMatch(const FFootballDeterministicState& A, const FFootballDeterministicState& B) const;
};

/**
 * Deterministic State Structure - Fixed-point for network replication
 */
USTRUCT(BlueprintType)
struct FFootballDeterministicState
{
    GENERATED_BODY()

    UPROPERTY()
    int32 FrameNumber = 0;

    UPROPERTY()
    FFootballFixedVector Position;

    UPROPERTY()
    FFootballFixedVector Velocity;

    UPROPERTY()
    FFootballFixedVector AngularVelocity;

    UPROPERTY()
    FFootballFixedRotator Rotation;

    UPROPERTY()
    EFootballBallState::Type BallState = EFootballBallState::Free;

    UPROPERTY()
    float Stamina = 100.0f;

    UPROPERTY()
    FGameplayTag ActionState;

    UPROPERTY()
    uint32 InputFlags = 0; // Packed input state

    FFootballDeterministicState() {}
    FFootballDeterministicState(int32 InFrame) : FrameNumber(InFrame) {}

    bool IsValid() const { return FrameNumber >= 0; }
};

/**
 * Input Frame - For deterministic input replay
 */
USTRUCT(BlueprintType)
struct FFootballInputFrame
{
    GENERATED_BODY()

    UPROPERTY()
    int32 FrameNumber = 0;

    UPROPERTY()
    FVector2D MoveInput = FVector2D::ZeroVector;

    UPROPERTY()
    FVector2D LookInput = FVector2D::ZeroVector;

    UPROPERTY()
    uint32 ActionBits = 0; // Packed action buttons

    UPROPERTY()
    float DeltaTime = 1.0f / 60.0f;

    FFootballInputFrame() {}
    FFootballInputFrame(int32 InFrame) : FrameNumber(InFrame) {}
};

/**
 * Deterministic Replication Manager - Coordinates replication for all entities
 */
UCLASS(Blueprintable, BlueprintType)
class FOOTBALLCORENETWORKING_API AFootballDeterministicManager : public AActor
{
    GENERATED_BODY()

public:
    AFootballDeterministicManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Entity Registration
    UFUNCTION(BlueprintCallable, Category = "Football|Network|Deterministic")
    void RegisterEntity(AActor* Entity, UFootballDeterministicReplicationComponent* ReplicationComponent);

    UFUNCTION(BlueprintCallable, Category = "Football|Network|Deterministic")
    void UnregisterEntity(AActor* Entity);

    // Frame Management
    UFUNCTION(BlueprintCallable, Category = "Football|Network|Deterministic")
    void AdvanceFrame();

    UFUNCTION(BlueprintCallable, Category = "Football|Network|Deterministic")
    int32 GetCurrentFrame() const { return CurrentFrame; }

    UFUNCTION(BlueprintCallable, Category = "Football|Network|Deterministic")
    void SetServerFrame(int32 Frame) { ServerFrame = Frame; }

    // Input Management
    UFUNCTION(BlueprintCallable, Category = "Football|Network|Deterministic")
    void AddInputFrame(const FFootballInputFrame& InputFrame);

    UFUNCTION(BlueprintCallable, Category = "Football|Network|Deterministic")
    FFootballInputFrame GetInputFrame(int32 FrameNumber) const;

    // State Synchronization
    UFUNCTION(BlueprintCallable, Category = "Football|Network|Deterministic")
    void SynchronizeAll();

    UFUNCTION(BlueprintCallable, Category = "Football|Network|Deterministic")
    void ForceCorrection(AActor* Entity);

    // Replay
    UFUNCTION(BlueprintCallable, Category = "Football|Network|Replay")
    void StartRecording();

    UFUNCTION(BlueprintCallable, Category = "Football|Network|Replay")
    void StopRecording();

    UFUNCTION(BlueprintCallable, Category = "Football|Network|Replay")
    void PlayReplay(int32 StartFrame);

    // Getters
    UFUNCTION(BlueprintPure, Category = "Football|Network|Deterministic")
    int32 GetRegisteredEntityCount() const { return RegisteredEntities.Num(); }

protected:
    // Frame counter
    UPROPERTY()
    int32 CurrentFrame = 0;

    UPROPERTY()
    int32 ServerFrame = 0;

    // Registered entities
    UPROPERTY()
    TMap<TObjectPtr<AActor>, TObjectPtr<UFootballDeterministicReplicationComponent>> RegisteredEntities;

    // Input history for replay
    UPROPERTY()
    TArray<FFootballInputFrame> InputHistory;

    // Replay
    UPROPERTY()
    bool bRecording = false;

    UPROPERTY()
    bool bReplaying = false;

    UPROPERTY()
    int32 ReplayStartFrame = 0;

    // Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Network|Deterministic")
    int32 MaxInputHistory = 3600; // 60 seconds at 60Hz

private:
    void UpdateEntities(float DeltaTime);
    void ProcessInputs();
    void CheckDesync();
};

/**
 * Blueprint Interface for Deterministic Replication
 * Allows Blueprint-only classes to participate in deterministic replication
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UFootballDeterministicInterface : public UInterface
{
    GENERATED_BODY()
};

class FOOTBALLCORENETWORKING_API IFootballDeterministicInterface
{
    GENERATED_BODY()

public:
    // Called to get current deterministic state
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Football|Network|Deterministic")
    FFootballDeterministicState GetDeterministicState();

    // Called to apply deterministic state
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Football|Network|Deterministic")
    void ApplyDeterministicState(const FFootballDeterministicState& State);

    // Called when state correction is needed
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Football|Network|Deterministic")
    void OnStateCorrection(int32 FrameNumber, const FFootballDeterministicState& CorrectedState);

    // Called to pack inputs for network transmission
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Football|Network|Deterministic")
    FFootballInputFrame PackInputs();

    // Called to unpack and apply inputs
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Football|Network|Deterministic")
    void UnpackInputs(const FFootballInputFrame& InputFrame);
};

/**
 * Network Prediction Component - Client-side prediction for responsive gameplay
 */
UCLASS(ClassGroup = (Football), meta = (BlueprintSpawnableComponent))
class FOOTBALLCORENETWORKING_API UFootballNetworkPredictionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UFootballNetworkPredictionComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Prediction
    UFUNCTION(BlueprintCallable, Category = "Football|Network|Prediction")
    void EnablePrediction(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Football|Network|Prediction")
    void PredictMovement(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Football|Network|Prediction")
    void ConfirmServerState(const FFootballDeterministicState& ServerState);

    UFUNCTION(BlueprintCallable, Category = "Football|Network|Prediction")
    void ReconcileState();

    // Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Network|Prediction", meta = (AllowPrivateAccess = "true"))
    bool bEnablePrediction = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Network|Prediction", meta = (AllowPrivateAccess = "true"))
    float MaxPredictionError = 50.0f; // cm

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Network|Prediction", meta = (AllowPrivateAccess = "true"))
    float ReconciliationSpeed = 10.0f; // cm/s

protected:
    UPROPERTY()
    TArray<FFootballDeterministicState> PredictedStates;

    UPROPERTY()
    FFootballDeterministicState LastConfirmedState;

    UPROPERTY()
    int32 PendingFrames = 0;

    UPROPERTY()
    bool bReconciling = false;

private:
    void ApplyPrediction(float DeltaTime);
    void InterpolateToServerState();
};

/**
 * Lag Compensation Component - Server-side lag compensation for fair gameplay
 */
UCLASS(ClassGroup = (Football), meta = (BlueprintSpawnableComponent))
class FOOTBALLCORENETWORKING_API UFootballLagCompensationComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UFootballLagCompensationComponent();

    virtual void BeginPlay() override;

    // Lag Compensation
    UFUNCTION(BlueprintCallable, Category = "Football|Network|LagCompensation")
    void RewindToTime(float ServerTime, float ClientPing);

    UFUNCTION(BlueprintCallable, Category = "Football|Network|LagCompensation")
    void RestoreState();

    UFUNCTION(BlueprintCallable, Category = "Football|Network|LagCompensation")
    bool CheckHitAtTime(AActor* Target, float ServerTime, float ClientPing, FHitResult& OutHit);

    // Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Network|LagCompensation", meta = (AllowPrivateAccess = "true"))
    float MaxRewindTime = 0.5f; // 500ms max rewind

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Network|LagCompensation", meta = (AllowPrivateAccess = "true"))
    int32 MaxHistoryFrames = 60; // 1 second at 60Hz

protected:
    UPROPERTY()
    TArray<FFootballDeterministicState> StateHistory;

    UPROPERTY()
    TArray<FFootballInputFrame> InputHistory;

private:
    void RecordState();
    FFootballDeterministicState GetStateAtTime(float Time) const;
};