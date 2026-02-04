// Copyright 2023 Dexter.Wan. All Rights Reserved. 
// EMail: 45141961@qq.com

#include "DTCmd.h"
#include "Windows/AllowWindowsPlatformAtomics.h"
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows.h"
#include "Windows/HideWindowsPlatformTypes.h"
#include "Windows/HideWindowsPlatformAtomics.h"
#define LOCTEXT_NAMESPACE "FDTCmdModule"

void FDTCmdModule::StartupModule()
{
#if !UE_SERVER
	AllocConsole(); 
	ShowWindow(GetConsoleWindow(), SW_HIDE);
#endif
}

void FDTCmdModule::ShutdownModule()
{
#if !UE_SERVER
	FreeConsole();
#endif
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FDTCmdModule, DTCmd)