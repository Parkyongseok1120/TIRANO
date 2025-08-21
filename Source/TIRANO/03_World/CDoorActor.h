#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CDoorActor.generated.h"

class USphereComponent;
class UTextRenderComponent;
class UCurveFloat;
class UAudioComponent;

UCLASS()
class TIRANO_API ACDoorActor : public AActor
{
	GENERATED_BODY()

public:
	ACDoorActor();
	virtual void Tick(float DeltaSeconds) override;

	// 플레이어가 상호작용할 때 호출(플레이어의 F키 우선 처리에서 사용)
	UFUNCTION(BlueprintCallable, Category="Door")
	void ToggleDoor();

	// 현재 열림 상태 조회
	UFUNCTION(BlueprintPure, Category="Door")
	bool IsOpen() const { return bOpen; }

protected:
	virtual void BeginPlay() override;

	// ===== Components =====
	UPROPERTY(VisibleAnywhere, Category="Door")
	USceneComponent* HingeRoot;

	UPROPERTY(VisibleAnywhere, Category="Door")
	UStaticMeshComponent* DoorMesh;

	UPROPERTY(VisibleAnywhere, Category="Door|Interact")
	USphereComponent* InteractSphere;

	UPROPERTY(VisibleAnywhere, Category="Door|Interact")
	UTextRenderComponent* PromptText;

	// ===== Config =====
	UPROPERTY(EditAnywhere, Category="Door|Config", meta=(ClampMin="0.0", ClampMax="180.0"))
	float OpenAngle = 90.f;

	UPROPERTY(EditAnywhere, Category="Door|Config")
	bool bOpenClockwise = true;

	UPROPERTY(EditAnywhere, Category="Door|Animation", meta=(ClampMin="0.05"))
	float OpenTime = 0.7f;

	UPROPERTY(EditAnywhere, Category="Door|Animation")
	UCurveFloat* OpenCurve = nullptr;

	UPROPERTY(EditAnywhere, Category="Door")
	bool bOpen = false;

	UPROPERTY(EditAnywhere, Category="Door|Noise")
	float OpenNoiseLoudness = 1.0f;

	UPROPERTY(EditAnywhere, Category="Door|Noise")
	float OpenNoiseRange = 1800.f;

	UPROPERTY(EditAnywhere, Category="Door|Sound")
	USoundBase* OpenSound = nullptr;

	UPROPERTY(EditAnywhere, Category="Door|Sound")
	USoundBase* CloseSound = nullptr;

private:
	// 타임라인 대체용 내부 상태
	bool bAnimating = false;
	float AnimElapsed = 0.f;
	FRotator AnimStartRot;
	FRotator AnimTargetRot;

	// 기준선: 에디터 배치 시 HingeRoot 회전(닫힌 상태의 기준)
	UPROPERTY()
	FRotator InitialHingeRot = FRotator::ZeroRotator;

	// 내부: 애니메이션 업데이트
	void UpdateDoorAnimation(float DeltaSeconds);

	// 내부: 애니메이션 시작 설정
	void StartDoorAnimation(bool bOpenTarget);

	// 내부: 사운드/효과
	void PlayDoorAnimAndSound(bool bOpening);

	// 오버랩 핸들러
	UFUNCTION()
	void OnInteractBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	                     UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep);

	UFUNCTION()
	void OnInteractEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	                   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};