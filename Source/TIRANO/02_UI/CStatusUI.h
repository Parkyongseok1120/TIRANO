#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CStatusUI.generated.h"

class UProgressBar;
class ACPlayerCharacter;

UCLASS(BlueprintType, Blueprintable)
class TIRANO_API UCStatusUI : public UUserWidget
{
	GENERATED_BODY()

public:
	// 캐릭터 연결(델리게이트 구독)
	UFUNCTION(BlueprintCallable, Category="StatusUI")
	void SetupWithCharacter(ACPlayerCharacter* InCharacter);

	// 수동 갱신용 API(원하면 이벤트 대신 호출 가능)
	UFUNCTION(BlueprintCallable, Category="StatusUI")
	void SetHealth(float Current, float Max);

	UFUNCTION(BlueprintCallable, Category="StatusUI")
	void SetHealthRatio(float Ratio);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// 반드시 있어야 하는 ProgressBar. 없으면 위젯 BP 컴파일 실패.
	UPROPERTY(meta=(BindWidget))
	UProgressBar* PB_Health = nullptr;

private:
	// 연결된 캐릭터(약참조)
	TWeakObjectPtr<ACPlayerCharacter> OwnerCharacter;

	// 캐릭터 델리게이트 핸들러
	UFUNCTION()
	void OnCharacterStaminaChanged(float Current, float Max);

	void ApplyRatio(float Ratio);
};