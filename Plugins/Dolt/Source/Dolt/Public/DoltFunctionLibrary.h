#pragma once

#include "CoreMinimal.h"
#include "AssetActionUtility.h"
#include "DoltSettings.h"
#include "Engine/EngineTypes.h"
#include "ISourceControlProvider.h"

#include "./Result.h"

#include "DoltFunctionLibrary.generated.h"


UCLASS()
class DOLT_API UDoltFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dolt")
	static const UDoltSettings* GetDoltProjectSettings();

    UFUNCTION(BlueprintCallable, Category = "Dolt", meta=(ExpandEnumAsExecs="IsSuccess"))
	static void ExportDataTable(const UDoltConnection* Dolt, const FString &BranchName, TEnumAsByte<DoltResult::Type>& IsSuccess, FString &OutMessage);

    UFUNCTION(BlueprintCallable, Category = "Dolt", meta=(ExpandEnumAsExecs="IsSuccess"))
    static void ImportDataTable(const UDoltConnection* Dolt, TEnumAsByte<DoltResult::Type>& IsSuccess, FString &OutMessage);

    UFUNCTION(BlueprintCallable, Category = "Dolt", meta=(ExpandEnumAsExecs="IsSuccess"))
    static void DiffDataTable(const UDoltConnection* Dolt, TEnumAsByte<DoltResult::Type>& IsSuccess, FString &OutMessage);

    UFUNCTION(BlueprintCallable, Category = "Dolt", meta=(ExpandEnumAsExecs="IsSuccess"))
    static void RebaseOntoHeadRevision(const UDoltConnection* Dolt, TEnumAsByte<DoltResult::Type>& IsSuccess, FString &OutMessage);

};
