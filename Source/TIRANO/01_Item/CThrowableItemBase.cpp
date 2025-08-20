#include "CThrowableItemBase.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "00_Character/02_Component/03_Inventory/CInventoryComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AISense_Hearing.h"
#include "TimerManager.h"

ACThrowableItemBase::ACThrowableItemBase()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	Mesh->SetCollisionProfileName(TEXT("NoCollision"));
	Mesh->SetSimulatePhysics(false);
	Mesh->SetNotifyRigidBodyCollision(true);
	Mesh->OnComponentHit.AddDynamic(this, &ACThrowableItemBase::OnMeshHit);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->bAutoActivate = false;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;
	ProjectileMovement->Bounciness = 0.2f;
	ProjectileMovement->ProjectileGravityScale = 1.0f;
}

void ACThrowableItemBase::InitializeFromInventoryItem(const FInventoryItem& InItem, UCInventoryComponent* InInventory)
{
	SourceItem = InItem;
	ItemID = InItem.ItemID;
	OwnerInventory = InInventory;
}

void ACThrowableItemBase::SetupForHeld()
{
	ProjectileMovement->StopMovementImmediately();
	ProjectileMovement->Deactivate();

	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetSimulatePhysics(false);
}

void ACThrowableItemBase::SetupForThrown()
{
	Mesh->SetCollisionProfileName(TEXT("PhysicsActor"));
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Mesh->SetSimulatePhysics(false);
	ProjectileMovement->Activate();
}

void ACThrowableItemBase::SetHeld(bool bHeld, AActor* NewOwner, USceneComponent* AttachTo, FName SocketName, const FVector& LocationOffset, const FRotator& RotationOffset)
{
	SetOwner(NewOwner);

	if (bHeld && AttachTo)
	{
		AttachToComponent(AttachTo, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);
		AddActorLocalOffset(LocationOffset);
		AddActorLocalRotation(RotationOffset);
		SetupForHeld();
	}
	else
	{
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		SetupForThrown();
	}
}

void ACThrowableItemBase::Throw(const FVector& StartLocation, const FVector& Direction, AActor* InstigatorActor)
{
	SetActorLocation(StartLocation);
	SetHeld(false, InstigatorActor, nullptr, NAME_None);

	TemporarilyIgnoreOwner(InstigatorActor);

	if (ProjectileMovement)
	{
		ProjectileMovement->Velocity = Direction.GetSafeNormal() * ThrowSpeed;
	}

	if (FuseTime > 0.f)
	{
		GetWorldTimerManager().SetTimer(FuseTimerHandle, this, &ACThrowableItemBase::OnFuseExpired, FuseTime, false);
	}
}

void ACThrowableItemBase::TemporarilyIgnoreOwner(AActor* InOwner)
{
	if (!InOwner || !Mesh) return;

	Mesh->IgnoreActorWhenMoving(InOwner, true);

	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, [this, InOwner]()
	{
		if (Mesh) Mesh->IgnoreActorWhenMoving(InOwner, false);
	}, OwnerIgnoreTime, false);
}

void ACThrowableItemBase::OnFuseExpired()
{
	Destroy();
}

void ACThrowableItemBase::OnMeshHit(UPrimitiveComponent* /*HitComp*/, AActor* /*OtherActor*/, UPrimitiveComponent* /*OtherComp*/, FVector /*NormalImpulse*/, const FHitResult& /*Hit*/)
{
	// 충돌 시 청각 이벤트 보고
	UAISense_Hearing::ReportNoiseEvent(GetWorld(), GetActorLocation(), ImpactNoiseLoudness, GetOwner(), ImpactNoiseRange, TEXT("ImpactNoise"));
}