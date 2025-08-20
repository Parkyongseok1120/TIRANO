// Fill out your copyright notice in the Description page of Project Settings.

#include "00_Character/CPlayerCharacter.h"
#include "00_Character/02_Component/01_Input/CEnhancedInputComponent.h"
#include "00_Character/02_Component/01_Input/CGameplayTags.h"
#include "00_Character/02_Component/CDashComponent.h"
#include "00_Character/02_Component/03_Inventory/CInventoryComponent.h"
#include "02_UI/CHotbarWidget.h"
#include "02_UI/CStatusUI.h"

#include "EnhancedInput/Public/InputAction.h"
#include "GameFramework/Character.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "CGameInstance.h"
#include "00_Character/02_Component/02_ABS/CPlayerAttributeSet.h"

#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

#include "Global.h"
#include "Blueprint/UserWidget.h"

// Sets default values
ACPlayerCharacter::ACPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetRootComponent());

	PlayerCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	PlayerCamera->SetupAttachment(SpringArm);
	DashComponent = CreateDefaultSubobject<UCDashComponent>(TEXT("DashComponent"));
	InventoryComponent = CreateDefaultSubobject<UCInventoryComponent>(TEXT("InventoryComponent"));

	GetMesh()->SetRelativeLocation(FVector(0, 0, -90));
	GetMesh()->SetRelativeRotation(FRotator(0, -90, 0));
	SpringArm->SetRelativeRotation(FRotator(0, 0, 0));
	SpringArm->SetRelativeLocation(FVector(-18, 63, 49));

	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	bWantsToZoom = false;
	bisSprint = false;
	bCanDoubleJump = true;
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AttributeSet = CreateDefaultSubobject<UCPlayerAttributeSet>(TEXT("AttributeSet"));
}

void ACPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	DefaultFOV = PlayerCamera->FieldOfView;
	GetCharacterMovement()->MaxWalkSpeed = WalkingSpeed;
	SpringArm->bEnableCameraLag = true;

	// 스태미나 초기화 + UI에 초기값 브로드캐스트
	CurrentStamina = MaxStamina;
	TimeSinceLastStaminaUse = 0.f;
	bStaminaExhausted = false;
	OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);

	// DashComponent NULL 체크
	if (!DashComponent)
	{
		CLog::Log("DashComponent가 NULL입니다. 확인 필요!");
	}
	
	if (AbilitySystemComponent)
	{
		if (AttributeSet)
		{
			AbilitySystemComponent->InitStats(UCPlayerAttributeSet::StaticClass(), nullptr);
		}
	}

	// 인벤토리 이벤트: 슬롯 변경 시 자동 장착
	if (InventoryComponent)
	{
		InventoryComponent->OnSelectedSlotChanged.AddDynamic(this, &ACPlayerCharacter::OnSelectedSlotChanged);
		if (bAutoEquipOnSlotChange)
		{
			EquipSelectedItem();
		}
	}

	// 핫바 위젯 생성 및 설정
	if (HotbarWidgetClass)
	{
		APlayerController* PC = Cast<APlayerController>(GetController());
		if (PC && PC->IsLocalPlayerController())
		{
			HotbarWidget = CreateWidget<UCHotbarWidget>(PC, HotbarWidgetClass);
			if (HotbarWidget)
			{
				HotbarWidget->AddToViewport(0);
				HotbarWidget->SetupHotbar(InventoryComponent);
				CLog::Log("핫바 위젯 생성 및 표시됨");
			}
		}
	}

	// 상태 위젯 생성 및 캐릭터 연결
	if (StatusWidgetClass)
	{
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			if (PC->IsLocalPlayerController())
			{
				StatusWidget = CreateWidget<UCStatusUI>(PC, StatusWidgetClass);
				if (StatusWidget)
				{
					StatusWidget->AddToViewport(0);
					StatusWidget->SetupWithCharacter(this);
				}
			}
		}
	}
}

// Called every frame
void ACPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateStamina(DeltaTime);
}

