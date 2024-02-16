
#include "DoltConnection.h"
#include "CoreMinimal.h"

bool FDoltConnection::ExportDataTable(UDataTable* DataTable) {
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
    FString Args = FString::Printf(TEXT("table import -c -f %s \"%s\""), *DataTable->GetName(), *Path);
    FPlatformProcess::ExecProcess(*DoltBinPath, *Args, nullptr, &StdOut, &StdErr, *DoltRepoPath);
    
    if (StdOut.Len() > 0) {
        UE_LOG(LogTemp, Display, TEXT("Dolt Output: %s"), *StdOut);
    }

    if (StdErr.Len() > 0) {
        UE_LOG(LogTemp, Error, TEXT("Dolt Error: %s"), *StdErr);
        return false;
    }

    return true;
}

bool FDoltConnection::ImportDataTable(UDataTable* DataTable) {
    const TCHAR* TempDir = FGenericPlatformProcess::UserTempDir();
    FString Path = FPaths::CreateTempFilename(TempDir, u"Dolt", u".csv");
    UE_LOG(LogTemp, Display, TEXT("Created tmp file at %s"), *Path);
    FString StdOut, StdErr;
    FString Args = FString::Printf(TEXT("table export %s \"%s\""), *DataTable->GetName(), *Path);
    FPlatformProcess::ExecProcess(*DoltBinPath, *Args, nullptr, &StdOut, &StdErr, *DoltRepoPath);

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