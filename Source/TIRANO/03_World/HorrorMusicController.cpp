#include "03_World/HorrorMusicController.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "00_Character/01_Monster/EnemyCharacter.h"
#include "00_Character/CPlayerCharacter.h"

AHorrorMusicController::AHorrorMusicController()
{
	PrimaryActorTick.bCanEverTick = true;

	MusicComp = CreateDefaultSubobject<UAudioComponent>(TEXT("MusicComp"));
	SetRootComponent(MusicComp);
	MusicComp->bAutoActivate = false; // 수동 재생
	MusicComp->bAllowSpatialization = false; // 2D BGM 권장
	MusicComp->SetUISound(false);
}

void AHorrorMusicController::BeginPlay()
{
	Super::BeginPlay();

	EnsureDefaults();

	if (MusicLoop)
	{
		MusicComp->SetSound(MusicLoop);
		MusicComp->Play();
		MusicComp->SetVolumeMultiplier(0.f);
		CurrentVolume = 0.f;
	}
}

void AHorrorMusicController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	TimeAcc += DeltaSeconds;
	if (TimeAcc < UpdateInterval) return;
	TimeAcc = 0.f;

	const int32 Level = ComputeLevel();
	const float TargetVolume = (Level > 0 && LevelVolumes.IsValidIndex(Level - 1)) ? LevelVolumes[Level - 1] : 0.f;

	CurrentVolume = FMath::FInterpTo(CurrentVolume, TargetVolume, UpdateInterval, FadeSpeed);
	MusicComp->SetVolumeMultiplier(CurrentVolume);
}

int32 AHorrorMusicController::ComputeLevel() const
{
	const float Dist = GetNearestEnemyDistance();
	if (DistanceThresholds.Num() < 5) return 0;

	// 가까울수록 높은 레벨(5)
	if (Dist <= DistanceThresholds[0]) return 5;
	if (Dist <= DistanceThresholds[1]) return 4;
	if (Dist <= DistanceThresholds[2]) return 3;
	if (Dist <= DistanceThresholds[3]) return 2;
	if (Dist <= DistanceThresholds[4]) return 1;
	return 0;
}

float AHorrorMusicController::GetNearestEnemyDistance() const
{
	const APawn* P = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!P) return TNumericLimits<float>::Max();

	float Best = TNumericLimits<float>::Max();
	for (TActorIterator<AEnemyCharacter> It(GetWorld()); It; ++It)
	{
		const AEnemyCharacter* Enemy = *It;
		if (!IsValid(Enemy)) continue;
		const float D = FVector::Dist(P->GetActorLocation(), Enemy->GetActorLocation());
		if (D < Best) Best = D;
	}
	return Best;
}

void AHorrorMusicController::EnsureDefaults()
{
	if (DistanceThresholds.Num() == 0)
	{
		DistanceThresholds = { 400.f, 800.f, 1400.f, 2000.f, 2800.f };
	}
	if (LevelVolumes.Num() == 0)
	{
		LevelVolumes = { 1.0f, 0.8f, 0.6f, 0.4f, 0.2f }; // 5->1
	}
}