// Copyright (c) 2025 Picasso Labs. All Rights Reserved.
//
// This file is part of NitroPythonRuntime, licensed under the Picasso Labs Software License.

#include "NitroPythonRuntime.h"
#include "Misc/Paths.h"
#include "Logging/LogMacros.h"
#include "Interfaces/IPluginManager.h"

extern "C" PyMODINIT_FUNC PyInit_nitropy(void);

DEFINE_LOG_CATEGORY(NitroPythonLog);

bool FNitroPythonRuntimeModule::ExecutePythonScript(const FString& ScriptCode)
{
    if (!Py_IsInitialized())
    {
        UE_LOG(NitroPythonLog, Error, TEXT("Python is not initialized."));
        return false;
    }

    PyGILState_STATE gilState = PyGILState_Ensure();

    int Result = PyRun_SimpleString(TCHAR_TO_ANSI(*ScriptCode));

    PyGILState_Release(gilState);

    if (Result != 0) // If Python execution failed
    {
        UE_LOG(NitroPythonLog, Error, TEXT("Python script execution failed."));
        return false;
    }
    return true;
}

void FNitroPythonRuntimeModule::InitializePython()
{
    try
    {
        if (!Py_IsInitialized())
        {
            UE_LOG(NitroPythonLog, Log, TEXT("Initializing Python runtime..."));

            PyStatus status;
            PyConfig config;
            PyConfig_InitPythonConfig(&config);

            FString ProgramName = FPaths::ProjectDir();
            status = PyConfig_SetString(&config, &config.program_name, TCHAR_TO_WCHAR(*ProgramName));

            FString PythonHome = FPaths::Combine(FPaths::EngineDir(), TEXT("Binaries/ThirdParty/Python3/Win64"));
            UE_LOG(NitroPythonLog, Log, TEXT("Setting Python home to: %s"), *PythonHome);
            status = PyConfig_SetString(&config, &config.home, TCHAR_TO_WCHAR(*PythonHome));
            if (PyStatus_Exception(status)) goto exception;

            if (PyStatus_Exception(status))
            {
                UE_LOG(NitroPythonLog, Error, TEXT("Failed to set program name."));
                goto exception;
            }

            // Register custom module before initializing Python
            if (PyImport_AppendInittab("nitropy", PyInit_nitropy) != 0)
            {
                UE_LOG(NitroPythonLog, Error, TEXT("Failed to register the 'nitropy' module."));
                goto exception;
            }

            status = Py_InitializeFromConfig(&config);

            if (PyStatus_Exception(status))
            {
                UE_LOG(NitroPythonLog, Error, TEXT("Failed to initialize Python: %s"),
                    *FString(status.err_msg ? UTF8_TO_TCHAR(status.err_msg) : TEXT("Unknown error")));
                goto exception;
            }

            PyConfig_Clear(&config);
            
            // For packaged builds only, ensure thread state is properly initialized
            // by releasing the GIL so background threads can acquire it
            if (FPlatformProperties::RequiresCookedData())
            {
                PyThreadState* save = PyEval_SaveThread();
                UE_LOG(NitroPythonLog, Log, TEXT("Released GIL for background threads in packaged build. Main thread state: %p"), save);
            }
            
            UE_LOG(NitroPythonLog, Log, TEXT("Python initialized successfully."));
            return;

        exception:
            PyConfig_Clear(&config);
            Py_ExitStatusException(status);
        }
    }
    catch (const std::exception& e)
    {
        UE_LOG(NitroPythonLog, Error, TEXT("Nitro Init Python error: %s"), UTF8_TO_TCHAR(e.what()));
    }
}

void FNitroPythonRuntimeModule::FinalizePython()
{
    if (Py_IsInitialized())
    {
        // In packaged builds only, reacquire GIL before finalizing
        if (FPlatformProperties::RequiresCookedData())
        {
            PyGILState_STATE gstate = PyGILState_Ensure();
            UE_LOG(NitroPythonLog, Log, TEXT("Reacquired GIL for finalization in packaged build"));
        }
        
        Py_Finalize();
        UE_LOG(NitroPythonLog, Log, TEXT("Python finalized."));
    }
}

void FNitroPythonRuntimeModule::StartupModule()
{
    InitializePython();
}

void FNitroPythonRuntimeModule::ShutdownModule()
{
    FinalizePython();
}

#undef LOCTEXT_NAMESPACE
IMPLEMENT_MODULE(FNitroPythonRuntimeModule, NitroPythonRuntime)
