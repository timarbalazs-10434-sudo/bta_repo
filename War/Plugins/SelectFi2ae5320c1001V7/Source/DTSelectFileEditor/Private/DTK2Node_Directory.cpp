// Copyright 2022 Dexter.Wan. All Rights Reserved. 
// EMail: 45141961@qq.com


#include "DTK2Node_Directory.h"
#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "EdGraphSchema_K2.h"
#include "K2Node_CallFunction.h"
#include "K2Node_MakeMap.h"
#include "KismetCompiler.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "DTSelectFileBPLib.h"


// 构造函数
UDTK2Node_Directory::UDTK2Node_Directory(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

// 节点初始化
void UDTK2Node_Directory::AllocateDefaultPins()
{
	Super::AllocateDefaultPins();

	// 执行节点
	CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute);
	CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Then);

	// 输入节点
	CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_String, FName(TEXT("DialogTitle")));
	CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_String, FName(TEXT("DefaultDirectory")));
	CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Boolean, FName(TEXT("NewFolder")));
	
	// 输出节点
	CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_String, FName(TEXT("DirectoryPath")));
}

// 菜单注册
void UDTK2Node_Directory::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
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
void UDTK2Node_Directory::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	UEdGraphPin* ExecPin = GetExecPin();
	UEdGraphPin* ThenPin = GetThenPin();
	if (ExecPin && ThenPin)
	{
		// 创建函数回调
		FName FunctionName = GET_FUNCTION_NAME_CHECKED(UDTSelectFileBPLib, OpenDirectoryDialog);
		UK2Node_CallFunction* CallFuncNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
		CallFuncNode->FunctionReference.SetExternalMember(FunctionName, UDTSelectFileBPLib::StaticClass());
		CallFuncNode->AllocateDefaultPins();
		
		// 绑定参数节点
		CompilerContext.MovePinLinksToIntermediate(*(FindPinChecked(TEXT("DialogTitle"), EGPD_Input)), *(CallFuncNode->FindPinChecked(TEXT("DialogTitle"))));
		CompilerContext.MovePinLinksToIntermediate(*(FindPinChecked(TEXT("DefaultDirectory"), EGPD_Input)), *(CallFuncNode->FindPinChecked(TEXT("DefaultDirectory"))));
		CompilerContext.MovePinLinksToIntermediate(*(FindPinChecked(TEXT("NewFolder"), EGPD_Input)), *(CallFuncNode->FindPinChecked(TEXT("bNewFolder"))));
		CompilerContext.MovePinLinksToIntermediate(*(FindPinChecked(TEXT("DirectoryPath"), EGPD_Output)), *(CallFuncNode->FindPinChecked(TEXT("DirectoryPath"))));
		
		// 绑定执行节点
		CompilerContext.MovePinLinksToIntermediate(*ExecPin, *(CallFuncNode->GetExecPin()));
		CompilerContext.MovePinLinksToIntermediate(*ThenPin, *(CallFuncNode->GetThenPin()));
	}

	BreakAllNodeLinks();

}

// 获取执行结束节点
UEdGraphPin* UDTK2Node_Directory::GetThenPin() const
{
	UEdGraphPin* Pin = FindPin(UEdGraphSchema_K2::PN_Then);
	check(Pin == nullptr || Pin->Direction == EGPD_Output);
	return Pin;
}