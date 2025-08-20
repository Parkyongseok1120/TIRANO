// BombItem.h
#pragma once

#include "CoreMinimal.h"
#include "CThrowableItemBase.h"
#include "CPickupItem.h" // 반드시 include!
#include "CBombItem.generated.h"

UCLASS()
class TIRANO_API ACBombItem : public ACThrowableItemBase
{
	GENERATED_BODY()

public:
	ACBombItem();

protected:
	UPROPERTY(EditAnywhere, Category = "Bomb")
	float ExplosionDamage = 60.f;

	UPROPERTY(EditAnywhere, Category = "Bomb")
	float ExplosionRadius = 300.f;

	UPROPERTY(EditAnywhere, Category = "Bomb")
	UParticleSystem* ExplosionFX;

	UPROPERTY(EditAnywhere, Category = "Bomb")
	USoundBase* ExplosionSFX;

	// ★ 여기 추가!
	// BP에서 'Pickup 파생 클래스' 선택 가능
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bomb")
	TSubclassOf<ACPickupItem> PickupClass;

	virtual void OnFuseExpired() override;
	virtual void OnMeshHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;

	UPROPERTY(EditAnywhere, Category = "Bomb")
	bool bExplodeOnHit = false;

	void Explode();
};