// Copyright 2022 Dexter.Wan. All Rights Reserved. 
// EMail: 45141961@qq.com

#pragma once

#include "CoreMinimal.h"
#include "DTK2Node_SelectFile.h"
#include "DTK2Node_LoadFile.generated.h"

// 单选文件
UCLASS()
class DTSELECTFILEEDITOR_API UDTK2Node_LoadFile : public UDTK2Node_SelectFile
{
	GENERATED_UCLASS_BODY()
	
public:
	// 返回标题
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override { return NSLOCTEXT("DT SelectFile", "Open Load File Window", "Open Load File Window"); }
	// 返回标题提示
	virtual FText GetTooltipText() const override { return NSLOCTEXT("DT SelectFile", "Open The Read File Selection Window, Return To Select File Path", "Open The Read File Selection Window, Return To Select File Path"); }

public:
	// 菜单注册
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	// 编译节点
	virtual void ExpandNode(class FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
};
