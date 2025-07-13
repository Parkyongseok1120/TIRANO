// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "CAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class TIRANO_API UCAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation")
	float C_Speed;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation")
	float C_Direction;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation")
	float C_Pitch;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation")
	float C_Yaw;
	UPROPERTY(BlueprintReadonly)
	class UCharacterMovementComponent* MovementComponent;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Animation")
	bool bFalling;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Animation")
	bool bisBackward = false;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Animation")
	bool bisEqiupWepaon = false;

	bool bEquipFucCall = false;
	
	FRotator PrevRotation; // 이전 회전값.

public:
	void NativeBeginPlay() override;
	void NativeUpdateAnimation(float DeltaSeconds) override;
	//void EquipWeapon(bool B_EW);

private:
	class ACPlayerCharacter* PlayerCharacter;
	
	
};
