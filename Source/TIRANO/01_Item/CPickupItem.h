// CPickupItem.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "00_Character/02_Component/03_Inventory/CInventoryItem.h"
#include "CPickupItem.generated.h"

UCLASS()
class TIRANO_API ACPickupItem : public AActor
{
	GENERATED_BODY()
    
public:    
	ACPickupItem();

protected:
	virtual void BeginPlay() override;
    
	UPROPERTY(EditAnywhere, Category = "Components")
	UStaticMeshComponent* Mesh;
    
	UPROPERTY(EditAnywhere, Category = "Components")
	class USphereComponent* CollisionSphere;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FInventoryItem ItemData;
    
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(EditAnywhere, Category = "Effects")
	UParticleSystem* PickupEffect;
    
	UPROPERTY(EditAnywhere, Category = "Effects")
	USoundBase* PickupSound;
};