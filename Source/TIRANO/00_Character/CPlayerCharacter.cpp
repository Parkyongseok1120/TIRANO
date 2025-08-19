// Fill out your copyright notice in the Description page of Project Settings.

#include "00_Character/CPlayerCharacter.h"
#include "00_Character/02_Component/01_Input/CEnhancedInputComponent.h"
#include "00_Character/02_Component/01_Input/CGameplayTags.h"
#include "00_Character/02_Component/CDashComponent.h"
#include "00_Character/02_Component/03_Inventory/CInventoryComponent.h"
#include "02_UI/CHotbarWidget.h"

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
#include "02_Component/03_Inventory/CItemImageManager.h"
#include "Blueprint/UserWidget.h"

// Sets default values
ACPlayerCharacter::ACPlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
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
	//StateComponent->OnMovementTypeChanged.AddDynamic(this, &ACPlayerCharacter::OnMovementTypeChanged);
	SpringArm->bEnableCameraLag = true;

	// DashComponent NULL 체크
	if (!DashComponent)
	{
		CLog::Log("DashComponent가 NULL입니다. 확인 필요!");
	}
	
	if (AbilitySystemComponent)
	{
		// 속성 초기화 (필요한 경우)
		if (AttributeSet)
		{
			// 초기값 설정 (데이터 테이블 등에서 불러오는 방식으로 확장 가능)
			AbilitySystemComponent->InitStats(UCPlayerAttributeSet::StaticClass(), nullptr);
		}
    
		// 기본 어빌리티 부여 로직 (필요한 경우 추가)
		// GiveAbility(DefaultAbilities) 등으로 확장 가능
	}

	// 인벤토리 이벤트: 슬롯 변경 시 자동 장착
	if (InventoryComponent)
	{
		InventoryComponent->OnSelectedSlotChanged.AddDynamic(this, &ACPlayerCharacter::OnSelectedSlotChanged);
		// 시작 시 현재 선택 슬롯 장착 시도
		if (bAutoEquipOnSlotChange)
		{
			EquipSelectedItem();
		}
	}

	// 핫바 위젯 생성 및 설정
	if (HotbarWidgetClass)
	{
		APlayerController* PC = Cast<APlayerController>(GetController());
		if (PC && PC->IsLocalPlayerController()) // 로컬 플레이어만 UI 생성
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
}

// Called every frame
void ACPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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
	// 대시 입력 바인딩 (Shift 키로 가정)
	CEnhancedInputComponent->BindActionByTag(InputConfig, CGameplayTags::InputTag_Dash, ETriggerEvent::Started, this, &ACPlayerCharacter::BeginDash);
	CEnhancedInputComponent->BindActionByTag(InputConfig, CGameplayTags::InputTag_Jump, ETriggerEvent::Started, this, &ACPlayerCharacter::Jump);
	CEnhancedInputComponent->BindActionByTag(InputConfig, CGameplayTags::InputTag_Sprint, ETriggerEvent::Started, this, &ACPlayerCharacter::BeginSprint);
	CEnhancedInputComponent->BindActionByTag(InputConfig, CGameplayTags::InputTag_Sprint, ETriggerEvent::Completed, this, &ACPlayerCharacter::EndSprint);

	// 인벤토리 관련 입력 바인딩
	CEnhancedInputComponent->BindActionByTag(InputConfig, CGameplayTags::InputTag_NextItem, ETriggerEvent::Triggered, this, &ACPlayerCharacter::Input_NextItem);
	CEnhancedInputComponent->BindActionByTag(InputConfig, CGameplayTags::InputTag_PrevItem, ETriggerEvent::Triggered, this, &ACPlayerCharacter::Input_PrevItem);
	CEnhancedInputComponent->BindActionByTag(InputConfig, CGameplayTags::InputTag_SelectSlot, ETriggerEvent::Triggered, this, &ACPlayerCharacter::Input_SelectSlot);

	// 던지기 입력 바인딩(프로젝트 태그에 InputTag_Throw 존재해야 함)
	if (CGameplayTags::InputTag_Throw.IsValid())
	{
		CEnhancedInputComponent->BindActionByTag(InputConfig, CGameplayTags::InputTag_Throw, ETriggerEvent::Started, this, &ACPlayerCharacter::ThrowCurrentItem);
	}
}

void ACPlayerCharacter::BeginZoom()
{
	/*if (bisSprint == true)
		EndSprint();

	bWantsToZoom = true;
	SpringArm->bEnableCameraLag = false;
	*/
}

