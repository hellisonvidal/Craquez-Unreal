// Copyright 2024 Craquez. All Rights Reserved.

#include "BallPhysics/FootballBallComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Character.h"
#include "Sound/SoundCue.h"

UFootballBallComponent::UFootballBallComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    SetIsReplicatedByDefault(true);
}

void UFootballBallComponent::BeginPlay()
{
    Super::BeginPlay();

    // Find or create ball mesh component
    AActor* Owner = GetOwner();
    if (Owner)
    {
        BallMeshComponent = Owner->FindComponentByClass<UPrimitiveComponent>();
        if (!BallMeshComponent)
        {
            // Create a sphere component for physics
            BallMeshComponent = NewObject<USphereComponent>(Owner, TEXT("BallSphere"));
            BallMeshComponent->RegisterComponent();
            BallMeshComponent->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
            BallMeshComponent->SetSphereRadius(PhysicsParams.Radius * 100.0f); // Convert to cm
            BallMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            BallMeshComponent->SetCollisionObjectType(ECC_WorldDynamic);
            BallMeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
            BallMeshComponent->SetSimulatePhysics(false); // We simulate manually for determinism
            BallMeshComponent->SetMassOverrideInKg(NAME_None, PhysicsParams.Mass);
        }
    }

    // Initialize Niagara effects
    if (TrailEffect)
    {
        TrailEffect->SetActive(false);
    }
    if (SpinEffect)
    {
        SpinEffect->SetActive(false);
    }
    if (DeformationEffect)
    {
        DeformationEffect->SetActive(false);
    }

    // Set initial state
    TransitionToState(EFootballBallState::Free);
}

void UFootballBallComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!BallMeshComponent)
    {
        return;
    }

    // Fixed timestep simulation for determinism
    AccumulatedTime += DeltaTime;
    const int32 MaxSubSteps = 4;
    int32 SubSteps = 0;

    while (AccumulatedTime >= FixedTimeStep && SubSteps < MaxSubSteps)
    {
        SimulatePhysics(FixedTimeStep);
        AccumulatedTime -= FixedTimeStep;
        SimulationFrame++;
        SubSteps++;
    }

    // Interpolate for smooth rendering
    if (AccumulatedTime > 0.0f && SubSteps > 0)
    {
        float Alpha = AccumulatedTime / FixedTimeStep;
        // Render interpolation would go here
    }

    UpdateVisuals(DeltaTime);
    UpdateNiagaraParameters();
}

void UFootballBallComponent::OnRegister()
{
    Super::OnRegister();
}

void UFootballBallComponent::InitializeBall(const FFootballBallPhysicsParams& InParams)
{
    PhysicsParams = InParams;

    if (BallMeshComponent)
    {
        BallMeshComponent->SetMassOverrideInKg(NAME_None, PhysicsParams.Mass);
        if (USphereComponent* SphereComp = Cast<USphereComponent>(BallMeshComponent))
        {
            SphereComp->SetSphereRadius(PhysicsParams.Radius * 100.0f);
        }
    }
}

void UFootballBallComponent::ApplyKick(const FVector& Impulse, const FVector& Spin, AActor* Instigator)
{
    if (CurrentBallState == EFootballBallState::Possessed || CurrentBallState == EFootballBallState::Free)
    {
        CurrentVelocity = Impulse / PhysicsParams.Mass;
        CurrentAngularVelocity = Spin;

        // Clamp spin to max
        float SpinMagnitude = CurrentAngularVelocity.Size();
        if (SpinMagnitude > PhysicsParams.MaxSpinRate)
        {
            CurrentAngularVelocity = CurrentAngularVelocity.GetSafeNormal() * PhysicsParams.MaxSpinRate;
        }

        SetPossessingPlayer(nullptr);
        TransitionToState(EFootballBallState::Flying);

        OnBallKicked.Broadcast(Impulse, Spin, Instigator);

        if (KickSound)
        {
            UGameplayStatics::PlaySoundAtLocation(this, KickSound, GetComponentLocation());
        }

        // Spawn kick effect
        if (DeformationEffect)
        {
            DeformationEffect->SetActive(true);
            DeformationEffect->SetVariableVec3("ImpactDirection", -Impulse.GetSafeNormal());
            DeformationEffect->SetVariableFloat("ImpactStrength", Impulse.Size() / 1000.0f);
        }

        CurrentDeformation = FMath::Clamp(Impulse.Size() / 5000.0f, 0.0f, 1.0f);
        DeformationDirection = -Impulse.GetSafeNormal();
    }
}

