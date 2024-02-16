// Fill out your copyright notice in the Description page of Project Settings.


#include "DoltFunctionLibrary.h"
#include "EditorUtilityLibrary.h"
#include "HAL/PlatformProcess.h"
#include "./DoltConnection.h"

bool UDoltFunctionLibrary::ExportDataTable(FString DoltBinPath,FString DoltRepoPath)
{
    FDoltConnection Dolt = FDoltConnection::Connect(DoltBinPath, DoltRepoPath);
    TArray<UObject*> Assets = UEditorUtilityLibrary::GetSelectedAssets();
    for (UObject* Asset : Assets)
    {
        UDataTable* DataTable = Cast<UDataTable>(Asset);
        if (DataTable)
        {
            if (!Dolt.ExportDataTable(DataTable)) {
                return false;
            }
        }
    }
    return true;
}

bool UDoltFunctionLibrary::ImportDataTable(FString DoltBinPath,FString DoltRepoPath) {
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
                UE_LOG(LogTemp, Error, TEXT("Failed to load DataTable from file"));
                return false;
            }
            DataTable->EmptyTable();
            DataTable->CreateTableFromCSVString(DataTableContents);
        }
    }
    return true;
}
