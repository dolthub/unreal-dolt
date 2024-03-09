#include "DoltTestHarness.h"

#include "DoltConnection.h"
#include "DoltFunctionLibrary.h"

const TArray<FDoltTestCase> UDoltTestHarness::TestCases = {
    FDoltTestCase::Create("Simple Test", UDoltTestHarness::SampleTest)
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
    FFilePath DoltBinPath = {TEXT("C:/Program Files/Dolt/dolt.exe")};
    FDirectoryPath DoltRepoPath = {TEXT("C:/Users/username/Documents/Unreal Projects/MyProject/DoltRepo")};
    UDoltConnection* Dolt = UDoltConnection::ConnectToDolt(DoltBinPath, DoltRepoPath);
    UDataTable* TestTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/MyProject/MyDataTable"));
    TEnumAsByte<DoltResult::Type> IsSuccess;
    FString OutMessage;
    UDoltFunctionLibrary::PullRebase(Dolt,
        {TestTable},
        "local",
        "remote",
        IsSuccess,
        OutMessage);
    return true;
};

FAutoConsoleCommand UDoltTestHarness::RunAllDoltTestsCommand = FAutoConsoleCommand(
    TEXT("RunAllDoltTests"),
    TEXT("Runs all tests for the Dolt plugin"),
    FConsoleCommandWithArgsDelegate::CreateStatic(&UDoltTestHarness::RunAllTests)
);
