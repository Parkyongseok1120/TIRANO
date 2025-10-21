
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
class ACFlashlightItem;
class UCBatteryHUDWidget;
class USphereComponent;
class UUserWidget;          
class ACDoorActor;
class UProximityFootstepComponent;

struct FInputActionValue;

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

	// 문 근처 오버랩 진입/이탈에서 호출
	void SetNearbyDoor(ACDoorActor* Door, bool bEnter);

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
	float ThrowImpulseStrength = 5500.f;

	UPROPERTY(EditAnywhere, Category = "Inventory|Throw", meta=(ClampMin="0.0", ClampMax="89.0"))
	float ThrowPitchOffsetDeg = 10.0f;
	
	UPROPERTY(EditAnywhere, Category = "Inventory|Throw", meta=(ClampMin="0.0"))
	float ThrowStartZOffset = 10.0f;

	UPROPERTY(EditAnywhere, Category = "Inventory|Equip")
	bool bAutoEquipOnSlotChange = true;

	void EquipSelectedItem();
	void UnequipCurrentItem();
	void ThrowCurrentItem();
	void GetAimInfo(FVector& OutStart, FVector& OutDir) const;

	UFUNCTION()
	void OnSelectedSlotChanged(int32 NewIndex);
	UFUNCTION()
	void OnInventoryUpdated();

	// Held 아이템이 파괴되었을 때 포인터 정리
	UFUNCTION()
	void OnHeldActorDestroyed(AActor* DestroyedActor);

private:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UCHotbarWidget> HotbarWidgetClass;

	UPROPERTY()
	UCHotbarWidget* HotbarWidget;

	// ===== Door prompt UI =====
	UPROPERTY(EditDefaultsOnly, Category="UI|Door")
	TSubclassOf<UUserWidget> DoorPromptWidgetClass;

	UPROPERTY()
	UUserWidget* DoorPromptWidget = nullptr;

	// 현재 근처 문
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Door", meta=(AllowPrivateAccess="true"))
	TWeakObjectPtr<ACDoorActor> NearbyDoor;

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

private:
	// 발소리 노이즈
	UPROPERTY(EditAnywhere, Category="Noise|Footstep")
	float FootstepIntervalWalk = 0.6f;

	UPROPERTY(EditAnywhere, Category="Noise|Footstep")
	float FootstepIntervalSprint = 0.35f;

	UPROPERTY(EditAnywhere, Category="Noise|Footstep")
	float FootstepLoudnessWalk = 0.7f;

	UPROPERTY(EditAnywhere, Category="Noise|Footstep")
	float FootstepLoudnessSprint = 1.2f;

	UPROPERTY(EditAnywhere, Category="Noise|Footstep")
	float FootstepMaxRange = 1800.f;

	float FootstepTimer = 0.f;

	// 배터리 아이템 ID
	UPROPERTY(EditAnywhere, Category="Item|Flashlight")
	FName BatteryItemId = "Item_FlashlightBattery";

	UPROPERTY(EditAnywhere, Category="Item|Flashlight")
	FName UsedBatteryItemId = "Item_FlashlightBattery_Used";

	// HUD: 배터리/교체 프롬프트
	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<UCBatteryHUDWidget> BatteryWidgetClass;

	UPROPERTY()
	UCBatteryHUDWidget* BatteryWidget = nullptr;

	// UI 업데이트 헬퍼
	void UpdateBatteryUI();
	void UpdateBatteryReplacePrompt();
	
	// 입력
	void OnFPressed();

	void EmitFootstepIfNeeded(float DeltaSeconds);

	// 헬퍼: 현재 손에 든 손전등 얻기
	ACFlashlightItem* GetHeldFlashlight() const;

	// ===== Enemy KillRange 전용 오버랩 스피어 =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Collision|KillOverlap", meta=(AllowPrivateAccess="true"))
	USphereComponent* KillOverlapSphere = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Collision|KillOverlap", meta=(ClampMin="10.0", AllowPrivateAccess="true"))
	float KillOverlapRadius = 45.f; // 캡슐 반경(보통 42)보다 약간 크게

protected:
	UPROPERTY(EditAnywhere, Category="UI|Sound")
	UProximityFootstepComponent* FootstepComponent;
};
