// CPickupItem.cpp 전체 수정
#include "CPickupItem.h"
#include "Components/SphereComponent.h"
#include "00_Character/CPlayerCharacter.h"
#include "00_Character/02_Component/03_Inventory/CInventoryComponent.h"
#include "Global.h"

ACPickupItem::ACPickupItem()
{
    PrimaryActorTick.bCanEverTick = false;

    // 메시 컴포넌트 생성
    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    RootComponent = Mesh;

    // 충돌 구체 생성
    CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
    CollisionSphere->SetupAttachment(Mesh);
    CollisionSphere->SetSphereRadius(100.0f);
    // 플레이어만 오버랩하도록 명확히
    CollisionSphere->SetCollisionProfileName(TEXT("OverlapOnlyPawn"));

    // 기본 아이템 데이터 설정
    ItemData.ItemID = TEXT("Item_HealthPotion");
    ItemData.ItemName = TEXT("체력 물약");
    ItemData.Quantity = 1;
    // 필요 시 장착 가능 및 클래스 지정
    // ItemData.bIsEquippable = true;
    // ItemData.ItemClass = ...; 또는 ItemData.ThrowableClass = ...;
}

void ACPickupItem::BeginPlay()
{
    Super::BeginPlay();

    // 오버랩 이벤트 등록
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

    // 인벤토리 컴포넌트 가져오기
    UCInventoryComponent* InventoryComp = Player->GetInventoryComponent();
    if (!InventoryComp)
    {
        return;
    }

    // 아이템 추가 시도
    if (InventoryComp->AddItem(ItemData))
    {
        CLog::Log(ItemData.ItemName + TEXT(" 획득! (액터: ") + GetName() + TEXT(")"));

        // 오버랩 이벤트 해제 (중복 실행 방지)
        if (CollisionSphere)
        {
            CollisionSphere->OnComponentBeginOverlap.RemoveAll(this);
            CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }

        // 아이템 제거
        Destroy();
    }
    else
    {
        CLog::Log(FString::Printf(TEXT("인벤토리가 꽉 참! %s 획득 실패"), *ItemData.ItemName));
    }

}