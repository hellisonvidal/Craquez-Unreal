// Copyright 2024 Craquez. All Rights Reserved.

#include "Deterministic/FootballDeterministicReplication.h"
#include "BallPhysics/FootballBallComponent.h"
#include "Player/FootballPlayerCharacter.h"
#include "Team/FootballTeam.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

UFootballDeterministicReplicationComponent::UFootballDeterministicReplicationComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    SetIsReplicatedByDefault(true);
}

void UFootballDeterministicReplicationComponent::BeginPlay()
{
    Super::BeginPlay();

    // Find ball component if not set
    if (!BallComponent)
    {
        BallComponent = GetOwner()->FindComponentByClass<UFootballBallComponent>();
    }

    // Find player character if not set
    if (!PlayerCharacter)
    {
        PlayerCharacter = Cast<AFootballPlayerCharacter>(GetOwner());
    }

    StateHistory.Reserve(StateHistorySize);
}

void UFootballDeterministicReplicationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bDeterministicMode)
    {
        return;
    }

    UpdateInterpolation(DeltaTime);

    if (GetOwner()->HasAuthority())
    {
        // Server: capture state and send to clients
        CurrentFrame++;
        CaptureState(CurrentFrame);
        SendStateToServer();
    }
    else
    {
        // Client: receive state from server
        ReceiveStateFromServer();
    }

    CheckStateDivergence();
}

void UFootballDeterministicReplicationComponent::SetDeterministicMode(bool bEnabled)
{
    bDeterministicMode = bEnabled;
}

void UFootballDeterministicReplicationComponent::SetFixedTimeStep(float TimeStep)
{
    FixedTimeStep = FMath::Clamp(TimeStep, 1.0f / 120.0f, 1.0f / 30.0f);
}

void UFootballDeterministicReplicationComponent::SetMaxPredictionFrames(int32 Frames)
{
    MaxPredictionFrames = FMath::Clamp(Frames, 1, 10);
}

void UFootballDeterministicReplicationComponent::CaptureState(int32 FrameNumber)
{
    FFootballDeterministicState State(FrameNumber);

    if (BallComponent)
    {
        FFootballFixedVector Pos, Vel, AngVel;
        int32 Frame;
        BallComponent->GetDeterministicState(Pos, Vel, AngVel, Frame);

        State.Position = Pos;
        State.Velocity = Vel;
        State.AngularVelocity = AngVel;
        State.BallState = BallComponent->GetBallState();
    }
    else if (PlayerCharacter)
    {
        State.Position = FFootballFixedVector::FromVector(PlayerCharacter->GetActorLocation());
        State.Velocity = FFootballFixedVector::FromVector(PlayerCharacter->GetVelocity());
        State.Rotation = FFootballFixedRotator::FromRotator(PlayerCharacter->GetActorRotation());
        State.Stamina = PlayerCharacter->GetStaminaPercent();
        State.ActionState = PlayerCharacter->GetCurrentActionState();
    }

    // Add to history
    StateHistory.Add(State);

    // Trim history
    if (StateHistory.Num() > StateHistorySize)
    {
        StateHistory.RemoveAt(0);
    }
}

void UFootballDeterministicReplicationComponent::ApplyState(int32 FrameNumber)
{
    for (const FFootballDeterministicState& State : StateHistory)
    {
        if (State.FrameNumber == FrameNumber)
        {
            if (BallComponent)
            {
                BallComponent->SetDeterministicState(State.Position, State.Velocity, State.AngularVelocity, State.FrameNumber);
            }
            else if (PlayerCharacter)
            {
                PlayerCharacter->Server_SetDeterministicState(State.Position, State.Velocity, State.Rotation, State.FrameNumber);
            }
            break;
        }
    }
}

void UFootballDeterministicReplicationComponent::RollbackToFrame(int32 FrameNumber)
{
    // Find state in history
    for (int32 i = StateHistory.Num() - 1; i >= 0; i--)
    {
        if (StateHistory[i].FrameNumber <= FrameNumber)
        {
            ApplyState(StateHistory[i].FrameNumber);
            CurrentFrame = StateHistory[i].FrameNumber;
            break;
        }
    }
}

