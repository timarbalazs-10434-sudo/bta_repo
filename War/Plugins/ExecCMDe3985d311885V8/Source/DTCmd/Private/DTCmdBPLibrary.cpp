// Copyright 2023 - 2024 Dexter.Wan. All Rights Reserved. 
// EMail: 45141961@qq.com

#include "DTCmdBPLibrary.h"
#include "Windows/AllowWindowsPlatformAtomics.h"
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows.h"
#include "Windows/HideWindowsPlatformTypes.h"
#include "Windows/HideWindowsPlatformAtomics.h"

#include "LatentActions.h"
#include "Async/Async.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Async/AsyncWork.h"
#include "Containers/Queue.h"
#include "Misc/ScopeLock.h"
#include <string>

TArray<class FDTCmdDelayAction*>		GDelayAction;
FCriticalSection						GDelayActionCS;

class FDTCmdDelayAction : public FPendingLatentAction
{
public:
	
	// 系统回调函数
	int32 OutputLink;
	FName ExecutionFunction;
	FWeakObjectPtr CallbackTarget;
	
	// 函数返回值
	FString& Result;
	
	// 结束运行
	TQueue<FString> OverInfos;

	FDTCmdDelayAction(FString & _Result, const FLatentActionInfo& _LatentInfo)
		: OutputLink(_LatentInfo.Linkage)
		, ExecutionFunction(_LatentInfo.ExecutionFunction)
		, CallbackTarget(_LatentInfo.CallbackTarget)
		, Result(_Result)
	{
		FScopeLock Lock(&GDelayActionCS);
		GDelayAction.Add(this);
	}

	virtual ~FDTCmdDelayAction() override
	{
		FScopeLock Lock(&GDelayActionCS);
		GDelayAction.Remove(this);
	}

	virtual void UpdateOperation(FLatentResponse& Response) override
	{
		if ( OverInfos.Dequeue( Result ))
		{
			Response.TriggerLink(ExecutionFunction, OutputLink, CallbackTarget);
		}
	}

	// 结束回调函数
	void OnOver(FString _Result)
	{
		OverInfos.Enqueue(_Result);
	}
};

// 延迟线程
class FDTCmdAsyncTask : public FNonAbandonableTask
{
	friend class FAsyncTask<FDTCmdAsyncTask>;

private:
	bool				ErrorOut;
	FString				Cmd;
	UINT				CodePage;
	FDTCmdProcess		Process;
	FDTCmdDelayAction * DTCmdDelayAction;
	
public:
	// 初始化
	FDTCmdAsyncTask(const bool _ErrorOut, const FString & _Cmd, UINT _CodePage, const FDTCmdProcess _Process, FDTCmdDelayAction * _DTCmdDelayAction)
		: ErrorOut(_ErrorOut), Cmd(_Cmd), CodePage(_CodePage), Process(_Process), DTCmdDelayAction(_DTCmdDelayAction) {}
	
	// 线程标识
	FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(FDTCmdAsyncTask, STATGROUP_ThreadPoolAsyncTasks);}
	
	// 延迟线程
	void DoWork()
	{
		constexpr int nMaxCount = 2046;
		FString szCmd = Cmd;
		if ( ErrorOut ) { szCmd += TEXT(" 2>&1"); }
		FString szResult;
		FILE* pPipe = _wpopen(*szCmd, TEXT("rt"));
		if (pPipe == nullptr)
		{
			wchar_t ErrorBuffer[nMaxCount + 1] = {0};
			_wperror(ErrorBuffer);
			FString szErrorBuffer = ErrorBuffer;
			AsyncTask(ENamedThreads::GameThread, [DTCmdDelayAction = DTCmdDelayAction, Cmd = Cmd, ErrorBuffer = szErrorBuffer]()
			{
				FScopeLock Lock(&GDelayActionCS);
				if ( GDelayAction.Find(DTCmdDelayAction) != INDEX_NONE )
				{
					DTCmdDelayAction->OnOver(FString::Printf(TEXT("%s run failed %s"), *Cmd, *ErrorBuffer));
				}
			});
			return;
		}
		
		char szBuffer[nMaxCount + 1] = { 0 };
		while ( fgets(szBuffer, nMaxCount, pPipe) )
		{
			wchar_t outChar[nMaxCount + 1] = { 0 };
			MultiByteToWideChar(CodePage, 0, szBuffer, -1, outChar, nMaxCount );
			FString szProcess = outChar;
			AsyncTask(ENamedThreads::GameThread, [Process = Process, szProcess]()
			{
				Process.ExecuteIfBound(szProcess);
			});
			
			szResult += szProcess;
			memset(szBuffer, 0, sizeof(szBuffer));
		}
	
		_pclose(pPipe);
		AsyncTask(ENamedThreads::GameThread, [DTCmdDelayAction = DTCmdDelayAction, szResult]()
		{
			FScopeLock Lock(&GDelayActionCS);
			if ( GDelayAction.Find(DTCmdDelayAction) != INDEX_NONE )
			{
				DTCmdDelayAction->OnOver(szResult);
			}
		});
	}
};

UDTCmdBPLibrary::UDTCmdBPLibrary(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UDTCmdBPLibrary::ExecCMD(const UObject* WorldContextObject, const FString& Cmd, EDTCmdCharacter CodePage, bool ErrorOut, FString & Result, FLatentActionInfo LatentInfo)
{
	ExecCMDHaveProcess(WorldContextObject, Cmd, CodePage, ErrorOut, FDTCmdProcess(), Result, LatentInfo);
}

void UDTCmdBPLibrary::ExecCMDHaveProcess(const UObject* WorldContextObject, const FString& Cmd, EDTCmdCharacter CodePage, bool ErrorOut, const FDTCmdProcess Process, FString& Result, FLatentActionInfo LatentInfo)
{
	UINT uCodePage = CP_OEMCP;
	switch (CodePage)
	{
	case EDTCmdCharacter::ACP:
		uCodePage = CP_ACP;
		break;
	case EDTCmdCharacter::OEMCP:
		uCodePage = CP_OEMCP;
		break;
	case EDTCmdCharacter::MACCP:
		uCodePage = CP_MACCP;
		break;
	case EDTCmdCharacter::THREAD_ACP:
		uCodePage = CP_THREAD_ACP;
		break;
	case EDTCmdCharacter::SYMBOL:
		uCodePage = CP_SYMBOL;
		break;
	case EDTCmdCharacter::UTF7:
		uCodePage = CP_UTF7;
		break;
	case EDTCmdCharacter::UTF8:
		uCodePage = CP_UTF8;
		break;
	}

	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
		FDTCmdDelayAction * pDTCmdDelayAction = LatentActionManager.FindExistingAction<FDTCmdDelayAction>(LatentInfo.CallbackTarget, LatentInfo.UUID);
		if ( pDTCmdDelayAction == nullptr )
		{
			pDTCmdDelayAction = new FDTCmdDelayAction(Result, LatentInfo);
			LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, pDTCmdDelayAction);
		}
		( new FAutoDeleteAsyncTask< FDTCmdAsyncTask >( ErrorOut, Cmd, uCodePage, Process, pDTCmdDelayAction ) )->StartBackgroundTask();
	}
}


