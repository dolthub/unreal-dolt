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