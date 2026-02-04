// Copyright 2024 Arkai Interactive. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/UniformGridPanel.h"
#include "InventoryPluginStruct.h"
#include "InventoryUniformGridPanel.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FContainerOpen, FContainerSetting, ContainerSetting);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FContainerClose);


class UInventorySubsystem;

/**
 * 
 */
UCLASS(Category = "Inventory")
class CHESTORY_API UInventoryUniformGridPanel : public UUniformGridPanel
{
	GENERATED_BODY()

	UInventoryUniformGridPanel(const FObjectInitializer& ObjectInitializer);
	virtual void SynchronizeProperties();
	virtual void RemoveFromParent();
#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override;
#endif

	

public:



	UInventorySubsystem* InventorySubsystem;
	void InitializeSubsystem();

	void UpdatePanelOnSubsystem();

	void ClearPanel();

	/*
	* If true, the RegirsterInventoryGrid function will be called automatically when the grid is created.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Grid Panel")
	bool bEnableAutoRegister = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Grid Panel")
	ESlotType Type = ESlotType::EInventory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Grid Panel")
	FName GridKey = "Default Grid";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Grid Panel")
	UWidget* WidgetParentVisibility = this;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Grid Panel")
	UPanelWidget* PanelCrafting = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Grid Panel")
	UPanelWidget* PanelWaitingList = nullptr;

	UPROPERTY(BlueprintAssignable, Category = "Inventory Grid Panel Event")
	FContainerOpen OnOpenContainer;

	UPROPERTY(BlueprintAssignable, Category = "Inventory Grid Panel Event")
	FContainerClose OnCloseContainer;

	// ----------------------Inventory

	FGuid InventoryId;



	void SetInventoryId(FGuid NewInventoryId) { InventoryId = NewInventoryId; }

	bool OwnerInventoryUseSizeItem = false;


	void SetMaxPerRow(int32 MaxRow) { MaxPerRow = MaxRow; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory Grid Panel")
	int32 GetMaxPerRow() { return MaxPerRow; }

	// Return the current InventoryId Linked to this grid
	UFUNCTION(BlueprintCallable,BlueprintPure, Category = "Inventory Grid Panel")
	FGuid GetInventoryId() { return InventoryId; }

	/*
	* Add this grid to the register, if OverrideGridKey is empty then use variable GridKey
	*/
	UFUNCTION(BlueprintCallable, Category = "Inventory Grid Panel")
	void RegisterInventoryGrid(FName OverrideGridKey);


	//Return Array of the inventorys slots
	TArray<class UInventorySlotUserWidget*> GetInventorySlots() { return InsideSlots; };
	UInventorySlotUserWidget* GetInventorySlot(int32 Index) { return InsideSlots[Index]; }
	void AddInventorySlot(UInventorySlotUserWidget* NewSlot, int32 Row, int32 Column);

	void DecreaseInventorySlots(int32 Amount);

	// ----------------------Crafting

	TArray<class UCraftingSlotUserWidget*> GetCraftingSlots() { return CraftingSlots; };
	UCraftingSlotUserWidget* GetCraftingSlot(int32 Index) { return CraftingSlots[Index]; }
	void AddCraftingSlot(UCraftingSlotUserWidget* NewSlot, bool AutoAddPanel = true);
	void RemoveCraftingSlot(int32 IndexSlotRemove, bool AutoAddPanel = true);
	void ClearCraftingSlots();

	//----------------------Waiting

	TArray<class UCraftingSlotUserWidget*> GetWaitingSlots() { return WaitingSlots; };
	UCraftingSlotUserWidget* GetWaitingSlot(int32 Index) { return WaitingSlots[Index]; }
	void AddWaitingSlot(UCraftingSlotUserWidget* NewSlot);
	void RemoveWaitingSlot(int32 Index);

	void FilledSlots(TArray<int32> Indexs);
	void UnfilledSlots(TArray<int32> Indexs);
	void CancelOverSlots(TArray<int32> Indexs);

	void UpdateIndexWaitingSlots();
	void ClearWaitingSlots();
	void StartCrafting(int32 Index, float TimeOfCraft);
	void UpdateCrafting(int32 Index, float CurrentTimer);
	void EndCrafting(int32 Index);


	void CloseContainer();

	// Save Variable Crafting And Waiting (Need to Private)
	TArray<class UCraftingSlotUserWidget*> CraftingSlots;
	TArray<class UCraftingSlotUserWidget*> WaitingSlots;

private:
	// Widget in UniformGridPanel 
	TArray<class UInventorySlotUserWidget*> InsideSlots;

	int32 MaxPerRow = 0;

};
