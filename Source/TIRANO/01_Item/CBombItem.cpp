// BombItem.cpp
#include "CBombItem.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "00_Character/02_Component/03_Inventory/CInventoryComponent.h"
#include "Sound/SoundBase.h"

ACBombItem::ACBombItem()
{
	// 기본 퓨즈 시간
	FuseTime = 2.0f;
	ThrowSpeed = 1400.f;
}

void ACBombItem::OnFuseExpired()
{
	Explode();
}

void ACBombItem::OnMeshHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (bExplodeOnHit)
	{
		Explode();
	}
}

void ACBombItem::Explode()
{
	// (예시) 동일 ID 보유 여부 확인하여 추가 효과 분기 가능
	if (UCInventoryComponent* Inv = GetOwnerInventory())
	{
		const bool bHasSameID = Inv->HasItemByID(GetItemID());
		// 필요시 bHasSameID에 따라 다른 데미지/반경/드랍 로직 처리 가능
		// UE_LOG(LogTemp, Log, TEXT("Has same ID in inventory? %s"), bHasSameID ? TEXT("Yes") : TEXT("No"));
	}

	UGameplayStatics::ApplyRadialDamage(this, ExplosionDamage, GetActorLocation(), ExplosionRadius, nullptr, TArray<AActor*>{ GetOwner() }, this, GetInstigatorController(), true);

	if (ExplosionFX)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionFX, GetActorTransform());
	}
	if (ExplosionSFX)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ExplosionSFX, GetActorLocation());
	}

	Destroy();
}