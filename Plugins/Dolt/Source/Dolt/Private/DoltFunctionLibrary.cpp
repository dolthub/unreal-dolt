// Fill out your copyright notice in the Description page of Project Settings.


#include "DoltFunctionLibrary.h"

#include "CoreMinimal.h"
#include "HAL/PlatformProcess.h"
#include "ISourceControlModule.h"
#include "ISourceControlProvider.h"
#include "ISourceControlRevision.h"
#include "SourceControlOperations.h"
#include "UObject/ObjectMacros.h"

#include "./DoltConnection.h"
#include "./SourceControlHelpers.h"
#include "./SourceControlUtils.h"

const UDoltSettings* UDoltFunctionLibrary::GetDoltProjectSettings() {
    return GetDefault<UDoltSettings>();
}


void UDoltFunctionLibrary::ExportDataTable(
    const UDoltConnection *Dolt,
    const TArray<UObject*> &DataTableObjects,
    const FString &BranchName,
    TEnumAsByte<DoltResult::Type> &IsSuccess,
    FString &OutMessage)
{
    TArray<UDataTable*> DataTables = GetObjectsOfType<UDataTable>(DataTableObjects);
    if (DataTables.Num() == 0) {
        DOLT_FAIL("No DataTables selected");
    }

    Dolt->ExportDataTables(DataTables, BranchName, BranchName, "Exported from Unreal", IsSuccess, OutMessage);
    if (IsSuccess != DoltResult::Success) {
        return;
    }

    DOLT_SUCCEEDF("Exported %d DataTable(s) to dolt branch %s", DataTables.Num(), *BranchName);
}

void UDoltFunctionLibrary::ImportDataTable(
        const UDoltConnection* Dolt,
        const TArray<UObject*> &DataTableObjects,
        const FString &BranchName,
        TEnumAsByte<DoltResult::Type> &IsSuccess,
        FString &OutMessage) {
    TArray<UDataTable*> DataTables = GetObjectsOfType<UDataTable>(DataTableObjects);
    if (DataTables.Num() == 0) {
        DOLT_FAIL("No DataTables selected");
    }

    Dolt->ImportDataTables(DataTables, BranchName, IsSuccess, OutMessage);
    if (IsSuccess != DoltResult::Success) {
        return;
    }

    DOLT_SUCCEEDF("Imported %d DataTable(s) from dolt branch %s. You still need to save changes.", DataTables.Num(), *BranchName);
}

void UDoltFunctionLibrary::ThreeWayExport(
        const UDoltConnection* Dolt,
        FString LocalBranch,
        FString RemoteBranch,
        const TArray<UObject*> &DataTableObjects,
        TEnumAsByte<DoltResult::Type> &IsSuccess,
        FString &OutMessage) {
    UE_LOG(LogTemp, Display, TEXT("Beginning Three Way Export Operation"));
    TArray<UDataTable*> LocalDataTables = GetObjectsOfType<UDataTable>(DataTableObjects);
    ISourceControlProvider *SourceControlProvider = GetSourceControlProvider();
    if (!SourceControlProvider) {
        DOLT_FAIL("Failed to load Source Control Provider");
    }
    TArray<UDataTable*> RemoteDataTables;
    TArray<UDataTable*> AncestorDataTables;

    for (UDataTable* DataTable : LocalDataTables)
    {
        FString DataTableName = DataTable->GetName();
        UPackage *Package = DataTable->GetPackage();
        
        auto State = GetStateWithHistory(*SourceControlProvider, Package);
        
        UDataTable* CurrentTable = GetObjectFromRevision<UDataTable>(State->GetCurrentRevision());
        if (!CurrentTable) {
            DOLT_FAILF("Failed to load synced revision for %s", *DataTableName);
        }

        AncestorDataTables.Add(CurrentTable);

        UDataTable* HeadTable = GetObjectFromRevision<UDataTable>(GetHeadRevision(State.Get()));
        if (!CurrentTable) {
            DOLT_FAILF("Failed to load most recent revision for %s", *DataTableName);
        }

        RemoteDataTables.Add(HeadTable);
    }

    Dolt->ExportDataTables(AncestorDataTables, RemoteBranch, "HEAD", "Ancestor Commit", IsSuccess, OutMessage);
    if (IsSuccess != DoltResult::Success) {
        return;
    }
    Dolt->ExportDataTables(LocalDataTables, LocalBranch, RemoteBranch, "Local Commit", IsSuccess, OutMessage);
    if (IsSuccess != DoltResult::Success) {
        return;
    }
    Dolt->ExportDataTables(RemoteDataTables, RemoteBranch, RemoteBranch, "Remote Commit", IsSuccess, OutMessage);
    if (IsSuccess != DoltResult::Success) {
        return;
    }
    
    DOLT_SUCCEED("Three Way Export Completed");
}

