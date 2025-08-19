#include "CStatusUI.h"
#include "Components/ProgressBar.h"
#include "00_Character/CPlayerCharacter.h"

void UCStatusUI::NativeConstruct()
{
	Super::NativeConstruct();

	// 런타임에서도 강하게 체크(디자이너는 BindWidget로 컴파일 시점 실패)
	checkf(PB_Health != nullptr, TEXT("PB_Health ProgressBar is required on %s widget."), *GetName());

	// 이미 캐릭터가 연결되어 있으면 시작값 반영
	if (OwnerCharacter.IsValid())
	{
		OnCharacterStaminaChanged(OwnerCharacter->GetStamina(), OwnerCharacter->GetMaxStamina());
	}
}

void UCStatusUI::NativeDestruct()
{
	// 델리게이트 해제
	if (OwnerCharacter.IsValid())
	{
		OwnerCharacter->OnStaminaChanged.RemoveDynamic(this, &UCStatusUI::OnCharacterStaminaChanged);
	}
	Super::NativeDestruct();
}

void UCStatusUI::SetupWithCharacter(ACPlayerCharacter* InCharacter)
{
	// 기존 바인딩 해제
	if (OwnerCharacter.IsValid())
	{
		OwnerCharacter->OnStaminaChanged.RemoveDynamic(this, &UCStatusUI::OnCharacterStaminaChanged);
	}

	OwnerCharacter = InCharacter;

	if (OwnerCharacter.IsValid())
	{
		OwnerCharacter->OnStaminaChanged.AddDynamic(this, &UCStatusUI::OnCharacterStaminaChanged);
		// 즉시 초기값 반영
		OnCharacterStaminaChanged(OwnerCharacter->GetStamina(), OwnerCharacter->GetMaxStamina());
	}
}

void UCStatusUI::SetHealth(float Current, float Max)
{
	const float Ratio = (Max > 0.f) ? (Current / Max) : 0.f;
	ApplyRatio(Ratio);
}

void UCStatusUI::SetHealthRatio(float Ratio)
{
	ApplyRatio(Ratio);
}

void UCStatusUI::OnCharacterStaminaChanged(float Current, float Max)
{
	const float Ratio = (Max > 0.f) ? (Current / Max) : 0.f;
	ApplyRatio(Ratio);
}

void UCStatusUI::ApplyRatio(float Ratio)
{
	if (!PB_Health) return;
	PB_Health->SetPercent(FMath::Clamp(Ratio, 0.f, 1.f));
}