// CItemImageManager.cpp
#include "CItemImageManager.h"
#include "Engine/AssetManager.h"
#include "Global.h"

UCItemImageManager::UCItemImageManager()
{
}

void UCItemImageManager::Initialize()
{
	LoadItemImageTable();
	PreloadCommonItems();
}

UTexture2D* UCItemImageManager::GetItemIcon(const FString& ItemID)
{
	// 캐시에서 먼저 확인
	UTexture2D** CachedIcon = ItemIconCache.Find(ItemID);
	if (CachedIcon && *CachedIcon)
	{
		return *CachedIcon;
	}
    
	// 캐시에 없으면 데이터 테이블에서 로드
	if (ItemImageTable)
	{
		FItemImageData* ItemData = ItemImageTable->FindRow<FItemImageData>(*ItemID, TEXT(""));
		if (ItemData && !ItemData->ItemIcon.IsNull())
		{
			UTexture2D* LoadedIcon = ItemData->ItemIcon.LoadSynchronous();
			ItemIconCache.Add(ItemID, LoadedIcon);
			return LoadedIcon;
		}
	}
    
	// 기본 아이콘 반환
	UTexture2D* DefaultIcon = LoadObject<UTexture2D>(nullptr, TEXT("/Game/UI/Icons/DefaultItemIcon"));
	return DefaultIcon ? DefaultIcon : nullptr;
}

void UCItemImageManager::LoadItemImageTable()
{

	// 런타임에 데이터 테이블 로드
	ItemImageTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/Data/DT_ItemImages"));
	if (ItemImageTable)
	{
		CLog::Log("아이템 이미지 테이블 로드 성공");
	}
	else
	{
		CLog::Log("아이템 이미지 테이블 로드 실패");
        
		// 데이터 테이블이 없는 경우 대체 방법 제공
		// 여기서는 게임 실행 중 에러를 방지하기 위한 로직을 추가할 수 있습니다
	}
}

void UCItemImageManager::PreloadCommonItems()
{
	// 자주 사용하는 아이템은 미리 로드
	if (ItemImageTable)
	{
		TArray<FString> CommonItems = { "Weapon_Sword", "Item_HealthPotion", "Item_ManaPotion" };
        
		for (const FString& ItemID : CommonItems)
		{
			GetItemIcon(ItemID);
		}
	}
}