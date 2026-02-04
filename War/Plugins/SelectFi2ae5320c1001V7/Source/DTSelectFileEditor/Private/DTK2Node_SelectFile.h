// Copyright 2022 Dexter.Wan. All Rights Reserved. 
// EMail: 45141961@qq.com

#pragma once

#include "CoreMinimal.h"
#include "K2Node.h"
#include "DTK2Node_SelectFile.generated.h"


USTRUCT()
struct FTypeStruct
{
	GENERATED_BODY()

	UPROPERTY()	FName		szShowInfoName;
	UPROPERTY() FName		szFileTypeName;
};

// 单选文件
UCLASS()
class DTSELECTFILEEDITOR_API UDTK2Node_SelectFile : public UK2Node
{
	GENERATED_UCLASS_BODY()

	// 节点数量
	UPROPERTY() TArray<FTypeStruct>					m_ArrayType;
	
public:
	// 创建输出节点
	virtual void CreateOutputPin();
	// 默认分组
	virtual FText GetMenuCategory() const override { return FText::FromString(TEXT("DT SelectFile")); }
	// 快捷键
	virtual FText GetKeywords() const override { return NSLOCTEXT("DT SelectFile", "Open File", "Open File"); }
	// 节点样式
	virtual bool IsNodePure() const override { return false; }
	// 节点初始化
	virtual void AllocateDefaultPins() override;
	// 右键菜单
	virtual void GetNodeContextMenuActions(class UToolMenu* Menu, class UGraphNodeContextMenuContext* Context) const override;
	// 返回添加界面
	virtual TSharedPtr<SGraphNode> CreateVisualWidget() override;

public:
	// 获取执行结束节点
	UEdGraphPin* GetThenPin() const;
	// 获取节点名称
	FName GetShowInfoName(int32 nIndex) const;
	// 获取节点名称
	FName GetFileTypeName(int32 nIndex) const;
	// 添加节点
	bool AddInputPin();
	// 删除节点
	void RemoveInputPin(UEdGraphPin* Pin);
};
