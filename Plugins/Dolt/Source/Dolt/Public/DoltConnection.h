#pragma once

#include "CoreMinimal.h"

class FDoltConnection {

public:
    const FString DoltBinPath;
    const FString DoltRepoPath;

    static FDoltConnection Connect(FString DoltBinPath, FString DoltRepoPath) {
        return FDoltConnection {
            .DoltBinPath = DoltBinPath,
            .DoltRepoPath = DoltRepoPath
        };
    }

    bool ExportDataTable(UDataTable* DataTable);

    bool ImportDataTable(UDataTable* DataTable);
};