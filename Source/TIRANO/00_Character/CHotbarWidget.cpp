// Fill out your copyright notice in the Description page of Project Settings.
#include "CHotbarWidget.h"
#include "CGameInstance.h"
#include "00_Character/02_Component/CInventoryComponent.h"

void UCHotbarWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 배열 초기화
    SlotBorders.SetNum(MAX_SLOTS);
    ItemIcons.SetNum(MAX_SLOTS);
    ItemCounts.SetNum(MAX_SLOTS);

    // 위젯 요소들을 이름으로 찾아서 저장
    for (int32 i = 0; i < MAX_SLOTS; i++)
    {
        FString BorderName = FString::Printf(TEXT("BorderSlot_%d"), i);
        FString IconName = FString::Printf(TEXT("IconImage_%d"), i);
        FString CountName = FString::Printf(TEXT("CountText_%d"), i);

        SlotBorders[i] = Cast<UBorder>(GetWidgetFromName(*BorderName));
        ItemIcons[i] = Cast<UImage>(GetWidgetFromName(*IconName));
        ItemCounts[i] = Cast<UTextBlock>(GetWidgetFromName(*CountName));
    }
}

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

void UCHotbarWidget::UpdateHotbar(const TArray<FInventoryItem>& Items, int32 SelectedIndex)
{
    UCGameInstance* GameInstance = Cast<UCGameInstance>(GetGameInstance());
    UCItemImageManager* ItemImageManager = GameInstance ? GameInstance->GetItemImageManager() : nullptr;
    
    // 모든 슬롯 업데이트
    for (int32 i = 0; i < FMath::Min(Items.Num(), MAX_SLOTS); i++)
    {
        const FInventoryItem& Item = Items[i];

        if (SlotBorders[i])
        {
            // 선택된 슬롯은 다른 색상으로 표시
            SlotBorders[i]->SetBrushColor(i == SelectedIndex ? SelectedSlotColor : DefaultSlotColor);
        }

        if (ItemIcons[i])
        {
            if (Item.Quantity > 0)
            {
                // 아이콘 설정 - ItemID로 이미지 매니저에서 먼저 찾고, 없으면 직접 참조 사용
                UTexture2D* IconTexture = Item.ItemIcon;
                
                // ItemID가 있으면 이미지 매니저에서 아이콘 로드
                if (!Item.ItemID.IsEmpty() && ItemImageManager)
                {
                    UTexture2D* ManagedIcon = ItemImageManager->GetItemIcon(Item.ItemID);
                    if (ManagedIcon)
                    {
                        IconTexture = ManagedIcon;
                    }
                }
                
                if (IconTexture)
                {
                    FSlateBrush Brush;
                    Brush.SetResourceObject(IconTexture);
                    ItemIcons[i]->SetBrush(Brush);
                    ItemIcons[i]->SetVisibility(ESlateVisibility::Visible);
                }
                else
                {
                    ItemIcons[i]->SetVisibility(ESlateVisibility::Hidden);
                }
            }
            else
            {
                // 빈 슬롯은 아이콘 숨김
                ItemIcons[i]->SetVisibility(ESlateVisibility::Hidden);
            }
        }

        if (ItemCounts[i])
        {
            if (Item.Quantity > 1)
            {
                // 수량이 1보다 많을 경우 표시
                ItemCounts[i]->SetText(FText::AsNumber(Item.Quantity));
                ItemCounts[i]->SetVisibility(ESlateVisibility::Visible);
            }
            else
            {
                // 수량이 1 이하면 숨김
                ItemCounts[i]->SetVisibility(ESlateVisibility::Hidden);
            }
        }
    }
}

void UCHotbarWidget::OnInventoryUpdated()
{
    if (InventoryComponent)
    {
        TArray<FInventoryItem> Items;
        for (int32 i = 0; i < MAX_SLOTS; i++)
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