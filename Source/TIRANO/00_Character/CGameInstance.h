// CGameInstance.h
#pragma once

#include "00_Character/02_Component/CItemImageManager.h"
#include "CGameInstance.generated.h"

UCLASS()
class TIRANO_API UCGameInstance : public UGameInstance
{
	GENERATED_BODY()
    
public:
	UCGameInstance();
    
	virtual void Init() override;
    
	UFUNCTION(BlueprintCallable, Category = "Item")
	UCItemImageManager* GetItemImageManager() const { return ItemImageManager; }
    
private:
	UPROPERTY()
	UCItemImageManager* ItemImageManager;
};