// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CPlayerCharacter.generated.h"


class UCInputComponent;
class UCInputConfig;
class USkeletalMeshComponent;

struct FInputActionValue;


UCLASS()
class TIRANO_API ACPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACPlayerCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	
	void BeginZoom();
	void EndZoom();

private:
	void Input_Move(const FInputActionValue& InputActionValue);
	void Input_Look(const FInputActionValue& InputActionValue);
	void BeginSprint();
	void EndSprint();
	void OnWalk();
	void Jump();
	bool IsGrounded()const;

	
	
	//-----------------Camera------------------------------

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* SpringArm;

	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* PlayerCamera;

	UPROPERTY()
	bool bWantsToZoom;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	float ZoomedFOV = 40.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Player", meta = (ClampMin = 0.1, ClampMax = 100))
	float ZoomInterpSpeed = 20.0f;

	//Default camera status value at game startup
	float DefaultFOV;

public:
	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;


	//-----------------Movement----------------------------
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MovementSpeed")
	float SprintingSpeed = 650.0f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MovementSpeed")
	float WalkingSpeed = 250.0f;

private:
	UPROPERTY(VisibleAnywhere)
	bool bisSprint;
	
	UPROPERTY(EditAnywhere, Category = "Jump")
	float JumpForce = 500.0f;

	UPROPERTY(EditAnywhere, Category = "Jump")
	int32 MaxJumpCount = 2;

	int32 CurrentJumpCount = 0;
	
	bool bCanDoubleJump;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UCInputConfig* InputConfig;


	
};
