
#include "DoltConnection.h"
#include "CoreMinimal.h"

bool FDoltConnection::ExecuteCommand(CommandOutput Output, FString Args) {
    UE_LOG(LogTemp, Error, TEXT("Executing dolt %s"), *Args);
    return FPlatformProcess::ExecProcess(*DoltBinPath.FilePath, *Args, Output.OutReturnCode, Output.StdOut, Output.StdErr, *DoltRepoPath.Path);
}

bool FDoltConnection::ExportDataTable(UDataTable* DataTable, FString BranchName, FString ParentBranchName) {
    const TCHAR* TempDir = FGenericPlatformProcess::UserTempDir();
    FString Path = FPaths::CreateTempFilename(TempDir, u"Dolt", u".csv");
    FString DataTableContents = DataTable->GetTableAsCSV(EDataTableExportFlags::None);
    if (!FFileHelper::SaveStringToFile(DataTableContents, *Path)) {
        UE_LOG(LogTemp, Error, TEXT("Failed to save DataTable to file"));
        return false;
    }
    UE_LOG(LogTemp, Display, TEXT("Saved to %s"), *Path);
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    
    CheckoutExistingBranch(ParentBranchName);
    if (BranchName != ParentBranchName) {
        CheckoutNewBranch(BranchName);
    }
    ImportTableToDolt(DataTable->GetName(), Path);
    Commit(FString::Printf(TEXT("Imported %s from Unreal"), *DataTable->GetName()));

    return true;
}

bool FDoltConnection::ImportDataTable(UDataTable* DataTable) {
    const TCHAR* TempDir = FGenericPlatformProcess::UserTempDir();
    FString Path = FPaths::CreateTempFilename(TempDir, u"Dolt", u".csv");
    UE_LOG(LogTemp, Display, TEXT("Created tmp file at %s"), *Path);
    FString StdOut, StdErr;
    FString Args = FString::Printf(TEXT("table export %s \"%s\""), *DataTable->GetName(), *Path);
    FPlatformProcess::ExecProcess(*DoltBinPath.FilePath, *Args, nullptr, &StdOut, &StdErr, *DoltRepoPath.Path);

    if (StdOut.Len() > 0) {
        UE_LOG(LogTemp, Display, TEXT("Dolt Output: %s"), *StdOut);
    }

    if (StdErr.Len() > 0) {
        UE_LOG(LogTemp, Error, TEXT("Dolt Error: %s"), *StdErr);
    }

    FString DataTableContents;
    if (!FFileHelper::LoadFileToString(DataTableContents, *Path)) {
        UE_LOG(LogTemp, Error, TEXT("Failed to load DataTable from file"));
        return false;
    }
    DataTable->EmptyTable();
    DataTable->CreateTableFromCSVString(DataTableContents);
    return true;
}

bool FDoltConnection::CheckoutNewBranch(FString BranchName) {
    FString StdOut, StdErr;
    int32 OutReturnCode;
    ExecuteCommand(
        {.StdOut = &StdOut, .StdErr = &StdErr, .OutReturnCode = &OutReturnCode},
        FString::Printf(TEXT("checkout -b %s"), *BranchName)
    );

    if (OutReturnCode != 0) {
        UE_LOG(LogTemp, Error, TEXT("Failed to checkout branch %s"), *BranchName);
        UE_LOG(LogTemp, Error, TEXT("Dolt Error: %s"), *StdErr);
        UE_LOG(LogTemp, Display, TEXT("Dolt Output: %s"), *StdOut);

        return false;
    }
    
    return true;
}

bool FDoltConnection::CheckoutExistingBranch(FString BranchName) {
    FString StdOut, StdErr;
    int32 OutReturnCode;
    ExecuteCommand(
        {.StdOut = &StdOut, .StdErr = &StdErr, .OutReturnCode = &OutReturnCode},
        FString::Printf(TEXT("checkout %s"), *BranchName)
    );

    if (OutReturnCode != 0) {
        UE_LOG(LogTemp, Error, TEXT("Failed to checkout branch %s"), *BranchName);
        UE_LOG(LogTemp, Error, TEXT("Dolt Error: %s"), *StdErr);
        UE_LOG(LogTemp, Display, TEXT("Dolt Output: %s"), *StdOut);

        return false;
    }
    
    return true;
}

bool FDoltConnection::ImportTableToDolt(FString TableName, FString FilePath) {
    FString StdOut, StdErr;
    int32 OutReturnCode;
    ExecuteCommand(
        {.StdOut = &StdOut, .StdErr = &StdErr, .OutReturnCode = &OutReturnCode},
        FString::Printf(TEXT("table import -c -f %s \"%s\""), *TableName, *FilePath)
    );
        if (OutReturnCode != 0) {
        UE_LOG(LogTemp, Error, TEXT("Failed to import table %s"), *FilePath);
        UE_LOG(LogTemp, Error, TEXT("Dolt Error: %s"), *StdErr);
        UE_LOG(LogTemp, Display, TEXT("Dolt Output: %s"), *StdOut);

        return false;
    }
    
    return true;
}

bool FDoltConnection::Commit(FString Message) {
    FString StdOut, StdErr;
    int32 OutReturnCode;
    ExecuteCommand(
        {.StdOut = &StdOut, .StdErr = &StdErr, .OutReturnCode = &OutReturnCode},
        FString::Printf(TEXT("commit -A -m \"%s\""), *Message)
    );
    if (OutReturnCode != 0) {
        UE_LOG(LogTemp, Error, TEXT("Failed to commit changes"));
        UE_LOG(LogTemp, Error, TEXT("Dolt Error: %s"), *StdErr);
        UE_LOG(LogTemp, Display, TEXT("Dolt Output: %s"), *StdOut);

        return false;
    }
    
    return true;
}