// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "00_Character/02_Component/03_Inventory/CInventoryItem.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "CHotbarWidget.generated.h"

UCLASS()
class TIRANO_API UCHotbarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 이벤트는 C++ 구현으로 대체할 것이므로 필요 없어짐
	// UFUNCTION(BlueprintImplementableEvent, Category = "Inventory") 
	// void UpdateHotbar(const TArray<FInventoryItem>& Items, int32 SelectedIndex);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetupHotbar(class UCInventoryComponent* InInventoryComponent);

	virtual void NativeConstruct() override;

	// C++에서 직접 업데이트 구현
	void UpdateHotbar(const TArray<FInventoryItem>& Items, int32 SelectedIndex);

private:
	UPROPERTY()
	UCInventoryComponent* InventoryComponent;

	UFUNCTION()
	void OnInventoryUpdated();

	UFUNCTION()
	void OnSelectedSlotChanged(int32 NewSlotIndex);

	// 위젯 요소 배열
	UPROPERTY()
	TArray<UBorder*> SlotBorders;

	UPROPERTY()
	TArray<UImage*> ItemIcons;

	UPROPERTY()
	TArray<UTextBlock*> ItemCounts;

	// 선택된 슬롯과 기본 슬롯 색상
	UPROPERTY(EditAnywhere, Category = "Appearance")
	FLinearColor SelectedSlotColor = FLinearColor(1.0f, 0.8f, 0.0f, 0.7f);

	UPROPERTY(EditAnywhere, Category = "Appearance")
	FLinearColor DefaultSlotColor = FLinearColor(0.3f, 0.3f, 0.3f, 0.5f);

	const int32 MAX_SLOTS = 10;
};