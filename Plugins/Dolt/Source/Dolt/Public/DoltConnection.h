#pragma once

#include "CoreMinimal.h"

#include "./DoltSettings.h"
#include "./Result.h"

#include "DoltConnection.generated.h"

enum class CommitOptions : uint8 {
    None = 0,
    SkipEmpty = 1 << 0,
};
ENUM_CLASS_FLAGS(CommitOptions);

UCLASS()
class UDoltConnection : public UObject {
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
        return Result;
    }

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

    void ImportTableToDolt(
        FString TableName,
        FString FilePath,
        TEnumAsByte<DoltResult::Type> &IsSuccess,
        FString &OutMessage) const;
    
    void Commit(FString Message,
        CommitOptions Options,
        TEnumAsByte<DoltResult::Type> &IsSuccess,
        FString &OutMessage) const;
};