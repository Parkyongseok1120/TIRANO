// Fill out your copyright notice in the Description page of Project Settings.


#include "99_Util/CGameModeBase.h"
#include "00_Character/CPlayerCharacter.h"

ACGameModeBase::ACGameModeBase()
{
    PlayerCharacterClass = nullptr;
}

void ACGameModeBase::BeginPlay()
{
    Super::BeginPlay();
}

void ACGameModeBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
}