void UFootballBallComponent::ApplyImpulseAtLocation(const FVector& Impulse, const FVector& Location)
{
    // Calculate torque from off-center impulse
    FVector CenterOfMass = GetComponentLocation();
    FVector LeverArm = Location - CenterOfMass;
    FVector Torque = FVector::CrossProduct(LeverArm, Impulse);

    CurrentVelocity += Impulse / PhysicsParams.Mass;

    // Simplified moment of inertia for sphere: I = 2/5 * m * r^2
    float MomentOfInertia = 0.4f * PhysicsParams.Mass * FMath::Square(PhysicsParams.Radius);
    CurrentAngularVelocity += Torque / MomentOfInertia;

    // Clamp spin
    float SpinMagnitude = CurrentAngularVelocity.Size();
    if (SpinMagnitude > PhysicsParams.MaxSpinRate)
    {
        CurrentAngularVelocity = CurrentAngularVelocity.GetSafeNormal() * PhysicsParams.MaxSpinRate;
    }
}

void UFootballBallComponent::SetBallState(EFootballBallState::Type NewState)
{
    TransitionToState(NewState);
}

void UFootballBallComponent::ResetBall(const FVector& Location, const FRotator& Rotation)
{
    SetWorldLocationAndRotation(Location, Rotation.Quaternion());
    CurrentVelocity = FVector::ZeroVector;
    CurrentAngularVelocity = FVector::ZeroVector;
    CurrentDeformation = 0.0f;
    DeformationDirection = FVector::ZeroVector;
    SetPossessingPlayer(nullptr);
    TransitionToState(EFootballBallState::Free);
    SimulationFrame = 0;
    AccumulatedTime = 0.0f;
}

void UFootballBallComponent::SetPossessingPlayer(AActor* Player)
{
    PossessingPlayer = Player;
    if (Player)
    {
        TransitionToState(EFootballBallState::Possessed);
    }
    else if (CurrentBallState == EFootballBallState::Possessed)
    {
        TransitionToState(EFootballBallState::Free);
    }
}

void UFootballBallComponent::SetDeterministicState(const FFootballFixedVector& Position, const FFootballFixedVector& Velocity, const FFootballFixedVector& AngularVelocity, int32 FrameNumber)
{
    SetWorldLocation(Position.ToVector());
    CurrentVelocity = Velocity.ToVector();
    CurrentAngularVelocity = AngularVelocity.ToVector();
    SimulationFrame = FrameNumber;
}

void UFootballBallComponent::GetDeterministicState(FFootballFixedVector& OutPosition, FFootballFixedVector& OutVelocity, FFootballFixedVector& OutAngularVelocity, int32& OutFrameNumber) const
{
    OutPosition = FFootballFixedVector::FromVector(GetComponentLocation());
    OutVelocity = FFootballFixedVector::FromVector(CurrentVelocity);
    OutAngularVelocity = FFootballFixedVector::FromVector(CurrentAngularVelocity);
    OutFrameNumber = SimulationFrame;
}

