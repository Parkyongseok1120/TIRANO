#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ProximityFootstepComponent.generated.h"

class USoundBase;
class USoundAttenuation;

/**
 * 서로 가까워졌을 때만 발소리를 재생하는 컴포넌트
 * - 이동 속도 기반 주기 재생 또는 애님 노티파이로 수동 호출
 * - 특정 리스너(상대 캐릭터)만 대상 지정 가능
 * - AI Hearing 이벤트(옵션)도 함께 발생 가능
 */
UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TIRANO_API UProximityFootstepComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UProximityFootstepComponent();

	// 애님 노티파이에서 호출용(달리기/걷기 선택)
	UFUNCTION(BlueprintCallable, Category="Footstep")
	void AnimFootstep(bool bRun);

	// 특정 리스너(상대)만 거리 판정에 사용하고 싶을 때 지정
	UFUNCTION(BlueprintCallable, Category="Footstep")
	void SetSpecificListener(AActor* InListener);

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ===== 재생 자산 =====
	UPROPERTY(EditAnywhere, Category="Footstep")
	USoundBase* FootstepWalk = nullptr;

	UPROPERTY(EditAnywhere, Category="Footstep")
	USoundBase* FootstepRun = nullptr;

	UPROPERTY(EditAnywhere, Category="Footstep")
	USoundAttenuation* Attenuation = nullptr;

	UPROPERTY(EditAnywhere, Category="Footstep", meta=(ClampMin="0.0"))
	float VolumeMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, Category="Footstep")
	float PitchMin = 0.97f;

	UPROPERTY(EditAnywhere, Category="Footstep")
	float PitchMax = 1.03f;

	// ===== 로직/튜닝 =====
	// 이동 속도 임계값: 이 속도 이상이면 달리기로 간주
	UPROPERTY(EditAnywhere, Category="Footstep|Logic", meta=(ClampMin="0.0"))
	float RunSpeedThreshold = 320.f;

	// 이동으로 간주할 최소 속도(정지/미세 이동 무시)
	UPROPERTY(EditAnywhere, Category="Footstep|Logic", meta=(ClampMin="0.0"))
	float MinMoveSpeed = 10.f;

	// 주기(초): 걷기/달리기
	UPROPERTY(EditAnywhere, Category="Footstep|Logic", meta=(ClampMin="0.05"))
	float IntervalWalk = 0.6f;

	UPROPERTY(EditAnywhere, Category="Footstep|Logic", meta=(ClampMin="0.05"))
	float IntervalRun = 0.38f;

	// 리스너와의 청취 거리(이 거리 이내면 재생)
	UPROPERTY(EditAnywhere, Category="Footstep|Logic", meta=(ClampMin="50.0"))
	float HearingRange = 1200.f;

	// 애님 노티파이만 사용할지(틱 기반 주기 재생 비활성)
	UPROPERTY(EditAnywhere, Category="Footstep|Logic")
	bool bUseAnimNotifiesOnly = false;

	// 특정 리스너만 판정할지
	UPROPERTY(EditAnywhere, Category="Footstep|Logic")
	bool bOnlySpecificListener = false;

	// (옵션) AI Hearing 시스템에 노이즈 리포트
	UPROPERTY(EditAnywhere, Category="Footstep|AI Noise")
	bool bReportAIHearing = true;

	UPROPERTY(EditAnywhere, Category="Footstep|AI Noise")
	float NoiseLoudnessWalk = 0.7f;

	UPROPERTY(EditAnywhere, Category="Footstep|AI Noise")
	float NoiseLoudnessRun = 1.2f;

	UPROPERTY(EditAnywhere, Category="Footstep|AI Noise")
	float NoiseMaxRange = 1800.f;

private:
	float TimerAcc = 0.f;
	TWeakObjectPtr<AActor> SpecificListener;

	// 내부: 재생 시도(조건 만족 시 사운드 실행)
	void TryPlayFootstep(bool bRun);

	// 내부: 현 소유자 이동 상태 읽기
	bool GetOwnerMoveInfo(bool& bIsMoving, bool& bIsRunning, FVector& OutLocation) const;

	// 내부: 리스너(한 명 또는 월드의 다른 캐릭터)까지 최소 거리
	float GetMinDistanceToListeners() const;
};