#pragma once

#include "CoreMinimal.h"
#include "Result.generated.h"

UENUM(BlueprintType)
namespace DoltResult
{
    enum Type {
        Success UMETA(DisplayName = "Success"),
        Failure UMETA(DisplayName = "Failure")
    };
}