// Called to bind functionality to input
void ACPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	UCEnhancedInputComponent* CEnhancedInputComponent = Cast<UCEnhancedInputComponent>(PlayerInputComponent);
	check(CEnhancedInputComponent);

	CEnhancedInputComponent->BindActionByTag(InputConfig, CGameplayTags::InputTag_Move, ETriggerEvent::Triggered, this, &ACPlayerCharacter::Input_Move);
	CEnhancedInputComponent->BindActionByTag(InputConfig, CGameplayTags::InputTag_Look, ETriggerEvent::Triggered, this, &ACPlayerCharacter::Input_Look);
	
	CEnhancedInputComponent->BindActionByTag(InputConfig, CGameplayTags::InputTag_Mouse_Right, ETriggerEvent::Started, this, &ACPlayerCharacter::BeginZoom);
	CEnhancedInputComponent->BindActionByTag(InputConfig, CGameplayTags::InputTag_Mouse_Right, ETriggerEvent::Completed, this, &ACPlayerCharacter::EndZoom);

	// 대시/점프/스프린트
	CEnhancedInputComponent->BindActionByTag(InputConfig, CGameplayTags::InputTag_Dash, ETriggerEvent::Started, this, &ACPlayerCharacter::BeginDash);
	CEnhancedInputComponent->BindActionByTag(InputConfig, CGameplayTags::InputTag_Jump, ETriggerEvent::Started, this, &ACPlayerCharacter::Jump);
	CEnhancedInputComponent->BindActionByTag(InputConfig, CGameplayTags::InputTag_Sprint, ETriggerEvent::Started, this, &ACPlayerCharacter::BeginSprint);
	CEnhancedInputComponent->BindActionByTag(InputConfig, CGameplayTags::InputTag_Sprint, ETriggerEvent::Completed, this, &ACPlayerCharacter::EndSprint);

	// 인벤토리 관련 입력 바인딩
	CEnhancedInputComponent->BindActionByTag(InputConfig, CGameplayTags::InputTag_NextItem, ETriggerEvent::Triggered, this, &ACPlayerCharacter::Input_NextItem);
	CEnhancedInputComponent->BindActionByTag(InputConfig, CGameplayTags::InputTag_PrevItem, ETriggerEvent::Triggered, this, &ACPlayerCharacter::Input_PrevItem);
	CEnhancedInputComponent->BindActionByTag(InputConfig, CGameplayTags::InputTag_SelectSlot, ETriggerEvent::Triggered, this, &ACPlayerCharacter::Input_SelectSlot);

	// 던지기
	CEnhancedInputComponent->BindActionByTag(InputConfig, CGameplayTags::InputTag_Throw, ETriggerEvent::Started, this, &ACPlayerCharacter::ThrowCurrentItem);
}

void ACPlayerCharacter::BeginZoom()
{
	/* Zoom 로직 사용 시 활성화 */
}

void ACPlayerCharacter::EndZoom()
{
	/* Zoom 로직 사용 시 활성화 */
}

void ACPlayerCharacter::Input_Move(const FInputActionValue& InputActionValue)
{
	// input is a Vector2D
	FVector2D MovementVector = InputActionValue.Get<FVector2D>();

	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// 최종 입력 방향 계산
		FVector InputDirection = ForwardDirection * MovementVector.Y + RightDirection * MovementVector.X;
        
		// 대시 컴포넌트에 입력 방향 전달
		if (DashComponent)
		{
			DashComponent->SetInputDirection(InputDirection);
		}

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ACPlayerCharacter::Input_Look(const FInputActionValue& InputActionValue)
{
	FVector2D LookAxisVector = InputActionValue.Get<FVector2D>();
	
	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X / 5);
		AddControllerPitchInput(LookAxisVector.Y / -5);
	}
}

