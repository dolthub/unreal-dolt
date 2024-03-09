#pragma once

#include <functional>

#include "CoreMinimal.h"

#include "DoltTestCase.generated.h"

using TestType = bool (*)();

USTRUCT(BlueprintType)
struct FDoltTestCase {
    GENERATED_BODY()
public:
    FString Name;
    bool (*TestFunction)();

    FDoltTestCase() : TestFunction(nullptr) {}
    FDoltTestCase(FString Name, bool (*TestFunction)()) : Name(Name), TestFunction(TestFunction) {}

    static FDoltTestCase Create(FString Name, bool (*TestFunction)()) {
        return FDoltTestCase(Name, TestFunction);
    }
};