void UFootballDeterministicReplicationComponent::Server_SendState(int32 FrameNumber, const FFootballFixedVector& Position, const FFootballFixedVector& Velocity, const FFootballFixedRotator& Rotation)
{
    // Server sends authoritative state to clients
    // This would be called via RPC in a real implementation
}

void UFootballDeterministicReplicationComponent::Client_ReceiveState(int32 FrameNumber, const FFootballFixedVector& Position, const FFootballFixedVector& Velocity, const FFootballFixedRotator& Rotation)
{
    // Client receives state from server
    FFootballDeterministicState State(FrameNumber);
    State.Position = Position;
    State.Velocity = Velocity;
    State.Rotation = Rotation;

    // Check if we need to correct
    if (StateHistory.Num() > 0)
    {
        const FFootballDeterministicState& LatestLocal = StateHistory.Last();

        float PosDiff = FVector::Dist(State.Position.ToVector(), LatestLocal.Position.ToVector());
        float VelDiff = FVector::Dist(State.Velocity.ToVector(), LatestLocal.Velocity.ToVector());

        if (PosDiff > PositionTolerance || VelDiff > VelocityTolerance)
        {
            // Significant divergence - correct
            CorrectState(FrameNumber);
            OnStateCorrected.Broadcast(FrameNumber, State.Position.ToVector(), State.Velocity.ToVector());
        }
        else
        {
            // Minor difference - interpolate
            if (bInterpolate)
            {
                InterpolationStart = LatestLocal;
                InterpolationTarget = State;
                InterpolationAlpha = 0.0f;
            }
            else
            {
                ApplyState(FrameNumber);
            }
        }
    }
    else
    {
        // First state received
        ApplyState(FrameNumber);
    }
}

void UFootballDeterministicReplicationComponent::SetBallComponent(UFootballBallComponent* InBallComponent)
{
    BallComponent = InBallComponent;
}

void UFootballDeterministicReplicationComponent::ReplicateBallState(int32 FrameNumber)
{
    if (BallComponent && BallComponent->GetOwner()->HasAuthority())
    {
        CaptureState(FrameNumber);
        // Send to clients via RPC
    }
}

void UFootballDeterministicReplicationComponent::ReplicatePlayerState(AFootballPlayerCharacter* Player, int32 FrameNumber)
{
    if (Player && Player->HasAuthority())
    {
        // Capture player state and send
    }
}

void UFootballDeterministicReplicationComponent::SetInterpolationEnabled(bool bEnabled)
{
    bInterpolate = bEnabled;
}

void UFootballDeterministicReplicationComponent::SetInterpolationTime(float Time)
{
    InterpolationTime = FMath::Max(Time, 0.0f);
}

void UFootballDeterministicReplicationComponent::UpdateInterpolation(float DeltaTime)
{
    if (!bInterpolate || InterpolationAlpha >= 1.0f)
    {
        return;
    }

    InterpolationAlpha += DeltaTime / InterpolationTime;
    InterpolationAlpha = FMath::Clamp(InterpolationAlpha, 0.0f, 1.0f);

    // Interpolate position
    FVector StartPos = InterpolationStart.Position.ToVector();
    FVector TargetPos = InterpolationTarget.Position.ToVector();
    FVector InterpPos = FMath::Lerp(StartPos, TargetPos, InterpolationAlpha);

    // Interpolate velocity
    FVector StartVel = InterpolationStart.Velocity.ToVector();
    FVector TargetVel = InterpolationTarget.Velocity.ToVector();
    FVector InterpVel = FMath::Lerp(StartVel, TargetVel, InterpolationAlpha);

    // Interpolate rotation
    FRotator StartRot = InterpolationStart.Rotation.ToRotator();
    FRotator TargetRot = InterpolationTarget.Rotation.ToRotator();
    FRotator InterpRot = FMath::Lerp(StartRot, TargetRot, InterpolationAlpha);

    if (BallComponent)
    {
        BallComponent->SetWorldLocationAndRotation(InterpPos, InterpRot.Quaternion());
        // Velocity is set by physics simulation
    }
    else if (PlayerCharacter)
    {
        PlayerCharacter->SetActorLocationAndRotation(InterpPos, InterpRot);
        PlayerCharacter->GetCharacterMovement()->Velocity = InterpVel;
    }

    if (InterpolationAlpha >= 1.0f)
    {
        // Interpolation complete
        InterpolationAlpha = 1.0f;
    }
}

