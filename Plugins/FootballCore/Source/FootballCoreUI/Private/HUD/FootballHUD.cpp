// Copyright 2024 Craquez. All Rights Reserved.

#include "HUD/FootballHUD.h"
#include "Widgets/FootballMatchWidget.h"
#include "Widgets/FootballPlayerWidget.h"
#include "Widgets/FootballTacticalWidget.h"
#include "Widgets/FootballReplayWidget.h"
#include "Widgets/FootballMenuWidget.h"
#include "Match/FootballGameMode.h"
#include "Team/FootballTeam.h"
#include "Player/FootballPlayerCharacter.h"
#include "Kismet/GameplayStatics.h"

AFootballHUD::AFootballHUD()
{
}

void AFootballHUD::BeginPlay()
{
    Super::BeginPlay();

    // Find game mode
    GameMode = Cast<AFootballGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
    if (GameMode)
    {
        HomeTeam = GameMode->GetHomeTeam();
        AwayTeam = GameMode->GetAwayTeam();
    }

    CreateWidgets();
    InitializeWidgetReferences();

    // Show match widget by default
    ShowMatchWidget();
}

void AFootballHUD::DrawHUD()
{
    Super::DrawHUD();

    // Custom drawing for pitch lines, etc. if needed
}

void AFootballHUD::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update widgets
    if (MatchWidget && bMatchWidgetVisible)
    {
        // Match widget updates itself via bindings
    }

    if (PlayerWidget)
    {
        // Player widget updates via bindings
    }
}

void AFootballHUD::CreateWidgets()
{
    if (MatchWidgetClass)
    {
        MatchWidget = CreateWidget<UFootballMatchWidget>(GetWorld(), MatchWidgetClass);
        if (MatchWidget)
        {
            MatchWidget->AddToViewport(0);
            MatchWidget->SetVisibility(ESlateVisibility::Hidden);
        }
    }

    if (PlayerWidgetClass)
    {
        PlayerWidget = CreateWidget<UFootballPlayerWidget>(GetWorld(), PlayerWidgetClass);
        if (PlayerWidget)
        {
            PlayerWidget->AddToViewport(1);
            PlayerWidget->SetVisibility(ESlateVisibility::Hidden);
        }
    }

    if (TacticalWidgetClass)
    {
        TacticalWidget = CreateWidget<UFootballTacticalWidget>(GetWorld(), TacticalWidgetClass);
        if (TacticalWidget)
        {
            TacticalWidget->AddToViewport(2);
            TacticalWidget->SetVisibility(ESlateVisibility::Hidden);
        }
    }

    if (ReplayWidgetClass)
    {
        ReplayWidget = CreateWidget<UFootballReplayWidget>(GetWorld(), ReplayWidgetClass);
        if (ReplayWidget)
        {
            ReplayWidget->AddToViewport(10);
            ReplayWidget->SetVisibility(ESlateVisibility::Hidden);
        }
    }

    if (MenuWidgetClass)
    {
        MenuWidget = CreateWidget<UFootballMenuWidget>(GetWorld(), MenuWidgetClass);
        if (MenuWidget)
        {
            MenuWidget->AddToViewport(100);
            MenuWidget->SetVisibility(ESlateVisibility::Hidden);
        }
    }
}

void AFootballHUD::InitializeWidgetReferences()
{
    if (MatchWidget && GameMode)
    {
        MatchWidget->SetGameMode(GameMode);
        MatchWidget->SetTeams(HomeTeam, AwayTeam);
    }

    if (PlayerWidget)
    {
        // Will be set when showing player widget
    }

    if (TacticalWidget)
    {
        if (HomeTeam) TacticalWidget->SetTeam(HomeTeam);
        if (AwayTeam) TacticalWidget->SetTeam(AwayTeam);
    }

    if (ReplayWidget && GameMode)
    {
        ReplayWidget->SetGameMode(GameMode);
    }

    if (MenuWidget)
    {
        MenuWidget->SetHUD(this);
    }
}

void AFootballHUD::ShowMatchWidget()
{
    if (MatchWidget)
    {
        MatchWidget->SetVisibility(ESlateVisibility::Visible);
        bMatchWidgetVisible = true;
    }
}

void AFootballHUD::HideMatchWidget()
{
    if (MatchWidget)
    {
        MatchWidget->SetVisibility(ESlateVisibility::Hidden);
        bMatchWidgetVisible = false;
    }
}

void AFootballHUD::ShowPlayerWidget(AFootballPlayerCharacter* Player)
{
    if (PlayerWidget && Player)
    {
        PlayerWidget->SetPlayer(Player);
        PlayerWidget->SetVisibility(ESlateVisibility::Visible);
    }
}