void UFootballBallComponent::SimulatePhysics(float DeltaTime)
{
    if (CurrentBallState == EFootballBallState::Possessed)
    {
        // Ball follows possessing player
        if (PossessingPlayer)
        {
            FVector PlayerLocation = PossessingPlayer->GetActorLocation();
            FVector Offset = FVector(0, 0, 100.0f); // Adjust based on player
            SetWorldLocation(PlayerLocation + Offset);
        }
        return;
    }

    // Apply forces
    ApplyAerodynamicForces(DeltaTime);
    ApplyMagnusForce(DeltaTime);
    ApplyGroundInteraction(DeltaTime);
    UpdateDeformation(DeltaTime);
    CheckCollisions(DeltaTime);

    // Integrate velocity
    FVector NewLocation = GetComponentLocation() + CurrentVelocity * DeltaTime * 100.0f; // Convert to cm
    SetWorldLocation(NewLocation);

    // Integrate rotation from angular velocity
    FQuat CurrentRotation = GetComponentQuat();
    FVector AngularDelta = CurrentAngularVelocity * DeltaTime;
    FQuat RotationDelta = FQuat(AngularDelta);
    SetWorldRotation(CurrentRotation * RotationDelta);

    // Update mesh component if separate
    if (BallMeshComponent && BallMeshComponent != this)
    {
        BallMeshComponent->SetWorldLocationAndRotation(NewLocation, GetComponentQuat());
    }
}

void UFootballBallComponent::ApplyAerodynamicForces(float DeltaTime)
{
    if (CurrentVelocity.IsNearlyZero())
    {
        return;
    }

    FVector VelocityDir = CurrentVelocity.GetSafeNormal();
    float Speed = CurrentVelocity.Size();
    float SpeedSq = Speed * Speed;

    // Cross-sectional area
    float Area = PI * FMath::Square(PhysicsParams.Radius);

    // Drag Force: F_d = 0.5 * rho * v^2 * Cd * A
    float DragForce = 0.5f * PhysicsParams.AirDensity * SpeedSq * PhysicsParams.DragCoefficient * Area;
    FVector DragAcceleration = -VelocityDir * DragForce / PhysicsParams.Mass;

    CurrentVelocity += DragAcceleration * DeltaTime;
}

void UFootballBallComponent::ApplyMagnusForce(float DeltaTime)
{
    if (!PhysicsParams.bUseMagnusEffect || CurrentAngularVelocity.IsNearlyZero() || CurrentVelocity.IsNearlyZero())
    {
        return;
    }

    // Magnus Force: F_m = 0.5 * rho * v^2 * Cl * A * (omega x v) / (|omega| * |v|)
    // Simplified: F_m = S * (omega x v) where S = 0.5 * rho * Cl * A * r / m

    FVector SpinAxis = CurrentAngularVelocity.GetSafeNormal();
    float SpinRate = CurrentAngularVelocity.Size();
    float Speed = CurrentVelocity.Size();

    // Magnus coefficient
    float S = 0.5f * PhysicsParams.AirDensity * PhysicsParams.LiftCoefficient * PI * FMath::Square(PhysicsParams.Radius) * PhysicsParams.Radius / PhysicsParams.Mass;

    // Magnus force is perpendicular to both spin axis and velocity
    FVector MagnusDir = FVector::CrossProduct(SpinAxis, CurrentVelocity.GetSafeNormal());
    float MagnusMagnitude = S * SpinRate * Speed * Speed;

    FVector MagnusAcceleration = MagnusDir * MagnusMagnitude;
    CurrentVelocity += MagnusAcceleration * DeltaTime;
}

