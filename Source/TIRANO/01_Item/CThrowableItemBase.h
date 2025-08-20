// ThrowableItemBase.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "00_Character/02_Component/03_Inventory/CInventoryItem.h"
#include "CThrowableItemBase.generated.h"

class UStaticMeshComponent;
class UProjectileMovementComponent;
class UCInventoryComponent;

UCLASS()
class TIRANO_API ACThrowableItemBase : public AActor
{
	GENERATED_BODY()

public:
	ACThrowableItemBase();

	virtual void SetHeld(bool bHeld, AActor* NewOwner, USceneComponent* AttachTo, FName SocketName, const FVector& LocationOffset = FVector::ZeroVector, const FRotator& RotationOffset = FRotator::ZeroRotator);
	virtual void Throw(const FVector& StartLocation, const FVector& Direction, AActor* InstigatorActor);
	virtual void InitializeFromInventoryItem(const FInventoryItem& InItem, UCInventoryComponent* InInventory);

	UFUNCTION(BlueprintPure, Category="Throwable")
	const FString& GetItemID() const { return ItemID; }

	UFUNCTION(BlueprintPure, Category="Throwable")
	UCInventoryComponent* GetOwnerInventory() const { return OwnerInventory.Get(); }

	UFUNCTION(BlueprintPure, Category="Throwable")
	const FInventoryItem& GetSourceItem() const { return SourceItem; }

protected:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(EditAnywhere, Category = "Throwable")
	float ThrowSpeed = 1500.f;

	UPROPERTY(EditAnywhere, Category = "Throwable")
	float FuseTime = 2.0f;

	UPROPERTY(EditAnywhere, Category = "Throwable")
	float OwnerIgnoreTime = 0.2f;

	// 충돌 소리
	UPROPERTY(EditAnywhere, Category="Throwable|Noise")
	float ImpactNoiseLoudness = 1.0f;

	UPROPERTY(EditAnywhere, Category="Throwable|Noise")
	float ImpactNoiseRange = 1600.f;

	FTimerHandle FuseTimerHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Throwable|Meta")
	FString ItemID;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Throwable|Meta")
	FInventoryItem SourceItem;

	TWeakObjectPtr<UCInventoryComponent> OwnerInventory;

	virtual void OnFuseExpired();
	virtual void SetupForHeld();
	virtual void SetupForThrown();

	void TemporarilyIgnoreOwner(AActor* InOwner);

	UFUNCTION()
	virtual void OnMeshHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};