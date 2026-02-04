// Copyright 2020-2022 MassSun. All Rights Reserved.


#include "DasZipCompress.h"


#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/MinWindows.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif


#include <GenericPlatform/GenericPlatformFile.h>
#include "HAL/FileManager.h"
#include "Async/Async.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Misc/Compression.h"

#include "../../ThirdParty/minizip/minizip.h"

//#include "ThirdParty/zlib/1.2.12/include/zlib.h"


#ifndef VERSIONMADEBY
# define VERSIONMADEBY   (0x0) /* platform depedent */
#endif


bool UDasZipCompress::UnZipFile(FString zipFilename, FString outpath, FString Password)
{
	char* password = NULL;
	int32 passwordlen = 0;
	if (!Password.IsEmpty())
	{
		TArray<uint8> uint8Password = FStringToANSIBytes(Password);
		password = new char[uint8Password.Num() + 1];
		memset(password, 0, uint8Password.Num() + 1);
		memcpy(password, uint8Password.GetData(), uint8Password.Num());
		passwordlen = strlen(password);
	}

	if (!outpath.EndsWith(TEXT("/")) && !outpath.EndsWith(TEXT("\\")))
	{
		outpath = outpath + TEXT("\\");
	}


	uint32 ErrorCount = 0;
	uint32 TotalCount = 0;

	TArray path_array = FStringToANSIBytes(zipFilename);
	unzFile ZFile = unzOpen64(path_array.GetData());


	unz_global_info64 ZGlobalInfo;
	unzGetGlobalInfo64(ZFile, &ZGlobalInfo);

	const int64 CompressedTotalBytesSize = IFileManager::Get().FileSize(*zipFilename);
	int64 CurrentCompressedBytesSize = 0;

	unz_file_info64 ZFileInfo;

	const int FILENAME_BYTES_SIZE = 512;
	const int FILE_DATA_INIT_BYTE_SIZE = 1024 * 1024;
	char* Filename = new char[FILENAME_BYTES_SIZE];
	char* FileData = new char[FILE_DATA_INIT_BYTE_SIZE];

	int ReadLength = 0;
	TotalCount = ZGlobalInfo.number_entry;
	for (int i = 0; i < ZGlobalInfo.number_entry; ++i)
	{
		if (i > 0)
		{
			unzCloseCurrentFile(ZFile);
			unzGoToNextFile(ZFile);
		}

		if (unzGetCurrentFileInfo64(ZFile, &ZFileInfo, Filename, FILENAME_BYTES_SIZE, nullptr, 0, nullptr, 0) != UNZ_OK)
		{
			ErrorCount++;
			continue;
		}


		//if (unzOpenCurrentFile(ZFile) != UNZ_OK)
		if (unzOpenCurrentFilePassword(ZFile, password) != UNZ_OK)
		{
			ErrorCount++;
			continue;
		}

		TArray<uint8> data;
		data.Append((uint8*)Filename, FILENAME_BYTES_SIZE);
		FString FullFilePath = outpath + ANSIBytesToFString(data);	// ZipDir / FMiniCodeConvert::MultiByteToWideChar(Filename);
		if (ZFileInfo.uncompressed_size == 0)
		{
			if (FullFilePath.EndsWith(TEXT("/")) || FullFilePath.EndsWith(TEXT("\\")))
			{
				if (!IFileManager::Get().MakeDirectory(*FullFilePath, true))
				{
					ErrorCount++;
					continue;
				}
			}
			else
			{
				if (!FFileHelper::SaveStringToFile(TEXT(""), *FullFilePath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
				{
					ErrorCount++;
					continue;
				}
			}
		}
		else
		{
			TSharedPtr<FArchive> ArchivePtr = TSharedPtr<FArchive>(IFileManager::Get().CreateFileWriter(*FullFilePath));
			if (ArchivePtr != nullptr)
			{
				const double UnCompressRadio = ((double)ZFileInfo.compressed_size) / ZFileInfo.uncompressed_size;
				const int BlockCount = (ZFileInfo.uncompressed_size + FILE_DATA_INIT_BYTE_SIZE - 1) / FILE_DATA_INIT_BYTE_SIZE;
				for (int Block = 0; Block < BlockCount; ++Block)
				{
					ReadLength = unzReadCurrentFile(ZFile, FileData, FILE_DATA_INIT_BYTE_SIZE);
					ArchivePtr->Serialize(FileData, ReadLength);

					CurrentCompressedBytesSize += ReadLength * UnCompressRadio;

					AsyncTask(ENamedThreads::GameThread, [=,this]()
					{
						unZipFileProgressDelegate.Broadcast(zipFilename, CompressedTotalBytesSize, CurrentCompressedBytesSize);

					});

				}

				if (!ArchivePtr->Close())
				{
					ErrorCount++;
					continue;
				}
			}
			else
			{
				ErrorCount++;
				continue;
			}
		}
	}

	if (password)
	{
		delete[]password;
	}
	unzClose(ZFile);
	delete[] Filename;
	delete[] FileData;

	bool isSuccess = true;
	if (ErrorCount > 0)
		isSuccess = false;
	AsyncTask(ENamedThreads::GameThread, [=,this]()
	{
		unZipFileCompleteDelegate.Broadcast(zipFilename, isSuccess);

	});
	return isSuccess;
}

void UDasZipCompress::UnZipFileByThread(FString zipFilename, FString outpath, FString password)
{
	AsyncTask(ENamedThreads::AnyThread, [=, this]()
	{
		bool isSuccess = UnZipFile(zipFilename, outpath, password);
	});
}


void UDasZipCompress::ZipFileByThread(FString inpath, FString ZipFilename, FString password)
{
	AsyncTask(ENamedThreads::AnyThread, [=, this]()
	{
		bool isSuccess = ZipFile(inpath, ZipFilename, password);


	});
}

FString UDasZipCompress::GetFileDirectory(FString ZipDir)
{

	int32 index;
	bool s = ZipDir.FindLastChar('\\', index);
	if (index > 0)
	{
		return ZipDir.Left(index + 1);
	}
	s = ZipDir.FindLastChar('/', index);
	if (index > 0)
	{
		return ZipDir.Left(index + 1);
	}
	return TEXT("");
}

int32 UDasZipCompress::getCRC32(FString Filename)
{
	int32 len = 1024 * 1024;
	uint8* FileData = new uint8[len];
	uLong _crc32 = 0;

	_crc32 = crc32(0L, Z_NULL, 0);
	TSharedPtr<FArchive> ArchivePtr = TSharedPtr<FArchive>(IFileManager::Get().CreateFileReader(*Filename));
	if (ArchivePtr != nullptr)
	{
		const int64 TotalReadSize = ArchivePtr->TotalSize();

		const int BlockCount = (TotalReadSize + len - 1) / len;
		for (int Block = 0; Block < BlockCount; ++Block)
		{
			int CurrReadSize = 0;
			if (Block == BlockCount - 1)
			{
				CurrReadSize = TotalReadSize - Block * len;
			}
			else
			{
				CurrReadSize = len;
			}

			ArchivePtr->Serialize(FileData, CurrReadSize);

			_crc32 = crc32(_crc32, FileData, CurrReadSize);
		}

		ArchivePtr->Close();
	}






	return _crc32;
}

bool UDasZipCompress::ZipFile(FString Inpath, FString ZipFilename, FString Password)
{

	FString ZipDir = Inpath;
	uint32 ErrorCount = 0;
	uint32 TotalCount = 0;

	const zipFile ZFile = zipOpen64(FStringToANSIBytes(ZipFilename).GetData(), APPEND_STATUS_CREATE);

	int64 TotalUnCompressedFileSize = 0;
	int64 CurrentUnCompressedFileSize = 0;

	const int FILE_DATA_INIT_BYTE_SIZE = 1024 * 1024;
	char* FileData = new char[FILE_DATA_INIT_BYTE_SIZE];

	TArray<FString> AllFilenames; // file or directories

	bool isDirectory = true;
	if (IFileManager::Get().FileExists(*ZipDir))
	{
		isDirectory = false;
		TotalUnCompressedFileSize = IFileManager::Get().FileSize(*ZipDir);
		AllFilenames.Add(ZipDir);

		ZipDir = GetFileDirectory(ZipDir);
	}
	else
	{
		if (!ZipDir.EndsWith(TEXT("/")) && !ZipDir.EndsWith(TEXT("\\")))
		{
			ZipDir = ZipDir + TEXT("\\");
		}
		IFileManager::Get().IterateDirectoryRecursively(*ZipDir, [&TotalUnCompressedFileSize](const TCHAR* FilenameOrDirectory, bool bIsDirectory)
		{
			if (!bIsDirectory)
			{
				TotalUnCompressedFileSize += IFileManager::Get().FileSize(FilenameOrDirectory);
			}
			return true;
		});
		IFileManager::Get().FindFilesRecursive(AllFilenames, *ZipDir, TEXT("*.*"), true, true);
	}



	char* password = NULL;
	int32 crcForCrypting = 0;

	TotalCount = AllFilenames.Num();
	for (uint32 i = 0; i < TotalCount; ++i)
	{
		const FString& Filename = AllFilenames[i];
		if (!Password.IsEmpty())
		{
			TArray<uint8> uint8Password = FStringToANSIBytes(Password);
			password = new char[uint8Password.Num() + 1];
			memset(password, 0, uint8Password.Num() + 1);
			memcpy(password, uint8Password.GetData(), uint8Password.Num());
			crcForCrypting = getCRC32(Filename);
		}


		FDateTime DataTime = IFileManager::Get().GetTimeStamp(*Filename);
		DataTime = DataTime + (FDateTime::Now() - FDateTime::UtcNow()); //   Ҫ   ϱ   ʱ    ʱ  
		const int64 LocalFileSize = IFileManager::Get().FileSize(*Filename);

		zip_fileinfo ZipFileInfo;
		int32 Year, Month, Day;
		DataTime.GetDate(Year, Month, Day);
		ZipFileInfo.tmz_date.tm_year = Year;
		ZipFileInfo.tmz_date.tm_mon = Month - 1;
		ZipFileInfo.tmz_date.tm_mday = Day;
		ZipFileInfo.tmz_date.tm_hour = DataTime.GetHour();
		ZipFileInfo.tmz_date.tm_min = DataTime.GetMinute();
		ZipFileInfo.tmz_date.tm_sec = DataTime.GetSecond();

		ZipFileInfo.dosDate = 0;
		ZipFileInfo.internal_fa = 0;
		ZipFileInfo.external_fa = 0;

		FString LocalFilename = *Filename.Replace(*ZipDir, TEXT("")); //  滻  zip µ    ·  
		if (LocalFilename.StartsWith(TEXT("/")))
		{
			LocalFilename = LocalFilename.RightChop(1);
		}
		else if (LocalFilename.StartsWith(TEXT("\\")))
		{
			LocalFilename = LocalFilename.RightChop(2);
		}

		const bool bDirectory = FPaths::DirectoryExists(AllFilenames[i]);
		if (bDirectory)
		{
			LocalFilename += TEXT("/");
		}

		if (i > 0)
		{
			zipCloseFileInZip(ZFile);
		}

		//zipOpenNewFileInZip3_64
		if (zipOpenNewFileInZip3_64(ZFile, (char*)FStringToANSIBytes(LocalFilename).GetData(), &ZipFileInfo, nullptr, 0, nullptr, 0, nullptr, Z_DEFLATED, Z_DEFAULT_COMPRESSION, 0, -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, password, crcForCrypting, LocalFileSize >= UINT32_MAX) != ZIP_OK)
		{
			ErrorCount++;
			continue;
		}

		if (!bDirectory)
		{
			TSharedPtr<FArchive> ArchivePtr = TSharedPtr<FArchive>(IFileManager::Get().CreateFileReader(*Filename));
			if (ArchivePtr != nullptr)
			{
				const int64 TotalReadSize = ArchivePtr->TotalSize();
				const int BlockCount = (TotalReadSize + FILE_DATA_INIT_BYTE_SIZE - 1) / FILE_DATA_INIT_BYTE_SIZE;
				for (int Block = 0; Block < BlockCount; ++Block)
				{
					int CurrReadSize = 0;
					if (Block == BlockCount - 1)
					{
						CurrReadSize = TotalReadSize - Block * FILE_DATA_INIT_BYTE_SIZE;
					}
					else
					{
						CurrReadSize = FILE_DATA_INIT_BYTE_SIZE;
					}

					ArchivePtr->Serialize(FileData, CurrReadSize);
					if (zipWriteInFileInZip(ZFile, FileData, CurrReadSize) != ZIP_OK)
					{
						ErrorCount++;
						continue;
					}
					else
					{
						CurrentUnCompressedFileSize += CurrReadSize;
						AsyncTask(ENamedThreads::GameThread, [=, this]()
						{
							unZipFileProgressDelegate.Broadcast(ZipFilename, TotalUnCompressedFileSize, CurrentUnCompressedFileSize);

						});
					}
				}

				if (!ArchivePtr->Close())
				{
					ErrorCount++;
					continue;
				}
			}
			else
			{
				ErrorCount++;
				continue;
			}
		}
	}

	if (password)
	{
		delete[]password;
	}
	delete[] FileData;
	zipClose(ZFile, nullptr);

	bool isSuccess = true;
	if (ErrorCount > 0)
		isSuccess = false;

	AsyncTask(ENamedThreads::GameThread, [=, this]()
	{
		zipFileCompleteDelegate.Broadcast(ZipFilename, isSuccess);

	});
	return isSuccess;
}

bool UDasZipCompress::ZipStringToFile(const FString& InData, FString FileFullName, ECompressType comressType)
{
	TArray<uint8> OutData;
	bool b = ZipStringToBytes(InData, OutData,comressType);
	if (b)
	{
		TSharedPtr<FArchive> ArchivePtr = TSharedPtr<FArchive>(IFileManager::Get().CreateFileWriter(*FileFullName));
		if (ArchivePtr != nullptr)
		{

			ArchivePtr->Serialize(OutData.GetData(), OutData.Num());
			ArchivePtr->Close();
		}
	}
	return b;
}


bool UDasZipCompress::ZipStringToBytes(const FString& InData, TArray<uint8>& OutData, ECompressType comressType)
{
	if (InData.Len() == 0)
	{
		return true;
	}

	//TArray<uint8> bytes = FStringToANSIBytes(InData);
	TArray<uint8> bytes = FStringToUTF8Bytes(InData);
	uint32  bytes_length = bytes.Num();
	uint32 UncompressedSize = bytes_length;// -1;


	const uint32 UintSize = sizeof(uint32);

	FName FormatName;
	if (comressType == ECompressType::Zlib)
	{
		FormatName = NAME_Zlib;
	}
	else
	{
		FormatName = NAME_Gzip;
	}

	int CompressBufferLen = FCompression::CompressMemoryBound(FormatName, UncompressedSize);
//	int CompressBufferLen = FCompression::CompressMemoryBound(NAME_Gzip, UncompressedSize);
//	int CompressBufferLen = FCompression::CompressMemoryBound(NAME_Zlib, UncompressedSize);

	uint8* CompressBuffer = static_cast<uint8*>(FMemory::Malloc(CompressBufferLen));

	FMemory::Memzero(CompressBuffer, UncompressedSize);

	const bool success = FCompression::CompressMemory(FormatName, CompressBuffer, CompressBufferLen, bytes.GetData(), UncompressedSize);
	if (success)
	{
		OutData.Append(CompressBuffer, CompressBufferLen);
	}
	FMemory::Free(CompressBuffer);

	return  success;
}


bool UDasZipCompress::UnZipBytesToString(const TArray<uint8>& InData, FString& OutData, ECompressType comressType)
{
	if (InData.Num() == 0)
	{
		return true;
	}
	const uint32 UintSize = sizeof(uint32);
	uint32 UncompressedSize = InData.Num() * 1000;
	uint8* UncompressBuffer = static_cast<uint8*>(FMemory::Malloc(UncompressedSize));
	FMemory::Memzero(UncompressBuffer, UncompressedSize);

	FName FormatName;
	if (comressType == ECompressType::Zlib)
	{
		FormatName = NAME_Zlib;
	}
	else
	{
		FormatName = NAME_Gzip;
	}

	const bool success = FCompression::UncompressMemory(FormatName, UncompressBuffer, UncompressedSize, InData.GetData(), InData.Num());
	//if (success)
	//{
	TArray<uint8> bytes;
	bytes.Append(UncompressBuffer, UncompressedSize);
	//OutData = ANSIBytesToFString(bytes);
	OutData = UTF8BytesToFString(bytes);
	//}
	FMemory::Free(UncompressBuffer);
	return success;
}



/**
* Convert FString to  UTF8 bytes
*/
TArray<uint8> UDasZipCompress::FStringToUTF8Bytes(FString Message)
{
	const TCHAR* tchar = Message.GetCharArray().GetData();
	int iLength = WideCharToMultiByte(CP_UTF8, 0, tchar, -1, NULL, 0, NULL, NULL);
	char* _utfchar = new char[iLength];
	//memset(_utfchar, 0, iLength);
	WideCharToMultiByte(CP_UTF8, 0, tchar, -1, _utfchar, iLength, NULL, NULL);

	TArray<uint8> utf8Array;
	utf8Array.Append((uint8*)_utfchar, iLength);

	delete[]_utfchar;
	return utf8Array;
}

/**
* Convert FString to  ANSI bytes
*/
TArray<uint8> UDasZipCompress::FStringToANSIBytes(FString Message)
{
	int iLength;

	const TCHAR* tchar = Message.GetCharArray().GetData();
	iLength = WideCharToMultiByte(CP_ACP, 0, tchar, -1, NULL, 0, NULL, NULL);
	char* _char = new char[iLength];
	memset(_char, 0, iLength);
	WideCharToMultiByte(CP_ACP, 0, tchar, -1, _char, iLength, NULL, NULL);

	TArray<uint8> bytesArray;
	bytesArray.Append((uint8*)_char, iLength-1);

	delete[]_char;
	return bytesArray;
}



/**
* Convert ANSI bytes to FString
*/
FString UDasZipCompress::ANSIBytesToFString(const TArray<uint8>& data)
{

	int32 num = data.Num();
	char* pchar = new char[num];
	memcpy(pchar, data.GetData(), num);

	int32 iLength = MultiByteToWideChar(CP_ACP, 0, pchar, num, NULL, 0);
	TCHAR* tchar2 = new TCHAR[iLength + 1];
	memset(tchar2, 0, sizeof(TCHAR) * (iLength + 1));
	MultiByteToWideChar(CP_ACP, 0, pchar, num, tchar2, iLength);

	FString fstring = FString(tchar2);
	delete[]pchar;
	delete[]tchar2;
	return fstring;
}

/**
* Convert UTF8 bytes to FString
*/
FString UDasZipCompress::UTF8BytesToFString(const TArray<uint8>& data)
{
	int32 num = data.Num();
	char* _utfchar = new char[num];
	memcpy(_utfchar, data.GetData(), num);
	int iLength = MultiByteToWideChar(CP_UTF8, 0, _utfchar, num, NULL, 0);
	TCHAR* tchar3 = new TCHAR[iLength + 1];
	memset(tchar3, 0, sizeof(TCHAR) * (iLength + 1));
	MultiByteToWideChar(CP_UTF8, 0, _utfchar, num, tchar3, iLength);
	FString utf8str = FString(tchar3);
	delete[]_utfchar;
	delete[]tchar3;
	return utf8str;
}
