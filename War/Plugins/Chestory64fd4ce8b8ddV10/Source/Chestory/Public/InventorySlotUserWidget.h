// Copyright 2024 Arkai Interactive. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InventoryPluginStruct.h"
//#include "Blueprint/UserWidget.h"
#include "Blueprint/DragDropOperation.h"
#include "Components/SlateWrapperTypes.h"
//#include "InventoryManagerComponent.h"
#include "Styling/StarshipCoreStyle.h"
#include "DragItem.h"
#include "DraggedItemSlot.h"
#include "Socket/SocketUserWidget.h"

#include "InventorySlotUserWidget.generated.h"



static FSlateBrush OverridenFocusBrush;

class UInventorySubsystem;

/**
 * 
 */
UCLASS(Abstract)
class CHESTORY_API UInventorySlotUserWidget : public UUserWidget
{
	//GENERATED_BODY()
	GENERATED_UCLASS_BODY()

	virtual void SynchronizeProperties();


public:
	
//	UInventorySlotUserWidget(const FObjectInitializer& ObjectInitializer);

	
	/*The type working with Inventory Manager*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory", meta = (DisplayPriority = "1"))
	ESlotType Type;

	/*
	* Indicated the context that will be used at the opening of the equipment
	* Link the inventory to a Slot group
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory", meta = (EditCondition = "Type == ESlotType::EEquipement", DisplayPriority = "1"))
	FName Context = FName("DefaultContext");


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory", meta = (EditCondition = "Type == ESlotType::EEquipement || Type == ESlotType::EHotBar", DisplayPriority = "2"))
	FName SlotKey = FName("DefaultSlot");

	// Slot for Hotbar Index	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory", meta = (EditCondition = "Type == ESlotType::EHotBar", DisplayPriority = "3"))
	int32 InventorySlot;

	// Indicated the panel where the sockets will be created
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sockets Panel", meta = (BindWidgetOptional))
	UPanelWidget* PanelSockets;

	/*Item infromation given by client */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	FItemDataInfo ItemInformation;

	//id linked inventory
	FGuid OwnerInventoryId;

	UFUNCTION(BlueprintCallable,BlueprintPure,Category = "Inventory")
	FGuid GetOwnerInventoryId() { return OwnerInventoryId; }

	class UInventoryUniformGridPanel* OwnerInventoryGrid = nullptr;
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	class UInventoryUniformGridPanel* GetOwnerInventoryGrid() { return OwnerInventoryGrid; }

	bool OwnerInventoryUseSizeItem = false;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	bool GetOwnerInventoryUseSizeItem() { return OwnerInventoryUseSizeItem; }

	void SetItem(FGuid InventoryId,FItemDataInfo NewData);
	void ClearItem(FGuid InventoryId);
	void SetDecay(FGuid InventoryId, float Decay, float OnDecayFactor);
	void SetRepair(FItemDataInfoCraft NewRepair);
	void SetDurability(float Durability);
	void SetBlueprint(FItemDataInfoCraft NewBlueprint);
	void SetAttributes(FGameplayTagAttributeContainer Attributes);
	void SetGameplayTag(FGameplayTag Tag, bool IsAdd);
	void SetJsonParameter(FString JsonParameter);

	FPrimaryAssetId PrimaryAssetId;

	FItemDataInfoCraft ItemRepairInformation;

	FItemDataInfoCraft ItemBlueprintInformation;

	TArray<FSocketItem> SocketsInfo;
	TArray<USocketUserWidget*> SocketsWidget;

	void AddSocketSlot(FSocketItem Socket);
	void SetSocketInformation(FGuid InventoryID,FSocketItem Socket, int32 SocketSlot);
	void ClearSocket(int32 SocketSlot);
	void ClearSockets();

	bool CanDepositeSocket(int32& SocketSlot);

	bool bCanRepair;

	class UInventoryManagerComponent* InventoryManager;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	UInventoryManagerComponent* GetInventoryManager() { return InventoryManager; };

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	FItemDataInfo GetItemInformationSlot() { return ItemInformation; };
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	FItemDataInfoCraft GetItemRepairInformationSlot() { return ItemRepairInformation; };

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	FItemDataInfoCraft GetItemBlueprintInformationSlot() { return ItemBlueprintInformation; };

