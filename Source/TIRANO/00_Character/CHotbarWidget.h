// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "00_Character/02_Component/CInventoryItem.h"
#include "CHotbarWidget.generated.h"

/**
 * 
 */

UCLASS()
class TIRANO_API UCHotbarWidget : public UUserWidget
{
	GENERATED_BODY()
    
public:
	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory")
	void UpdateHotbar(const TArray<FInventoryItem>& Items, int32 SelectedIndex);
    
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetupHotbar(class UCInventoryComponent* InInventoryComponent);
    
private:
	UPROPERTY()
	UCInventoryComponent* InventoryComponent;
    
	UFUNCTION()
	void OnInventoryUpdated();
    
	UFUNCTION()
	void OnSelectedSlotChanged(int32 NewSlotIndex);
};