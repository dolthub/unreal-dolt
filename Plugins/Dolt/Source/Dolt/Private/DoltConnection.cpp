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

#include "DoltConnection.h"
#include "Engine/DataTable.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"

#include "./Result.h"
#include "./DoltFunctionLibrary.h"

#include "CoreMinimal.h"

void UDoltConnection::MaybeInitDolt(FString LocalBranch, FString RemoteBranch) const {
    bool DoltDirExists = FPaths::DirectoryExists(FPaths::Combine(*DoltRepoPath.Path, ".dolt"));
    if (!DoltDirExists) {
        ExecuteCommand("init");
        ExecuteCommand("branch " + LocalBranch);
        ExecuteCommand("branch " + RemoteBranch);
    }
}

void UDoltConnection::ExecuteCommand(FString Args) const {
    FString StdOut, StdErr;
    int32 OutReturnCode;
    bool ProcessExecuted = FPlatformProcess::ExecProcess(*DoltBinPath.FilePath, *Args, &OutReturnCode, &StdOut, &StdErr, *DoltRepoPath.Path);
    if (!ProcessExecuted) {
        UE_LOG(LogTemp, Error, TEXT("Failed to execute command `dolt %s`. Make sure you have configured the Dolt plugin in \"Edit > Project Settings\""), *Args);
    }
    if (OutReturnCode != 0) {
        UE_LOG(LogTemp, Error, TEXT("Dolt Output: \n%s"), *StdOut);
        UE_LOG(LogTemp, Error, TEXT("Dolt Error: \n%s"), *StdErr);
    } else {
        UE_LOG(LogTemp, Display, TEXT("Dolt Output: \n%s"), *StdOut);
    }
}

void UDoltConnection::ExecuteCommand(ExecuteCommandArgs Args, TEnumAsByte<DoltResult::Type> &IsSuccess, FString &OutMessage) const {
    FString StdOut, StdErr;
    int32 OutReturnCode;
    UE_LOG(LogTemp, Display, TEXT("Executing dolt %s"), *Args.Command);

    // bool ProcessExecuted = FPlatformProcess::ExecProcess(*DoltBinPath.FilePath, *Args.Command, &OutReturnCode, &StdOut, &StdErr, *DoltRepoPath.Path);
    void *StdOutPipeRead = nullptr;
    void *StdOutPipeWrite = nullptr;
    bool PipeCreated = FPlatformProcess::CreatePipe(StdOutPipeRead, StdOutPipeWrite, false);
    if (!PipeCreated) {
        DOLT_FAIL("Failed to create pipe for stdout");
    }

    void *StdInPipeRead = nullptr;
    void *StdInPipeWrite = nullptr;
    PipeCreated = FPlatformProcess::CreatePipe(StdInPipeRead, StdInPipeWrite, true);
    if (!PipeCreated) {
        DOLT_FAIL("Failed to create pipe for stdin");
    }

    FProcHandle CommandProc = FPlatformProcess::CreateProc(*DoltBinPath.FilePath, *Args.Command, false, false, false, nullptr, 0, *DoltRepoPath.Path, StdOutPipeWrite);
    /*if (!ProcessExecuted) {
        DOLT_FAILF("Failed to execute command `dolt %s`. Make sure you have configured the Dolt plugin in \"Edit > Project Settings\"", *Args.Command);
    }*/

    FPlatformProcess::ClosePipe(StdInPipeRead, StdInPipeWrite);
    FPlatformProcess::WaitForProc(CommandProc);

    FPlatformProcess::GetProcReturnCode(CommandProc, &OutReturnCode);

    StdOut = FPlatformProcess::ReadPipe(StdOutPipeRead);
    if (OutReturnCode != 0) {
        UE_LOG(LogTemp, Error, TEXT("Dolt Output: \n%s"), *StdOut);
        UE_LOG(LogTemp, Error, TEXT("Dolt Error: \n%s"), *StdErr);
        DOLT_FAILF("%s: %s", *Args.FailureMessage, StdErr.IsEmpty() ? *StdOut : *StdErr);
    } else {
        UE_LOG(LogTemp, Display, TEXT("Dolt Output: \n%s"), *StdOut);
        DOLT_SUCCEED(*Args.SuccessMessage);
    }
}

void UDoltConnection::ExportDataTables(
        TArray<UDataTable*> DataTables,
        FString BranchName,
        FString ParentBranchName,
        FString CommitMessage,
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

    CreateOrResetBranch(BranchName, ParentBranchName, IsSuccess, OutMessage);
    if (IsSuccess != DoltResult::Success) {
        return;
    }

    CheckoutExistingBranch(BranchName, IsSuccess, OutMessage);
    if (IsSuccess != DoltResult::Success) {
        return;
    }

    for (UDataTable* DataTable : DataTables) {
        FString Path = Paths[DataTable];
        ImportTableToDolt(DataTable->GetName(), Path, IsSuccess, OutMessage);
        if (IsSuccess != DoltResult::Success) {
            return;
        }
    }

    Commit(CommitMessage, CommitOptions::SkipEmpty, IsSuccess, OutMessage);
    if (IsSuccess != DoltResult::Success) {
        return;
    }

    IsSuccess = DoltResult::Success;
    return;
}

