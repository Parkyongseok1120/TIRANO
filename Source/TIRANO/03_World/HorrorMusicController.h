#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HorrorMusicController.generated.h"

class UAudioComponent;
class USoundBase;

UCLASS()
class TIRANO_API AHorrorMusicController : public AActor
{
	GENERATED_BODY()
	
public:	
	AHorrorMusicController();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	// 루프 음악 컴포넌트(2D BGM 추천: Spatialize 비활성)
	UPROPERTY(VisibleAnywhere, Category="Music")
	UAudioComponent* MusicComp;

	// 루프 음악
	UPROPERTY(EditAnywhere, Category="Music")
	USoundBase* MusicLoop = nullptr;

	// 단계별 거리 임계값(가까운 순서). 예: [400, 800, 1400, 2000, 2800]
	// 거리 <= Thresholds[0] -> Level 5, <= Thresholds[1] -> Level 4, ... > 마지막 -> Level 0
	UPROPERTY(EditAnywhere, Category="Music|Tuning")
	TArray<float> DistanceThresholds;

	// 단계별 볼륨(1~5). Level 0은 0으로 간주.
	UPROPERTY(EditAnywhere, Category="Music|Tuning")
	TArray<float> LevelVolumes;

	// 볼륨 페이드 속도(초당 변화량)
	UPROPERTY(EditAnywhere, Category="Music|Tuning", meta=(ClampMin="0.1"))
	float FadeSpeed = 2.5f;

	// 업데이트 간격(틱 스로틀링)
	UPROPERTY(EditAnywhere, Category="Music|Perf", meta=(ClampMin="0.01"))
	float UpdateInterval = 0.1f;

private:
	float TimeAcc = 0.f;
	float CurrentVolume = 0.f;

	// 현재 단계 계산
	int32 ComputeLevel() const;

	// 플레이어-가장 가까운 몬스터 거리
	float GetNearestEnemyDistance() const;

	// 내부 헬퍼
	void EnsureDefaults();
};