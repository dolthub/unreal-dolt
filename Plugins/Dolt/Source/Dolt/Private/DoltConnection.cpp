
#include "DoltConnection.h"

#include "./Result.h"

#include "CoreMinimal.h"

bool UDoltConnection::ExecuteCommand(CommandOutput Output, FString Args) const {
    UE_LOG(LogTemp, Display, TEXT("Executing dolt %s"), *Args);
    return FPlatformProcess::ExecProcess(*DoltBinPath.FilePath, *Args, Output.OutReturnCode, Output.StdOut, Output.StdErr, *DoltRepoPath.Path);
}

void UDoltConnection::ExportDataTables(
        TArray<UDataTable*> DataTables,
        FString BranchName,
        FString ParentBranchName,
        TEnumAsByte<DoltResult::Type> &IsSuccess,
        FString &OutMessage) const {

    const TCHAR* TempDir = FGenericPlatformProcess::UserTempDir();

    TMap<UDataTable*, FString> Paths;

    for (UDataTable* DataTable : DataTables) {
        FString TableName = DataTable->GetName();
        FString Path = FPaths::CreateTempFilename(TempDir, *TableName, u".csv");
        Paths.Emplace(DataTable, Path);
        FString DataTableContents = DataTable->GetTableAsCSV(EDataTableExportFlags::None);
        if (!FFileHelper::SaveStringToFile(DataTableContents, *Path)) {
            OutMessage = FString::Printf(TEXT("Failed to save DataTable %s to %s"), *TableName, *Path);
            IsSuccess = DoltResult::Failure;
            return;
        }
    }

    CheckoutExistingBranch(ParentBranchName, IsSuccess, OutMessage);
    if (IsSuccess != DoltResult::Success) {
        return;
    }

    if (BranchName != ParentBranchName) {
        CheckoutNewBranch(BranchName, IsSuccess, OutMessage);
        if (IsSuccess != DoltResult::Success) {
            return;
        }
    }

    for (UDataTable* DataTable : DataTables) {
        FString Path = Paths[DataTable];
        ImportTableToDolt(DataTable->GetName(), Path, IsSuccess, OutMessage);
        if (IsSuccess != DoltResult::Success) {
            return;
        }
    }

    Commit(FString::Printf(TEXT("Imported from Unreal")), IsSuccess, OutMessage);
    if (IsSuccess != DoltResult::Success) {
        return;
    }

    IsSuccess = DoltResult::Success;
    return;
}

void UDoltConnection::ImportDataTables(
        TArray<UDataTable*> DataTables,
        TEnumAsByte<DoltResult::Type> &IsSuccess,
        FString &OutMessage) const {
    const TCHAR* TempDir = FGenericPlatformProcess::UserTempDir();

    for (UDataTable* DataTable : DataTables) {
        FString TableName = DataTable->GetName();
        FString Path = FPaths::CreateTempFilename(TempDir, *TableName, u".csv");

        FString StdOut, StdErr;
        int32 OutReturnCode;
        ExecuteCommand(
            {.StdOut = &StdOut, .StdErr = &StdErr, .OutReturnCode = &OutReturnCode},
          FString::Printf(TEXT("table export %s \"%s\""), *DataTable->GetName(), *Path));

        if (OutReturnCode != 0) {
            OutMessage = TEXT("Failed to export table %s: %s"), *DataTable->GetName(), *StdErr;
            IsSuccess = DoltResult::Failure;
            return;
        }

        FString DataTableContents;
        if (!FFileHelper::LoadFileToString(DataTableContents, *Path)) {
            UE_LOG(LogTemp, Error, TEXT(""));
            OutMessage = FString::Printf(TEXT("Failed to load DataTable from %s"), *Path);
            IsSuccess = DoltResult::Failure;
            return;
        }

        DataTable->EmptyTable();
        DataTable->CreateTableFromCSVString(DataTableContents);
    }

    IsSuccess = DoltResult::Success;
    return;
}

void UDoltConnection::CheckoutNewBranch(
        FString BranchName,
        TEnumAsByte<DoltResult::Type> &IsSuccess,
        FString &OutMessage) const {
    FString StdOut, StdErr;
    int32 OutReturnCode;
    ExecuteCommand(
        {.StdOut = &StdOut, .StdErr = &StdErr, .OutReturnCode = &OutReturnCode},
        FString::Printf(TEXT("checkout -b %s"), *BranchName)
    );

    if (OutReturnCode != 0) {
        OutMessage = FString::Printf(TEXT("Failed to checkout branch %s: %s"), *BranchName, *StdErr);
        IsSuccess = DoltResult::Failure;
    } else {
        OutMessage = FString::Printf(TEXT("Checked out branch %s"), *BranchName);
        UE_LOG(LogTemp, Display, TEXT("Dolt Output: %s"), *StdOut);
        UE_LOG(LogTemp, Display, TEXT("%s"), *OutMessage);
        IsSuccess = DoltResult::Success;
    }
}

