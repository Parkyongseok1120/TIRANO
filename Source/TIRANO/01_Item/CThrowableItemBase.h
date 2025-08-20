// ThrowableItemBase.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "00_Character/02_Component/03_Inventory/CInventoryItem.h" // FInventoryItem
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

    // 장착/해제
    virtual void SetHeld(bool bHeld, AActor* NewOwner, USceneComponent* AttachTo, FName SocketName, const FVector& LocationOffset = FVector::ZeroVector, const FRotator& RotationOffset = FRotator::ZeroRotator);

    // 던지기
    virtual void Throw(const FVector& StartLocation, const FVector& Direction, AActor* InstigatorActor);

    // 인벤토리/아이템 정보 주입
    virtual void InitializeFromInventoryItem(const FInventoryItem& InItem, UCInventoryComponent* InInventory);

    // 조회용 Getter
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

    FTimerHandle FuseTimerHandle;

    // 주입받은 메타: 이 액터가 어떤 인벤토리 아이템에서 생성되었는지
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Throwable|Meta")
    FString ItemID;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Throwable|Meta")
    FInventoryItem SourceItem;

    // 소유자의 인벤토리(존재하면 동일 ID 존재 여부 질의 가능)
    TWeakObjectPtr<UCInventoryComponent> OwnerInventory;

    virtual void OnFuseExpired();
    virtual void SetupForHeld();
    virtual void SetupForThrown();

    void TemporarilyIgnoreOwner(AActor* InOwner);

    UFUNCTION()
    virtual void OnMeshHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};