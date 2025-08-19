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

	//-----------------Stamina (달리기 스테이터스)----------------------------
private:
	// 최대 스태미나
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stamina", meta = (AllowPrivateAccess = "true", ClampMin = "1.0"))
	float MaxStamina = 100.f;

	// 현재 스태미나
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stamina", meta = (AllowPrivateAccess = "true"))
	float CurrentStamina = 100.f;

	// 달릴 때 초당 소모량
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stamina", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float SprintDrainPerSecond = 25.f;

	// 걷거나 정지 시 초당 회복량
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stamina", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float WalkRegenPerSecond = 15.f;

	// 소모 후 회복이 시작되기까지 딜레이(초)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stamina", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float RegenDelay = 0.6f;

	// 바닥난 후 다시 달리기 시작할 수 있는 최소 회복치
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stamina", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float ExhaustedRecoverThreshold = 15.f;

	// 마지막 소모 이후 경과 시간(회복 딜레이 판단용)
	float TimeSinceLastStaminaUse = 0.f;

	// 바닥나서 강제 종료 상태
	bool bStaminaExhausted = false;

	// 프레임마다 스태미나 업데이트
	void UpdateStamina(float DeltaTime);

	// 소모 직후 호출(딜레이 리셋)
	void MarkStaminaUsed();

	// 내부 헬퍼: 스태미나 값을 갱신하고 변경 시 브로드캐스트
	void SetCurrentStamina(float NewValue);

public:
	// 블루프린트/UI 용
	UFUNCTION(BlueprintPure, Category = "Stamina")
	float GetStamina() const { return CurrentStamina; }

	UFUNCTION(BlueprintPure, Category = "Stamina")
	float GetMaxStamina() const { return MaxStamina; }

	UFUNCTION(BlueprintPure, Category = "Stamina")
	float GetStaminaRatio() const { return MaxStamina > 0.f ? CurrentStamina / MaxStamina : 0.f; }

	// 스태미나 변경 이벤트(위젯에서 구독)
	UPROPERTY(BlueprintAssignable, Category = "Stamina")
	FOnStaminaChanged OnStaminaChanged;

	//-----------------Inventory----------------------------
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	class UCInventoryComponent* InventoryComponent;

	void Input_NextItem(const FInputActionValue& InputActionValue);
	void Input_PrevItem(const FInputActionValue& InputActionValue);
	void Input_SelectSlot(const FInputActionValue& InputActionValue);

	// 현재 손에 들고 있는 아이템 액터
	UPROPERTY(Transient)
	AActor* HeldItemActor = nullptr;

	// 손 소켓 이름(스켈레탈 메시 소켓 필요)
	UPROPERTY(EditAnywhere, Category = "Inventory|Equip")
	FName HandSocketName = TEXT("RightHandSocket");

	// 들고 있을 때의 위치/회전 오프셋
	UPROPERTY(EditAnywhere, Category = "Inventory|Equip")
	FVector HoldOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "Inventory|Equip")
	FRotator HoldRotationOffset = FRotator::ZeroRotator;

	// 던지기 힘(임펄스)
	UPROPERTY(EditAnywhere, Category = "Inventory|Throw")
	float ThrowImpulseStrength = 2500.f;

	// 슬롯 변경 시 자동 장착
	UPROPERTY(EditAnywhere, Category = "Inventory|Equip")
	bool bAutoEquipOnSlotChange = true;

	// 장착/해제/던지기 로직
	void EquipSelectedItem();
	void UnequipCurrentItem();
	void ThrowCurrentItem();
	void GetAimInfo(FVector& OutStart, FVector& OutDir) const;

	// 인벤토리 선택 슬롯 변경 이벤트
	UFUNCTION()
	void OnSelectedSlotChanged(int32 NewIndex);

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
	
	// --------------GAS(Health, Mana) & StatusUI----------------
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	UAbilitySystemComponent* AbilitySystemComponent;
	
	UPROPERTY()
	UCPlayerAttributeSet* AttributeSet;

	// 상태 UI
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UCStatusUI> StatusWidgetClass;

	UPROPERTY()
	UCStatusUI* StatusWidget = nullptr;
};