#include "01_Item/CFlashlightItem.h"
#include "Components/SpotLightComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "00_Character/01_Monster/EnemyCharacter.h"
#include "Engine/World.h"
#include "Engine/EngineTypes.h"
#include "CollisionQueryParams.h"
#include "Camera/CameraComponent.h"
#include "00_Character/CPlayerCharacter.h"
#include "00_Character/02_Component/03_Inventory/CInventoryComponent.h"

ACFlashlightItem::ACFlashlightItem()
{
	PrimaryActorTick.bCanEverTick = true;

	Spot = CreateDefaultSubobject<USpotLightComponent>(TEXT("FlashlightSpot"));
	SetRootComponent(Spot);
	Spot->Intensity = 5000.f;
	Spot->AttenuationRadius = LightRange;
	Spot->InnerConeAngle = InnerConeAngle;
	Spot->OuterConeAngle = OuterConeAngle;
	Spot->SetVisibility(false); // 기본 꺼짐
}

void ACFlashlightItem::BeginPlay()
{
	Super::BeginPlay();
}

void ACFlashlightItem::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// 플레이어 카메라 방향에 라이트 방향 동기화
	if (AActor* OwnerActor = GetOwner())
	{
		if (UCameraComponent* Cam = OwnerActor->FindComponentByClass<UCameraComponent>())
		{
			Spot->SetWorldRotation(Cam->GetComponentRotation());
		}
	}

	if (bOn)
	{
		if (BatteryPercent > 0.f)
		{
			BatteryPercent = FMath::Max(0.f, BatteryPercent - DrainPercentPerSecond * DeltaSeconds);
		}

		if (BatteryPercent <= 0.f)
		{
			// 방전 처리: 끄고 인벤토리에서 손전등 제거
			if (bOn) TurnOff();
			RemoveFromInventoryOnDeplete();
			return;
		}
	}
}

void ACFlashlightItem::AttachToHand(USceneComponent* Mesh, FName SocketName, const FVector& Offset, const FRotator& RotOffset)
{
	if (!Mesh) return;
	AttachToComponent(Mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);
	AddActorLocalOffset(Offset);
	AddActorLocalRotation(RotOffset);
}

void ACFlashlightItem::InitializeFromInventoryItem(const FInventoryItem& InItem, UCInventoryComponent* InInventory, ACPlayerCharacter* InOwner)
{
	ItemIDForInventory = InItem.ItemID;
	OwnerInventory = InInventory;
	OwningPlayer = InOwner;
}

void ACFlashlightItem::Toggle()
{
	if (bOn) TurnOff();
	else TurnOn();
}

void ACFlashlightItem::TurnOn()
{
	if (BatteryPercent <= 0.f) return;
	bOn = true;
	if (Spot) Spot->SetVisibility(true);

	// 주기적으로 적 체크
	GetWorld()->GetTimerManager().SetTimer(StunTimerHandle, this, &ACFlashlightItem::ApplyStunCone, StunCheckInterval, true);
}

void ACFlashlightItem::TurnOff()
{
	bOn = false;
	if (Spot) Spot->SetVisibility(false);
	GetWorld()->GetTimerManager().ClearTimer(StunTimerHandle);
}

void ACFlashlightItem::ApplyStunCone()
{
	if (!Spot) return;

	const FVector Start = Spot->GetComponentLocation();
	const FVector Dir = Spot->GetForwardVector();
	const float Radius = FMath::Min(StunMaxDistance, Spot->AttenuationRadius);

	// Pawn만 오버랩
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	// 자기 자신 무시
	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(this);

	// 결과: 액터만 받음
	TArray<AActor*> OverlappedActors;
	const bool bAny = UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(), Start, Radius, ObjectTypes, AEnemyCharacter::StaticClass(), IgnoreActors, OverlappedActors);

	if (!bAny) return;

	// 가시선 체크용 파라미터
	FCollisionQueryParams Params(SCENE_QUERY_STAT(FlashlightStun), /*bTraceComplex=*/false, this);

	for (AActor* HitActor : OverlappedActors)
	{
		AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(HitActor);
		if (!Enemy) continue;

		// 각도/거리 판정
		const FVector ToEnemy = Enemy->GetActorLocation() - Start;
		const float Dist = ToEnemy.Size();
		if (Dist > StunMaxDistance) continue;

		const FVector ToEnemyN = ToEnemy.GetSafeNormal();
		const float CosAngle = FVector::DotProduct(Dir, ToEnemyN);
		const float CosOuter = FMath::Cos(FMath::DegreesToRadians(OuterConeAngle));
		if (CosAngle < CosOuter) continue; // 콘 밖

		// 가시선 체크
		FHitResult Hit;
		const FVector End = Enemy->GetActorLocation() + FVector(0, 0, 50);
		if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
		{
			if (Hit.GetActor() != Enemy)
				continue; // 가려짐
		}

		// 강도 계산
		const float AngleFactor = FMath::GetMappedRangeValueClamped(FVector2D(CosOuter, 1.f), FVector2D(0.f, 1.f), CosAngle);
		const float DistFactor = 1.f - FMath::Clamp(Dist / StunMaxDistance, 0.f, 1.f);
		const float Intensity = AngleFactor * DistFactor;

		Enemy->NotifyShinedByFlashlight(Intensity, Start);
	}
}

void ACFlashlightItem::RemoveFromInventoryOnDeplete()
{
	if (bPendingRemovalOnDeplete)
		return;
	bPendingRemovalOnDeplete = true;

	// 인벤토리에서 손전등 1개 제거
	if (UCInventoryComponent* Inv = OwnerInventory.Get())
	{
		int32 SlotIdx = Inv->FindFirstSlotIndexByID(ItemIDForInventory);
		if (SlotIdx >= 0)
		{
			Inv->RemoveItem(SlotIdx, 1);
		}
	}

	// 자기 자신 파괴(플레이어는 OnDestroyed로 HeldItemActor 정리)
	Destroy();
}