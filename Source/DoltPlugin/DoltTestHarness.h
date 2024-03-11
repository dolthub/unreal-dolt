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

#include "CoreMinimal.h"

#include <functional>

#include "./DoltTestCase.h"

#include "DoltTestHarness.generated.h"

static const TArray<FDoltTestCase> TestCases;

UCLASS()
class UDoltTestHarness : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dolt")
	static const TArray<FDoltTestCase> GetTestCases() {
        return TestCases;
    }

    UFUNCTION(Exec, BlueprintCallable, Category = "Dolt")
    static void RunAllTests(const TArray<FString>& Messages);

    UFUNCTION(BlueprintCallable, Category = "Dolt")
    static bool RunTest(const FDoltTestCase Test);

    static bool SampleTest() {
        return true;
    }

    static bool TestRebaseOntoHead();

    static const TArray<FDoltTestCase> TestCases;

    static FAutoConsoleCommand RunAllDoltTestsCommand;
};