void UFootballDeterministicReplicationComponent::CheckStateDivergence()
{
    if (StateHistory.Num() < 2)
    {
        return;
    }

    // Compare last two states for divergence detection
    const FFootballDeterministicState& Latest = StateHistory.Last();
    const FFootballDeterministicState& Previous = StateHistory[StateHistory.Num() - 2];

    // Check for impossible physics (teleportation, extreme acceleration)
    FVector VelDiff = Latest.Velocity.ToVector() - Previous.Velocity.ToVector();
    float Acceleration = VelDiff.Size() / FixedTimeStep;

    if (Acceleration > 5000.0f) // 50 m/s² threshold
    {
        // Possible desync - log warning
        UE_LOG(LogFootballCoreNetworking, Warning, TEXT("High acceleration detected: %f m/s²"), Acceleration / 100.0f);
    }
}

void UFootballDeterministicReplicationComponent::CorrectState(int32 FrameNumber)
{
    ApplyState(FrameNumber);
    LastAcknowledgedFrame = FrameNumber;
}

void UFootballDeterministicReplicationComponent::SendStateToServer()
{
    if (!GetOwner()->HasAuthority())
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime - LastStateSendTime >= StateSendRate)
        {
            // Send current state to server via RPC
            // Client_SendState(CurrentFrame, Position, Velocity, Rotation);
            LastStateSendTime = CurrentTime;
        }
    }
}

void UFootballDeterministicReplicationComponent::ReceiveStateFromServer()
{
    // Client receives state from server via RPC
    // Handled by Client_ReceiveState RPC
}

bool UFootballDeterministicReplicationComponent::StatesMatch(const FFootballDeterministicState& A, const FFootballDeterministicState& B) const
{
    float PosDiff = FVector::Dist(A.Position.ToVector(), B.Position.ToVector());
    float VelDiff = FVector::Dist(A.Velocity.ToVector(), B.Velocity.ToVector());

    return PosDiff <= PositionTolerance && VelDiff <= VelocityTolerance;
}

// AFootballDeterministicManager
AFootballDeterministicManager::AFootballDeterministicManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PrePhysics;
    bReplicates = true;
}

void AFootballDeterministicManager::BeginPlay()
{
    Super::BeginPlay();
    InputHistory.Reserve(MaxInputHistory);
}

void AFootballDeterministicManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bReplaying)
    {
        AdvanceFrame();
        UpdateEntities(DeltaTime);
        ProcessInputs();
        CheckDesync();
    }
    else
    {
        // Replay mode
        // Would replay recorded inputs
    }
}

void AFootballDeterministicManager::RegisterEntity(AActor* Entity, UFootballDeterministicReplicationComponent* ReplicationComponent)
{
    if (Entity && ReplicationComponent)
    {
        RegisteredEntities.Add(Entity, ReplicationComponent);
    }
}

void AFootballDeterministicManager::UnregisterEntity(AActor* Entity)
{
    RegisteredEntities.Remove(Entity);
}

void AFootballDeterministicManager::AdvanceFrame()
{
    CurrentFrame++;
    ServerFrame = CurrentFrame;
}

void AFootballDeterministicManager::AddInputFrame(const FFootballInputFrame& InputFrame)
{
    if (bRecording)
    {
        InputHistory.Add(InputFrame);

        if (InputHistory.Num() > MaxInputHistory)
        {
            InputHistory.RemoveAt(0);
        }
    }
}

