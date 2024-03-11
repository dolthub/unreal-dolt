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