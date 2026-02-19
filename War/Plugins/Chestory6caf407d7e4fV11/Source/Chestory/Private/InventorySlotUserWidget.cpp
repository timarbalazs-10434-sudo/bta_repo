// Copyright 2024 Arkai Interactive. All Rights Reserved.


#include "InventorySlotUserWidget.h"
//#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/SlateWrapperTypes.h"
#include "Components/UniformGridSlot.h"
#include "InventoryManagerComponent.h"
#include "InventorySubsystem.h"
#include "Chestory.h"
#include "ItemScript.h"
#include "ChestoryFunctionLibrary.h"

#define LOCTEXT_NAMESPACE "UMG"

//

#if WITH_EDITOR
// Set Catagory in Widget Blueprint
const FText UInventorySlotUserWidget::GetPaletteCategory()
{
	return LOCTEXT("Inventory", "Inventory");
}
#endif



UInventorySlotUserWidget::UInventorySlotUserWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	//: UUserWidget(ObjectInitializer)
{
	SetIsFocusable(true);
	SetVisibility(ESlateVisibility::Visible);
	
#if WITH_EDITORONLY_DATA
	SetCategoryName("Inventory");
#endif
	
}



FReply UInventorySlotUserWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	FReply reply = Super::NativeOnKeyDown(InGeometry, InKeyEvent);

	//if (!IsValid(InventoryManager))
	//{
	//	return reply;

	//}

	//	UE_LOG(LogInventory, Warning, TEXT("Button Pressed:[%s]"), *InKeyEvent.GetKey().GetDisplayName().ToString())
	//	if (InKeyEvent.GetKey() == EKeys::H)
	//	{
	//		LeftMouseButtonPressed();
	//		InventoryManager->LMBPressedOnSlot();

	//		//OnDrag
	//		if (ItemInformation.bIsDraggable)
	//		{
	//			
	//			return reply.DetectDrag(TakeWidget(), EKeys::Gamepad_FaceButton_Left).ReleaseMouseCapture();
	//		}
	//	}


	return reply;
}

FReply UInventorySlotUserWidget::NativeOnPreviewMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply reply = Super::NativeOnPreviewMouseButtonDown(InGeometry, InMouseEvent);

	if (IsValid(InventoryManager))
	{
		if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
		{
			//UE_LOG(LogInventory, Warning, TEXT("UI  LeftMouseButtonDown"));
			//InventoryManager->PrintPlayerHasAuthority("Left Btn Pressed");
			// Get Value In Item not on DataAsset 
			UInventoryComponent* LocalInventory = InventorySubsystem->GetInventory(OwnerInventoryId);
			if (IsValid(LocalInventory))
			{
				LeftMouseButtonPressed();
				InventoryManager->LMBPressedOnSlot();

				//OnDrag
				if (ItemInformation.bIsDraggable)
				{
					FKey Button = InventoryManager->bEnableHoldButton ? EKeys::LeftMouseButton : EKeys::RightMouseButton;
					return reply.DetectDrag(TakeWidget(), Button).ReleaseMouseCapture();
				}

			}
			else
			{
				UE_LOG(LogInventory, Warning, TEXT("UI Slot Inventory Component link Invalid"));
			}
		}
		else if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
		{
			//UE_LOG(LogInventory, Warning, TEXT("UI  RightMouseButtonDown"));
			if (IsValid(InventoryManager))
			{
				if (InventoryManager->bRightMouseButtonForUseItem)
				{
					InventoryManager->UseInventoryItem(OwnerInventoryId, InventorySlot);
				}
				else
				{
					RightMouseButtonPressed();
					InventoryManager->RMBPressedOnSlot();
				}
			}

		}
		else
		{
			return reply.Handled();
			//return reply.Unhandled();
		}
	}
	
	//return FReply::Handled();
	return reply;
}

FReply UInventorySlotUserWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply reply =  Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);


	return reply;
}

void UInventorySlotUserWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);
	SetToolTip(nullptr);
	OutOperation = CreateItemDrag();
	OnMouseLeaveSlot();

	if (IsValid(InventoryManager))
	{
		InventoryManager->Server_UnfilledItemInventory(OwnerInventoryId, InventorySlot);
	}

	//UpdateVisbilityFilled(true);
}

void UInventorySlotUserWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragCancelled(InDragDropEvent, InOperation);

	if (UDragItem* MyDragItem = Cast<UDragItem>(InOperation))
	{
		//UpdateVisbilityFilled(false);

		if (IsValid(InventoryManager))
		{
			//InventoryManager->Server_FilledItemInventory(OwnerInventoryId, InventorySlot);
		}

		for (auto SlotFound : GetOverSizeSlots(MyDragItem->DraggedItemInformation.Size))
		{
			SlotFound->OnMouseLeaveSlot();
		}
	}
}

bool UInventorySlotUserWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

	if (IsValid(InventoryManager))
	{

		if (UDragItem* MyDragItem = Cast<UDragItem>(InOperation))
		{
			UDraggedItemSlot* DragVisuel = Cast<UDraggedItemSlot>(MyDragItem->DefaultDragVisual);

			//Re hide if the item do not mov
			if (DragVisuel->FromInventoryId == OwnerInventoryId && DragVisuel->VisualInventorySlot == InventorySlot)
			{
				UpdateVisbilityFilled(false);
			}


			// Check can be socket and if item enter is Socket
			if (IsValid(MyDragItem) &&
				(ItemInformation.bCanSockets &&
					ItemInformation.Sockets.Num() > 0 &&
					MyDragItem->DraggedItemInformation.ItemType == EItemType::ESocket))
			{
				int32 ToSkocketSlotFound = -1;
				if (CanDepositeSocket(ToSkocketSlotFound))
				{
					DragVisuel->bIsDropped = false;
					DragVisuel->bDroppedOnSocket = true;

					//UE_LOG(LogInventory, Warning, TEXT("Server_DepositSocketItem %d sub %d From : %d ID %s"), FromInventorySlot, FromSocketSlot, MyDragItem->DraggedInventorySlot, *MyDragItem->DraggedItemInformation.ID.ToString());
					InventoryManager->Server_DepositSocketItem(MyDragItem->DraggedFromInventoryId, MyDragItem->DraggedInventorySlot, OwnerInventoryId, InventorySlot, ToSkocketSlotFound);

					return true;
				}
			}


			if (IsValid(DragVisuel))
				//else if (IsValid(DragVisuel) && !(ItemInformation.bCanSockets && ItemInformation.Sockets.Num() > 0 && MyDragItem->DraggedItemInformation.ItemType == EItemType::ESocket))
			{

				DragVisuel->bIsDropped = false;

				// Get Client if Split is enable
				bool bIsShiftPressed = InventoryManager->bUseSplitOnDrag;

				InventoryManager->Server_MoveInventoryToInventory(MyDragItem->DraggedFromInventoryId, MyDragItem->DraggedInventorySlot, OwnerInventoryId, InventorySlot, bIsShiftPressed);

				return true;
			}


		}
		else if (UDragSocket* DragSocket = Cast<UDragSocket>(InOperation))
		{
			if (UDraggedItemSlot* DragVisuel = Cast<UDraggedItemSlot>(DragSocket->DefaultDragVisual))
			{
				DragVisuel->bIsDropped = false;


				if (!(DragSocket->DraggedSlotType == Type && DragSocket->DraggedInventorySlot == InventorySlot) && Type != ESlotType::EEquipement && !DragVisuel->bDroppedOnSocket)
				{
					UE_LOG(LogInventory, Verbose, TEXT("UI  Take Socket Item"));
					InventoryManager->Server_TakeSocketItem(DragSocket->DraggedInventoryId, DragSocket->DraggedInventorySlot, DragSocket->DraggedSocketSlot, OwnerInventoryId, InventorySlot);
				}

			}
		}
	}

	return true;
}

void UInventorySlotUserWidget::NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{

	Super::NativeOnDragEnter(InGeometry, InDragDropEvent, InOperation);


	if (UDragItem* MyDragItem = Cast<UDragItem>(InOperation))
	{
		for (auto SlotFound : GetOverSizeSlots(MyDragItem->DraggedItemInformation.Size))
		{
			SlotFound->OnMouseEnterSlot();
		}
	}
}

