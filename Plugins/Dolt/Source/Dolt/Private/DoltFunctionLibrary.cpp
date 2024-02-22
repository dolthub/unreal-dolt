// Fill out your copyright notice in the Description page of Project Settings.


#include "DoltFunctionLibrary.h"

#include "CoreMinimal.h"
#include "EditorUtilityLibrary.h"
#include "HAL/PlatformProcess.h"
#include "ISourceControlModule.h"
#include "ISourceControlProvider.h"
#include "ISourceControlRevision.h"
#include "SourceControlOperations.h"
#include "EditorAssetLibrary.h"
#include "UObject/ObjectMacros.h"

#include "./DoltConnection.h"
#include "./SourceControlHelpers.h"
#include "./SourceControlUtils.h"

const UDoltSettings* UDoltFunctionLibrary::GetDoltProjectSettings() {
    return GetDefault<UDoltSettings>();
}

void UDoltFunctionLibrary::ExportDataTable(
    const UDoltConnection *Dolt,
    const FString &BranchName,
    TEnumAsByte<DoltResult::Type> &IsSuccess,
    FString &OutMessage)
{
    TArray<UObject*> Assets = UEditorUtilityLibrary::GetSelectedAssets();
    TArray<UDataTable*> DataTables;
    for (UObject* Asset : Assets)
    {
        UDataTable* DataTable = Cast<UDataTable>(Asset);
        if (DataTable)
        {
            DataTables.Add(DataTable);
        }
    }

    if (DataTables.Num() == 0) {
        IsSuccess = DoltResult::Failure;
        OutMessage = TEXT("No DataTables selected");
        return;
    }

    Dolt->ExportDataTables(DataTables, BranchName, BranchName, IsSuccess, OutMessage);
    if (IsSuccess != DoltResult::Success) {
        return;
    }

    IsSuccess = DoltResult::Success;
    return;
}

template <typename T> TArray<T*> GetSelectedAssetsOfType() {
    TArray<UObject*> Assets = UEditorUtilityLibrary::GetSelectedAssets();
    TArray<T*> TypedAssets;
    for (UObject* Asset : Assets)
    {
        T* TypedAsset = Cast<T>(Asset);
        if (TypedAsset)
        {
            TypedAssets.Add(TypedAsset);
        }
    }
    return TypedAssets;
}

void UDoltFunctionLibrary::ImportDataTable(
        const UDoltConnection* Dolt,
        TEnumAsByte<DoltResult::Type> &IsSuccess,
        FString &OutMessage) {
    Dolt->ImportDataTables(GetSelectedAssetsOfType<UDataTable>(), IsSuccess, OutMessage);
}


void UDoltFunctionLibrary::DiffDataTable(const UDoltConnection* Dolt, TEnumAsByte<DoltResult::Type> &IsSuccess, FString &OutMessage) {
    ISourceControlProvider *SourceControlProvider = GetSourceControlProvider();
    if (!SourceControlProvider) {
        UE_LOG(LogTemp, Error, TEXT("Failed to load Source Control Provider"));
        IsSuccess = DoltResult::Failure;
        return;
    }
    TArray<UObject*> Assets = UEditorUtilityLibrary::GetSelectedAssets();
    TArray<UDataTable*> LocalDataTables;
    TArray<UDataTable*> RemoteDataTables;
    TArray<UDataTable*> AncestorDataTables;

    for (UObject* Asset : Assets)
    {
        UDataTable* DataTable = Cast<UDataTable>(Asset);
        if (DataTable)
        {
            FString DataTableName = DataTable->GetName();
            LocalDataTables.Add(DataTable);

            UPackage *Package = DataTable->GetPackage();
            
            auto State = GetStateWithHistory(*SourceControlProvider, Package);
            
            UDataTable* CurrentTable = GetObjectFromRevision<UDataTable>(State->GetCurrentRevision());
            if (!CurrentTable) {
                IsSuccess = DoltResult::Failure;
                OutMessage = FString::Printf(TEXT("Failed to load synced revision for %s"), *DataTableName);
                return;
            }

            AncestorDataTables.Add(CurrentTable);

            UDataTable* HeadTable = GetObjectFromRevision<UDataTable>(GetHeadRevision(State.Get()));
            if (!CurrentTable) {
                IsSuccess = DoltResult::Failure;
                OutMessage = FString::Printf(TEXT("Failed to load most recent revision for %s"), *DataTableName);
                return;
            }

            RemoteDataTables.Add(HeadTable);
        }
    }

    Dolt->ExportDataTables(AncestorDataTables, "remote", "remote", IsSuccess, OutMessage);
    if (IsSuccess != DoltResult::Success) {
        return;
    }
    Dolt->ExportDataTables(LocalDataTables, "local", "remote", IsSuccess, OutMessage);
    if (IsSuccess != DoltResult::Success) {
        return;
    }
    Dolt->ExportDataTables(RemoteDataTables, "remote", "remote", IsSuccess, OutMessage);
    if (IsSuccess != DoltResult::Success) {
        return;
    }
    
    IsSuccess = DoltResult::Success;
    return;
}

void UDoltFunctionLibrary::RebaseOntoHeadRevision(const UDoltConnection* Dolt, TEnumAsByte<DoltResult::Type> &IsSuccess, FString &OutMessage) {
    ISourceControlProvider *SourceControlProvider = GetSourceControlProvider();
    if (!SourceControlProvider) {
        OutMessage = TEXT("Failed to load Source Control Provider");
        IsSuccess = DoltResult::Failure;
        return;
    }

    DiffDataTable(Dolt, IsSuccess, OutMessage);
    if (IsSuccess != DoltResult::Success) {
        return;
    }

    Dolt->Rebase({.From = "remote", .To = "local"}, IsSuccess, OutMessage);
    if (IsSuccess != DoltResult::Success) {
        return;
    }

    TArray<UObject*> Assets = UEditorUtilityLibrary::GetSelectedAssets();
    TArray<UDataTable*> DataTables;
    for (UObject* Asset : Assets)
    {
        UDataTable* DataTable = Cast<UDataTable>(Asset);
        if (DataTable)
        {
            DataTables.Add(DataTable);
        }
    }

    for (UObject* DataTable : DataTables)
    {
        RevertAndSync(*SourceControlProvider, DataTable->GetPackage(), IsSuccess, OutMessage);
        if (IsSuccess != DoltResult::Success) {
            return;
        }
    }

    Dolt->ImportDataTables(DataTables, IsSuccess, OutMessage);
    if (IsSuccess != DoltResult::Success) {
        return;
    }

    IsSuccess = DoltResult::Success;
    return;
}
