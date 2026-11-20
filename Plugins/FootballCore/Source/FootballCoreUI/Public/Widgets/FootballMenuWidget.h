// Copyright 2024 Craquez. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FootballCoreTypes.h"
#include "FootballMenuWidget.generated.h"

class AFootballHUD;
class AFootballGameMode;
class UButton;
class UVerticalBox;
class UHorizontalBox;
class UComboBoxString;
class USlider;
class UTextBlock;
class UWidgetAnimation;

/**
 * Football Menu Widget - Main menu, pause menu, settings, team management
 */
UCLASS(Blueprintable, BlueprintType)
class FOOTBALLCOREUI_API UFootballMenuWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    // Menu Management
    UFUNCTION(BlueprintCallable, Category = "Football|UI")
    void SetHUD(AFootballHUD* InHUD);

    UFUNCTION(BlueprintCallable, Category = "Football|UI")
    void ShowMenu(FName MenuName);

    UFUNCTION(BlueprintCallable, Category = "Football|UI")
    void HideMenu();

    UFUNCTION(BlueprintCallable, Category = "Football|UI")
    void SetGameMode(AFootballGameMode* InGameMode);

    // Menu Navigation
    UFUNCTION(BlueprintCallable, Category = "Football|UI")
    void OnMainMenuClicked();

    UFUNCTION(BlueprintCallable, Category = "Football|UI")
    void OnPauseMenuClicked();

    UFUNCTION(BlueprintCallable, Category = "Football|UI")
    void OnSettingsMenuClicked();

    UFUNCTION(BlueprintCallable, Category = "Football|UI")
    void OnTeamManagementClicked();

    UFUNCTION(BlueprintCallable, Category = "Football|UI")
    void OnTacticsMenuClicked();

    UFUNCTION(BlueprintCallable, Category = "Football|UI")
    void OnReplayMenuClicked();

    UFUNCTION(BlueprintCallable, Category = "Football|UI")
    void OnQuitClicked();

    // Game Actions
    UFUNCTION(BlueprintCallable, Category = "Football|UI")
    void OnStartMatchClicked();

    UFUNCTION(BlueprintCallable, Category = "Football|UI")
    void OnResumeMatchClicked();

    UFUNCTION(BlueprintCallable, Category = "Football|UI")
    void OnRestartMatchClicked();

    // Settings
    UFUNCTION(BlueprintCallable, Category = "Football|UI")
    void OnGraphicsQualityChanged(int32 QualityLevel);

    UFUNCTION(BlueprintCallable, Category = "Football|UI")
    void OnAudioVolumeChanged(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Football|UI")
    void OnDifficultyChanged(int32 Difficulty);

    UFUNCTION(BlueprintCallable, Category = "Football|UI")
    void OnCameraStyleChanged(int32 Style);

    // Team Management
    UFUNCTION(BlueprintCallable, Category = "Football|UI")
    void OnFormationChanged(int32 FormationID);

    UFUNCTION(BlueprintCallable, Category = "Football|UI")
    void OnPlayerRoleChanged(FString PlayerName, int32 RoleIndex);

    UFUNCTION(BlueprintCallable, Category = "Football|UI")
    void OnSubstituteClicked(FString PlayerOut, FString PlayerIn);

protected:
    // References
    UPROPERTY()
    TObjectPtr<AFootballHUD> HUD = nullptr;

    UPROPERTY()
    TObjectPtr<AFootballGameMode> GameMode = nullptr;

    // Menu Panels
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UVerticalBox> MainMenuPanel = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UVerticalBox> PauseMenuPanel = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UVerticalBox> SettingsPanel = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UVerticalBox> TeamManagementPanel = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UVerticalBox> TacticsPanel = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UVerticalBox> ReplayPanel = nullptr;

    // Main Menu Buttons
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> StartMatchButton = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> SettingsButton = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> TeamManagementButton = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> QuitButton = nullptr;

    // Pause Menu Buttons
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> ResumeButton = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> RestartButton = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> PauseSettingsButton = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> PauseQuitButton = nullptr;

    // Settings Controls
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UComboBoxString> GraphicsQualityCombo = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<USlider> MasterVolumeSlider = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<USlider> MusicVolumeSlider = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<USlider> SFXVolumeSlider = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UComboBoxString> DifficultyCombo = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UComboBoxString> CameraStyleCombo = nullptr;

    // Team Management Controls
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UComboBoxString> FormationCombo = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UVerticalBox> PlayerListContainer = nullptr;

    // Animations
    UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
    TObjectPtr<UWidgetAnimation> MenuOpenAnimation = nullptr;

    UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
    TObjectPtr<UWidgetAnimation> MenuCloseAnimation = nullptr;

    UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
    TObjectPtr<UWidgetAnimation> PanelTransitionAnimation = nullptr;

    // Current State
    UPROPERTY()
    FName CurrentMenu = NAME_None;

private:
    UFUNCTION()
    void HandleGraphicsQualitySelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

    UFUNCTION()
    void HandleDifficultySelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

    UFUNCTION()
    void HandleCameraStyleSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

    UFUNCTION()
    void HandleFormationSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

    void BindButtonEvents();
    void ShowPanel(UVerticalBox* Panel);
    void HideAllPanels();
    void InitializeSettings();
    void InitializeTeamManagement();
    void ApplySettings();
    void ApplyTeamChanges();
    void PopulateFormationCombo();
    void PopulatePlayerList();
};