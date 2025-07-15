// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "CPlayerCharacter.generated.h"


class UCPlayerAttributeSet;
class UCInputComponent;
class UCInputConfig;
class USkeletalMeshComponent;
class UCDashComponent;
struct FInputActionValue;
class UAbilitySystemComponent;

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
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UCInputConfig* InputConfig;
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
	void BeginDash();

	
	
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

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities", meta = (AllowPrivateAccess = "true"))
	class UCDashComponent* DashComponent;


	//-----------------Inventory----------------------------

	// 클래스 내에 추가
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	class UCInventoryComponent* InventoryComponent;

	void Input_NextItem(const FInputActionValue& InputActionValue);
	void Input_PrevItem(const FInputActionValue& InputActionValue);
	void Input_SelectSlot(const FInputActionValue& InputActionValue);

private:
	// 핫바 위젯 관련
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UCHotbarWidget> HotbarWidgetClass;

	UPROPERTY()
	UCHotbarWidget* HotbarWidget;

public:
	// 인벤토리 컴포넌트 게터 함수
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	UCInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }
	
	// --------------GAS(Health, Mana)----------------
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	UAbilitySystemComponent* AbilitySystemComponent;
	
	UPROPERTY()
	UCPlayerAttributeSet* AttributeSet;
};
