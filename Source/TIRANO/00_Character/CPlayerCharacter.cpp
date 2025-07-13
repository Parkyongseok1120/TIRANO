// Fill out your copyright notice in the Description page of Project Settings.


#include "00_Character/CPlayerCharacter.h"
#include "00_Character/02_Component/CEnhancedInputComponent.h"
#include "00_Character/02_Component/CGameplayTags.h"
#include "00_Character/02_Component/CDashComponent.h"


#include "EnhancedInput/Public/InputAction.h"
#include "GameFramework/Character.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Global.h"

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

	GetMesh()->SetRelativeLocation(FVector(0, 0, -90));
	GetMesh()->SetRelativeRotation(FRotator(0, -90, 0));
	SpringArm->SetRelativeRotation(FRotator(0, 0, 0));
	SpringArm->SetRelativeLocation(FVector(-18, 63, 49));

	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	bWantsToZoom = false;
	bisSprint = false;
	bCanDoubleJump = true;
}

// Called when the game starts or when spawned
void ACPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	DefaultFOV = PlayerCamera->FieldOfView;
	GetCharacterMovement()->MaxWalkSpeed = WalkingSpeed;
	//StateComponent->OnMovementTypeChanged.AddDynamic(this, &ACPlayerCharacter::OnMovementTypeChanged);
	SpringArm->bEnableCameraLag = true;
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
}

void ACPlayerCharacter::BeginZoom()
{
	if (bisSprint == true)
		EndSprint();

	bWantsToZoom = true;
	SpringArm->bEnableCameraLag = false;
}

void ACPlayerCharacter::EndZoom()
{
	bWantsToZoom = false;
	SpringArm->bEnableCameraLag = true;
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
	// 첫 번째 점프
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
	}
}

// 대시 시작 함수
void ACPlayerCharacter::BeginDash()
{
	if (DashComponent)
	{
		DashComponent->StartDash();
	}
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

