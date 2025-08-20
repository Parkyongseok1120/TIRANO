#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CNoiseFunctionLibrary.generated.h"

UCLASS()
class TIRANO_API UCNoiseFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	// Perception의 Hearing에 노이즈를 보고
	static void ReportNoise(UObject* WorldContext, const FVector& Location, float Loudness, AActor* Instigator, float MaxRange);
};