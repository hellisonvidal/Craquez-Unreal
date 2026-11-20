// Copyright 2024 Craquez. All Rights Reserved.

#include "Widgets/FootballMenuWidget.h"
#include "HUD/FootballHUD.h"
#include "Match/FootballGameMode.h"
#include "Team/FootballTeam.h"
#include "Player/FootballPlayerCharacter.h"
#include "Components/Button.h"
#include "Components/VerticalBox.h"
#include "Components/ComboBoxString.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Animation/WidgetAnimation.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void UFootballMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();

    BindButtonEvents();
    InitializeSettings();
    InitializeTeamManagement();

    // Show main menu by default
    ShowMenu(FName("MainMenu"));
}

void UFootballMenuWidget::SetHUD(AFootballHUD* InHUD)
{
    HUD = InHUD;
}

void UFootballMenuWidget::ShowMenu(FName MenuName)
{
    HideAllPanels();

    CurrentMenu = MenuName;

    UVerticalBox* TargetPanel = nullptr;

    if (MenuName == FName("MainMenu"))
    {
        TargetPanel = MainMenuPanel;
    }
    else if (MenuName == FName("PauseMenu"))
    {
        TargetPanel = PauseMenuPanel;
    }
    else if (MenuName == FName("Settings"))
    {
        TargetPanel = SettingsPanel;
    }
    else if (MenuName == FName("TeamManagement"))
    {
        TargetPanel = TeamManagementPanel;
        PopulatePlayerList();
    }
    else if (MenuName == FName("Tactics"))
    {
        TargetPanel = TacticsPanel;
    }
    else if (MenuName == FName("Replay"))
    {
        TargetPanel = ReplayPanel;
    }

    ShowPanel(TargetPanel);

    if (MenuOpenAnimation)
    {
        PlayAnimation(MenuOpenAnimation);
    }
}

void UFootballMenuWidget::HideMenu()
{
    if (MenuCloseAnimation)
    {
        PlayAnimation(MenuCloseAnimation);
    }

    CurrentMenu = NAME_None;
}

void UFootballMenuWidget::SetGameMode(AFootballGameMode* InGameMode)
{
    GameMode = InGameMode;
}

void UFootballMenuWidget::OnMainMenuClicked()
{
    ShowMenu(FName("MainMenu"));
}

void UFootballMenuWidget::OnPauseMenuClicked()
{
    ShowMenu(FName("PauseMenu"));
}

void UFootballMenuWidget::OnSettingsMenuClicked()
{
    ShowMenu(FName("Settings"));
}

void UFootballMenuWidget::OnTeamManagementClicked()
{
    ShowMenu(FName("TeamManagement"));
}

void UFootballMenuWidget::OnTacticsMenuClicked()
{
    ShowMenu(FName("Tactics"));
}

void UFootballMenuWidget::OnReplayMenuClicked()
{
    ShowMenu(FName("Replay"));
}

void UFootballMenuWidget::OnQuitClicked()
{
    if (HUD)
    {
        HUD->HideMenuWidget();
    }

    // Quit game
    UKismetSystemLibrary::QuitGame(GetWorld(), GetWorld()->GetFirstPlayerController(), EQuitPreference::Quit, false);
}

void UFootballMenuWidget::OnStartMatchClicked()
{
    if (GameMode)
    {
        GameMode->StartMatch();
    }

    if (HUD)
    {
        HUD->HideMenuWidget();
    }
}

void UFootballMenuWidget::OnResumeMatchClicked()
{
    if (GameMode)
    {
        GameMode->ResumeMatch();
    }

    if (HUD)
    {
        HUD->HideMenuWidget();
    }
}

void UFootballMenuWidget::OnRestartMatchClicked()
{
    if (GameMode)
    {
        GameMode->RestartMatch();
    }

    if (HUD)
    {
        HUD->HideMenuWidget();
    }
}

