// CInventoryItem.h (추가 필드 포함)
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "CInventoryItem.generated.h"

class ACThrowableItemBase;

USTRUCT(BlueprintType)
struct FInventoryItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* ItemIcon;

	// 일반적으로 월드에 배치/소환할 때 쓸 수 있는 액터 클래스
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> ItemClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Quantity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsEquippable;

	// [추가] 던질 수 있는 아이템인 경우 클래스 지정 (AThrowableItemBase 파생)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<ACThrowableItemBase> ThrowableClass;

	// [선택] 인스펙터에서 조정할 프리뷰/홀드용 오프셋
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector HoldOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator HoldRotationOffset = FRotator::ZeroRotator;

	// 손전등/손전등 배터리는 1로 설정하세요.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0"))
	int32 MaxStackPerSlotOverride = 0;

	// [신규] 배터리형 아이템(손전등 배터리)일 때 퍼센트(0~100). 다른 아이템은 무시.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0", ClampMax="100.0"))
	float BatteryPercent = 100.f;


	FInventoryItem()
	{
		ItemID = TEXT("");
		ItemName = TEXT("아이템 없음");
		ItemIcon = nullptr;
		ItemClass = nullptr;
		Quantity = 0;
		bIsEquippable = false;
		ThrowableClass = nullptr;
	}
};