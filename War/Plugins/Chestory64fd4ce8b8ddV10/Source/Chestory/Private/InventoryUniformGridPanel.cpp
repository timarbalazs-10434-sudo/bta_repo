// Copyright 2024 Arkai Interactive. All Rights Reserved.


#include "InventoryUniformGridPanel.h"
#include "InventorySubsystem.h"
#include "InventorySlotUserWidget.h"
#include "CraftingSlotUserWidget.h"
#include "Chestory.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
//#include "InventoryPluginStruct.h"

#define LOCTEXT_NAMESPACE "UMG"




#if WITH_EDITOR
const FText UInventoryUniformGridPanel::GetPaletteCategory()
{
	return LOCTEXT("Inventory", "Inventory");
}
#endif

UInventoryUniformGridPanel::UInventoryUniformGridPanel(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bIsVariable = true;
	InitializeSubsystem();
	
}


void UInventoryUniformGridPanel::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	if (!IsDesignTime())
	{
		InitializeSubsystem();


		//FString OutEnumValue = "Invalid";
		//OutEnumValue = UEnum::GetValueAsString(Type);
		//UE_LOG(LogInventory, Display, TEXT(" Add %s Grid"), *OutEnumValue);

		if (bEnableAutoRegister)
		{
			UpdatePanelOnSubsystem();
		}
	}
}


void UInventoryUniformGridPanel::RemoveFromParent()
{
	Super::RemoveFromParent();

	if (IsValid(InventorySubsystem))
	{
		switch (Type)
		{
		case ESlotType::EInventory:
		{
			InventorySubsystem->UnregisterInventoryGrid(GridKey);
			break;
		}
		default:
			break;
		}
	}

}




void UInventoryUniformGridPanel::InitializeSubsystem()
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

void UInventoryUniformGridPanel::UpdatePanelOnSubsystem()
{

		if (IsValid(InventorySubsystem))
		{
			InventorySubsystem->RegisterInventoryGrid(GridKey, this);
		}
		else
		{
			UE_LOG(LogInventory, Warning, TEXT("[InventoryUniformGridPanel] InventorySubSystem is NotValid can't init grid on SubSystem"));
		}
}

void UInventoryUniformGridPanel::ClearPanel()
{
	if (HasAnyChildren())
	{
		ClearChildren();
	}
	
	InventoryId = FGuid();

	for (auto slot : InsideSlots)
	{
		if (IsValid(slot))
		{
			slot->RemoveFromParent();
		}
		
	}

	InsideSlots.Empty();

	for (auto slot : CraftingSlots)
	{
		if (IsValid(slot))
		{
			slot->RemoveFromParent();
		}
		
	}
	CraftingSlots.Empty();

	for (auto slot : WaitingSlots)
	{
		if (IsValid(slot))
		{
			slot->RemoveFromParent();
		}
		
	}
	WaitingSlots.Empty();
}

void UInventoryUniformGridPanel::RegisterInventoryGrid(FName OverrideGridKey)
{
	if (!OverrideGridKey.IsNone())
	{
		GridKey = OverrideGridKey;
	}

	UpdatePanelOnSubsystem();
}

void UInventoryUniformGridPanel::AddInventorySlot(UInventorySlotUserWidget* NewSlot, int32 Row, int32 Column)
{
	if (!IsValid(NewSlot))
	{
		return;
	}

	InsideSlots.Add(NewSlot);

	AddChildToUniformGrid(NewSlot, Row, Column);
}

void UInventoryUniformGridPanel::DecreaseInventorySlots(int32 Amount)
{
	InsideSlots.RemoveAt(InsideSlots.Num() - Amount, Amount,EAllowShrinking::Yes);
	const int32 StartLoop = (InsideSlots.Num() - Amount);
	const int32 Endloop = InsideSlots.Num();

	for (int32 i = StartLoop; i < Endloop; i++)
	{
		RemoveChildAt(GetChildrenCount() - 1);
	}
}

void UInventoryUniformGridPanel::AddCraftingSlot(UCraftingSlotUserWidget* NewSlot, bool AutoAddPanel)
{
	CraftingSlots.Add(NewSlot);
	if(AutoAddPanel){ PanelCrafting->AddChild(NewSlot); }	
}

void UInventoryUniformGridPanel::RemoveCraftingSlot(int32 IndexSlotRemove, bool AutoAddPanel)
{
	if (CraftingSlots.IsValidIndex(IndexSlotRemove))
	{
		CraftingSlots.RemoveAt(IndexSlotRemove);
	}

	if (AutoAddPanel)
	{
		if (PanelCrafting->GetChildAt(IndexSlotRemove))
		{
			PanelCrafting->RemoveChildAt(IndexSlotRemove);
			PanelCrafting->InvalidateLayoutAndVolatility();
		}
	}
}

void UInventoryUniformGridPanel::ClearCraftingSlots()
{
	for (auto& C_Slot : GetCraftingSlots())
	{
		C_Slot->RemoveFromParent();
	}
	GetCraftingSlots().Empty();

	PanelCrafting->ClearChildren();
}

