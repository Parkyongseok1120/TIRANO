// Fill out your copyright notice in the Description page of Project Settings.

#include "00_Character/CPlayerCharacter.h"
#include "00_Character/02_Component/01_Input/CEnhancedInputComponent.h"
#include "00_Character/02_Component/01_Input/CGameplayTags.h"
#include "00_Character/02_Component/CDashComponent.h"
#include "00_Character/02_Component/03_Inventory/CInventoryComponent.h"
#include "00_Character/02_Component/02_ABS/CPlayerAttributeSet.h"

#include "02_UI/CHotbarWidget.h"
#include "02_UI/CBatteryHUDWidget.h"
#include "02_UI/CStatusUI.h"
#include "03_World/CDoorActor.h"     

#include "01_Item/CThrowableItemBase.h"
#include "01_Item/CFlashlightItem.h"

#include "Perception/AISense_Hearing.h"
#include "EnhancedInput/Public/InputAction.h"
#include "GameFramework/Character.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"         
#include "Components/CapsuleComponent.h"    
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "CGameInstance.h"

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
	// ===== Enemy KillRange 전용 오버랩 스피어 생성 =====
	KillOverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("KillOverlapSphere"));
	KillOverlapSphere->SetupAttachment(GetCapsuleComponent()); // 캡슐에 부착
	KillOverlapSphere->SetSphereRadius(KillOverlapRadius);
	KillOverlapSphere->SetRelativeLocation(FVector::ZeroVector);
	KillOverlapSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	// Enemy KillRange는 WorldDynamic + Pawn Overlap.
	// 우리 스피어는 Pawn 타입으로, WorldDynamic에 Overlap으로 응답해 상호 Overlap 달성.
	KillOverlapSphere->SetCollisionObjectType(ECC_Pawn);
	KillOverlapSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	KillOverlapSphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	KillOverlapSphere->SetGenerateOverlapEvents(true);
	KillOverlapSphere->SetCanEverAffectNavigation(false);
	KillOverlapSphere->SetHiddenInGame(true); // 시각적으로 숨김

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

	// 배터리 HUD 생성
	if (BatteryWidgetClass)
	{
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			if (PC->IsLocalPlayerController())
			{
				BatteryWidget = CreateWidget<UCBatteryHUDWidget>(PC, BatteryWidgetClass);
				if (BatteryWidget)
				{
					BatteryWidget->AddToViewport(1);
					UpdateBatteryUI();
					UpdateBatteryReplacePrompt();
				}
			}
		}
	}

	// 인벤토리 변경 이벤트에 UI 갱신 연결(파라미터 없는 델리게이트에 맞게 전용 핸들러 사용)
	if (InventoryComponent)
	{
		InventoryComponent->OnInventoryUpdated.AddDynamic(this, &ACPlayerCharacter::OnInventoryUpdated);
	}
}


// Called every frame
void ACPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateStamina(DeltaTime);
	UpdateBatteryUI();
	EmitFootstepIfNeeded(DeltaTime);

}

// Called to bind functionality to input
void ACPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	UCEnhancedInputComponent* CEnhancedInputComponent = Cast<UCEnhancedInputComponent>(PlayerInputComponent);
	check(CEnhancedInputComponent);

	CEnhancedInputComponent->BindActionByTag(InputConfig, CGameplayTags::InputTag_Move, ETriggerEvent::Triggered, this, &ACPlayerCharacter::Input_Move);
	CEnhancedInputComponent->BindActionByTag(InputConfig, CGameplayTags::InputTag_Look, ETriggerEvent::Triggered, this, &ACPlayerCharacter::Input_Look);

	// 스프린트
	CEnhancedInputComponent->BindActionByTag(InputConfig, CGameplayTags::InputTag_Sprint, ETriggerEvent::Started, this, &ACPlayerCharacter::BeginSprint);
	CEnhancedInputComponent->BindActionByTag(InputConfig, CGameplayTags::InputTag_Sprint, ETriggerEvent::Completed, this, &ACPlayerCharacter::EndSprint);

	// 인벤토리 관련 입력 바인딩
	CEnhancedInputComponent->BindActionByTag(InputConfig, CGameplayTags::InputTag_NextItem, ETriggerEvent::Triggered, this, &ACPlayerCharacter::Input_NextItem);
	CEnhancedInputComponent->BindActionByTag(InputConfig, CGameplayTags::InputTag_PrevItem, ETriggerEvent::Triggered, this, &ACPlayerCharacter::Input_PrevItem);
	CEnhancedInputComponent->BindActionByTag(InputConfig, CGameplayTags::InputTag_SelectSlot, ETriggerEvent::Triggered, this, &ACPlayerCharacter::Input_SelectSlot);

	// 던지기
	CEnhancedInputComponent->BindActionByTag(InputConfig, CGameplayTags::InputTag_Throw, ETriggerEvent::Started, this, &ACPlayerCharacter::ThrowCurrentItem);
	CEnhancedInputComponent->BindActionByTag(InputConfig, CGameplayTags::InputTag_Interaction, ETriggerEvent::Started, this, &ACPlayerCharacter::OnFPressed);

}