void UFootballMenuWidget::OnGraphicsQualityChanged(int32 QualityLevel)
{
    // Apply graphics quality
    // Would use scalability settings
}

void UFootballMenuWidget::OnAudioVolumeChanged(float Volume)
{
    // Apply audio volume
    // Would use audio settings
}

void UFootballMenuWidget::OnDifficultyChanged(int32 Difficulty)
{
    // Apply difficulty
    if (GameMode)
    {
        // GameMode->SetDifficulty((EFootballDifficulty)Difficulty);
    }
}

void UFootballMenuWidget::OnCameraStyleChanged(int32 Style)
{
    // Apply camera style
    if (GameMode)
    {
        // GameMode->SetCameraStyle((EFootballCameraStyle)Style);
    }
}

void UFootballMenuWidget::OnFormationChanged(int32 FormationID)
{
    if (GameMode)
    {
        AFootballTeam* HomeTeam = GameMode->GetHomeTeam();
        if (HomeTeam)
        {
            HomeTeam->SetFormation(FormationID);
        }
    }
}

void UFootballMenuWidget::OnPlayerRoleChanged(FString PlayerName, int32 RoleIndex)
{
    // Change player role in team management
}

void UFootballMenuWidget::OnSubstituteClicked(FString PlayerOut, FString PlayerIn)
{
    // Process substitution
}

void UFootballMenuWidget::BindButtonEvents()
{
    if (StartMatchButton)
    {
        StartMatchButton->OnClicked.AddDynamic(this, &UFootballMenuWidget::OnStartMatchClicked);
    }
    if (SettingsButton)
    {
        SettingsButton->OnClicked.AddDynamic(this, &UFootballMenuWidget::OnSettingsMenuClicked);
    }
    if (TeamManagementButton)
    {
        TeamManagementButton->OnClicked.AddDynamic(this, &UFootballMenuWidget::OnTeamManagementClicked);
    }
    if (QuitButton)
    {
        QuitButton->OnClicked.AddDynamic(this, &UFootballMenuWidget::OnQuitClicked);
    }
    if (ResumeButton)
    {
        ResumeButton->OnClicked.AddDynamic(this, &UFootballMenuWidget::OnResumeMatchClicked);
    }
    if (RestartButton)
    {
        RestartButton->OnClicked.AddDynamic(this, &UFootballMenuWidget::OnRestartMatchClicked);
    }
    if (PauseSettingsButton)
    {
        PauseSettingsButton->OnClicked.AddDynamic(this, &UFootballMenuWidget::OnSettingsMenuClicked);
    }
    if (PauseQuitButton)
    {
        PauseQuitButton->OnClicked.AddDynamic(this, &UFootballMenuWidget::OnQuitClicked);
    }
    if (GraphicsQualityCombo)
    {
        GraphicsQualityCombo->OnSelectionChanged.AddDynamic(this, &UFootballMenuWidget::OnGraphicsQualityChanged);
    }
    if (MasterVolumeSlider)
    {
        MasterVolumeSlider->OnValueChanged.AddDynamic(this, &UFootballMenuWidget::OnAudioVolumeChanged);
    }
    if (DifficultyCombo)
    {
        DifficultyCombo->OnSelectionChanged.AddDynamic(this, &UFootballMenuWidget::OnDifficultyChanged);
    }
    if (CameraStyleCombo)
    {
        CameraStyleCombo->OnSelectionChanged.AddDynamic(this, &UFootballMenuWidget::OnCameraStyleChanged);
    }
    if (FormationCombo)
    {
        FormationCombo->OnSelectionChanged.AddDynamic(this, &UFootballMenuWidget::OnFormationChanged);
    }
}

void UFootballMenuWidget::ShowPanel(UVerticalBox* Panel)
{
    if (Panel)
    {
        Panel->SetVisibility(ESlateVisibility::Visible);

        if (PanelTransitionAnimation)
        {
            PlayAnimation(PanelTransitionAnimation);
        }
    }
}

