#include "ProximityFootstepComponent.h"
#include "Sound/SoundBase.h"
#include "Sound/SoundAttenuation.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EngineUtils.h"
#include "Perception/AISense_Hearing.h"

UProximityFootstepComponent::UProximityFootstepComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UProximityFootstepComponent::BeginPlay()
{
	Super::BeginPlay();
	TimerAcc = 0.f;
}

void UProximityFootstepComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bUseAnimNotifiesOnly)
		return;

	bool bMoving = false, bRunning = false;
	FVector Loc;
	if (!GetOwnerMoveInfo(bMoving, bRunning, Loc) || !bMoving)
	{
		TimerAcc = 0.f;
		return;
	}

	TimerAcc += DeltaTime;
	const float Interval = bRunning ? IntervalRun : IntervalWalk;
	if (TimerAcc >= Interval)
	{
		TimerAcc = 0.f;
		TryPlayFootstep(bRunning);
	}
}

void UProximityFootstepComponent::AnimFootstep(bool bRun)
{
	TryPlayFootstep(bRun);
}

void UProximityFootstepComponent::SetSpecificListener(AActor* InListener)
{
	SpecificListener = InListener;
	bOnlySpecificListener = (InListener != nullptr);
}

bool UProximityFootstepComponent::GetOwnerMoveInfo(bool& bIsMoving, bool& bIsRunning, FVector& OutLocation) const
{
	bIsMoving = false;
	bIsRunning = false;
	const AActor* Owner = GetOwner();
	if (!Owner) return false;

	OutLocation = Owner->GetActorLocation();

	const ACharacter* Char = Cast<ACharacter>(Owner);
	if (Char && Char->GetCharacterMovement())
	{
		const float Speed = Char->GetVelocity().Size2D();
		bIsMoving = (Speed > MinMoveSpeed);
		bIsRunning = (Speed >= RunSpeedThreshold);
		return true;
	}

	// 일반 액터인 경우도 지원
	const float Speed = Owner->GetVelocity().Size();
	bIsMoving = (Speed > MinMoveSpeed);
	bIsRunning = (Speed >= RunSpeedThreshold);
	return true;
}

float UProximityFootstepComponent::GetMinDistanceToListeners() const
{
	const AActor* Owner = GetOwner();
	if (!Owner) return TNumericLimits<float>::Max();

	// 특정 리스너만 사용
	if (bOnlySpecificListener && SpecificListener.IsValid())
	{
		return FVector::Dist(Owner->GetActorLocation(), SpecificListener->GetActorLocation());
	}

	// 월드의 다른 캐릭터들 중 가장 가까운 거리
	float Best = TNumericLimits<float>::Max();
	for (TActorIterator<ACharacter> It(GetWorld()); It; ++It)
	{
		const ACharacter* Other = *It;
		if (!IsValid(Other) || Other == Owner) continue;
		const float D = FVector::Dist(Owner->GetActorLocation(), Other->GetActorLocation());
		if (D < Best) Best = D;
	}
	return Best;
}

void UProximityFootstepComponent::TryPlayFootstep(bool bRun)
{
	// 리스너와의 거리 체크
	const float MinDist = GetMinDistanceToListeners();
	if (MinDist > HearingRange)
	{
		return; // 멀리 있으면 재생 안 함
	}

	// 사운드 선택
	USoundBase* Snd = bRun ? FootstepRun : FootstepWalk;
	if (!Snd) return;

	const FVector Loc = GetOwner()->GetActorLocation();
	const float Pitch = FMath::FRandRange(PitchMin, PitchMax);

	UGameplayStatics::PlaySoundAtLocation(this, Snd, Loc, VolumeMultiplier, Pitch, 0.f, Attenuation);

	// AI Hearing(optional)
	if (bReportAIHearing)
	{
		const float Loud = bRun ? NoiseLoudnessRun : NoiseLoudnessWalk;
		UAISense_Hearing::ReportNoiseEvent(GetWorld(), Loc, Loud, GetOwner(), NoiseMaxRange, TEXT("Footstep"));
	}
}