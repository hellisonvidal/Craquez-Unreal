// Copyright 2024 Craquez. All Rights Reserved.

#include "HUD/FootballHUD.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Match/FootballGameMode.h"
#include "Player/FootballPlayerCharacter.h"
#include "Team/FootballTeam.h"
#include "Widgets/FootballMatchWidget.h"
#include "Widgets/FootballMenuWidget.h"
#include "Widgets/FootballPlayerWidget.h"
#include "Widgets/FootballReplayWidget.h"
#include "Widgets/FootballTacticalWidget.h"

AFootballHUD::AFootballHUD() {}

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

    if (!Canvas || MatchWidget)
    {
        return;
    }

    const float W = Canvas->ClipX;
    const float H = Canvas->ClipY;
    const float Scale = FMath::Clamp(H / 720.0f, 0.8f, 2.2f);
    UFont* Font = GEngine ? GEngine->GetMediumFont() : nullptr;

    // 1. Top Broadcast Scoreboard Bar (vivid full-color overlay)
    const float BarW = 540.0f * Scale;
    const float BarH = 56.0f * Scale;
    const float BarX = (W - BarW) * 0.5f;
    const float BarY = 18.0f * Scale;

    DrawRect(FLinearColor(0.04f, 0.08f, 0.16f, 0.86f), BarX, BarY, BarW, BarH);
    DrawRect(FLinearColor(0.95f, 0.76f, 0.04f, 1.0f), BarX, BarY, 10.0f * Scale, BarH); // Home Yellow accent
    DrawRect(FLinearColor(0.86f, 0.12f, 0.16f, 1.0f), BarX + BarW - 10.0f * Scale, BarY, 10.0f * Scale,
             BarH); // Away Red accent

    const int32 HomeGoals = GameMode ? GameMode->GetHomeScore() : 0;
    const int32 AwayGoals = GameMode ? GameMode->GetAwayScore() : 0;
    const float Elapsed = GameMode ? GameMode->GetMatchTimeElapsed() : 0.0f;
    const int32 Mins = FMath::FloorToInt(Elapsed / 60.0f);
    const int32 Secs = FMath::FloorToInt(FMath::Fmod(Elapsed, 60.0f));
    const bool bIs1v1 = GameMode && GameMode->GetGameMode() == EFootballGameMode::OneVOne_StrikerVsKeeper;

    const FString ScoreLine =
        FString::Printf(TEXT("CRAQUEZ FC   %d  -  %d   RIVALS FC   |   %02d:%02d   |   %s"), HomeGoals, AwayGoals, Mins,
                        Secs, bIs1v1 ? TEXT("1v1 DUELO") : TEXT("11v11 TREINO"));
    DrawText(ScoreLine, FLinearColor::White, BarX + 24.0f * Scale, BarY + 16.0f * Scale, Font, 1.25f * Scale, false);

    // 2. Bottom-Left Player Stamina + Touch D-Pad
    APlayerController* PC = GetOwningPlayerController();
    AFootballPlayerCharacter* Player = PC ? Cast<AFootballPlayerCharacter>(PC->GetPawn()) : nullptr;
    const float StaminaPct = Player ? FMath::Clamp(Player->GetStaminaPercent() / 100.0f, 0.0f, 1.0f) : 1.0f;
    const FLinearColor StaminaCol = (StaminaPct > 0.5f)
                                        ? FLinearColor(0.08f, 0.85f, 0.35f, 1.0f)
                                        : ((StaminaPct > 0.25f) ? FLinearColor(0.98f, 0.78f, 0.05f, 1.0f)
                                                                : FLinearColor(0.92f, 0.18f, 0.18f, 1.0f));

    const float StamX = 24.0f * Scale;
    const float StamY = H - 52.0f * Scale;
    const float StamW = 210.0f * Scale;
    const float StamH = 22.0f * Scale;
    DrawRect(FLinearColor(0.05f, 0.09f, 0.14f, 0.82f), StamX - 6.0f * Scale, StamY - 24.0f * Scale,
             StamW + 12.0f * Scale, StamH + 32.0f * Scale);
    DrawText(TEXT("ESTAMINA"), FLinearColor(0.85f, 0.95f, 1.0f, 1.0f), StamX, StamY - 21.0f * Scale, Font,
             0.95f * Scale, false);
    DrawRect(FLinearColor(0.15f, 0.18f, 0.22f, 0.9f), StamX, StamY, StamW, StamH);
    DrawRect(StaminaCol, StamX, StamY, StamW * StaminaPct, StamH);

    // Helper to draw colorful interactive touch buttons
    auto DrawActionButton = [this, Font, Scale](FName Id, const FString& Label, float X, float Y, float BW, float BH,
                                                const FLinearColor& FillColor)
    {
        DrawRect(FillColor, X, Y, BW, BH);
        DrawRect(FLinearColor(1.0f, 1.0f, 1.0f, 0.25f), X, Y, BW, 4.0f * Scale);
        DrawText(Label, FLinearColor::White, X + 14.0f * Scale, Y + BH * 0.32f, Font, 1.1f * Scale, false);
        AddHitBox(FVector2D(X, Y), FVector2D(BW, BH), Id, true, 10);
    };

    // Touch D-Pad (bottom-left above stamina)
    const float PadSize = 56.0f * Scale;
    const float PadBaseX = 88.0f * Scale;
    const float PadBaseY = H - 205.0f * Scale;
    const FLinearColor PadCol(0.12f, 0.24f, 0.40f, 0.78f);
    DrawActionButton(FName("MoveUp"), TEXT(" ^"), PadBaseX, PadBaseY, PadSize, PadSize, PadCol);
    DrawActionButton(FName("MoveDown"), TEXT(" v"), PadBaseX, PadBaseY + PadSize * 1.15f, PadSize, PadSize, PadCol);
    DrawActionButton(FName("MoveLeft"), TEXT(" <"), PadBaseX - PadSize * 1.15f, PadBaseY + PadSize * 0.58f, PadSize,
                     PadSize, PadCol);
    DrawActionButton(FName("MoveRight"), TEXT(" >"), PadBaseX + PadSize * 1.15f, PadBaseY + PadSize * 0.58f, PadSize,
                     PadSize, PadCol);

    // 3. Bottom-Right Colorful Action Buttons (Android & Mouse clickable)
    const float BtnW = 122.0f * Scale;
    const float BtnH = 54.0f * Scale;
    const float Gap = 14.0f * Scale;
    const float RightX = W - BtnW - 24.0f * Scale;
    const float RightX2 = RightX - BtnW - Gap;
    const float Row1Y = H - BtnH * 2.0f - Gap - 26.0f * Scale;
    const float Row2Y = H - BtnH - 26.0f * Scale;

    DrawActionButton(FName("BtnPass"), TEXT("PASSAR"), RightX2, Row2Y, BtnW, BtnH,
                     FLinearColor(0.10f, 0.48f, 0.88f, 0.90f));
    DrawActionButton(FName("BtnShoot"), TEXT("CHUTAR"), RightX, Row2Y, BtnW, BtnH,
                     FLinearColor(0.95f, 0.52f, 0.04f, 0.92f));
    DrawActionButton(FName("BtnSprint"), TEXT("SPRINT"), RightX2, Row1Y, BtnW, BtnH,
                     FLinearColor(0.06f, 0.70f, 0.34f, 0.90f));
    DrawActionButton(FName("BtnTackle"), TEXT("DESARME"), RightX, Row1Y, BtnW, BtnH,
                     FLinearColor(0.84f, 0.16f, 0.20f, 0.90f));

    // Top-Right Mode Switch Button (11v11 <-> 1v1)
    DrawActionButton(FName("BtnMode"), bIs1v1 ? TEXT("IR 11v11") : TEXT("IR 1v1"), W - 150.0f * Scale, 18.0f * Scale,
                     130.0f * Scale, 44.0f * Scale, FLinearColor(0.48f, 0.16f, 0.76f, 0.90f));
}