	void SetCanRepair(bool CanRepair) {bCanRepair = CanRepair;};

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	bool GetCanRepair() const {return bCanRepair;};

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	bool GetIsFilled() { return ItemInformation.bIsFilled; };

	// Called event when ItemInformation variable has changed 
	// @Function GetItemInformationSlot return new value 
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Event")
	void UpdateItemInformation(FItemDataInfo ItemInfo);

	// Called event when Hover on Item check if repair is possible
	// @Function GetItemRepairInformationSlot return new value 
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Event")
	void UpdateItemRepairInformation(FItemDataInfoCraft RepairInfo);

	// Called event when Hover on Item check is Blueprint Type
	// @Function GetItemBlueprintInformationSlot return new value 
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Event")
	void UpdateItemBlueprintInformation(FItemDataInfoCraft BlueprintInfo);


	UFUNCTION(BlueprintImplementableEvent, Category = "Event")
	void OnDurabilityChanged(const float & NewDurability);

	UFUNCTION(BlueprintImplementableEvent, Category = "Event")
	void OnAttributesChanged(const FGameplayTagAttributeContainer& Attributes);

	UFUNCTION(BlueprintImplementableEvent, Category = "Event")
	void OnGameplayTagContainerAdd(const FGameplayTag& Tag);

	UFUNCTION(BlueprintImplementableEvent, Category = "Event")
	void OnGameplayTagContainerRemove(const FGameplayTag& Tag);

	UFUNCTION(BlueprintImplementableEvent, Category = "Event")
	void OnJsonParameterChanged(const FString& JsonParameter);

	UFUNCTION(BlueprintImplementableEvent, Category = "Event")
	void OnDecayChanged(const float & NewDecay,const float & DecayFactor);


	UFUNCTION(BlueprintImplementableEvent, Category = "Event")
	void OnSocketsChanged(const FSocketItem& Socket, const int32& SocketSlot);

	UFUNCTION(BlueprintImplementableEvent, Category = "Event")
	void OnItemUsed(EUsedInputTrigger Trigger);

	UFUNCTION(BlueprintImplementableEvent, Category = "Event")
	void OnEnableSlot();

	UFUNCTION(BlueprintImplementableEvent, Category = "Event")
	void OnDisableSlot();
	
	// Called event when inventory  data has changed 
	// this event can be called by all inventory type
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Event")
	void InventoryHasUpdate();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Event")
	void OnMouseEnterSlot();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Event")
	void OnMouseLeaveSlot();

	// Event Call if in InventoryManager the variable RightMouseButtonForUseItem is false;
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Event")
	void RightMouseButtonPressed();

	// Event Call if in InventoryManager the variable LeftMouseButtonForUseItem is false;
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Event")
	void LeftMouseButtonPressed();

	UDraggedItemSlot* Drag;

	TSubclassOf<UDraggedItemSlot> WidgetDragItem;

	UDragItem* CreateItemDrag();

	TArray<UInventorySlotUserWidget*> GetOverSizeSlots(FIntPoint Size);
	void UpdateVisbilityFilled(bool IsVisible, bool IgnoreSelf = true);


	#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override;
	#endif

	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent);
	virtual FReply NativeOnPreviewMouseButtonDown(const FGeometry & InGeometry, const FPointerEvent & InMouseEvent);
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation);
	virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation);
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation);
	virtual void NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation);
	virtual void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation);

	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent);
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent);

	virtual FReply NativeOnMouseButtonDoubleClick( const FGeometry& InGeometry, const FPointerEvent& InMouseEvent );

	virtual void NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent);
	virtual void NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent);



	UInventorySubsystem* InventorySubsystem;
	void InitializeSubsystem();

	virtual void NativeOnInitialized();

	int32 GetAmountToSplit(FItemDataInfo ItemToSplit);
	void SetCurrentHover();
	void ClearCurrentHover();


	//UFUNCTION(BlueprintCallable, Category = "Inventory | Slate Overrides")
	//void SetFocusBrush(FSlateBrush InBrush) {
	//	OverridenFocusBrush = InBrush;
	//	FStarshipCoreStyle::SetFocusBrush(&OverridenFocusBrush);
	//}

};
