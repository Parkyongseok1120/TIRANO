// ThrowableItemBase.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CThrowableItemBase.generated.h"

class UStaticMeshComponent;
class UProjectileMovementComponent;

UCLASS()
class TIRANO_API ACThrowableItemBase : public AActor
{
	GENERATED_BODY()

public:
	ACThrowableItemBase();

	// 장착/해제
	virtual void SetHeld(bool bHeld, AActor* NewOwner, USceneComponent* AttachTo, FName SocketName, const FVector& LocationOffset = FVector::ZeroVector, const FRotator& RotationOffset = FRotator::ZeroRotator);

	// 던지기
	virtual void Throw(const FVector& StartLocation, const FVector& Direction, AActor* InstigatorActor);

protected:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UProjectileMovementComponent* ProjectileMovement;

	// 던지는 속도
	UPROPERTY(EditAnywhere, Category = "Throwable")
	float ThrowSpeed = 1500.f;

	// 던진 후 폭발까지 시간(0이면 폭발 없음)
	UPROPERTY(EditAnywhere, Category = "Throwable")
	float FuseTime = 2.0f;

	// 플레이어와의 초기 충돌 무시 시간
	UPROPERTY(EditAnywhere, Category = "Throwable")
	float OwnerIgnoreTime = 0.2f;

	FTimerHandle FuseTimerHandle;

	// 폭발 또는 효과 처리(파생에서 구현)
	virtual void OnFuseExpired();

	// 초기화 및 충돌 설정
	virtual void SetupForHeld();
	virtual void SetupForThrown();

	// 소유자와의 충돌 잠시 무시
	void TemporarilyIgnoreOwner(AActor* InOwner);

	// 충돌 콜백(필요 시)
	UFUNCTION()
	virtual void OnMeshHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};