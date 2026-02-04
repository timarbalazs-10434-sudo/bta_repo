// Copyright 2020-2022 MassSun. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DasZipCompress.generated.h"

/**
 *
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FZipFileProgressDelegate, const FString, ZipFilename, int32, total, int32, size);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FZipFileCompleteDelegate, const FString, ZipFilename, bool, isSuccess);


DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FUnZipFileProgressDelegate, const FString, ZipFilename, int32, total, int32, size);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FUnZipFileCompleteDelegate, const FString, ZipFilename, bool, isSuccess);

UENUM(BlueprintType)
enum class ECompressType :uint8
{
	Zlib,
	Gzip
};


UCLASS(BlueprintType, Blueprintable)
class DASZIPPLUGIN_API UDasZipCompress : public UObject
{
	GENERATED_BODY()

public:

	//Create Zip Compress Object
	UFUNCTION(BlueprintCallable, Category = "ZipCompress")
		static UDasZipCompress* GetZipCompress()
	{
		UDasZipCompress* compress = NewObject<UDasZipCompress>();
		return compress;
	}

	//Compress a directory or file. </br>inpath:Compress directory or file</br>ZipFilename:target Zip File.</br>Password:set a password,When empty, do not set password
	UFUNCTION(BlueprintCallable, Category = "ZipCompress")
		bool ZipFile(FString inpath, FString ZipFilename, FString Password);

	//Compress a directory or file. </br>inpath:Compress directory or file</br>ZipFilename:target Zip File.</br>Password:set a password,When empty, do not set password
	UFUNCTION(BlueprintCallable, Category = "ZipCompress")
		void ZipFileByThread(FString inpath, FString ZipFilename, FString Password);

	//UnZip a zip file to outpath
	UFUNCTION(BlueprintCallable, Category = "ZipCompress")
		void UnZipFileByThread(FString zipFilename, FString outpath, FString Password);

	//UnZip a zip file to outpath
	UFUNCTION(BlueprintCallable, Category = "ZipCompress")
		bool UnZipFile(FString ZipFilename, FString outpath, FString Password);

	//Zip String to Byte Array
	UFUNCTION(BlueprintCallable, Category = "ZipCompress")
		bool ZipStringToBytes(const FString& InData, TArray<uint8>& OutData, ECompressType comressType= ECompressType::Zlib);

	//Unzip byte to String
	UFUNCTION(BlueprintCallable, Category = "ZipCompress")
		bool UnZipBytesToString(const TArray<uint8>& InData, FString& OutData, ECompressType comressType = ECompressType::Zlib);

	//Zip String to File</br>InData:source String.</br>FileFullName:target file path
	UFUNCTION(BlueprintCallable, Category = "ZipCompress")
		bool ZipStringToFile(const FString& InData, FString FileFullName, ECompressType comressType = ECompressType::Zlib);

	//Zip File Progress Delegate
	UPROPERTY(BlueprintAssignable)
		FZipFileProgressDelegate  zipFileProgressDelegate;

	//Unzip File Progress Delegate
	UPROPERTY(BlueprintAssignable)
		FUnZipFileProgressDelegate  unZipFileProgressDelegate;

	//Zip File  Complete Delegate
	UPROPERTY(BlueprintAssignable)
		FZipFileCompleteDelegate  zipFileCompleteDelegate;

	//UnZip File  Complete Delegate
	UPROPERTY(BlueprintAssignable)
		FUnZipFileCompleteDelegate unZipFileCompleteDelegate;
private:
	TArray<uint8> FStringToUTF8Bytes(FString Message);
	TArray<uint8> FStringToANSIBytes(FString Message);
	FString ANSIBytesToFString(const TArray<uint8>& data);
	FString UTF8BytesToFString(const TArray<uint8>& data);
	FString GetFileDirectory(FString ZipDir);
	int32 getCRC32(FString Filename);
};
