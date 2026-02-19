// Copyright 2024 Arkai Interactive. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryPluginStruct.h"
#include "DraggedItemSlot.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class CHESTORY_API UDraggedItemSlot : public UUserWidget
{
	GENERATED_BODY()
	

public : 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DraggedItemSlot", meta = (ExposeOnSpawn = "true"))
	int32 VisualInventorySlot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DraggedItemSlot", meta = (ExposeOnSpawn = "true"))
	FItemDataInfo VisualItemInformation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DraggedItemSlot")
	class UInventoryManagerComponent* InventoryManager;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DraggedItemSlot", meta = (ExposeOnSpawn = "true"))
	ESlotType VisualSlotType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DraggedItemSlot", meta = (ExposeOnSpawn = "true"))
	int32 VisualSocketSlot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DraggedItemSlot", meta = (ExposeOnSpawn = "true"))
	FGuid FromInventoryId;


	bool bIsDropped = true;
	bool bFromSocket = false;
	bool bDroppedOnSocket = false;

	virtual void NativeDestruct();



};
