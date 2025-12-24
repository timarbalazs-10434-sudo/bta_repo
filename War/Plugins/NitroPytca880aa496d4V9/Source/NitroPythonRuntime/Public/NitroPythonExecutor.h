// Copyright (c) 2025 Picasso Labs. All Rights Reserved.
//
// This file is part of NitroPythonRuntime, licensed under the Picasso Labs Software License.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NitroPythonTypes.h"
#include "NitroPythonExecutor.generated.h"

/**
 * NitroPythonExecutor - Handles execution of Python scripts from C++
 */
UCLASS()
class NITROPYTHONRUNTIME_API UNitroPythonExecutor : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /** Executes a Python script from a file path and returns the output */
    UFUNCTION(BlueprintCallable, meta = (AutoCreateRefTerm = "InputData", ReturnDisplayName = "Output Data"), Category = "Nitro|Python")
    static FPythonData RunPythonScriptFromFile(const FString& FilePath, const FPythonData& InputData);

    /** Executes a Python script from a string and returns the output */
    UFUNCTION(BlueprintCallable, meta = (AutoCreateRefTerm = "InputData", ReturnDisplayName = "Output Data"), Category = "Nitro|Python")
    static FPythonData RunPythonScriptFromString(const FString& ScriptCode, const FPythonData& InputData);

    /** Helper function to set Python variables and run an additional script */
    static FPythonErrorData SetPythonVariablesAndRunScript(const FPythonData& InputData, const FString& Script);

    /** Helper function to set Python variables, run an additional script, and provide output data from the script */
    static void SetPythonVariablesAndRunScriptWithOutput(const FPythonData& InputData, const FString& Script, FPythonData& OutputData);

    /** Helper function to execute python script and log the terminal output + error trace (if any) */
    static FPythonErrorData ExecuteScriptWithLogging(const FString& CombinedScript);

    /** Dynamically injects the 'nitropy' module into Python if it is missing. */
    static void TryInjectNitropyModuleIfMissing();

    /** Helper function to replace """ with escaped """ in python string definitions */
    static FString EscapeTripleQuotes(const FString& Input);

};
