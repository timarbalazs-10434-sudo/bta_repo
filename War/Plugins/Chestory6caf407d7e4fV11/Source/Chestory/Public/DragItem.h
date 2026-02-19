// Copyright 2024 Arkai Interactive. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "InventoryPluginStruct.h"
#include "DragItem.generated.h"

/**
 * 
 */
UCLASS()
class CHESTORY_API UDragItem : public UDragDropOperation
{
	GENERATED_BODY()

public : 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DragItem", meta = (ExposeOnSpawn = "true"))
		int32 DraggedInventorySlot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DragItem", meta = (ExposeOnSpawn = "true"))
		FItemDataInfo DraggedItemInformation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DragItem")
		class UInventoryManagerComponent* DraggedInventoryManager;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DragItem", meta = (ExposeOnSpawn = "true"))
		ESlotType DraggedSlotType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DragItem", meta = (ExposeOnSpawn = "true"))
		bool IsDraggedFromInventory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DragItem", meta = (ExposeOnSpawn = "true"))
		bool IsDraggedFromContainer;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DragItem", meta = (ExposeOnSpawn = "true"))
		FGuid DraggedFromInventoryId;
};