void UDoltFunctionLibrary::PullRebase(
        const UDoltConnection* Dolt,
        const TArray<UObject*> &DataTableObjects,
        FString LocalBranch,
        FString RemoteBranch,
        TEnumAsByte<DoltResult::Type> &IsSuccess,
        FString &OutMessage) {
    TArray<UDataTable*> DataTables = GetObjectsOfType<UDataTable>(DataTableObjects);
    UE_LOG(LogTemp, Display, TEXT("Beginning Pull Rebase Operation"));
    ISourceControlProvider *SourceControlProvider = GetSourceControlProvider();
    if (!SourceControlProvider) {
        DOLT_FAIL("Failed to load Source Control Provider");
    }

    ThreeWayExport(Dolt, LocalBranch, RemoteBranch, DataTableObjects, IsSuccess, OutMessage);
    if (IsSuccess != DoltResult::Success) {
        return;
    }

    Dolt->Merge({.From = RemoteBranch, .To = LocalBranch}, IsSuccess, OutMessage);
    if (IsSuccess != DoltResult::Success) {
        return;
    }

    for (UObject* DataTable : DataTables)
    {
        ForceSync(*SourceControlProvider, DataTable->GetPackage(), IsSuccess, OutMessage);
        if (IsSuccess != DoltResult::Success) {
            return;
        }
    }

    Dolt->ImportDataTables(DataTables, LocalBranch, IsSuccess, OutMessage);
    if (IsSuccess != DoltResult::Success) {
        return;
    }

    DOLT_SUCCEED("Pull Rebase Completed. You still need to save changes.");
}

void UDoltFunctionLibrary::ResumePullRebase(
        const UDoltConnection* Dolt,
        const TArray<UObject*> &DataTableObjects,
        FString LocalBranch,
        FString RemoteBranch,
        TEnumAsByte<DoltResult::Type> &IsSuccess,
        FString &OutMessage) {
    TArray<UDataTable*> DataTables = GetObjectsOfType<UDataTable>(DataTableObjects);
    ISourceControlProvider *SourceControlProvider = GetSourceControlProvider();
    if (!SourceControlProvider) {
        DOLT_FAIL("Failed to load Source Control Provider");
    }
    
    // If there's a merge in progress, we need to finish it before we can continue
    // Otherwise, we either didn't merge or finished the merge, in which case merging again should be safe.
    bool IsMerging = Dolt->IsMerging(IsSuccess, OutMessage);
    if (IsSuccess != DoltResult::Success) {
        return;
    }

    if (IsMerging) {
        Dolt->Commit("Resolve Merge", CommitOptions::SkipEmpty, IsSuccess, OutMessage);
        if (IsSuccess != DoltResult::Success) {
            return;
        }
    }

    Dolt->Merge({.From = RemoteBranch, .To = LocalBranch}, IsSuccess, OutMessage);
    if (IsSuccess != DoltResult::Success) {
        return;
    }
    
    for (UObject* DataTable : DataTables)
    {
        ForceSync(*SourceControlProvider, DataTable->GetPackage(), IsSuccess, OutMessage);
        if (IsSuccess != DoltResult::Success) {
            return;
        }
    }

    Dolt->ImportDataTables(DataTables, LocalBranch, IsSuccess, OutMessage);
    if (IsSuccess != DoltResult::Success) {
        return;
    }

    DOLT_SUCCEED("Force Sync and Import Completed. You still need to save changes.");
}

void UDoltFunctionLibrary::DoltEcho(const TArray<FString>& Messages) {
    FString Message;
    for (FString Msg : Messages) {
        Message += " " + Msg;
    }
    const UDoltSettings* DoltSettings = GetDefault<UDoltSettings>();
    const UDoltConnection* Dolt = UDoltConnection::ConnectToDolt(DoltSettings->DoltBinPath, DoltSettings->DoltRepoPath);
    TEnumAsByte<DoltResult::Type> IsSuccess;
    FString OutMessage;
    Dolt->ExecuteCommand(
            {
                .Command = Message,
                .SuccessMessage = FString::Printf(TEXT("")),
                .FailureMessage = FString::Printf(TEXT("")),
            },
            IsSuccess,
            OutMessage
        );
}

FAutoConsoleCommand UDoltFunctionLibrary::DoltEchoCommand = FAutoConsoleCommand(
    TEXT("dolt"),
    TEXT("dolt"),
    FConsoleCommandWithArgsDelegate::CreateStatic(&UDoltFunctionLibrary::DoltEcho)
);
