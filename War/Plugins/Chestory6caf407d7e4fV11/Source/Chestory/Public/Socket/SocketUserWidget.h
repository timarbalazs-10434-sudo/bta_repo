// Copyright 2024 Arkai Interactive. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InventoryPluginStruct.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/DragDropOperation.h"
#include "Components/SlateWrapperTypes.h"
#include "DragSocket.h"
#include "SocketUserWidget.generated.h"


class UInventorySlotUserWidget;
/**
 * 
 */
UCLASS()
class CHESTORY_API USocketUserWidget : public UUserWidget
{
	GENERATED_BODY()
	

public:

	FSocketItem SocketInfo;
	FItemDataInfo SocketItemInfo;
	ESlotType FromType;
	int32 FromInventorySlot;
	int32 FromSocketSlot;
	FGameplayTagContainer SocketRestrictionSlot;
	UInventorySlotUserWidget* InventorySlotUserWidget;
	class UInventoryManagerComponent* InventoryManager;
	class UInventorySubsystem* InventorySubsystem;


	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Socket")
	FItemDataInfo GetSocketItemInformation() {return SocketItemInfo; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Socket")
	UInventoryManagerComponent* GetInventoryManager() { return InventoryManager; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Socket")
	ESlotType GetFromTypeInventory() { return FromType; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Socket")
	int32 GetFromInventorySlot() { return FromInventorySlot; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Socket")
	int32 GetSocketSlot() { return FromSocketSlot; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Socket")
	UInventorySlotUserWidget* GetParentInventorySlotUserWidget() { return InventorySlotUserWidget; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Socket")
	FGameplayTagContainer GetSocketRestrictionSlot() { return SocketRestrictionSlot; }


	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Event")
	void SocketUpdate(const FSocketItem& Socket);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Socket")
	FGuid GetOwnerInventoryId() { return OwnerInventoryId; }

	void SetOwnerInventoryId(FGuid NewInventoryId) {OwnerInventoryId = NewInventoryId;}

	// Event Call if in InventoryManager the variable RightMouseButtonForUseItem is false;
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Event")
	void RightMouseButtonPressed();

	// Event Call if in InventoryManager the variable LeftMouseButtonForUseItem is false;
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Event")
	void LeftMouseButtonPressed();
	

#if WITH_EDITOR
		virtual const FText GetPaletteCategory() override;
#endif



	virtual FReply NativeOnPreviewMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent);
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation);
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation);
	//virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent);
	//virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent);


	UDragSocket* CreateDragSocket();

private : 

	FGuid OwnerInventoryId;
};
