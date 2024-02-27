#pragma once

#include "CoreMinimal.h"
#include "ISourceControlProvider.h"
#include "ISourceControlRevision.h"

#include "./Result.h"

using FSourceControlRevisionPtr = TSharedPtr<ISourceControlRevision, ESPMode::ThreadSafe>;

TSharedPtr<ISourceControlRevision, ESPMode::ThreadSafe> GetHeadRevision(ISourceControlState* State);

ISourceControlProvider* GetSourceControlProvider();

TSharedPtr< ISourceControlState, ESPMode::ThreadSafe > GetStateWithHistory(ISourceControlProvider& SourceControlProvider, UPackage* Package);

UObject* GetUObjectFromRevision(ISourceControlRevision* Revision);

template <typename T>
T* GetObjectFromRevision(FSourceControlRevisionPtr Revision) {
    if (!Revision.IsValid()) {
        UE_LOG(LogTemp, Error, TEXT("Revision is not valid"));
        return nullptr;
    }

    UObject* Object = GetUObjectFromRevision(Revision.Get());
    if (!Object) {
        UE_LOG(LogTemp, Error, TEXT("Failed to load UObject for revision %s"), *Revision->GetRevision());
        return nullptr;
    }

    T* CastObject = Cast<T>(Object);
    if (!CastObject) {
        UE_LOG(LogTemp, Error, TEXT("Object loaded from file is not %s"), *T::StaticClass()->GetName());
        return nullptr;
    }
    
    return CastObject;
}

FString AbsoluteFilename(FString FileName);

void ForceSync(ISourceControlProvider& SourceControlProvider, UPackage *Package, TEnumAsByte<DoltResult::Type>& IsSuccess, FString &OutErrorMessage);
