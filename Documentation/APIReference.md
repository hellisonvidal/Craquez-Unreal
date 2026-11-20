# Craquez FootballCore API Reference

Complete C++ and Blueprint API reference for the FootballCore plugin modules.

## Table of Contents
1. [FootballCore (Base)](#footballcore-base-module)
2. [FootballCoreGameplay](#footballcoregameplay)
3. [FootballCoreAI](#footballcoreai)
4. [FootballCoreNetworking](#footballcorenetworking)
5. [FootballCoreUI](#footballcoreui)
6. [Data Structures](#data-structures)
7. [Gameplay Tags](#gameplay-tags)

---

## FootballCore (Base Module)

### UFootballBallComponent
**Module:** FootballCore  
**Header:** `BallPhysics/FootballBallComponent.h`

Main ball physics component with Magnus effect, deformation, and precise collision.

#### Properties
| Property | Type | Category | Description |
|----------|------|----------|-------------|
| PhysicsParams | FFootballBallPhysicsParams | Ball Physics | Ball physical parameters (mass, radius, drag, lift, etc.) |
| CurrentVelocity | FVector | Ball Physics (ReadOnly) | Current linear velocity |
| CurrentAngularVelocity | FVector | Ball Physics (ReadOnly) | Current angular velocity (spin) |
| CurrentBallState | EFootballBallState::Type | Ball Physics (ReadOnly) | Current ball state |
| PossessingPlayer | AActor* | Ball Physics (ReadOnly) | Player currently possessing ball |
| CurrentDeformation | float | Deformation (ReadOnly) | Current deformation amount (0-1) |

#### Functions
```cpp
// Initialize ball with physics parameters
UFUNCTION(BlueprintCallable, Category="Football|Ball Physics")
void InitializeBall(const FFootballBallPhysicsParams& InParams);

// Apply kick impulse and spin
UFUNCTION(BlueprintCallable, Category="Football|Ball Physics")
void ApplyKick(const FVector& Impulse, const FVector& Spin, AActor* Instigator);

// Apply impulse at world location
UFUNCTION(BlueprintCallable, Category="Football|Ball Physics")
void ApplyImpulseAtLocation(const FVector& Impulse, const FVector& Location);

// Set ball state manually
UFUNCTION(BlueprintCallable, Category="Football|Ball Physics")
void SetBallState(EFootballBallState::Type NewState);

// Reset ball to location
UFUNCTION(BlueprintCallable, Category="Football|Ball Physics")
void ResetBall(const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator);

// Set/get possessing player
UFUNCTION(BlueprintCallable, Category="Football|Ball Physics")
void SetPossessingPlayer(AActor* Player);

// Network replication (deterministic)
UFUNCTION(BlueprintCallable, Category="Football|Network")
void SetDeterministicState(const FFootballFixedVector& Position, const FFootballFixedVector& Velocity, const FFootballFixedVector& AngularVelocity, int32 FrameNumber);

UFUNCTION(BlueprintCallable, Category="Football|Network")
void GetDeterministicState(FFootballFixedVector& OutPosition, FFootballFixedVector& OutVelocity, FFootballFixedVector& OutAngularVelocity, int32& OutFrameNumber) const;
```

#### Events
| Event | Parameters | Description |
|-------|------------|-------------|
| OnBallKicked | Impulse, Spin, Instigator | Ball was kicked |
| OnBallCollision | HitResult, OtherActor | Ball collided with something |
| OnBallGoalScored | ScoringTeam | Goal was scored |
| OnBallOutOfPlay | LastTouchTeam | Ball went out of bounds |
| OnBallStateChanged | NewState | Ball state changed |

---

### AFootballBallActor
**Module:** FootballCore  
**Header:** `BallPhysics/FootballBallActor.h`

Actor wrapper for ball component with visual mesh and effects.

#### Functions
```cpp
UFUNCTION(BlueprintCallable, Category="Football|Ball")
UFootballBallComponent* GetBallComponent() const;

UFUNCTION(BlueprintCallable, Category="Football|Ball")
void SetBallMesh(UStaticMesh* Mesh);

UFUNCTION(BlueprintCallable, Category="Football|Ball")
void SetBallMaterial(UMaterialInterface* Material);

UFUNCTION(BlueprintCallable, Category="Football|Ball")
void ApplyKick(const FVector& Impulse, const FVector& Spin, AActor* Instigator);

UFUNCTION(BlueprintCallable, Category="Football|Ball")
void ResetBall(const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator);
```

---

## FootballCoreGameplay

### AFootballPlayerCharacter
**Module:** FootballCoreGameplay  
**Header:** `Player/FootballPlayerCharacter.h`

Base player character with procedural animation, ball interaction, and full attribute system.

#### Properties
| Property | Type | Category | Description |
|----------|------|----------|-------------|
| Attributes | FFootballPlayerAttributes | Attributes | All player attributes (physical, technical, mental, GK) |
| CurrentStamina | float | Attributes (ReadOnly) | Current stamina value |
| bIsFatigued | bool | Attributes (ReadOnly) | Whether player is fatigued |
| PlayerRole | EFootballPlayerRole::Type | Role | Player position/role |
| Team | AFootballTeam* | Team (ReadOnly) | Player's team |
| CurrentBall | UFootballBallComponent* | Ball (ReadOnly) | Currently possessed ball |
| CurrentActionState | FGameplayTag | State (ReadOnly) | Current action being performed |
| CurrentMovementMode | EFootballMovementMode::Type | Movement (ReadOnly) | Current movement mode |

#### Ball Interaction
```cpp
UFUNCTION(BlueprintCallable, Category="Football|Ball")
void RequestPossession(UFootballBallComponent* Ball);

UFUNCTION(BlueprintCallable, Category="Football|Ball")
void ReleasePossession();

UFUNCTION(BlueprintCallable, Category="Football|Ball")
void KickBall(const FVector& Direction, float Power, float Spin, FGameplayTag KickType);

UFUNCTION(BlueprintCallable, Category="Football|Ball")
void PassBall(AActor* TargetPlayer, float Power, FGameplayTag PassType);

UFUNCTION(BlueprintCallable, Category="Football|Ball")
void ShootAtGoal(const FVector& TargetLocation, float Power, float Placement, FGameplayTag ShotType);

UFUNCTION(BlueprintCallable, Category="Football|Ball")
void HeaderBall(const FVector& Direction, float Power);

UFUNCTION(BlueprintCallable, Category="Football|Ball")
void ChestControl();

UFUNCTION(BlueprintCallable, Category="Football|Ball")
void TrapBall();

UFUNCTION(BlueprintCallable, Category="Football|Ball")
void SkillMove(FGameplayTag SkillTag);

UFUNCTION(BlueprintCallable, Category="Football|Ball")
void Tackle(FGameplayTag TackleType);

UFUNCTION(BlueprintCallable, Category="Football|Ball")
void Intercept();

UFUNCTION(BlueprintCallable, Category="Football|Ball")
void BlockShot();

UFUNCTION(BlueprintCallable, Category="Football|Ball")
void Clearance();
```

#### Goalkeeper Specific
```cpp
UFUNCTION(BlueprintCallable, Category="Football|Goalkeeper")
void GoalkeeperDive(FVector Direction);

UFUNCTION(BlueprintCallable, Category="Football|Goalkeeper")
void GoalkeeperCatch();

UFUNCTION(BlueprintCallable, Category="Football|Goalkeeper")
void GoalkeeperPunch();

UFUNCTION(BlueprintCallable, Category="Football|Goalkeeper")
void GoalkeeperDistributeThrow(AActor* Target);

UFUNCTION(BlueprintCallable, Category="Football|Goalkeeper")
void GoalkeeperDistributeKick(FVector Direction, float Power);
```

#### Movement & Stamina
```cpp
UFUNCTION(BlueprintCallable, Category="Football|Movement")
void SetMovementMode(EFootballMovementMode::Type Mode);

UFUNCTION(BlueprintCallable, Category="Football|Movement")
void StartSprint();

UFUNCTION(BlueprintCallable, Category="Football|Movement")
void StopSprint();

UFUNCTION(BlueprintCallable, Category="Football|Stamina")
void ConsumeStamina(float Amount);

UFUNCTION(BlueprintCallable, Category="Football|Stamina")
void RecoverStamina(float DeltaTime);

UFUNCTION(BlueprintPure, Category="Football|Stamina")
float GetStaminaPercent() const;

UFUNCTION(BlueprintPure, Category="Football|Stamina")
bool IsFatigued() const;
```

#### Procedural Animation
```cpp
UFUNCTION(BlueprintCallable, Category="Football|Animation")
void UpdateProceduralAnimation(float DeltaTime);

UFUNCTION(BlueprintCallable, Category="Football|Animation")
void SetIKTarget(FName BoneName, FVector TargetLocation, float BlendTime = 0.2f);

UFUNCTION(BlueprintCallable, Category="Football|Animation")
void ClearIKTarget(FName BoneName, float BlendTime = 0.2f);

UFUNCTION(BlueprintCallable, Category="Football|Animation")
void SetLookAtTarget(AActor* Target, float BlendTime = 0.3f);

UFUNCTION(BlueprintCallable, Category="Football|Animation")
void ClearLookAtTarget(float BlendTime = 0.3f);

UFUNCTION(BlueprintCallable, Category="Football|Animation")
void PlayActionMontage(UAnimMontage* Montage, float PlayRate = 1.0f, FName StartSection = NAME_None);
```

#### Tactical & Network
```cpp
UFUNCTION(BlueprintCallable, Category="Football|Tactical")
void SetTacticalInstruction(FGameplayTag InstructionTag, float Value);

UFUNCTION(BlueprintPure, Category="Football|Tactical")
float GetTacticalInstruction(FGameplayTag InstructionTag) const;

UFUNCTION(BlueprintCallable, Category="Football|Network")
void Server_SetDeterministicState(const FFootballFixedVector& Position, const FFootballFixedVector& Velocity, const FFootballFixedRotator& Rotation, int32 FrameNumber);

UFUNCTION(BlueprintCallable, Category="Football|Network")
void GetDeterministicState(FFootballFixedVector& OutPosition, FFootballFixedVector& OutVelocity, FFootballFixedRotator& OutRotation, int32& OutFrameNumber) const;
```

#### Events
| Event | Parameters |
|-------|------------|
| OnPossessionChanged | Player, Ball |
| OnActionExecuted | ActionTag, bSuccess, Target |
| OnStaminaChanged | CurrentStamina |

---

### AFootballTeam
**Module:** FootballCoreGameplay  
**Header:** `Team/FootballTeam.h`

Manages a team of 11 players, formations, and tactics.

#### Properties
| Property | Type | Description |
|----------|------|-------------|
| TeamSide | EFootballTeamSide::Type | Home/Away/Neutral |
| TeamName | FString | Team display name |
| TeamColor | FColor | Team color for UI |
| TeamCrest | UTexture2D* | Team crest/logo |
| Tactics | FFootballTacticalInstructions | Current tactical instructions |
| CurrentFormation | int32 | Active formation ID (433, 442, 352, etc.) |

#### Functions
```cpp
UFUNCTION(BlueprintCallable, Category="Football|Team")
void InitializeTeam(EFootballTeamSide::Type InSide, const FString& TeamName, const FFootballTacticalInstructions& InTactics);

UFUNCTION(BlueprintCallable, Category="Football|Team")
void SetTeamManager(AFootballTeamManager* InManager);

UFUNCTION(BlueprintCallable, Category="Football|Team")
void AddPlayer(AFootballPlayerCharacter* Player, EFootballPlayerRole::Type Role);

UFUNCTION(BlueprintCallable, Category="Football|Team")
void RemovePlayer(AFootballPlayerCharacter* Player);

UFUNCTION(BlueprintCallable, Category="Football|Team")
void SetFormation(int32 FormationID);

UFUNCTION(BlueprintCallable, Category="Football|Team")
void SetTactics(const FFootballTacticalInstructions& NewTactics);

UFUNCTION(BlueprintCallable, Category="Football|Team")
void ApplyTactic(FGameplayTag TacticTag, float Value);

UFUNCTION(BlueprintCallable, Category="Football|Team")
AFootballPlayerCharacter* GetPlayerByRole(EFootballPlayerRole::Type Role) const;

UFUNCTION(BlueprintCallable, Category="Football|Team")
TArray<AFootballPlayerCharacter*> GetPlayersByRole(EFootballPlayerRole::Type Role) const;

UFUNCTION(BlueprintCallable, Category="Football|Team")
AFootballPlayerCharacter* GetClosestPlayerToBall() const;

UFUNCTION(BlueprintCallable, Category="Football|Team")
AFootballPlayerCharacter* GetPlayerWithBall() const;

UFUNCTION(BlueprintCallable, Category="Football|Team")
TArray<AFootballPlayerCharacter*> GetAllPlayers() const;

UFUNCTION(BlueprintCallable, Category="Football|Tactical")
void SetTeamInstruction(FGameplayTag Instruction, float Value);

UFUNCTION(BlueprintPure, Category="Football|Tactical")
float GetTeamInstruction(FGameplayTag Instruction) const;

UFUNCTION(BlueprintCallable, Category="Football|Match")
void OnKickOff(bool bWeHaveBall);

UFUNCTION(BlueprintCallable, Category="Football|Match")
void OnPossessionGained();

UFUNCTION(BlueprintCallable, Category="Football|Match")
void OnPossessionLost();

UFUNCTION(BlueprintCallable, Category="Football|Match")
void OnGoalScored(AFootballPlayerCharacter* Scorer);

UFUNCTION(BlueprintCallable, Category="Football|Match")
void OnGoalConceded();

UFUNCTION(BlueprintCallable, Category="Football|Match")
void OnSetPiece(EFootballMatchPhase::Type SetPieceType, bool bAttacking);

UFUNCTION(BlueprintCallable, Category="Football|Match")
void OnHalfTime();

UFUNCTION(BlueprintCallable, Category="Football|Match")
void OnFullTime();

UFUNCTION(BlueprintCallable, Category="Football|Team")
bool RequestSubstitution(AFootballPlayerCharacter* PlayerOut, AFootballPlayerCharacter* PlayerIn);
```

#### Getters
```cpp
UFUNCTION(BlueprintPure, Category="Football|Team")
EFootballTeamSide::Type GetTeamSide() const;

UFUNCTION(BlueprintPure, Category="Football|Team")
FString GetTeamName() const;

UFUNCTION(BlueprintPure, Category="Football|Team")
FFootballTacticalInstructions GetTactics() const;

UFUNCTION(BlueprintPure, Category="Football|Team")
bool HasPossession() const;

UFUNCTION(BlueprintPure, Category="Football|Team")
int32 GetGoals() const;

UFUNCTION(BlueprintPure, Category="Football|Team")
int32 GetShots() const;

UFUNCTION(BlueprintPure, Category="Football|Team")
int32 GetShotsOnTarget() const;
```

#### Events
| Event | Parameters |
|-------|------------|
| OnPossessionChanged | PossessingTeam |
| OnGoalScored | ScoringTeam, Scorer |
| OnTacticChanged | TacticTag |

---

### AFootballGameMode
**Module:** FootballCoreGameplay  
**Header:** `Match/FootballGameMode.h`

Main match controller handling phases, scoring, rules, and match flow.

#### Properties
| Property | Type | Description |
|----------|------|-------------|
| GameModeType | EFootballGameMode::Type | 11v11, 1v1, PenaltyShootout, etc. |
| MatchDuration | float | Total match minutes (default 90) |
| HalfDuration | float | Half duration in minutes (default 45) |
| HomeScore | int32 | Home team goals |
| AwayScore | int32 | Away team goals |
| CurrentPhase | EFootballMatchPhase::Type | Current match phase |
| MatchTimeElapsed | float | Seconds elapsed in match |

#### Functions
```cpp
UFUNCTION(BlueprintCallable, Category="Football|Match")
void StartMatch();

UFUNCTION(BlueprintCallable, Category="Football|Match")
void PauseMatch();

UFUNCTION(BlueprintCallable, Category="Football|Match")
void ResumeMatch();

UFUNCTION(BlueprintCallable, Category="Football|Match")
void EndMatch();

UFUNCTION(BlueprintCallable, Category="Football|Match")
void RestartMatch();

UFUNCTION(BlueprintCallable, Category="Football|Team")
void SetHomeTeam(AFootballTeam* Team);

UFUNCTION(BlueprintCallable, Category="Football|Team")
void SetAwayTeam(AFootballTeam* Team);

UFUNCTION(BlueprintPure, Category="Football|Team")
AFootballTeam* GetHomeTeam() const;

UFUNCTION(BlueprintPure, Category="Football|Team")
AFootballTeam* GetAwayTeam() const;

UFUNCTION(BlueprintCallable, Category="Football|Ball")
UFootballBallComponent* GetBall() const;

UFUNCTION(BlueprintCallable, Category="Football|Ball")
void SpawnBall(FVector Location = FVector::ZeroVector);

UFUNCTION(BlueprintCallable, Category="Football|Ball")
void ResetBall(EFootballMatchPhase::Type RestartType);

UFUNCTION(BlueprintCallable, Category="Football|Scoring")
void RegisterGoal(EFootballTeamSide::Type ScoringTeam, AFootballPlayerCharacter* Scorer);

UFUNCTION(BlueprintPure, Category="Football|Scoring")
int32 GetHomeScore() const;

UFUNCTION(BlueprintPure, Category="Football|Scoring")
int32 GetAwayScore() const;

UFUNCTION(BlueprintPure, Category="Football|Time")
float GetMatchTimeRemaining() const;

UFUNCTION(BlueprintPure, Category="Football|Time")
float GetMatchTimeElapsed() const;

UFUNCTION(BlueprintPure, Category="Football|Time")
int32 GetCurrentMinute() const;

UFUNCTION(BlueprintCallable, Category="Football|Match")
void SetMatchPhase(EFootballMatchPhase::Type NewPhase);

UFUNCTION(BlueprintPure, Category="Football|Match")
EFootballMatchPhase::Type GetCurrentPhase() const;

// Rules
UFUNCTION(BlueprintCallable, Category="Football|Rules")
void CheckOffside(AFootballPlayerCharacter* Attacker);

UFUNCTION(BlueprintCallable, Category="Football|Rules")
void RegisterFoul(EFootballTeamSide::Type OffendingTeam, AFootballPlayerCharacter* Offender, FVector Location, FGameplayTag FoulType);

UFUNCTION(BlueprintCallable, Category="Football|Rules")
void AwardFreeKick(EFootballTeamSide::Type Team, FVector Location, FGameplayTag FoulType);

UFUNCTION(BlueprintCallable, Category="Football|Rules")
void AwardPenalty(EFootballTeamSide::Type Team, FVector Location);

UFUNCTION(BlueprintCallable, Category="Football|Rules")
void AwardCorner(EFootballTeamSide::Type Team);

UFUNCTION(BlueprintCallable, Category="Football|Rules")
void AwardGoalKick(EFootballTeamSide::Type Team);

UFUNCTION(BlueprintCallable, Category="Football|Rules")
void AwardThrowIn(EFootballTeamSide::Type Team, FVector Location);

// Substitutions
UFUNCTION(BlueprintCallable, Category="Football|Team")
bool ProcessSubstitution(EFootballTeamSide::Type Team, AFootballPlayerCharacter* PlayerOut, AFootballPlayerCharacter* PlayerIn);

// Game Mode
UFUNCTION(BlueprintCallable, Category="Football|Match")
void SetGameMode(EFootballGameMode::Type Mode);

UFUNCTION(BlueprintPure, Category="Football|Match")
EFootballGameMode::Type GetGameMode() const;

// Player Spawning
UFUNCTION(BlueprintCallable, Category="Football|Players")
void SpawnPlayers();

UFUNCTION(BlueprintCallable, Category="Football|Players")
AFootballPlayerCharacter* SpawnPlayer(EFootballTeamSide::Type Team, EFootballPlayerRole::Type Role, FVector Location, FRotator Rotation);
```

#### Events
| Event | Parameters |
|-------|------------|
| OnMatchEvent | EventTag, Instigator, Target |
| OnGoalScored | ScoringTeam, Scorer |
| OnMatchPhaseChanged | NewPhase |
| OnMatchTimeChanged | TimeRemaining |

---

### UFootballTacticalComponent
**Module:** FootballCoreGameplay  
**Header:** `Tactical/FootballTacticalComponent.h`

Handles team tactics, player positioning, and strategic decisions.

#### Functions
```cpp
UFUNCTION(BlueprintCallable, Category="Football|Tactical")
void SetTeam(AFootballTeam* InTeam);

UFUNCTION(BlueprintCallable, Category="Football|Tactical")
void Initialize(const FFootballTacticalInstructions& InTactics);

UFUNCTION(BlueprintCallable, Category="Football|Tactical")
void UpdateTactics(const FFootballTacticalInstructions& InTactics);

UFUNCTION(BlueprintCallable, Category="Football|Tactical")
void SetInstruction(FGameplayTag Instruction, float Value);

UFUNCTION(BlueprintPure, Category="Football|Tactical")
float GetInstruction(FGameplayTag Instruction) const;

UFUNCTION(BlueprintCallable, Category="Football|Tactical")
void SetPlayerTargetPosition(AFootballPlayerCharacter* Player, FVector TargetPosition);

UFUNCTION(BlueprintCallable, Category="Football|Tactical")
void UpdatePlayerPosition(AFootballPlayerCharacter* Player, float DeltaTime);

UFUNCTION(BlueprintCallable, Category="Football|Tactical")
FVector CalculateDefensivePosition(AFootballPlayerCharacter* Player) const;

UFUNCTION(BlueprintCallable, Category="Football|Tactical")
FVector CalculateAttackingPosition(AFootballPlayerCharacter* Player) const;

UFUNCTION(BlueprintCallable, Category="Football|Tactical")
FVector CalculateSupportPosition(AFootballPlayerCharacter* Player) const;

UFUNCTION(BlueprintCallable, Category="Football|Tactical")
void MaintainTeamShape(float DeltaTime);

UFUNCTION(BlueprintCallable, Category="Football|Tactical")
float CalculateTeamCompactness() const;

UFUNCTION(BlueprintCallable, Category="Football|Tactical")
FVector CalculateTeamCenter() const;

UFUNCTION(BlueprintCallable, Category="Football|Tactical")
float CalculateDefensiveLine() const;

UFUNCTION(BlueprintCallable, Category="Football|Tactical")
float CalculateOffsideLine() const;

UFUNCTION(BlueprintCallable, Category="Football|Tactical")
TArray<AFootballPlayerCharacter*> GetPlayersInZone(FVector ZoneCenter, float ZoneRadius) const;

UFUNCTION(BlueprintCallable, Category="Football|Tactical")
int32 CountPlayersInArea(FVector Min, FVector Max) const;

UFUNCTION(BlueprintCallable, Category="Football|Tactical")
bool IsPlayerMarked(AFootballPlayerCharacter* Player) const;

UFUNCTION(BlueprintCallable, Category="Football|Tactical")
AFootballPlayerCharacter* FindBestPassTarget(AFootballPlayerCharacter* Passer) const;

UFUNCTION(BlueprintCallable, Category="Football|Tactical")
AFootballPlayerCharacter* FindBestInterceptTarget() const;

UFUNCTION(BlueprintCallable, Category="Football|Tactical")
FVector CalculateInterceptPoint(UFootballBallComponent* Ball, AFootballPlayerCharacter* Interceptor) const;

// Set Pieces
UFUNCTION(BlueprintCallable, Category="Football|Tactical|SetPiece")
void SetupCornerRoutine(bool bAttacking);

UFUNCTION(BlueprintCallable, Category="Football|Tactical|SetPiece")
void SetupFreeKickRoutine(FVector BallPosition, bool bAttacking);

UFUNCTION(BlueprintCallable, Category="Football|Tactical|SetPiece")
void SetupPenaltyRoutine(bool bAttacking);

UFUNCTION(BlueprintCallable, Category="Football|Tactical|SetPiece")
void ExecuteSetPieceRoutine(EFootballMatchPhase::Type SetPieceType);
```

#### Events
| Event | Parameters |
|-------|------------|
| OnInstructionChanged | Instruction, Value |

---

## FootballCoreAI

### Behavior Tree Tasks
All tasks inherit from `UFootballBTTask` and are in namespace `FootballCoreAI`.

#### UBTTask_MoveToBall
Moves AI to ball location.
```cpp
UPROPERTY(EditAnywhere) float AcceptanceRadius = 150.0f;
UPROPERTY(EditAnywhere) bool bStopOnOverlap = true;
UPROPERTY(EditAnywhere) EFootballMovementMode::Type MovementMode = EFootballMovementMode::Jogging;
```

#### UBTTask_InterceptBall
Predicts ball trajectory and intercepts.
```cpp
UPROPERTY(EditAnywhere) float PredictionTime = 2.0f;
UPROPERTY(EditAnywhere) float MaxInterceptDistance = 5000.0f;
UPROPERTY(EditAnywhere) bool bPredictBallPhysics = true;
```

#### UBTTask_PassToTeammate
Finds best pass target using tactical analysis.
```cpp
UPROPERTY(EditAnywhere) FGameplayTag PassType = FFootballTags::Action_Pass_Short;
UPROPERTY(EditAnywhere) float MinPassDistance = 500.0f;
UPROPERTY(EditAnywhere) float MaxPassDistance = 4000.0f;
UPROPERTY(EditAnywhere) bool bRequireOpenLane = true;
```

#### UBTTask_ShootAtGoal
Shoots at goal with power/placement options.
```cpp
UPROPERTY(EditAnywhere) FGameplayTag ShotType = FFootballTags::Action_Shoot_Power;
UPROPERTY(EditAnywhere) float MinShootDistance = 1000.0f;
UPROPERTY(EditAnywhere) float MaxShootDistance = 30000.0f;
UPROPERTY(EditAnywhere) float GoalTargetVariance = 1000.0f;
UPROPERTY(EditAnywhere) bool bPreferPlacement = false;
```

#### UBTTask_TackleOpponent
Attempts tackle on ball carrier.
```cpp
UPROPERTY(EditAnywhere) FGameplayTag TackleType = FFootballTags::Action_Tackle_Standing;
UPROPERTY(EditAnywhere) float TackleRange = 200.0f;
UPROPERTY(EditAnywhere) float MinTackleSuccessChance = 0.4f;
```

#### UBTTask_MarkOpponent
Man-to-man marking with goal-side/ball-side positioning.
```cpp
UPROPERTY(EditAnywhere) float MarkDistance = 200.0f;
UPROPERTY(EditAnywhere) bool bGoalSide = true;
UPROPERTY(EditAnywhere) bool bBallSide = true;
```

#### UBTTask_MoveToFormationPosition
Moves to tactical formation position.
```cpp
UPROPERTY(EditAnywhere) float PositionTolerance = 200.0f;
UPROPERTY(EditAnywhere) bool bAdjustForBall = true;
UPROPERTY(EditAnywhere) float BallInfluenceRadius = 3000.0f;
```

#### UBTTask_PressBallCarrier
Presses ball carrier from angle.
```cpp
UPROPERTY(EditAnywhere) float PressDistance = 300.0f;
UPROPERTY(EditAnywhere) float PressAngle = 45.0f;
UPROPERTY(EditAnywhere) bool bForceWeakFoot = true;
```

#### UBTTask_SupportBallCarrier
Provides passing support to ball carrier.
```cpp
UPROPERTY(EditAnywhere) float SupportDistance = 1500.0f;
UPROPERTY(EditAnywhere) float SupportAngle = 45.0f;
UPROPERTY(EditAnywhere) bool bProvidePassingOption = true;
```

#### UBTTask_GoalkeeperSave
Goalkeeper dive reaction to shots.
```cpp
UPROPERTY(EditAnywhere) float ReactionTime = 0.15f;
UPROPERTY(EditAnywhere) float DiveRange = 300.0f;
UPROPERTY(EditAnywhere) bool bPreferCatch = true;
```

#### UBTTask_GoalkeeperDistribute
Goalkeeper distribution (throw/kick).
```cpp
UPROPERTY(EditAnywhere) bool bPreferThrow = false;
UPROPERTY(EditAnywhere) float MinKickDistance = 4000.0f;
```

---

### StateTree Tasks

#### UPressTask
Pressing state with fixed transition to Intercept.
```cpp
UPROPERTY(EditAnywhere) float PressDistance = 300.0f;
UPROPERTY(EditAnywhere) float PressAngle = 45.0f;
UPROPERTY(EditAnywhere) bool bForceWeakFoot = true;
```

#### UInterceptTask
Interception execution state.
```cpp
UPROPERTY(EditAnywhere) float PredictionTime = 2.0f;
UPROPERTY(EditAnywhere) float MaxInterceptDistance = 5000.0f;
```

#### Key Evaluators
- **UInterceptionEvaluator** - Calculates intercept confidence with proper physics prediction
- **UBallStateEvaluator** - Tracks ball state, position, velocity
- **UTacticalEvaluator** - Team tactics, pressing intensity, defensive line

---

### GOAP System

#### UFootballGOAPPlanner
```cpp
UFUNCTION(BlueprintCallable, Category="Football|GOAP")
FGOAPPlan Plan(const FGOAPWorldState& CurrentState, const TArray<FGOAPGoal>& Goals, const TArray<FGOAPAction>& AvailableActions);

UFUNCTION(BlueprintCallable, Category="Football|GOAP")
FGOAPWorldState EvaluateWorldState(AFootballPlayerCharacter* Character, UFootballBallComponent* Ball, AFootballTeam* Team, AFootballTeam* OpponentTeam) const;

UFUNCTION(BlueprintCallable, Category="Football|GOAP")
TArray<FGOAPAction> GetDefaultActions() const;

UFUNCTION(BlueprintCallable, Category="Football|GOAP")
TArray<FGOAPGoal> GetDefaultGoals() const;
```

#### UFootballGOAPComponent
Attaches to AI characters for runtime planning.
```cpp
UPROPERTY(EditAnywhere) float ReplanInterval = 2.0f;
UPROPERTY(EditAnywhere) float MaxPlanDepth = 10;
UPROPERTY(EditAnywhere) TArray<FGOAPGoal> Goals;
UPROPERTY(EditAnywhere) TArray<FGOAPAction> CustomActions;

UFUNCTION(BlueprintCallable, Category="Football|GOAP")
void RequestPlan();

UFUNCTION(BlueprintCallable, Category="Football|GOAP")
void ExecuteCurrentAction(float DeltaTime);

UFUNCTION(BlueprintCallable, Category="Football|GOAP")
void AbortCurrentPlan();
```

---

## FootballCoreNetworking

### UFootballDeterministicReplicationComponent
**Header:** `Deterministic/FootballDeterministicReplication.h`

```cpp
UPROPERTY(EditAnywhere) bool bDeterministicMode = true;
UPROPERTY(EditAnywhere) float FixedTimeStep = 1.0f / 60.0f;
UPROPERTY(EditAnywhere) int32 MaxPredictionFrames = 4;
UPROPERTY(EditAnywhere) int32 StateHistorySize = 120;
UPROPERTY(EditAnywhere) float PositionTolerance = 10.0f;
UPROPERTY(EditAnywhere) float VelocityTolerance = 50.0f;
UPROPERTY(EditAnywhere) bool bInterpolate = true;
UPROPERTY(EditAnywhere) float InterpolationTime = 0.1f;

UFUNCTION(BlueprintCallable, Category="Football|Network|Deterministic")
void SetDeterministicMode(bool bEnabled);

UFUNCTION(BlueprintCallable, Category="Football|Network|Deterministic")
void SetFixedTimeStep(float TimeStep);

UFUNCTION(BlueprintCallable, Category="Football|Network|Deterministic")
void CaptureState(int32 FrameNumber);

UFUNCTION(BlueprintCallable, Category="Football|Network|Deterministic")
void ApplyState(int32 FrameNumber);

UFUNCTION(BlueprintCallable, Category="Football|Network|Deterministic")
void RollbackToFrame(int32 FrameNumber);

UFUNCTION(BlueprintCallable, Category="Football|Network|Deterministic")
void Server_SendState(int32 FrameNumber, const FFootballFixedVector& Position, const FFootballFixedVector& Velocity, const FFootballFixedRotator& Rotation);

UFUNCTION(BlueprintCallable, Category="Football|Network|Deterministic")
void Client_ReceiveState(int32 FrameNumber, const FFootballFixedVector& Position, const FFootballFixedVector& Velocity, const FFootballFixedRotator& Rotation);

UFUNCTION(BlueprintCallable, Category="Football|Network|Deterministic")
void SetBallComponent(UFootballBallComponent* InBallComponent);

UFUNCTION(BlueprintCallable, Category="Football|Network|Deterministic")
void ReplicateBallState(int32 FrameNumber);

UFUNCTION(BlueprintCallable, Category="Football|Network|Deterministic")
void ReplicatePlayerState(AFootballPlayerCharacter* Player, int32 FrameNumber);

UFUNCTION(BlueprintCallable, Category="Football|Network|Deterministic")
void SetInterpolationEnabled(bool bEnabled);

UFUNCTION(BlueprintCallable, Category="Football|Network|Deterministic")
void SetInterpolationTime(float Time);
```

#### Events
| Event | Parameters |
|-------|------------|
| OnStateCorrected | FrameNumber, CorrectedPosition, CorrectedVelocity |

---

### IFootballDeterministicInterface
Blueprint interface for deterministic replication.

```cpp
UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Football|Network|Deterministic")
FFootballDeterministicState GetDeterministicState();

UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Football|Network|Deterministic")
void ApplyDeterministicState(const FFootballDeterministicState& State);

UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Football|Network|Deterministic")
void OnStateCorrection(int32 FrameNumber, const FFootballDeterministicState& CorrectedState);

UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Football|Network|Deterministic")
FFootballInputFrame PackInputs();

UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Football|Network|Deterministic")
void UnpackInputs(const FFootballInputFrame& InputFrame);
```

---

### AFootballDeterministicManager
Central coordinator for deterministic replication.

```cpp
UFUNCTION(BlueprintCallable, Category="Football|Network|Deterministic")
void RegisterEntity(AActor* Entity, UFootballDeterministicReplicationComponent* ReplicationComponent);

UFUNCTION(BlueprintCallable, Category="Football|Network|Deterministic")
void UnregisterEntity(AActor* Entity);

UFUNCTION(BlueprintCallable, Category="Football|Network|Deterministic")
void AdvanceFrame();

UFUNCTION(BlueprintPure, Category="Football|Network|Deterministic")
int32 GetCurrentFrame() const;

UFUNCTION(BlueprintCallable, Category="Football|Network|Deterministic")
void SetServerFrame(int32 Frame);

UFUNCTION(BlueprintCallable, Category="Football|Network|Deterministic")
void AddInputFrame(const FFootballInputFrame& InputFrame);

UFUNCTION(BlueprintCallable, Category="Football|Network|Deterministic")
FFootballInputFrame GetInputFrame(int32 FrameNumber) const;

UFUNCTION(BlueprintCallable, Category="Football|Network|Deterministic")
void SynchronizeAll();

UFUNCTION(BlueprintCallable, Category="Football|Network|Deterministic")
void ForceCorrection(AActor* Entity);

// Replay
UFUNCTION(BlueprintCallable, Category="Football|Network|Replay")
void StartRecording();

UFUNCTION(BlueprintCallable, Category="Football|Network|Replay")
void StopRecording();

UFUNCTION(BlueprintCallable, Category="Football|Network|Replay")
void PlayReplay(int32 StartFrame);
```

---

### UFootballNetworkPredictionComponent
Client-side prediction for responsive gameplay.

```cpp
UPROPERTY(EditAnywhere) bool bEnablePrediction = true;
UPROPERTY(EditAnywhere) float MaxPredictionError = 50.0f;
UPROPERTY(EditAnywhere) float ReconciliationSpeed = 10.0f;

UFUNCTION(BlueprintCallable, Category="Football|Network|Prediction")
void EnablePrediction(bool bEnable);

UFUNCTION(BlueprintCallable, Category="Football|Network|Prediction")
void PredictMovement(float DeltaTime);

UFUNCTION(BlueprintCallable, Category="Football|Network|Prediction")
void ConfirmServerState(const FFootballDeterministicState& ServerState);

UFUNCTION(BlueprintCallable, Category="Football|Network|Prediction")
void ReconcileState();
```

---

### UFootballLagCompensationComponent
Server-side lag compensation.

```cpp
UPROPERTY(EditAnywhere) float MaxRewindTime = 0.5f;
UPROPERTY(EditAnywhere) int32 MaxHistoryFrames = 60;

UFUNCTION(BlueprintCallable, Category="Football|Network|LagCompensation")
void RewindToTime(float ServerTime, float ClientPing);

UFUNCTION(BlueprintCallable, Category="Football|Network|LagCompensation")
void RestoreState();

UFUNCTION(BlueprintCallable, Category="Football|Network|LagCompensation")
bool CheckHitAtTime(AActor* Target, float ServerTime, float ClientPing, FHitResult& OutHit);
```

---

## FootballCoreUI

### AFootballHUD
Main HUD controller managing all widgets.

```cpp
UFUNCTION(BlueprintCallable, Category="Football|HUD")
void ShowMatchWidget();

UFUNCTION(BlueprintCallable, Category="Football|HUD")
void HideMatchWidget();

UFUNCTION(BlueprintCallable, Category="Football|HUD")
void ShowPlayerWidget(AFootballPlayerCharacter* Player);

UFUNCTION(BlueprintCallable, Category="Football|HUD")
void HidePlayerWidget();

UFUNCTION(BlueprintCallable, Category="Football|HUD")
void ShowTacticalWidget();

UFUNCTION(BlueprintCallable, Category="Football|HUD")
void HideTacticalWidget();

UFUNCTION(BlueprintCallable, Category="Football|HUD")
void ShowReplayWidget();

UFUNCTION(BlueprintCallable, Category="Football|HUD")
void HideReplayWidget();

UFUNCTION(BlueprintCallable, Category="Football|HUD")
void ShowMenuWidget(FName MenuName);

UFUNCTION(BlueprintCallable, Category="Football|HUD")
void HideMenuWidget();

// Match Info
UFUNCTION(BlueprintCallable, Category="Football|HUD")
void UpdateScore(int32 HomeScore, int32 AwayScore);

UFUNCTION(BlueprintCallable, Category="Football|HUD")
void UpdateMatchTime(float TimeRemaining);

UFUNCTION(BlueprintCallable, Category="Football|HUD")
void UpdatePossession(float HomePossession, float AwayPossession);

UFUNCTION(BlueprintCallable, Category="Football|HUD")
void ShowGoalScored(EFootballTeamSide::Type ScoringTeam, AFootballPlayerCharacter* Scorer);

UFUNCTION(BlueprintCallable, Category="Football|HUD")
void ShowCard(EFootballTeamSide::Type Team, AFootballPlayerCharacter* Player, bool bYellow);

UFUNCTION(BlueprintCallable, Category="Football|HUD")
void ShowSubstitution(EFootballTeamSide::Type Team, AFootballPlayerCharacter* PlayerOut, AFootballPlayerCharacter* PlayerIn);

// Player Info
UFUNCTION(BlueprintCallable, Category="Football|HUD")
void UpdatePlayerStamina(float StaminaPercent);

UFUNCTION(BlueprintCallable, Category="Football|HUD")
void UpdatePlayerFatigue(bool bFatigued);

UFUNCTION(BlueprintCallable, Category="Football|HUD")
void ShowPlayerAction(FGameplayTag ActionTag, bool bSuccess);

// Tactical
UFUNCTION(BlueprintCallable, Category="Football|HUD")
void UpdateFormation(int32 FormationID);

UFUNCTION(BlueprintCallable, Category="Football|HUD")
void UpdateTeamInstructions(const FFootballTacticalInstructions& Instructions);

UFUNCTION(BlueprintCallable, Category="Football|HUD")
void HighlightPlayer(AFootballPlayerCharacter* Player);

// Replay
UFUNCTION(BlueprintCallable, Category="Football|HUD")
void StartReplay(int32 StartFrame);

UFUNCTION(BlueprintCallable, Category="Football|HUD")
void StopReplay();

// Widget Getters
UFUNCTION(BlueprintPure, Category="Football|HUD")
UFootballMatchWidget* GetMatchWidget() const;

UFUNCTION(BlueprintPure, Category="Football|HUD")
UFootballPlayerWidget* GetPlayerWidget() const;

UFUNCTION(BlueprintPure, Category="Football|HUD")
UFootballTacticalWidget* GetTacticalWidget() const;

UFUNCTION(BlueprintPure, Category="Football|HUD")
UFootballReplayWidget* GetReplayWidget() const;

UFUNCTION(BlueprintPure, Category="Football|HUD")
UFootballMenuWidget* GetMenuWidget() const;
```

---

## Data Structures

### FFootballBallPhysicsParams
```cpp
float Mass = 0.43f;              // kg (FIFA: 410-450g)
float Radius = 0.11f;            // meters (FIFA: 21.65-22.29cm diameter)
float AirDensity = 1.225f;       // kg/m³ at sea level
float DragCoefficient = 0.25f;   // Smooth sphere
float LiftCoefficient = 0.2f;    // Magnus effect coefficient
float Restitution = 0.6f;        // Bounce coefficient
float RollingFriction = 0.02f;   // Rolling friction
float SlidingFriction = 0.1f;    // Sliding friction
float SpinDecayRate = 0.98f;     // Per second
float MaxSpinRate = 50.0f;       // rad/s
float DeformationFactor = 0.1f;  // Visual deformation
bool bUseMagnusEffect = true;
bool bUseDeformation = true;
```

### FFootballPlayerAttributes
Complete attribute system (Physical, Technical, Mental, Goalkeeping).
All attributes 0-100 scale. See `FootballCoreTypes.h` for full list.

### FFootballTacticalInstructions
Team tactical instructions (0-100 scale):
- **Team Shape**: FormationID, DefensiveLine, EngagementLine, Width
- **In Possession**: BuildUpPlay, ChanceCreation, PassingDirectness, CrossingFrequency, ShootOnSight, Tempo, CreativeFreedom
- **Out of Possession**: PressingIntensity, DefensiveShape, Interceptions, TacklingAggression, OffsideTrap
- **Goalkeeper**: GK_RushOut, GK_Distribution

### FFootballMatchStats
Complete match statistics tracking.

---

## Gameplay Tags (FFootballTags)

### Game States
- `Game.Active`, `Game.Paused`, `Game.HalfTime`, `Game.Finished`, `Game.SetPiece`

### Ball States
- `Ball.Free`, `Ball.Possessed`, `Ball.Flying`, `Ball.Rolling`, `Ball.OutOfPlay`, `Ball.Goal`

### Player States
- `Player.Idle`, `Player.Running`, `Player.Sprinting`, `Player.Dribbling`, `Player.Passing`, `Player.Shooting`, `Player.Tackling`, `Player.Intercepting`, `Player.Goalkeeping`, `Player.Celebrating`, `Player.Injured`, `Player.Fatigued`

### Team States
- `Team.Attacking`, `Team.Defending`, `Team.Pressing`, `Team.CounterAttacking`, `Team.Possession`, `Team.SetPieceAttack`, `Team.SetPieceDefense`

### Actions
- `Action.Pass.Short`, `Action.Pass.Long`, `Action.Pass.Through`, `Action.Pass.Cross`
- `Action.Shoot.Power`, `Action.Shoot.Placement`, `Action.Shoot.Chip`, `Action.Shoot.Volley`
- `Action.Dribble`, `Action.SkillMove`
- `Action.Tackle.Standing`, `Action.Tackle.Sliding`
- `Action.Intercept`, `Action.Block`, `Action.Clearance`, `Action.Header`
- `Action.Goalkeeper.Save`, `Action.Goalkeeper.Catch`, `Action.Goalkeeper.Punch`
- `Action.Goalkeeper.Distribute.Throw`, `Action.Goalkeeper.Distribute.Kick`

### Body Parts
- `Body.LeftFoot`, `Body.RightFoot`, `Body.Head`, `Body.Chest`, `Body.LeftThigh`, `Body.RightThigh`, `Body.LeftShoulder`, `Body.RightShoulder`

---

## Enums

### EFootballGameMode
- `TrainingMatch_11v11` - Full 11v11 training match
- `OneVOne_StrikerVsKeeper` - 1v1 striker vs goalkeeper
- `PenaltyShootout` - Penalty shootout mode
- `FreeKick_Practice` - Free kick practice
- `Custom` - Custom mode

### EFootballBallState
- `Free` - Ball free on pitch
- `Possessed` - Controlled by player
- `Flying` - In air
- `Rolling` - Rolling on ground
- `OutOfPlay` - Out of bounds
- `Goal` - In goal

### EFootballPlayerRole
- `Goalkeeper`, `CenterBack`, `LeftBack`, `RightBack`
- `DefensiveMidfielder`, `CentralMidfielder`, `AttackingMidfielder`
- `LeftWinger`, `RightWinger`
- `Striker`, `CenterForward`
- `Substitute`

### EFootballTeamSide
- `Home`, `Away`, `Neutral`

### EFootballMatchPhase
- `KickOff`, `OpenPlay`
- `SetPiece_Corner`, `SetPiece_FreeKick`, `SetPiece_Penalty`, `SetPiece_ThrowIn`, `SetPiece_GoalKick`
- `HalfTime`, `FullTime`

### EFootballMovementMode
- `Walking`, `Jogging`, `Sprinting`, `Dribbling`, `Shielding`, `Jockeying`, `Sliding`, `Jumping`, `Falling`

---

## Blueprints - Key Classes to Extend

| C++ Class | Blueprint Path | Purpose |
|-----------|----------------|---------|
| `AFootballPlayerCharacter` | `/Game/Blueprints/Characters/BP_FootballPlayer` | Base player |
| `AFootballBallActor` | `/Game/Blueprints/Ball/BP_FootballBall` | Ball actor |
| `AFootballTeam` | `/Game/Blueprints/Team/BP_FootballTeam` | Team setup |
| `AFootballGameMode` | `/Game/Blueprints/GameMode/BP_FootballGameMode` | Match rules |
| `AFootballHUD` | `/Game/Blueprints/UI/BP_FootballHUD` | HUD controller |
| `UFootballMatchWidget` | `/Game/Blueprints/UI/Widgets/WBP_MatchWidget` | Match UI |
| `UFootballPlayerWidget` | `/Game/Blueprints/UI/Widgets/WBP_PlayerWidget` | Player UI |
| `UFootballTacticalWidget` | `/Game/Blueprints/UI/Widgets/WBP_TacticalWidget` | Tactical UI |
| `UFootballReplayWidget` | `/Game/Blueprints/UI/Widgets/WBP_ReplayWidget` | Replay UI |
| `UFootballMenuWidget` | `/Game/Blueprints/UI/Widgets/WBP_MenuWidget` | Menu system |

---

## Console Commands

```bash
# Debug commands
Football.DebugBallPhysics 1          # Show ball physics debug
Football.DebugAI 1                   # Show AI state
Football.DebugTactical 1             # Show tactical positions
Football.DebugNetwork 1              # Show network replication
Football.DebugCloth 1                # Show cloth simulation

# Gameplay
Football.SpawnBall                   # Spawn ball at center
Football.ResetMatch                  # Restart current match
Football.SetGameMode 11v11           # Change game mode
Football.ToggleDeterministic         # Toggle deterministic mode
```

---

*API Reference for Craquez FootballCore Plugin - UE5.4+*
*Last Updated: 2024*