void UFootballBallComponent::ApplyGroundInteraction(float DeltaTime)
{
    FHitResult GroundHit;
    bool bGrounded = CheckGroundContact(GroundHit);

    if (bGrounded)
    {
        bWasGrounded = true;
        LastGroundHit = GroundHit;

        FVector GroundNormal = GroundHit.Normal;
        float PenetrationDepth = GroundHit.PenetrationDepth;

        // Resolve penetration
        if (PenetrationDepth > 0.0f)
        {
            FVector Correction = GroundNormal * PenetrationDepth * 1.1f;
            AddWorldOffset(Correction * 100.0f); // Convert to cm
        }

        // Calculate relative velocity at contact point
        FVector ContactVelocity = CurrentVelocity + FVector::CrossProduct(CurrentAngularVelocity, -GroundNormal * PhysicsParams.Radius);
        float NormalVelocity = FVector::DotProduct(ContactVelocity, GroundNormal);

        // Only apply collision response if moving toward ground
        if (NormalVelocity < -1.0f)
        {
            // Restitution (bounce)
            FVector NormalImpulse = -GroundNormal * NormalVelocity * (1.0f + PhysicsParams.Restitution);
            CurrentVelocity += NormalImpulse;

            // Friction
            FVector TangentialVelocity = ContactVelocity - GroundNormal * NormalVelocity;
            float TangentialSpeed = TangentialVelocity.Size();

            if (TangentialSpeed > 1.0f)
            {
                FVector FrictionDir = -TangentialVelocity.GetSafeNormal();

                // Determine if rolling or sliding
                FVector RollingVelocity = FVector::CrossProduct(CurrentAngularVelocity, -GroundNormal * PhysicsParams.Radius);
                float SlipSpeed = (TangentialVelocity - RollingVelocity).Size();

                float FrictionCoeff = (SlipSpeed < 10.0f) ? PhysicsParams.RollingFriction : PhysicsParams.SlidingFriction;
                float FrictionForce = FrictionCoeff * FMath::Abs(NormalVelocity) * PhysicsParams.Mass;
                FVector FrictionImpulse = FrictionDir * FMath::Min(FrictionForce * DeltaTime, TangentialSpeed) / PhysicsParams.Mass;

                CurrentVelocity += FrictionImpulse;

                // Torque from friction
                FVector Torque = FVector::CrossProduct(-GroundNormal * PhysicsParams.Radius, FrictionImpulse * PhysicsParams.Mass);
                float MomentOfInertia = 0.4f * PhysicsParams.Mass * FMath::Square(PhysicsParams.Radius);
                CurrentAngularVelocity += Torque / MomentOfInertia;
            }

            // Update state
            if (CurrentVelocity.Size() < 50.0f) // 0.5 m/s
            {
                TransitionToState(EFootballBallState::Rolling);
            }
            else
            {
                TransitionToState(EFootballBallState::Flying);
            }

            // Play impact sound/effect
            PlayImpactSound(GroundHit);
            SpawnImpactEffect(GroundHit);
        }
        else if (CurrentVelocity.Size() < 10.0f)
        {
            // Ball is essentially at rest on ground
            CurrentVelocity = FVector::ZeroVector;
            CurrentAngularVelocity = FVector::ZeroVector;
            TransitionToState(EFootballBallState::Free);
        }
    }
    else
    {
        bWasGrounded = false;

        if (CurrentBallState == EFootballBallState::Rolling)
        {
            TransitionToState(EFootballBallState::Flying);
        }
    }
}

void UFootballBallComponent::UpdateDeformation(float DeltaTime)
{
    if (!PhysicsParams.bUseDeformation)
    {
        CurrentDeformation = FMath::FInterpTo(CurrentDeformation, 0.0f, DeltaTime, 10.0f);
        return;
    }

    // Deformation recovers over time
    CurrentDeformation = FMath::FInterpTo(CurrentDeformation, 0.0f, DeltaTime, 5.0f);
}

void UFootballBallComponent::CheckCollisions(float DeltaTime)
{
    if (!BallMeshComponent)
    {
        return;
    }

    // Sweep test for continuous collision detection
    FVector Start = GetComponentLocation();
    FVector End = Start + CurrentVelocity * DeltaTime * 100.0f;

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    QueryParams.bReturnPhysicalMaterial = true;
    QueryParams.bReturnFaceIndex = true;

    FHitResult HitResult;
    bool bHit = GetWorld()->SweepSingleByChannel(
        HitResult,
        Start,
        End,
        FQuat::Identity,
        ECC_WorldDynamic,
        FCollisionShape::MakeSphere(PhysicsParams.Radius * 100.0f),
        QueryParams
    );

    if (bHit)
    {
        HandleCollision(HitResult);
    }
}

