// Copyright 2024 Craquez. All Rights Reserved.

#include "BallPhysics/FootballBallActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

AFootballBallActor::AFootballBallActor()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PrePhysics;

    bReplicates = true;
    SetReplicateMovement(true);
    SetNetUpdateFrequency(60.0f);
    SetMinNetUpdateFrequency(30.0f);

    SetupComponents();
}

void AFootballBallActor::BeginPlay()
{
    Super::BeginPlay();

    InitializeEffects();

    if (BallComponent)
    {
        BallComponent->InitializeBall(PhysicsParams);

        // Bind events
        BallComponent->OnBallKicked.AddDynamic(this, &AFootballBallActor::OnBallKicked);
        BallComponent->OnBallCollision.AddDynamic(this, &AFootballBallActor::OnBallCollision);
        BallComponent->OnBallGoalScored.AddDynamic(this, &AFootballBallActor::OnBallGoalScored);
        BallComponent->OnBallOutOfPlay.AddDynamic(this, &AFootballBallActor::OnBallOutOfPlay);
        BallComponent->OnBallStateChanged.AddDynamic(this, &AFootballBallActor::OnBallStateChanged);
    }

    // Set initial mesh and material
    if (DefaultBallMesh && BallMesh)
    {
        BallMesh->SetStaticMesh(DefaultBallMesh);
    }
    if (DefaultBallMaterial && BallMesh)
    {
        BallMesh->SetMaterial(0, DefaultBallMaterial);
    }
}

void AFootballBallActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Sync visual mesh with ball component
    if (BallComponent && BallMesh)
    {
        BallMesh->SetWorldLocationAndRotation(BallComponent->GetComponentLocation(), BallComponent->GetComponentQuat());
    }
}

void AFootballBallActor::SetupComponents()
{
    // Ball Component (physics simulation)
    BallComponent = CreateDefaultSubobject<UFootballBallComponent>(TEXT("BallComponent"));
    SetRootComponent(BallComponent);

    // Visual Mesh - load imported Meshy Craquez soccer ball with uniform scale (zero distortion)
    BallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BallMesh"));
    BallMesh->SetupAttachment(BallComponent);
    BallMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    BallMesh->SetCastShadow(true);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CraquezBallMeshAsset(
        TEXT("/Game/Assets/Ball/SM_CraquezBall.SM_CraquezBall"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> CraquezBallMatAsset(
        TEXT("/Game/Materials/M_CraquezBall.M_CraquezBall"));

    if (CraquezBallMeshAsset.Succeeded())
    {
        DefaultBallMesh = CraquezBallMeshAsset.Object;
        BallMesh->SetStaticMesh(DefaultBallMesh);
        // Uniform scale matching FIFA ball visibility (~30cm on 190cm Meshy export) without distortion
        BallMesh->SetRelativeScale3D(FVector(0.16f));
    }
    else
    {
        static ConstructorHelpers::FObjectFinder<UStaticMesh> FallbackSphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
        if (FallbackSphere.Succeeded())
        {
            DefaultBallMesh = FallbackSphere.Object;
            BallMesh->SetStaticMesh(DefaultBallMesh);
            BallMesh->SetRelativeScale3D(FVector(0.28f));
        }
    }

    if (CraquezBallMatAsset.Succeeded())
    {
        DefaultBallMaterial = CraquezBallMatAsset.Object;
        BallMesh->SetMaterial(0, DefaultBallMaterial);
    }

    // Collision Sphere (for physics interactions)
    CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
    CollisionSphere->SetupAttachment(BallComponent);
    CollisionSphere->SetSphereRadius(11.0f); // 11cm radius
    CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    CollisionSphere->SetCollisionObjectType(ECC_WorldDynamic);
    CollisionSphere->SetCollisionResponseToAllChannels(ECR_Block);
    CollisionSphere->SetSimulatePhysics(false); // We simulate manually

    // Niagara Effects
    TrailEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TrailEffect"));
    TrailEffect->SetupAttachment(BallComponent);
    TrailEffect->SetActive(false);
    TrailEffect->bAutoActivate = false;

    SpinEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("SpinEffect"));
    SpinEffect->SetupAttachment(BallComponent);
    SpinEffect->SetActive(false);
    SpinEffect->bAutoActivate = false;

    ImpactEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ImpactEffect"));
    ImpactEffect->SetupAttachment(BallComponent);
    ImpactEffect->SetActive(false);
    ImpactEffect->bAutoActivate = false;

    // Assign effects to ball component
    if (BallComponent)
    {
        BallComponent->SetEffectComponents(TrailEffect, SpinEffect, ImpactEffect);
    }
}

void AFootballBallActor::InitializeEffects()
{
    // Load default Niagara effects if not set
    // These would be set in the editor or via Blueprints
}

void AFootballBallActor::SetBallMesh(UStaticMesh* Mesh)
{
    if (Mesh && BallMesh)
    {
        BallMesh->SetStaticMesh(Mesh);
        DefaultBallMesh = Mesh;
    }
}

void AFootballBallActor::SetBallMaterial(UMaterialInterface* Material)
{
    if (Material && BallMesh)
    {
        BallMesh->SetMaterial(0, Material);
        DefaultBallMaterial = Material;
    }
}

void AFootballBallActor::ApplyKick(const FVector& Impulse, const FVector& Spin, AActor* KickInstigator)
{
    if (BallComponent)
    {
        BallComponent->ApplyKick(Impulse, Spin, KickInstigator);
    }
}

void AFootballBallActor::ResetBall(const FVector& Location, const FRotator& Rotation)
{
    if (BallComponent)
    {
        BallComponent->ResetBall(Location, Rotation);
    }
    SetActorLocationAndRotation(Location, Rotation);
}

void AFootballBallActor::OnBallKicked(const FVector& Impulse, const FVector& Spin, AActor* KickInstigator)
{
    // Visual feedback for kick
    if (ImpactEffect)
    {
        ImpactEffect->SetWorldLocation(GetActorLocation());
        ImpactEffect->SetVariableVec3("ImpactDirection", -Impulse.GetSafeNormal());
        ImpactEffect->SetVariableFloat("ImpactStrength", Impulse.Size() / 1000.0f);
        ImpactEffect->Activate(true);
    }
}

void AFootballBallActor::OnBallCollision(const FHitResult& HitResult, AActor* OtherActor)
{
    // Impact effects handled by ball component
}

void AFootballBallActor::OnBallGoalScored(EFootballTeamSide::Type ScoringTeam)
{
    // Goal celebration effects
    // Could spawn confetti, trigger crowd cheer, etc.
}

void AFootballBallActor::OnBallOutOfPlay(EFootballTeamSide::Type LastTouchTeam)
{
    // Out of play effects
}

void AFootballBallActor::OnBallStateChanged(EFootballBallState::Type NewState)
{
    // State change visual feedback
    switch (NewState)
    {
    case EFootballBallState::Flying:
        if (TrailEffect)
        {
            TrailEffect->Activate(true);
        }
        break;
    case EFootballBallState::Rolling:
        if (TrailEffect)
        {
            TrailEffect->Deactivate();
        }
        break;
    case EFootballBallState::Free:
    case EFootballBallState::Possessed:
        if (TrailEffect)
        {
            TrailEffect->Deactivate();
        }
        if (SpinEffect)
        {
            SpinEffect->Deactivate();
        }
        break;
    default:
        break;
    }
}