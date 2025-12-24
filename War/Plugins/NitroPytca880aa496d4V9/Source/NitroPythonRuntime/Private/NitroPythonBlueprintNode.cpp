// Copyright (c) 2025 Picasso Labs. All Rights Reserved.
//
// This file is part of NitroPythonRuntime, licensed under the Picasso Labs Software License.

#include "NitroPythonBlueprintNode.h"
#include "NitroPythonRuntime.h"
#include "NitroPythonExecutor.h"
#include "Misc/FileHelper.h"
#include "Async/Async.h"
#include "Engine/World.h"


UNitroPythonBlueprintNode* UNitroPythonBlueprintNode::RunPythonScriptFromFileAsync(UObject* WorldContextObject, const FString& FilePath, const FPythonData& InputData)
{

    FString ScriptContent;
    if (!FFileHelper::LoadFileToString(ScriptContent, *FilePath))
    {
        UE_LOG(NitroPythonLog, Error, TEXT("Failed to load Python script from file: %s"), *FilePath);
    }

    UNitroPythonBlueprintNode* NodeObj = NewObject<UNitroPythonBlueprintNode>();
	if (NodeObj)
	{
		NodeObj->ThreadScript = ScriptContent;
        NodeObj->ThreadInputData = InputData;
	}

	NodeObjCopy = NodeObj;
    return NodeObj;
}

UNitroPythonBlueprintNode* UNitroPythonBlueprintNode::RunPythonScriptFromStringAsync(UObject* WorldContextObject, const FString& ScriptCode, const FPythonData& InputData)
{
    UNitroPythonBlueprintNode* NodeObj = NewObject<UNitroPythonBlueprintNode>();
	if (NodeObj)
	{
		NodeObj->ThreadScript = ScriptCode;
        NodeObj->ThreadInputData = InputData;
	}

	NodeObjCopy = NodeObj;
    return NodeObj;
}

static PyObject* NitroOutput(PyObject* self, PyObject* args)
{
    NodeObjCopy->execIfBound();
    Py_RETURN_NONE;  // Return None in Python
}

static PyMethodDef NitroMethods[] = {
    {"output", NitroOutput, METH_NOARGS, "Provides the 'output' global variable to blueprint node"},
    {nullptr, nullptr, 0, nullptr}  // Sentinel value
};

static struct PyModuleDef NitroModule = {
    PyModuleDef_HEAD_INIT,
    "nitropy",   // Module name
    "Provides the 'output' global variable to blueprint node",  // Module documentation
    -1,         // Size of per-interpreter state of the module
    NitroMethods
};

PyMODINIT_FUNC PyInit_nitropy(void)
{
    return PyModule_Create(&NitroModule);
}

void UNitroPythonBlueprintNode::execIfBound()
{
    PythonThreadDelegate.ExecuteIfBound();
}

