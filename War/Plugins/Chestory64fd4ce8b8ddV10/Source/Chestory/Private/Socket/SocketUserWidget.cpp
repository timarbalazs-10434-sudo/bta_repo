// Copyright 2024 Arkai Interactive. All Rights Reserved.


#include "Socket/SocketUserWidget.h"
#include "InventoryManagerComponent.h"
#include "InventorySubsystem.h"
#include "Chestory.h"


#define LOCTEXT_NAMESPACE "UMG"

#if WITH_EDITOR
// Set Catagory in Widget Blueprint
const FText USocketUserWidget::GetPaletteCategory()
{
	return LOCTEXT("Inventory", "Inventory");
}
#endif

FReply USocketUserWidget::NativeOnPreviewMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply reply = Super::NativeOnPreviewMouseButtonDown(InGeometry, InMouseEvent);

	if (IsValid(InventoryManager))
	{
		if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
		{
			UE_LOG(LogInventory, Verbose, TEXT("Pressed on Socket"));
			LeftMouseButtonPressed();
			FKey Button = InventoryManager->bEnableHoldButton ? EKeys::LeftMouseButton : EKeys::RightMouseButton;
			reply.DetectDrag(TakeWidget(), Button);
		}
		else if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
		{
			RightMouseButtonPressed();

			return reply.Unhandled();
		}
	}
	else
	{
		return reply.Unhandled();
	}

	return reply;
}

void USocketUserWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);
	SetToolTip(nullptr);
	OutOperation = CreateDragSocket();
}

bool USocketUserWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
	if (IsValid(InventoryManager))
	{
		UDragItem* MyDragItem = Cast<UDragItem>(InOperation);
		if (IsValid(MyDragItem))
		{
			if (UDraggedItemSlot* DragVisuel = Cast<UDraggedItemSlot>(MyDragItem->DefaultDragVisual))
			{
				DragVisuel->bIsDropped = false;
				DragVisuel->bDroppedOnSocket = true;

				//UE_LOG(LogInventory, Warning, TEXT("Server_DepositSocketItem %d sub %d From : %d ID %s"), FromInventorySlot, FromSocketSlot, MyDragItem->DraggedInventorySlot, *MyDragItem->DraggedItemInformation.ID.ToString());
				InventoryManager->Server_DepositSocketItem(MyDragItem->DraggedFromInventoryId, MyDragItem->DraggedInventorySlot, OwnerInventoryId, FromInventorySlot, FromSocketSlot);
				return true;
			}
			
		}
		else if (UDragSocket* DragSocket = Cast<UDragSocket>(InOperation))
		{
			if (UDraggedItemSlot* DragVisuel = Cast<UDraggedItemSlot>(DragSocket->DefaultDragVisual))
			{
				DragVisuel->bIsDropped = false;
				DragVisuel->bDroppedOnSocket = true;
			}
			//UE_LOG(LogInventory, Warning, TEXT("Server_MoveSocketItem"));
			InventoryManager->Server_MoveSocketItem(DragSocket->DraggedInventoryId, DragSocket->DraggedInventorySlot, DragSocket->DraggedSocketSlot, OwnerInventoryId, FromInventorySlot, FromSocketSlot);
			return true;
		}
	}
//	InventorySlotUserWidget->PirorityToSocket = true;
	return false;
}

UDragSocket* USocketUserWidget::CreateDragSocket()
{
	if (SocketInfo.PrimaryAssetId.IsValid())
	{
		if (IsValid(InventoryManager))
		{
			if (InventoryManager->WidgetDragSlot != NULL)
			{
				UDragSocket* Drag_Drop_OperationSocket = NewObject<UDragSocket>();
				Drag_Drop_OperationSocket->DraggedInventoryManager = InventoryManager;
				Drag_Drop_OperationSocket->DraggedInventorySlot = FromInventorySlot;
				Drag_Drop_OperationSocket->DraggedSlotType = FromType;
				Drag_Drop_OperationSocket->DraggedSocket = SocketInfo;
				Drag_Drop_OperationSocket->DraggedSocketSlot = FromSocketSlot;
				Drag_Drop_OperationSocket->DraggedSocketInfo = SocketItemInfo;
				Drag_Drop_OperationSocket->DraggedInventoryId = GetOwnerInventoryId();

				TSubclassOf<UDraggedItemSlot> WidgetDragItem = InventoryManager->WidgetDragSlot;
				UDraggedItemSlot* DragVisuel = CreateWidget<UDraggedItemSlot>(GetWorld(), WidgetDragItem);

				DragVisuel->VisualInventorySlot = FromInventorySlot;
				DragVisuel->VisualItemInformation = SocketItemInfo;
				DragVisuel->InventoryManager = InventoryManager;
				DragVisuel->VisualSlotType = FromType;
				DragVisuel->bFromSocket = true;
				DragVisuel->VisualSocketSlot = FromSocketSlot;
				DragVisuel->FromInventoryId = GetOwnerInventoryId();

				Drag_Drop_OperationSocket->DefaultDragVisual = DragVisuel;
				Drag_Drop_OperationSocket->Pivot = EDragPivot::MouseDown;

				return Drag_Drop_OperationSocket;

				
			}
			else
			{
				UE_LOG(LogInventory, Warning, TEXT("UI  No WidgetDragSlot in Inventory Manager "));
			}
		}
	}

	return nullptr;
}