void UFootballBallComponent::HandleCollision(const FHitResult& HitResult)
{
    AActor* OtherActor = HitResult.GetActor();
    if (!OtherActor || OtherActor == GetOwner())
    {
        return;
    }

    // Determine collision type and respond
    UPhysicalMaterial* PhysMat = HitResult.PhysMaterial.Get();
    FVector ImpactNormal = HitResult.Normal;
    FVector ImpactPoint = HitResult.Location;

    // Calculate impact velocity
    FVector RelativeVelocity = CurrentVelocity;
    if (ACharacter* Character = Cast<ACharacter>(OtherActor))
    {
        RelativeVelocity -= Character->GetVelocity();
    }

    float ImpactSpeed = FVector::DotProduct(-RelativeVelocity, ImpactNormal);

    if (ImpactSpeed > 100.0f) // Significant impact
    {
        // Apply impulse response
        float Restitution = PhysicsParams.Restitution;
        if (PhysMat)
        {
            Restitution *= PhysMat->Restitution;
        }

        FVector Impulse = ImpactNormal * ImpactSpeed * (1.0f + Restitution) * PhysicsParams.Mass;
        CurrentVelocity += Impulse / PhysicsParams.Mass;

        // Add spin from glancing blows
        FVector TangentialVel = RelativeVelocity - ImpactNormal * FVector::DotProduct(RelativeVelocity, ImpactNormal);
        if (TangentialVel.Size() > 50.0f)
        {
            FVector Torque = FVector::CrossProduct(ImpactNormal * PhysicsParams.Radius, TangentialVel * PhysicsParams.Mass * 0.1f);
            float MomentOfInertia = 0.4f * PhysicsParams.Mass * FMath::Square(PhysicsParams.Radius);
            CurrentAngularVelocity += Torque / MomentOfInertia;
        }

        // Deformation
        CurrentDeformation = FMath::Clamp(ImpactSpeed / 2000.0f, 0.0f, 1.0f);
        DeformationDirection = -ImpactNormal;

        // Check for goal
        if (OtherActor->ActorHasTag("Goal"))
        {
            EFootballTeamSide::Type ScoringTeam = EFootballTeamSide::Home; // Determine from goal
            OnBallGoalScored.Broadcast(ScoringTeam);
            TransitionToState(EFootballBallState::Goal);
        }
        // Check for out of play
        else if (OtherActor->ActorHasTag("OutOfBounds"))
        {
            EFootballTeamSide::Type LastTouch = EFootballTeamSide::Home; // Determine from last touch
            OnBallOutOfPlay.Broadcast(LastTouch);
            TransitionToState(EFootballBallState::OutOfPlay);
        }

        // Broadcast collision event
        OnBallCollision.Broadcast(HitResult, OtherActor);

        // Sound and effects
        PlayImpactSound(HitResult);
        SpawnImpactEffect(HitResult);
    }
}

void UFootballBallComponent::TransitionToState(EFootballBallState::Type NewState)
{
    if (CurrentBallState != NewState && CanTransitionTo(NewState))
    {
        CurrentBallState = NewState;
        OnBallStateChanged.Broadcast(NewState);
    }
}

bool UFootballBallComponent::CanTransitionTo(EFootballBallState::Type NewState) const
{
    // Define valid state transitions
    switch (CurrentBallState)
    {
        case EFootballBallState::Free:
            return NewState == EFootballBallState::Possessed ||
                   NewState == EFootballBallState::Flying ||
                   NewState == EFootballBallState::Rolling ||
                   NewState == EFootballBallState::OutOfPlay;

        case EFootballBallState::Possessed:
            return NewState == EFootballBallState::Free ||
                   NewState == EFootballBallState::Flying ||
                   NewState == EFootballBallState::OutOfPlay;

        case EFootballBallState::Flying:
            return NewState == EFootballBallState::Rolling ||
                   NewState == EFootballBallState::Free ||
                   NewState == EFootballBallState::Possessed ||
                   NewState == EFootballBallState::Goal ||
                   NewState == EFootballBallState::OutOfPlay;

        case EFootballBallState::Rolling:
            return NewState == EFootballBallState::Free ||
                   NewState == EFootballBallState::Possessed ||
                   NewState == EFootballBallState::Flying ||
                   NewState == EFootballBallState::Goal ||
                   NewState == EFootballBallState::OutOfPlay;

        case EFootballBallState::Goal:
        case EFootballBallState::OutOfPlay:
            return NewState == EFootballBallState::Free;

        default:
            return true;
    }
}

