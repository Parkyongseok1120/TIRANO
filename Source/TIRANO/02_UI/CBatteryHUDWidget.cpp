#include "02_UI/CBatteryHUDWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"

void UCBatteryHUDWidget::SetBatteryPercent(float Percent)
{
	const float Clamped = FMath::Clamp(Percent, 0.f, 100.f);
	if (BatteryBar)
	{
		BatteryBar->SetPercent(Clamped / 100.f);
	}
	if (BatteryText)
	{
		BatteryText->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), FMath::RoundToInt(Clamped))));
	}
}

void UCBatteryHUDWidget::ShowReplacePrompt(bool bShow, float SelectedBatteryPercent)
{
	if (ReplacePromptBorder)
	{
		ReplacePromptBorder->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
	if (ReplacePromptText)
	{
		if (!bShow)
		{
			ReplacePromptText->SetText(FText::GetEmpty());
			return;
		}

		if (SelectedBatteryPercent >= 0.f)
		{
			const int32 Pct = FMath::Clamp(FMath::RoundToInt(SelectedBatteryPercent), 0, 100);
			FFormatNamedArguments Args;
			Args.Add(TEXT("0"), FText::AsNumber(Pct));
			ReplacePromptText->SetText(FText::Format(ReplacePromptFormat, FText::AsNumber(Pct)));
		}
		else
		{
			// 퍼센트 미지정 시 기본 문구만
			ReplacePromptText->SetText(FText::FromString(TEXT("F키를 누르면 손전등 배터리가 교체됩니다.")));
		}
	}
}