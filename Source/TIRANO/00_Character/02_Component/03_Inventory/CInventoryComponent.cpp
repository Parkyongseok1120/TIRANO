// Fill out your copyright notice in the Description page of Project Settings.


#include "CInventoryComponent.h"
#include "Global.h"

UCInventoryComponent::UCInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UCInventoryComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // 핫바 슬롯 초기화
    HotbarItems.SetNum(HotbarSlots);
    
    CLog::Log("인벤토리 컴포넌트 초기화됨. 슬롯 개수: " + FString::FromInt(HotbarSlots));
}

void UCInventoryComponent::SelectSlot(int32 SlotIndex)
{
    if (SlotIndex >= 0 && SlotIndex < HotbarSlots)
    {
        SelectedSlotIndex = SlotIndex;
        OnSelectedSlotChanged.Broadcast(SelectedSlotIndex);
        CLog::Log("슬롯 선택: " + FString::FromInt(SelectedSlotIndex));
    }
}

void UCInventoryComponent::NextSlot()
{
    int32 NewIndex = (SelectedSlotIndex + 1) % HotbarSlots;
    SelectSlot(NewIndex);
}

void UCInventoryComponent::PrevSlot()
{
    int32 NewIndex = (SelectedSlotIndex - 1 + HotbarSlots) % HotbarSlots;
    SelectSlot(NewIndex);
}

bool UCInventoryComponent::AddItem(const FInventoryItem& Item)
{
    int32 RemainingQuantity = Item.Quantity;

    // 1단계: 같은 아이템 ID를 가진 슬롯에 추가 (최대 스택 제한 적용)
    for (int32 i = 0; i < HotbarItems.Num(); i++)
    {
        if (HotbarItems[i].ItemID == Item.ItemID && !Item.ItemID.IsEmpty() && HotbarItems[i].Quantity > 0)
        {
            int32 MaxStack = 99; // 최대 스택 수 (필요에 따라 설정)
            int32 SpaceLeft = MaxStack - HotbarItems[i].Quantity;

            if (SpaceLeft > 0)
            {
                int32 QuantityToAdd = FMath::Min(SpaceLeft, RemainingQuantity);
                HotbarItems[i].Quantity += QuantityToAdd;
                RemainingQuantity -= QuantityToAdd;

                if (RemainingQuantity <= 0)
                {
                    OnInventoryUpdated.Broadcast();
                    CLog::Log(Item.ItemName + TEXT(" 스택됨. 총 수량: ") + FString::FromInt(HotbarItems[i].Quantity));
                    return true;
                }
            }
        }
    }

    // 2단계: 빈 슬롯에 추가
    for (int32 i = 0; i < HotbarItems.Num(); i++)
    {
        if (HotbarItems[i].Quantity == 0)
        {
            int32 MaxStack = 99; // 최대 스택 수
            int32 QuantityToAdd = FMath::Min(MaxStack, RemainingQuantity);

            FInventoryItem NewItem = Item;
            NewItem.Quantity = QuantityToAdd;

            HotbarItems[i] = NewItem;
            RemainingQuantity -= QuantityToAdd;

            OnInventoryUpdated.Broadcast();
            CLog::Log(Item.ItemName + TEXT(" 새 슬롯에 추가됨. 수량: ") + FString::FromInt(NewItem.Quantity));

            if (RemainingQuantity <= 0)
            {
                return true;
            }
        }
    }

    // 3단계: 남은 수량 처리
    if (RemainingQuantity > 0)
    {
        CLog::Log(Item.ItemName + TEXT(" 추가 실패. 남은 수량: ") + FString::FromInt(RemainingQuantity));
    }

    return RemainingQuantity <= 0;
}

void UCInventoryComponent::RemoveItem(int32 SlotIndex, int32 Count)
{
    if (SlotIndex >= 0 && SlotIndex < HotbarItems.Num())
    {
        HotbarItems[SlotIndex].Quantity -= Count;
        
        if (HotbarItems[SlotIndex].Quantity <= 0)
        {
            // 아이템이 모두 소진되면 초기화
            HotbarItems[SlotIndex] = FInventoryItem();
        }
        
        OnInventoryUpdated.Broadcast();
    }
}

FInventoryItem UCInventoryComponent::GetItemAt(int32 SlotIndex) const
{
    if (SlotIndex >= 0 && SlotIndex < HotbarItems.Num())
    {
        return HotbarItems[SlotIndex];
    }
    
    return FInventoryItem();
}

FInventoryItem UCInventoryComponent::GetSelectedItem() const
{
    return GetItemAt(SelectedSlotIndex);
}