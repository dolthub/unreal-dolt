#pragma once

#include "Engine/DeveloperSettings.h"
#include "Engine/EngineTypes.h"
#include "DoltSettings.generated.h"

UCLASS(Config=Editor, defaultconfig, meta = (DisplayName="Dolt Settings"))
class UDoltSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Dolt General")
	FFilePath DoltBinPath;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Dolt General")
	FDirectoryPath DoltRepoPath;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Dolt General")
	FString LocalBranchName;

    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Dolt General")
	FString RemoteBranchName;
};