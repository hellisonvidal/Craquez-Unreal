// Copyright 2024 Craquez. All Rights Reserved.

#include "Widgets/FootballMatchWidget.h"
#include "Match/FootballGameMode.h"
#include "Team/FootballTeam.h"
#include "Player/FootballPlayerCharacter.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"
#include "Animation/WidgetAnimation.h"
#include "Kismet/KismetTextLibrary.h"

void UFootballMatchWidget::NativeConstruct()
{
    Super::NativeConstruct();

    UpdateScoreDisplay();
    UpdateTimeDisplay();
    UpdatePossessionDisplay();
}

void UFootballMatchWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (GameMode)
    {
        UpdateMatchTime(GameMode->GetMatchTimeRemaining());
    }
}

void UFootballMatchWidget::SetGameMode(AFootballGameMode* InGameMode)
{
    GameMode = InGameMode;

    if (GameMode)
    {
        HomeTeam = GameMode->GetHomeTeam();
        AwayTeam = GameMode->GetAwayTeam();
        RefreshTeamNames();
        RefreshTeamCrests();
    }
}

void UFootballMatchWidget::SetTeams(AFootballTeam* InHomeTeam, AFootballTeam* InAwayTeam)
{
    HomeTeam = InHomeTeam;
    AwayTeam = InAwayTeam;
    RefreshTeamNames();
    RefreshTeamCrests();
}

void UFootballMatchWidget::UpdateScore(int32 HomeScore, int32 AwayScore)
{
    if (HomeScoreText)
    {
        HomeScoreText->SetText(FText::AsNumber(HomeScore));
    }
    if (AwayScoreText)
    {
        AwayScoreText->SetText(FText::AsNumber(AwayScore));
    }
}

void UFootballMatchWidget::UpdateMatchTime(float TimeRemaining)
{
    if (MatchTimeText)
    {
        MatchTimeText->SetText(FText::FromString(FormatTime(TimeRemaining)));
    }

    if (GameMode && MatchPhaseText)
    {
        EFootballMatchPhase::Type Phase = GameMode->GetCurrentPhase();
        const UEnum* PhaseEnum = FindObject<UEnum>(nullptr, TEXT("/Script/FootballCore.EFootballMatchPhase"));
        FText PhaseText =
            PhaseEnum ? PhaseEnum->GetDisplayNameTextByValue(static_cast<int64>(Phase)) : FText::GetEmpty();
        MatchPhaseText->SetText(PhaseText);
    }
}

void UFootballMatchWidget::UpdatePossession(float HomePossession, float AwayPossession)
{
    if (PossessionBar)
    {
        float Total = HomePossession + AwayPossession;
        if (Total > 0.0f)
        {
            PossessionBar->SetPercent(HomePossession / Total);
        }
    }

    if (HomePossessionText)
    {
        HomePossessionText->SetText(FText::FromString(FString::Printf(TEXT("%.0f%%"), HomePossession)));
    }
    if (AwayPossessionText)
    {
        AwayPossessionText->SetText(FText::FromString(FString::Printf(TEXT("%.0f%%"), AwayPossession)));
    }
}

void UFootballMatchWidget::ShowGoalScored(EFootballTeamSide::Type ScoringTeam, AFootballPlayerCharacter* Scorer)
{
    CreateGoalEvent(ScoringTeam, Scorer);

    if (GoalAnimation)
    {
        PlayAnimation(GoalAnimation);
    }
}

void UFootballMatchWidget::ShowCard(EFootballTeamSide::Type Team, AFootballPlayerCharacter* Player, bool bYellow)
{
    CreateCardEvent(Team, Player, bYellow);

    if (CardAnimation)
    {
        PlayAnimation(CardAnimation);
    }
}

void UFootballMatchWidget::ShowSubstitution(EFootballTeamSide::Type Team, AFootballPlayerCharacter* PlayerOut,
                                            AFootballPlayerCharacter* PlayerIn)
{
    CreateSubstitutionEvent(Team, PlayerOut, PlayerIn);

    if (SubstitutionAnimation)
    {
        PlayAnimation(SubstitutionAnimation);
    }
}

