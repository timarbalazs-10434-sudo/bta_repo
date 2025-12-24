// Copyright (c) 2025 Picasso Labs. All Rights Reserved.
//
// This file is part of NitroPythonRuntime, licensed under the Picasso Labs Software License.

#include "NitroPythonExecutor.h"
#include "NitroPythonRuntime.h"
#include "NitroPythonBlueprintNode.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"
#include "Interfaces/IPluginManager.h"

extern "C" PyMODINIT_FUNC PyInit_nitropy(void);


void UNitroPythonExecutor::SetPythonVariablesAndRunScriptWithOutput(const FPythonData& InputData, const FString& Script, FPythonData& OutputData)
{

    if (!Py_IsInitialized())
    {
        UE_LOG(NitroPythonLog, Error, TEXT("Python is not initialized."));
        return;
    }

    UE_LOG(NitroPythonLog, Log, TEXT("Start acquire GIL"));

    // **Ensure thread state is set correctly**
    PyGILState_STATE gstate = PyGILState_Ensure();

    UE_LOG(NitroPythonLog, Log, TEXT("Acquired GIL"));

    FString CombinedScript;

    // Set Bools
    for (const auto& Entry : InputData.Bools)
    {
        CombinedScript += FString::Printf(TEXT("%s = %s\n"), *Entry.Key, Entry.Value ? TEXT("True") : TEXT("False"));
    }

    // Set Strings
    for (const auto& Entry : InputData.Strings)
    {
        CombinedScript += FString::Printf(TEXT("%s = %s\n"), *Entry.Key, *UNitroPythonExecutor::EscapeTripleQuotes(Entry.Value));
    }

    // Set Ints
    for (const auto& Entry : InputData.Ints)
    {
        CombinedScript += FString::Printf(TEXT("%s = %d\n"), *Entry.Key, Entry.Value);
    }

    // Set Floats
    for (const auto& Entry : InputData.Floats)
    {
        CombinedScript += FString::Printf(TEXT("%s = %f\n"), *Entry.Key, Entry.Value);
    }

    // Set Bool Arrays
    for (const auto& Entry : InputData.BoolArrays)
    {
        CombinedScript += FString::Printf(TEXT("%s = [%s]\n"), *Entry.Key,
            *FString::JoinBy(Entry.Value.BoolArray, TEXT(", "), [](bool Value) { return Value ? TEXT("True") : TEXT("False"); }));
    }

    // Set String Arrays
    for (const auto& Entry : InputData.StringArrays)
    {
        CombinedScript += FString::Printf(TEXT("%s = [%s]\n"), *Entry.Key,
            *FString::JoinBy(Entry.Value.StringArray, TEXT(", "),
                [](const FString& Value) { return UNitroPythonExecutor::EscapeTripleQuotes(Value); }));
    }

    // Set Int Arrays
    for (const auto& Entry : InputData.IntArrays)
    {
        CombinedScript += FString::Printf(TEXT("%s = [%s]\n"), *Entry.Key,
            *FString::JoinBy(Entry.Value.IntArray, TEXT(", "), [](int32 Value) { return FString::Printf(TEXT("%d"), Value); }));
    }

    // Set Float Arrays
    for (const auto& Entry : InputData.FloatArrays)
    {
        CombinedScript += FString::Printf(TEXT("%s = [%s]\n"), *Entry.Key,
            *FString::JoinBy(Entry.Value.FloatArray, TEXT(", "), [](float Value) { return FString::Printf(TEXT("%f"), Value); }));
    }

    // Append the provided script
    CombinedScript += TEXT("\n") + Script;

    UE_LOG(NitroPythonLog, Log, TEXT("Executing script:\n%s"), *CombinedScript);

    FPythonErrorData ErrorData = UNitroPythonExecutor::ExecuteScriptWithLogging(CombinedScript);

    UE_LOG(NitroPythonLog, Log, TEXT("Finished execution"));

    // Retrieve global variables from Python
    PyObject* MainModule = PyImport_AddModule("__main__");
    PyObject* GlobalDict = PyModule_GetDict(MainModule);

    if (!GlobalDict || !PyDict_Check(GlobalDict))
    {
        UE_LOG(NitroPythonLog, Error, TEXT("Failed to retrieve Python global dictionary."));
        OutputData.Bools.Add(TEXT("nitro_is_error"), ErrorData.IsError);
        OutputData.Strings.Add(TEXT("nitro_error_message"), ErrorData.ErrorMessage);
        PyGILState_Release(gstate);
        return;
    }
    
    // Extract the 'output' dictionary from the global namespace
    PyObject* PyOutput = PyDict_GetItemString(GlobalDict, "output");
    // If no output defined in script, just return
    if (!PyOutput || !PyDict_Check(PyOutput))
    {
        OutputData.Bools.Add(TEXT("nitro_is_error"), ErrorData.IsError);
        OutputData.Strings.Add(TEXT("nitro_error_message"), ErrorData.ErrorMessage);
        PyGILState_Release(gstate);
        return;
    }
        
    // Iterate through the 'output' dictionary
    PyObject *Key, *Value;
    Py_ssize_t Pos = 0;
    while (PyDict_Next(PyOutput, &Pos, &Key, &Value))
    {
        FString KeyStr = FString(UTF8_TO_TCHAR(PyUnicode_AsUTF8(Key)));
    
        if (PyBool_Check(Value))
        {
            OutputData.Bools.Add(KeyStr, static_cast<bool>(PyObject_IsTrue(Value)));
        }
        else if (PyLong_Check(Value))
        {
            OutputData.Ints.Add(KeyStr, static_cast<int32>(PyLong_AsLong(Value)));
        }
        else if (PyFloat_Check(Value))
        {
            OutputData.Floats.Add(KeyStr, static_cast<float>(PyFloat_AsDouble(Value)));
        }
        else if (PyUnicode_Check(Value))
        {
            OutputData.Strings.Add(KeyStr, FString(UTF8_TO_TCHAR(PyUnicode_AsUTF8(Value))));
        }
        else if (PyList_Check(Value))
        {
            // Handling list types
            TArray<FString> StringArray;
            TArray<bool> BoolArray;
            TArray<int32> IntArray;
            TArray<float> FloatArray;
    
            bool bIsStringArray = false, bIsIntArray = false, bIsFloatArray = false, bIsBoolArray = false;
    
            Py_ssize_t ListSize = PyList_Size(Value);
            for (Py_ssize_t i = 0; i < ListSize; ++i)
            {
                PyObject* Entry = PyList_GetItem(Value, i); // Borrowed reference
                if (PyUnicode_Check(Entry))
                {
                    bIsStringArray = true;
                    StringArray.Add(FString(UTF8_TO_TCHAR(PyUnicode_AsUTF8(Entry))));
                }
                else if (PyBool_Check(Entry))
                {
                    bIsBoolArray = true;
                    BoolArray.Add(static_cast<bool>(PyObject_IsTrue(Entry)));
                }
                else if (PyLong_Check(Entry))
                {
                    bIsIntArray = true;
                    IntArray.Add(static_cast<int32>(PyLong_AsLong(Entry)));
                }
                else if (PyFloat_Check(Entry))
                {
                    bIsFloatArray = true;
                    FloatArray.Add(static_cast<float>(PyFloat_AsDouble(Entry)));
                }
            }
    
            if (bIsStringArray) OutputData.StringArrays.Add(KeyStr, {StringArray});
            if (bIsBoolArray) OutputData.BoolArrays.Add(KeyStr, {BoolArray});
            if (bIsIntArray) OutputData.IntArrays.Add(KeyStr, {IntArray});
            if (bIsFloatArray) OutputData.FloatArrays.Add(KeyStr, {FloatArray});
        }
    }

    OutputData.Bools.Add(TEXT("nitro_is_error"), ErrorData.IsError);
    OutputData.Strings.Add(TEXT("nitro_error_message"), ErrorData.ErrorMessage);

    PyGILState_Release(gstate);  // Release GIL and restore previous state
    UE_LOG(NitroPythonLog, Log, TEXT("Released GIL"));
}

