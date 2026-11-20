// Copyright 2024 Craquez. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FootballCoreTypes.h"
#include "FootballPlayerWidget.generated.h"

class AFootballPlayerCharacter;
class UProgressBar;
class UTextBlock;
class UImage;
class UVerticalBox;
class UHorizontalBox;
class UWidgetAnimation;

/**
 * Football Player Widget - Shows controlled player info: stamina, fatigue, action feedback
 */
UCLASS(Blueprintable, BlueprintType)
class FOOTBALLCOREUI_API UFootballPlayerWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    // Setup
    UFUNCTION(BlueprintCallable, Category = "Football|UI")
    void SetPlayer(AFootballPlayerCharacter* InPlayer);

    // Stats
    UFUNCTION(BlueprintCallable, Category = "Football|UI")
    void UpdateStamina(float StaminaPercent);

    UFUNCTION(BlueprintCallable, Category = "Football|UI")
    void UpdateFatigue(bool bFatigued);

    UFUNCTION(BlueprintCallable, Category = "Football|UI")
    void ShowAction(FGameplayTag ActionTag, bool bSuccess);

    UFUNCTION(BlueprintCallable, Category = "Football|UI")
    void UpdateAttributes();

protected:
    // Player Reference
    UPROPERTY()
    TObjectPtr<AFootballPlayerCharacter> Player = nullptr;

    // UI Elements - Stamina
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UProgressBar> StaminaBar = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> StaminaText = nullptr;

    // UI Elements - Fatigue
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> FatigueIndicator = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UWidgetAnimation> FatiguePulseAnimation = nullptr;

    // UI Elements - Action Feedback
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UVerticalBox> ActionFeedbackContainer = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UWidgetAnimation> ActionSuccessAnimation = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UWidgetAnimation> ActionFailAnimation = nullptr;

    // UI Elements - Attributes
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UHorizontalBox> AttributeContainer = nullptr;

    // Action Feedback Class
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|UI")
    TSubclassOf<UUserWidget> ActionFeedbackWidgetClass;

    // Colors
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|UI|Colors")
    FLinearColor StaminaHighColor = FLinearColor::Green;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|UI|Colors")
    FLinearColor StaminaMediumColor = FLinearColor::Yellow;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|UI|Colors")
    FLinearColor StaminaLowColor = FLinearColor::Red;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|UI|Colors")
    FLinearColor FatigueColor = FLinearColor::Orange;

private:
    void UpdateStaminaDisplay(float StaminaPercent);
    void UpdateFatigueDisplay(bool bFatigued);
    void CreateActionFeedback(FGameplayTag ActionTag, bool bSuccess);
    FString GetActionDisplayName(FGameplayTag ActionTag) const;
};