void UInventoryUniformGridPanel::AddWaitingSlot(UCraftingSlotUserWidget* NewSlot)
{
	WaitingSlots.Add(NewSlot);
	PanelWaitingList->AddChild(NewSlot);
}

void UInventoryUniformGridPanel::RemoveWaitingSlot(int32 Index)
{
	if (GetWaitingSlots().IsEmpty())
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryUniformGridPanel::RemoveWaitingSlot] WaitingSlots is Empty"));
		return;
	}

	if (!GetWaitingSlots().IsValidIndex(Index))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryUniformGridPanel::RemoveWaitingSlot] WaitingSlots Index:[%d] Invalid"), Index);
		return;
	}

	GetWaitingSlot(Index)->RemoveFromParent();
	WaitingSlots.RemoveAt(Index,EAllowShrinking::Yes);

	UpdateIndexWaitingSlots();
}

void UInventoryUniformGridPanel::FilledSlots(TArray<int32> Indexs)
{
	//if (!OwnerInventoryUseSizeItem) return;

	for (auto _Index : Indexs)
	{
		//UE_LOG(LogInventory, Warning, TEXT("[UInventoryUniformGridPanel::FilledSlots] Index:[%d] "), _Index);

		GetChildAt(_Index)->SetVisibility(ESlateVisibility::Hidden);
		if (InsideSlots.IsValidIndex(_Index))
		{
			if(IsValid(InsideSlots[_Index])) InsideSlots[_Index]->OnMouseLeaveSlot();			
		}
	}
}

void UInventoryUniformGridPanel::UnfilledSlots(TArray<int32> Indexs)
{
	//if (!OwnerInventoryUseSizeItem) return;

	for (auto _Index : Indexs)
	{
		//UE_LOG(LogInventory, Warning, TEXT("[UInventoryUniformGridPanel::UnfilledSlots] Index:[%d] "), _Index);
		GetChildAt(_Index)->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		if (InsideSlots.IsValidIndex(_Index))
		{
			if(IsValid(InsideSlots[_Index])) InsideSlots[_Index]->OnMouseLeaveSlot();
		}
	}
}

void UInventoryUniformGridPanel::CancelOverSlots(TArray<int32> Indexs)
{
	//if (!OwnerInventoryUseSizeItem) return;

	for (auto _Index : Indexs)
	{
		if (!InsideSlots.IsValidIndex(_Index))
		{
			continue;
		}

		if(UInventorySlotUserWidget* ChildSlot = InsideSlots[_Index])
		{
			//UE_LOG(LogInventory, Warning, TEXT("[UInventoryUniformGridPanel::CancelOverSlots] Index:[%d]"), _Index);
			ChildSlot->OnMouseLeaveSlot();
		}
	}
}


void UInventoryUniformGridPanel::UpdateIndexWaitingSlots()
{
	for (int32 i = 0; i < GetWaitingSlots().Num(); i++)
	{
		WaitingSlots[i]->IndexSlot = i;
		GetWaitingSlot(i)->UpdateCraftingData();
	}
}

void UInventoryUniformGridPanel::ClearWaitingSlots()
{
	for (auto& W_Slot : GetWaitingSlots())
	{
		W_Slot->RemoveFromParent();
	}
	GetWaitingSlots().Empty();

	PanelWaitingList->ClearChildren();
}

void UInventoryUniformGridPanel::StartCrafting(int32 Index, float TimeOfCraft)
{
	if (GetWaitingSlots().IsEmpty())
	{
		return;
	}

	if (!GetWaitingSlots().IsValidIndex(Index))
	{
		return;
	}

	GetWaitingSlot(Index)->CraftingStart(TimeOfCraft);
}

void UInventoryUniformGridPanel::UpdateCrafting(int32 Index, float CurrentTimer)
{
	if (GetWaitingSlots().IsEmpty())
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryUniformGridPanel::UpdateCrafting] WaitingSlots is Empty"));
		return;
	}

	if (!GetWaitingSlots().IsValidIndex(Index))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryUniformGridPanel::UpdateCrafting] WaitingSlots Index:[%d] Invalid"), Index);
		return;
	}

	GetWaitingSlot(Index)->UpdateCraftingProgress(CurrentTimer);
}

void UInventoryUniformGridPanel::EndCrafting(int32 Index)
{
	if (GetWaitingSlots().IsEmpty())
	{
		return;
	}

	if (!GetWaitingSlots().IsValidIndex(Index))
	{
		return;
	}

	GetWaitingSlot(Index)->CraftingEnd();
}

void UInventoryUniformGridPanel::CloseContainer()
{
	//Prepare data for cancel Drag
	if (IsValid(UWidgetBlueprintLibrary::GetDragDroppingContent()))
	{
		if (UDragItem* MyDragItem = Cast<UDragItem>(UWidgetBlueprintLibrary::GetDragDroppingContent()))
		{
			if(UDraggedItemSlot* DragVisuel = Cast<UDraggedItemSlot>(MyDragItem->DefaultDragVisual))
			{
				DragVisuel->bIsDropped = false;
			}
		}
	}
	
	//Cancel DragDrop
	UWidgetBlueprintLibrary::CancelDragDrop();
	OnCloseContainer.Broadcast();
}
