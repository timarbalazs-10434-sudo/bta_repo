// Copyright 2022 Dexter.Wan. All Rights Reserved. 
// EMail: 45141961@qq.com

#pragma once
#include "KismetNodes/SGraphNodeK2Base.h"

class SDTGraphNodeFileType : public SGraphNodeK2Base
{

public:

	SLATE_BEGIN_ARGS(SDTGraphNodeFileType){}
	SLATE_END_ARGS()

public:
	// 输入文本
	FString					StrText;

public:
	// 初始化
	void Construct(const FArguments& InArgs, class UDTK2Node_SelectFile * InNode);
	// 创建菜单
	virtual void CreateInputSideAddButton(TSharedPtr<SVerticalBox> InputBox) override;
	// 添加节点
	virtual FReply OnAddPin() override;
	// 文本改变
	void OnTextChanged(const FText& InNewText);
};
