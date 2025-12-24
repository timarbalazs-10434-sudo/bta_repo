// Copyright (c) 2025 Picasso Labs. All Rights Reserved.
//
// This file is part of NitroPythonRuntime, licensed under the Picasso Labs Software License.

#pragma once

#define _Py_USE_GCC_BUILTIN_ATOMICS 0
#define __STDC_VERSION__ 199901L  // Simulate a C standard version
#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "NitroPythonTypes.h"
#include "HAL/RunnableThread.h"
#include "HAL/Runnable.h"
#include "NitroPythonBlueprintNode.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPythonDataReceived, FPythonData, OutputData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPythonErrorReceived, FPythonData, OutputData);
DECLARE_DELEGATE(FThreadDelegate);
UNitroPythonBlueprintNode* NodeObjCopy = nullptr;

/**
 * Async Latent Blueprint Node for running Python scripts in Unreal Engine
 */
UCLASS()
class NITROPYTHONRUNTIME_API UNitroPythonBlueprintNode : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

private:
    FString ThreadScript;
    FPythonData ThreadInputData;

public:
    /** Runs a Python script from a file path asynchronously */
    UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", AutoCreateRefTerm = "InputData"), Category = "Nitro|Python")
    static UNitroPythonBlueprintNode* RunPythonScriptFromFileAsync(UObject* WorldContextObject, const FString& FilePath, const FPythonData& InputData);

    /** Runs a Python script from a string asynchronously */
    UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", AutoCreateRefTerm = "InputData"), Category = "Nitro|Python")
    static UNitroPythonBlueprintNode* RunPythonScriptFromStringAsync(UObject* WorldContextObject, const FString& ScriptCode, const FPythonData& InputData);

    UFUNCTION()
    void NitroOutputData();

    FThreadDelegate PythonThreadDelegate;

    void execIfBound();

    /** Delegate that fires when Python sends output */
    UPROPERTY(BlueprintAssignable)
    FOnPythonDataReceived OnPythonOutputReceived;

    /** Delegate that fires when Python sends error */
    UPROPERTY(BlueprintAssignable, Category = "Nitro|Python", meta = (DisplayName = "On Error"))
    FOnPythonErrorReceived OnPythonErrorReceived;

    virtual void Activate() override;
};

class NITROPYTHONRUNTIME_API FNitroRunnable : public FRunnable
{
private:
	FRunnableThread* NitroRunnableThread;
	FString RunnableScript;
    FPythonData RunnableInput;
    TWeakObjectPtr<UNitroPythonBlueprintNode> OwningNode;

public:
    FNitroRunnable(const FString& PythonScript, const FPythonData& InputData, UNitroPythonBlueprintNode* InNode);
	virtual ~FNitroRunnable() override;

	virtual bool   Init() override;
	virtual uint32 Run()  override;
	virtual void   Exit() override;
	virtual void   Stop() override;
};
