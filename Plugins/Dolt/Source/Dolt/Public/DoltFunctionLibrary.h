#pragma once

#include "CoreMinimal.h"
#include "AssetActionUtility.h"
#include "DoltFunctionLibrary.generated.h"

UCLASS()
class DOLT_API UDoltFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
    UFUNCTION(BlueprintCallable, Category = "Dolt")
	static bool ExportDataTable(FString DoltBinPath, FString DoltRepoPath);

    UFUNCTION(BlueprintCallable, Category = "Dolt")
    static bool ImportDataTable(FString DoltBinPath, FString DoltRepoPath);
};
