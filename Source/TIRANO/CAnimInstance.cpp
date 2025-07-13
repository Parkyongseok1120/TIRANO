// Fill out your copyright notice in the Description page of Project Settings.


#include "CAnimInstance.h"

#include "Global.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "00_Character/CPlayerCharacter.h"

void UCAnimInstance::NativeBeginPlay()

{
	Super::NativeBeginPlay();

	PlayerCharacter = Cast<ACPlayerCharacter>(TryGetPawnOwner());

	if (PlayerCharacter)
	{
		MovementComponent = PlayerCharacter->GetCharacterMovement();
	}
}

void UCAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	
	Super::NativeUpdateAnimation(DeltaSeconds);
	CheckNull(PlayerCharacter);
	if (MovementComponent)
	{
		if (C_Speed !=0)
		{
			PlayerCharacter->bUseControllerRotationYaw = true;            
		}
		else if (C_Speed == 0)
		{
			PlayerCharacter->bUseControllerRotationYaw = false;            
		}

		const FVector Velocity = PlayerCharacter->GetVelocity();
		const FVector CharacterRotation = PlayerCharacter->GetActorRotation().Vector();
		
		C_Speed = PlayerCharacter->GetVelocity().Size2D();
		FRotator rotator = PlayerCharacter->GetVelocity().ToOrientationRotator(); //현재 가는 벡터위치의 회전값을 구함.
		FRotator rotator2 = PlayerCharacter->GetControlRotation(); // 카메라의 회전방향
		FRotator DeltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(rotator, rotator2); // 두 회전값의 평균을 구함

		//// 이전 회전값.
		PrevRotation = UKismetMathLibrary::RInterpTo(PrevRotation, DeltaRotation, DeltaSeconds, 100.0f);
		C_Direction = PrevRotation.Yaw;
	
		if (FMath::Abs(C_Direction) > 146.0f) // 뒤로 이동 방향을 감지
		{
			if (C_Speed <= 10.0f)
				C_Direction = FMath::Abs(0);
			C_Direction = FMath::Abs(180.0f);//;FMath::Clamp(C_Direction, -15.0f, 155.0f); // 극단적인 값을 제한
		}
		// 방향값의 급격한 변화를 제한하여 부드럽게 만듦
		static float SmoothedDirection = 0.0f;
		SmoothedDirection = FMath::FInterpTo(SmoothedDirection, C_Direction, DeltaSeconds, 8.0f); // 보간 속도 설정
		C_Direction = SmoothedDirection;
	
		bFalling = PlayerCharacter->GetCharacterMovement()->IsFalling();

		
		// 이동 중일 때 Yaw 값을 고정하고 Pitch만 업데이트
		if (Velocity.SizeSquared() > 0.0f) // 이동 중일 때만
		{
			// Yaw 값은 고정
			const FRotator AimRotation = PlayerCharacter->GetBaseAimRotation();
			C_Pitch = AimRotation.Pitch;
            C_Yaw = FMath::FInterpTo(C_Yaw, 0.0f, DeltaSeconds, 5.0f);  // 5.0f는 변화 속도
		}
		else
		{
			// 이동하지 않으면 기존의 회전 값으로 업데이트
			const FRotator AimRotation = PlayerCharacter->GetBaseAimRotation();
			C_Pitch = AimRotation.Pitch;
			C_Yaw = AimRotation.Yaw;

			bFalling = PlayerCharacter->GetCharacterMovement()->IsFalling();
		}
	}
}
