// Copyright 2022 Dexter.Wan. All Rights Reserved. 
// EMail: 45141961@qq.com

#include "DTGraphNodeFileType.h"
#include "GraphEditorSettings.h"
#include "../DTK2Node_SelectFile.h"

// 初始化
void SDTGraphNodeFileType::Construct(const FArguments& InArgs, UDTK2Node_SelectFile* InNode)
{
	this->GraphNode = Cast<UEdGraphNode>(InNode);

	this->SetCursor( EMouseCursor::CardinalCross );

	this->UpdateGraphNode();
}


// 创建菜单
void SDTGraphNodeFileType::CreateInputSideAddButton(TSharedPtr<SVerticalBox> InputBox)
{
	// 添加按钮
	const TSharedRef<SButton> AddPinButton = SNew(SButton)
		.ContentPadding(0.0f)
		.ButtonStyle(FAppStyle::Get(), "NoBorder")
		.OnClicked(this, &SDTGraphNodeFileType::OnAddPin)
		.ToolTipText(NSLOCTEXT("DT SelectFile", "Add Select File Type", "Add Select File Type"))
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
				.AutoWidth()
				.HAlign(HAlign_Left)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("DT SelectFile", "Add File Type", "Add File Type"))
				.ColorAndOpacity(FLinearColor::White)
				]
			+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(7, 0, 0, 0)
				[
					SNew(SImage)
					.Image(FAppStyle::GetBrush(TEXT("Icons.PlusCircle")))
				]
		];
	AddPinButton->SetCursor(EMouseCursor::Hand);
	
	FMargin AddPinPadding = Settings->GetOutputPinPadding();
	AddPinPadding.Top += 6.0f;
	InputBox->AddSlot()
		.AutoHeight()
		.VAlign(VAlign_Center)
		.Padding(AddPinPadding)
		[
			AddPinButton
		];
}

// 添加节点
FReply SDTGraphNodeFileType::OnAddPin()
{
	UDTK2Node_SelectFile* BPNode = CastChecked<UDTK2Node_SelectFile>(GraphNode);
	if (BPNode != nullptr && BPNode->AddInputPin())
	{
		UpdateGraphNode();
	}

	return FReply::Handled();
}

// 文本改变
void SDTGraphNodeFileType::OnTextChanged(const FText& InNewText)
{
	StrText = InNewText.ToString();
}