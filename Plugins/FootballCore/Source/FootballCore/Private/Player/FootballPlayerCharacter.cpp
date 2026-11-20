// Copyright 2024 Craquez. All Rights Reserved.

#include "Player/FootballPlayerCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "ControlRigComponent.h"
#include "FootballCoreTypes.h"
#include "BallPhysics/FootballBallComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

AFootballPlayerCharacter::AFootballPlayerCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PrePhysics;

    // Configure capsule
    GetCapsuleComponent()->SetCapsuleHalfHeight(90.0f);
    GetCapsuleComponent()->SetCapsuleRadius(35.0f);
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Configure movement
    GetCharacterMovement()->MaxWalkSpeed = 400.0f; // 4 m/s base
    GetCharacterMovement()->MaxWalkSpeedCrouched = 200.0f;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->bUseControllerDesiredRotation = false;
    GetCharacterMovement()->JumpZVelocity = 500.0f;
    GetCharacterMovement()->AirControl = 0.3f;
    GetCharacterMovement()->GroundFriction = 3.0f;
    GetCharacterMovement()->BrakingDecelerationWalking = 2000.0f;
    GetCharacterMovement()->bEnablePhysicsInteraction = true;

    // Mesh
    GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));
    GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
    GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);

    // Control Rig
    ControlRigComponent = CreateDefaultSubobject<UControlRigComponent>(TEXT("ControlRig"));
    ControlRigComponent->SetupAttachment(GetMesh());

    // Tactical Component
    TacticalComponent = CreateDefaultSubobject<UFootballTacticalComponent>(TEXT("TacticalComponent"));

    // Replication
    bReplicates = true;
    SetReplicateMovement(true);
    NetUpdateFrequency = 60.0f;
    MinNetUpdateFrequency = 30.0f;
}

void AFootballPlayerCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Initialize stamina
    CurrentStamina = Attributes.Stamina;

    // Setup input mapping
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            if (FootballInputMapping)
            {
                Subsystem->AddMappingContext(FootballInputMapping, 0);
            }
        }
    }

    // Bind montage ended
    if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
    {
        AnimInstance->OnMontageEnded.AddDynamic(this, &AFootballPlayerCharacter::OnMontageEnded);
    }

    // Initialize Control Rig
    if (ControlRigComponent)
    {
        ControlRigComponent->OnInit();
    }
}

void AFootballPlayerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateStamina(DeltaTime);
    UpdateMovement(DeltaTime);
    UpdateActionCooldown(DeltaTime);
    UpdateProceduralAnimation(DeltaTime);
    CheckBallPossession();
    SimulationFrame++;
}

void AFootballPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        if (MoveAction)
        {
            EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AFootballPlayerCharacter::OnMove);
        }
        if (LookAction)
        {
            EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &AFootballPlayerCharacter::OnLook);
        }
        if (SprintAction)
        {
            EnhancedInput->BindAction(SprintAction, ETriggerEvent::Started, this, &AFootballPlayerCharacter::OnSprintPressed);
            EnhancedInput->BindAction(SprintAction, ETriggerEvent::Completed, this, &AFootballPlayerCharacter::OnSprintReleased);
        }
        if (PassAction)
        {
            EnhancedInput->BindAction(PassAction, ETriggerEvent::Started, this, &AFootballPlayerCharacter::OnPassPressed);
        }
        if (ShootAction)
        {
            EnhancedInput->BindAction(ShootAction, ETriggerEvent::Started, this, &AFootballPlayerCharacter::OnShootPressed);
        }
        if (TackleAction)
        {
            EnhancedInput->BindAction(TackleAction, ETriggerEvent::Started, this, &AFootballPlayerCharacter::OnTacklePressed);
        }
        if (SkillMoveAction)
        {
            EnhancedInput->BindAction(SkillMoveAction, ETriggerEvent::Started, this, &AFootballPlayerCharacter::OnSkillMovePressed);
        }
    }
}

void AFootballPlayerCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
}

void AFootballPlayerCharacter::UnPossessed()
{
    Super::UnPossessed();

    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            if (FootballInputMapping)
            {
                Subsystem->RemoveMappingContext(FootballInputMapping);
            }
        }
    }
}

// Ball Interaction
void AFootballPlayerCharacter::RequestPossession(UFootballBallComponent* Ball)
{
    if (!Ball || Ball->GetBallState() == EFootballBallState::Possessed)
    {
        return;
    }

    float Distance = FVector::Dist(GetActorLocation(), Ball->GetComponentLocation());
    if (Distance <= PossessionRange)
    {
        Ball->SetPossessingPlayer(this);
        CurrentBall = Ball;
        CurrentActionState = FFootballTags::Player_Dribbling;
        OnPossessionChanged.Broadcast(this, Ball);
    }
}

void AFootballPlayerCharacter::ReleasePossession()
{
    if (CurrentBall)
    {
        CurrentBall->SetPossessingPlayer(nullptr);
        OnPossessionChanged.Broadcast(this, CurrentBall);
        CurrentBall = nullptr;
        CurrentActionState = FFootballTags::Player_Idle;
    }
}

void AFootballPlayerCharacter::KickBall(const FVector& Direction, float Power, float Spin, FGameplayTag KickType)
{
    if (!CurrentBall)
    {
        OnActionExecuted.Broadcast(KickType, false, nullptr);
        return;
    }

    // Calculate impulse based on attributes and power
    float ImpulseMagnitude = Power * Attributes.ShotPower * 10.0f;
    FVector Impulse = Direction.GetSafeNormal() * ImpulseMagnitude;

    // Calculate spin
    FVector SpinVector = FVector::CrossProduct(Direction.GetSafeNormal(), FVector::UpVector) * Spin * Attributes.Curve;

    CurrentBall->ApplyKick(Impulse, SpinVector, this);

    // Play montage
    if (UAnimMontage* Montage = ActionMontages.FindRef(KickType))
    {
        PlayActionMontage(Montage);
    }

    CurrentActionState = KickType;
    ActionCooldown = 0.5f;

    ReleasePossession();
    OnActionExecuted.Broadcast(KickType, true, nullptr);
}