void UNitroPythonBlueprintNode::NitroOutputData()
{
    // Acquire the Global Interpreter Lock (GIL)
    PyGILState_STATE gil_state = PyGILState_Ensure();

    try
    {
        PyObject* globals_dict = PyEval_GetGlobals();
        if (!globals_dict || !PyDict_Check(globals_dict))
        {
            UE_LOG(NitroPythonLog, Error, TEXT("Failed to retrieve Python global dictionary."));
            PyGILState_Release(gil_state);
            return;
        }

        PyObject* py_output = PyDict_GetItemString(globals_dict, "output");
        if (!py_output || !PyDict_Check(py_output))
        {
            UE_LOG(NitroPythonLog, Warning, TEXT("No 'output' dictionary found in Python globals."));
            PyGILState_Release(gil_state);
            return;
        }

        FPythonData OutputData;

        // Iterate through the dictionary
        PyObject *key, *value;
        Py_ssize_t pos = 0;
        while (PyDict_Next(py_output, &pos, &key, &value))
        {
            FString Key = FString(UTF8_TO_TCHAR(PyUnicode_AsUTF8(key)));

            if (PyBool_Check(value))
            {
                OutputData.Bools.Add(Key, static_cast<bool>(PyObject_IsTrue(value)));
            }
            else if (PyLong_Check(value))
            {
                OutputData.Ints.Add(Key, static_cast<int32>(PyLong_AsLong(value)));
            }
            else if (PyFloat_Check(value))
            {
                OutputData.Floats.Add(Key, static_cast<float>(PyFloat_AsDouble(value)));
            }
            else if (PyUnicode_Check(value))
            {
                OutputData.Strings.Add(Key, FString(UTF8_TO_TCHAR(PyUnicode_AsUTF8(value))));
            }
            else if (PyList_Check(value))
            {
                TArray<FString> StringArray;
                TArray<bool> BoolArray;
                TArray<int32> IntArray;
                TArray<float> FloatArray;
                
                bool bIsStringArray = false, bIsIntArray = false, bIsFloatArray = false, bIsBoolArray = false;

                Py_ssize_t list_size = PyList_Size(value);
                for (Py_ssize_t i = 0; i < list_size; ++i)
                {
                    PyObject* entry = PyList_GetItem(value, i);  // Borrowed reference
                    if (PyUnicode_Check(entry))
                    {
                        bIsStringArray = true;
                        StringArray.Add(FString(UTF8_TO_TCHAR(PyUnicode_AsUTF8(entry))));
                    }
                    else if (PyBool_Check(entry))
                    {
                        bIsBoolArray = true;
                        BoolArray.Add(static_cast<bool>(PyObject_IsTrue(entry)));
                    }
                    else if (PyLong_Check(entry))
                    {
                        bIsIntArray = true;
                        IntArray.Add(static_cast<int32>(PyLong_AsLong(entry)));
                    }
                    else if (PyFloat_Check(entry))
                    {
                        bIsFloatArray = true;
                        FloatArray.Add(static_cast<float>(PyFloat_AsDouble(entry)));
                    }
                }

                if (bIsStringArray) OutputData.StringArrays.Add(Key, {StringArray});
                if (bIsBoolArray) OutputData.BoolArrays.Add(Key, {BoolArray});
                if (bIsIntArray) OutputData.IntArrays.Add(Key, {IntArray});
                if (bIsFloatArray) OutputData.FloatArrays.Add(Key, {FloatArray});
            }
        }

        // Broadcast the extracted output to Unreal Engine
        AsyncTask(ENamedThreads::GameThread, [this, OutputData]()
        {
            OnPythonOutputReceived.Broadcast(OutputData);
        });
    }
    catch (const std::exception& e)
    {
        UE_LOG(NitroPythonLog, Error, TEXT("Error extracting output: %s"), UTF8_TO_TCHAR(e.what()));
    }

    // Release the GIL
    PyGILState_Release(gil_state);
}

void UNitroPythonBlueprintNode::Activate()
{	
    PythonThreadDelegate.BindUFunction(this, FName("NitroOutputData"), true);
	
	FNitroRunnable* PythonWorker = new FNitroRunnable(ThreadScript, ThreadInputData, this);
}

FNitroRunnable::FNitroRunnable(const FString& PyScript, const FPythonData& InputData, UNitroPythonBlueprintNode* InNode)
{
	RunnableScript = PyScript;
    RunnableInput = InputData;
    OwningNode = InNode;
    NitroRunnableThread = FRunnableThread::Create(static_cast<FRunnable*>(this), TEXT("NitroRunnableThread"));
}

FNitroRunnable::~FNitroRunnable()
{
	NitroRunnableThread = nullptr;
}

bool FNitroRunnable::Init()
{
	return true;
}

uint32 FNitroRunnable::Run()
{
    UE_LOG(NitroPythonLog, Log, TEXT("Nitro thread running: %s"), *RunnableScript);

    try
    {
        FPythonErrorData ErrorData = UNitroPythonExecutor::SetPythonVariablesAndRunScript(RunnableInput, RunnableScript);
        
        if (ErrorData.IsError && OwningNode.IsValid())
        {
            FPythonData ErrorOutputData;
            ErrorOutputData.Bools.Add(TEXT("nitro_is_error"), true);
            ErrorOutputData.Strings.Add(TEXT("nitro_error_message"), ErrorData.ErrorMessage);

            AsyncTask(ENamedThreads::GameThread, [WeakOwningNode = OwningNode, ErrorOutputData]()
            {
                if (WeakOwningNode.IsValid())
                {
                    WeakOwningNode->OnPythonErrorReceived.Broadcast(ErrorOutputData);
                }
            });
        }
    }
    catch (const std::exception& Err)
    {        
        if (NitroRunnableThread)
        {
            NitroRunnableThread->Kill();
        }
        UE_LOG(NitroPythonLog, Error, TEXT("ERROR in Script: %s"), UTF8_TO_TCHAR(Err.what()));
        return 1;
    }

    return 0;
}

void FNitroRunnable::Exit()
{
    if (NitroRunnableThread)
    {
        delete this;
    }
}

void FNitroRunnable::Stop()
{
	delete this;
}