FPythonErrorData UNitroPythonExecutor::SetPythonVariablesAndRunScript(const FPythonData& InputData, const FString& Script)
{
    FPythonErrorData ErrorData;
    ErrorData.IsError = false;
    ErrorData.ErrorMessage = TEXT("");

    if (!Py_IsInitialized())
    {
        ErrorData.IsError = true;
        ErrorData.ErrorMessage = TEXT("Python interpreter is not initialized.");
        UE_LOG(NitroPythonLog, Error, TEXT("Python is not initialized."));
        return ErrorData;
    }

    UE_LOG(NitroPythonLog, Log, TEXT("Start acquire GIL"));

    // **Acquire GIL using PyGILState_Ensure**
    PyGILState_STATE gstate = PyGILState_Ensure();

    UE_LOG(NitroPythonLog, Log, TEXT("Acquired GIL"));

    FString CombinedScript;

    // Set Bools
    for (const auto& Entry : InputData.Bools)
    {
        CombinedScript += FString::Printf(TEXT("%s = %s\n"), *Entry.Key, Entry.Value ? TEXT("True") : TEXT("False"));
    }

    // Set Strings
    for (const auto& Entry : InputData.Strings)
    {
        CombinedScript += FString::Printf(TEXT("%s = %s\n"), *Entry.Key, *UNitroPythonExecutor::EscapeTripleQuotes(Entry.Value));
    }

    // Set Ints
    for (const auto& Entry : InputData.Ints)
    {
        CombinedScript += FString::Printf(TEXT("%s = %d\n"), *Entry.Key, Entry.Value);
    }

    // Set Floats
    for (const auto& Entry : InputData.Floats)
    {
        CombinedScript += FString::Printf(TEXT("%s = %f\n"), *Entry.Key, Entry.Value);
    }

    // Set Bool Arrays
    for (const auto& Entry : InputData.BoolArrays)
    {
        CombinedScript += FString::Printf(TEXT("%s = [%s]\n"), *Entry.Key,
            *FString::JoinBy(Entry.Value.BoolArray, TEXT(", "), [](bool Value) { return Value ? TEXT("True") : TEXT("False"); }));
    }

    // Set String Arrays
    for (const auto& Entry : InputData.StringArrays)
    {
        CombinedScript += FString::Printf(TEXT("%s = [%s]\n"), *Entry.Key,
            *FString::JoinBy(Entry.Value.StringArray, TEXT(", "),
                [](const FString& Value) { return UNitroPythonExecutor::EscapeTripleQuotes(Value); }));
    }

    // Set Int Arrays
    for (const auto& Entry : InputData.IntArrays)
    {
        CombinedScript += FString::Printf(TEXT("%s = [%s]\n"), *Entry.Key,
            *FString::JoinBy(Entry.Value.IntArray, TEXT(", "), [](int32 Value) { return FString::Printf(TEXT("%d"), Value); }));
    }

    // Set Float Arrays
    for (const auto& Entry : InputData.FloatArrays)
    {
        CombinedScript += FString::Printf(TEXT("%s = [%s]\n"), *Entry.Key,
            *FString::JoinBy(Entry.Value.FloatArray, TEXT(", "), [](float Value) { return FString::Printf(TEXT("%f"), Value); }));
    }

    // Append the provided script
    CombinedScript += TEXT("\n") + Script;

    UE_LOG(NitroPythonLog, Log, TEXT("Executing script:\n%s"), *CombinedScript);

    ErrorData = UNitroPythonExecutor::ExecuteScriptWithLogging(CombinedScript);

    UE_LOG(NitroPythonLog, Log, TEXT("Finished execution"));

    // Release the GIL properly
    PyGILState_Release(gstate);  // releases GIL and restores previous thread state
    UE_LOG(NitroPythonLog, Log, TEXT("Released GIL"));

    return ErrorData;
}