void UInventorySlotUserWidget::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragLeave(InDragDropEvent, InOperation);


	if (UDragItem* MyDragItem = Cast<UDragItem>(InOperation))
	{

		for (auto SlotFound : GetOverSizeSlots(MyDragItem->DraggedItemInformation.Size))
		{
			SlotFound->OnMouseLeaveSlot();
		}
	}
}



void UInventorySlotUserWidget::UpdateItemInformation_Implementation(FItemDataInfo ItemInfo)
{
	
	if (IsValid(InventoryManager) && IsValid(InventorySubsystem))
	{
		if (ItemInformation.IsValid() && IsValid(PanelSockets))
		{
			if (ItemInformation.bCanSockets)
			{
				//UE_LOG(LogInventory, Warning, TEXT("Try to create socket Widget"));
				if (ItemInformation.Sockets.IsEmpty())
				{
					//Ensure Clear
					ClearSockets();
				}
				else if (SocketsWidget.Num() == ItemInformation.Sockets.Num())
				{
					//Just Update Socket Info
					for (int32 i = 0; i < ItemInformation.Sockets.Num(); i++)
					{
						SetSocketInformation(GetOwnerInventoryId(), ItemInformation.Sockets[i], i);
					}
				}
				else
				{
					ClearSockets(); 


					for (auto NewSocket : ItemInformation.Sockets)
					{
						//UE_LOG(LogInventory, Warning, TEXT("Try to create socket Widget Item:[%s]"), *ItemInformation.ID.ToString());
						
						AddSocketSlot(NewSocket);
					}
				}
			}
			else if (PanelSockets->HasAnyChildren())
			{
				ClearSockets();
			}
		}
		else
		{
			if (GetIsFilled())
			{
				SetVisibility(ESlateVisibility::Hidden);
			}

			if (IsValid(PanelSockets))
			{
				//UE_LOG(LogInventory, Warning, TEXT("ID or Panel Invalid try to clear"));
				ClearSockets();
			}
		}
		
		
		//Update Hovered Data
		if (IsHovered())
		{
			SetCurrentHover();
			//UE_LOG(LogInventory, Warning, TEXT("Is Hovered In CPP Update Data"));
		}
		
	}
	//UE_LOG(LogInventory, Warning, TEXT("UpdateInformation In CPP"));
}


void UInventorySlotUserWidget::UpdateItemRepairInformation_Implementation(FItemDataInfoCraft RepairInfo)
{
	
}


void UInventorySlotUserWidget::UpdateItemBlueprintInformation_Implementation(FItemDataInfoCraft BlueprintInfo)
{
}

// Create Drag Component with VisualWidget 
UDragItem* UInventorySlotUserWidget::CreateItemDrag()
{
	if (ItemInformation.IsValid())
	{
		if (IsValid(InventoryManager))
		{
			if (InventoryManager->WidgetDragSlot != NULL)
			{
				auto drag_drop_operation = NewObject<UDragItem>();
				drag_drop_operation->DraggedInventorySlot = InventorySlot;
				drag_drop_operation->DraggedItemInformation = ItemInformation;
				drag_drop_operation->DraggedInventoryManager = InventoryManager;
				drag_drop_operation->DraggedSlotType = Type;
				drag_drop_operation->DraggedFromInventoryId = OwnerInventoryId;

				WidgetDragItem = InventoryManager->WidgetDragSlot;
				UDraggedItemSlot* DragVisuel = CreateWidget<UDraggedItemSlot>(GetWorld(), WidgetDragItem);

				DragVisuel->VisualInventorySlot = InventorySlot;
				DragVisuel->VisualItemInformation = ItemInformation;
				DragVisuel->InventoryManager = InventoryManager;
				DragVisuel->VisualSlotType = Type;
				DragVisuel->FromInventoryId = OwnerInventoryId;

				drag_drop_operation->DefaultDragVisual = DragVisuel;
				drag_drop_operation->Pivot = EDragPivot::TopLeft;
				drag_drop_operation->Offset = FVector2D(-0.2, -0.2);
				//UE_LOG(LogInventory, Warning, TEXT("UI  Drag IndexnSlot : %d"), InventorySlot);

				return drag_drop_operation;
			}
			else
			{
				UE_LOG(LogInventory, Warning, TEXT("UI  No WidgetDragSlot in Inventory Manager "));
			}
		
		}
	}

	return nullptr;
}

