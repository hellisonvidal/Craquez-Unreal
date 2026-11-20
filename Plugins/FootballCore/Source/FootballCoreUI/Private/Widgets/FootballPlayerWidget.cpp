// Copyright 2024 Craquez. All Rights Reserved.

#include "Widgets/FootballPlayerWidget.h"
#include "Player/FootballPlayerCharacter.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"
#include "Animation/WidgetAnimation.h"
#include "Kismet/KismetTextLibrary.h"

void UFootballPlayerWidget::NativeConstruct()
{
    Super::NativeConstruct();

    UpdateStaminaDisplay(100.0f);
    UpdateFatigueDisplay(false);
}

void UFootballPlayerWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (Player)
    {
        UpdateStaminaDisplay(Player->GetStaminaPercent());
        UpdateFatigueDisplay(Player->IsFatigued());
    }
}

void UFootballPlayerWidget::SetPlayer(AFootballPlayerCharacter* InPlayer)
{
    Player = InPlayer;
    UpdateAttributes();
}

void UFootballPlayerWidget::UpdateStamina(float StaminaPercent)
{
    UpdateStaminaDisplay(StaminaPercent);
}

void UFootballPlayerWidget::UpdateFatigue(bool bFatigued)
{
    UpdateFatigueDisplay(bFatigued);
}

void UFootballPlayerWidget::ShowAction(FGameplayTag ActionTag, bool bSuccess)
{
    CreateActionFeedback(ActionTag, bSuccess);

    if (bSuccess && ActionSuccessAnimation)
    {
        PlayAnimation(ActionSuccessAnimation);
    }
    else if (!bSuccess && ActionFailAnimation)
    {
        PlayAnimation(ActionFailAnimation);
    }
}

void UFootballPlayerWidget::UpdateAttributes()
{
    if (!Player || !AttributeContainer)
    {
        return;
    }

    // Clear existing attribute widgets
    AttributeContainer->ClearChildren();

    // Would create attribute widgets here
    // For now, just update stamina
    if (Player)
    {
        UpdateStaminaDisplay(Player->GetStaminaPercent());
    }
}

void UFootballPlayerWidget::UpdateStaminaDisplay(float StaminaPercent)
{
    if (StaminaBar)
    {
        StaminaBar->SetPercent(StaminaPercent / 100.0f);

        // Color based on stamina level
        FLinearColor BarColor;
        if (StaminaPercent > 60.0f)
        {
            BarColor = StaminaHighColor;
        }
        else if (StaminaPercent > 30.0f)
        {
            BarColor = StaminaMediumColor;
        }
        else
        {
            BarColor = StaminaLowColor;
        }

        StaminaBar->SetFillColorAndOpacity(BarColor);
    }

    if (StaminaText)
    {
        StaminaText->SetText(FText::FromString(FString::Printf(TEXT("%.0f%%"), StaminaPercent)));
    }
}

void UFootballPlayerWidget::UpdateFatigueDisplay(bool bFatigued)
{
    if (FatigueIndicator)
    {
        FatigueIndicator->SetVisibility(bFatigued ? ESlateVisibility::Visible : ESlateVisibility::Hidden);

        if (bFatigued && FatiguePulseAnimation)
        {
            PlayAnimation(FatiguePulseAnimation, 0.0f, 0, EUMGSequencePlayMode::Loop);
        }
        else
        {
            StopAnimation(FatiguePulseAnimation);
        }

        FatigueIndicator->SetColorAndOpacity(FatigueColor);
    }
}

void UFootballPlayerWidget::CreateActionFeedback(FGameplayTag ActionTag, bool bSuccess)
{
    if (!ActionFeedbackWidgetClass || !ActionFeedbackContainer)
    {
        return;
    }

    UUserWidget* FeedbackWidget = CreateWidget<UUserWidget>(GetWorld(), ActionFeedbackWidgetClass);
    if (FeedbackWidget)
    {
        // Set action name and success state
        // Would use BindWidget or custom event

        ActionFeedbackContainer->AddChildToVerticalBox(FeedbackWidget);

        // Animate and remove after delay
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, [FeedbackWidget]()
        {
            if (FeedbackWidget)
            {
                FeedbackWidget->RemoveFromParent();
            }
        }, 2.0f, false);
    }
}

FString UFootballPlayerWidget::GetActionDisplayName(FGameplayTag ActionTag) const
{
    FString TagName = ActionTag.GetTagName().ToString();
    // Remove "Action." prefix for display
    if (TagName.StartsWith("Action."))
    {
        TagName = TagName.RightChop(7);
    }
    return TagName;
}