#pragma once

#include "CoreMinimal.h"

class FDoltConnection {

public:
    const FFilePath DoltBinPath;
    const FDirectoryPath DoltRepoPath;

    static FDoltConnection Connect(FFilePath DoltBinPath, FDirectoryPath DoltRepoPath) {
        // TODO: Require that the repo has a clean working set before connecting.
        return FDoltConnection {
            .DoltBinPath = DoltBinPath,
            .DoltRepoPath = DoltRepoPath
        };
    }

    struct CommandOutput {
        FString *StdOut = nullptr;
        FString *StdErr = nullptr;
        int32 *OutReturnCode = nullptr;
    };
    
    bool ExecuteCommand(CommandOutput Output, FString Args);

    bool ExportDataTable(UDataTable* DataTable, FString BranchName, FString ParentBranchName);

    bool ImportDataTable(UDataTable* DataTable);

private:

    bool CheckoutNewBranch(FString BranchName);

    bool CheckoutExistingBranch(FString BranchName);

    bool ImportTableToDolt(FString TableName, FString FilePath);

    bool Commit(FString Message);
};