TArray<UInventorySlotUserWidget*> UInventorySlotUserWidget::GetOverSizeSlots(FIntPoint Size)
{

	TArray<UInventorySlotUserWidget*> OutData;

	if (UInventoryUniformGridPanel* ParentGrid = Cast<UInventoryUniformGridPanel>(GetParent()))
	{
		if (UUniformGridSlot* slot = Cast<UUniformGridSlot>(ParentGrid->GetChildAt(InventorySlot)->Slot))
		{
			//UE_LOG(LogInventory, Warning, TEXT("Enter x:%d y:%d"), MyDragItem->DraggedItemInformation.Size.X, MyDragItem->DraggedItemInformation.Size.Y);

			for (int32 x = 0; x < Size.X; x++)
			{
				for (int32 y = 0; y < Size.Y; y++)
				{
					//int32 IndexChild = UChestoryFunctionLibrary::GetIndexFromRowColumn(FIntPoint(slot->GetColumn() + x, slot->GetRow() + y), 5, InventorySlot);
					int32 MaxPerRow = ParentGrid->GetMaxPerRow();
					int32 X = slot->GetColumn() + x;
					int32 Y = slot->GetRow() + y;
					if (X > MaxPerRow - 1)
					{
						continue;
					}
					int32 IndexChild = Y * MaxPerRow + X;

					if (IndexChild > ParentGrid->GetChildrenCount())
					{
						continue;
					}

					//UE_LOG(LogInventory, Warning, TEXT("Debug Row:[%d] Column:[%d] Index:[%d]"), Y, X, IndexChild);

					if (UInventorySlotUserWidget* SlotWidget = Cast<UInventorySlotUserWidget>(ParentGrid->GetChildAt(IndexChild)))
					{
						OutData.Add(SlotWidget);
						if (!SlotWidget->GetOwnerInventoryUseSizeItem()) return OutData; //Just leave on first data if not use SizeItem
					}
				}

			}
		}
	}
	else{ OutData.Add(this);	return OutData;	} //Not in Grid return just slot

	return OutData;
}

void UInventorySlotUserWidget::UpdateVisbilityFilled(bool IsVisible, bool IgnoreSelf)
{
	for (auto SlotFound : GetOverSizeSlots(ItemInformation.Size))
	{
		if (IgnoreSelf)
		{
			if (SlotFound->InventorySlot != InventorySlot)
			{
				SlotFound->SetVisibility(IsVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);

			}
		}
		
	}
}


void UInventorySlotUserWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	SetCurrentHover();
	

	OnMouseEnterSlot();

}

void UInventorySlotUserWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	//SetToolTip(nullptr);

	ClearCurrentHover();
	OnMouseLeaveSlot();

}

FReply UInventorySlotUserWidget::NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	FReply reply = Super::NativeOnMouseButtonDoubleClick(InGeometry, InMouseEvent);

	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		if (ItemInformation.IsValid())
		{
			if (IsValid(InventoryManager); InventoryManager->LeftMouseButtonDoubleClickForUseItem)
			{
				InventoryManager->UseItemFromType(OwnerInventoryId,InventorySlot);
			}
		}
		//UE_LOG(LogInventory,Warning,TEXT("Double Click"));
	}


	return reply;
	
}

void UInventorySlotUserWidget::NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnAddedToFocusPath(InFocusEvent);
	SetCurrentHover();
	OnMouseEnterSlot();
}

void UInventorySlotUserWidget::NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnRemovedFromFocusPath(InFocusEvent);
	ClearCurrentHover();
	OnMouseLeaveSlot();
}

void UInventorySlotUserWidget::InitializeSubsystem()
{
	if (IsValid(GetOwningPlayer()))
	{
		UGameInstance* GameInstance = GetOwningPlayer()->GetGameInstance();

		if (IsValid(GameInstance))
		{
			InventorySubsystem = GameInstance->GetSubsystem<UInventorySubsystem>();
		}
	}
}

void UInventorySlotUserWidget::NativeOnInitialized()
{
	InitializeSubsystem();
}