void ACPlayerCharacter::BeginSprint()
{
	// 줌 중이면 스프린트 금지(기존 로직 유지)
	if (bWantsToZoom)
		return;

	// 스태미나 부족/기진 상태면 시작 불가
	if (CurrentStamina < ExhaustedRecoverThreshold || bStaminaExhausted)
	{
		return;
	}

	bisSprint = true;
	GetCharacterMovement()->MaxWalkSpeed = SprintingSpeed;

	// 대시 컴포넌트에 달리기 상태 전달
	if (DashComponent)
	{
		DashComponent->SetOwnerSprinting(true);
	}

	// 소모 딜레이 리셋
	MarkStaminaUsed();
}

void ACPlayerCharacter::EndSprint()
{
	if (!bisSprint)
		return;

	bisSprint = false;
	OnWalk();
    
	if (DashComponent)
	{
		DashComponent->SetOwnerSprinting(false);
	}
}

void ACPlayerCharacter::OnWalk()
{
	GetCharacterMovement()->MaxWalkSpeed = WalkingSpeed;
}

void ACPlayerCharacter::Jump()
{
	/* 더블 점프 로직 사용 시 활성화 */
}

// 대시 시작 함수
void ACPlayerCharacter::BeginDash()
{
	/*if (DashComponent)
	{
		DashComponent->StartDash();
	}*/
}

bool ACPlayerCharacter::IsGrounded() const
{
	FVector Start = GetActorLocation();
	FVector End = Start - FVector(0, 0, 100.0f);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult, Start, End, ECC_Visibility, QueryParams
	);

	DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 1.0f, 0, 1.0f);
	return bHit;
}

void ACPlayerCharacter::Input_NextItem(const FInputActionValue& InputActionValue)
{
	if (InventoryComponent)
	{
		InventoryComponent->NextSlot();
	}
}

void ACPlayerCharacter::Input_PrevItem(const FInputActionValue& InputActionValue)
{
	if (InventoryComponent)
	{
		InventoryComponent->PrevSlot();
	}
}

void ACPlayerCharacter::Input_SelectSlot(const FInputActionValue& InputActionValue)
{
	// 1~0 키를 0~9 인덱스로 변환 (프로젝트 InputAction에서 1~10 값을 넘겨주도록 설정)
	int32 SlotIndex = FMath::FloorToInt(InputActionValue.Get<float>()) - 1;
	if (InventoryComponent && SlotIndex >= 0 && SlotIndex < 10)
	{
		InventoryComponent->SelectSlot(SlotIndex);
	}
}

UAbilitySystemComponent* ACPlayerCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

// -------------------- 인벤토리 장착/던지기 --------------------

void ACPlayerCharacter::OnSelectedSlotChanged(int32 /*NewIndex*/)
{
	if (bAutoEquipOnSlotChange)
	{
		EquipSelectedItem();
	}
}

void ACPlayerCharacter::EquipSelectedItem()
{
	if (!InventoryComponent) return;

	UnequipCurrentItem();

	const FInventoryItem Item = InventoryComponent->GetSelectedItem();
	if (Item.Quantity <= 0 || !Item.bIsEquippable || !Item.ItemClass)
	{
		return;
	}

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.Instigator = this;

	AActor* NewHeld = GetWorld()->SpawnActor<AActor>(Item.ItemClass, FTransform::Identity, Params);
	if (!NewHeld)
	{
		return;
	}

	if (USkeletalMeshComponent* CharMesh = GetMesh())
	{
		NewHeld->AttachToComponent(CharMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, HandSocketName);
		NewHeld->AddActorLocalOffset(HoldOffset);
		NewHeld->AddActorLocalRotation(HoldRotationOffset);

		if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(NewHeld->GetRootComponent()))
		{
			Prim->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Prim->SetSimulatePhysics(false);
		}

		HeldItemActor = NewHeld;
	}
	else
	{
		NewHeld->Destroy();
	}
}

void ACPlayerCharacter::UnequipCurrentItem()
{
	if (HeldItemActor)
	{
		HeldItemActor->Destroy();
		HeldItemActor = nullptr;
	}
}

