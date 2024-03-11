// Copyright 2024 Dolthub, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include "CoreMinimal.h"
#include "DoltSettings.h"
#include "Engine/EngineTypes.h"
#include "ISourceControlProvider.h"
#include "HAL/IConsoleManager.h"

#include "./Result.h"

#include "DoltFunctionLibrary.generated.h"

#define DOLT_FAIL(Message) \
    IsSuccess = DoltResult::Failure;\
    OutMessage = Message;\
    UE_LOG(LogTemp, Error, TEXT("%s"), *OutMessage);\
    return;

#define DOLT_FAILF(Message, ...) \
    IsSuccess = DoltResult::Failure;\
    OutMessage = FString::Printf(TEXT(Message), ##__VA_ARGS__);\
    UE_LOG(LogTemp, Error, TEXT("%s"), *OutMessage);\
    return;

#define DOLT_SUCCEED(Message) \
    IsSuccess = DoltResult::Success;\
    OutMessage = Message;\
    UE_LOG(LogTemp, Display, TEXT("%s"), *OutMessage);\
    return;

#define DOLT_SUCCEEDF(Message, ...) \
    IsSuccess = DoltResult::Success;\
    OutMessage = FString::Printf(TEXT(Message), ##__VA_ARGS__);\
    UE_LOG(LogTemp, Display, TEXT("%s"), *OutMessage);\
    return;

#define DOLT_CHECK(Condition, Message) \
    if (!(Condition)) {\
        DOLT_FAIL(Message);\
    }

#define DOLT_CHECKF(Condition, Message, ...) \
    if (!(Condition)) {\
        DOLT_FAIL(Message, ##__VA_ARGS__);\
    }

template <typename T> TArray<T*> GetObjectsOfType(const TArray<UObject*> &Assets) {
    TArray<T*> TypedAssets;
    for (UObject* Asset : Assets)
    {
        T* TypedAsset = Cast<T>(Asset);
        if (TypedAsset)
        {
            TypedAssets.Add(TypedAsset);
        }
    }
    return TypedAssets;
}

UCLASS()
class DOLT_API UDoltFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dolt")
	static const UDoltSettings* GetDoltProjectSettings();

    UFUNCTION(BlueprintCallable, Category = "Dolt|Import/Export", meta=(ExpandEnumAsExecs="IsSuccess"))
	static void ExportDataTable(
        const UDoltConnection* Dolt,
        const TArray<UObject*> &DataTables,
        FString BranchName,
        TEnumAsByte<DoltResult::Type>& IsSuccess,
        FString &OutMessage);

    UFUNCTION(BlueprintCallable, Category = "Dolt|Import/Export", meta=(ExpandEnumAsExecs="IsSuccess"))
    static void ImportDataTable(
        const UDoltConnection* Dolt,
        const TArray<UObject*> &DataTables,
        FString BranchName,
        TEnumAsByte<DoltResult::Type>& IsSuccess,
        FString &OutMessage);

    UFUNCTION(BlueprintCallable, Category = "Dolt|Import/Export", meta=(ExpandEnumAsExecs="IsSuccess"))
    static void ThreeWayExport(
        const UDoltConnection* Dolt,
        const FString LocalBranch,
        const FString RemoteBranch,
        const TArray<UObject*> &DataTables,
        TEnumAsByte<DoltResult::Type>& IsSuccess,
        FString &OutMessage);

    UFUNCTION(BlueprintCallable, Category = "Dolt|Pull Rebase", meta=(ExpandEnumAsExecs="IsSuccess"))
    static void PullRebase(
        const UDoltConnection* Dolt,
        const TArray<UObject*> &DataTables,
        FString LocalBranch,
        FString RemoteBranch,
        TEnumAsByte<DoltResult::Type>& IsSuccess,
        FString &OutMessage);

    UFUNCTION(BlueprintCallable, Category = "Dolt|Pull Rebase", meta=(ExpandEnumAsExecs="IsSuccess"))
    static void ResumePullRebase(
        const UDoltConnection* Dolt,
        const TArray<UObject*> &DataTables,
        FString LocalBranch,
        FString RemoteBranch,
        TEnumAsByte<DoltResult::Type>& IsSuccess,
        FString &OutMessage);

    UFUNCTION(Exec)
    static void DoltEcho(const TArray<FString>& Messages);

    static FAutoConsoleCommand DoltEchoCommand;
};
