#include "00_Character/01_Monster/EnemyCharacter.h"
#include "00_Character/01_Monster/EnemyAIController.h"
#include "NavigationSystem.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "Kismet/GameplayStatics.h"
#include "00_Character/CPlayerCharacter.h"
#include "Navigation/PathFollowingComponent.h"


AEnemyCharacter::AEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	KillRange = CreateDefaultSubobject<USphereComponent>(TEXT("KillRange"));
	KillRange->SetupAttachment(GetRootComponent());
	KillRange->SetSphereRadius(120.f);
	KillRange->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	KillRange->OnComponentBeginOverlap.AddDynamic(this, &AEnemyCharacter::OnKillRangeBeginOverlap);

	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	CachedAI = Cast<AEnemyAIController>(GetController());
	SetState(EEnemyState::Idle);
	StartRandomPatrol();
}

void AEnemyCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	switch (State)
	{
	case EEnemyState::Idle:       TickIdle(DeltaSeconds); break;
	case EEnemyState::Suspicious: TickSuspicious(DeltaSeconds); break;
	case EEnemyState::Chasing:    TickChasing(DeltaSeconds); break;
	case EEnemyState::Stunned:    TickStunned(DeltaSeconds); break;
	default: break;
	}
}


void AEnemyCharacter::TickStunned(float /*DeltaSeconds*/)
{
	// 스턴 상태에서의 프레임별 처리(필요 시 확장)
	// 현재는 스턴 해제는 타이머로 관리됨(EnterStunned에서 설정)
}

void AEnemyCharacter::HandlePerception(AActor* Actor, const FAIStimulus& Stimulus)
{
	if (!Actor) return;

	if (Stimulus.Type == UAISense::GetSenseID(UAISense_Sight::StaticClass()))
	{
		// Player visible -> chase
		if (ACPlayerCharacter* Player = Cast<ACPlayerCharacter>(Actor))
		{
			EnterChase(Player);
		}
	}
	else if (Stimulus.Type == UAISense::GetSenseID(UAISense_Hearing::StaticClass()))
	{
		// 소리듣는 기능 -> 이미 추격하지 않는 한 의심스럽게 그 위치로 이동합니다
		if (State != EEnemyState::Chasing && State != EEnemyState::Executing)
		{
			EnterSuspicious(Stimulus.StimulusLocation);
		}
	}
}

void AEnemyCharacter::NotifyShinedByFlashlight(float Intensity, const FVector& /*FromLocation*/)
{
	// 기절한 상태로 진입하고 이미 기절한 상태인 경우 시간 연장
	const float Duration = StunBaseDuration + FMath::Clamp(Intensity * 0.5f, 0.f, 1.f);
	EnterStunned(Duration);
}

void AEnemyCharacter::SetState(EEnemyState NewState)
{
	if (State == NewState) return;
	State = NewState;
	UpdateSpeedForState();
}

void AEnemyCharacter::UpdateSpeedForState()
{
	switch (State)
	{
	case EEnemyState::Idle:
	case EEnemyState::Suspicious:
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
		break;
	case EEnemyState::Chasing:
		GetCharacterMovement()->MaxWalkSpeed = ChaseSpeed;
		break;
	case EEnemyState::Stunned:
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed * StunMoveSlowMultiplier;
		break;
	default: break;
	}
}

void AEnemyCharacter::TickIdle(float /*DeltaSeconds*/)
{
	// 현재 이동 목표에 도달하면 다음 선택.
	if (CachedAI)
	{
	// 컨트롤러가 MoveTo를 처리합니다. 주기적으로 새로운 순찰 지점을 설정할 수 있습니다
	}
}

void AEnemyCharacter::StartRandomPatrol()
{
	if (!CachedAI) CachedAI = Cast<AEnemyAIController>(GetController());
	if (!CachedAI) return;

	const UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSys) return;

	FNavLocation RandomPt;
	const bool bFound = NavSys->GetRandomReachablePointInRadius(GetActorLocation(), PatrolRadius, RandomPt);
	if (bFound)
	{
		MoveToLocation(RandomPt.Location);
	}
}

void AEnemyCharacter::MoveToLocation(const FVector& Dest)
{
	if (!CachedAI) return;
	FAIMoveRequest Req;
	Req.SetGoalLocation(Dest);
	Req.SetAcceptanceRadius(100.f);
	CachedAI->MoveTo(Req);
}

void AEnemyCharacter::MoveToActor(AActor* TargetActor)
{
	if (!CachedAI || !TargetActor) return;
	FAIMoveRequest Req;
	Req.SetGoalActor(TargetActor);
	Req.SetAcceptanceRadius(120.f);
	CachedAI->MoveTo(Req);
}