void ACPlayerCharacter::GetAimInfo(FVector& OutStart, FVector& OutDir) const
{
	FRotator ViewRot;
	FVector ViewLoc;
	GetActorEyesViewPoint(ViewLoc, ViewRot);
	OutStart = ViewLoc;
	OutDir = ViewRot.Vector();

	if (const USkeletalMeshComponent* CharMesh = GetMesh())
	{
		const FVector HandLoc = CharMesh->GetSocketLocation(HandSocketName);
		OutStart = HandLoc + OutDir * 20.f;
	}
}

void ACPlayerCharacter::ThrowCurrentItem()
{
	if (!InventoryComponent || !HeldItemActor)
	{
		return;
	}

	const int32 SlotIndex = InventoryComponent->GetSelectedSlotIndex();
	FInventoryItem Item = InventoryComponent->GetSelectedItem();
	if (Item.Quantity <= 0 || !Item.bIsEquippable || !Item.ItemClass)
	{
		return;
	}

	FVector Start, Dir;
	GetAimInfo(Start, Dir);

	AActor* ThrownActor = HeldItemActor;
	HeldItemActor = nullptr;

	ThrownActor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(ThrownActor->GetRootComponent()))
	{
		Prim->IgnoreActorWhenMoving(this, true);
		Prim->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

		if (UProjectileMovementComponent* Proj = ThrownActor->FindComponentByClass<UProjectileMovementComponent>())
		{
			Proj->Velocity = Dir * ThrowImpulseStrength;
			Proj->Activate(true);
			Prim->SetSimulatePhysics(false);
		}
		else
		{
			Prim->SetSimulatePhysics(true);
			Prim->AddImpulse(Dir * ThrowImpulseStrength, NAME_None, true);
		}

		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle, [Prim, this]()
		{
			if (IsValid(Prim))
			{
				Prim->IgnoreActorWhenMoving(this, false);
			}
		}, 0.2f, false);
	}

	InventoryComponent->RemoveItem(SlotIndex, 1);

	const FInventoryItem After = InventoryComponent->GetItemAt(SlotIndex);
	if (After.Quantity > 0 && After.bIsEquippable && After.ItemClass)
	{
		EquipSelectedItem();
	}
}

// -------------------- 스태미나 로직 --------------------

void ACPlayerCharacter::UpdateStamina(float DeltaTime)
{
	// 달리는 중이면 소모
	if (bisSprint)
	{
		const float Drain = SprintDrainPerSecond * DeltaTime;
		const float Before = CurrentStamina;
		SetCurrentStamina(FMath::Max(0.f, CurrentStamina - Drain));
		MarkStaminaUsed();

		// 바닥나면 강제 종료 + 기진 상태
		if (Before > 0.f && CurrentStamina <= 0.f)
		{
			bStaminaExhausted = true;
			EndSprint();
		}
	}
	else
	{
		// 소모 후 일정 시간 지나면 회복
		TimeSinceLastStaminaUse += DeltaTime;
		if (TimeSinceLastStaminaUse >= RegenDelay && CurrentStamina < MaxStamina)
		{
			SetCurrentStamina(FMath::Min(MaxStamina, CurrentStamina + WalkRegenPerSecond * DeltaTime));

			// 임계치 이상 회복되면 기진 상태 해제
			if (bStaminaExhausted && CurrentStamina >= ExhaustedRecoverThreshold)
			{
				bStaminaExhausted = false;
			}
		}
	}
}

void ACPlayerCharacter::MarkStaminaUsed()
{
	TimeSinceLastStaminaUse = 0.f;
}

void ACPlayerCharacter::SetCurrentStamina(float NewValue)
{
	NewValue = FMath::Clamp(NewValue, 0.f, MaxStamina);
	if (!FMath::IsNearlyEqual(NewValue, CurrentStamina))
	{
		CurrentStamina = NewValue;
		OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
	}
	else
	{
		CurrentStamina = NewValue;
	}
}