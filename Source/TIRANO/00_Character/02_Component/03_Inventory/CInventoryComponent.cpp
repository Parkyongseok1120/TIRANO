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

	if (HotbarItems.Num() != HotbarSlots)
	{
		HotbarItems.SetNum(HotbarSlots);
	}
}

void UCInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

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

static int32 ResolveMaxStackForItem(const FInventoryItem& Item, int32 DefaultMax)
{
	return Item.MaxStackPerSlotOverride > 0 ? Item.MaxStackPerSlotOverride : DefaultMax;
}

bool UCInventoryComponent::AddItem(const FInventoryItem& Item)
{
	if (HotbarItems.Num() == 0)
	{
		HotbarItems.SetNum(HotbarSlots);
	}

	int32 RemainingQuantity = Item.Quantity;

	// 1단계: 같은 아이템 ID를 가진 슬롯에 추가(해당 슬롯/아이템의 MaxStack 기준)
	for (int32 i = 0; i < HotbarItems.Num(); i++)
	{
		FInventoryItem& SlotItem = HotbarItems[i];
		if (SlotItem.Quantity > 0 && SlotItem.ItemID == Item.ItemID && !Item.ItemID.IsEmpty())
		{
			const int32 MaxStack = ResolveMaxStackForItem(SlotItem, MaxStackPerSlot);
			const int32 SpaceLeft = MaxStack - SlotItem.Quantity;

			if (SpaceLeft > 0)
			{
				const int32 QuantityToAdd = FMath::Min(SpaceLeft, RemainingQuantity);
				SlotItem.Quantity += QuantityToAdd;
				RemainingQuantity -= QuantityToAdd;

				if (RemainingQuantity <= 0)
				{
					OnInventoryUpdated.Broadcast();
					CLog::Log(FString::Printf(TEXT("%s 스택됨. 총 수량: %d"), *Item.ItemName, SlotItem.Quantity));
					return true;
				}
			}
		}
	}

	// 2단계: 빈 슬롯에 추가(신규 아이템의 MaxStack 기준으로 분할)
	for (int32 i = 0; i < HotbarItems.Num(); i++)
	{
		if (HotbarItems[i].Quantity == 0)
		{
			const int32 MaxStack = ResolveMaxStackForItem(Item, MaxStackPerSlot);
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
		FInventoryItem& Slot = HotbarItems[SlotIndex];
		const int32 BeforeQty = Slot.Quantity;

		Slot.Quantity -= Count;
		if (Slot.Quantity <= 0)
		{
			Slot = FInventoryItem(); // 완전 비움
		}

		OnInventoryUpdated.Broadcast();

		CLog::Log(FString::Printf(TEXT("[Inventory] RemoveItem slot:%d count:%d -> before:%d, now:%d"),
			SlotIndex, Count, BeforeQty, HotbarItems[SlotIndex].Quantity));
	}
	else
	{
		CLog::Log(FString::Printf(TEXT("[Inventory] RemoveItem ignored. Invalid slot:%d"), SlotIndex));
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

bool UCInventoryComponent::SetItemAt(int32 SlotIndex, const FInventoryItem& NewItem)
{
	if (SlotIndex < 0 || SlotIndex >= HotbarItems.Num())
		return false;

	HotbarItems[SlotIndex] = NewItem;
	OnInventoryUpdated.Broadcast();
	return true;
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

	OnInventoryUpdated.Broadcast();
	return false;
}

TArray<FInventoryItem> UCInventoryComponent::GetAllItems() const
{
	return HotbarItems;
}