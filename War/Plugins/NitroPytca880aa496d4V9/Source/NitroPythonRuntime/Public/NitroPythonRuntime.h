// Copyright (c) 2025 Picasso Labs. All Rights Reserved.
// 
// This file is part of NitroPythonRuntime, licensed under the Picasso Labs Software License. 

#pragma once

#define _Py_USE_GCC_BUILTIN_ATOMICS 0
#define __STDC_VERSION__ 199901L  // Simulate a C standard version
#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(NitroPythonLog, Log, All);

class FNitroPythonRuntimeModule : public IModuleInterface
{
public:
    /** IModuleInterface implementation */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
    
    /** Initializes the Python runtime */
    static void InitializePython();
    
    /** Finalizes the Python runtime */
    static void FinalizePython();
    
    /** Executes a Python script string using the embedded Python runtime */
    static bool ExecutePythonScript(const FString& ScriptCode);
    
};
