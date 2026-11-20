// Copyright 2024 Craquez. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "PhysicsEngine/PhysicsThrusterComponent.h"
#include "FootballCoreTypes.h"
#include "FootballBallComponent.generated.h"

class UPrimitiveComponent;
class UNiagaraComponent;
class USoundBase;
class UPhysicalMaterial;
class UCurveFloat;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnBallKicked, const FVector&, Impulse, const FVector&, Spin, AActor*,
                                               Instigator);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBallCollision, const FHitResult&, HitResult, AActor*, OtherActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBallGoalScored, EFootballTeamSide::Type, ScoringTeam);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBallOutOfPlay, EFootballTeamSide::Type, LastTouchTeam);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBallStateChanged, EFootballBallState::Type, NewState);

/**
 * Football Ball Component - Implements realistic ball physics including:
 * - Magnus Effect (curved trajectories from spin)
 * - Air drag and lift forces
 * - Ground friction (rolling/sliding)
 * - Deformation on impact
 * - Precise collision detection
 * - Deterministic simulation for network replication
 */
UCLASS(ClassGroup = (Football), meta = (BlueprintSpawnableComponent), Blueprintable)
class FOOTBALLCORE_API UFootballBallComponent : public USceneComponent
{
    GENERATED_BODY()

public:
    UFootballBallComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;
    virtual void OnRegister() override;

    // Ball Physics Simulation
    UFUNCTION(BlueprintCallable, Category = "Football|Ball Physics")
    void InitializeBall(const FFootballBallPhysicsParams& InParams);

    UFUNCTION(BlueprintCallable, Category = "Football|Ball Physics")
    void ApplyKick(const FVector& Impulse, const FVector& Spin, AActor* Instigator);

    UFUNCTION(BlueprintCallable, Category = "Football|Ball Physics")
    void ApplyImpulseAtLocation(const FVector& Impulse, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Football|Ball Physics")
    void SetBallState(EFootballBallState::Type NewState);

    UFUNCTION(BlueprintCallable, Category = "Football|Ball Physics")
    void ResetBall(const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "Football|Ball Physics")
    void SetPossessingPlayer(AActor* Player);

    UFUNCTION(BlueprintPure, Category = "Football|Ball Physics")
    FVector GetVelocity() const { return CurrentVelocity; }

    UFUNCTION(BlueprintPure, Category = "Football|Ball Physics")
    FVector GetAngularVelocity() const { return CurrentAngularVelocity; }

    UFUNCTION(BlueprintPure, Category = "Football|Ball Physics")
    EFootballBallState::Type GetBallState() const { return CurrentBallState; }

    UFUNCTION(BlueprintPure, Category = "Football|Ball Physics")
    AActor* GetPossessingPlayer() const { return PossessingPlayer; }

    UFUNCTION(BlueprintPure, Category = "Football|Ball Physics")
    FFootballBallPhysicsParams GetPhysicsParams() const { return PhysicsParams; }

    void SetEffectComponents(UNiagaraComponent* InTrail, UNiagaraComponent* InSpin, UNiagaraComponent* InDeformation)
    {
        TrailEffect = InTrail;
        SpinEffect = InSpin;
        DeformationEffect = InDeformation;
    }

    // Deterministic replication
    UFUNCTION(BlueprintCallable, Category = "Football|Network")
    void SetDeterministicState(const FFootballFixedVector& Position, const FFootballFixedVector& Velocity,
                               const FFootballFixedVector& AngularVelocity, int32 FrameNumber);

    UFUNCTION(BlueprintCallable, Category = "Football|Network")
    void GetDeterministicState(FFootballFixedVector& OutPosition, FFootballFixedVector& OutVelocity,
                               FFootballFixedVector& OutAngularVelocity, int32& OutFrameNumber) const;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Football|Events")
    FOnBallKicked OnBallKicked;

    UPROPERTY(BlueprintAssignable, Category = "Football|Events")
    FOnBallCollision OnBallCollision;

    UPROPERTY(BlueprintAssignable, Category = "Football|Events")
    FOnBallGoalScored OnBallGoalScored;

    UPROPERTY(BlueprintAssignable, Category = "Football|Events")
    FOnBallOutOfPlay OnBallOutOfPlay;

    UPROPERTY(BlueprintAssignable, Category = "Football|Events")
    FOnBallStateChanged OnBallStateChanged;

protected:
    // Physics simulation
    void SimulatePhysics(float DeltaTime);
    void ApplyAerodynamicForces(float DeltaTime);
    void ApplyMagnusForce(float DeltaTime);
    void ApplyGroundInteraction(float DeltaTime);
    void UpdateDeformation(float DeltaTime);
    void CheckCollisions(float DeltaTime);
    void HandleCollision(const FHitResult& HitResult);

    // State management
    void TransitionToState(EFootballBallState::Type NewState);
    bool CanTransitionTo(EFootballBallState::Type NewState) const;

    // Visual/Audio
    void UpdateVisuals(float DeltaTime);
    void PlayImpactSound(const FHitResult& HitResult);
    void SpawnImpactEffect(const FHitResult& HitResult);
    void UpdateNiagaraParameters();

    // Ground check
    bool CheckGroundContact(FHitResult& OutHitResult) const;
    FVector CalculateGroundVelocity(const FHitResult& HitResult) const;

private:
    // Physics Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Ball Physics",
              meta = (AllowPrivateAccess = "true"))
    FFootballBallPhysicsParams PhysicsParams;

    // Runtime Physics State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Ball Physics",
              meta = (AllowPrivateAccess = "true"))
    FVector CurrentVelocity = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Ball Physics",
              meta = (AllowPrivateAccess = "true"))
    FVector CurrentAngularVelocity = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Ball Physics",
              meta = (AllowPrivateAccess = "true"))
    TEnumAsByte<EFootballBallState::Type> CurrentBallState = EFootballBallState::Free;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Ball Physics",
              meta = (AllowPrivateAccess = "true"))
    AActor* PossessingPlayer = nullptr;

    // Deformation
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Deformation",
              meta = (AllowPrivateAccess = "true"))
    float CurrentDeformation = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Deformation",
              meta = (AllowPrivateAccess = "true"))
    FVector DeformationDirection = FVector::ZeroVector;

    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Components",
              meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UPrimitiveComponent> BallMeshComponent = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Components",
              meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UNiagaraComponent> TrailEffect = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Components",
              meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UNiagaraComponent> SpinEffect = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Components",
              meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UNiagaraComponent> DeformationEffect = nullptr;

    // Audio
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Audio", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<USoundBase> KickSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Audio", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<USoundBase> ImpactSound_Ground = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Audio", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<USoundBase> ImpactSound_Post = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Audio", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<USoundBase> ImpactSound_Player = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Audio", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<USoundBase> WhistleSound = nullptr;

    // Visual Curves
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Visuals", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCurveFloat> DeformationCurve = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Visuals", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCurveFloat> TrailIntensityCurve = nullptr;

    // Deterministic Simulation
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Network", meta = (AllowPrivateAccess = "true"))
    int32 SimulationFrame = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Network", meta = (AllowPrivateAccess = "true"))
    float FixedTimeStep = 1.0f / 60.0f; // 60Hz fixed timestep

    float AccumulatedTime = 0.0f;

    // Ground contact cache
    mutable bool bWasGrounded = false;
    mutable FHitResult LastGroundHit;
};