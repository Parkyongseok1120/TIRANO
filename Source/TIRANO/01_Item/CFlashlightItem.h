#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CFlashlightItem.generated.h"

class USpotLightComponent;

/**
 * 손전등 아이템(Actor). 인벤토리에서 선택 시 손에 장착되어 조작 가능.
 * - 배터리 퍼센트 보관 및 소모
 * - 켜짐 상태에서 주기적으로 적을 조사하여 빈사(스턴) 유도
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

	// 적 스턴 처리
	void ApplyStunCone();
};