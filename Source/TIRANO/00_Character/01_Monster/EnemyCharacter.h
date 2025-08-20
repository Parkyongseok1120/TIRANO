#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Perception/AIPerceptionTypes.h"
#include "EnemyCharacter.generated.h"

class UNavigationSystemV1;
class AEnemyAIController;
class UCapsuleComponent;

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

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AI")
	EEnemyState State = EEnemyState::Idle;

	UPROPERTY(EditAnywhere, Category="AI|Movement")
	float WalkSpeed = 220.f;

	UPROPERTY(EditAnywhere, Category="AI|Movement")
	float ChaseSpeed = 420.f;

	UPROPERTY(EditAnywhere, Category="AI|Idle")
	float PatrolRadius = 1000.f;

	UPROPERTY(EditAnywhere, Category="AI|Suspicious")
	float SuspiciousAcceptanceRadius = 150.f;

	UPROPERTY(EditAnywhere, Category="AI|Suspicious")
	float SuspiciousTimeout = 4.0f;

	UPROPERTY(EditAnywhere, Category="AI|Stun")
	float StunMoveSlowMultiplier = 0.3f;

	UPROPERTY(EditAnywhere, Category="AI|Stun")
	float StunBaseDuration = 1.0f;

	FTimerHandle SuspiciousTimerHandle;
	FVector SuspiciousPoint = FVector::ZeroVector;

	UPROPERTY()
	AActor* ChaseTarget = nullptr;

	// Kill range
	UPROPERTY(VisibleAnywhere, Category="AI|Combat")
	class USphereComponent* KillRange;

	// Cached controller
	UPROPERTY()
	AEnemyAIController* CachedAI = nullptr;

	// Helpers
	void UpdateSpeedForState();

	// Utility: Has LOS to player?
	bool HasLineOfSightToActor(AActor* Other) const;
};