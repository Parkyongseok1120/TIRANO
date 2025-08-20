#include "03_World/CDoorActor.h"
#include "01_Item/CNoiseFunctionLibrary.h"

ACDoorActor::ACDoorActor()
{
	PrimaryActorTick.bCanEverTick = false;
	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
	SetRootComponent(DoorMesh);
}

void ACDoorActor::ToggleDoor()
{
	bOpen = !bOpen;
	PlayDoorAnimAndSound(bOpen);

	// 문 열릴 때 소음 이벤트
	if (bOpen)
	{
		UCNoiseFunctionLibrary::ReportNoise(this, GetActorLocation(), OpenNoiseLoudness, this, OpenNoiseRange);
	}
}

void ACDoorActor::PlayDoorAnimAndSound(bool /*bOpening*/)
{
	// TODO: 회전/타임라인/사운드 재생 구현
}