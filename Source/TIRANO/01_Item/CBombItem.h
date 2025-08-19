// BombItem.h
#pragma once

#include "CoreMinimal.h"
#include "CThrowableItemBase.h"
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

	virtual void OnFuseExpired() override;
	virtual void OnMeshHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;
	
	// 바닥 충돌 즉시 터질지 여부
	UPROPERTY(EditAnywhere, Category = "Bomb")
	bool bExplodeOnHit = false;

	void Explode();
};