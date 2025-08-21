#include "03_World/CEndDoorActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/TextRenderComponent.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "00_Character/CPlayerCharacter.h"
#include "00_Character/02_Component/03_Inventory/CInventoryComponent.h"

ACEndDoorActor::ACEndDoorActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
	DoorMesh->SetupAttachment(Root);
	DoorMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	InteractSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractSphere"));
	InteractSphere->SetupAttachment(Root);
	InteractSphere->SetSphereRadius(180.f);
	InteractSphere->SetCollisionProfileName(TEXT("OverlapOnlyPawn"));
	InteractSphere->SetGenerateOverlapEvents(true);

	PromptText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("PromptText"));
	PromptText->SetupAttachment(Root);
	PromptText->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
	PromptText->SetVerticalAlignment(EVerticalTextAligment::EVRTA_TextCenter);
	PromptText->SetWorldSize(26.f);
	PromptText->SetText(FText::FromString(TEXT("키 0/3")));
	PromptText->SetHiddenInGame(true);
	PromptText->SetRelativeLocation(FVector(0, 0, 140));

	InteractSphere->OnComponentBeginOverlap.AddDynamic(this, &ACEndDoorActor::OnOverlapBegin);
	InteractSphere->OnComponentEndOverlap.AddDynamic(this, &ACEndDoorActor::OnOverlapEnd);
}

void ACEndDoorActor::BeginPlay()
{
	Super::BeginPlay();
}

int32 ACEndDoorActor::GetCurrentKeyCount(ACPlayerCharacter* Player) const
{
	if (!Player) return 0;
	if (UCInventoryComponent* Inv = Player->GetInventoryComponent())
	{
		return Inv->GetTotalCountByID(KeyItemID.ToString());
	}
	return 0;
}

bool ACEndDoorActor::HasRequiredKeys(ACPlayerCharacter* Player) const
{
	return GetCurrentKeyCount(Player) >= RequiredKeyCount;
}

void ACEndDoorActor::UpdatePrompt(ACPlayerCharacter* Player) const
{
	if (!PromptText) return;

	const int32 Current = GetCurrentKeyCount(Player);
	const FString Msg = FString::Printf(TEXT("Key: %d/%d"), Current, RequiredKeyCount);
	PromptText->SetText(FText::FromString(Msg));
}

void ACEndDoorActor::TriggerClear(ACPlayerCharacter* Player)
{
	if (bCleared || !Player) return;
	bCleared = true;

	// 게임 일시정지 + UI 표시
	if (APlayerController* PC = Cast<APlayerController>(Player->GetController()))
	{
		UGameplayStatics::SetGamePaused(this, true);

		if (ClearWidgetClass)
		{
			if (UUserWidget* W = CreateWidget<UUserWidget>(PC, ClearWidgetClass))
			{
				W->AddToViewport(100);
				FInputModeUIOnly Mode;
				Mode.SetWidgetToFocus(W->TakeWidget());
				PC->SetInputMode(Mode);
				PC->bShowMouseCursor = true;
			}
		}
	}

	// 정리
	if (PromptText) PromptText->SetHiddenInGame(true);
	UnbindInventory();
	OverlappingPlayer = nullptr;
}

void ACEndDoorActor::OnPlayerInventoryUpdated()
{
	if (!OverlappingPlayer.IsValid() || bCleared)
		return;

	// 프롬프트 갱신
	UpdatePrompt(OverlappingPlayer.Get());

	// 조건 충족 시 클리어
	if (HasRequiredKeys(OverlappingPlayer.Get()))
	{
		TriggerClear(OverlappingPlayer.Get());
	}
}

void ACEndDoorActor::BindInventory(ACPlayerCharacter* Player)
{
	if (bInventoryBound || !Player) return;

	if (UCInventoryComponent* Inv = Player->GetInventoryComponent())
	{
		Inv->OnInventoryUpdated.AddDynamic(this, &ACEndDoorActor::OnPlayerInventoryUpdated);
		bInventoryBound = true;
	}
}

void ACEndDoorActor::UnbindInventory()
{
	if (!OverlappingPlayer.IsValid()) { bInventoryBound = false; return; }

	if (UCInventoryComponent* Inv = OverlappingPlayer->GetInventoryComponent())
	{
		Inv->OnInventoryUpdated.RemoveDynamic(this, &ACEndDoorActor::OnPlayerInventoryUpdated);
	}
	bInventoryBound = false;
}

void ACEndDoorActor::OnOverlapBegin(UPrimitiveComponent* /*OverlappedComp*/, AActor* OtherActor,
                                    UPrimitiveComponent* /*OtherComp*/, int32 /*OtherBodyIndex*/, bool /*bFromSweep*/, const FHitResult& /*Sweep*/)
{
	ACPlayerCharacter* Player = Cast<ACPlayerCharacter>(OtherActor);
	if (!Player || bCleared) return;

	OverlappingPlayer = Player;

	// 프롬프트 표시 + 현재 보유 수량 갱신
	if (PromptText)
	{
		UpdatePrompt(Player);
		PromptText->SetHiddenInGame(false);
	}

	// 인벤토리 변경 감지 바인딩
	BindInventory(Player);

	// 오버랩 순간에 바로 충족했는지 검사
	if (bAutoClearOnOverlap && HasRequiredKeys(Player))
	{
		TriggerClear(Player);
	}
}

void ACEndDoorActor::OnOverlapEnd(UPrimitiveComponent* /*OverlappedComp*/, AActor* OtherActor,
                                  UPrimitiveComponent* /*OtherComp*/, int32 /*OtherBodyIndex*/)
{
	ACPlayerCharacter* Player = Cast<ACPlayerCharacter>(OtherActor);
	if (!Player) return;

	// 프롬프트 숨김
	if (PromptText) PromptText->SetHiddenInGame(true);

	// 델리게이트 해제
	if (OverlappingPlayer.Get() == Player)
	{
		UnbindInventory();
		OverlappingPlayer = nullptr;
	}
}