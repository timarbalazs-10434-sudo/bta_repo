// Copyright 2022 Dexter.Wan. All Rights Reserved. 
// EMail: 45141961@qq.com


#include "DTK2Node_RunFile.h"
#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "EdGraphSchema_K2.h"
#include "K2Node_CallFunction.h"
#include "K2Node_MakeMap.h"
#include "KismetCompiler.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "DTSelectFileBPLib.h"


// 构造函数
UDTK2Node_RunFile::UDTK2Node_RunFile(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

// 节点初始化
void UDTK2Node_RunFile::AllocateDefaultPins()
{
	Super::AllocateDefaultPins();

	// 执行节点
	CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute);
	CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Then);

	// 输入节点
	CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_String, FName(TEXT("FilePath")));
}

// 菜单注册
void UDTK2Node_RunFile::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	UClass* ActionKey = GetClass();

	if (ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
		check(NodeSpawner != nullptr);

		ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
	}
}

// 编译节点
void UDTK2Node_RunFile::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	UEdGraphPin* ExecPin = GetExecPin();
	UEdGraphPin* ThenPin = GetThenPin();
	if (ExecPin && ThenPin)
	{
		// 创建函数回调
		FName FunctionName = GET_FUNCTION_NAME_CHECKED(UDTSelectFileBPLib, RunFile);
		UK2Node_CallFunction* CallFuncNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
		CallFuncNode->FunctionReference.SetExternalMember(FunctionName, UDTSelectFileBPLib::StaticClass());
		CallFuncNode->AllocateDefaultPins();
		
		// 绑定参数节点
		CompilerContext.MovePinLinksToIntermediate(*(FindPinChecked(TEXT("FilePath"), EGPD_Input)), *(CallFuncNode->FindPinChecked(TEXT("FilePath"))));
		
		// 绑定执行节点
		CompilerContext.MovePinLinksToIntermediate(*ExecPin, *(CallFuncNode->GetExecPin()));
		CompilerContext.MovePinLinksToIntermediate(*ThenPin, *(CallFuncNode->GetThenPin()));
	}

	BreakAllNodeLinks();

}

// 获取执行结束节点
UEdGraphPin* UDTK2Node_RunFile::GetThenPin() const
{
	UEdGraphPin* Pin = FindPin(UEdGraphSchema_K2::PN_Then);
	check(Pin == nullptr || Pin->Direction == EGPD_Output);
	return Pin;
}