void AFootballHUD::NotifyHitBoxClick(FName BoxName)
{
    Super::NotifyHitBoxClick(BoxName);

    APlayerController* PC = GetOwningPlayerController();
    AFootballPlayerCharacter* Player = PC ? Cast<AFootballPlayerCharacter>(PC->GetPawn()) : nullptr;

    if (BoxName == FName("BtnMode") && GameMode)
    {
        GameMode->ToggleTrainingOrDuelMode();
        return;
    }

    if (!Player)
    {
        return;
    }

    if (BoxName == FName("BtnPass"))
    {
        Player->TriggerPass();
    }
    else if (BoxName == FName("BtnShoot"))
    {
        Player->TriggerShoot();
    }
    else if (BoxName == FName("BtnSprint"))
    {
        Player->TriggerToggleSprint();
    }
    else if (BoxName == FName("BtnTackle"))
    {
        Player->TriggerTackle();
    }
    else if (BoxName == FName("MoveUp"))
    {
        Player->AddMovementInput(FVector(0.0f, -1.0f, 0.0f), 1.0f);
    }
    else if (BoxName == FName("MoveDown"))
    {
        Player->AddMovementInput(FVector(0.0f, 1.0f, 0.0f), 1.0f);
    }
    else if (BoxName == FName("MoveLeft"))
    {
        Player->AddMovementInput(FVector(-1.0f, 0.0f, 0.0f), 1.0f);
    }
    else if (BoxName == FName("MoveRight"))
    {
        Player->AddMovementInput(FVector(1.0f, 0.0f, 0.0f), 1.0f);
    }
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
        if (HomeTeam)
            TacticalWidget->SetTeam(HomeTeam);
        if (AwayTeam)
            TacticalWidget->SetTeam(AwayTeam);
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

void AFootballHUD::ShowSubstitution(EFootballTeamSide::Type Team, AFootballPlayerCharacter* PlayerOut,
                                    AFootballPlayerCharacter* PlayerIn)
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