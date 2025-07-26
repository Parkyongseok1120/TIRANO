// CItemImageManager.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/DataTable.h"
#include "CItemImageManager.generated.h"

USTRUCT(BlueprintType)
struct FItemImageData : public FTableRowBase
{
	GENERATED_BODY()
    
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString ItemID;
    
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> ItemIcon;
    
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Description;
};

UCLASS()
class TIRANO_API UCItemImageManager : public UObject
{
	GENERATED_BODY()
    
public:
	UCItemImageManager();
    
	void Initialize();
    
	UFUNCTION(BlueprintCallable, Category = "Item")
	UTexture2D* GetItemIcon(const FString& ItemID);
    
private:
	UPROPERTY()
	TMap<FString, UTexture2D*> ItemIconCache;
    
	UPROPERTY()
	UDataTable* ItemImageTable;
    
	void LoadItemImageTable();
	void PreloadCommonItems();
};