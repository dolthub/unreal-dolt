
#include "DoltConnection.h"

#include "./Result.h"
#include "./DoltFunctionLibrary.h"

#include "CoreMinimal.h"

void UDoltConnection::ExecuteCommand(ExecuteCommandArgs Args, TEnumAsByte<DoltResult::Type> &IsSuccess, FString &OutMessage) const {
    FString StdOut, StdErr;
    int32 OutReturnCode;
    UE_LOG(LogTemp, Display, TEXT("Executing dolt %s"), *Args.Command);
    bool ProcessExecuted = FPlatformProcess::ExecProcess(*DoltBinPath.FilePath, *Args.Command, &OutReturnCode, &StdOut, &StdErr, *DoltRepoPath.Path);
    if (!ProcessExecuted) {
        DOLT_FAILF("Failed to execute command `dolt %s`. Make sure you have configured the Dolt plugin in \"Edit > Project Settings\"", *Args.Command);
    }

    if (OutReturnCode != 0) {
        UE_LOG(LogTemp, Error, TEXT("Dolt Output: %s"), *StdOut);
        UE_LOG(LogTemp, Error, TEXT("Dolt Error: %s"), *StdErr);
        DOLT_FAILF("%s: %s", *Args.FailureMessage, StdErr.IsEmpty() ? *StdOut : *StdErr);
    } else {
        UE_LOG(LogTemp, Display, TEXT("Dolt Output: %s"), *StdOut);
        DOLT_SUCCEED(*Args.SuccessMessage);
    }
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
            {
                .Command = FString::Printf(TEXT("table export %s \"%s\""), *DataTable->GetName(), *Path),
                .SuccessMessage = FString::Printf(TEXT("exported table %s"), *DataTable->GetName()),
                .FailureMessage = FString::Printf(TEXT("failed to export table %s"), *DataTable->GetName()),
            },
            IsSuccess,
            OutMessage
        );
        if (IsSuccess != DoltResult::Success) {
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
    ExecuteCommand(
        {
            .Command = FString::Printf(TEXT("checkout -b %s"), *BranchName),
            .SuccessMessage = FString::Printf(TEXT("checked out new branch %s"), *BranchName),
            .FailureMessage = FString::Printf(TEXT("failed to checkout new branch %s"), *BranchName),
        },
        IsSuccess,
        OutMessage
    );
}

void UDoltConnection::CheckoutExistingBranch(
        FString BranchName,
        TEnumAsByte<DoltResult::Type> &IsSuccess,
        FString &OutMessage) const {
    ExecuteCommand(
        {
            .Command = FString::Printf(TEXT("checkout %s"), *BranchName),
            .SuccessMessage = FString::Printf(TEXT("checked out existing branch %s"), *BranchName),
            .FailureMessage = FString::Printf(TEXT("failed to checkout existing branch %s"), *BranchName),
        },
        IsSuccess,
        OutMessage
    );
}

void UDoltConnection::ImportTableToDolt(
        FString TableName,
        FString FilePath,
        TEnumAsByte<DoltResult::Type> &IsSuccess,
        FString &OutMessage) const {
    ExecuteCommand(
        {
            .Command = FString::Printf(TEXT("table import -c -f %s \"%s\""), *TableName, *FilePath),
            .SuccessMessage = FString::Printf(TEXT("imported table %s from %s"), *TableName, *FilePath),
            .FailureMessage = FString::Printf(TEXT("failed to import table %s from %s"), *TableName, *FilePath)
        },
        IsSuccess,
        OutMessage
    );
}

void UDoltConnection::Commit(
        FString CommitMessage,
        TEnumAsByte<DoltResult::Type> &IsSuccess,
        FString &OutMessage) const {
    FString StdOut, StdErr;
    bool SkipEmpty = false;
    ExecuteCommand(
        {
            .Command = FString::Printf(TEXT("commit -A %s -m \"%s\""), SkipEmpty ? TEXT("--skip-empty") : TEXT(""), *CommitMessage),
            .SuccessMessage = FString::Printf(TEXT("made commit \"%s\""), *CommitMessage),
            .FailureMessage = FString::Printf(TEXT("failed to make commit \"%s\""), *CommitMessage)
        },
        IsSuccess,
        OutMessage
    );
}
void UDoltConnection::Rebase(RebaseArgs Args,
        TEnumAsByte<DoltResult::Type> &IsSuccess,
        FString &OutMessage) const {
    CheckoutExistingBranch(Args.To, IsSuccess, OutMessage);
    if (IsSuccess != DoltResult::Success) {
        return;
    }
    ExecuteCommand(
        {
            .Command = FString::Printf(TEXT("rebase %s"), *Args.From),
            .SuccessMessage = FString::Printf(TEXT("rebased %s onto %s"), *Args.From, *Args.To),
            .FailureMessage = FString::Printf(TEXT("failed to rebase %s onto %s"), *Args.From, *Args.To)
        },
        IsSuccess,
        OutMessage
    );
}