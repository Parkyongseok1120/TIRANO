// Fill out your copyright notice in the Description page of Project Settings.

#include "CPlayerAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GameplayEffectExtension.h"
#include "GameFramework/Actor.h"

UCPlayerAttributeSet::UCPlayerAttributeSet()
{
    Health = 100.f;
    MaxHealth = 100.f;
    Mana = 50.f;
    MaxMana = 50.f;
}

void UCPlayerAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
    Super::PreAttributeChange(Attribute, NewValue);

    if (Attribute == GetHealthAttribute())
    {
        // Health가 변경될 때 MaxHealth를 초과하지 않도록 제한
        if (NewValue > MaxHealth.GetCurrentValue())
        {
            NewValue = MaxHealth.GetCurrentValue();
        }
    }
    else if (Attribute == GetManaAttribute())
    {
        // Mana가 변경될 때 MaxMana를 초과하지 않도록 제한
        if (NewValue > MaxMana.GetCurrentValue())
        {
            NewValue = MaxMana.GetCurrentValue();
        }
    }
}

void UCPlayerAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    Super::PostGameplayEffectExecute(Data);

    // 데미지 처리나 사망 판정 등
    if (Data.EvaluatedData.Attribute == GetHealthAttribute())
    {
        // 체력이 0 이하면 사망 처리
        if (GetHealth() <= 0.f)
        {
            // 소유자 캐릭터 가져와서 사망 처리
            AActor* Owner = GetOwningActor();
            if (Owner)
            {
                // 사망 로직 (예: 이벤트 호출)
            }
        }
    }
}

AActor* UCPlayerAttributeSet::GetOwningActor() const
{
    UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
    return ASC ? ASC->GetOwnerActor() : nullptr;
}