void AFootballPlayerCharacter::PassBall(AActor* TargetPlayer, float Power, FGameplayTag PassType)
{
    if (!CurrentBall || !TargetPlayer)
    {
        OnActionExecuted.Broadcast(PassType, false, TargetPlayer);
        return;
    }

    FVector Direction = (TargetPlayer->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    float Distance = FVector::Dist(GetActorLocation(), TargetPlayer->GetActorLocation());

    // Adjust power based on distance and passing attribute
    float BasePower = (PassType == FFootballTags::Action_Pass_Long) ? Attributes.LongPassing : Attributes.ShortPassing;
    float ImpulseMagnitude = Power * BasePower * FMath::Clamp(Distance / 3000.0f, 0.5f, 2.0f) * 8.0f;

    // Add some height for longer passes
    FVector Impulse = Direction * ImpulseMagnitude;
    if (PassType == FFootballTags::Action_Pass_Long || PassType == FFootballTags::Action_Pass_Through)
    {
        Impulse.Z += ImpulseMagnitude * 0.3f;
    }

    // Spin for curve passes
    FVector SpinVector = FVector::ZeroVector;
    if (PassType == FFootballTags::Action_Pass_Cross)
    {
        SpinVector = FVector::CrossProduct(Direction, FVector::UpVector) * Attributes.Curve * 0.5f;
    }

    CurrentBall->ApplyKick(Impulse, SpinVector, this);

    if (UAnimMontage* Montage = ActionMontages.FindRef(PassType))
    {
        PlayActionMontage(Montage);
    }

    CurrentActionState = PassType;
    ActionCooldown = 0.4f;

    ReleasePossession();
    OnActionExecuted.Broadcast(PassType, true, TargetPlayer);
}

void AFootballPlayerCharacter::ShootAtGoal(const FVector& TargetLocation, float Power, float Placement, FGameplayTag ShotType)
{
    if (!CurrentBall)
    {
        OnActionExecuted.Broadcast(ShotType, false, nullptr);
        return;
    }

    FVector Direction = (TargetLocation - GetActorLocation()).GetSafeNormal();
    float Distance = FVector::Dist(GetActorLocation(), TargetLocation);

    // Shot power based on attributes
    float PowerAttr = (ShotType == FFootballTags::Action_Shoot_Power) ? Attributes.ShotPower : Attributes.Finishing;
    float ImpulseMagnitude = Power * PowerAttr * 12.0f;

    // Placement affects direction slightly
    FVector FinalDirection = Direction;
    if (Placement != 0.0f)
    {
        FVector Right = FVector::CrossProduct(Direction, FVector::UpVector);
        FinalDirection = (Direction + Right * Placement * 0.2f).GetSafeNormal();
    }

    FVector Impulse = FinalDirection * ImpulseMagnitude;

    // Spin for placement/curve shots
    FVector SpinVector = FVector::ZeroVector;
    if (ShotType == FFootballTags::Action_Shoot_Placement || ShotType == FFootballTags::Action_Shoot_Chip)
    {
        SpinVector = FVector::CrossProduct(FinalDirection, FVector::UpVector) * Attributes.Curve * Placement;
    }

    CurrentBall->ApplyKick(Impulse, SpinVector, this);

    if (UAnimMontage* Montage = ActionMontages.FindRef(ShotType))
    {
        PlayActionMontage(Montage);
    }

    CurrentActionState = ShotType;
    ActionCooldown = 0.6f;

    ReleasePossession();
    OnActionExecuted.Broadcast(ShotType, true, nullptr);
}

void AFootballPlayerCharacter::HeaderBall(const FVector& Direction, float Power)
{
    if (!CurrentBall)
    {
        OnActionExecuted.Broadcast(FFootballTags::Action_Header, false, nullptr);
        return;
    }

    float ImpulseMagnitude = Power * Attributes.Heading * 8.0f;
    FVector Impulse = Direction.GetSafeNormal() * ImpulseMagnitude;
    FVector SpinVector = FVector::ZeroVector; // Headers typically have little spin

    CurrentBall->ApplyKick(Impulse, SpinVector, this);

    if (UAnimMontage* Montage = ActionMontages.FindRef(FFootballTags::Action_Header))
    {
        PlayActionMontage(Montage);
    }

    CurrentActionState = FFootballTags::Action_Header;
    ActionCooldown = 0.4f;

    ReleasePossession();
    OnActionExecuted.Broadcast(FFootballTags::Action_Header, true, nullptr);
}

void AFootballPlayerCharacter::ChestControl()
{
    if (!CurrentBall)
    {
        return;
    }

    // Trap ball with chest - kill velocity and gain possession
    CurrentBall->ApplyImpulseAtLocation(-CurrentBall->GetVelocity() * CurrentBall->GetPhysicsParams().Mass, GetActorLocation());
    CurrentBall->SetPossessingPlayer(this);
    CurrentBall = CurrentBall;
    CurrentActionState = FFootballTags::Player_Dribbling;
}

void AFootballPlayerCharacter::TrapBall()
{
    if (!CurrentBall)
    {
        return;
    }

    // Soft touch to control ball
    CurrentBall->ApplyImpulseAtLocation(-CurrentBall->GetVelocity() * CurrentBall->GetPhysicsParams().Mass * 0.5f, GetActorLocation());
    CurrentBall->SetPossessingPlayer(this);
    CurrentBall = CurrentBall;
    CurrentActionState = FFootballTags::Player_Dribbling;
}

void AFootballPlayerCharacter::SkillMove(FGameplayTag SkillTag)
{
    if (ActionCooldown > 0.0f || CurrentStamina < 10.0f)
    {
        OnActionExecuted.Broadcast(SkillTag, false, nullptr);
        return;
    }

    if (UAnimMontage* Montage = ActionMontages.FindRef(SkillTag))
    {
        PlayActionMontage(Montage);
        CurrentActionState = SkillTag;
        ActionCooldown = 1.0f;
        ConsumeStamina(10.0f);
        OnActionExecuted.Broadcast(SkillTag, true, nullptr);
    }
}

void AFootballPlayerCharacter::Tackle(FGameplayTag TackleType)
{
    if (ActionCooldown > 0.0f)
    {
        OnActionExecuted.Broadcast(TackleType, false, nullptr);
        return;
    }

    if (UAnimMontage* Montage = ActionMontages.FindRef(TackleType))
    {
        PlayActionMontage(Montage);
        CurrentActionState = TackleType;
        ActionCooldown = 1.5f;

        // Tackle logic - check for ball in range
        if (CurrentBall)
        {
            float Dist = FVector::Dist(GetActorLocation(), CurrentBall->GetComponentLocation());
            if (Dist < 200.0f)
            {
                // Successful tackle
                CurrentBall->ApplyImpulseAtLocation(GetActorForwardVector() * 5000.0f, CurrentBall->GetComponentLocation());
                ReleasePossession();
            }
        }

        ConsumeStamina(15.0f);
        OnActionExecuted.Broadcast(TackleType, true, CurrentBall ? CurrentBall->GetOwner() : nullptr);
    }
}

void AFootballPlayerCharacter::Intercept()
{
    CurrentActionState = FFootballTags::Player_Intercepting;
    ActionCooldown = 0.5f;
    // Intercept logic handled by AI/Tactical component
}

void AFootballPlayerCharacter::BlockShot()
{
    CurrentActionState = FFootballTags::Action_Block;
    ActionCooldown = 0.5f;
    // Block logic - expand collision temporarily
}

void AFootballPlayerCharacter::Clearance()
{
    if (!CurrentBall)
    {
        return;
    }

    // Panic clearance - just get it away
    FVector Direction = GetActorForwardVector();
    float ImpulseMagnitude = Attributes.LongPassing * 10.0f;
    CurrentBall->ApplyKick(Direction * ImpulseMagnitude, FVector::ZeroVector, this);
    ReleasePossession();
}

// Goalkeeper
void AFootballPlayerCharacter::GoalkeeperDive(FVector Direction)
{
    if (PlayerRole != EFootballPlayerRole::Goalkeeper)
    {
        return;
    }

    if (UAnimMontage* Montage = ActionMontages.FindRef(FGameplayTag::RequestGameplayTag("Action.Goalkeeper.Dive")))
    {
        PlayActionMontage(Montage);
        CurrentActionState = FFootballTags::Action_Goalkeeper_Save;
        ActionCooldown = 1.0f;

        // Extend collision capsule for dive
        GetCapsuleComponent()->SetCapsuleHalfHeight(60.0f);
        GetCapsuleComponent()->SetCapsuleRadius(80.0f);

        FTimerHandle TimerHandle;
        GetWorldTimerManager().SetTimer(TimerHandle, [this]()
        {
            GetCapsuleComponent()->SetCapsuleHalfHeight(90.0f);
            GetCapsuleComponent()->SetCapsuleRadius(35.0f);
        }, 1.0f, false);
    }
}

void AFootballPlayerCharacter::GoalkeeperCatch()
{
    if (PlayerRole != EFootballPlayerRole::Goalkeeper || !CurrentBall)
    {
        return;
    }

    CurrentActionState = FFootballTags::Action_Goalkeeper_Catch;
    ActionCooldown = 0.5f;
    // Catch logic - ball sticks to hands
}

void AFootballPlayerCharacter::GoalkeeperPunch()
{
    if (PlayerRole != EFootballPlayerRole::Goalkeeper || !CurrentBall)
    {
        return;
    }

    CurrentActionState = FFootballTags::Action_Goalkeeper_Punch;
    ActionCooldown = 0.5f;
    // Punch logic - deflect ball
    CurrentBall->ApplyImpulseAtLocation(GetActorForwardVector() * 8000.0f + FVector::UpVector * 2000.0f, CurrentBall->GetComponentLocation());
    ReleasePossession();
}

void AFootballPlayerCharacter::GoalkeeperDistributeThrow(AActor* Target)
{
    if (PlayerRole != EFootballPlayerRole::Goalkeeper || !CurrentBall || !Target)
    {
        return;
    }

    FVector Direction = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    float ImpulseMagnitude = Attributes.GK_Kicking * 8.0f;
    CurrentBall->ApplyKick(Direction * ImpulseMagnitude, FVector::ZeroVector, this);
    ReleasePossession();
    CurrentActionState = FFootballTags::Action_Goalkeeper_Distribute_Throw;
    ActionCooldown = 1.0f;
}

void AFootballPlayerCharacter::GoalkeeperDistributeKick(FVector Direction, float Power)
{
    if (PlayerRole != EFootballPlayerRole::Goalkeeper || !CurrentBall)
    {
        return;
    }

    float ImpulseMagnitude = Power * Attributes.GK_Kicking * 12.0f;
    CurrentBall->ApplyKick(Direction.GetSafeNormal() * ImpulseMagnitude, FVector::ZeroVector, this);
    ReleasePossession();
    CurrentActionState = FFootballTags::Action_Goalkeeper_Distribute_Kick;
    ActionCooldown = 1.0f;
}

// Movement
void AFootballPlayerCharacter::SetMovementMode(EFootballMovementMode::Type Mode)
{
    CurrentMovementMode = Mode;

    switch (Mode)
    {
        case EFootballMovementMode::Walking:
            GetCharacterMovement()->MaxWalkSpeed = 200.0f;
            break;
        case EFootballMovementMode::Jogging:
            GetCharacterMovement()->MaxWalkSpeed = 400.0f;
            break;
        case EFootballMovementMode::Sprinting:
            GetCharacterMovement()->MaxWalkSpeed = 700.0f * (Attributes.SprintSpeed / 100.0f);
            break;
        case EFootballMovementMode::Dribbling:
            GetCharacterMovement()->MaxWalkSpeed = 300.0f * (Attributes.Dribbling / 100.0f);
            GetCharacterMovement()->BrakingDecelerationWalking = 3000.0f;
            break;
        case EFootballMovementMode::Shielding:
            GetCharacterMovement()->MaxWalkSpeed = 150.0f;
            GetCharacterMovement()->BrakingDecelerationWalking = 5000.0f;
            break;
        case EFootballMovementMode::Jockeying:
            GetCharacterMovement()->MaxWalkSpeed = 300.0f;
            GetCharacterMovement()->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
            break;
        default:
            break;
    }
}

void AFootballPlayerCharacter::StartSprint()
{
    bWantsToSprint = true;
    if (CurrentStamina > 10.0f && !bIsFatigued)
    {
        SetMovementMode(EFootballMovementMode::Sprinting);
        CurrentActionState = FFootballTags::Player_Sprinting;
    }
}

void AFootballPlayerCharacter::StopSprint()
{
    bWantsToSprint = false;
    SetMovementMode(EFootballMovementMode::Jogging);
    CurrentActionState = FFootballTags::Player_Running;
}

void AFootballPlayerCharacter::StartJog()
{
    SetMovementMode(EFootballMovementMode::Jogging);
    CurrentActionState = FFootballTags::Player_Running;
}

void AFootballPlayerCharacter::StopJog()
{
    SetMovementMode(EFootballMovementMode::Walking);
    CurrentActionState = FFootballTags::Player_Idle;
}

// Stamina
void AFootballPlayerCharacter::ConsumeStamina(float Amount)
{
    CurrentStamina = FMath::Max(0.0f, CurrentStamina - Amount);
    OnStaminaChanged.Broadcast(CurrentStamina);

    if (CurrentStamina < Attributes.Stamina * 0.2f)
    {
        bIsFatigued = true;
        StopSprint();
    }
}

void AFootballPlayerCharacter::RecoverStamina(float DeltaTime)
{
    if (!bWantsToSprint && CurrentStamina < Attributes.Stamina)
    {
        CurrentStamina = FMath::Min(Attributes.Stamina, CurrentStamina + FatigueRecoveryRate * DeltaTime);
        OnStaminaChanged.Broadcast(CurrentStamina);

        if (CurrentStamina > Attributes.Stamina * 0.3f)
        {
            bIsFatigued = false;
        }
    }
}

// Attributes
void AFootballPlayerCharacter::SetAttributes(const FFootballPlayerAttributes& NewAttributes)
{
    Attributes = NewAttributes;
    CurrentStamina = Attributes.Stamina;
    OnStaminaChanged.Broadcast(CurrentStamina);

    // Update movement speeds based on new attributes
    SetMovementMode(CurrentMovementMode);
}

float AFootballPlayerCharacter::GetAttributeValue(FGameplayTag AttributeTag) const
{
    // Map gameplay tags to attribute values
    if (AttributeTag == FFootballTags::Body_LeftFoot || AttributeTag == FFootballTags::Body_RightFoot)
    {
        return Attributes.BallControl;
    }
    return 0.0f;
}

// Role and Team
void AFootballPlayerCharacter::SetPlayerRole(EFootballPlayerRole::Type NewRole)
{
    PlayerRole = NewRole;
}

void AFootballPlayerCharacter::SetTeam(AFootballTeam* NewTeam)
{
    Team = NewTeam;
}

EFootballTeamSide::Type AFootballPlayerCharacter::GetTeamSide() const
{
    return Team ? Team->GetTeamSide() : EFootballTeamSide::Neutral;
}

// Procedural Animation
void AFootballPlayerCharacter::UpdateProceduralAnimation(float DeltaTime)
{
    UpdateProceduralIK(DeltaTime);
    UpdateLookAt(DeltaTime);
    ApplyMotionMatching(DeltaTime);
}

void AFootballPlayerCharacter::SetIKTarget(FName BoneName, FVector TargetLocation, float BlendTime)
{
    IKTargets.Add(BoneName, TargetLocation);
    IKBlendWeights.Add(BoneName, 0.0f); // Will interpolate to 1.0f
}

void AFootballPlayerCharacter::ClearIKTarget(FName BoneName, float BlendTime)
{
    if (IKBlendWeights.Contains(BoneName))
    {
        IKBlendWeights[BoneName] = 0.0f; // Will interpolate to 0
    }
}

void AFootballPlayerCharacter::SetLookAtTarget(AActor* Target, float BlendTime)
{
    LookAtTarget = Target;
    LookAtBlendWeight = 0.0f;
}

void AFootballPlayerCharacter::ClearLookAtTarget(float BlendTime)
{
    LookAtTarget = nullptr;
    LookAtBlendWeight = 0.0f;
}

void AFootballPlayerCharacter::PlayActionMontage(UAnimMontage* Montage, float PlayRate, FName StartSection)
{
    if (Montage && GetMesh()->GetAnimInstance())
    {
        GetMesh()->GetAnimInstance()->Montage_Play(Montage, PlayRate);
        if (StartSection != NAME_None)
        {
            GetMesh()->GetAnimInstance()->Montage_JumpToSection(StartSection, Montage);
        }
    }
}

// Tactical
void AFootballPlayerCharacter::SetTacticalInstruction(FGameplayTag InstructionTag, float Value)
{
    if (TacticalComponent)
    {
        TacticalComponent->SetInstruction(InstructionTag, Value);
    }
}

float AFootballPlayerCharacter::GetTacticalInstruction(FGameplayTag InstructionTag) const
{
    return TacticalComponent ? TacticalComponent->GetInstruction(InstructionTag) : 0.0f;
}

// Network
void AFootballPlayerCharacter::Server_SetDeterministicState(const FFootballFixedVector& Position, const FFootballFixedVector& Velocity, const FFootballFixedRotator& Rotation, int32 FrameNumber)
{
    SetActorLocation(Position.ToVector());
    GetCharacterMovement()->Velocity = Velocity.ToVector();
    SetActorRotation(Rotation.ToRotator());
    SimulationFrame = FrameNumber;
}

void AFootballPlayerCharacter::GetDeterministicState(FFootballFixedVector& OutPosition, FFootballFixedVector& OutVelocity, FFootballFixedRotator& OutRotation, int32& OutFrameNumber) const
{
    OutPosition = FFootballFixedVector::FromVector(GetActorLocation());
    OutVelocity = FFootballFixedVector::FromVector(GetCharacterMovement()->Velocity);
    OutRotation = FFootballFixedRotator::FromRotator(GetActorRotation());
    OutFrameNumber = SimulationFrame;
}

// Private methods
void AFootballPlayerCharacter::UpdateStamina(float DeltaTime)
{
    if (bWantsToSprint && CurrentMovementMode == EFootballMovementMode::Sprinting)
    {
        ConsumeStamina(DeltaTime * 15.0f); // 15 stamina per second while sprinting
    }
    else
    {
        RecoverStamina(DeltaTime);
    }

    // Fatigue affects attributes
    if (bIsFatigued)
    {
        float FatigueFactor = CurrentStamina / (Attributes.Stamina * 0.2f);
        GetCharacterMovement()->MaxWalkSpeed *= FMath::Lerp(0.5f, 1.0f, FatigueFactor);
    }
}

void AFootballPlayerCharacter::UpdateMovement(float DeltaTime)
{
    // Auto-switch movement modes based on ball possession
    if (CurrentBall && CurrentActionState != FFootballTags::Player_Sprinting)
    {
        SetMovementMode(EFootballMovementMode::Dribbling);
    }
    else if (bWantsToSprint && CurrentStamina > 10.0f)
    {
        SetMovementMode(EFootballMovementMode::Sprinting);
    }
}

void AFootballPlayerCharacter::UpdateActionCooldown(float DeltaTime)
{
    if (ActionCooldown > 0.0f)
    {
        ActionCooldown -= DeltaTime;
        if (ActionCooldown <= 0.0f)
        {
            ActionCooldown = 0.0f;
            if (CurrentActionState != FFootballTags::Player_Idle &&
                CurrentActionState != FFootballTags::Player_Running &&
                CurrentActionState != FFootballTags::Player_Sprinting &&
                CurrentActionState != FFootballTags::Player_Dribbling)
            {
                CurrentActionState = FFootballTags::Player_Idle;
            }
        }
    }
}

void AFootballPlayerCharacter::ProcessInput(float DeltaTime)
{
    // Input processing handled by Enhanced Input
}

void AFootballPlayerCharacter::CheckBallPossession()
{
    if (!CurrentBall)
    {
        // Check for nearby balls
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFootballBallComponent::StaticClass(), FoundActors);

        for (AActor* Actor : FoundActors)
        {
            if (UFootballBallComponent* Ball = Cast<UFootballBallComponent>(Actor->GetComponentByClass(UFootballBallComponent::StaticClass())))
            {
                if (Ball->GetBallState() == EFootballBallState::Free || Ball->GetBallState() == EFootballBallState::Rolling)
                {
                    float Distance = FVector::Dist(GetActorLocation(), Ball->GetComponentLocation());
                    if (Distance <= PossessionRange)
                    {
                        RequestPossession(Ball);
                        break;
                    }
                }
            }
        }
    }
}

