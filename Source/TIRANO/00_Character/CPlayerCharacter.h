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
class UAbilitySystemComponent;
class UCInventoryComponent;
class UCStatusUI;

struct FInputActionValue;

// 스태미나 변경 델리게이트(현재값, 최대값)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStaminaChanged, float, Current, float, Max);

UCLASS()
class TIRANO_API ACPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ACPlayerCharacter();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

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

	float DefaultFOV;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

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

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stamina", meta = (AllowPrivateAccess = "true", ClampMin = "1.0"))
	float MaxStamina = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stamina", meta = (AllowPrivateAccess = "true"))
	float CurrentStamina = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stamina", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float SprintDrainPerSecond = 25.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stamina", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float WalkRegenPerSecond = 15.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stamina", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float RegenDelay = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stamina", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float ExhaustedRecoverThreshold = 15.f;

	float TimeSinceLastStaminaUse = 0.f;
	bool bStaminaExhausted = false;

	void UpdateStamina(float DeltaTime);
	void MarkStaminaUsed();
	void SetCurrentStamina(float NewValue);

public:
	UFUNCTION(BlueprintPure, Category = "Stamina")
	float GetStamina() const { return CurrentStamina; }

	UFUNCTION(BlueprintPure, Category = "Stamina")
	float GetMaxStamina() const { return MaxStamina; }

	UFUNCTION(BlueprintPure, Category = "Stamina")
	float GetStaminaRatio() const { return MaxStamina > 0.f ? CurrentStamina / MaxStamina : 0.f; }

	UPROPERTY(BlueprintAssignable, Category = "Stamina")
	FOnStaminaChanged OnStaminaChanged;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	class UCInventoryComponent* InventoryComponent;

	void Input_NextItem(const FInputActionValue& InputActionValue);
	void Input_PrevItem(const FInputActionValue& InputActionValue);
	void Input_SelectSlot(const FInputActionValue& InputActionValue);

	UPROPERTY(Transient)
	AActor* HeldItemActor = nullptr;

	UPROPERTY(EditAnywhere, Category = "Inventory|Equip")
	FName HandSocketName = TEXT("RightHandSocket");

	UPROPERTY(EditAnywhere, Category = "Inventory|Equip")
	FVector HoldOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "Inventory|Equip")
	FRotator HoldRotationOffset = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, Category = "Inventory|Throw")
	float ThrowImpulseStrength = 2500.f;

	UPROPERTY(EditAnywhere, Category = "Inventory|Equip")
	bool bAutoEquipOnSlotChange = true;

	void EquipSelectedItem();
	void UnequipCurrentItem();
	void ThrowCurrentItem();
	void GetAimInfo(FVector& OutStart, FVector& OutDir) const;

	UFUNCTION()
	void OnSelectedSlotChanged(int32 NewIndex);

private:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UCHotbarWidget> HotbarWidgetClass;

	UPROPERTY()
	UCHotbarWidget* HotbarWidget;

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	UCInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	UAbilitySystemComponent* AbilitySystemComponent;
	
	UPROPERTY()
	UCPlayerAttributeSet* AttributeSet;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UCStatusUI> StatusWidgetClass;

	UPROPERTY()
	UCStatusUI* StatusWidget = nullptr;

	// -------------Item------------
	
};