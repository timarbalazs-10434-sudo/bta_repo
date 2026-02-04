// Copyright 2024 Arkai Interactive. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemDataAsset.h"
#include "CraftingSlotUserWidget.generated.h"

class UInventorySubsystem;
class UCraftingComponent;
class UInventoryManagerComponent;

/**
 * 
 */
UCLASS(Abstract)
class CHESTORY_API UCraftingSlotUserWidget : public UUserWidget
{
	GENERATED_BODY()

#if WITH_EDITOR
		virtual const FText GetPaletteCategory() override;
#endif
	
public:

	/* If this widget is used for the waiting list */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
	bool bIsWaitingSlot = false;

	UPROPERTY(BlueprintReadOnly, Category = "Crafting",Meta = (ExposeOnSpawn = true))
	UItemDataAsset* ItemDataAsset;

	FItemDataInfoCraft ItemInformationCraft;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crafting")
	int32 IndexSlot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crafting")
	ESlotType Type;



	UCraftingComponent* CraftingSystem;
	UInventoryManagerComponent* InventoryManager;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	UInventoryManagerComponent* GetInventoryManager() { return InventoryManager; };

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	UCraftingComponent* GetCraftingSystem() { return CraftingSystem; };

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	UItemDataAsset* GetItemDataAsset() { return ItemDataAsset; };

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	FItemDataInfoCraft GetItemInformationCraftSlot() { return ItemInformationCraft; };

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Event")
	void UpdateCraftingData();
	virtual void UpdateCraftingData_Implementation();

	/*Event call only if is a WaitingSlot*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Event")
	void UpdateCraftingProgress(float CurrentTime);

	/*Event call only if is a WaitingSlot*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Event")
	void CraftingStart(float Time);

	/*Event call only if is a WaitingSlot*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Event")
	void CraftingEnd();


	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	FItemData GetItemData() { return ItemDataAsset->Item; };

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	int32 GetIndexSlot() { return IndexSlot; };


	UInventorySubsystem* InventorySubsystem;
	void InitializeSubsystem();

	virtual void NativeOnInitialized();


	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crafting")
	FGuid GetOwnerCraftingId() { return OwnerCraftingId; }

	void SetOwnerCraftingId(FGuid NewCraftingId) { OwnerCraftingId = NewCraftingId; };

	void InitCraft(FGuid CraftingId, int32 MySlot, UInventoryManagerComponent* Manger, bool IsWaiting, FItemDataInfoCraft DefaultValue = FItemDataInfoCraft());
	void SetCraft(FItemDataInfoCraft InfoCraft);
	
	void UpdateIngredientNeeded(const TArray<int32>& ItemCraftAmount, int32 MaxAmount);

private:

	UPROPERTY()
	FGuid OwnerCraftingId;

};
