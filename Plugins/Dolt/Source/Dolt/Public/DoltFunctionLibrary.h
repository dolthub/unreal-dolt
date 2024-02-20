#pragma once

#include "CoreMinimal.h"
#include "AssetActionUtility.h"
#include "Engine/EngineTypes.h"
#include "DoltFunctionLibrary.generated.h"

UCLASS()
class DOLT_API UDoltFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
    UFUNCTION(BlueprintCallable, Category = "Dolt")
	static bool ExportDataTable(FFilePath DoltBinPath, FDirectoryPath DoltRepoPath);

    UFUNCTION(BlueprintCallable, Category = "Dolt")
    static bool ImportDataTable(FFilePath DoltBinPath, FDirectoryPath DoltRepoPath);

    UFUNCTION(BlueprintCallable, Category = "Dolt")
    static bool DiffDataTable(FFilePath DoltBinPath, FDirectoryPath DoltRepoPath);

    UFUNCTION(BlueprintCallable, Category = "Dolt")
    static bool DiffDataTable(FString DoltBinPath, FString DoltRepoPath);

};
