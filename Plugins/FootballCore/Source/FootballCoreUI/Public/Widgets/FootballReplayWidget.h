// Copyright 2024 Craquez. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FootballCoreTypes.h"
#include "FootballReplayWidget.generated.h"

class AFootballGameMode;
class AFootballDeterministicManager;
class UButton;
class USlider;
class UTextBlock;
class UImage;
class UVerticalBox;
class UHorizontalBox;
class UWidgetAnimation;

/**
 * Football Replay Widget - Replay system with timeline, camera controls, analysis tools
 */
UCLASS(Blueprintable, BlueprintType)
class FOOTBALLCOREUI_API UFootballReplayWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    // Setup
    UFUNCTION(BlueprintCallable, Category = "Football|UI")
    void SetGameMode(AFootballGameMode* InGameMode);

    // Replay Control
    UFUNCTION(BlueprintCallable, Category = "Football|UI")
    void StartReplay(int32 StartFrame);

    UFUNCTION(BlueprintCallable, Category = "Football|UI")
    void StopReplay();

    UFUNCTION(BlueprintCallable, Category = "Football|UI")
    void PauseReplay();

    UFUNCTION(BlueprintCallable, Category = "Football|UI")
    void ResumeReplay();

    UFUNCTION(BlueprintCallable, Category = "Football|UI")
    void SetReplaySpeed(float Speed);

    UFUNCTION(BlueprintCallable, Category = "Football|UI")
    void SeekToFrame(int32 Frame);

    UFUNCTION(BlueprintCallable, Category = "Football|UI")
    void SeekToTime(float TimeSeconds);

    // Camera
    UFUNCTION(BlueprintCallable, Category = "Football|UI")
    void SetCameraMode(int32 Mode); // 0=Broadcast, 1=Player, 2=Tactical, 3=Free

    UFUNCTION(BlueprintCallable, Category = "Football|UI")
    void SetFollowPlayer(AFootballPlayerCharacter* Player);

    // Analysis
    UFUNCTION(BlueprintCallable, Category = "Football|UI")
    void ToggleHeatmap(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Football|UI")
    void TogglePassLines(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Football|UI")
    void TogglePlayerPaths(bool bEnabled);

    // Events
    UFUNCTION(BlueprintCallable, Category = "Football|UI")
    void AddEventMarker(int32 Frame, FString Description, FLinearColor Color);

protected:
    // References
    UPROPERTY()
    TObjectPtr<AFootballGameMode> GameMode = nullptr;

    UPROPERTY()
    TObjectPtr<AFootballDeterministicManager> DeterministicManager = nullptr;

    // UI Elements - Transport Controls
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> PlayButton = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> PauseButton = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> StopButton = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> StepBackButton = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> StepForwardButton = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<USlider> TimelineSlider = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> CurrentTimeText = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> TotalTimeText = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> CurrentFrameText = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> SpeedText = nullptr;

    // UI Elements - Camera
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UHorizontalBox> CameraModeButtons = nullptr;

    // UI Elements - Analysis
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> HeatmapToggle = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> PassLinesToggle = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> PlayerPathsToggle = nullptr;

    // UI Elements - Events
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UVerticalBox> EventMarkersContainer = nullptr;

    // State
    UPROPERTY()
    bool bIsPlaying = false;

    UPROPERTY()
    bool bIsPaused = false;

    UPROPERTY()
    float ReplaySpeed = 1.0f;

    UPROPERTY()
    int32 CurrentFrame = 0;

    UPROPERTY()
    int32 TotalFrames = 0;

    UPROPERTY()
    int32 CameraMode = 0;

    UPROPERTY()
    TObjectPtr<AFootballPlayerCharacter> FollowPlayer = nullptr;

private:
    void BindButtonEvents();
    void UpdateTransportControls();
    void UpdateTimeline();
    void UpdateTimeDisplay();
    void OnPlayClicked();
    void OnPauseClicked();
    void OnStopClicked();
    void OnStepBackClicked();
    void OnStepForwardClicked();
    void OnTimelineChanged(float Value);
    void OnCameraModeChanged(int32 Mode);
    void OnHeatmapToggled();
    void OnPassLinesToggled();
    void OnPlayerPathsToggled();
    FString FormatTime(float Seconds) const;
};