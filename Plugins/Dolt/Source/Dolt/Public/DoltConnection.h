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
#include "Engine/DataTable.h"

#include "./DoltSettings.h"
#include "./Result.h"

#include "DoltConnection.generated.h"

enum class CommitOptions : uint8 {
    None = 0,
    SkipEmpty = 1 << 0,
};
ENUM_CLASS_FLAGS(CommitOptions);

UCLASS()
class DOLT_API UDoltConnection : public UObject {
    GENERATED_BODY()

public:
	FFilePath DoltBinPath;
	FDirectoryPath DoltRepoPath;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dolt")
    static UDoltConnection* ConnectToDolt(FFilePath DoltBinPath, FDirectoryPath DoltRepoPath) {
        // TODO: Require that the repo has a clean working set before connecting.

        UDoltConnection* Result = NewObject<UDoltConnection>();
        Result->DoltBinPath = DoltBinPath;
        Result->DoltRepoPath = DoltRepoPath;
        Result->MaybeInitDolt("local", "remote");
        return Result;
    }

    void MaybeInitDolt(FString LocalBranch, FString RemoteBranch) const;

    void ExecuteCommand(FString Args) const;

    struct ExecuteCommandArgs {
        FString Command, SuccessMessage, FailureMessage;
    };
    
    void ExecuteCommand(
        ExecuteCommandArgs DoltCommand,
        TEnumAsByte<DoltResult::Type> &IsSuccess,
        FString &OutMessage) const;

    void ExportDataTables(
        TArray<UDataTable*> DataTable,
        FString BranchName,
        FString ParentBranchName,
        FString CommitMessage,
        TEnumAsByte<DoltResult::Type> &IsSuccess,
        FString &OutMessage) const;

    void ImportDataTables(
        TArray<UDataTable*> DataTables,
        FString BranchName,
        TEnumAsByte<DoltResult::Type> &IsSuccess,
        FString &OutMessage) const;

    struct MergeArgs {
        FString From, To;
    };

    void Merge(
        MergeArgs Args,
        TEnumAsByte<DoltResult::Type> &IsSuccess,
        FString &OutMessage) const;

    struct RebaseArgs {
        FString From, To;
    };

    void Rebase(
        RebaseArgs Args,
        TEnumAsByte<DoltResult::Type> &IsSuccess,
        FString &OutMessage) const;

    bool IsMerging(
        TEnumAsByte<DoltResult::Type> &IsSuccess,
        FString &OutMessage) const;

    void Commit(
        FString Message,
        CommitOptions Options,
        TEnumAsByte<DoltResult::Type> &IsSuccess,
        FString &OutMessage) const;

private:

    void CheckoutNewBranch(
        FString BranchName,
        TEnumAsByte<DoltResult::Type> &IsSuccess,
        FString &OutMessage) const;

    void CheckoutExistingBranch(
        FString BranchName,
        TEnumAsByte<DoltResult::Type> &IsSuccess,
        FString &OutMessage) const;

    void CheckoutNewOrExistingBranch(
        FString BranchName,
        TEnumAsByte<DoltResult::Type> &IsSuccess,
        FString &OutMessage) const;

    void CreateOrResetBranch(
        FString BranchName,
        FString Target,
        TEnumAsByte<DoltResult::Type> &IsSuccess,
        FString &OutMessage) const;

    void ImportTableToDolt(
        FString TableName,
        FString FilePath,
        TEnumAsByte<DoltResult::Type> &IsSuccess,
        FString &OutMessage) const;

    void HardReset(
        FString TargetBranchName,
        TEnumAsByte<DoltResult::Type> &IsSuccess,
        FString &OutMessage) const;
};