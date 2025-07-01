// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "00_Character/CPlayerCharacter.h"

#include "CGameModeBase.generated.h"

/**
 * 
 */


UCLASS()
class TIRANO_API ACGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
		
protected:
	//UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
	//TSubclassOf<UUserWidget> PlayerWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Player")
	TSubclassOf<class ACPlayerCharacter> PlayerCharacterClass;
    
public:
	ACGameModeBase();
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	
};