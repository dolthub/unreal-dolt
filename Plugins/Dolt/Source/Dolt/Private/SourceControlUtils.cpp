#include "SourceControlUtils.h"

#include "CoreMinimal.h"
#include "ISourceControlProvider.h"
#include "ISourceControlRevision.h"
#include "ISourceControlModule.h"
#include "SourceControlOperations.h"

#include "./SourceControlHelpers.h"

TSharedPtr<ISourceControlRevision, ESPMode::ThreadSafe> GetHeadRevision(ISourceControlState *State) {
    int32 HistorySize = State->GetHistorySize();
    UE_LOG(LogTemp, Error, TEXT("Found %d Revisions"), HistorySize);
    TSharedPtr<ISourceControlRevision, ESPMode::ThreadSafe> HeadRevision;
    for (int32 i = 0; i < HistorySize; i++) {
        TSharedPtr<ISourceControlRevision, ESPMode::ThreadSafe> Revision = State->GetHistoryItem(i);
        UE_LOG(LogTemp, Error, TEXT("Found revision %d"), Revision->GetRevisionNumber());
        if (HeadRevision == nullptr || Revision->GetRevisionNumber() > HeadRevision->GetRevisionNumber()) {
            HeadRevision = Revision;
        }
    }
    return HeadRevision;
}

ISourceControlProvider* GetSourceControlProvider() {
    ISourceControlModule &SourceControlModule = ISourceControlModule::Get();
    if (!SourceControlModule.IsEnabled())
	{
        UE_LOG(LogTemp, Error, TEXT("Source Control is not enabled"));
        return nullptr;
    }

    ISourceControlProvider& SourceControlProvider = SourceControlModule.GetProvider();
    UE_LOG(LogTemp, Error, TEXT("Loaded Source Control Provider: %s"), *SourceControlProvider.GetName().ToString());
    if (!SourceControlProvider.IsAvailable()) {
        UE_LOG(LogTemp, Error, TEXT("Source Control is not available"));
        return nullptr;
    }

    return &SourceControlProvider;
}

TSharedPtr< ISourceControlState, ESPMode::ThreadSafe > GetStateWithHistory(ISourceControlProvider& SourceControlProvider, UPackage *Package) {
    UE_LOG(LogTemp, Error, TEXT("Package Name: %s"), *Package->GetName());
    UE_LOG(LogTemp, Error, TEXT("Package FileName: %s"), *Package->GetLoadedPath().GetLocalFullPath());
    auto UpdateStatusCommand = ISourceControlOperation::Create<FUpdateStatus>();
    UpdateStatusCommand->SetUpdateHistory(true);
    FString Filename = PackageFilename(Package);

    TArray<FString> InFiles = USourceControlHelpers::AbsoluteFilenames({Filename});

    UE_LOG(LogTemp, Error, TEXT("Calling Update on: %s"), *Filename);
    SourceControlProvider.Execute(UpdateStatusCommand, { Filename });
    auto State = SourceControlProvider.GetState(Package, EStateCacheUsage::ForceUpdate);
    UE_LOG(LogTemp, Error, TEXT("Version Control Filename: %s"), *State->GetFilename());
    UE_LOG(LogTemp, Error, TEXT("Version Control Displayname: %s"), *State->GetDisplayName().ToString());
    UE_LOG(LogTemp, Error, TEXT("Version Control IsCurrent: %d"), State->IsCurrent());
    UE_LOG(LogTemp, Error, TEXT("Version Control IsSourceControlled: %d"), State->IsSourceControlled());
    return State;
}

void RevertAndSync(ISourceControlProvider& SourceControlProvider, UPackage *Package, TEnumAsByte<DoltResult::Type>& IsSuccess, FString &OutErrorMessage) {
    auto RevertCommand = ISourceControlOperation::Create<FRevert>();
    auto SyncCommand = ISourceControlOperation::Create<FSync>();
    FString Filename = USourceControlHelpers::PackageFilename(Package);
    TArray<FString> InFiles = USourceControlHelpers::AbsoluteFilenames({Filename});
    SourceControlProvider.Execute(RevertCommand, InFiles);
    SourceControlProvider.Execute(SyncCommand, InFiles);
}

UObject* GetUObjectFromRevision(ISourceControlRevision* Revision) {
    FString DownloadedFilePath;
    Revision->Get(DownloadedFilePath, EConcurrency::Synchronous);
    UE_LOG(LogTemp, Error, TEXT("Current temp filename: %s"), *DownloadedFilePath);

    UPackage* CurrentPackage = LoadPackage(nullptr, *DownloadedFilePath, LOAD_ForDiff | LOAD_DisableCompileOnLoad);
    if (!CurrentPackage) {
        UE_LOG(LogTemp, Error, TEXT("Failed to load package from file"));
        return nullptr;
    }
    UE_LOG(LogTemp, Error, TEXT("Loaded package %s"), *CurrentPackage->GetLoadedPath().GetLocalFullPath());
    return CurrentPackage->FindAssetInPackage(RF_NoFlags);
}