// Fill out your copyright notice in the Description page of Project Settings.


#include "DoltFunctionLibrary.h"
#include "EditorUtilityLibrary.h"
#include "HAL/PlatformProcess.h"

bool UDoltFunctionLibrary::ExportDataTable(FString DoltBinPath,FString DoltRepoPath)
{
    TArray<UObject*> Assets = UEditorUtilityLibrary::GetSelectedAssets();
    for (UObject* Asset : Assets)
    {
        UDataTable* DataTable = Cast<UDataTable>(Asset);
        if (DataTable)
        {
            const TCHAR* TempDir = FGenericPlatformProcess::UserTempDir();
            FString Path = FPaths::CreateTempFilename(TempDir, u"Dolt", u".csv");
            FString DataTableContents = DataTable->GetTableAsCSV(EDataTableExportFlags::None);
            if (!FFileHelper::SaveStringToFile(DataTableContents, *Path)) {
                UE_LOG(LogTemp, Error, TEXT("Failed to save DataTable to file"));
                return false;
            }
            UE_LOG(LogTemp, Display, TEXT("Saved to %s"), *Path);
            IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

            IFileHandle* FileHandle = PlatformFile.OpenRead(*Path);
            FString StdOut, StdErr;
            FString Args = FString::Printf(TEXT("table import -c %s \"%s\""), *DataTable->GetName(), *Path);
            FPlatformProcess::ExecProcess(*DoltBinPath, *Args, nullptr, &StdOut, &StdErr, *DoltRepoPath);
            
            if (StdErr.Len() > 0) {
                UE_LOG(LogTemp, Error, TEXT("Dolt Error: %s"), *StdErr);
                return false;
            }

            if (StdOut.Len() > 0) {
                UE_LOG(LogTemp, Display, TEXT("Dolt Output: %s"), *StdOut);
            }

        }
    }
    return true;
}