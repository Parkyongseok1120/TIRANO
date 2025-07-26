// CPickupItem.cpp 전체 수정
#include "CPickupItem.h"
#include "Components/SphereComponent.h"
#include "00_Character/CPlayerCharacter.h"
#include "00_Character/02_Component/CInventoryComponent.h"
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
    CollisionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

    // 기본 아이템 데이터 설정
    ItemData.ItemID = TEXT("Item_HealthPotion");
    ItemData.ItemName = TEXT("체력 물약");
    ItemData.Quantity = 1;
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

void ACPickupItem::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                 UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                 bool bFromSweep, const FHitResult& SweepResult)
{
    // 이미 제거된 액터라면 무시 (UE5에서는 IsValid만 사용)
    if (!IsValid(this))
    {
        return;
    }

    // 플레이어와 충돌했는지 확인
    ACPlayerCharacter* Player = Cast<ACPlayerCharacter>(OtherActor);
    if (!Player || !IsValid(Player))
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
        CLog::Log("인벤토리가 꽉 참! " + ItemData.ItemName + TEXT(" 획득 실패"));
    }
}