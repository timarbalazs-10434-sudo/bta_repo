// Copyright 2022 Dexter.Wan. All Rights Reserved. 
// EMail: 45141961@qq.com


#include "DTK2Node_SelectFile.h"
#include "./GraphNode/DTGraphNodeFileType.h"
#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "EdGraphSchema_K2.h"
#include "K2Node_CallFunction.h"
#include "K2Node_MakeMap.h"
#include "KismetCompiler.h"
#include "Kismet2/BlueprintEditorUtils.h"


// 构造函数
UDTK2Node_SelectFile::UDTK2Node_SelectFile(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// 添加到数据保存
	FTypeStruct TypeStruct;
	TypeStruct.szShowInfoName = GetShowInfoName(0);
	TypeStruct.szFileTypeName = GetFileTypeName(0);
	m_ArrayType.Empty();
	m_ArrayType.Add(TypeStruct);
}

// 创建输出节点
void UDTK2Node_SelectFile::CreateOutputPin()
{
	// 输出节点
	CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_String, FName(TEXT("FilePath")));
}

// 节点初始化
void UDTK2Node_SelectFile::AllocateDefaultPins()
{
	Super::AllocateDefaultPins();

	// 执行节点
	CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute);
	CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Then);

	// 输入节点
	CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_String, FName(TEXT("DialogTitle")));
	CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_String, FName(TEXT("DefaultPath")));
	
	// 输入节点
	for (int32 n = 0; n < m_ArrayType.Num(); ++n)
	{
		const FTypeStruct & TypeStruct = m_ArrayType[n];
		UEdGraphPin* pShowInfo = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_String, TypeStruct.szShowInfoName);
		UEdGraphPin* pFileType = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_String, TypeStruct.szFileTypeName);
		if (n == 0)
		{
			pShowInfo->DefaultValue = TEXT("All File");
			pFileType->DefaultValue = TEXT("*.*");
		}
	}
	
	// 创建输出节点
	CreateOutputPin();
}

// 右键菜单
void UDTK2Node_SelectFile::GetNodeContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const
{
	Super::GetNodeContextMenuActions(Menu, Context);

	if (!Context->bIsDebugging)
	{
		FToolMenuSection& Section = Menu->AddSection("UDTK2NodeOpenFile", NSLOCTEXT("DT SelectFile", "OpenFile", "OpenFile"));
		if (Context->Pin)
		{
			if (Context->Pin->Direction == EGPD_Input && Context->Pin->ParentPin == nullptr)
			{
				Section.AddMenuEntry(
					"RemovePin",
					NSLOCTEXT("DT SelectFile", "RemovePin", "Remove Show Info/File Type Pair"),
					NSLOCTEXT("DT SelectFile", "RemovePinTooltip", "Remove This Pin And Its Corresponding Show Info/File Type Pin"),
					FSlateIcon(),
					FUIAction(
						FExecuteAction::CreateUObject(const_cast<UDTK2Node_SelectFile*>(this), &UDTK2Node_SelectFile::RemoveInputPin, const_cast<UEdGraphPin*>(Context->Pin))
					)
				);
			}
		}
	}
}

// 返回添加界面
TSharedPtr<SGraphNode> UDTK2Node_SelectFile::CreateVisualWidget()
{
	return SNew(SDTGraphNodeFileType, this);
}

// 获取执行结束节点
UEdGraphPin* UDTK2Node_SelectFile::GetThenPin() const
{
	UEdGraphPin* Pin = FindPin(UEdGraphSchema_K2::PN_Then);
	check(Pin == nullptr || Pin->Direction == EGPD_Output);
	return Pin;
}

// 获取节点名称
FName UDTK2Node_SelectFile::GetShowInfoName(int32 nIndex) const
{
	return *FString::Printf(TEXT("ShowInfo %d"), nIndex);
}

// 获取节点名称
FName UDTK2Node_SelectFile::GetFileTypeName(int32 nIndex) const
{
	return *FString::Printf(TEXT("FileType %d"), nIndex);
}

// 添加节点
bool UDTK2Node_SelectFile::AddInputPin()
{
	// 通知修改
	Modify();

	// 节点添加
	const UEdGraphPin* pShowInfo = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_String, GetShowInfoName(m_ArrayType.Num()));
	const UEdGraphPin* pFileType = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_String, GetFileTypeName(m_ArrayType.Num()));

	// 添加到数据保存
	FTypeStruct TypeStruct;
	TypeStruct.szShowInfoName = pShowInfo->PinName;
	TypeStruct.szFileTypeName = pFileType->PinName;
	m_ArrayType.Add(TypeStruct);

	// 标记蓝图
	FBlueprintEditorUtils::MarkBlueprintAsModified(GetBlueprint());
	GetGraph()->NotifyGraphChanged();

	return true;
}

// 删除节点
void UDTK2Node_SelectFile::RemoveInputPin(UEdGraphPin* Pin)
{
	FScopedTransaction Transaction(NSLOCTEXT("DT SelectFile", "RemovePinTx", "RemovePin"));
	Modify();

	// 删除节点
	TFunction<void(UEdGraphPin*)> RemovePinLambda = [this, &RemovePinLambda](UEdGraphPin* PinToRemove)
	{
		if (PinToRemove == nullptr)
		{
			return;
		}
		
		for (int32 SubPinIndex = PinToRemove->SubPins.Num()-1; SubPinIndex >= 0; --SubPinIndex)
		{
			RemovePinLambda(PinToRemove->SubPins[SubPinIndex]);
		}

		int32 PinRemovalIndex = INDEX_NONE;
		if (Pins.Find(PinToRemove, PinRemovalIndex))
		{
			Pins.RemoveAt(PinRemovalIndex);
			PinToRemove->MarkAsGarbage();
		}
	};

	// 遍历节点删除
	for ( int n = 0; n < m_ArrayType.Num(); ++n )
	{
		const FTypeStruct & TypeStruct = m_ArrayType[n];
		if (TypeStruct.szShowInfoName == Pin->PinName || TypeStruct.szFileTypeName == Pin->PinName)
		{
			UEdGraphPin* pShowInfo = FindPinChecked(TypeStruct.szShowInfoName);
			UEdGraphPin* pFileType = FindPinChecked(TypeStruct.szFileTypeName);
			RemovePinLambda(pShowInfo);
			RemovePinLambda(pFileType);
			PinConnectionListChanged(pShowInfo);
			PinConnectionListChanged(pFileType);
			m_ArrayType.RemoveAt(n);
			break;
		}
	}

	// 重置节点
	for ( int n = 0; n < m_ArrayType.Num(); ++n )
	{
		FTypeStruct & TypeStruct = m_ArrayType[n];
		UEdGraphPin* pShowInfo = FindPinChecked(TypeStruct.szShowInfoName);
		UEdGraphPin* pFileType = FindPinChecked(TypeStruct.szFileTypeName);
		pShowInfo->Modify();
		pFileType->Modify();
		TypeStruct.szShowInfoName = GetShowInfoName(n);
		TypeStruct.szFileTypeName = GetFileTypeName(n);
		pShowInfo->PinName = TypeStruct.szShowInfoName;
		pFileType->PinName = TypeStruct.szFileTypeName;
	}

	// 通知刷新
	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(GetBlueprint());
}

