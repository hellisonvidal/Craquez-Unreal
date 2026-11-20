// Copyright 2024 Craquez. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BallPhysics/FootballBallComponent.h"
#include "FootballBallActor.generated.h"

class UStaticMeshComponent;
class UNiagaraComponent;
class USphereComponent;

/**
 * Football Ball Actor - Wrapper actor for the ball component with visual representation
 */
UCLASS(Blueprintable, BlueprintType)
class FOOTBALLCORE_API AFootballBallActor : public AActor
{
    GENERATED_BODY()

public:
    AFootballBallActor();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Ball Component Access
    UFUNCTION(BlueprintCallable, Category = "Football|Ball")
    UFootballBallComponent* GetBallComponent() const { return BallComponent; }

    // Visual
    UFUNCTION(BlueprintCallable, Category = "Football|Ball")
    void SetBallMesh(UStaticMesh* Mesh);

    UFUNCTION(BlueprintCallable, Category = "Football|Ball")
    void SetBallMaterial(UMaterialInterface* Material);

    // Physics
    UFUNCTION(BlueprintCallable, Category = "Football|Ball")
    void ApplyKick(const FVector& Impulse, const FVector& Spin, AActor* Instigator);

    UFUNCTION(BlueprintCallable, Category = "Football|Ball")
    void ResetBall(const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator);

    // Network
    UFUNCTION(BlueprintCallable, Category = "Football|Network")
    void SetReplicateMovement(bool bReplicate);

protected:
    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Ball", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UFootballBallComponent> BallComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Ball", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UStaticMeshComponent> BallMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Ball", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<USphereComponent> CollisionSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Ball|Effects", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UNiagaraComponent> TrailEffect;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Ball|Effects", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UNiagaraComponent> SpinEffect;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Ball|Effects", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UNiagaraComponent> ImpactEffect;

    // Ball Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Ball", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UStaticMesh> DefaultBallMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Ball", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UMaterialInterface> DefaultBallMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Ball", meta = (AllowPrivateAccess = "true"))
    FFootballBallPhysicsParams PhysicsParams;

private:
    void SetupComponents();
    void InitializeEffects();
};