void ACPlayerCharacter::OnFPressed()
{
	// 0) 문이 가까우면 문 토글이 최우선
	if (NearbyDoor.IsValid())
	{
		NearbyDoor->ToggleDoor();
		return;
	}

	const int32 SlotIdx = InventoryComponent ? InventoryComponent->GetSelectedSlotIndex() : -1;
	const FInventoryItem Selected = InventoryComponent ? InventoryComponent->GetSelectedItem() : FInventoryItem();

	ACFlashlightItem* Flashlight = GetHeldFlashlight();

	// 1) 선택 슬롯이 '배터리'이고, 손전등을 들고 있다면: 배터리 '소비'
	if (Flashlight && Selected.Quantity > 0 && Selected.ItemID == BatteryItemId.ToString())
	{
		const float NewBatteryFromInventory = FMath::Clamp(Selected.BatteryPercent, 0.f, 100.f);
		Flashlight->SetBatteryPercent(NewBatteryFromInventory);

		if (InventoryComponent && SlotIdx >= 0)
		{
			InventoryComponent->RemoveItem(SlotIdx, 1);
		}

		UpdateBatteryUI();
		UpdateBatteryReplacePrompt();
		return;
	}

	// 2) 그렇지 않으면: 손전등이 들려 있으면 토글
	if (Flashlight)
	{
		Flashlight->Toggle();
	}
}


// 문 근처 감지 진입/이탈 시 호출
void ACPlayerCharacter::SetNearbyDoor(ACDoorActor* Door, bool bEnter)
{
	if (bEnter)
	{
		NearbyDoor = Door;
		if (DoorPromptWidget)
		{
			DoorPromptWidget->SetVisibility(ESlateVisibility::Visible);
		}
	}
	else
	{
		if (NearbyDoor.Get() == Door)
		{
			NearbyDoor = nullptr;
			if (DoorPromptWidget)
			{
				DoorPromptWidget->SetVisibility(ESlateVisibility::Hidden);
			}
		}
	}
}


ACFlashlightItem* ACPlayerCharacter::GetHeldFlashlight() const
{
	return HeldItemActor ? Cast<ACFlashlightItem>(HeldItemActor) : nullptr;
}




void ACPlayerCharacter::EmitFootstepIfNeeded(float DeltaSeconds)
{
	// 간단한 속도 기반(달리기/걷기) 발소리
	const float Speed2D = GetVelocity().Size2D();
	const bool bMoving = Speed2D > 10.f;
	if (!bMoving) { FootstepTimer = 0.f; return; }

	FootstepTimer += DeltaSeconds;
	const bool bSprinting = bisSprint; // 기존 변수 사용
	const float Interval = bSprinting ? FootstepIntervalSprint : FootstepIntervalWalk;
	if (FootstepTimer >= Interval)
	{
		FootstepTimer = 0.f;
		const float Loud = bSprinting ? FootstepLoudnessSprint : FootstepLoudnessWalk;
		UAISense_Hearing::ReportNoiseEvent(GetWorld(), GetActorLocation(), Loud, this, FootstepMaxRange, TEXT("Footstep"));
	}
}

