// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "00_Character/02_Component/03_Inventory/CInventoryItem.h"
#include "CInventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSelectedSlotChanged, int32, NewSlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TIRANO_API UCInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCInventoryComponent();

    virtual void BeginPlay() override;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void SelectSlot(int32 SlotIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void NextSlot();

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void PrevSlot();

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool AddItem(const FInventoryItem& Item);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void RemoveItem(int32 SlotIndex, int32 Count = 1);

    UFUNCTION(BlueprintPure, Category = "Inventory")
    FInventoryItem GetItemAt(int32 SlotIndex) const;

    UFUNCTION(BlueprintPure, Category = "Inventory")
    int32 GetSelectedSlotIndex() const { return SelectedSlotIndex; }

    UFUNCTION(BlueprintPure, Category = "Inventory")
    FInventoryItem GetSelectedItem() const;

    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FOnSelectedSlotChanged OnSelectedSlotChanged;

    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FOnInventoryUpdated OnInventoryUpdated;

private:
    UPROPERTY(EditAnywhere, Category = "Inventory")
    int32 HotbarSlots = 10;

    UPROPERTY()
    TArray<FInventoryItem> HotbarItems;

    UPROPERTY()
    int32 SelectedSlotIndex = 0;
};