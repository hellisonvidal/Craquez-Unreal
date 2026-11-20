// Copyright 2024 Craquez. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FootballCoreTypes.h"
#include "FootballMatchWidget.generated.h"

class AFootballGameMode;
class AFootballTeam;
class AFootballPlayerCharacter;
class UProgressBar;
class UTextBlock;
class UImage;
class UVerticalBox;
class UHorizontalBox;
class UWidgetAnimation;

/**
 * Football Match Widget - Main match UI showing score, time, possession, events
 */
UCLASS(Blueprintable, BlueprintType)
class FOOTBALLCOREUI_API UFootballMatchWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    // Setup
    UFUNCTION(BlueprintCallable, Category = "Football|UI")
    void SetGameMode(AFootballGameMode* InGameMode);

    UFUNCTION(BlueprintCallable, Category = "Football|UI")
    void SetTeams(AFootballTeam* InHomeTeam, AFootballTeam* InAwayTeam);

    // Score
    UFUNCTION(BlueprintCallable, Category = "Football|UI")
    void UpdateScore(int32 HomeScore, int32 AwayScore);

    // Time
    UFUNCTION(BlueprintCallable, Category = "Football|UI")
    void UpdateMatchTime(float TimeRemaining);

    // Possession
    UFUNCTION(BlueprintCallable, Category = "Football|UI")
    void UpdatePossession(float HomePossession, float AwayPossession);

    // Events
    UFUNCTION(BlueprintCallable, Category = "Football|UI")
    void ShowGoalScored(EFootballTeamSide::Type ScoringTeam, AFootballPlayerCharacter* Scorer);

    UFUNCTION(BlueprintCallable, Category = "Football|UI")
    void ShowCard(EFootballTeamSide::Type Team, AFootballPlayerCharacter* Player, bool bYellow);

    UFUNCTION(BlueprintCallable, Category = "Football|UI")
    void ShowSubstitution(EFootballTeamSide::Type Team, AFootballPlayerCharacter* PlayerOut, AFootballPlayerCharacter* PlayerIn);

    // Team Info
    UFUNCTION(BlueprintCallable, Category = "Football|UI")
    void UpdateTeamNames(const FString& HomeName, const FString& AwayName);

    UFUNCTION(BlueprintCallable, Category = "Football|UI")
    void UpdateTeamCrests(UTexture2D* HomeCrest, UTexture2D* AwayCrest);

protected:
    // Game Mode Reference
    UPROPERTY()
    TObjectPtr<AFootballGameMode> GameMode = nullptr;

    UPROPERTY()
    TObjectPtr<AFootballTeam> HomeTeam = nullptr;

    UPROPERTY()
    TObjectPtr<AFootballTeam> AwayTeam = nullptr;

    // UI Elements - Score
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> HomeScoreText = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> AwayScoreText = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> HomeCrestImage = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> AwayCrestImage = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> HomeNameText = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> AwayNameText = nullptr;

    // UI Elements - Time
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> MatchTimeText = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> MatchPhaseText = nullptr;

    // UI Elements - Possession
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UProgressBar> PossessionBar = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> HomePossessionText = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> AwayPossessionText = nullptr;

    // UI Elements - Events
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UVerticalBox> EventsContainer = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UWidgetAnimation> GoalAnimation = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UWidgetAnimation> CardAnimation = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UWidgetAnimation> SubstitutionAnimation = nullptr;

    // Event Widget Class
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|UI")
    TSubclassOf<UUserWidget> GoalEventWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|UI")
    TSubclassOf<UUserWidget> CardEventWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|UI")
    TSubclassOf<UUserWidget> SubstitutionEventWidgetClass;

    // Colors
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|UI|Colors")
    FLinearColor HomeColor = FLinearColor::Blue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|UI|Colors")
    FLinearColor AwayColor = FLinearColor::Red;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|UI|Colors")
    FLinearColor NeutralColor = FLinearColor::White;

private:
    void UpdateScoreDisplay();
    void UpdateTimeDisplay();
    void UpdatePossessionDisplay();
    void CreateGoalEvent(EFootballTeamSide::Type ScoringTeam, AFootballPlayerCharacter* Scorer);
    void CreateCardEvent(EFootballTeamSide::Type Team, AFootballPlayerCharacter* Player, bool bYellow);
    void CreateSubstitutionEvent(EFootballTeamSide::Type Team, AFootballPlayerCharacter* PlayerOut, AFootballPlayerCharacter* PlayerIn);
    FString FormatTime(float Seconds) const;
};