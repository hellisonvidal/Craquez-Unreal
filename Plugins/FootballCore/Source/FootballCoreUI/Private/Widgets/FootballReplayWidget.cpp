// Copyright 2024 Craquez. All Rights Reserved.

#include "Widgets/FootballReplayWidget.h"
#include "Match/FootballGameMode.h"
#include "Deterministic/FootballDeterministicReplication.h"
#include "Player/FootballPlayerCharacter.h"
#include "Components/Button.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "Components/VerticalBox.h"
#include "Kismet/KismetTextLibrary.h"

void UFootballReplayWidget::NativeConstruct()
{
    Super::NativeConstruct();

    BindButtonEvents();
    UpdateTransportControls();
}

void UFootballReplayWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (bIsPlaying && !bIsPaused && DeterministicManager)
    {
        // Advance replay
        CurrentFrame += FMath::RoundToInt(60.0f * InDeltaTime * ReplaySpeed);
        CurrentFrame = FMath::Clamp(CurrentFrame, 0, TotalFrames);

        if (CurrentFrame >= TotalFrames)
        {
            StopReplay();
        }
        else
        {
            DeterministicManager->PlayReplay(CurrentFrame);
            UpdateTimeline();
        }
    }
}

void UFootballReplayWidget::SetGameMode(AFootballGameMode* InGameMode)
{
    GameMode = InGameMode;

    if (GameMode)
    {
        DeterministicManager = Cast<AFootballDeterministicManager>(
            UGameplayStatics::GetActorOfClass(GetWorld(), AFootballDeterministicManager::StaticClass()));
    }
}

void UFootballReplayWidget::StartReplay(int32 StartFrame)
{
    CurrentFrame = StartFrame;
    TotalFrames = DeterministicManager ? DeterministicManager->GetCurrentFrame() : 3600;
    bIsPlaying = true;
    bIsPaused = false;
    ReplaySpeed = 1.0f;

    if (DeterministicManager)
    {
        DeterministicManager->PlayReplay(StartFrame);
    }

    UpdateTransportControls();
    UpdateTimeline();
    UpdateTimeDisplay();
}

void UFootballReplayWidget::StopReplay()
{
    bIsPlaying = false;
    bIsPaused = false;
    CurrentFrame = 0;

    if (DeterministicManager)
    {
        DeterministicManager->StopRecording(); // Would stop replay
    }

    UpdateTransportControls();
    UpdateTimeline();
    UpdateTimeDisplay();
}

void UFootballReplayWidget::PauseReplay()
{
    bIsPaused = true;
    UpdateTransportControls();
}

void UFootballReplayWidget::ResumeReplay()
{
    bIsPaused = false;
    UpdateTransportControls();
}

void UFootballReplayWidget::SetReplaySpeed(float Speed)
{
    ReplaySpeed = FMath::Clamp(Speed, 0.1f, 4.0f);

    if (SpeedText)
    {
        SpeedText->SetText(FText::FromString(FString::Printf(TEXT("%.1fx"), ReplaySpeed)));
    }
}

void UFootballReplayWidget::SeekToFrame(int32 Frame)
{
    CurrentFrame = FMath::Clamp(Frame, 0, TotalFrames);

    if (DeterministicManager)
    {
        DeterministicManager->PlayReplay(CurrentFrame);
    }

    UpdateTimeline();
    UpdateTimeDisplay();
}

void UFootballReplayWidget::SeekToTime(float TimeSeconds)
{
    int32 Frame = FMath::RoundToInt(TimeSeconds * 60.0f);
    SeekToFrame(Frame);
}

void UFootballReplayWidget::SetCameraMode(int32 Mode)
{
    CameraMode = FMath::Clamp(Mode, 0, 3);

    // Update camera mode buttons visual state
    // Would set active button style
}

void UFootballReplayWidget::SetFollowPlayer(AFootballPlayerCharacter* Player)
{
    FollowPlayer = Player;

    // Set camera to follow player
}

void UFootballReplayWidget::ToggleHeatmap(bool bEnabled)
{
    // Toggle heatmap visualization
}

void UFootballReplayWidget::TogglePassLines(bool bEnabled)
{
    // Toggle pass lines visualization
}

void UFootballReplayWidget::TogglePlayerPaths(bool bEnabled)
{
    // Toggle player paths visualization
}

void UFootballReplayWidget::AddEventMarker(int32 Frame, FString Description, FLinearColor Color)
{
    // Create event marker widget on timeline
}

