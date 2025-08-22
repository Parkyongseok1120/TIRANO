#include "00_Character/01_Monster/EnemyCharacter.h"
#include "00_Character/01_Monster/EnemyAIController.h"
#include "NavigationSystem.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"        // 캡슐 반경 조회
#include "GameFramework/CharacterMovementComponent.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "Kismet/GameplayStatics.h"
#include "00_Character/CPlayerCharacter.h"
#include "00_Character/02_Component/ProximityFootstepComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "Blueprint/UserWidget.h" // 추가
#include "00_Character/02_Component/ProximityFootstepComponent.h"

AEnemyCharacter::AEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	FootstepComponent = CreateDefaultSubobject<UProximityFootstepComponent>(TEXT("FootStepComponent"));

	KillRange = CreateDefaultSubobject<USphereComponent>(TEXT("KillRange"));
	KillRange->SetupAttachment(GetRootComponent());
	KillRange->SetSphereRadius(120.f);

	// Trigger로 명확히 설정: Pawn만 Overlap
	KillRange->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	KillRange->SetCollisionObjectType(ECC_WorldDynamic);
	KillRange->SetCollisionResponseToAllChannels(ECR_Ignore);
	KillRange->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	KillRange->SetGenerateOverlapEvents(true);

	KillRange->OnComponentBeginOverlap.AddDynamic(this, &AEnemyCharacter::OnKillRangeBeginOverlap);

	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	CachedAI = Cast<AEnemyAIController>(GetController());
	SetupKillRangeCollision();

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
}

void AEnemyCharacter::HandlePerception(AActor* Actor, const FAIStimulus& Stimulus)
{
	if (!Actor) return;

	if (Stimulus.Type == UAISense::GetSenseID(UAISense_Sight::StaticClass()))
	{
		// Player visible -> chase (단, 스턴/처형 중에는 제외)
		if (ACPlayerCharacter* Player = Cast<ACPlayerCharacter>(Actor))
		{
			if (State != EEnemyState::Stunned && State != EEnemyState::Executing)
			{
				EnterChase(Player);
			}
		}
	}
	else if (Stimulus.Type == UAISense::GetSenseID(UAISense_Hearing::StaticClass()))
	{
		// 소리 -> (추격/처형 중이 아니면) 의심 위치로 이동
		if (State != EEnemyState::Chasing && State != EEnemyState::Executing)
		{
			EnterSuspicious(Stimulus.StimulusLocation);
		}
	}
}

void AEnemyCharacter::NotifyShinedByFlashlight(float Intensity, const FVector& FromLocation)
{
	// 기절 시간 = 기본 + 강도 기반 가산 + 라이트를 꺼도 유지할 추가 홀드 시간
	LastStunSource = FromLocation;
	const float Duration = StunBaseDuration + FMath::Clamp(Intensity * 0.5f, 0.f, 1.f) + PostLightOffHoldSeconds;
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

bool AEnemyCharacter::ChooseNewPatrolGoal(FVector& OutGoal) const
{
	const UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSys) return false;

	const FVector Origin = GetActorLocation();
	FNavLocation Candidate;
	for (int32 i = 0; i < 20; ++i) // 최대 20회 시도
	{
		if (NavSys->GetRandomReachablePointInRadius(Origin, PatrolRadius, Candidate))
		{
			if (FVector::Dist2D(Origin, Candidate.Location) >= MinPatrolDistance)
			{
				OutGoal = Candidate.Location;
				return true;
			}
		}
	}
	return false;
}

bool AEnemyCharacter::HasReachedLocation2D(const FVector& Loc, float Tolerance) const
{
	return FVector::Dist2D(GetActorLocation(), Loc) <= Tolerance;
}

void AEnemyCharacter::TickIdle(float /*DeltaSeconds*/)
{
	// 목표가 없거나 도달하면 즉시 다음 목적지 선택
	if (CurrentPatrolGoal.IsNearlyZero() || HasReachedLocation2D(CurrentPatrolGoal, PatrolAcceptanceRadius))
	{
		StartRandomPatrol();
		return;
	}

	// 경로 추종 상태가 Idle이면 새 목적지 선택(길 막힘 등 복구)
	if (CachedAI && CachedAI->GetPathFollowingComponent())
	{
		const auto Status = CachedAI->GetPathFollowingComponent()->GetStatus();
		if (Status == EPathFollowingStatus::Idle)
		{
			StartRandomPatrol();
		}
	}
}

void AEnemyCharacter::TickSuspicious(float /*DeltaSeconds*/)
{
	// 의심 지점까지 계속 이동. 도달 여부는 bReachedSuspiciousPoint로 판정
	// 상태 해제(Idle 전환)는 EnterSuspicious에서 건 타이머로 처리
	if (!bReachedSuspiciousPoint())
	{
		MoveToLocation(SuspiciousPoint);
	}
}

