#include "SourceControlHelpers.h"

#include "CoreMinimal.h"

/*
For some reason attempting to link against the SourceControlHelpers module is causing linking errors.
As a temporary workaround I've just copied the relevant functions here.
*/

FString PackageFilename_Internal(const FString& InPackageName) {
	FString Filename = InPackageName;

	// Get the filename by finding it on disk first
	if ( !FPackageName::IsMemoryPackage(InPackageName) && !FPackageName::DoesPackageExist(InPackageName, &Filename) )
	{
		// The package does not exist on disk, see if we can find it in memory and predict the file extension
		// Only do this if the supplied package name is valid
		const bool bIncludeReadOnlyRoots = false;
		if ( FPackageName::IsValidLongPackageName(InPackageName, bIncludeReadOnlyRoots) )
		{
			UPackage* Package = FindPackage(nullptr, *InPackageName);
			// This is a package in memory that has not yet been saved. Determine the extension and convert to a filename, if we do have the package, just assume normal asset extension
			const FString PackageExtension = Package && Package->ContainsMap() ? FPackageName::GetMapPackageExtension() : FPackageName::GetAssetPackageExtension();
			Filename = FPackageName::LongPackageNameToFilename(InPackageName, PackageExtension);
		}
	}

	return Filename;
}

FString PackageFilename(const FString& InPackageName) {
	return FPaths::ConvertRelativePathToFull(PackageFilename_Internal(InPackageName));
}

FString PackageFilename(const UPackage* InPackage) {
	FString Filename;
	if(InPackage != nullptr)
	{
		// Prefer using package loaded path to resolve file name as it properly resolves memory packages
		FString PackageLoadedPath = InPackage->GetLoadedPath().GetPackageName();
		if (!InPackage->GetLoadedPath().IsEmpty() && FPackageName::IsMemoryPackage(PackageLoadedPath))
		{
			const FString PackageExtension = InPackage->ContainsMap() ? FPackageName::GetMapPackageExtension() : FPackageName::GetAssetPackageExtension();
			Filename = FPaths::ConvertRelativePathToFull(FPackageName::LongPackageNameToFilename(PackageLoadedPath, PackageExtension));
		}
		else
		{
			Filename = FPaths::ConvertRelativePathToFull(PackageFilename_Internal(InPackage->GetName()));
		}
	}
	return Filename;
}

FString AbsoluteFilename(FString FileName) {
	if(FPaths::IsRelative(FileName)) {
		FileName = FPaths::ConvertRelativePathToFull(FileName);
    }

	FPaths::NormalizeFilename(FileName);
	
	return FileName;
}
