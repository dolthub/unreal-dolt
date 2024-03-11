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

#include "SourceControlUtils.h"

#include "CoreMinimal.h"
#include "ISourceControlProvider.h"
#include "ISourceControlRevision.h"
#include "ISourceControlModule.h"
#include "HAL/PlatformFileManager.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "SourceControlOperations.h"
#include "SourceControlHelpers.h"
#include "Misc/Paths.h"

#include "./DoltFunctionLibrary.h"

TSharedPtr<ISourceControlRevision, ESPMode::ThreadSafe> GetHeadRevision(ISourceControlState *State) {
    int32 HistorySize = State->GetHistorySize();
    TSharedPtr<ISourceControlRevision, ESPMode::ThreadSafe> HeadRevision;
    for (int32 i = 0; i < HistorySize; i++) {
        TSharedPtr<ISourceControlRevision, ESPMode::ThreadSafe> Revision = State->GetHistoryItem(i);
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
    UE_LOG(LogTemp, Display, TEXT("Loaded Source Control Provider: %s"), *SourceControlProvider.GetName().ToString());
    if (!SourceControlProvider.IsAvailable()) {
        UE_LOG(LogTemp, Error, TEXT("Source Control is not available"));
        return nullptr;
    }

    return &SourceControlProvider;
}

FSourceControlStatePtr GetStateWithHistory(ISourceControlProvider& SourceControlProvider, UPackage *Package) {
    auto UpdateStatusCommand = ISourceControlOperation::Create<FUpdateStatus>();
    UpdateStatusCommand->SetUpdateHistory(true);
    FString Filename = USourceControlHelpers::PackageFilename(Package);

    TArray<FString> InFiles = USourceControlHelpers::AbsoluteFilenames({Filename});

    SourceControlProvider.Execute(UpdateStatusCommand, { Filename });
    auto State = SourceControlProvider.GetState(Package, EStateCacheUsage::ForceUpdate);
    return State;
}

void ForceSync(ISourceControlProvider& SourceControlProvider, UPackage *Package, TEnumAsByte<DoltResult::Type>& IsSuccess, FString &OutMessage) {
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

    FSourceControlStatePtr State = GetStateWithHistory(SourceControlProvider, Package);

	FString Filename = SourceControlHelpers::PackageFilename(Package);

    TArray<FString> InFiles = USourceControlHelpers::AbsoluteFilenames({Filename});

	FSourceControlStatePtr SourceControlState = GetStateWithHistory(SourceControlProvider, Package);

	TSharedPtr<class ISourceControlRevision, ESPMode::ThreadSafe> Revision = GetHeadRevision(SourceControlState.Get());
    if (!Revision.IsValid()) {
        DOLT_FAILF("Failed to get current revision for package %s", *Package->GetName());
    }
	
    TSharedRef<FSync, ESPMode::ThreadSafe> SyncOperation = ISourceControlOperation::Create<FSync>();
	SyncOperation->SetRevision(Revision->GetRevision());
	SyncOperation->SetForce(true);
	if (SourceControlProvider.Execute(SyncOperation, Filename) == ECommandResult::Succeeded)
	{
        PlatformFile.SetReadOnly(*Filename, true);
    }

    DOLT_SUCCEEDF("Synced %s to revision %s.", *Filename, *Revision->GetRevision());
}

UObject* GetUObjectFromRevision(ISourceControlRevision* Revision) {
    FString DownloadedFilePath;
    Revision->Get(DownloadedFilePath, EConcurrency::Synchronous);

    UPackage* CurrentPackage = LoadPackage(nullptr, *DownloadedFilePath, LOAD_ForDiff | LOAD_DisableCompileOnLoad);
    if (!CurrentPackage) {
        UE_LOG(LogTemp, Error, TEXT("Failed to load package from file"));
        return nullptr;
    }
    return CurrentPackage->FindAssetInPackage(RF_NoFlags);
}

FString AbsoluteFilename(FString FileName) {
	if(FPaths::IsRelative(FileName)) {
		FileName = FPaths::ConvertRelativePathToFull(FileName);
    }

	FPaths::NormalizeFilename(FileName);
	
	return FileName;
}
