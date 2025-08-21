#include "03_World/CDoorActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/TextRenderComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "01_Item/CNoiseFunctionLibrary.h"
#include "00_Character/CPlayerCharacter.h"

ACDoorActor::ACDoorActor()
{
	PrimaryActorTick.bCanEverTick = true;

	// 경첩 피벗을 루트로
	HingeRoot = CreateDefaultSubobject<USceneComponent>(TEXT("HingeRoot"));
	SetRootComponent(HingeRoot);

	// 문 메시
	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
	DoorMesh->SetupAttachment(HingeRoot);
	DoorMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	// 상호작용 스피어
	InteractSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractSphere"));
	InteractSphere->SetupAttachment(HingeRoot);
	InteractSphere->SetSphereRadius(150.f);
	InteractSphere->SetCollisionProfileName(TEXT("OverlapOnlyPawn"));
	InteractSphere->SetGenerateOverlapEvents(true);

	// 월드 텍스트(선택)
	PromptText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("PromptText"));
	PromptText->SetupAttachment(HingeRoot);
	PromptText->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
	PromptText->SetVerticalAlignment(EVerticalTextAligment::EVRTA_TextCenter);
	PromptText->SetTextRenderColor(FColor::White);
	PromptText->SetWorldSize(24.f);
	PromptText->SetText(FText::FromString(TEXT("F키로 문 열기")));
	PromptText->SetHiddenInGame(true);
	PromptText->SetRelativeLocation(FVector(0, 0, 120));

	InteractSphere->OnComponentBeginOverlap.AddDynamic(this, &ACDoorActor::OnInteractBegin);
	InteractSphere->OnComponentEndOverlap.AddDynamic(this, &ACDoorActor::OnInteractEnd);
}

void ACDoorActor::BeginPlay()
{
	Super::BeginPlay();

	// 에디터에서 배치된 힌지의 초기 회전을 기준선으로 저장
	InitialHingeRot = HingeRoot->GetRelativeRotation();

	// 시작 상태 반영: 기준선 + ΔYaw
	const float Sign = bOpenClockwise ? +1.f : -1.f;
	const float DeltaYaw = bOpen ? (Sign * OpenAngle) : 0.f;
	HingeRoot->SetRelativeRotation(InitialHingeRot + FRotator(0.f, DeltaYaw, 0.f));
}

void ACDoorActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UpdateDoorAnimation(DeltaSeconds);
}

void ACDoorActor::UpdateDoorAnimation(float DeltaSeconds)
{
	if (!bAnimating)
		return;

	AnimElapsed += DeltaSeconds;
	const float AlphaRaw = FMath::Clamp(AnimElapsed / FMath::Max(0.01f, OpenTime), 0.f, 1.f);
	const float Alpha = (OpenCurve != nullptr) ? OpenCurve->GetFloatValue(AlphaRaw) : AlphaRaw;

	// 회전 보간: 최단 경로로 보간
	const FRotator NewRot = UKismetMathLibrary::RLerp(AnimStartRot, AnimTargetRot, Alpha, true);
	HingeRoot->SetRelativeRotation(NewRot);

	if (AlphaRaw >= 1.f)
	{
		bAnimating = false;
	}
}

void ACDoorActor::StartDoorAnimation(bool bOpenTarget)
{
	AnimStartRot = HingeRoot->GetRelativeRotation();

	const float Sign = bOpenClockwise ? +1.f : -1.f;
	const float DeltaYaw = bOpenTarget ? (Sign * OpenAngle) : 0.f;

	// 목표 = 기준선 + ΔYaw
	AnimTargetRot = InitialHingeRot + FRotator(0.f, DeltaYaw, 0.f);

	AnimElapsed = 0.f;
	bAnimating = true;
}

void ACDoorActor::ToggleDoor()
{
	bOpen = !bOpen;
	StartDoorAnimation(bOpen);
	PlayDoorAnimAndSound(bOpen);

	if (bOpen)
	{
		UCNoiseFunctionLibrary::ReportNoise(this, GetActorLocation(), OpenNoiseLoudness, this, OpenNoiseRange);
	}
}

void ACDoorActor::PlayDoorAnimAndSound(bool bOpening)
{
	USoundBase* Snd = bOpening ? OpenSound : CloseSound;
	if (Snd)
	{
		UGameplayStatics::PlaySoundAtLocation(this, Snd, GetActorLocation());
	}
}

void ACDoorActor::OnInteractBegin(UPrimitiveComponent* /*OverlappedComp*/, AActor* OtherActor,
                                  UPrimitiveComponent* /*OtherComp*/, int32 /*OtherBodyIndex*/, bool /*bFromSweep*/, const FHitResult& /*Sweep*/)
{
	ACPlayerCharacter* Player = Cast<ACPlayerCharacter>(OtherActor);
	if (!Player) return;

	// 플레이어 HUD 위젯 표시 위임
	Player->SetNearbyDoor(this, true);
}

void ACDoorActor::OnInteractEnd(UPrimitiveComponent* /*OverlappedComp*/, AActor* OtherActor,
                                UPrimitiveComponent* /*OtherComp*/, int32 /*OtherBodyIndex*/)
{
	ACPlayerCharacter* Player = Cast<ACPlayerCharacter>(OtherActor);
	if (!Player) return;

	Player->SetNearbyDoor(this, false);
}