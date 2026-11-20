// Copyright 2024 Craquez. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FootballCoreTypes.h"
#include "FootballTacticalWidget.generated.h"

class AFootballTeam;
class AFootballPlayerCharacter;
class UImage;
class UTextBlock;
class UVerticalBox;
class UHorizontalBox;
class UCanvasPanel;
class UWidgetAnimation;

/**
 * Football Tactical Widget - Shows formation, team instructions, player positions
 */
UCLASS(Blueprintable, BlueprintType)
class FOOTBALLCOREUI_API UFootballTacticalWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    // Setup
    UFUNCTION(BlueprintCallable, Category = "Football|UI")
    void SetTeam(AFootballTeam* InTeam);

    UFUNCTION(BlueprintCallable, Category = "Football|UI")
    void RemoveTeam(AFootballTeam* InTeam);

    // Formation
    UFUNCTION(BlueprintCallable, Category = "Football|UI")
    void UpdateFormation(int32 FormationID);

    // Instructions
    UFUNCTION(BlueprintCallable, Category = "Football|UI")
    void UpdateInstructions(const FFootballTacticalInstructions& Instructions);

    // Player Highlight
    UFUNCTION(BlueprintCallable, Category = "Football|UI")
    void HighlightPlayer(AFootballPlayerCharacter* Player);

    UFUNCTION(BlueprintCallable, Category = "Football|UI")
    void ClearHighlight();

protected:
    // Teams
    UPROPERTY()
    TArray<TObjectPtr<AFootballTeam>> Teams;

    // UI Elements - Pitch
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCanvasPanel> PitchCanvas = nullptr;

    // UI Elements - Formation
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> FormationText = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> FormationImage = nullptr;

    // UI Elements - Instructions
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UVerticalBox> InstructionsContainer = nullptr;

    // UI Elements - Player Slots
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UHorizontalBox> HomePlayerSlots = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UHorizontalBox> AwayPlayerSlots = nullptr;

    // Player Slot Widget Class
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|UI")
    TSubclassOf<UUserWidget> PlayerSlotWidgetClass;

    // Instruction Widget Class
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|UI")
    TSubclassOf<UUserWidget> InstructionWidgetClass;

    // Colors
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|UI|Colors")
    FLinearColor HomeTeamColor = FLinearColor::Blue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|UI|Colors")
    FLinearColor AwayTeamColor = FLinearColor::Red;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|UI|Colors")
    FLinearColor HighlightColor = FLinearColor::Yellow;

private:
    void DrawPitch();
    void DrawFormation(AFootballTeam* Team);
    void UpdatePlayerSlots(AFootballTeam* Team);
    void UpdateInstructionSliders(const FFootballTacticalInstructions& Instructions);
    void CreateInstructionSlider(FString Label, float Value, FLinearColor Color);
    FVector2D WorldToCanvas(FVector WorldLocation) const;
};