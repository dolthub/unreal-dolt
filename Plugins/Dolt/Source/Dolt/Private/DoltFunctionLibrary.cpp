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

bool UDoltFunctionLibrary::ExportDataTable(FFilePath DoltBinPath, FDirectoryPath DoltRepoPath)
{
    FDoltConnection Dolt = FDoltConnection::Connect(DoltBinPath, DoltRepoPath);
    TArray<UObject*> Assets = UEditorUtilityLibrary::GetSelectedAssets();
    for (UObject* Asset : Assets)
    {
        UDataTable* DataTable = Cast<UDataTable>(Asset);
        if (DataTable)
        {
            if (!Dolt.ExportDataTable(DataTable, "exported", "main")) {
                return false;
            }
        }
    }
    return true;
}

bool UDoltFunctionLibrary::ImportDataTable(FFilePath DoltBinPath, FDirectoryPath DoltRepoPath) {
    FDoltConnection Dolt = FDoltConnection::Connect(DoltBinPath, DoltRepoPath);
    TArray<UObject*> Assets = UEditorUtilityLibrary::GetSelectedAssets();
    for (UObject* Asset : Assets)
    {
        UDataTable* DataTable = Cast<UDataTable>(Asset);
        if (DataTable)
        {
            if (!Dolt.ImportDataTable(DataTable)) {
                return false;
            }
        }
    }
    return true;
}


bool UDoltFunctionLibrary::DiffDataTable(FFilePath DoltBinPath, FDirectoryPath DoltRepoPath) {
    ISourceControlProvider *SourceControlProvider = GetSourceControlProvider();
    if (!SourceControlProvider) {
        UE_LOG(LogTemp, Error, TEXT("Failed to load Source Control Provider"));
        return false;
    }
    FDoltConnection Dolt = FDoltConnection::Connect(DoltBinPath, DoltRepoPath);
    TArray<UObject*> Assets = UEditorUtilityLibrary::GetSelectedAssets();
    for (UObject* Asset : Assets)
    {
        UDataTable* DataTable = Cast<UDataTable>(Asset);
        if (DataTable)
        {
            UPackage *Package = DataTable->GetPackage();
            
            auto State = GetStateWithHistory(*SourceControlProvider, Package);
            
            UDataTable* CurrentTable = GetObjectFromRevision<UDataTable>(State->GetCurrentRevision());
            if (!CurrentTable) {
                UE_LOG(LogTemp, Error, TEXT("Failed to load DataTable from file"));
                return false;
            }

            UDataTable* HeadTable = GetObjectFromRevision<UDataTable>(GetHeadRevision(State.Get()));
            if (!CurrentTable) {
                UE_LOG(LogTemp, Error, TEXT("Failed to load DataTable from file"));
                return false;
            }
            
            // Get Current Branch Name
            FString CurrentBranch;

            Dolt.ExportDataTable(CurrentTable, "remote", CurrentBranch);
            Dolt.ExportDataTable(DataTable, "local", "remote");
            Dolt.ExportDataTable(HeadTable, "remote", "remote");
        }
    }
    return true;
}
