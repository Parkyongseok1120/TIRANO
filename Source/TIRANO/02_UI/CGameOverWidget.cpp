#include "02_UI/CGameOverWidget.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Kismet/GameplayStatics.h"

void UCGameOverWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (BtnMainMenu)
	{
		BtnMainMenu->OnClicked.AddDynamic(this, &UCGameOverWidget::OnClicked_MainMenu);
	}
}

void UCGameOverWidget::OnClicked_MainMenu()
{
	// 일시정지 해제 후 메인메뉴 이동
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (PC->IsPaused())
		{
			PC->SetPause(false);
		}
	}
	if (MainMenuLevelName != NAME_None)
	{
		UGameplayStatics::OpenLevel(this, MainMenuLevelName);
	}
}