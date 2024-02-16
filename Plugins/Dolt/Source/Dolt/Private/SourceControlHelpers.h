#pragma once

#include "CoreMinimal.h"

/*
For some reason attempting to link against the SourceControlHelpers module is causing linking errors.
As a temporary workaround I've just copied the relevant functions here.
*/

FString PackageFilename_Internal(const FString& InPackageName);

FString PackageFilename(const FString& InPackageName);

FString PackageFilename(const UPackage* InPackage);

FString AbsoluteFilename(FString FileName );
