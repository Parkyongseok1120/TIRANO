// BombItem.cpp
#include "CBombItem.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
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
	// 데미지
	UGameplayStatics::ApplyRadialDamage(this, ExplosionDamage, GetActorLocation(), ExplosionRadius, nullptr, TArray<AActor*>{ GetOwner() }, this, GetInstigatorController(), true);

	// FX/SFX
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