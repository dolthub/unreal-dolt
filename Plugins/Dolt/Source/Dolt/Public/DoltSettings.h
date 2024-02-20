#pragma once

#include "DoltSettings.generated.h"

UCLASS(Config=Plugins, defaultconfig, meta = (DisplayName="Dolt Settings"))
class UDoltSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "General")
	FString DoltBinPath;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "General")
	FString DoltRepoPath;

};