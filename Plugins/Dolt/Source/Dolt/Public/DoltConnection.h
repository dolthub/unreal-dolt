#pragma once

#include "CoreMinimal.h"

#include "./DoltSettings.h"
#include "./Result.h"

#include "DoltConnection.generated.h"

UCLASS()
class UDoltConnection : public UObject {
    GENERATED_BODY()

public:
	const FFilePath DoltBinPath;
	const FDirectoryPath DoltRepoPath;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dolt")
    static UDoltConnection* ConnectToDolt(const FFilePath DoltBinPath, const FDirectoryPath DoltRepoPath) {
        // TODO: Require that the repo has a clean working set before connecting.

        UDoltConnection* Result = NewObject<UDoltConnection>();
        Result->DoltBinPath = DoltBinPath;
        Result->DoltRepoPath = DoltRepoPath;
        return Result;
    }

    struct CommandOutput {
        FString *StdOut = nullptr;
        FString *StdErr = nullptr;
        int32 *OutReturnCode = nullptr;
    };
    
    bool ExecuteCommand(
        CommandOutput Output,
        FString Args) const;

    void ExportDataTables(
        TArray<UDataTable*> DataTable,
        FString BranchName,
        FString ParentBranchName,
        TEnumAsByte<DoltResult::Type> &IsSuccess,
        FString &OutMessage) const;

    void ImportDataTables(
        TArray<UDataTable*> DataTables,
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

    void ImportTableToDolt(
        FString TableName,
        FString FilePath,
        TEnumAsByte<DoltResult::Type> &IsSuccess,
        FString &OutMessage) const;

    void Commit(FString Message,
        TEnumAsByte<DoltResult::Type> &IsSuccess,
        FString &OutMessage) const;
};