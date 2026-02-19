// Copyright 2024 Arkai Interactive. All Rights Reserved.


#include "DraggedItemSlot.h"
#include "InventoryManagerComponent.h"
#include "ItemScript.h"

void UDraggedItemSlot::NativeDestruct()
{
	
	if (bIsDropped)
	{
		if (IsValid(InventoryManager))
		{
			if (bFromSocket)
			{
				//UE_LOG(LogTemp, Warning, TEXT("UI  DragVisual si destruct"));

				InventoryManager->Server_DropSocketItem(FromInventoryId, VisualInventorySlot, VisualSocketSlot);
			}
			else
			{
				InventoryManager->UseDropItemFromType(FromInventoryId, VisualInventorySlot);
			}
			
		}
	}
	
}


