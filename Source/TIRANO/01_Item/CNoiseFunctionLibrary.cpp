#include "01_Item/CNoiseFunctionLibrary.h"
#include "Perception/AISense_Hearing.h"

void UCNoiseFunctionLibrary::ReportNoise(UObject* WorldContext, const FVector& Location, float Loudness, AActor* Instigator, float MaxRange)
{
	if (!WorldContext) return;
	UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContext);
	UAISense_Hearing::ReportNoiseEvent(World, Location, Loudness, Instigator, MaxRange, TEXT("GenericNoise"));
}