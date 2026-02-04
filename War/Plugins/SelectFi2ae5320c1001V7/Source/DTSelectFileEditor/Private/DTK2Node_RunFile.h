// Copyright 2022 Dexter.Wan. All Rights Reserved. 
// EMail: 45141961@qq.com

#pragma once

#include "CoreMinimal.h"
#include "K2Node.h"
#include "DTK2Node_RunFile.generated.h"


// 单选文件
UCLASS()
class DTSELECTFILEEDITOR_API UDTK2Node_RunFile : public UK2Node
{
	GENERATED_UCLASS_BODY()

public:
	// 返回标题
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override { return NSLOCTEXT("DT SelectFile", "Run File", "Run File"); }
	// 返回标题提示
	virtual FText GetTooltipText() const override { return NSLOCTEXT("DT SelectFile", "Run The Input File Using The System Default Program", "Run The Input File Using The System Default Program"); }
	// 默认分组
	virtual FText GetMenuCategory() const override { return FText::FromString(TEXT("DT SelectFile")); }
	// 快捷键
	virtual FText GetKeywords() const override { return NSLOCTEXT("DT SelectFile", "OpenFile,RunFile", "OpenFile,RunFile"); }
	// 节点样式
	virtual bool IsNodePure() const override { return false; }
	// 节点初始化
	virtual void AllocateDefaultPins() override;

public:
	// 菜单注册
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	// 编译节点
	virtual void ExpandNode(class FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;

public:
	// 获取执行结束节点
	UEdGraphPin* GetThenPin() const;
};