void ACPlayerCharacter::Input_Move(const FInputActionValue& InputActionValue)
{
	FVector2D MovementVector = InputActionValue.Get<FVector2D>();

	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		FVector InputDirection = ForwardDirection * MovementVector.Y + RightDirection * MovementVector.X;
        
		if (DashComponent)
		{
			DashComponent->SetInputDirection(InputDirection);
		}

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
	if (bWantsToZoom)
		return;

	if (CurrentStamina < ExhaustedRecoverThreshold || bStaminaExhausted)
	{
		return;
	}

	bisSprint = true;
	GetCharacterMovement()->MaxWalkSpeed = SprintingSpeed;

	if (DashComponent)
	{
		DashComponent->SetOwnerSprinting(true);
	}

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

void ACPlayerCharacter::Input_NextItem(const FInputActionValue& /*InputActionValue*/)
{
	if (InventoryComponent)
	{
		InventoryComponent->NextSlot();
	}
}

void ACPlayerCharacter::Input_PrevItem(const FInputActionValue& /*InputActionValue*/)
{
	if (InventoryComponent)
	{
		InventoryComponent->PrevSlot();
	}
}

void ACPlayerCharacter::Input_SelectSlot(const FInputActionValue& InputActionValue)
{
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
	UpdateBatteryUI();
	UpdateBatteryReplacePrompt();
}

// 파라미터 없는 인벤토리 갱신 핸들러
void ACPlayerCharacter::OnInventoryUpdated()
{
	UpdateBatteryUI();
	UpdateBatteryReplacePrompt();
}

// EquipSelectedItem 수정: 손전등일 때도 일반 장착 경로를 사용하되 AttachToHand 호출
void ACPlayerCharacter::EquipSelectedItem()
{
	if (!InventoryComponent) return;

	UnequipCurrentItem();

	const FInventoryItem Item = InventoryComponent->GetSelectedItem();
	if (Item.Quantity <= 0 || !Item.bIsEquippable || (!Item.ItemClass && !Item.ThrowableClass))
	{
		UpdateBatteryUI();
		UpdateBatteryReplacePrompt();
		return;
	}

	UClass* SpawnClass = Item.ThrowableClass ? Item.ThrowableClass.Get() : Item.ItemClass.Get();
	if (!SpawnClass) return;

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.Instigator = this;

	AActor* NewHeld = GetWorld()->SpawnActor<AActor>(SpawnClass, FTransform::Identity, Params);
	if (!NewHeld)
		return;

	const FVector FinalOffset = HoldOffset + Item.HoldOffset;
	const FRotator FinalRot = HoldRotationOffset + Item.HoldRotationOffset;

	if (ACFlashlightItem* Flashlight = Cast<ACFlashlightItem>(NewHeld))
	{
		Flashlight->AttachToHand(GetMesh(), HandSocketName, FinalOffset, FinalRot);
		// 인벤토리/아이템ID/소유자 정보 넘겨서 방전 시 제거 처리
		Flashlight->InitializeFromInventoryItem(Item, InventoryComponent, this);
	}
	else if (ACThrowableItemBase* Throwable = Cast<ACThrowableItemBase>(NewHeld))
	{
		Throwable->SetHeld(true, this, GetMesh(), HandSocketName, FinalOffset, FinalRot);
	}
	else
	{
		if (USkeletalMeshComponent* CharMesh = GetMesh())
		{
			NewHeld->AttachToComponent(CharMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, HandSocketName);
			NewHeld->AddActorLocalOffset(FinalOffset);
			NewHeld->AddActorLocalRotation(FinalRot);

			if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(NewHeld->GetRootComponent()))
			{
				Prim->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				Prim->SetSimulatePhysics(false);
			}
		}
		else
		{
			NewHeld->Destroy();
			return;
		}
	}

	HeldItemActor = NewHeld;

	// Held 액터 파괴 시 포인터 정리
	if (HeldItemActor)
	{
		HeldItemActor->OnDestroyed.AddDynamic(this, &ACPlayerCharacter::OnHeldActorDestroyed);
	}

	UpdateBatteryUI();
	UpdateBatteryReplacePrompt();
}

void ACPlayerCharacter::OnHeldActorDestroyed(AActor* DestroyedActor)
{
	// 손에 들고 있던 액터가 파괴되면 포인터 정리 및 UI 갱신
	if (DestroyedActor == HeldItemActor)
	{
		HeldItemActor = nullptr;
		UpdateBatteryUI();
		UpdateBatteryReplacePrompt();
	}
}

