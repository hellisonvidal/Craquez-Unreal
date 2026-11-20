// Copyright 2024 Craquez. All Rights Reserved.

#include "Widgets/FootballTacticalWidget.h"
#include "Team/FootballTeam.h"
#include "Player/FootballPlayerCharacter.h"
#include "Components/CanvasPanel.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"
#include "Components/Slider.h"
#include "Components/ProgressBar.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetTextLibrary.h"

void UFootballTacticalWidget::NativeConstruct()
{
    Super::NativeConstruct();

    DrawPitch();
}

void UFootballTacticalWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    // Update player positions on pitch
    for (AFootballTeam* Team : Teams)
    {
        if (Team)
        {
            DrawFormation(Team);
        }
    }
}

void UFootballTacticalWidget::SetTeam(AFootballTeam* InTeam)
{
    if (InTeam && !Teams.Contains(InTeam))
    {
        Teams.Add(InTeam);
        UpdatePlayerSlots(InTeam);
    }
}

void UFootballTacticalWidget::RemoveTeam(AFootballTeam* InTeam)
{
    Teams.Remove(InTeam);
}

void UFootballTacticalWidget::UpdateFormation(int32 FormationID)
{
    if (FormationText)
    {
        FormationText->SetText(FText::FromString(FString::Printf(TEXT("%d"), FormationID)));
    }

    // Update formation image based on ID
    // Would load formation diagram texture
}

void UFootballTacticalWidget::UpdateInstructions(const FFootballTacticalInstructions& Instructions)
{
    if (InstructionsContainer)
    {
        InstructionsContainer->ClearChildren();
        UpdateInstructionSliders(Instructions);
    }
}

void UFootballTacticalWidget::HighlightPlayer(AFootballPlayerCharacter* Player)
{
    // Would highlight player on pitch and in player slots
}

void UFootballTacticalWidget::ClearHighlight()
{
    // Clear all highlights
}

void UFootballTacticalWidget::DrawPitch()
{
    if (!PitchCanvas)
    {
        return;
    }

    // Pitch is drawn via canvas or image
    // For now, assume background image is set in designer
}

void UFootballTacticalWidget::DrawFormation(AFootballTeam* Team)
{
    if (!Team || !PitchCanvas)
    {
        return;
    }

    // Clear existing player markers
    // Would remove previous marker widgets

    // Draw players at current positions
    for (AFootballPlayerCharacter* Player : Team->GetAllPlayers())
    {
        if (Player)
        {
            FVector2D CanvasPos = WorldToCanvas(Player->GetActorLocation());

            // Create or update player marker
            // This would use a small widget for each player
        }
    }
}

void UFootballTacticalWidget::UpdatePlayerSlots(AFootballTeam* Team)
{
    if (!Team)
    {
        return;
    }

    UHorizontalBox* TargetSlots = (Team->GetTeamSide() == EFootballTeamSide::Home) ? HomePlayerSlots : AwayPlayerSlots;
    FLinearColor TeamColor = (Team->GetTeamSide() == EFootballTeamSide::Home) ? HomeTeamColor : AwayTeamColor;

    if (!TargetSlots || !PlayerSlotWidgetClass)
    {
        return;
    }

    TargetSlots->ClearChildren();

    for (AFootballPlayerCharacter* Player : Team->GetAllPlayers())
    {
        if (Player)
        {
            UUserWidget* SlotWidget = CreateWidget<UUserWidget>(GetWorld(), PlayerSlotWidgetClass);
            if (SlotWidget)
            {
                // Set player info on slot widget
                // Would bind player name, number, role, stamina

                TargetSlots->AddChildToHorizontalBox(SlotWidget);
            }
        }
    }
}

void UFootballTacticalWidget::UpdateInstructionSliders(const FFootballTacticalInstructions& Instructions)
{
    if (!InstructionsContainer || !InstructionWidgetClass)
    {
        return;
    }

    // Create sliders for each tactical instruction
    CreateInstructionSlider(TEXT("Defensive Line"), Instructions.DefensiveLine, HomeTeamColor);
    CreateInstructionSlider(TEXT("Engagement Line"), Instructions.EngagementLine, HomeTeamColor);
    CreateInstructionSlider(TEXT("Width"), Instructions.Width, HomeTeamColor);
    CreateInstructionSlider(TEXT("Build Up Play"), Instructions.BuildUpPlay, HomeTeamColor);
    CreateInstructionSlider(TEXT("Chance Creation"), Instructions.ChanceCreation, HomeTeamColor);
    CreateInstructionSlider(TEXT("Passing Directness"), Instructions.PassingDirectness, HomeTeamColor);
    CreateInstructionSlider(TEXT("Pressing Intensity"), Instructions.PressingIntensity, HomeTeamColor);
    CreateInstructionSlider(TEXT("Defensive Shape"), Instructions.DefensiveShape, HomeTeamColor);
    CreateInstructionSlider(TEXT("Interceptions"), Instructions.Interceptions, HomeTeamColor);
    CreateInstructionSlider(TEXT("Offside Trap"), Instructions.OffsideTrap, HomeTeamColor);
}

void UFootballTacticalWidget::CreateInstructionSlider(FString Label, float Value, FLinearColor Color)
{
    if (!InstructionWidgetClass || !InstructionsContainer)
    {
        return;
    }

    UUserWidget* InstructionWidget = CreateWidget<UUserWidget>(GetWorld(), InstructionWidgetClass);
    if (InstructionWidget)
    {
        // Set label, value, color on widget
        // Would use BindWidget for label text, slider, color

        InstructionsContainer->AddChildToVerticalBox(InstructionWidget);
    }
}

FVector2D UFootballTacticalWidget::WorldToCanvas(FVector WorldLocation) const
{
    if (!PitchCanvas)
    {
        return FVector2D::ZeroVector;
    }

    // Convert world coordinates to canvas coordinates
    // Pitch is roughly 105m x 68m = 10500cm x 6800cm
    // Canvas size would be set in designer

    FVector2D CanvasSize = PitchCanvas->GetDesiredSize();
    if (CanvasSize.IsZero())
    {
        CanvasSize = FVector2D(800, 500); // Default
    }

    // Normalize world position (assuming center at 0,0)
    float NormalizedX = (WorldLocation.X + 5250.0f) / 10500.0f; // -5250 to 5250 -> 0 to 1
    float NormalizedY = (WorldLocation.Y + 3400.0f) / 6800.0f;  // -3400 to 3400 -> 0 to 1

    // Clamp
    NormalizedX = FMath::Clamp(NormalizedX, 0.0f, 1.0f);
    NormalizedY = FMath::Clamp(NormalizedY, 0.0f, 1.0f);

    // Convert to canvas coordinates (Y flipped for screen coords)
    return FVector2D(NormalizedX * CanvasSize.X, (1.0f - NormalizedY) * CanvasSize.Y);
}