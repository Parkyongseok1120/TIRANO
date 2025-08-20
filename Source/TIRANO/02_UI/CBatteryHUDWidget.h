#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CBatteryHUDWidget.generated.h"

class UProgressBar;
class UTextBlock;
class UBorder;

UCLASS()
class TIRANO_API UCBatteryHUDWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category="Battery")
	void SetBatteryPercent(float Percent);

	UFUNCTION(BlueprintCallable, Category="Battery")
	void ShowReplacePrompt(bool bShow, float SelectedBatteryPercent = -1.f);

protected:
	// 퍼센트 표시용
	UPROPERTY(meta=(BindWidgetOptional))
	UProgressBar* BatteryBar;

	UPROPERTY(meta=(BindWidgetOptional))
	UTextBlock* BatteryText;

	// 교체 프롬프트
	UPROPERTY(meta=(BindWidgetOptional))
	UBorder* ReplacePromptBorder;

	UPROPERTY(meta=(BindWidgetOptional))
	UTextBlock* ReplacePromptText;

	// 표시 문자열 포맷
	UPROPERTY(EditAnywhere, Category="Battery")
	FText ReplacePromptFormat = FText::FromString(TEXT("F키를 누르면 손전등 배터리가 교체됩니다. (배터리: {0}%)"));
};