// Copyright 2024 Craquez. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "FootballCoreTypes.h"
#include "FootballHUD.generated.h"

class UUserWidget;
class UFootballMatchWidget;
class UFootballPlayerWidget;
class UFootballTacticalWidget;
class UFootballReplayWidget;
class UFootballMenuWidget;
class AFootballGameMode;
class AFootballTeam;
class AFootballPlayerCharacter;

/**
 * Football HUD - Main HUD class for the football game
 * Handles all UI widgets and match display
 */
UCLASS(Blueprintable, BlueprintType)
class FOOTBALLCOREUI_API AFootballHUD : public AHUD
{
    GENERATED_BODY()

public:
    AFootballHUD();

    virtual void BeginPlay() override;
    virtual void DrawHUD() override;
    virtual void Tick(float DeltaTime) override;
    virtual void NotifyHitBoxClick(FName BoxName) override;

    // Widget Management
    UFUNCTION(BlueprintCallable, Category = "Football|HUD")
    void ShowMatchWidget();

    UFUNCTION(BlueprintCallable, Category = "Football|HUD")
    void HideMatchWidget();

    UFUNCTION(BlueprintCallable, Category = "Football|HUD")
    void ShowPlayerWidget(AFootballPlayerCharacter* Player);

    UFUNCTION(BlueprintCallable, Category = "Football|HUD")
    void HidePlayerWidget();

    UFUNCTION(BlueprintCallable, Category = "Football|HUD")
    void ShowTacticalWidget();

    UFUNCTION(BlueprintCallable, Category = "Football|HUD")
    void HideTacticalWidget();

    UFUNCTION(BlueprintCallable, Category = "Football|HUD")
    void ShowReplayWidget();

    UFUNCTION(BlueprintCallable, Category = "Football|HUD")
    void HideReplayWidget();

    UFUNCTION(BlueprintCallable, Category = "Football|HUD")
    void ShowMenuWidget(FName MenuName);

    UFUNCTION(BlueprintCallable, Category = "Football|HUD")
    void HideMenuWidget();

    // Match Info
    UFUNCTION(BlueprintCallable, Category = "Football|HUD")
    void UpdateScore(int32 HomeScore, int32 AwayScore);

    UFUNCTION(BlueprintCallable, Category = "Football|HUD")
    void UpdateMatchTime(float TimeRemaining);

    UFUNCTION(BlueprintCallable, Category = "Football|HUD")
    void UpdatePossession(float HomePossession, float AwayPossession);

    UFUNCTION(BlueprintCallable, Category = "Football|HUD")
    void ShowGoalScored(EFootballTeamSide::Type ScoringTeam, AFootballPlayerCharacter* Scorer);

    UFUNCTION(BlueprintCallable, Category = "Football|HUD")
    void ShowCard(EFootballTeamSide::Type Team, AFootballPlayerCharacter* Player, bool bYellow);

    UFUNCTION(BlueprintCallable, Category = "Football|HUD")
    void ShowSubstitution(EFootballTeamSide::Type Team, AFootballPlayerCharacter* PlayerOut,
                          AFootballPlayerCharacter* PlayerIn);

    // Player Info
    UFUNCTION(BlueprintCallable, Category = "Football|HUD")
    void UpdatePlayerStamina(float StaminaPercent);

    UFUNCTION(BlueprintCallable, Category = "Football|HUD")
    void UpdatePlayerFatigue(bool bFatigued);

    UFUNCTION(BlueprintCallable, Category = "Football|HUD")
    void ShowPlayerAction(FGameplayTag ActionTag, bool bSuccess);

    // Tactical
    UFUNCTION(BlueprintCallable, Category = "Football|HUD")
    void UpdateFormation(int32 FormationID);

    UFUNCTION(BlueprintCallable, Category = "Football|HUD")
    void UpdateTeamInstructions(const FFootballTacticalInstructions& Instructions);

    UFUNCTION(BlueprintCallable, Category = "Football|HUD")
    void HighlightPlayer(AFootballPlayerCharacter* Player);

    // Replay
    UFUNCTION(BlueprintCallable, Category = "Football|HUD")
    void StartReplay(int32 StartFrame);

    UFUNCTION(BlueprintCallable, Category = "Football|HUD")
    void StopReplay();

    // Widget Getters
    UFUNCTION(BlueprintPure, Category = "Football|HUD")
    UFootballMatchWidget* GetMatchWidget() const { return MatchWidget; }

    UFUNCTION(BlueprintPure, Category = "Football|HUD")
    UFootballPlayerWidget* GetPlayerWidget() const { return PlayerWidget; }

    UFUNCTION(BlueprintPure, Category = "Football|HUD")
    UFootballTacticalWidget* GetTacticalWidget() const { return TacticalWidget; }

    UFUNCTION(BlueprintPure, Category = "Football|HUD")
    UFootballReplayWidget* GetReplayWidget() const { return ReplayWidget; }

    UFUNCTION(BlueprintPure, Category = "Football|HUD")
    UFootballMenuWidget* GetMenuWidget() const { return MenuWidget; }

protected:
    // Widget Classes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|HUD|Widgets", meta = (AllowPrivateAccess = "true"))
    TSubclassOf<UFootballMatchWidget> MatchWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|HUD|Widgets", meta = (AllowPrivateAccess = "true"))
    TSubclassOf<UFootballPlayerWidget> PlayerWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|HUD|Widgets", meta = (AllowPrivateAccess = "true"))
    TSubclassOf<UFootballTacticalWidget> TacticalWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|HUD|Widgets", meta = (AllowPrivateAccess = "true"))
    TSubclassOf<UFootballReplayWidget> ReplayWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|HUD|Widgets", meta = (AllowPrivateAccess = "true"))
    TSubclassOf<UFootballMenuWidget> MenuWidgetClass;

    // Widget Instances
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|HUD|Widgets",
              meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UFootballMatchWidget> MatchWidget = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|HUD|Widgets",
              meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UFootballPlayerWidget> PlayerWidget = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|HUD|Widgets",
              meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UFootballTacticalWidget> TacticalWidget = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|HUD|Widgets",
              meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UFootballReplayWidget> ReplayWidget = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|HUD|Widgets",
              meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UFootballMenuWidget> MenuWidget = nullptr;

    // References
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|HUD", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<AFootballGameMode> GameMode = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|HUD", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<AFootballTeam> HomeTeam = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|HUD", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<AFootballTeam> AwayTeam = nullptr;

    // State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|HUD", meta = (AllowPrivateAccess = "true"))
    bool bMatchWidgetVisible = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Football|HUD", meta = (AllowPrivateAccess = "true"))
    bool bTacticalWidgetVisible = false;

private:
    void CreateWidgets();
    void InitializeWidgetReferences();
};