void UFootballReplayWidget::BindButtonEvents()
{
    if (PlayButton)
    {
        PlayButton->OnClicked.AddDynamic(this, &UFootballReplayWidget::OnPlayClicked);
    }
    if (PauseButton)
    {
        PauseButton->OnClicked.AddDynamic(this, &UFootballReplayWidget::OnPauseClicked);
    }
    if (StopButton)
    {
        StopButton->OnClicked.AddDynamic(this, &UFootballReplayWidget::OnStopClicked);
    }
    if (StepBackButton)
    {
        StepBackButton->OnClicked.AddDynamic(this, &UFootballReplayWidget::OnStepBackClicked);
    }
    if (StepForwardButton)
    {
        StepForwardButton->OnClicked.AddDynamic(this, &UFootballReplayWidget::OnStepForwardClicked);
    }
    if (TimelineSlider)
    {
        TimelineSlider->OnValueChanged.AddDynamic(this, &UFootballReplayWidget::OnTimelineChanged);
    }
    if (HeatmapToggle)
    {
        HeatmapToggle->OnClicked.AddDynamic(this, &UFootballReplayWidget::OnHeatmapToggled);
    }
    if (PassLinesToggle)
    {
        PassLinesToggle->OnClicked.AddDynamic(this, &UFootballReplayWidget::OnPassLinesToggled);
    }
    if (PlayerPathsToggle)
    {
        PlayerPathsToggle->OnClicked.AddDynamic(this, &UFootballReplayWidget::OnPlayerPathsToggled);
    }
}

void UFootballReplayWidget::UpdateTransportControls()
{
    if (PlayButton)
    {
        PlayButton->SetIsEnabled(!bIsPlaying || bIsPaused);
    }
    if (PauseButton)
    {
        PauseButton->SetIsEnabled(bIsPlaying && !bIsPaused);
    }
    if (StopButton)
    {
        StopButton->SetIsEnabled(bIsPlaying);
    }
}

void UFootballReplayWidget::UpdateTimeline()
{
    if (TimelineSlider && TotalFrames > 0)
    {
        TimelineSlider->SetValue((float)CurrentFrame / (float)TotalFrames);
    }
}

void UFootballReplayWidget::UpdateTimeDisplay()
{
    if (CurrentTimeText)
    {
        CurrentTimeText->SetText(FText::FromString(FormatTime(CurrentFrame / 60.0f)));
    }
    if (TotalTimeText)
    {
        TotalTimeText->SetText(FText::FromString(FormatTime(TotalFrames / 60.0f)));
    }
    if (CurrentFrameText)
    {
        CurrentFrameText->SetText(FText::FromString(FString::Printf(TEXT("Frame: %d"), CurrentFrame)));
    }
}

void UFootballReplayWidget::OnPlayClicked()
{
    if (bIsPaused)
    {
        ResumeReplay();
    }
    else if (!bIsPlaying)
    {
        StartReplay(CurrentFrame);
    }
}

void UFootballReplayWidget::OnPauseClicked()
{
    PauseReplay();
}

void UFootballReplayWidget::OnStopClicked()
{
    StopReplay();
}

void UFootballReplayWidget::OnStepBackClicked()
{
    SeekToFrame(CurrentFrame - 60); // Step back 1 second
}

void UFootballReplayWidget::OnStepForwardClicked()
{
    SeekToFrame(CurrentFrame + 60); // Step forward 1 second
}

void UFootballReplayWidget::OnTimelineChanged(float Value)
{
    if (!bIsPlaying)
    {
        int32 Frame = FMath::RoundToInt(Value * TotalFrames);
        SeekToFrame(Frame);
    }
}

void UFootballReplayWidget::OnCameraModeChanged(int32 Mode)
{
    SetCameraMode(Mode);
}

void UFootballReplayWidget::OnHeatmapToggled()
{
    // Toggle heatmap
}

void UFootballReplayWidget::OnPassLinesToggled()
{
    // Toggle pass lines
}

void UFootballReplayWidget::OnPlayerPathsToggled()
{
    // Toggle player paths
}

FString UFootballReplayWidget::FormatTime(float Seconds) const
{
    int32 Minutes = FMath::FloorToInt(Seconds / 60.0f);
    int32 Secs = FMath::FloorToInt(Seconds) % 60;
    return FString::Printf(TEXT("%02d:%02d"), Minutes, Secs);
}