void UFootballMenuWidget::HideAllPanels()
{
    if (MainMenuPanel) MainMenuPanel->SetVisibility(ESlateVisibility::Hidden);
    if (PauseMenuPanel) PauseMenuPanel->SetVisibility(ESlateVisibility::Hidden);
    if (SettingsPanel) SettingsPanel->SetVisibility(ESlateVisibility::Hidden);
    if (TeamManagementPanel) TeamManagementPanel->SetVisibility(ESlateVisibility::Hidden);
    if (TacticsPanel) TacticsPanel->SetVisibility(ESlateVisibility::Hidden);
    if (ReplayPanel) ReplayPanel->SetVisibility(ESlateVisibility::Hidden);
}

void UFootballMenuWidget::InitializeSettings()
{
    if (GraphicsQualityCombo)
    {
        GraphicsQualityCombo->ClearOptions();
        GraphicsQualityCombo->AddOption(TEXT("Low"));
        GraphicsQualityCombo->AddOption(TEXT("Medium"));
        GraphicsQualityCombo->AddOption(TEXT("High"));
        GraphicsQualityCombo->AddOption(TEXT("Epic"));
        GraphicsQualityCombo->SetSelectedIndex(2); // High default
    }

    if (DifficultyCombo)
    {
        DifficultyCombo->ClearOptions();
        DifficultyCombo->AddOption(TEXT("Beginner"));
        DifficultyCombo->AddOption(TEXT("Amateur"));
        DifficultyCombo->AddOption(TEXT("Professional"));
        DifficultyCombo->AddOption(TEXT("World Class"));
        DifficultyCombo->AddOption(TEXT("Legendary"));
        DifficultyCombo->SetSelectedIndex(2); // Professional default
    }

    if (CameraStyleCombo)
    {
        CameraStyleCombo->ClearOptions();
        CameraStyleCombo->AddOption(TEXT("Broadcast"));
        CameraStyleCombo->AddOption(TEXT("Player Follow"));
        CameraStyleCombo->AddOption(TEXT("Tactical"));
        CameraStyleCombo->AddOption(TEXT("Free"));
        CameraStyleCombo->SetSelectedIndex(0); // Broadcast default
    }

    // Set default volumes
    if (MasterVolumeSlider) MasterVolumeSlider->SetValue(1.0f);
    if (MusicVolumeSlider) MusicVolumeSlider->SetValue(0.8f);
    if (SFXVolumeSlider) SFXVolumeSlider->SetValue(1.0f);
}

void UFootballMenuWidget::InitializeTeamManagement()
{
    PopulateFormationCombo();
}

void UFootballMenuWidget::ApplySettings()
{
    // Apply all settings
}

void UFootballMenuWidget::ApplyTeamChanges()
{
    // Apply team management changes
}

void UFootballMenuWidget::PopulateFormationCombo()
{
    if (FormationCombo)
    {
        FormationCombo->ClearOptions();
        FormationCombo->AddOption(TEXT("4-3-3"));
        FormationCombo->AddOption(TEXT("4-4-2"));
        FormationCombo->AddOption(TEXT("3-5-2"));
        FormationCombo->AddOption(TEXT("4-2-3-1"));
        FormationCombo->AddOption(TEXT("4-5-1"));
        FormationCombo->AddOption(TEXT("3-4-3"));
        FormationCombo->AddOption(TEXT("5-3-2"));
        FormationCombo->SetSelectedIndex(0); // 4-3-3 default
    }
}

void UFootballMenuWidget::PopulatePlayerList()
{
    if (!PlayerListContainer || !GameMode)
    {
        return;
    }

    PlayerListContainer->ClearChildren();

    AFootballTeam* HomeTeam = GameMode->GetHomeTeam();
    if (HomeTeam)
    {
        for (AFootballPlayerCharacter* Player : HomeTeam->GetAllPlayers())
        {
            if (Player)
            {
                // Create player entry widget
                // Would show name, role, attributes, substitution button
            }
        }
    }
}