FFootballInputFrame AFootballDeterministicManager::GetInputFrame(int32 FrameNumber) const
{
    for (const FFootballInputFrame& Frame : InputHistory)
    {
        if (Frame.FrameNumber == FrameNumber)
        {
            return Frame;
        }
    }
    return FFootballInputFrame(FrameNumber);
}

void AFootballDeterministicManager::SynchronizeAll()
{
    for (auto& Pair : RegisteredEntities)
    {
        if (Pair.Value)
        {
            Pair.Value->CaptureState(CurrentFrame);
        }
    }
}

void AFootballDeterministicManager::ForceCorrection(AActor* Entity)
{
    if (UFootballDeterministicReplicationComponent* Comp = RegisteredEntities.FindRef(Entity))
    {
        Comp->CorrectState(ServerFrame);
    }
}

void AFootballDeterministicManager::StartRecording()
{
    bRecording = true;
    InputHistory.Empty();
}

void AFootballDeterministicManager::StopRecording()
{
    bRecording = false;
}

void AFootballDeterministicManager::PlayReplay(int32 StartFrame)
{
    bReplaying = true;
    ReplayStartFrame = StartFrame;
    CurrentFrame = StartFrame;
}

void AFootballDeterministicManager::UpdateEntities(float DeltaTime)
{
    for (auto& Pair : RegisteredEntities)
    {
        if (Pair.Value && Pair.Key)
        {
            Pair.Value->TickComponent(DeltaTime, TG_PrePhysics, nullptr);
        }
    }
}

void AFootballDeterministicManager::ProcessInputs()
{
    // Process input frames for all registered entities
    // This would apply recorded inputs during replay
}

void AFootballDeterministicManager::CheckDesync()
{
    // Check for desync between entities
    // Compare states of interacting entities (ball vs players)
}

// UFootballNetworkPredictionComponent
UFootballNetworkPredictionComponent::UFootballNetworkPredictionComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UFootballNetworkPredictionComponent::BeginPlay()
{
    Super::BeginPlay();
    PredictedStates.Reserve(60); // 1 second at 60Hz
}

void UFootballNetworkPredictionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bEnablePrediction && !GetOwner()->HasAuthority())
    {
        PredictMovement(DeltaTime);
    }

    if (bReconciling)
    {
        InterpolateToServerState();
    }
}

void UFootballNetworkPredictionComponent::EnablePrediction(bool bEnable)
{
    bEnablePrediction = bEnable;
}

void UFootballNetworkPredictionComponent::PredictMovement(float DeltaTime)
{
    // Client-side prediction: simulate movement locally
    // Store predicted state for reconciliation
    FFootballDeterministicState PredictedState;
    PredictedState.FrameNumber = GetWorld()->GetFrameCount();
    PredictedState.Position = FFootballFixedVector::FromVector(GetOwner()->GetActorLocation());
    PredictedState.Velocity = FFootballFixedVector::FromVector(GetOwner()->GetVelocity());
    PredictedState.Rotation = FFootballFixedRotator::FromRotator(GetOwner()->GetActorRotation());

    PredictedStates.Add(PredictedState);

    // Trim history
    if (PredictedStates.Num() > 120)
    {
        PredictedStates.RemoveAt(0);
    }

    PendingFrames++;
}

void UFootballNetworkPredictionComponent::ConfirmServerState(const FFootballDeterministicState& ServerState)
{
    // Server state received - reconcile
    LastConfirmedState = ServerState;
    bReconciling = true;
    PendingFrames = 0;
}

void UFootballNetworkPredictionComponent::ReconcileState()
{
    // Compare predicted state with server state
    if (PredictedStates.Num() > 0)
    {
        const FFootballDeterministicState& LatestPredicted = PredictedStates.Last();

        float PosDiff = FVector::Dist(LatestPredicted.Position.ToVector(), LastConfirmedState.Position.ToVector());
        float VelDiff = FVector::Dist(LatestPredicted.Velocity.ToVector(), LastConfirmedState.Velocity.ToVector());

        if (PosDiff > MaxPredictionError || VelDiff > MaxPredictionError * 10.0f)
        {
            // Significant divergence - snap to server state
            GetOwner()->SetActorLocationAndRotation(
                LastConfirmedState.Position.ToVector(),
                LastConfirmedState.Rotation.ToRotator()
            );

            // Clear predicted states
            PredictedStates.Empty();
            bReconciling = false;
        }
        else
        {
            // Minor difference - interpolate
            InterpolateToServerState();
        }
    }
}