void AFootballPlayerCharacter::UpdateProceduralIK(float DeltaTime)
{
    // Interpolate IK weights
    for (auto& Pair : IKBlendWeights)
    {
        float TargetWeight = IKTargets.Contains(Pair.Key) ? 1.0f : 0.0f;
        Pair.Value = FMath::FInterpTo(Pair.Value, TargetWeight, DeltaTime, 10.0f);
    }

    // Apply IK to Control Rig
    if (ControlRigComponent)
    {
        for (const auto& Pair : IKTargets)
        {
            if (IKBlendWeights[Pair.Key] > 0.01f)
            {
                ControlRigComponent->SetControlOffset(Pair.Key, Pair.Value * IKBlendWeights[Pair.Key]);
            }
        }
    }
}

void AFootballPlayerCharacter::UpdateLookAt(float DeltaTime)
{
    if (LookAtTarget)
    {
        LookAtBlendWeight = FMath::FInterpTo(LookAtBlendWeight, 1.0f, DeltaTime, 5.0f);
        if (ControlRigComponent)
        {
            FVector LookDirection = (LookAtTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
            ControlRigComponent->SetControlOffset(FName("Head_LookAt"), LookDirection * LookAtBlendWeight);
        }
    }
    else
    {
        LookAtBlendWeight = FMath::FInterpTo(LookAtBlendWeight, 0.0f, DeltaTime, 5.0f);
    }
}

void AFootballPlayerCharacter::ApplyMotionMatching(float DeltaTime)
{
    // Motion matching would select best animation frame from database
    // based on current velocity, facing direction, and desired trajectory
    // This is a placeholder for the full implementation
}

void AFootballPlayerCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PrevCustomMode)
{
    Super::OnMovementModeChanged(PrevMovementMode, PrevCustomMode);
}