FPythonErrorData UNitroPythonExecutor::ExecuteScriptWithLogging(const FString& CombinedScript)
{
    FPythonErrorData ErrorData;
    ErrorData.IsError = false;
    ErrorData.ErrorMessage = TEXT("");

    // Attempt to inject the nitropy module if missing
    UNitroPythonExecutor::TryInjectNitropyModuleIfMissing();

    // Redirect sys.stdout and sys.stderr to capture output and errors
    PyObject* sysModule = PyImport_ImportModule("sys");
    if (sysModule)
    {
        PyObject* stringIO = PyImport_ImportModule("io");
        if (stringIO)
        {
            PyObject* stringBufferOut = PyObject_CallMethod(stringIO, "StringIO", nullptr);
            PyObject* stringBufferErr = PyObject_CallMethod(stringIO, "StringIO", nullptr);

            if (stringBufferOut && stringBufferErr)
            {
                PyObject_SetAttrString(sysModule, "stdout", stringBufferOut);
                PyObject_SetAttrString(sysModule, "stderr", stringBufferErr);

                FString PluginBaseDir = IPluginManager::Get().FindPlugin(TEXT("NitroPythonRuntime"))->GetBaseDir();

                // ADD DLLs/ TO PATH
                FString DllDir = FPaths::ConvertRelativePathToFull(
                    FPaths::Combine(FPaths::EngineDir(), TEXT("Binaries/ThirdParty/Python3/Win64/DLLs"))
                );
                DllDir.ReplaceInline(TEXT("/"), TEXT("\\"));
                
                FString CurrentPath = FPlatformMisc::GetEnvironmentVariable(TEXT("PATH"));
                FString NewPath = DllDir + TEXT(";") + CurrentPath;
                FPlatformMisc::SetEnvironmentVar(TEXT("PATH"), *NewPath);

                // ADD SITE PACKAGES TO sys.path
                FString SitePackagesPath = FPaths::ConvertRelativePathToFull(
                    FPaths::Combine(FPaths::EngineDir(), TEXT("Binaries/ThirdParty/Python3/Win64/Lib/site-packages"))
                );
                SitePackagesPath.ReplaceInline(TEXT("/"), TEXT("\\"));
                
                FString SitePackagesInjectScript = FString::Printf(TEXT(
                    "import sys\n"
                    "if r\"%s\" not in sys.path:\n"
                    "    sys.path.append(r\"%s\")\n"
                ), *SitePackagesPath, *SitePackagesPath);
                
                FString FinalScript = SitePackagesInjectScript + LINE_TERMINATOR + CombinedScript;
                
                int Result = PyRun_SimpleString(TCHAR_TO_ANSI(*FinalScript));

                // Retrieve the standard output (print statements)
                PyObject* output = PyObject_CallMethod(stringBufferOut, "getvalue", nullptr);
                if (output)
                {
                    FString OutputDetails = UTF8_TO_TCHAR(PyUnicode_AsUTF8(output));
                    if (!OutputDetails.IsEmpty())
                    {
                        UE_LOG(NitroPythonLog, Log, TEXT("Python Output: \n%s"), *OutputDetails);
                    }
                    Py_DECREF(output);
                }
                else
                {
                    UE_LOG(NitroPythonLog, Warning, TEXT("Failed to retrieve standard output from StringIO buffer."));
                }

                // Retrieve the error output (exceptions, tracebacks)
                PyObject* errorOutput = PyObject_CallMethod(stringBufferErr, "getvalue", nullptr);
                if (errorOutput)
                {
                    FString ErrorDetails = UTF8_TO_TCHAR(PyUnicode_AsUTF8(errorOutput));
                    if (!ErrorDetails.IsEmpty())
                    {
                        UE_LOG(NitroPythonLog, Error, TEXT("Python Error Output: \n%s"), *ErrorDetails);
                        ErrorData.IsError = true;
                        ErrorData.ErrorMessage = ErrorDetails;
                    }
                    Py_DECREF(errorOutput);
                }
                else
                {
                    UE_LOG(NitroPythonLog, Warning, TEXT("Failed to retrieve error output from StringIO buffer."));
                }

                Py_DECREF(stringBufferOut);
                Py_DECREF(stringBufferErr);
            }
            else
            {
                UE_LOG(NitroPythonLog, Error, TEXT("Failed to create StringIO buffers for stdout and stderr."));
            }
            Py_DECREF(stringIO);
        }
        else
        {
            UE_LOG(NitroPythonLog, Error, TEXT("Failed to import Python 'io' module. Ensure the Python environment includes the standard library."));
        }
        Py_DECREF(sysModule);
    }
    else
    {
        UE_LOG(NitroPythonLog, Error, TEXT("Failed to import Python 'sys' module. Python environment might be corrupted or incomplete."));
    }

    return ErrorData;
}

