#include "DoltTestHarness.h"

#include "DoltConnection.h"
#include "DoltFunctionLibrary.h"

const TArray<FDoltTestCase> UDoltTestHarness::TestCases = {
    FDoltTestCase::Create("Simple Test", UDoltTestHarness::SampleTest),
    FDoltTestCase::Create("Test Rebase Onto Head", UDoltTestHarness::TestRebaseOntoHead)
};

void UDoltTestHarness::RunAllTests(const TArray<FString>& Messages) {
    UE_LOG(LogTemp, Display, TEXT("Running all tests"));
    bool AllTestsPassed = true;
    for (const FDoltTestCase& Test : TestCases) {
        UE_LOG(LogTemp, Display, TEXT("Running Test: %s"), *Test.Name);
        if (!RunTest(Test)) {
            UE_LOG(LogTemp, Error, TEXT("Test failed: %s"), *Test.Name);
            AllTestsPassed = false;
        }
    }
    if (AllTestsPassed) {
        UE_LOG(LogTemp, Display, TEXT("All tests passed"));
    }
}

bool UDoltTestHarness::RunTest(const FDoltTestCase Test) {
    return Test.TestFunction();
}

bool UDoltTestHarness::TestRebaseOntoHead() {
    const UDoltSettings *Settings = UDoltFunctionLibrary::GetDoltProjectSettings();
    UDoltConnection* Dolt = UDoltConnection::ConnectToDolt(Settings->DoltBinPath, Settings->DoltRepoPath);
    UDataTable* TestTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/MyProject/MyDataTable"));
    TEnumAsByte<DoltResult::Type> IsSuccess;
    FString OutMessage;
    UDoltFunctionLibrary::PullRebase(Dolt,
        {TestTable},
        Settings->LocalBranchName,
        Settings->RemoteBranchName,
        IsSuccess,
        OutMessage);
    return true;
};

FAutoConsoleCommand UDoltTestHarness::RunAllDoltTestsCommand = FAutoConsoleCommand(
    TEXT("RunAllDoltTests"),
    TEXT("Runs all tests for the Dolt plugin"),
    FConsoleCommandWithArgsDelegate::CreateStatic(&UDoltTestHarness::RunAllTests)
);
