#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Perception/AIPerceptionTypes.h"
#include "EnemyCharacter.generated.h"

class UNavigationSystemV1;
class AEnemyAIController;

UENUM(BlueprintType)
enum class EEnemyState : uint8
{
	Idle UMETA(DisplayName="Idle"),
	Suspicious UMETA(DisplayName="Suspicious"),
	Chasing UMETA(DisplayName="Chasing"),
	Stunned UMETA(DisplayName="Stunned"),
	Executing UMETA(DisplayName="Executing")
};

UCLASS()
class TIRANO_API AEnemyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AEnemyCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

public:
	// Perception hook from controller
	void HandlePerception(AActor* Actor, const FAIStimulus& Stimulus);

	// Flashlight stun API
	void NotifyShinedByFlashlight(float Intensity, const FVector& FromLocation);

protected:
	// State machine
	void SetState(EEnemyState NewState);
	void TickIdle(float DeltaSeconds);
	void TickSuspicious(float DeltaSeconds);
	void TickChasing(float DeltaSeconds);
	void TickStunned(float DeltaSeconds);

	// Movement helpers
	void StartRandomPatrol();
	void MoveToLocation(const FVector& Dest);
	void MoveToActor(AActor* TargetActor);

	// Patrol helpers
	bool ChooseNewPatrolGoal(FVector& OutGoal) const;
	bool HasReachedLocation2D(const FVector& Loc, float Tolerance) const;

	// Suspicious
	void EnterSuspicious(const FVector& NoiseLocation);
	void ExitSuspicious(bool bToIdle);
	bool bReachedSuspiciousPoint() const;

	// Chasing
	void EnterChase(AActor* Target);
	void ExitChase(bool bToIdle);

	// Stunned
	void EnterStunned(float Duration);
	void ExitStunned();

	// Kill/Execute
	UFUNCTION()
	void OnKillRangeBeginOverlap(UPrimitiveComponent* Overlapped, AActor* Other, UPrimitiveComponent* OtherComp, int32 BodyIndex, bool bFromSweep, const FHitResult& Hit);
	void TryExecutePlayer(AActor* PlayerActor);

	// KillRange 충돌 재설정(에디터 프로필 꼬임 대비)
	void SetupKillRangeCollision();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AI")
	EEnemyState State = EEnemyState::Idle;

	// Movement
	UPROPERTY(EditAnywhere, Category="AI|Movement")
	float WalkSpeed = 220.f;

	UPROPERTY(EditAnywhere, Category="AI|Movement")
	float ChaseSpeed = 420.f;

	// Idle patrol
	UPROPERTY(EditAnywhere, Category="AI|Idle", meta=(ClampMin="100.0"))
	float PatrolRadius = 1000.f;

	// 다음 목적지는 최소 이 거리 이상 떨어져야 함(너무 가까운 곳 제외)
	UPROPERTY(EditAnywhere, Category="AI|Idle", meta=(ClampMin="100.0"))
	float MinPatrolDistance = 600.f;

	// 도달 판정 허용 반경
	UPROPERTY(EditAnywhere, Category="AI|Idle", meta=(ClampMin="50.0"))
	float PatrolAcceptanceRadius = 120.f;

	// 현재 순찰 목표
	UPROPERTY(VisibleAnywhere, Category="AI|Idle")
	FVector CurrentPatrolGoal = FVector::ZeroVector;

	// Suspicious
	UPROPERTY(EditAnywhere, Category="AI|Suspicious")
	float SuspiciousAcceptanceRadius = 150.f;

	UPROPERTY(EditAnywhere, Category="AI|Suspicious")
	float SuspiciousTimeout = 4.0f;

	// Stun
	UPROPERTY(EditAnywhere, Category="AI|Stun", meta=(ClampMin="0.1"))
	float StunMoveSlowMultiplier = 0.3f;

	UPROPERTY(EditAnywhere, Category="AI|Stun", meta=(ClampMin="0.0"))
	float StunBaseDuration = 1.0f;

	// 라이트 끈 뒤에도 유지할 최소 스턴 잔여 시간
	UPROPERTY(EditAnywhere, Category="AI|Stun", meta=(ClampMin="0.0"))
	float PostLightOffHoldSeconds = 1.5f;

	// 스턴 해제 후 바로 추격 금지: 의심으로 전환(LastStunSource 사용). 없으면 Idle
	UPROPERTY(EditAnywhere, Category="AI|Stun")
	bool bPostStunGoSuspicious = true;

	// 마지막 스턴 광원 위치
	UPROPERTY(VisibleAnywhere, Category="AI|Stun")
	FVector LastStunSource = FVector::ZeroVector;

	FTimerHandle SuspiciousTimerHandle;
	FTimerHandle StunExitTimerHandle;

	FVector SuspiciousPoint = FVector::ZeroVector;

	UPROPERTY()
	AActor* ChaseTarget = nullptr;

	// Kill range trigger
	UPROPERTY(VisibleAnywhere, Category="AI|Combat")
	class USphereComponent* KillRange;

	// 거리기반 킬 체크(페일세이프)
	UPROPERTY(EditAnywhere, Category="AI|Combat")
	bool bUseDistanceKillFallback = true;

	// Cached controller
	UPROPERTY()
	AEnemyAIController* CachedAI = nullptr;

	// Helpers
	void UpdateSpeedForState();

	// Utility: Has LOS to actor?
	bool HasLineOfSightToActor(AActor* Other) const;
};