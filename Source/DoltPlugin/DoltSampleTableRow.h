#pragma once
// Header
#include "CoreMinimal.h"
#include "Engine/DataTable.h"

#include "DoltSampleTableRow.generated.h"

USTRUCT(BlueprintType)
struct DOLT_API FDoltSampleTableRow : public FTableRowBase
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Type;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 AttackPower;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 DefensePower;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> Abilities;
};