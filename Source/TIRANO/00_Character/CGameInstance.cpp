#include "CGameInstance.h"
#include "00_Character/02_Component/CItemImageManager.h"

UCGameInstance::UCGameInstance()
{
}

void UCGameInstance::Init()
{
	Super::Init();
    
	ItemImageManager = NewObject<UCItemImageManager>(this);
	ItemImageManager->Initialize();
}