void ACPlayerCharacter::UnequipCurrentItem()
{
	if (HeldItemActor)
	{
		HeldItemActor->Destroy();
		HeldItemActor = nullptr;
	}

	// UI 갱신
	UpdateBatteryUI();
	UpdateBatteryReplacePrompt();
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

	// [추가] 시작 위치를 살짝 위로 올려서 시야/충돌 간섭 감소 + 초기 상승 느낌
	if (ThrowStartZOffset > 0.f)
	{
		OutStart += FVector(0, 0, ThrowStartZOffset);
	}
}

void ACPlayerCharacter::ThrowCurrentItem()
{
	if (!InventoryComponent || !HeldItemActor)
	{
		return;
	}

	// 손전등은 좌클릭으로 버려지지 않도록 막음
	if (Cast<ACFlashlightItem>(HeldItemActor))
	{
		return;
	}

	const int32 SlotIndex = InventoryComponent->GetSelectedSlotIndex();
	FInventoryItem Item = InventoryComponent->GetSelectedItem();
	if (Item.Quantity <= 0 || !Item.bIsEquippable || (!Item.ItemClass && !Item.ThrowableClass))
	{
		return;
	}

	FVector Start, Dir;
	GetAimInfo(Start, Dir);

	// [추가] 던질 방향을 '위로' 피치 오프셋 적용
	if (ThrowPitchOffsetDeg > 0.f)
	{
		FRotator UpRot = Dir.Rotation();
		UpRot.Pitch -= ThrowPitchOffsetDeg; // UE에서 Pitch를 줄이면 위로 향함
		Dir = UpRot.Vector();
	}


	// 분기: Throwable 기반이면 자체 Throw 사용, 아니면 기존 물리 임펄스
	if (ACThrowableItemBase* Throwable = Cast<ACThrowableItemBase>(HeldItemActor))
	{
		// Throw가 Detach/충돌 설정/무시시간/Fuse까지 처리
		Throwable->Throw(Start, Dir, this);
	}
	else
	{
		// 기존 물리/프로젝타일 기반 던지기
		AActor* ThrownActor = HeldItemActor;
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
	}

	// 손 비우기
	HeldItemActor = nullptr;

	// 인벤토리 차감
	InventoryComponent->RemoveItem(SlotIndex, 1);

	// 남은 동일 슬롯 아이템 자동 장착
	const FInventoryItem After = InventoryComponent->GetItemAt(SlotIndex);
	if (After.Quantity > 0 && After.bIsEquippable && (After.ItemClass || After.ThrowableClass))
	{
		EquipSelectedItem();
	}
}

// -------------------- 스태미나 로직 --------------------

void ACPlayerCharacter::UpdateStamina(float DeltaTime)
{
	if (bisSprint)
	{
		const float Drain = SprintDrainPerSecond * DeltaTime;
		const float Before = CurrentStamina;
		SetCurrentStamina(FMath::Max(0.f, CurrentStamina - Drain));
		MarkStaminaUsed();

		if (Before > 0.f && CurrentStamina <= 0.f)
		{
			bStaminaExhausted = true;
			EndSprint();
		}
	}
	else
	{
		TimeSinceLastStaminaUse += DeltaTime;
		if (TimeSinceLastStaminaUse >= RegenDelay && CurrentStamina < MaxStamina)
		{
			SetCurrentStamina(FMath::Min(MaxStamina, CurrentStamina + WalkRegenPerSecond * DeltaTime));

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

void ACPlayerCharacter::UpdateBatteryUI()
{
	if (!BatteryWidget) return;

	if (ACFlashlightItem* Flashlight = GetHeldFlashlight())
	{
		BatteryWidget->SetBatteryPercent(Flashlight->GetBatteryPercent());
	}
	else
	{
		// 손전등 없으면 0%로 표시하거나 필요 시 숨김 처리
		BatteryWidget->SetBatteryPercent(0.f);
	}
}

void ACPlayerCharacter::UpdateBatteryReplacePrompt()
{
	if (!BatteryWidget || !InventoryComponent) return;

	const FInventoryItem Selected = InventoryComponent->GetSelectedItem();
	const bool bIsBatterySelected = (Selected.Quantity > 0 && Selected.ItemID == BatteryItemId.ToString());
	const bool bHasFlashlight = (GetHeldFlashlight() != nullptr);

	if (bIsBatterySelected && bHasFlashlight)
	{
		BatteryWidget->ShowReplacePrompt(true, Selected.BatteryPercent);
	}
	else
	{
		BatteryWidget->ShowReplacePrompt(false);
	}
}