void AFootballPlayerCharacter::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (bInterrupted)
    {
        CurrentActionState = FFootballTags::Player_Idle;
    }
}

// Input Handlers
void AFootballPlayerCharacter::OnMove(const FInputActionValue& Value)
{
    FVector2D MoveInput = Value.Get<FVector2D>();
    if (Controller && (MoveInput.X != 0.0f || MoveInput.Y != 0.0f))
    {
        FRotator Rotation = Controller->GetControlRotation();
        FRotator YawRotation(0, Rotation.Yaw, 0);

        FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        AddMovementInput(ForwardDirection, MoveInput.Y);
        AddMovementInput(RightDirection, MoveInput.X);
    }
}

void AFootballPlayerCharacter::OnLook(const FInputActionValue& Value)
{
    FVector2D LookInput = Value.Get<FVector2D>();
    if (Controller)
    {
        AddControllerYawInput(LookInput.X);
        AddControllerPitchInput(LookInput.Y);
    }
}

void AFootballPlayerCharacter::OnSprintPressed()
{
    StartSprint();
}

void AFootballPlayerCharacter::OnSprintReleased()
{
    StopSprint();
}

void AFootballPlayerCharacter::OnPassPressed()
{
    if (CurrentBall)
    {
        // Find best pass target
        // For now, just do a short pass forward
        FVector Forward = GetActorForwardVector();
        FVector TargetLocation = GetActorLocation() + Forward * 2000.0f;
        PassBall(nullptr, 0.8f, FFootballTags::Action_Pass_Short);
    }
}

void AFootballPlayerCharacter::OnShootPressed()
{
    if (CurrentBall)
    {
        // Shoot toward goal
        FVector GoalDirection = GetActorForwardVector(); // Simplified
        ShootAtGoal(GetActorLocation() + GoalDirection * 5000.0f, 1.0f, 0.0f, FFootballTags::Action_Shoot_Power);
    }
}

void AFootballPlayerCharacter::OnTacklePressed()
{
    Tackle(FFootballTags::Action_Tackle_Standing);
}

void AFootballPlayerCharacter::OnSkillMovePressed()
{
    SkillMove(FGameplayTag::RequestGameplayTag("Action.SkillMove.StepOver"));
}