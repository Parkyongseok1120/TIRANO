#include "00_Character/02_Component/CInventoryComponent.h"
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
    // 빈 슬롯 찾기
    for (int32 i = 0; i < HotbarItems.Num(); i++)
    {
        if (HotbarItems[i].Quantity == 0)
        {
            HotbarItems[i] = Item;
            OnInventoryUpdated.Broadcast();
            return true;
        }
    }
    
    // 빈 슬롯이 없음
    return false;
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