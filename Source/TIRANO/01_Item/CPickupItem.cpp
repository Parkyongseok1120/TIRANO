// CPickupItem.cpp 전체 수정
#include "CPickupItem.h"
#include "Components/SphereComponent.h"
#include "00_Character/CPlayerCharacter.h"
#include "00_Character/02_Component/03_Inventory/CInventoryComponent.h"
#include "Global.h"

ACPickupItem::ACPickupItem()
{
    PrimaryActorTick.bCanEverTick = false;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    RootComponent = Mesh;

    CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
    CollisionSphere->SetupAttachment(Mesh);
    CollisionSphere->SetSphereRadius(100.0f);
    CollisionSphere->SetCollisionProfileName(TEXT("OverlapOnlyPawn"));

    ItemData.ItemID = TEXT("Item_HealthPotion");
    ItemData.ItemName = TEXT("체력 물약");
    ItemData.Quantity = 1;

}

void ACPickupItem::BeginPlay()
{
    Super::BeginPlay();

    if (CollisionSphere)
    {
        CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &ACPickupItem::OnOverlapBegin);
    }

    
}

void ACPickupItem::OnOverlapBegin(UPrimitiveComponent* /*OverlappedComponent*/, AActor* OtherActor,
                                 UPrimitiveComponent* /*OtherComp*/, int32 /*OtherBodyIndex*/,
                                 bool /*bFromSweep*/, const FHitResult& /*SweepResult*/)
{
    ACPlayerCharacter* Player = Cast<ACPlayerCharacter>(OtherActor);
    if (!Player)
    {
        return;
    }

    UCInventoryComponent* InventoryComp = Player->GetInventoryComponent();
    if (!InventoryComp)
    {
        return;
    }

    if (InventoryComp->AddItem(ItemData))
    {
        CLog::Log(ItemData.ItemName + TEXT(" 획득! (액터: ") + GetName() + TEXT(")"));

        if (CollisionSphere)
        {
            CollisionSphere->OnComponentBeginOverlap.RemoveAll(this);
            CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }

        Destroy();
    }
    else
    {
        CLog::Log(FString::Printf(TEXT("인벤토리가 꽉 참! %s 획득 실패"), *ItemData.ItemName));
    }

}