int32 UInventorySlotUserWidget::GetAmountToSplit(FItemDataInfo ItemToSplit)
{

	if (IsValid(InventoryManager))
	{
		if (InventoryManager->bHalfSplitItem)
		{
			return (ItemToSplit.Amount / 2);

		}
		else
		{
			return InventoryManager->DefaultAmountToSplit;
		}
	}

	return int32();
}

void UInventorySlotUserWidget::SetCurrentHover()
{
	if (IsValid(InventoryManager))
	{
		FPrimaryAssetId HoverPrimaryAssetId = InventoryManager->InventorySubsystem->GetPrimaryAssetIdFromName(ItemInformation.ID);
		InventoryManager->SetCurrentItemOnMouseHover(ItemInformation.IsValid(), HoverPrimaryAssetId, ItemInformation, Type, InventorySlot,this,OwnerInventoryId);
	}
}

void UInventorySlotUserWidget::ClearCurrentHover()
{
	bCanRepair = false;
	
	if (IsValid(InventoryManager))
	{
		InventoryManager->SetCurrentItemOnMouseHover(false, FPrimaryAssetId(), ItemInformation, Type, InventorySlot,nullptr,FGuid());
	}
}

void UInventorySlotUserWidget::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	if (!IsDesignTime())
	{
		InitializeSubsystem();

		if (!IsValid(InventorySubsystem)) { return; }

		if (Type == ESlotType::EEquipement)
		{
			InventorySubsystem->RegisterEquipmentSlot(Context, SlotKey, this);
		}
		else if (Type == ESlotType::EHotBar)
		{
			InventorySubsystem->RegisterHotbarSlot(SlotKey, this);
		}
	}

}


void UInventorySlotUserWidget::SetItem(FGuid InventoryId, FItemDataInfo NewData)
{
	ItemInformation = NewData;
	OwnerInventoryId = InventoryId;


	//Check if is Filled
	SetVisibility(GetIsFilled() ? ESlateVisibility::Hidden : ESlateVisibility::Visible);
	if (ItemInformation.IsValid()) UE_LOG(LogInventory, Verbose, TEXT("[UInventorySlotUserWidget::SetItem] Set Item Information Item Id:[%s] Index:[%d]"), *ItemInformation.ID.ToString(), InventorySlot);
	UpdateItemInformation(NewData);
}

void UInventorySlotUserWidget::ClearItem(FGuid InventoryId)
{

	//for (auto SlotFound : GetOverSizeSlots(ItemInformation.Size))
	//{
	//	if (SlotFound->InventorySlot != InventorySlot)
	//	{
	//		SlotFound->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	//	}
	//}

	ItemInformation = FItemDataInfo();
	OwnerInventoryId = InventoryId;


	UpdateItemInformation(ItemInformation);
}

void UInventorySlotUserWidget::SetDecay(FGuid InventoryId, float Decay, float OnDecayFactor)
{
	ItemInformation.Decay = Decay;
	OnDecayChanged(Decay, OnDecayFactor);
}

void UInventorySlotUserWidget::SetRepair(FItemDataInfoCraft NewRepair)
{
	ItemRepairInformation = NewRepair;
	SetCanRepair(true);
	UpdateItemRepairInformation(NewRepair);
}

void UInventorySlotUserWidget::SetDurability(float Durability)
{
	ItemInformation.Durability = Durability;
	OnDurabilityChanged(Durability);
}

void UInventorySlotUserWidget::SetBlueprint(FItemDataInfoCraft NewBlueprint)
{
	ItemBlueprintInformation = NewBlueprint;
	UpdateItemBlueprintInformation(NewBlueprint);
}

void UInventorySlotUserWidget::SetAttributes(FGameplayTagAttributeContainer Attributes)
{
	ItemInformation.Attributes = Attributes;
	OnAttributesChanged(Attributes);
}

void UInventorySlotUserWidget::SetGameplayTag(FGameplayTag Tag, bool IsAdd)
{
	if (IsAdd)
	{
		ItemInformation.GameplayTagContainer.AddTag(Tag);
		OnGameplayTagContainerAdd(Tag);
	}
	else
	{
		ItemInformation.GameplayTagContainer.RemoveTag(Tag);
		OnGameplayTagContainerRemove(Tag);
	}

}