bool UFootballBallComponent::CheckGroundContact(FHitResult& OutHitResult) const
{
    FVector Start = GetComponentLocation();
    FVector End = Start - FVector(0, 0, PhysicsParams.Radius * 100.0f + 5.0f); // 5cm extra

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    QueryParams.bReturnPhysicalMaterial = true;

    return GetWorld()->LineTraceSingleByChannel(
        OutHitResult,
        Start,
        End,
        ECC_WorldStatic,
        QueryParams
    );
}

FVector UFootballBallComponent::CalculateGroundVelocity(const FHitResult& HitResult) const
{
    // For moving platforms, etc.
    return FVector::ZeroVector;
}

void UFootballBallComponent::UpdateVisuals(float DeltaTime)
{
    if (BallMeshComponent && DeformationCurve && PhysicsParams.bUseDeformation)
    {
        float DeformValue = DeformationCurve->GetFloatValue(CurrentDeformation);
        FVector Scale = FVector(1.0f + DeformValue * DeformationDirection.X * 0.1f,
                                1.0f + DeformValue * DeformationDirection.Y * 0.1f,
                                1.0f - DeformValue * 0.15f); // Flatten on Z
        BallMeshComponent->SetRelativeScale3D(Scale);
    }
}

void UFootballBallComponent::PlayImpactSound(const FHitResult& HitResult)
{
    UPhysicalMaterial* PhysMat = HitResult.PhysMaterial.Get();
    USoundBase* SoundToPlay = nullptr;

    if (PhysMat)
    {
        FName SurfaceType = PhysMat->SurfaceType;
        if (SurfaceType == UPhysicalMaterial::GetDefault()->SurfaceType)
        {
            SoundToPlay = ImpactSound_Ground;
        }
        else if (SurfaceType == FName("Player"))
        {
            SoundToPlay = ImpactSound_Player;
        }
        else if (SurfaceType == FName("Metal") || SurfaceType == FName("Wood"))
        {
            SoundToPlay = ImpactSound_Post;
        }
        else
        {
            SoundToPlay = ImpactSound_Ground;
        }
    }
    else
    {
        SoundToPlay = ImpactSound_Ground;
    }

    if (SoundToPlay)
    {
        UGameplayStatics::PlaySoundAtLocation(this, SoundToPlay, HitResult.Location);
    }
}

void UFootballBallComponent::SpawnImpactEffect(const FHitResult& HitResult)
{
    // Niagara effects would be spawned here
    // For now, we use the existing components
    if (DeformationEffect)
    {
        DeformationEffect->SetWorldLocation(HitResult.Location);
        DeformationEffect->SetVariableVec3("ImpactNormal", HitResult.Normal);
        DeformationEffect->SetActive(true);
    }
}

void UFootballBallComponent::UpdateNiagaraParameters()
{
    if (TrailEffect)
    {
        bool bShowTrail = CurrentVelocity.Size() > 500.0f; // 5 m/s
        TrailEffect->SetActive(bShowTrail);

        if (bShowTrail && TrailIntensityCurve)
        {
            float Intensity = TrailIntensityCurve->GetFloatValue(FMath::Clamp(CurrentVelocity.Size() / 3000.0f, 0.0f, 1.0f));
            TrailEffect->SetVariableFloat("TrailIntensity", Intensity);
        }
    }

    if (SpinEffect)
    {
        float SpinRate = CurrentAngularVelocity.Size();
        bool bShowSpin = SpinRate > 5.0f; // 5 rad/s
        SpinEffect->SetActive(bShowSpin);

        if (bShowSpin)
        {
            SpinEffect->SetVariableFloat("SpinRate", FMath::Clamp(SpinRate / PhysicsParams.MaxSpinRate, 0.0f, 1.0f));
            SpinEffect->SetVariableVec3("SpinAxis", CurrentAngularVelocity.GetSafeNormal());
        }
    }
}