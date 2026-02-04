// Copyright 2022 Dexter.Wan. All Rights Reserved. 
// EMail: 45141961@qq.com


#include "DTSelectFileBPLib.h"
#include "Misc/Paths.h"
#include "Framework/Application/SlateApplication.h"

#include "Windows/AllowWindowsPlatformTypes.h"
#include <windows.h>
#include <commdlg.h>
#include <shlobj.h>
#include "Windows/HideWindowsPlatformTypes.h"

// 生成文件格式
void UDTSelectFileBPLib::GenerateFileType(const TMap<FString, FString>& FileType, TCHAR szFileTypes[MAX_TYPES], TArray<FString>& ArraySuffix)
{
	// 重置输出数据
	ArraySuffix.Empty();

	// 开始转换
	int nIndex = 0;
	FString Suffix;
	for ( const TTuple<FString, FString> & Type : FileType )
	{
		// 无效的选项
		if (Type.Key.IsEmpty() || Type.Value.IsEmpty())
		{
			continue;
		}

		// 添加显示信息
		for ( const TCHAR & C : Type.Key )
		{
			if ( nIndex < MAX_TYPES - 2 )
			{
				szFileTypes[nIndex] = C;
				Suffix += C;
				nIndex++;
			}
		}

		// 保存数据
		ArraySuffix.Add(Suffix);
		Suffix.Empty();

		// 空\0
		nIndex++;

		// 实际后缀
		for ( const TCHAR & C : Type.Value )
		{
			if ( nIndex < MAX_TYPES - 2 )
			{
				szFileTypes[nIndex] = C;
				Suffix += C;
				nIndex++;
			}
		}

		// 保存数据
		ArraySuffix.Add(Suffix);
		Suffix.Empty();

		// 空\0
		nIndex++;
	}
}

// 打开文件选择窗口（单选）
bool UDTSelectFileBPLib::OpenFileDialog(const FString& DialogTitle, const FString& DefaultPath, const TMap<FString, FString>& FileType, FString& FilePath)
{
	// 重置输出数据
	FilePath.Empty();

	// 后缀数据
	TArray<FString> ArraySuffix;

	// 格式化拓展名
	TCHAR szFileTypes[MAX_TYPES] = {};									
	memset(szFileTypes, 0, sizeof(szFileTypes));
	GenerateFileType(FileType, szFileTypes, ArraySuffix);
	
	// 选择窗口属性
	TCHAR szBuffer[MAX_PATH] = {0}; 
	memset(szBuffer, 0, sizeof(szBuffer));
	OPENFILENAME OepnFile = {};
	memset(&OepnFile, 0, sizeof(OepnFile));
	OepnFile.lStructSize = sizeof(OepnFile);
	OepnFile.hwndOwner = (HWND)FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);
	OepnFile.lpstrTitle = *DialogTitle;
	OepnFile.lpstrFilter = szFileTypes;
	OepnFile.lpstrInitialDir = *DefaultPath;
	OepnFile.lpstrFile = szBuffer;
	OepnFile.nMaxFile = sizeof(szBuffer)/sizeof(TCHAR); 
	OepnFile.nFilterIndex = 0; 
	OepnFile.Flags = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST|OFN_EXPLORER;

	// 打开窗口
	if (!GetOpenFileNameW(&OepnFile))
	{
		return false;
	}

	// 保存地址
	FilePath = szBuffer;
	
	return true;
}

// 打开文件选择窗口（多选）
bool UDTSelectFileBPLib::OpenFileMultipleDialog(const FString& DialogTitle, const FString& DefaultPath, const TMap<FString, FString>& FileType, TArray<FString>& FilePath)
{
	// 重置输出数据
	FilePath.Empty();

	// 后缀数据
	TArray<FString> ArraySuffix;

	// 格式化拓展名
	TCHAR szFileTypes[MAX_TYPES] = {};									
	memset(szFileTypes, 0, sizeof(szFileTypes));
	GenerateFileType(FileType, szFileTypes, ArraySuffix);
	
	// 选择窗口属性
	TCHAR szBuffer[MAX_PATH * 80] = {}; 
	memset(szBuffer, 0, sizeof(szBuffer));
	OPENFILENAME OepnFile = {};
	memset(&OepnFile, 0, sizeof(OepnFile));
	OepnFile.lStructSize = sizeof(OepnFile);
	OepnFile.hwndOwner = (HWND)FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);
	OepnFile.lpstrTitle = *DialogTitle;
	OepnFile.lpstrFilter = szFileTypes;
	OepnFile.lpstrInitialDir = *DefaultPath;
	OepnFile.lpstrFile = szBuffer;
	OepnFile.nMaxFile = sizeof(szBuffer)/sizeof(TCHAR); 
	OepnFile.nFilterIndex = 0; 
	OepnFile.Flags = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST|OFN_EXPLORER|OFN_ALLOWMULTISELECT;

	// 打开窗口
	if (!GetOpenFileNameW(&OepnFile))
	{
		return false;
	}

	// 保存目录
	TCHAR szPath[MAX_PATH] = {};
	memset(szPath, 0, sizeof(szPath));
	lstrcpyn(szPath, OepnFile.lpstrFile, OepnFile.nFileOffset);
	szPath[OepnFile.nFileOffset] = '\0';
	
	// 把指针移到第一个文件
	TCHAR * pName = OepnFile.lpstrFile + OepnFile.nFileOffset;  
	while(*pName)
	{
		FString File(szPath);
		File += TEXT("\\");
		File += pName;
		FilePath.Add(File);
		pName += lstrlen(pName) + 1;             //移至下一个文件
	}
	
	return true;
}