void AFootballHUD::HidePlayerWidget()
{
    if (PlayerWidget)
    {
        PlayerWidget->SetVisibility(ESlateVisibility::Hidden);
    }
}

void AFootballHUD::ShowTacticalWidget()
{
    if (TacticalWidget)
    {
        TacticalWidget->SetVisibility(ESlateVisibility::Visible);
        bTacticalWidgetVisible = true;
    }
}

void AFootballHUD::HideTacticalWidget()
{
    if (TacticalWidget)
    {
        TacticalWidget->SetVisibility(ESlateVisibility::Hidden);
        bTacticalWidgetVisible = false;
    }
}

void AFootballHUD::ShowReplayWidget()
{
    if (ReplayWidget)
    {
        ReplayWidget->SetVisibility(ESlateVisibility::Visible);
    }
}

void AFootballHUD::HideReplayWidget()
{
    if (ReplayWidget)
    {
        ReplayWidget->SetVisibility(ESlateVisibility::Hidden);
    }
}

void AFootballHUD::ShowMenuWidget(FName MenuName)
{
    if (MenuWidget)
    {
        MenuWidget->ShowMenu(MenuName);
        MenuWidget->SetVisibility(ESlateVisibility::Visible);
    }
}

void AFootballHUD::HideMenuWidget()
{
    if (MenuWidget)
    {
        MenuWidget->HideMenu();
        MenuWidget->SetVisibility(ESlateVisibility::Hidden);
    }
}

void AFootballHUD::UpdateScore(int32 HomeScore, int32 AwayScore)
{
    if (MatchWidget)
    {
        MatchWidget->UpdateScore(HomeScore, AwayScore);
    }
}

void AFootballHUD::UpdateMatchTime(float TimeRemaining)
{
    if (MatchWidget)
    {
        MatchWidget->UpdateMatchTime(TimeRemaining);
    }
}

void AFootballHUD::UpdatePossession(float HomePossession, float AwayPossession)
{
    if (MatchWidget)
    {
        MatchWidget->UpdatePossession(HomePossession, AwayPossession);
    }
}

void AFootballHUD::ShowGoalScored(EFootballTeamSide::Type ScoringTeam, AFootballPlayerCharacter* Scorer)
{
    if (MatchWidget)
    {
        MatchWidget->ShowGoalScored(ScoringTeam, Scorer);
    }
}

void AFootballHUD::ShowCard(EFootballTeamSide::Type Team, AFootballPlayerCharacter* Player, bool bYellow)
{
    if (MatchWidget)
    {
        MatchWidget->ShowCard(Team, Player, bYellow);
    }
}

void AFootballHUD::ShowSubstitution(EFootballTeamSide::Type Team, AFootballPlayerCharacter* PlayerOut, AFootballPlayerCharacter* PlayerIn)
{
    if (MatchWidget)
    {
        MatchWidget->ShowSubstitution(Team, PlayerOut, PlayerIn);
    }
}

void AFootballHUD::UpdatePlayerStamina(float StaminaPercent)
{
    if (PlayerWidget)
    {
        PlayerWidget->UpdateStamina(StaminaPercent);
    }
}

void AFootballHUD::UpdatePlayerFatigue(bool bFatigued)
{
    if (PlayerWidget)
    {
        PlayerWidget->UpdateFatigue(bFatigued);
    }
}

void AFootballHUD::ShowPlayerAction(FGameplayTag ActionTag, bool bSuccess)
{
    if (PlayerWidget)
    {
        PlayerWidget->ShowAction(ActionTag, bSuccess);
    }
}

void AFootballHUD::UpdateFormation(int32 FormationID)
{
    if (TacticalWidget)
    {
        TacticalWidget->UpdateFormation(FormationID);
    }
}

void AFootballHUD::UpdateTeamInstructions(const FFootballTacticalInstructions& Instructions)
{
    if (TacticalWidget)
    {
        TacticalWidget->UpdateInstructions(Instructions);
    }
}

void AFootballHUD::HighlightPlayer(AFootballPlayerCharacter* Player)
{
    if (TacticalWidget)
    {
        TacticalWidget->HighlightPlayer(Player);
    }
}

void AFootballHUD::StartReplay(int32 StartFrame)
{
    if (ReplayWidget)
    {
        ReplayWidget->StartReplay(StartFrame);
        ShowReplayWidget();
    }
}

void AFootballHUD::StopReplay()
{
    if (ReplayWidget)
    {
        ReplayWidget->StopReplay();
        HideReplayWidget();
    }
}