void ACPlayerCharacter::EndZoom()
{
	/*
	bWantsToZoom = false;
	SpringArm->bEnableCameraLag = true;
	*/
}

void ACPlayerCharacter::Input_Move(const FInputActionValue& InputActionValue)
{
	// input is a Vector2D
	FVector2D MovementVector = InputActionValue.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
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
	if (bWantsToZoom == false)
	{
		bisSprint = true;
		GetCharacterMovement()->MaxWalkSpeed = SprintingSpeed;
        
		// 대시 컴포넌트에 달리기 상태 전달
		if (DashComponent)
		{
			DashComponent->SetOwnerSprinting(true);
		}
	}
}

void ACPlayerCharacter::EndSprint()
{
	bisSprint = false;
	OnWalk();
    
	// 대시 컴포넌트에 달리기 종료 상태 전달
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
	/*// 첫 번째 점프
	Super::Jump();
	if (IsGrounded() != true && bCanDoubleJump == true)
	{
		// 더블 점프
		LaunchCharacter(FVector(0.0f, 0.0f, JumpForce), false, true);
		bCanDoubleJump = false;
	}

	if (IsGrounded() == true)
	{
		bCanDoubleJump = true;
	}*/
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
	FVector End = Start - FVector(0, 0, 100.0f);  // 발 아래 100 유닛까지 체크

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);  // 자기 자신은 무시

	// Line Trace 실행
	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		Start,
		End,
		ECC_Visibility,  // 또는 커스텀 트레이스 채널
		QueryParams
	);

	// 디버그 표시 (개발 중에 유용)
	DrawDebugLine(
		GetWorld(),
		Start,
		End,
		FColor::Red,
		false,
		1.0f,
		0,
		1.0f
	);
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

	// 기존 들고 있던 액터 정리
	UnequipCurrentItem();

	// 선택된 아이템 확인
	const FInventoryItem Item = InventoryComponent->GetSelectedItem();
	if (Item.Quantity <= 0 || !Item.bIsEquippable || !Item.ItemClass)
	{
		return;
	}

	// 액터 스폰
	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.Instigator = this;

	AActor* NewHeld = GetWorld()->SpawnActor<AActor>(Item.ItemClass, FTransform::Identity, Params);
	if (!NewHeld)
	{
		return;
	}

	// 손 소켓에 부착
	if (USkeletalMeshComponent* CharMesh = GetMesh())
	{
		NewHeld->AttachToComponent(CharMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, HandSocketName);
		NewHeld->AddActorLocalOffset(HoldOffset);
		NewHeld->AddActorLocalRotation(HoldRotationOffset);

		// 들고 있을 땐 충돌/물리 비활성
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

	// 손 위치 기준으로 살짝 앞으로
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

	// 조준 정보
	FVector Start, Dir;
	GetAimInfo(Start, Dir);

	// 들고 있던 액터 분리
	AActor* ThrownActor = HeldItemActor;
	HeldItemActor = nullptr;

	ThrownActor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	// 잠시 플레이어와의 충돌 무시
	if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(ThrownActor->GetRootComponent()))
	{
		Prim->IgnoreActorWhenMoving(this, true);
		// 던지기 직전 충돌/물리 설정
		Prim->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

		// 가능한 경우 ProjectileMovement 우선 사용
		if (UProjectileMovementComponent* Proj = ThrownActor->FindComponentByClass<UProjectileMovementComponent>())
		{
			Proj->Velocity = Dir * ThrowImpulseStrength;
			Proj->Activate(true);
			Prim->SetSimulatePhysics(false);
		}
		else
		{
			// 물리 임펄스 사용
			Prim->SetSimulatePhysics(true);
			Prim->AddImpulse(Dir * ThrowImpulseStrength, NAME_None, true);
		}

		// 짧은 시간 후 충돌 복구
		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle, [Prim, this]()
		{
			if (IsValid(Prim))
			{
				Prim->IgnoreActorWhenMoving(this, false);
			}
		}, 0.2f, false);
	}

	// 인벤토리 수량 감소
	InventoryComponent->RemoveItem(SlotIndex, 1);

	// 남은 수량이 있으면 같은 아이템 다시 손에 들기
	const FInventoryItem After = InventoryComponent->GetItemAt(SlotIndex);
	if (After.Quantity > 0 && After.bIsEquippable && After.ItemClass)
	{
		EquipSelectedItem();
	}
}