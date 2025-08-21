#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "00_Character/02_Component/03_Inventory/CInventoryItem.h"
#include "CFlashlightItem.generated.h"

class USpotLightComponent;
class UCInventoryComponent;
class ACPlayerCharacter;

/**
 * 손전등 아이템(Actor). 인벤토리에서 선택 시 손에 장착되어 조작 가능.
 * - 배터리 퍼센트 보관 및 소모
 * - 켜짐 상태에서 주기적으로 적을 조사하여 빈사(스턴) 유도
 * - 배터리 0%가 되면 인벤토리에서 손전등 자체 제거
 */
UCLASS()
class TIRANO_API ACFlashlightItem : public AActor
{
	GENERATED_BODY()

public:
	ACFlashlightItem();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	// 장착/해제 시 소켓에 부착을 도와주는 헬퍼(선택)
	UFUNCTION(BlueprintCallable, Category="Flashlight|Attach")
	void AttachToHand(USceneComponent* Mesh, FName SocketName, const FVector& Offset = FVector::ZeroVector, const FRotator& RotOffset = FRotator::ZeroRotator);

	// 인벤토리 메타 초기화(플레이어가 장착할 때 호출)
	void InitializeFromInventoryItem(const FInventoryItem& InItem, UCInventoryComponent* InInventory, ACPlayerCharacter* InOwner);

	// 켜기/끄기/토글
	UFUNCTION(BlueprintCallable, Category="Flashlight")
	void Toggle();

	UFUNCTION(BlueprintCallable, Category="Flashlight")
	void TurnOn();

	UFUNCTION(BlueprintCallable, Category="Flashlight")
	void TurnOff();

	// 배터리(플레이어와 교체 로직에서 사용)
	UFUNCTION(BlueprintPure, Category="Flashlight|Battery")
	float GetBatteryPercent() const { return BatteryPercent; }

	UFUNCTION(BlueprintCallable, Category="Flashlight|Battery")
	void SetBatteryPercent(float InPercent) { BatteryPercent = FMath::Clamp(InPercent, 0.f, 100.f); if (BatteryPercent <= 0.f && bOn) TurnOff(); }

	UFUNCTION(BlueprintPure, Category="Flashlight")
	bool IsOn() const { return bOn; }

protected:
	// 라이트 컴포넌트
	UPROPERTY(VisibleAnywhere, Category="Flashlight")
	USpotLightComponent* Spot;

	// 빛 설정
	UPROPERTY(EditAnywhere, Category="Flashlight")
	float OuterConeAngle = 40.f;

	UPROPERTY(EditAnywhere, Category="Flashlight")
	float InnerConeAngle = 25.f;

	UPROPERTY(EditAnywhere, Category="Flashlight")
	float LightRange = 2000.f;

	// 배터리 소모
	UPROPERTY(EditAnywhere, Category="Flashlight|Battery", meta=(ClampMin="0.0"))
	float BatteryPercent = 100.f;

	UPROPERTY(EditAnywhere, Category="Flashlight|Battery", meta=(ClampMin="0.0"))
	float DrainPercentPerSecond = 6.f;

	// 스턴(빈사) 판정
	UPROPERTY(EditAnywhere, Category="Flashlight|Stun", meta=(ClampMin="0.01"))
	float StunCheckInterval = 0.1f;

	UPROPERTY(EditAnywhere, Category="Flashlight|Stun", meta=(ClampMin="0.0"))
	float StunMaxDistance = 1500.f;

	FTimerHandle StunTimerHandle;

	bool bOn = false;

	// 인벤토리/소유자 참조
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Flashlight|Inventory")
	FString ItemIDForInventory;

	TWeakObjectPtr<UCInventoryComponent> OwnerInventory;
	TWeakObjectPtr<ACPlayerCharacter> OwningPlayer;

	// 중복 제거 방지
	bool bPendingRemovalOnDeplete = false;

	// 적 스턴 처리
	void ApplyStunCone();

	// 배터리 0% 시 인벤토리에서 손전등 제거 + 자신 파괴
	void RemoveFromInventoryOnDeplete();
};