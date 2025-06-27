// Copyright Epic Games, Inc. All Rights Reserved.

#include "TIRANOGameMode.h"
#include "TIRANOCharacter.h"
#include "UObject/ConstructorHelpers.h"

ATIRANOGameMode::ATIRANOGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
