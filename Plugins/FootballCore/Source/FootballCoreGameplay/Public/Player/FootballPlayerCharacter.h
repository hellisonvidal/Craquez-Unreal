// Copyright 2024 Craquez. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameplayTagContainer.h"
#include "InputActionValue.h"
#include "FootballCoreTypes.h"
#include "FootballPlayerCharacter.generated.h"

class UFootballBallComponent;
class UFootballTacticalComponent;
class UControlRigComponent;
class UAnimInstance;
class USkeletalMeshComponent;
class UAnimMontage;
class UCurveFloat;
class UInputAction;
class UInputMappingContext;
class UStaticMeshComponent;
class USpringArmComponent;
class UCameraComponent;
class UMaterialInterface;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerPossessionChanged, AActor*, Player, UFootballBallComponent*,
                                             Ball);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPlayerActionExecuted, FGameplayTag, ActionTag, bool, bSuccess,
                                               AActor*, Target);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerStaminaChanged, float, CurrentStamina);

/**
 * Football Player Character - Base class for all players with:
 * - Procedural animation (Control Rig, IK, Motion Matching)
 * - Ball possession and control
 * - Skill moves and actions
 * - Physical attributes
 * - Tactical awareness
 * - Stamina and fatigue system
 */
UCLASS(Blueprintable, BlueprintType)
class FOOTBALLCOREGAMEPLAY_API AFootballPlayerCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AFootballPlayerCharacter();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
    virtual void PossessedBy(AController* NewController) override;
    virtual void UnPossessed() override;

    // Ball Interaction
    UFUNCTION(BlueprintCallable, Category = "Football|Ball")
    void RequestPossession(UFootballBallComponent* Ball);

    UFUNCTION(BlueprintCallable, Category = "Football|Ball")
    void ReleasePossession();

    UFUNCTION(BlueprintCallable, Category = "Football|Ball")
    void KickBall(const FVector& Direction, float Power, float Spin, FGameplayTag KickType);

    UFUNCTION(BlueprintCallable, Category = "Football|Ball")
    void PassBall(AActor* TargetPlayer, float Power, FGameplayTag PassType);

    UFUNCTION(BlueprintCallable, Category = "Football|Ball")
    void ShootAtGoal(const FVector& TargetLocation, float Power, float Placement, FGameplayTag ShotType);

    UFUNCTION(BlueprintCallable, Category = "Football|Ball")
    void HeaderBall(const FVector& Direction, float Power);

    UFUNCTION(BlueprintCallable, Category = "Football|Ball")
    void ChestControl();

    UFUNCTION(BlueprintCallable, Category = "Football|Ball")
    void TrapBall();

    UFUNCTION(BlueprintCallable, Category = "Football|Ball")
    void SkillMove(FGameplayTag SkillTag);

    UFUNCTION(BlueprintCallable, Category = "Football|Ball")
    void Tackle(FGameplayTag TackleType);

    UFUNCTION(BlueprintCallable, Category = "Football|Ball")
    void Intercept();

    UFUNCTION(BlueprintCallable, Category = "Football|Ball")
    void BlockShot();

    UFUNCTION(BlueprintCallable, Category = "Football|Ball")
    void Clearance();

    // Goalkeeper Specific
    UFUNCTION(BlueprintCallable, Category = "Football|Goalkeeper")
    void GoalkeeperDive(FVector Direction);

    UFUNCTION(BlueprintCallable, Category = "Football|Goalkeeper")
    void GoalkeeperCatch();

    UFUNCTION(BlueprintCallable, Category = "Football|Goalkeeper")
    void GoalkeeperPunch();

    UFUNCTION(BlueprintCallable, Category = "Football|Goalkeeper")
    void GoalkeeperDistributeThrow(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Football|Goalkeeper")
    void GoalkeeperDistributeKick(FVector Direction, float Power);

    // Movement
    UFUNCTION(BlueprintCallable, Category = "Football|Movement")
    void SetMovementMode(EFootballMovementMode::Type Mode);

    UFUNCTION(BlueprintCallable, Category = "Football|Movement")
    void StartSprint();

    UFUNCTION(BlueprintCallable, Category = "Football|Movement")
    void StopSprint();

    UFUNCTION(BlueprintCallable, Category = "Football|Movement")
    void StartJog();

    UFUNCTION(BlueprintCallable, Category = "Football|Movement")
    void StopJog();

    // Stamina
    UFUNCTION(BlueprintCallable, Category = "Football|Stamina")
    void ConsumeStamina(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Football|Stamina")
    void RecoverStamina(float DeltaTime);

    UFUNCTION(BlueprintPure, Category = "Football|Stamina")
    float GetStaminaPercent() const
    {
        return Attributes.Stamina > 0 ? CurrentStamina / Attributes.Stamina * 100.0f : 0.0f;
    }

    UFUNCTION(BlueprintPure, Category = "Football|Stamina")
    bool IsFatigued() const { return bIsFatigued; }

    // Attributes
    UFUNCTION(BlueprintCallable, Category = "Football|Attributes")
    void SetAttributes(const FFootballPlayerAttributes& NewAttributes);

    UFUNCTION(BlueprintPure, Category = "Football|Attributes")
    FFootballPlayerAttributes GetAttributes() const { return Attributes; }

    UFUNCTION(BlueprintPure, Category = "Football|Attributes")
    float GetAttributeValue(FGameplayTag AttributeTag) const;

    // Role and Team
    UFUNCTION(BlueprintCallable, Category = "Football|Team")
    void SetPlayerRole(EFootballPlayerRole::Type NewRole);

    UFUNCTION(BlueprintCallable, Category = "Football|Team")
    void SetTeam(AFootballTeam* NewTeam);

    UFUNCTION(BlueprintPure, Category = "Football|Team")
    EFootballPlayerRole::Type GetPlayerRole() const { return PlayerRole; }

    UFUNCTION(BlueprintPure, Category = "Football|Team")
    AFootballTeam* GetTeam() const { return Team; }

    UFUNCTION(BlueprintPure, Category = "Football|Team")
    EFootballTeamSide::Type GetTeamSide() const;

    // Visual Uniform & Controls (Separate uniform meshes + touch/HUD helpers)
    UFUNCTION(BlueprintCallable, Category = "Football|Visuals")
    void ApplyTeamUniform(EFootballTeamSide::Type Side, bool bIsGoalkeeper);

    UFUNCTION(BlueprintCallable, Category = "Football|Visuals")
    void SetActiveControlIndicator(bool bActive);

    UFUNCTION(BlueprintCallable, Category = "Football|Input")
    void TriggerPass();

    UFUNCTION(BlueprintCallable, Category = "Football|Input")
    void TriggerShoot();

    UFUNCTION(BlueprintCallable, Category = "Football|Input")
    void TriggerTackle();

    UFUNCTION(BlueprintCallable, Category = "Football|Input")
    void TriggerToggleSprint();

    // Ball Possession
    UFUNCTION(BlueprintPure, Category = "Football|Ball")
    bool HasBallPossession() const { return CurrentBall != nullptr; }

    UFUNCTION(BlueprintPure, Category = "Football|Ball")
    UFootballBallComponent* GetCurrentBall() const { return CurrentBall; }

    // Procedural Animation
    UFUNCTION(BlueprintCallable, Category = "Football|Animation")
    void UpdateProceduralAnimation(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Football|Animation")
    void SetIKTarget(FName BoneName, FVector TargetLocation, float BlendTime = 0.2f);

    UFUNCTION(BlueprintCallable, Category = "Football|Animation")
    void ClearIKTarget(FName BoneName, float BlendTime = 0.2f);

    UFUNCTION(BlueprintCallable, Category = "Football|Animation")
    void SetLookAtTarget(AActor* Target, float BlendTime = 0.3f);

    UFUNCTION(BlueprintCallable, Category = "Football|Animation")
    void ClearLookAtTarget(float BlendTime = 0.3f);

    UFUNCTION(BlueprintCallable, Category = "Football|Animation")
    void PlayActionMontage(UAnimMontage* Montage, float PlayRate = 1.0f, FName StartSection = NAME_None);

    // Tactical
    UFUNCTION(BlueprintCallable, Category = "Football|Tactical")
    void SetTacticalInstruction(FGameplayTag InstructionTag, float Value);

    UFUNCTION(BlueprintPure, Category = "Football|Tactical")
    float GetTacticalInstruction(FGameplayTag InstructionTag) const;

    // State
    UFUNCTION(BlueprintPure, Category = "Football|State")
    FGameplayTag GetCurrentActionState() const { return CurrentActionState; }

    UFUNCTION(BlueprintCallable, Category = "Football|State")
    void SetCurrentActionState(FGameplayTag NewState) { CurrentActionState = NewState; }

    UFUNCTION(BlueprintPure, Category = "Football|State")
    bool IsPerformingAction() const
    {
        return CurrentActionState.IsValid() && CurrentActionState != FFootballTags::Player_Idle;
    }

    // Network
    UFUNCTION(BlueprintCallable, Category = "Football|Network")
    void Server_SetDeterministicState(const FFootballFixedVector& Position, const FFootballFixedVector& Velocity,
                                      const FFootballFixedRotator& Rotation, int32 FrameNumber);

    UFUNCTION(BlueprintCallable, Category = "Football|Network")
    void GetDeterministicState(FFootballFixedVector& OutPosition, FFootballFixedVector& OutVelocity,
                               FFootballFixedRotator& OutRotation, int32& OutFrameNumber) const;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Football|Events")
    FOnPlayerPossessionChanged OnPossessionChanged;

    UPROPERTY(BlueprintAssignable, Category = "Football|Events")
    FOnPlayerActionExecuted OnActionExecuted;

    UPROPERTY(BlueprintAssignable, Category = "Football|Events")
    FOnPlayerStaminaChanged OnStaminaChanged;

protected:
    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Components",
              meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UFootballTacticalComponent> TacticalComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Components",
              meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UControlRigComponent> ControlRigComponent;

    // Broadcast / Action Camera
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Camera", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<USpringArmComponent> CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Camera", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCameraComponent> FollowCamera;

    // Visual Player Body + Separate Uniform Meshes (Jersey/Shorts separate from body)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Visuals", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UStaticMeshComponent> HeadMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Visuals", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UStaticMeshComponent> JerseyMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Visuals", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UStaticMeshComponent> ShortsMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Visuals", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UStaticMeshComponent> LeftLegMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Visuals", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UStaticMeshComponent> RightLegMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Visuals", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UStaticMeshComponent> LeftBootMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Visuals", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UStaticMeshComponent> RightBootMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Visuals", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UStaticMeshComponent> ActiveRingMesh;

    // Cached Uniform Materials
    UPROPERTY()
    TObjectPtr<UMaterialInterface> MatUniformHome = nullptr;

    UPROPERTY()
    TObjectPtr<UMaterialInterface> MatShortsHome = nullptr;

    UPROPERTY()
    TObjectPtr<UMaterialInterface> MatUniformAway = nullptr;

    UPROPERTY()
    TObjectPtr<UMaterialInterface> MatShortsAway = nullptr;

    UPROPERTY()
    TObjectPtr<UMaterialInterface> MatUniformGKHome = nullptr;

    UPROPERTY()
    TObjectPtr<UMaterialInterface> MatUniformGKAway = nullptr;

    UPROPERTY()
    TObjectPtr<UMaterialInterface> MatPlayerSkin = nullptr;

    UPROPERTY()
    TObjectPtr<UMaterialInterface> MatPlayerBoots = nullptr;

    UPROPERTY()
    TObjectPtr<UMaterialInterface> MatActiveRing = nullptr;

    // Ball
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Ball", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UFootballBallComponent> CurrentBall = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Ball", meta = (AllowPrivateAccess = "true"))
    float PossessionRange = 150.0f; // cm

    // Attributes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Attributes", meta = (AllowPrivateAccess = "true"))
    FFootballPlayerAttributes Attributes;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Attributes",
              meta = (AllowPrivateAccess = "true"))
    float CurrentStamina = 100.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Attributes",
              meta = (AllowPrivateAccess = "true"))
    bool bIsFatigued = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Attributes",
              meta = (AllowPrivateAccess = "true"))
    float FatigueRecoveryRate = 5.0f; // per second

    // Role
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Role", meta = (AllowPrivateAccess = "true"))
    TEnumAsByte<EFootballPlayerRole::Type> PlayerRole = EFootballPlayerRole::CentralMidfielder;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Role", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<AFootballTeam> Team = nullptr;

    // Action State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|State", meta = (AllowPrivateAccess = "true"))
    FGameplayTag CurrentActionState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|State", meta = (AllowPrivateAccess = "true"))
    float ActionCooldown = 0.0f;

    // Animation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Animation", meta = (AllowPrivateAccess = "true"))
    TMap<FName, FVector> IKTargets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Animation", meta = (AllowPrivateAccess = "true"))
    TMap<FName, float> IKBlendWeights;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Animation", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<AActor> LookAtTarget = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Animation", meta = (AllowPrivateAccess = "true"))
    float LookAtBlendWeight = 0.0f;

    // Motion Matching
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|MotionMatching",
              meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UAnimSequence> MotionMatchingDatabase = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|MotionMatching",
              meta = (AllowPrivateAccess = "true"))
    int32 MotionMatchingFrameWindow = 30;

    // Input
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Input", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UInputMappingContext> FootballInputMapping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Input", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UInputAction> MoveAction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Input", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UInputAction> LookAction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Input", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UInputAction> SprintAction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Input", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UInputAction> PassAction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Input", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UInputAction> ShootAction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Input", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UInputAction> TackleAction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Input", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UInputAction> SkillMoveAction;

    // Action Montages
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Animation|Montages",
              meta = (AllowPrivateAccess = "true"))
    TMap<FGameplayTag, TObjectPtr<UAnimMontage>> ActionMontages;

    // Movement
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Movement", meta = (AllowPrivateAccess = "true"))
    TEnumAsByte<EFootballMovementMode::Type> CurrentMovementMode = EFootballMovementMode::Walking;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Movement", meta = (AllowPrivateAccess = "true"))
    bool bWantsToSprint = false;

    // Deterministic
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|Network", meta = (AllowPrivateAccess = "true"))
    int32 SimulationFrame = 0;

    virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PrevCustomMode = 0) override;

private:
    // Internal methods
    void UpdateStamina(float DeltaTime);
    void UpdateMovement(float DeltaTime);
    void UpdateActionCooldown(float DeltaTime);
    void ProcessInput(float DeltaTime);
    void CheckBallPossession();
    void UpdateProceduralIK(float DeltaTime);
    void UpdateLookAt(float DeltaTime);
    void ApplyMotionMatching(float DeltaTime);

    UFUNCTION()
    void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    // Input handlers
    void MoveForwardAxis(float Value);
    void MoveRightAxis(float Value);
    void OnMove(const FInputActionValue& Value);
    void OnLook(const FInputActionValue& Value);
    void OnSprintPressed();
    void OnSprintReleased();
    void OnPassPressed();
    void OnShootPressed();
    void OnTacklePressed();
    void OnSkillMovePressed();
};