void UInventorySlotUserWidget::SetJsonParameter(FString JsonParameter)
{
	ItemInformation.JsonParameter = JsonParameter;
	OnJsonParameterChanged(JsonParameter);
}



void UInventorySlotUserWidget::AddSocketSlot(FSocketItem Socket)
{
	if (IsValid(InventoryManager) && IsValid(InventorySubsystem))
	{
		if (IsValid(PanelSockets))
		{
			TSubclassOf<USocketUserWidget> SocketClass = InventoryManager->SocketWidget;
			if(SocketClass != NULL)
			{
				USocketUserWidget* MySocket = CreateWidget<USocketUserWidget>(GetWorld(), SocketClass);
				MySocket->FromInventorySlot = InventorySlot;
				MySocket->FromType = Type;
				MySocket->FromSocketSlot = SocketsWidget.Num();
				MySocket->SocketRestrictionSlot = Socket.Restriction;
				MySocket->InventoryManager = InventoryManager;
				MySocket->InventorySubsystem = InventorySubsystem;
				//MySocket->OwnerInventoryId = OwnerInventoryId;
				MySocket->SetOwnerInventoryId(OwnerInventoryId);
				MySocket->InventorySlotUserWidget = this;
				MySocket->SocketInfo = Socket;
				MySocket->SocketItemInfo = UChestoryFunctionLibrary::SocketToItemInfo(GetWorld(), Socket);
				MySocket->SocketUpdate(Socket);

				SocketsWidget.Add(MySocket);
				PanelSockets->AddChild(MySocket);
			}
		}
	}
}

void UInventorySlotUserWidget::SetSocketInformation(FGuid InventoryID, FSocketItem Socket, int32 SocketSlot)
{
	ItemInformation.Sockets[SocketSlot] = Socket;

	if (SocketsWidget.IsValidIndex(SocketSlot))
	{
		if (IsValid(SocketsWidget[SocketSlot]))
		{
			SocketsWidget[SocketSlot]->SocketInfo = Socket;
			//SocketsWidget[SocketSlot]->SocketRestrictionSlot = Socket.Restriction;
			SocketsWidget[SocketSlot]->FromType = Type;
			SocketsWidget[SocketSlot]->FromInventorySlot = InventorySlot;
			SocketsWidget[SocketSlot]->InventoryManager = InventoryManager;
			SocketsWidget[SocketSlot]->SetOwnerInventoryId(InventoryID);
			SocketsWidget[SocketSlot]->SocketItemInfo = UChestoryFunctionLibrary::SocketToItemInfo(GetWorld(), Socket);

			SocketsWidget[SocketSlot]->SocketUpdate(Socket);
		}
	}

	OnSocketsChanged(Socket, SocketSlot);
}

void UInventorySlotUserWidget::ClearSocket(int32 SocketSlot)
{
	if (SocketsWidget.IsValidIndex(SocketSlot))
	{
		if (IsValid(SocketsWidget[SocketSlot]))
		{
			SocketsWidget[SocketSlot]->SocketInfo = FSocketItem();
			SocketsWidget[SocketSlot]->SocketItemInfo = FItemDataInfo();

			SocketsWidget[SocketSlot]->SocketUpdate(FSocketItem());
		}		
	}
}

void UInventorySlotUserWidget::ClearSockets()
{
	//Debug
	//if (ItemInformation.IsValid())
	//{
	//	UE_LOG(LogInventory, Warning, TEXT("ClearSockets socket Widget Item:[%s]"), *ItemInformation.ID.ToString());
	//}

	for (const auto Wsocket : SocketsWidget)
	{
		Wsocket->RemoveFromParent();
	}

	PanelSockets->ClearChildren();

	SocketsWidget.Empty();

}

bool UInventorySlotUserWidget::CanDepositeSocket(int32& SocketSlot)
{
	for (int32 i = 0; i < SocketsWidget.Num(); i++)
	{
		if (!SocketsWidget[i]->SocketItemInfo.IsValid())
		{
			SocketSlot = i;
			return true;
		}
	}

	return false;
}
