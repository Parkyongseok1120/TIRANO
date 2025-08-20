#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CDoorActor.generated.h"

UCLASS()
class TIRANO_API ACDoorActor : public AActor
{
	GENERATED_BODY()
public:
	ACDoorActor();

	UFUNCTION(BlueprintCallable, Category="Door")
	void ToggleDoor();

protected:
	UPROPERTY(VisibleAnywhere, Category="Door")
	UStaticMeshComponent* DoorMesh;

	UPROPERTY(EditAnywhere, Category="Door")
	bool bOpen = false;

	UPROPERTY(EditAnywhere, Category="Door|Noise")
	float OpenNoiseLoudness = 1.0f;

	UPROPERTY(EditAnywhere, Category="Door|Noise")
	float OpenNoiseRange = 1800.f;

	void PlayDoorAnimAndSound(bool bOpening);
};