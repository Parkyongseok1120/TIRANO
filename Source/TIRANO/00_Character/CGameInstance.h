// CGameInstance.h
#pragma once

#include "CGameInstance.generated.h"

UCLASS()
class TIRANO_API UCGameInstance : public UGameInstance
{
	GENERATED_BODY()
    
public:
	UCGameInstance();
    
	virtual void Init() override;
	
};