void UDoltConnection::CheckoutExistingBranch(
        FString BranchName,
        TEnumAsByte<DoltResult::Type> &IsSuccess,
        FString &OutMessage) const {
    FString StdOut, StdErr;
    int32 OutReturnCode;
    ExecuteCommand(
        {.StdOut = &StdOut, .StdErr = &StdErr, .OutReturnCode = &OutReturnCode},
        FString::Printf(TEXT("checkout %s"), *BranchName)
    );

    if (OutReturnCode != 0) {
        FString *CommandOutput = StdErr.IsEmpty() ? &StdOut : &StdErr;
        OutMessage = FString::Printf(TEXT("Failed to checkout branch %s: %s"), *BranchName, **CommandOutput);
        IsSuccess = DoltResult::Failure;
    } else {
        OutMessage = FString::Printf(TEXT("Checked out branch %s"), *BranchName);
        UE_LOG(LogTemp, Display, TEXT("Dolt Output: %s"), *StdOut);
        UE_LOG(LogTemp, Display, TEXT("%s"), *OutMessage);
        IsSuccess = DoltResult::Success;
    }
}

void UDoltConnection::ImportTableToDolt(
        FString TableName,
        FString FilePath,
        TEnumAsByte<DoltResult::Type> &IsSuccess,
        FString &OutMessage) const {
    FString StdOut, StdErr;
    int32 OutReturnCode;
    ExecuteCommand(
        {.StdOut = &StdOut, .StdErr = &StdErr, .OutReturnCode = &OutReturnCode},
        FString::Printf(TEXT("table import -c -f %s \"%s\""), *TableName, *FilePath)
    );
    if (OutReturnCode != 0) {
        OutMessage = FString::Printf(TEXT("Failed to import table %s from %s: %s"), *TableName, *FilePath, *StdErr);
        IsSuccess = DoltResult::Failure;
    } else {
        OutMessage = FString::Printf(TEXT("Imported table %s from %s"), *TableName, *FilePath);
        UE_LOG(LogTemp, Display, TEXT("Dolt Output: %s"), *StdOut);
        UE_LOG(LogTemp, Display, TEXT("%s"), *OutMessage);
        IsSuccess = DoltResult::Success;
    }
}

void UDoltConnection::Commit(
        FString CommitMessage,
        TEnumAsByte<DoltResult::Type> &IsSuccess,
        FString &OutMessage) const {
    FString StdOut, StdErr;
    int32 OutReturnCode;
    ExecuteCommand(
        {.StdOut = &StdOut, .StdErr = &StdErr, .OutReturnCode = &OutReturnCode},
        FString::Printf(TEXT("commit -A -m \"%s\""), *CommitMessage)
    );
    if (OutReturnCode != 0) {
        OutMessage = FString::Printf(TEXT("Failed to commit: %s"), *StdErr);
        IsSuccess = DoltResult::Failure;
    } else {
        OutMessage = TEXT("Committed: %s"), *CommitMessage;
        UE_LOG(LogTemp, Display, TEXT("Dolt Output: %s"), *StdOut);
        UE_LOG(LogTemp, Display, TEXT("%s"), *OutMessage);
        IsSuccess = DoltResult::Success;
    }
}

void UDoltConnection::Rebase(RebaseArgs Args,
        TEnumAsByte<DoltResult::Type> &IsSuccess,
        FString &OutMessage) const {
    CheckoutExistingBranch(Args.To, IsSuccess, OutMessage);
    if (IsSuccess != DoltResult::Success) {
        return;
    }
    FString StdOut, StdErr;
    int32 OutReturnCode;
    ExecuteCommand(
        {.StdOut = &StdOut, .StdErr = &StdErr, .OutReturnCode = &OutReturnCode},
        FString::Printf(TEXT("rebase %s"), *Args.From)
    );
    if (OutReturnCode != 0) {
        OutMessage = FString::Printf(TEXT("Failed to rebase %s onto %s: %s"), *Args.From, *Args.To, *StdErr);
        IsSuccess = DoltResult::Failure;
    } else {
        OutMessage = FString::Printf(TEXT("Rebased %s onto %s"), *Args.From, *Args.To);
        UE_LOG(LogTemp, Display, TEXT("Dolt Output: %s"), *StdOut);
        UE_LOG(LogTemp, Display, TEXT("%s"), *OutMessage);
        IsSuccess = DoltResult::Success;
    }
}