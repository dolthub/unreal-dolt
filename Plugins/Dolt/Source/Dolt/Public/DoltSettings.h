#pragma once

#include "Engine/EngineTypes.h"
#include "DoltSettings.generated.h"

UCLASS(Config=Editor, defaultconfig, meta = (DisplayName="Dolt Settings"))
class UDoltSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "General")
	FFilePath DoltBinPath;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "General")
	FDirectoryPath DoltRepoPath;

};