void UFootballMatchWidget::UpdateTeamNames(const FString& HomeName, const FString& AwayName)
{
    if (HomeNameText)
    {
        HomeNameText->SetText(FText::FromString(HomeName));
    }
    if (AwayNameText)
    {
        AwayNameText->SetText(FText::FromString(AwayName));
    }
}

void UFootballMatchWidget::UpdateTeamCrests(UTexture2D* HomeCrest, UTexture2D* AwayCrest)
{
    if (HomeCrestImage && HomeCrest)
    {
        HomeCrestImage->SetBrushFromTexture(HomeCrest);
    }
    if (AwayCrestImage && AwayCrest)
    {
        AwayCrestImage->SetBrushFromTexture(AwayCrest);
    }
}

void UFootballMatchWidget::UpdateScoreDisplay()
{
    if (GameMode)
    {
        UpdateScore(GameMode->GetHomeScore(), GameMode->GetAwayScore());
    }
}

void UFootballMatchWidget::UpdateTimeDisplay()
{
    if (GameMode)
    {
        UpdateMatchTime(GameMode->GetMatchTimeRemaining());
    }
}

void UFootballMatchWidget::UpdatePossessionDisplay()
{
    if (HomeTeam && AwayTeam)
    {
        float HomePoss = HomeTeam->GetPossessionPercent();
        float AwayPoss = AwayTeam->GetPossessionPercent();
        UpdatePossession(HomePoss, AwayPoss);
    }
}

void UFootballMatchWidget::RefreshTeamNames()
{
    if (HomeTeam && AwayTeam)
    {
        UpdateTeamNames(HomeTeam->GetTeamName(), AwayTeam->GetTeamName());
    }
}

void UFootballMatchWidget::RefreshTeamCrests()
{
    if (HomeTeam && AwayTeam)
    {
        UpdateTeamCrests(HomeTeam->GetTeamCrest(), AwayTeam->GetTeamCrest());
    }
}

void UFootballMatchWidget::CreateGoalEvent(EFootballTeamSide::Type ScoringTeam, AFootballPlayerCharacter* Scorer)
{
    if (!GoalEventWidgetClass || !EventsContainer)
    {
        return;
    }

    UUserWidget* EventWidget = CreateWidget<UUserWidget>(GetWorld(), GoalEventWidgetClass);
    if (EventWidget)
    {
        // Set event data on widget (would use BindWidget or custom setup)
        EventsContainer->AddChildToVerticalBox(EventWidget);

        // Animate in
        // Would play entry animation

        // Remove after delay
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(
            TimerHandle,
            [EventWidget]()
            {
                if (EventWidget)
                {
                    EventWidget->RemoveFromParent();
                }
            },
            5.0f, false);
    }
}

void UFootballMatchWidget::CreateCardEvent(EFootballTeamSide::Type Team, AFootballPlayerCharacter* Player, bool bYellow)
{
    if (!CardEventWidgetClass || !EventsContainer)
    {
        return;
    }

    UUserWidget* EventWidget = CreateWidget<UUserWidget>(GetWorld(), CardEventWidgetClass);
    if (EventWidget)
    {
        EventsContainer->AddChildToVerticalBox(EventWidget);

        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(
            TimerHandle,
            [EventWidget]()
            {
                if (EventWidget)
                {
                    EventWidget->RemoveFromParent();
                }
            },
            5.0f, false);
    }
}

void UFootballMatchWidget::CreateSubstitutionEvent(EFootballTeamSide::Type Team, AFootballPlayerCharacter* PlayerOut,
                                                   AFootballPlayerCharacter* PlayerIn)
{
    if (!SubstitutionEventWidgetClass || !EventsContainer)
    {
        return;
    }

    UUserWidget* EventWidget = CreateWidget<UUserWidget>(GetWorld(), SubstitutionEventWidgetClass);
    if (EventWidget)
    {
        EventsContainer->AddChildToVerticalBox(EventWidget);

        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(
            TimerHandle,
            [EventWidget]()
            {
                if (EventWidget)
                {
                    EventWidget->RemoveFromParent();
                }
            },
            5.0f, false);
    }
}

FString UFootballMatchWidget::FormatTime(float Seconds) const
{
    int32 Minutes = FMath::FloorToInt(Seconds / 60.0f);
    int32 Secs = FMath::FloorToInt(Seconds) % 60;
    return FString::Printf(TEXT("%02d:%02d"), Minutes, Secs);
}