void UDoltConnection::ImportDataTables(
        TArray<UDataTable*> DataTables,
        FString BranchName,
        TEnumAsByte<DoltResult::Type> &IsSuccess,
        FString &OutMessage) const {
    const TCHAR* TempDir = FGenericPlatformProcess::UserTempDir();

    CheckoutExistingBranch(BranchName, IsSuccess, OutMessage);
    if (IsSuccess != DoltResult::Success) {
        return;
    }

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

void UDoltConnection::HardReset(
        FString TargetBranch,
        TEnumAsByte<DoltResult::Type> &IsSuccess,
        FString &OutMessage) const {
    ExecuteCommand(
        {
            .Command = FString::Printf(TEXT("reset --hard %s"), *TargetBranch),
            .SuccessMessage = FString::Printf(TEXT("hard reset to %s"), *TargetBranch),
            .FailureMessage = FString::Printf(TEXT("failed to hard reset to %s"), *TargetBranch),
        },
        IsSuccess,
        OutMessage
    );
}

void UDoltConnection::CreateOrResetBranch(
        FString NewBranchName,
        FString TargetBranchName,
        TEnumAsByte<DoltResult::Type> &IsSuccess,
        FString &OutMessage) const {
    DOLT_CHECK(NewBranchName.Len() > 0, "Unexpected empty branch name");
    DOLT_CHECK(TargetBranchName.Len() > 0, "Unexpected empty target branch name");
    ExecuteCommand(
        {
            .Command = FString::Printf(TEXT("branch -f %s %s"), *NewBranchName, *TargetBranchName),
            .SuccessMessage = FString::Printf(TEXT("set branch %s to %s"), *NewBranchName, *TargetBranchName),
            .FailureMessage = FString::Printf(TEXT("failed to set branch %s to %s"), *NewBranchName, *TargetBranchName),
        },
        IsSuccess,
        OutMessage
    );
}

void UDoltConnection::CheckoutNewOrExistingBranch(
        FString BranchName,
        TEnumAsByte<DoltResult::Type> &IsSuccess,
        FString &OutMessage) const {
    ExecuteCommand(
        {
            .Command = FString::Printf(TEXT("checkout -B %s"), *BranchName),
            .SuccessMessage = FString::Printf(TEXT("checked out branch %s"), *BranchName),
            .FailureMessage = FString::Printf(TEXT("failed to checkout branch %s"), *BranchName),
        },
        IsSuccess,
        OutMessage
    );
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
            .Command = FString::Printf(TEXT("table import -c -f -pk \"---\" %s \"%s\""), *TableName, *FilePath),
            .SuccessMessage = FString::Printf(TEXT("imported table %s from %s"), *TableName, *FilePath),
            .FailureMessage = FString::Printf(TEXT("failed to import table %s from %s"), *TableName, *FilePath)
        },
        IsSuccess,
        OutMessage
    );
}

void UDoltConnection::Commit(
        FString CommitMessage,
        CommitOptions Options,
        TEnumAsByte<DoltResult::Type> &IsSuccess,
        FString &OutMessage) const {
    bool SkipEmpty = (Options & CommitOptions::SkipEmpty) == CommitOptions::SkipEmpty;
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

void UDoltConnection::Merge(MergeArgs Args,
        TEnumAsByte<DoltResult::Type> &IsSuccess,
        FString &OutMessage) const {
    CheckoutExistingBranch(Args.To, IsSuccess, OutMessage);
    if (IsSuccess != DoltResult::Success) {
        return;
    }
    ExecuteCommand(
        {
            .Command = FString::Printf(TEXT("merge %s"), *Args.From),
            .SuccessMessage = FString::Printf(TEXT("merged %s onto %s"), *Args.From, *Args.To),
            .FailureMessage = FString::Printf(TEXT("failed to merge %s onto %s"), *Args.From, *Args.To)
        },
        IsSuccess,
        OutMessage
    );
}

// This function does not work: Dolt doesn't support non-interactive rebase yet. 
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

bool UDoltConnection::IsMerging(
        TEnumAsByte<DoltResult::Type> &IsSuccess,
        FString &OutMessage) const {
    FString StdOut, StdErr;
    int32 OutReturnCode;
    FString Command = "sql -q \"select is_merging from dolt_merge_status;\" -r json";
    UE_LOG(LogTemp, Display, TEXT("Executing dolt %s"), *Command);
    bool ProcessExecuted = FPlatformProcess::ExecProcess(*DoltBinPath.FilePath, *Command, &OutReturnCode, &StdOut, &StdErr, *DoltRepoPath.Path);
    if (!ProcessExecuted) {
        IsSuccess = DoltResult::Failure;
        OutMessage = FString::Printf(TEXT("Failed to execute command `dolt %s`. Make sure you have configured the Dolt plugin in \"Edit > Project Settings\""), *Command);
        return false;
    }

    if (OutReturnCode != 0) {
        UE_LOG(LogTemp, Error, TEXT("Dolt Output: \n%s"), *StdOut);
        UE_LOG(LogTemp, Error, TEXT("Dolt Error: \n%s"), *StdErr);
        IsSuccess = DoltResult::Failure;
        return false;
    } else {
        UE_LOG(LogTemp, Display, TEXT("Dolt Output: \n%s"), *StdOut);
        return StdOut.Contains("true");
    }
}