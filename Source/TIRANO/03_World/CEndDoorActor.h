#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CEndDoorActor.generated.h"

class USphereComponent;
class UTextRenderComponent;
class UUserWidget;
class ACPlayerCharacter;

/**
 * 엔딩(클리어) 도어 액터
 * - 플레이어가 InteractSphere에 오버랩했고
 * - 인벤토리에 KeyItemID 개수 >= RequiredKeyCount 이면
 *   클리어 처리(게임 일시정지 + 클리어 UI)
 */
UCLASS()
class TIRANO_API ACEndDoorActor : public AActor
{
	GENERATED_BODY()

public:
	ACEndDoorActor();

protected:
	virtual void BeginPlay() override;

	// ===== Components =====
	UPROPERTY(VisibleAnywhere, Category="EndDoor")
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, Category="EndDoor")
	UStaticMeshComponent* DoorMesh;

	UPROPERTY(VisibleAnywhere, Category="EndDoor|Interact")
	USphereComponent* InteractSphere;

	UPROPERTY(VisibleAnywhere, Category="EndDoor|Interact")
	UTextRenderComponent* PromptText;

	// ===== Config =====
	// 단일 키 아이템 ID (CPickupItem 파생 BP의 ItemData.ItemID와 동일해야 함)
	UPROPERTY(EditAnywhere, Category="EndDoor|Key")
	FName KeyItemID = "Item_Key";

	// 필요 키 개수
	UPROPERTY(EditAnywhere, Category="EndDoor|Key", meta=(ClampMin="1"))
	int32 RequiredKeyCount = 3;

	// 오버랩 시 자동 클리어
	UPROPERTY(EditAnywhere, Category="EndDoor|Clear")
	bool bAutoClearOnOverlap = true;

	// 클리어 시 표시할 UI
	UPROPERTY(EditAnywhere, Category="EndDoor|UI")
	TSubclassOf<UUserWidget> ClearWidgetClass;

private:
	// 현재 오버랩 중인 플레이어
	UPROPERTY()
	TWeakObjectPtr<ACPlayerCharacter> OverlappingPlayer;

	// 중복 실행 방지
	bool bCleared = false;

	// 인벤토리 델리게이트 바인딩 상태
	bool bInventoryBound = false;

	// 내부: 현재 플레이어 인벤토리에 키가 충분한지 검사
	bool HasRequiredKeys(ACPlayerCharacter* Player) const;

	// 내부: 현재 키 보유 수량
	int32 GetCurrentKeyCount(ACPlayerCharacter* Player) const;

	// 내부: 프롬프트 업데이트 (n/RequiredKeyCount)
	void UpdatePrompt(ACPlayerCharacter* Player) const;

	// 내부: 클리어 처리
	void TriggerClear(ACPlayerCharacter* Player);

	// 내부: 인벤토리 변경 콜백
	UFUNCTION()
	void OnPlayerInventoryUpdated();

	// 오버랩 핸들러
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	                    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	                  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// 인벤토리 델리게이트 바인딩/해제
	void BindInventory(ACPlayerCharacter* Player);
	void UnbindInventory();
};