void AEnemyCharacter::EnterSuspicious(const FVector& NoiseLocation)
{
	SuspiciousPoint = NoiseLocation;
	SetState(EEnemyState::Suspicious);
	MoveToLocation(SuspiciousPoint);

	// 해당 위치 근처에 플레이어가 보이지 않으면 타임아웃 후 유휴 상태로 돌아갑니다
    GetWorldTimerManager().ClearTimer(SuspiciousTimerHandle);
	GetWorldTimerManager().SetTimer(SuspiciousTimerHandle, [this]()
	{
		if (State == EEnemyState::Suspicious)
		{
			ExitSuspicious(true);
		}
	}, SuspiciousTimeout, false);
}

void AEnemyCharacter::ExitSuspicious(bool bToIdle)
{
	GetWorldTimerManager().ClearTimer(SuspiciousTimerHandle);
	if (bToIdle)
	{
		SetState(EEnemyState::Idle);
		StartRandomPatrol();
	}
}

bool AEnemyCharacter::bReachedSuspiciousPoint() const
{
	return FVector::Dist2D(GetActorLocation(), SuspiciousPoint) <= SuspiciousAcceptanceRadius;
}

void AEnemyCharacter::TickSuspicious(float /*DeltaSeconds*/)
{
	if (bReachedSuspiciousPoint())
	{
		// 타이머를 통해 대기함.(EnterSuspicious에서 처리)
	}
}

void AEnemyCharacter::EnterChase(AActor* Target)
{
	if (!Target) return;
	ChaseTarget = Target;
	SetState(EEnemyState::Chasing);
	MoveToActor(Target);
}

void AEnemyCharacter::ExitChase(bool bToIdle)
{
	ChaseTarget = nullptr;
	if (bToIdle)
	{
		SetState(EEnemyState::Idle);
		StartRandomPatrol();
	}
}

void AEnemyCharacter::TickChasing(float /*DeltaSeconds*/)
{
	if (ChaseTarget)
	{
		MoveToActor(ChaseTarget);

		// 시력을 잃고 마지막으로 알려진 위치에서 멀리 떨어진 경우 목표 마지막 위치 주변에서 의심스러운 위치로 다운그레이드합니다
		if (!HasLineOfSightToActor(ChaseTarget))
		{
			EnterSuspicious(ChaseTarget->GetActorLocation());
		}
	}
}

void AEnemyCharacter::EnterStunned(float Duration)
{
	if (State == EEnemyState::Executing) return;

	SetState(EEnemyState::Stunned);
	// 플레이어 애니메이션 몽타주 진입 위치

	// 시간 경과 후 아무 일도 일어나지 않으면 대기 상태로 전환합니다
	FTimerHandle Tmp;
	GetWorldTimerManager().SetTimer(Tmp, [this]()
	{
		if (State == EEnemyState::Stunned)
		{
			ExitStunned();
		}
	}, Duration, false);
}

void AEnemyCharacter::ExitStunned()
{
	// 재개: 플레이어가 보이면 추격하고, 그렇지 않으면 유휴 상태입니다
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (PlayerPawn && HasLineOfSightToActor(PlayerPawn))
	{
		EnterChase(PlayerPawn);
	}
	else
	{
		SetState(EEnemyState::Idle);
		StartRandomPatrol();
	}
}

bool AEnemyCharacter::HasLineOfSightToActor(AActor* Other) const
{
	if (!Other) return false;
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(Other);
	const FVector Start = GetActorLocation() + FVector(0,0,50);
	const FVector End = Other->GetActorLocation() + FVector(0,0,50);
	const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);
	return !bHit;
}

void AEnemyCharacter::OnKillRangeBeginOverlap(UPrimitiveComponent* /*Overlapped*/, AActor* Other, UPrimitiveComponent* /*OtherComp*/, int32 /*BodyIndex*/, bool /*bFromSweep*/, const FHitResult& /*Hit*/)
{
	if (State == EEnemyState::Stunned || State == EEnemyState::Executing) return;

	if (ACPlayerCharacter* Player = Cast<ACPlayerCharacter>(Other))
	{
		TryExecutePlayer(Player);
	}
}

void AEnemyCharacter::TryExecutePlayer(AActor* PlayerActor)
{
	SetState(EEnemyState::Executing);

	// TODO: Play execution animation montage here
	// 짧은 지연 후 게임 종료 알림
	FTimerHandle Tmp;
	GetWorldTimerManager().SetTimer(Tmp, [this, PlayerActor]()
	{
		// 게임 오버 트리거; 입력 비활성화, UI 표시 등.
		if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
		{
			PC->SetPause(true);
		}
	}, 1.2f, false);
}