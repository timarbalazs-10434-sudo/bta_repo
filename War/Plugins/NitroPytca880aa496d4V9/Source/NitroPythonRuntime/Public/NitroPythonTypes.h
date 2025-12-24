// Copyright (c) 2025 Picasso Labs. All Rights Reserved.
//
// This file is part of NitroPythonRuntime, licensed under the Picasso Labs Software License.

#pragma once

#include "CoreMinimal.h"
#include "NitroPythonTypes.generated.h"

/** Struct for storing an array of booleans */
USTRUCT(BlueprintType)
struct NITROPYTHONRUNTIME_API FPyBoolArray
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Nitro|Python")
    TArray<bool> BoolArray;
};

/** Struct for storing an array of strings */
USTRUCT(BlueprintType)
struct NITROPYTHONRUNTIME_API FPyStringArray
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Nitro|Python")
    TArray<FString> StringArray;
};

/** Struct for storing an array of integers */
USTRUCT(BlueprintType)
struct NITROPYTHONRUNTIME_API FPyIntArray
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Nitro|Python")
    TArray<int32> IntArray;
};

/** Struct for storing an array of floating-point values */
USTRUCT(BlueprintType)
struct NITROPYTHONRUNTIME_API FPyFloatArray
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Nitro|Python")
    TArray<float> FloatArray;
};

/** Struct for storing all Python input data */
USTRUCT(BlueprintType)
struct NITROPYTHONRUNTIME_API FPythonData
{
    GENERATED_BODY()

    /** Map of string variable names to boolean values */
    UPROPERTY(BlueprintReadWrite, Category = "Nitro|Python")
    TMap<FString, bool> Bools;

    /** Map of string variable names to string values */
    UPROPERTY(BlueprintReadWrite, Category = "Nitro|Python")
    TMap<FString, FString> Strings;

    /** Map of string variable names to integer values */
    UPROPERTY(BlueprintReadWrite, Category = "Nitro|Python")
    TMap<FString, int32> Ints;

    /** Map of string variable names to float values */
    UPROPERTY(BlueprintReadWrite, Category = "Nitro|Python")
    TMap<FString, float> Floats;

    /** Map of string variable names to boolean arrays */
    UPROPERTY(BlueprintReadWrite, Category = "Nitro|Python")
    TMap<FString, FPyBoolArray> BoolArrays;

    /** Map of string variable names to string arrays */
    UPROPERTY(BlueprintReadWrite, Category = "Nitro|Python")
    TMap<FString, FPyStringArray> StringArrays;

    /** Map of string variable names to integer arrays */
    UPROPERTY(BlueprintReadWrite, Category = "Nitro|Python")
    TMap<FString, FPyIntArray> IntArrays;

    /** Map of string variable names to float arrays */
    UPROPERTY(BlueprintReadWrite, Category = "Nitro|Python")
    TMap<FString, FPyFloatArray> FloatArrays;
};

USTRUCT()
struct NITROPYTHONRUNTIME_API FPythonErrorData
{
    GENERATED_BODY()

    bool IsError;

    FString ErrorMessage;
};
