#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CGameOverWidget.generated.h"

class UButton;
class UImage;

/**
 * 간단한 게임 오버 UI
 * - 게임오버 이미지(선택)
 * - 메인메뉴로 돌아가는 버튼
 */
UCLASS()
class TIRANO_API UCGameOverWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 메인메뉴 레벨 이름(에디터에서 지정)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GameOver")
	FName MainMenuLevelName = "MainMenu";

protected:
	virtual void NativeConstruct() override;

	// UMG에서 같은 이름으로 위젯 배치(옵션)
	UPROPERTY(meta=(BindWidgetOptional))
	UImage* GameOverImage = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	UButton* BtnMainMenu = nullptr;

	UFUNCTION()
	void OnClicked_MainMenu();
};