void UFootballNetworkPredictionComponent::ApplyPrediction(float DeltaTime)
{
    // Apply predicted movement
    // This is done in the character/ball movement components
}

void UFootballNetworkPredictionComponent::InterpolateToServerState()
{
    // Smooth interpolation to server state
    FVector CurrentPos = GetOwner()->GetActorLocation();
    FVector TargetPos = LastConfirmedState.Position.ToVector();

    FVector NewPos = FMath::VInterpTo(CurrentPos, TargetPos, GetWorld()->GetDeltaSeconds(), ReconciliationSpeed / 100.0f);
    GetOwner()->SetActorLocation(NewPos);

    // Check if close enough
    if (FVector::Dist(NewPos, TargetPos) < 5.0f)
    {
        bReconciling = false;
    }
}

// UFootballLagCompensationComponent
UFootballLagCompensationComponent::UFootballLagCompensationComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    StateHistory.Reserve(60);
    InputHistory.Reserve(60);
}

void UFootballLagCompensationComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UFootballLagCompensationComponent::RewindToTime(float ServerTime, float ClientPing)
{
    // Rewind entity to client's view time
    float TargetTime = ServerTime - ClientPing / 1000.0f; // Convert ping to seconds

    if (TargetTime < ServerTime - MaxRewindTime)
    {
        TargetTime = ServerTime - MaxRewindTime;
    }

    FFootballDeterministicState RewindState = GetStateAtTime(TargetTime);

    if (RewindState.IsValid())
    {
        GetOwner()->SetActorLocationAndRotation(
            RewindState.Position.ToVector(),
            RewindState.Rotation.ToRotator()
        );
    }
}

void UFootballLagCompensationComponent::RestoreState()
{
    // Restore to current server time
    if (StateHistory.Num() > 0)
    {
        const FFootballDeterministicState& Latest = StateHistory.Last();
        GetOwner()->SetActorLocationAndRotation(
            Latest.Position.ToVector(),
            Latest.Rotation.ToRotator()
        );
    }
}

bool UFootballLagCompensationComponent::CheckHitAtTime(AActor* Target, float ServerTime, float ClientPing, FHitResult& OutHit)
{
    if (!Target)
    {
        return false;
    }

    // Rewind target to client's view time
    RewindToTime(ServerTime, ClientPing);

    // Perform hit check
    // This would use the target's collision at the rewound position
    // For simplicity, we just do a basic trace

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());

    FVector Start = GetOwner()->GetActorLocation();
    FVector End = Start + GetOwner()->GetActorForwardVector() * 200.0f;

    bool bHit = GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Pawn, QueryParams);

    // Restore target
    RestoreState();

    return bHit;
}

void UFootballLagCompensationComponent::RecordState()
{
    FFootballDeterministicState State;
    State.FrameNumber = GetWorld()->GetFrameCount();
    State.Position = FFootballFixedVector::FromVector(GetOwner()->GetActorLocation());
    State.Velocity = FFootballFixedVector::FromVector(GetOwner()->GetVelocity());
    State.Rotation = FFootballFixedRotator::FromRotator(GetOwner()->GetActorRotation());

    StateHistory.Add(State);

    if (StateHistory.Num() > MaxHistoryFrames)
    {
        StateHistory.RemoveAt(0);
    }
}

FFootballDeterministicState UFootballLagCompensationComponent::GetStateAtTime(float Time) const
{
    int32 TargetFrame = FMath::RoundToInt(Time * 60.0f); // Assuming 60Hz

    // Find closest frame
    for (int32 i = StateHistory.Num() - 1; i >= 0; i--)
    {
        if (StateHistory[i].FrameNumber <= TargetFrame)
        {
            return StateHistory[i];
        }
    }

    return FFootballDeterministicState();
}