// 保存文件选择窗口（单选）
bool UDTSelectFileBPLib::SaveFileDialog(const FString& DialogTitle, const FString& DefaultPath, const TMap<FString, FString>& FileType, FString& FilePath)
{
	// 重置输出数据
	FilePath.Empty();

	// 后缀数据
	TArray<FString> ArraySuffix;

	// 格式化拓展名
	TCHAR szFileTypes[MAX_TYPES] = {};									
	memset(szFileTypes, 0, sizeof(szFileTypes));
	GenerateFileType(FileType, szFileTypes, ArraySuffix);
	
	// 选择窗口属性
	TCHAR szBuffer[MAX_PATH] = {0};
	memset(szBuffer, 0, sizeof(szBuffer));
	OPENFILENAME SaveFile = {};
	memset(&SaveFile, 0, sizeof(SaveFile));
	SaveFile.lStructSize = sizeof(SaveFile);
	SaveFile.hwndOwner = (HWND)FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);
	SaveFile.lpstrTitle = *DialogTitle;
	SaveFile.lpstrFilter = szFileTypes;
	SaveFile.lpstrInitialDir = *DefaultPath;
	SaveFile.lpstrFile = szBuffer;
	SaveFile.nMaxFile = sizeof(szBuffer)/sizeof(TCHAR); 
	SaveFile.nFilterIndex = 0; 
	SaveFile.Flags = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST|OFN_EXPLORER;

	// 打开窗口
	if (!GetSaveFileNameW(&SaveFile))
	{
		return false;
	}

	// 保存地址
	FilePath = szBuffer;
	if (FPaths::GetExtension(FilePath).IsEmpty())
	{
		const int nIndex = SaveFile.nFilterIndex * 2 - 1;
		if (nIndex < ArraySuffix.Num())
		{
			FilePath += FPaths::GetExtension(ArraySuffix[nIndex], true);
		}
	}
	return true;
}

// 目录回调函数
FString OpenDefaultDirectory;
int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	if (uMsg == BFFM_INITIALIZED)
	{
		::SendMessageW(hwnd, BFFM_SETSELECTION, 1, (LPARAM)(*OpenDefaultDirectory));
	}
	return 0;
}

// 选择目录（单选）
bool UDTSelectFileBPLib::OpenDirectoryDialog(const FString& DialogTitle, const FString& DefaultDirectory, bool bNewFolder, FString& DirectoryPath)
{
	// 重置输出数据
	DirectoryPath.Empty();

	// 保存地址
	OpenDefaultDirectory = DefaultDirectory;

	// 文件浏览信息
	BROWSEINFO BrowseInfo;
	BrowseInfo.hwndOwner = (HWND)FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);
	BrowseInfo.pidlRoot = nullptr;
	BrowseInfo.pszDisplayName = nullptr;
	BrowseInfo.lpszTitle = *DialogTitle; // 显示位于对话框左上部的提示信息
	BrowseInfo.ulFlags = BIF_DONTGOBELOWDOMAIN|BIF_RETURNONLYFSDIRS;
	if (bNewFolder) BrowseInfo.ulFlags |= BIF_NEWDIALOGSTYLE;
	BrowseInfo.lpfn = BrowseCallbackProc;
	BrowseInfo.iImage = 0;

	// 调用选择对话框
	const LPITEMIDLIST pItem = SHBrowseForFolderW(&BrowseInfo);
	if (pItem == nullptr)
	{
		return false;
	}

	// 获取选择目录
	TCHAR szFolder[MAX_PATH];
	SHGetPathFromIDListW(pItem, szFolder);
	DirectoryPath = szFolder;
	if (!FPaths::IsDrive(DirectoryPath))
	{
		DirectoryPath += TEXT("\\");
	}
	
	return true;
}

// 运行文件
bool UDTSelectFileBPLib::RunFile(const FString & FilePath)
{
	if (FilePath.IsEmpty() || !FPaths::FileExists(FilePath))
	{
		return true;
	}
	return FPlatformMisc::OsExecute(TEXT("open"), *FilePath);
}