void UNitroPythonExecutor::TryInjectNitropyModuleIfMissing()
{
    PyObject* sysModules = PyImport_GetModuleDict();
    if (!PyMapping_HasKeyString(sysModules, "nitropy"))
    {
        UE_LOG(NitroPythonLog, Log, TEXT("Adding 'nitropy' module dynamically."));

        // Initialize the nitropy module
        PyObject* NitroModuleObj = PyInit_nitropy();
        if (NitroModuleObj)
        {
            if (PyDict_SetItemString(sysModules, "nitropy", NitroModuleObj) == 0)
            {
                UE_LOG(NitroPythonLog, Log, TEXT("'nitropy' successfully added into sys.modules."));
            }
            else
            {
                UE_LOG(NitroPythonLog, Error, TEXT("Failed to add 'nitropy' to sys.modules."));
            }
            Py_DECREF(NitroModuleObj); // Decrease reference count
        }
        else
        {
            UE_LOG(NitroPythonLog, Error, TEXT("PyInit_nitropy() returned NULL during fallback injection."));
        }
    }
}

FString UNitroPythonExecutor::EscapeTripleQuotes(const FString& Input)
{
    FString Escaped = Input;
    Escaped.ReplaceInline(TEXT("\"\"\""), TEXT("\\\"\\\"\\\""));
    return FString::Printf(TEXT("\"\"\"%s\"\"\""), *Escaped);
}

FPythonData UNitroPythonExecutor::RunPythonScriptFromFile(const FString& FilePath, const FPythonData& InputData)
{
    FString ScriptContent;
    if (!FFileHelper::LoadFileToString(ScriptContent, *FilePath))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load Python script from file: %s"), *FilePath);
        FPythonData EmptyOutput;
        return EmptyOutput;
    }

    return RunPythonScriptFromString(ScriptContent, InputData);
}

FPythonData UNitroPythonExecutor::RunPythonScriptFromString(const FString& ScriptCode, const FPythonData& InputData)
{
    FPythonData OutputData;

    SetPythonVariablesAndRunScriptWithOutput(InputData, ScriptCode, OutputData);

    return OutputData;
}
