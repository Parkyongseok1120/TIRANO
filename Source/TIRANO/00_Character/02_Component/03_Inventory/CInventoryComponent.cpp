// Copyright

#include "CInventoryComponent.h"
#include "Global.h"

UCInventoryComponent::UCInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UCInventoryComponent::InitializeComponent()
{
    Super::InitializeComponent();

    // BeginPlay 이전에도 Overlap에 대응할 수 있도록 미리 슬롯 확보
    if (HotbarItems.Num() != HotbarSlots)
    {
        HotbarItems.SetNum(HotbarSlots);
    }
}

void UCInventoryComponent::BeginPlay()
{
    Super::BeginPlay();

    // 방어적으로 한 번 더 보정
    if (HotbarItems.Num() != HotbarSlots)
    {
        HotbarItems.SetNum(HotbarSlots);
    }
    
    CLog::Log(FString::Printf(TEXT("인벤토리 컴포넌트 초기화됨. 슬롯 개수: %d"), HotbarSlots));
}

void UCInventoryComponent::SelectSlot(int32 SlotIndex)
{
    if (SlotIndex >= 0 && SlotIndex < HotbarSlots)
    {
        SelectedSlotIndex = SlotIndex;
        OnSelectedSlotChanged.Broadcast(SelectedSlotIndex);
        CLog::Log(FString::Printf(TEXT("슬롯 선택: %d"), SelectedSlotIndex));
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
    // 혹시라도 배열이 비어 있으면 즉시 초기화(레이스 방지)
    if (HotbarItems.Num() == 0)
    {
        HotbarItems.SetNum(HotbarSlots);
    }

    int32 RemainingQuantity = Item.Quantity;

    // 1단계: 같은 아이템 ID를 가진 슬롯에 추가 (최대 스택 제한 적용)
    for (int32 i = 0; i < HotbarItems.Num(); i++)
    {
        if (HotbarItems[i].ItemID == Item.ItemID && !Item.ItemID.IsEmpty() && HotbarItems[i].Quantity > 0)
        {
            const int32 MaxStack = MaxStackPerSlot;
            const int32 SpaceLeft = MaxStack - HotbarItems[i].Quantity;

            if (SpaceLeft > 0)
            {
                const int32 QuantityToAdd = FMath::Min(SpaceLeft, RemainingQuantity);
                HotbarItems[i].Quantity += QuantityToAdd;
                RemainingQuantity -= QuantityToAdd;

                if (RemainingQuantity <= 0)
                {
                    OnInventoryUpdated.Broadcast();
                    CLog::Log(FString::Printf(TEXT("%s 스택됨. 총 수량: %d"), *Item.ItemName, HotbarItems[i].Quantity));
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
            const int32 MaxStack = MaxStackPerSlot;
            const int32 QuantityToAdd = FMath::Min(MaxStack, RemainingQuantity);

            FInventoryItem NewItem = Item;
            NewItem.Quantity = QuantityToAdd;

            HotbarItems[i] = NewItem;
            RemainingQuantity -= QuantityToAdd;

            OnInventoryUpdated.Broadcast();
            CLog::Log(FString::Printf(TEXT("%s 새 슬롯에 추가됨. 수량: %d"), *Item.ItemName, NewItem.Quantity));

            if (RemainingQuantity <= 0)
            {
                return true;
            }
        }
    }

    // 3단계: 남은 수량 처리
    if (RemainingQuantity > 0)
    {
        CLog::Log(FString::Printf(TEXT("%s 추가 실패. 남은 수량: %d"), *Item.ItemName, RemainingQuantity));
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

int32 UCInventoryComponent::GetTotalCountByID(const FString& InItemID) const
{
    if (InItemID.IsEmpty())
        return 0;

    int32 Total = 0;
    for (const FInventoryItem& It : HotbarItems)
    {
        if (It.Quantity > 0 && It.ItemID == InItemID)
        {
            Total += It.Quantity;
        }
    }
    return Total;
}

int32 UCInventoryComponent::FindFirstSlotIndexByID(const FString& InItemID) const
{
    if (InItemID.IsEmpty())
        return -1;

    for (int32 i = 0; i < HotbarItems.Num(); ++i)
    {
        if (HotbarItems[i].Quantity > 0 && HotbarItems[i].ItemID == InItemID)
        {
            return i;
        }
    }
    return -1;
}

bool UCInventoryComponent::TryConsumeByID(const FString& InItemID, int32 Count)
{
    if (InItemID.IsEmpty() || Count <= 0)
        return true;

    int32 Remaining = Count;

    for (int32 i = 0; i < HotbarItems.Num(); ++i)
    {
        FInventoryItem& Slot = HotbarItems[i];
        if (Slot.Quantity <= 0 || Slot.ItemID != InItemID)
            continue;

        const int32 ToConsume = FMath::Min(Slot.Quantity, Remaining);
        Slot.Quantity -= ToConsume;
        Remaining -= ToConsume;

        if (Slot.Quantity <= 0)
        {
            Slot = FInventoryItem();
        }

        if (Remaining <= 0)
        {
            OnInventoryUpdated.Broadcast();
            return true;
        }
    }

    // 충분히 소비하지 못함
    OnInventoryUpdated.Broadcast();
    return false;
}

TArray<FInventoryItem> UCInventoryComponent::GetAllItems() const
{
    return HotbarItems;
}