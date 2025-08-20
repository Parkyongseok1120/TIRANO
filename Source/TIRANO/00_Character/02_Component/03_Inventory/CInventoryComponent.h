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

    // BeginPlay보다 더 이른 라이프사이클에서 슬롯을 준비해 Overlap 레이스 방지
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

    UFUNCTION(BlueprintPure, Category = "Inventory")
    int32 GetSelectedSlotIndex() const { return SelectedSlotIndex; }

    UFUNCTION(BlueprintPure, Category = "Inventory")
    FInventoryItem GetSelectedItem() const;

    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FOnSelectedSlotChanged OnSelectedSlotChanged;

    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FOnInventoryUpdated OnInventoryUpdated;

    // ItemID로 합산 개수 조회(슬롯 합계)
    UFUNCTION(BlueprintPure, Category="Inventory")
    int32 GetTotalCountByID(const FString& InItemID) const;

    // ItemID 보유 여부
    UFUNCTION(BlueprintPure, Category="Inventory")
    bool HasItemByID(const FString& InItemID) const { return GetTotalCountByID(InItemID) > 0; }

    // 첫 슬롯 인덱스 조회 (없으면 -1)
    UFUNCTION(BlueprintPure, Category="Inventory")
    int32 FindFirstSlotIndexByID(const FString& InItemID) const;

    // ID로 지정 개수만큼 소비(여러 슬롯에 걸쳐 차감). 성공/실패 반환
    UFUNCTION(BlueprintCallable, Category="Inventory")
    bool TryConsumeByID(const FString& InItemID, int32 Count);

    // 전체 슬롯 스냅샷 가져오기(복사)
    UFUNCTION(BlueprintPure, Category="Inventory")
    TArray<FInventoryItem> GetAllItems() const;

private:
    UPROPERTY(EditAnywhere, Category = "Inventory")
    int32 HotbarSlots = 10;

    // 슬롯당 최대 스택
    UPROPERTY(EditAnywhere, Category = "Inventory", meta=(ClampMin="1"))
    int32 MaxStackPerSlot = 99;

    UPROPERTY()
    TArray<FInventoryItem> HotbarItems;

    UPROPERTY()
    int32 SelectedSlotIndex = 0;

};