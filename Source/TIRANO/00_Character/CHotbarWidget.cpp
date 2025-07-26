// Fill out your copyright notice in the Description page of Project Settings.
#include "CHotbarWidget.h"
#include "00_Character/02_Component/CInventoryComponent.h"

void UCHotbarWidget::SetupHotbar(UCInventoryComponent* InInventoryComponent)
{
	InventoryComponent = InInventoryComponent;

	if (InventoryComponent)
	{
		InventoryComponent->OnInventoryUpdated.AddDynamic(this, &UCHotbarWidget::OnInventoryUpdated);
		InventoryComponent->OnSelectedSlotChanged.AddDynamic(this, &UCHotbarWidget::OnSelectedSlotChanged);
        
		OnInventoryUpdated();
	}
}

void UCHotbarWidget::OnInventoryUpdated()
{
	if (InventoryComponent)
	{
		TArray<FInventoryItem> Items;
		for (int32 i = 0; i < 10; i++)
		{
			Items.Add(InventoryComponent->GetItemAt(i));
		}

		UpdateHotbar(Items, InventoryComponent->GetSelectedSlotIndex());
	}
}

void UCHotbarWidget::OnSelectedSlotChanged(int32 NewSlotIndex)
{
	OnInventoryUpdated();
}