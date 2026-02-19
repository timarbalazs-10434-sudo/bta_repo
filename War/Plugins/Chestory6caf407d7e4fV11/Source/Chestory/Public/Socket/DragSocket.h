// Copyright 2024 Arkai Interactive. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "InventoryPluginStruct.h"
#include "DragSocket.generated.h"

/**
 * 
 */
UCLASS()
class CHESTORY_API UDragSocket : public UDragDropOperation
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DragItem", meta = (ExposeOnSpawn = "true"))
	int32 DraggedInventorySlot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DragItem", meta = (ExposeOnSpawn = "true"))
	int32 DraggedSocketSlot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DragItem", meta = (ExposeOnSpawn = "true"))
	FSocketItem DraggedSocket;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DragItem", meta = (ExposeOnSpawn = "true"))
	FItemDataInfo DraggedSocketInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DragItem")
	class UInventoryManagerComponent* DraggedInventoryManager;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DragItem", meta = (ExposeOnSpawn = "true"))
	ESlotType DraggedSlotType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DragItem", meta = (ExposeOnSpawn = "true"))
	FGuid DraggedInventoryId;

};
