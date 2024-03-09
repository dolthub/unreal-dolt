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