void AEnemyCharacter::StartRandomPatrol()
{
	if (!CachedAI) CachedAI = Cast<AEnemyAIController>(GetController());
	if (!CachedAI) return;

	FVector NewGoal;
	if (ChooseNewPatrolGoal(NewGoal))
	{
		CurrentPatrolGoal = NewGoal;
		MoveToLocation(CurrentPatrolGoal);
	}
}

void AEnemyCharacter::MoveToLocation(const FVector& Dest)
{
	if (!CachedAI) return;
	FAIMoveRequest Req;
	Req.SetGoalLocation(Dest);
	Req.SetAcceptanceRadius(PatrolAcceptanceRadius);
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

	// 해당 위치 근처에 플레이어가 보이지 않으면 타임아웃 후 유휴 상태로 복귀
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
		CurrentPatrolGoal = FVector::ZeroVector; // 다음 틱에 새 목표 선택
		StartRandomPatrol();
	}
}

bool AEnemyCharacter::bReachedSuspiciousPoint() const
{
	return FVector::Dist2D(GetActorLocation(), SuspiciousPoint) <= SuspiciousAcceptanceRadius;
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
		CurrentPatrolGoal = FVector::ZeroVector;
		StartRandomPatrol();
	}
}

void AEnemyCharacter::TickChasing(float /*DeltaSeconds*/)
{
	if (ChaseTarget)
	{
		MoveToActor(ChaseTarget);

		// 시야 상실 시 마지막 위치로 의심 전환
		if (!HasLineOfSightToActor(ChaseTarget))
		{
			EnterSuspicious(ChaseTarget->GetActorLocation());
		}

		// [페일세이프] 거리 기반 킬 체크
		if (bUseDistanceKillFallback && State != EEnemyState::Stunned && State != EEnemyState::Executing)
		{
			const float MyKillR = KillRange ? KillRange->GetScaledSphereRadius() : 120.f;

			float PlayerR = 0.f;
			if (const ACharacter* PlayerChar = Cast<ACharacter>(ChaseTarget))
			{
				if (const UCapsuleComponent* Capsule = PlayerChar->GetCapsuleComponent())
				{
					PlayerR = Capsule->GetScaledCapsuleRadius();
				}
			}

			const float Dist2D = FVector::Dist2D(GetActorLocation(), ChaseTarget->GetActorLocation());
			if (Dist2D <= (MyKillR + PlayerR))
			{
				TryExecutePlayer(ChaseTarget);
			}
		}
	}
}

void AEnemyCharacter::EnterStunned(float Duration)
{
	if (State != EEnemyState::Stunned)
	{
		SetState(EEnemyState::Stunned);
	}

	// 스턴 타이머 갱신(연장)
	GetWorldTimerManager().ClearTimer(StunExitTimerHandle);
	GetWorldTimerManager().SetTimer(StunExitTimerHandle, this, &AEnemyCharacter::ExitStunned, Duration, false);
}

void AEnemyCharacter::ExitStunned()
{
	// 스턴 해제 후 즉시 추격 금지: 의심 또는 대기
	if (bPostStunGoSuspicious && !LastStunSource.IsNearlyZero())
	{
		EnterSuspicious(LastStunSource);
	}
	else
	{
		SetState(EEnemyState::Idle);
		CurrentPatrolGoal = FVector::ZeroVector;
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
	if (!IsValid(PlayerActor)) return;

	SetState(EEnemyState::Executing);

	// TODO: 처형 연출/애니메이션
	FTimerHandle Tmp;
	GetWorldTimerManager().SetTimer(Tmp, [this]()
	{
		if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
		{
			bPlayerIsDead=true;
			ShowGameOverUI();

			PC->SetPause(true);
		}
	}, 1.2f, false);
}

void AEnemyCharacter::SetupKillRangeCollision()
{
	if (!KillRange) return;
	KillRange->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	KillRange->SetCollisionObjectType(ECC_WorldDynamic);
	KillRange->SetCollisionResponseToAllChannels(ECR_Ignore);
	KillRange->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	KillRange->SetGenerateOverlapEvents(true);
}

void AEnemyCharacter::ShowGameOverUI()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC) return;

	if (!GameOverWidgetInstance && GameOverWidgetClass)
	{
		GameOverWidgetInstance = CreateWidget<UUserWidget>(PC, GameOverWidgetClass);
		if (GameOverWidgetInstance)
		{
			GameOverWidgetInstance->AddToViewport(100);
		}
	}

	// 마우스/입력 모드: UI Only
	FInputModeUIOnly Mode;
	if (GameOverWidgetInstance)
	{
		Mode.SetWidgetToFocus(GameOverWidgetInstance->TakeWidget());
	}
	PC->SetInputMode(Mode);
	PC->bShowMouseCursor = true;
}