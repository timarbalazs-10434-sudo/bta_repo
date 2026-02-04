// Copyright 2022 Dexter.Wan. All Rights Reserved. 
// EMail: 45141961@qq.com

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DTSelectFileBPLib.generated.h"

#define MAX_TYPES  (255 * 4)
/**
 * 
 */
UCLASS()
class DTSELECTFILE_API UDTSelectFileBPLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	
public:
	// 生成文件格式
	static void GenerateFileType(const TMap<FString, FString>& FileType, TCHAR szFileTypes[MAX_TYPES], TArray<FString> & ArraySuffix);
	
	// 打开文件选择窗口（单选）
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static bool OpenFileDialog(const FString& DialogTitle, const FString& DefaultPath, const TMap<FString, FString>& FileType, FString & FilePath );

	// 打开文件选择窗口（多选）
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static bool OpenFileMultipleDialog(const FString& DialogTitle, const FString& DefaultPath, const TMap<FString, FString>& FileType, TArray<FString>& FilePath );

	// 保存文件选择窗口（单选）
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static bool SaveFileDialog(const FString& DialogTitle, const FString& DefaultPath, const TMap<FString, FString>& FileType, FString & FilePath );

	// 选择目录（单选）
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static bool OpenDirectoryDialog(const FString& DialogTitle, const FString& DefaultDirectory, bool bNewFolder, FString & DirectoryPath );

	// 运行文件
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static bool RunFile(const FString & FilePath);
};
