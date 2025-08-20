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

	virtual void InitializeComponent() override;
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

	// [신규] 슬롯 내용 전체 교체(배터리 교체 등에 사용)
	UFUNCTION(BlueprintCallable, Category="Inventory")
	bool SetItemAt(int32 SlotIndex, const FInventoryItem& NewItem);

	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetSelectedSlotIndex() const { return SelectedSlotIndex; }

	UFUNCTION(BlueprintPure, Category = "Inventory")
	FInventoryItem GetSelectedItem() const;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnSelectedSlotChanged OnSelectedSlotChanged;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryUpdated OnInventoryUpdated;

	UFUNCTION(BlueprintPure, Category="Inventory")
	int32 GetTotalCountByID(const FString& InItemID) const;

	UFUNCTION(BlueprintPure, Category="Inventory")
	bool HasItemByID(const FString& InItemID) const { return GetTotalCountByID(InItemID) > 0; }

	UFUNCTION(BlueprintPure, Category="Inventory")
	int32 FindFirstSlotIndexByID(const FString& InItemID) const;

	UFUNCTION(BlueprintCallable, Category="Inventory")
	bool TryConsumeByID(const FString& InItemID, int32 Count);

	UFUNCTION(BlueprintPure, Category="Inventory")
	TArray<FInventoryItem> GetAllItems() const;

private:
	UPROPERTY(EditAnywhere, Category = "Inventory")
	int32 HotbarSlots = 10;

	// 기본 슬롯당 최대 스택(아이템이 Override하지 않으면 이 값 사용)
	UPROPERTY(EditAnywhere, Category = "Inventory", meta=(ClampMin="1"))
	int32 MaxStackPerSlot = 99;

	UPROPERTY()
	TArray<FInventoryItem> HotbarItems;

	UPROPERTY()
	int32 SelectedSlotIndex = 0;
};