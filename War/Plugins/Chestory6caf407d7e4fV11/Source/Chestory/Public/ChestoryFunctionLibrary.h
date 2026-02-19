// Copyright 2024 Arkai Interactive. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "InventoryPluginStruct.h"
#include "InventorySubsystem.h"
#include "ItemScript.h"
#include "ChestoryFunctionLibrary.generated.h"


/**
 * 
 */
UCLASS()
class CHESTORY_API UChestoryFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

private:

	static UInventorySubsystem* GetInventorySubsystem(const UObject* WorldContextObject);

public:



	// ---------------------- GETTER FROM ITEM DATA ---------------------- 

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Data", meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true", DisplayName = "Get Primary Asset Id"))
	static UPARAM(DisplayName = "PrimaryAssetId") FPrimaryAssetId GetPrimaryAssetIdFromItemData(const UObject* WorldContextObject, FItemData Item);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Data", meta = (DisplayName = "Is Valid"))
	static UPARAM(DisplayName = "IsValid") bool GetIsValidFromItemData(FItemData Item) { return Item.IsValid(); }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Data", meta = (DisplayName = "Is Equipment"))
	static UPARAM(DisplayName = "IsEquipment") bool GetIsEquipmentFromItemData(FItemData Item) { return Item.IsEquipment(); }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Data", meta = (DisplayName = "Is Blueprint"))
	static UPARAM(DisplayName = "IsBlueprint") bool GetIsBlueprintFromItemData(FItemData Item) { return Item.IsBlueprint(); }
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Data", meta = (DisplayName = "Is Currency"))
	static UPARAM(DisplayName = "IsCurrency") bool GetIsCurrencyFromItemData(FItemData Item) { return Item.IsCurrency(); }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Data", meta = (DisplayName = "Is Socket"))
	static UPARAM(DisplayName = "IsSocket") bool GetIsSocketFromItemData(FItemData Item) { return Item.IsSocket(); }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Data", meta = (DisplayName = "Is Consumbale"))
	static UPARAM(DisplayName = "IsConsumbale") bool GetIsConsumbaleFromItemData(FItemData Item) { return Item.IsConsumbale(); }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Data")
	static UPARAM(DisplayName = "Unique ID") FGuid GetUniqueId(FItemData Item) { return Item.UniqueId; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Data")
	static UPARAM(DisplayName = "ID") FName GetID(FItemData Item) { return Item.ID; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Data")
	static UPARAM(DisplayName = "Icon") UTexture2D* GetIcon(FItemData Item) { return Item.Icon; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Data")
	static UPARAM(DisplayName = "Name") FText GetName(FItemData Item) { return Item.Name; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Data")
	static UPARAM(DisplayName = "Description") FText GetDescription(FItemData Item) { return Item.Description; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Data")
	static UPARAM(DisplayName = "ItemType") EItemType GetItemType(FItemData Item) { return Item.ItemType; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Data")
	static UPARAM(DisplayName = "GameplayTagContainer") FGameplayTagContainer GetGameplayTagContainer(FItemData Item) { return Item.GameplayTagContainer; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Data")
	static UPARAM(DisplayName = "Size") FIntPoint GetSize(FItemData Item) { return Item.Size; }

	/*
	* Return Total Weight (per unit * amount)
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Data")
	static UPARAM(DisplayName = "Weight") float GetWeight(FItemData Item) { return Item.GetWeight(); }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Data")
	static UPARAM(DisplayName = "Weight") float GetWeightPerUnit(FItemData Item) { return Item.Weight; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Data")
	static UPARAM(DisplayName = "bCanAttributes") bool GetCanAttributes(FItemData Item) { return Item.bCanAttributes; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Data")
	static UPARAM(DisplayName = "Attributes") FGameplayTagAttributeContainer GetAttributes(FItemData Item) { return Item.Attributes; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Data")
	static UPARAM(DisplayName = "bCanSockets") bool GetCanSockets(FItemData Item) { return Item.bCanSockets; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Data")
	static UPARAM(DisplayName = "Sockets") TArray<FSocketItem> GetSockets(FItemData Item) { return Item.Sockets; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Data")
	static UPARAM(DisplayName = "MaxSockets") int32 GetMaxSocket(FItemData Item) { return Item.MaxSocket; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Data")
	static UPARAM(DisplayName = "IsStackable") bool GetIsStackable(FItemData Item) { return Item.bIsStackable; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Data")
	static UPARAM(DisplayName = "IsDroppable") bool GetIsDroppable(FItemData Item) { return Item.bIsDroppable; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Data")
	static UPARAM(DisplayName = "IsDraggable") bool GetIsDraggable(FItemData Item) { return Item.bIsDraggable; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Data")
	static UPARAM(DisplayName = "IsVisible") bool GetIsVisible(FItemData Item) { return Item.bIsVisible; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Data")
	static UPARAM(DisplayName = "Amount") int32 GetAmount(FItemData Item) { return Item.GetAmount(); }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Data")
	static UPARAM(DisplayName = "JsonParameter") FString GetJsonParameter(FItemData Item) { return Item.JsonParameter; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Data")
	static UPARAM(DisplayName = "MaxStackable") int32 GetMaxStackable(FItemData Item) { return Item.MaxStackable; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Data")
	static UPARAM(DisplayName = "Durability") float GetDurability(FItemData Item) { return Item.Durability; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Data")
	static UPARAM(DisplayName = "MaxDurability") float GetMaxDurability(FItemData Item) { return Item.MaxDurability; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Data")
	static UPARAM(DisplayName = "CanRepairable") bool GetCanRepairable(FItemData Item) { return Item.bCanRepairable; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Data")
	static UPARAM(DisplayName = "CanDecay") bool GetCanDecay(FItemData Item) { return Item.bCanDecay; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Data")
	static UPARAM(DisplayName = "Decay") float GetDecay(FItemData Item) { return Item.Decay; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Data")
	static UPARAM(DisplayName = "DecayTime") float GetDecayTime(FItemData Item) { return Item.DecayTime; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Data")
	static UPARAM(DisplayName = "DecayItem") FPrimaryAssetId GetDecayItem(FItemData Item) { return Item.DecayItem; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Data")
	static UPARAM(DisplayName = "ActorDropped") TSubclassOf<class AActor> GetActorDropped(FItemData Item) { return Item.ActorDroped; }

	/*Actor Equipment spawn if equip*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Data")
	static UPARAM(DisplayName = "ActorInWorld") AActor* GetActorInWorld(FItemData Item) { return Item.ActorInWorld; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Data")
	static UPARAM(DisplayName = "FilledSlots") TArray<int32> GetFilledSlotsIndex(FItemData Item) { return Item.FilledSlotsIndex; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Data")
	static UPARAM(DisplayName = "LocationItem") void GetLocationItem(FItemData Item,FGuid& InventoryId, int32& InventorySlot) { InventoryId = Item.LocationItemInventoryId; InventorySlot = Item.LocationItemInventoryIndex; return; }
 
	// ---------------------- GETTER FROM ITEM DATA INFORMATION ---------------------- 

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Information", meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true", DisplayName = "Get Primary Asset Id"))
	static UPARAM(DisplayName = "PrimaryAssetId") FPrimaryAssetId GetPrimaryAssetIdFromInfo(const UObject* WorldContextObject, FItemDataInfo Item);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Information", meta = (DisplayName = "Is Valid"))
	static UPARAM(DisplayName = "IsValid") bool GetIsValidFromInfo(FItemDataInfo Item) { return Item.IsValid(); }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Data", meta = (DisplayName = "Is Equipment"))
	static UPARAM(DisplayName = "IsEquipment") bool GetIsEquipmentFromInfo(FItemDataInfo Item) { return Item.IsEquipment(); }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Data", meta = (DisplayName = "Is Blueprint"))
	static UPARAM(DisplayName = "IsBlueprint") bool GetIsBlueprintFromInfo(FItemDataInfo Item) { return Item.IsBlueprint(); }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Data", meta = (DisplayName = "Is Currency"))
	static UPARAM(DisplayName = "IsCurrency") bool GetIsCurrencyFromInfo(FItemDataInfo Item) { return Item.IsCurrency(); }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Data", meta = (DisplayName = "Is Socket"))
	static UPARAM(DisplayName = "IsSocket") bool GetIsSocketFromInfo(FItemDataInfo Item) { return Item.IsSocket(); }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Data", meta = (DisplayName = "Is Consumbale"))
	static UPARAM(DisplayName = "IsConsumbale") bool GetIsConsumbaleFromInfo(FItemDataInfo Item) { return Item.IsConsumbale(); }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Information", meta = (DisplayName = "Get ID"))
	static UPARAM(DisplayName = "ID") FName GetIDFromInfo(FItemDataInfo Item) { return Item.ID; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Information", meta = (DisplayName = "Get Icon"))
	static UPARAM(DisplayName = "Icon") UTexture2D* GetIconFromInfo(FItemDataInfo Item) { return Item.Icon; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Information", meta = (DisplayName = "Get Name"))
	static UPARAM(DisplayName = "Name") FText GetNameFromInfo(FItemDataInfo Item) { return Item.Name; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Information", meta = (DisplayName = "Get ItemType"))
	static UPARAM(DisplayName = "ItemType") EItemType GetItemTypeFromInfo(FItemDataInfo Item) { return Item.ItemType; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Information", meta = (DisplayName = "Get Gameplay Tag Container"))
	static UPARAM(DisplayName = "GameplayTagContainer") FGameplayTagContainer GetGameplayTagContainerFromInfo(FItemDataInfo Item) { return Item.GameplayTagContainer; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Information", meta = (DisplayName = "Get Size"))
	static UPARAM(DisplayName = "Size") FIntPoint GetSizeFromInfo(FItemDataInfo Item) { return Item.Size; }

	/*
	* Return Total Weight (per unit * amount)
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Information", meta = (DisplayName = "Get Weight"))
	static UPARAM(DisplayName = "Weight") float GetWeightFromInfo(FItemDataInfo Item) { return Item.GetWeight(); }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Information", meta = (DisplayName = "Get Weight Per Unit"))
	static UPARAM(DisplayName = "Weight") float GetWeightPerUnitFromInfo(FItemDataInfo Item) { return Item.Weight; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Information", meta = (DisplayName = "Get Is Filled"))
	static UPARAM(DisplayName = "Filled") bool GetIsFilledFromInfo(FItemDataInfo Item) { return Item.bIsFilled; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Information", meta = (DisplayName = "Get Can Sockets"))
	static UPARAM(DisplayName = "bCanSockets") bool GetCanSocketsFromInfo(FItemDataInfo Item) { return Item.bCanSockets; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Information", meta = (DisplayName = "Get Sockets"))
	static UPARAM(DisplayName = "Sockets") TArray<FSocketItem> GetSocketsFromInfo(FItemDataInfo Item) { return Item.Sockets; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Information", meta = (DisplayName = "Get Amount"))
	static UPARAM(DisplayName = "Amount") int32 GetAmountFromInfo(FItemDataInfo Item) { return Item.Amount; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Information", meta = (DisplayName = "Get Json Parameter"))
	static UPARAM(DisplayName = "JsonParameter") FString GetJsonParameterFromInfo(FItemDataInfo Item) { return Item.JsonParameter; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Information", meta = (DisplayName = "Get Is Stackable"))
	static UPARAM(DisplayName = "IsStackable") bool GetIsStackableFromInfo(FItemDataInfo Item) { return Item.bIsStackable; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Information", meta = (DisplayName = "Get Is Droppable"))
	static UPARAM(DisplayName = "IsDroppable") bool GetIsDroppableFromInfo(FItemDataInfo Item) { return Item.bIsDroppable; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Information", meta = (DisplayName = "Get Is Draggable"))
	static UPARAM(DisplayName = "IsDraggable") bool GetIsDraggableFromInfo(FItemDataInfo Item) { return Item.bIsDraggable; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Information", meta = (DisplayName = "Get Is Visible"))
	static UPARAM(DisplayName = "IsVisible") bool GetIsVisibleFromInfo(FItemDataInfo Item) { return Item.bIsVisible; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Information", meta = (DisplayName = "Get Can Decay"))
	static UPARAM(DisplayName = "CanDecay") bool GetCanDecayFromInfo(FItemDataInfo Item) { return Item.bCanDecay; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Information", meta = (DisplayName = "Get Decay"))
	static UPARAM(DisplayName = "Decay") float GetDecayFromInfo(FItemDataInfo Item) { return Item.Decay; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Information", meta = (DisplayName = "Get Decay Time"))
	static UPARAM(DisplayName = "Decay Time") float GetDecayTimeFromInfo(FItemDataInfo Item) { return Item.DecayTime; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Information", meta = (DisplayName = "Get Durability"))
	static UPARAM(DisplayName = "Durability") float GetDurabilityFromInfo(FItemDataInfo Item) { return Item.Durability; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Information", meta = (DisplayName = "Get Max Durability"))
	static UPARAM(DisplayName = "Max Durability") float GetMaxDurabilityFromInfo(FItemDataInfo Item) { return Item.MaxDurability; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Information", meta = (DisplayName = "Get Can Repairable"))
	static UPARAM(DisplayName = "Can Repairable") bool GetCanRepairableFromInfo(FItemDataInfo Item) { return Item.bCanRepairable; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Information", meta = (DisplayName = "Get Attributes"))
	static UPARAM(DisplayName = "Attributes") FGameplayTagAttributeContainer GetAttributesFromInfo(FItemDataInfo Item) { return Item.Attributes; }


	// ---------------------- GETTER FROM ITEM ---------------------- 

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item", meta = (DisplayName = "Get Primary Asset Id"))
	static UPARAM(DisplayName = "PrimaryAssetId") FPrimaryAssetId GetPrimaryAssetIdFromItem(FItem Item) { return Item.PrimaryAssetId; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item", meta = (DisplayName = "Get Amount"))
	static UPARAM(DisplayName = "Amount") int32 GetAmountFromItem(FItem Item) { return Item.Amount; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item")
	static UPARAM(DisplayName = "JsonParameter") FString GetJsonParameterFromItem(FItem Item) { return Item.JsonParameter; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item", meta = (DisplayName = "Get Index Slot"))
	static UPARAM(DisplayName = "IndexSlot") int32 GetIndexSlotFromItem(FItem Item) { return Item.IndexSlot; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item", meta = (DisplayName = "Get In Equipement"))
	static UPARAM(DisplayName = "InEquipement") bool GetInEquipementFromItem(FItem Item) { return Item.bInEquipement; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item", meta = (DisplayName = "Get In Hotbar"))
	static UPARAM(DisplayName = "InHotbar") bool GetInHotbarFromItem(FItem Item) { return Item.bInHotbar; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item", meta = (DisplayName = "Get Decay"))
	static UPARAM(DisplayName = "Decay") float GetDecayFromItem(FItem Item) { return Item.Decay; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item", meta = (DisplayName = "Get Durability"))
	static UPARAM(DisplayName = "Durability") float GetDurabilityFromItem(FItem Item) { return Item.Durability; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item", meta = (DisplayName = "Get Attributes"))
	static UPARAM(DisplayName = "Attributes") FGameplayTagAttributeContainer GetAttributesFromItem(FItem Item) { return Item.Attributes; }

	// ---------------------- GETTER FROM ITEM STARTER ---------------------- 

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Starter", meta = (DisplayName = "Get Items"))
	static UPARAM(DisplayName = "Items") TArray<FItem> GetItemsFromItemStarter(FItemStarter ItemStarter) { return ItemStarter.Items; }

	// ---------------------- GETTER FROM CONTAINER SETTING ---------------------- 

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Container Setting", meta = (DisplayName = "Get Name Container"))
	static UPARAM(DisplayName = "NameContainer") FName GetNameContainerFromContainerSetting(FContainerSetting ContainerSetting) { return ContainerSetting.NameContainer; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Container Setting", meta = (DisplayName = "Get Container Size"))
	static UPARAM(DisplayName = "ContainerSize") int32 GetContainerSizeFromContainerSetting(FContainerSetting ContainerSetting) { return ContainerSetting.ContainerSize; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Container Setting", meta = (DisplayName = "Get Container Slots Per Row"))
	static UPARAM(DisplayName = "ContainerSlotsPerRow") int32 GetContainerSlotsPerRowFromContainerSetting(FContainerSetting ContainerSetting) { return ContainerSetting.ContainerSlotsPerRow; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Container Setting", meta = (DisplayName = "Get Is Storage Container"))
	static UPARAM(DisplayName = "IsStorageContainer") bool GetIsStorageContainerFromContainerSetting(FContainerSetting ContainerSetting) { return ContainerSetting.bIsStorageContainer; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Container Setting", meta = (DisplayName = "Get Dropped Location"))
	static UPARAM(DisplayName = "DroppedLocation") FVector GetDroppedLocationFromContainerSetting(FContainerSetting ContainerSetting) { return ContainerSetting.DroppedLocation; }


	// ---------------------- GETTER FROM ITEM CURRENCY ---------------------- 

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Currency", meta = (DisplayName = "Get ID"))
	static UPARAM(DisplayName = "ID") FName GetIDFromtIemCurrency(FItemCurrency ItemCurrency) { return ItemCurrency.ID; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Currency", meta = (DisplayName = "Get Amount"))
	static UPARAM(DisplayName = "Amount") int32 GetAmountIemCurrency(FItemCurrency ItemCurrency) { return ItemCurrency.Amount; }


	// ---------------------- GETTER FROM ITEM CURRENCY ---------------------- 

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item For Save", meta = (DisplayName = "Get Items Inventory"))
	static UPARAM(DisplayName = "ItemsInventory") TArray<FItem> GetItemsInventoryFromItemForSave(FItemForSave Save) { return Save.ItemsInventory; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item For Save", meta = (DisplayName = "Get Currency"))
	static UPARAM(DisplayName = "Currency") TArray<FItemCurrency> GetCurrencyFromItemForSave(FItemForSave Save) { return Save.Currency; }

	// ---------------------- GETTER FROM ITEM CRAFT ---------------------- 

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Craft", meta = (DisplayName = "Get Primary Asset Id"))
	static UPARAM(DisplayName = "PrimaryAssetId") FPrimaryAssetId GetPrimaryAssetIdFromtItemCraft(FItemCraft Craft) { return Craft.PrimaryAssetId; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Craft", meta = (DisplayName = "Get Amount"))
	static UPARAM(DisplayName = "Amount") int32 GetAmountFromtItemCraft(FItemCraft Craft) { return Craft.Amount; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Craft", meta = (DisplayName = "Get Current"))
	static UPARAM(DisplayName = "Current") int32 GetCurrentFromtItemCraft(FItemCraft Craft) { return Craft.Current; }


	// ---------------------- GETTER FROM RECIPE CRAFT ---------------------- 

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Recipe Craft", meta = (DisplayName = "Get Ingredient Needed"))
	static UPARAM(DisplayName = "IngredientNeeded") TArray<FItemCraft> GetIngredientNeededFromtRecipeCraft(FRecipeCraft Recipe) { return Recipe.IngredientNeeded; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Recipe Craft", meta = (DisplayName = "Get Time To Craft"))
	static UPARAM(DisplayName = "TimeToCraft") float GetTimeToCraftFromtRecipeCraft(FRecipeCraft Recipe) { return Recipe.TimeToCraft; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Recipe Craft", meta = (DisplayName = "Get Return Self"))
	static UPARAM(DisplayName = "ReturnSelf") bool GetReturnSelfFromtRecipeCraft(FRecipeCraft Recipe) { return Recipe.ReturnSelf; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Recipe Craft", meta = (DisplayName = "Get Primary Asset Id Return"))
	static UPARAM(DisplayName = "PrimaryAssetIdReturn") FPrimaryAssetId GetPrimaryAssetIdReturnFromtRecipeCraft(FRecipeCraft Recipe) { return Recipe.PrimaryAssetIdReturn; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Recipe Craft", meta = (DisplayName = "Get Can Failure"))
	static UPARAM(DisplayName = "CanFailure") bool GetCanFailureRecipeCraft(FRecipeCraft Recipe) { return Recipe.bCanFailure; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Recipe Craft", meta = (DisplayName = "Get Percent Failure"))
	static UPARAM(DisplayName = "PercentFailure") float GetPercentFailureFromtRecipeCraft(FRecipeCraft Recipe) { return Recipe.PercentFailure; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Recipe Craft", meta = (DisplayName = "Get Primary Asset Id Failure"))
	static UPARAM(DisplayName = "PrimaryAssetIdFailure") FPrimaryAssetId GetPrimaryAssetIdFailureFromtRecipeCraft(FRecipeCraft Recipe) { return Recipe.PrimaryAssetIdFailure; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Recipe Craft", meta = (DisplayName = "Get Item Return Amount"))
	static UPARAM(DisplayName = "ItemReturnAmount") int32 GetItemReturnAmountFromtRecipeCraft(FRecipeCraft Recipe) { return Recipe.ItemReturnAmount; }

	// ---------------------- GETTER FROM ITEM DATA INFO CRAFT ---------------------- 

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Data Information Craft", meta = (DisplayName = "Get Item Information"))
	static UPARAM(DisplayName = "ItemInformation") FItemDataInfo GetItemInformationFromtItemDataInfoCraft(FItemDataInfoCraft CraftInfo) { return CraftInfo.ItemInformation; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Data Information Craft", meta = (DisplayName = "Get Tag"))
	static UPARAM(DisplayName = "Tag") FName GetTagFromtItemDataInfoCraft(FItemDataInfoCraft CraftInfo) { return CraftInfo.Tag; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Data Information Craft", meta = (DisplayName = "Get Category"))
	static UPARAM(DisplayName = "Category") FName GetCategoryFromtItemDataInfoCraft(FItemDataInfoCraft CraftInfo) { return CraftInfo.Category; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Data Information Craft", meta = (DisplayName = "Get Name Category"))
	static UPARAM(DisplayName = "NameCategory") FText GetNameCategoryFromtItemDataInfoCraft(FItemDataInfoCraft CraftInfo) { return CraftInfo.NameCategory; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Data Information Craft", meta = (DisplayName = "Get Priority"))
	static UPARAM(DisplayName = "Priority") int32 GetPriorityFromtItemDataInfoCraft(FItemDataInfoCraft CraftInfo) { return CraftInfo.Priority; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Data Information Craft", meta = (DisplayName = "Get Recipe"))
	static UPARAM(DisplayName = "Recipe") FRecipeCraft GetRecipeFromtItemDataInfoCraft(FItemDataInfoCraft CraftInfo) { return CraftInfo.Recipe; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Data Information Craft", meta = (DisplayName = "Get Max Count Craft"))
	static UPARAM(DisplayName = "MaxCountCraft") int32 GetMaxCountCraftFromtItemDataInfoCraft(FItemDataInfoCraft CraftInfo) { return CraftInfo.MaxCountCraft; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Data Information Craft", meta = (DisplayName = "Get Is Repair"))
	static UPARAM(DisplayName = "IsRepair") bool GetIsRepairFromtItemDataInfoCraft(FItemDataInfoCraft CraftInfo) { return CraftInfo.bIsRepair; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Data Information Craft", meta = (DisplayName = "Get In Type"))
	static UPARAM(DisplayName = "InType") ESlotType GetInTypeFromtItemDataInfoCraft(FItemDataInfoCraft CraftInfo) { return CraftInfo.InType; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item Data Information Craft", meta = (DisplayName = "Get Inventory Slot"))
	static UPARAM(DisplayName = "InventorySlot") int GetInventorySlotFromtItemDataInfoCraft(FItemDataInfoCraft CraftInfo) { return CraftInfo.InventorySlot; }

	// ---------------------- FUNCTION INVENTORY SUBSYSTEM ---------------------- 


	//Return Default Item value from PrimaryAssetId
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item", meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
	static UPARAM(DisplayName = "ItemData") FItemData GetItemDataFromPrimaryAssetId(const UObject* WorldContextObject, FPrimaryAssetId PrimaryAssetId);

	//Return Default Item value from ID
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item", meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
	static UPARAM(DisplayName = "ItemData") FItemData GetItemDataFromID(const UObject* WorldContextObject, FName ID);


	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Make", meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
	static UPARAM(DisplayName = "ItemData") FItemData MakeItemAmount(const UObject* WorldContextObject, FPrimaryAssetId PrimaryAssetId, int32 Amount);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Make", meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
	static UPARAM(DisplayName = "ItemData") FItemData MakeItemDecay(const UObject* WorldContextObject, FPrimaryAssetId PrimaryAssetId, float Decay);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Make", meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
	static UPARAM(DisplayName = "ItemData") FItemData MakeItemDurability(const UObject* WorldContextObject, FPrimaryAssetId PrimaryAssetId, float Durability);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Make", meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
	static UPARAM(DisplayName = "ItemData") FItemData MakeItemAttributes(const UObject* WorldContextObject, FPrimaryAssetId PrimaryAssetId, FGameplayTagAttributeContainer Attributes);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Make", meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
	static UPARAM(DisplayName = "ItemData") FItemData MakeItemSockets(const UObject* WorldContextObject, FPrimaryAssetId PrimaryAssetId, TArray<FSocketItem> Sockets);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Make", meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true", AutoCreateRefTerm = "Sockets"))
	static UPARAM(DisplayName = "ItemData") FItemData MakeItem(const UObject* WorldContextObject, FPrimaryAssetId PrimaryAssetId, int32 Amount, float Decay, float Durability, FGameplayTagAttributeContainer Attributes, TArray<FSocketItem> Sockets, FString JsonParameter);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Make", meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true", AutoCreateRefTerm="Sockets"))
	static UPARAM(DisplayName = "ItemData") FItemData MakeItemByID(const UObject* WorldContextObject, FName ID, int32 Amount, float Decay, float Durability, FGameplayTagAttributeContainer Attributes, TArray<FSocketItem> Sockets, FString JsonParameter);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Make", meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
	static UPARAM(DisplayName = "PrimaryAssetId") FPrimaryAssetId IdToPrimaryAssetId(const UObject* WorldContextObject, FName ID);

	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintAuthorityOnly, Category = "Chestory|Item", meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
	static FItemData GetItemFromInventoryId(const UObject* WorldContextObject, FGuid InventoryId, int32 Slot);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item", meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
	static ESlotType GetTypeFromInventoryId(const UObject* WorldContextObject, FGuid InventoryId);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item")
	static UPARAM(DisplayName = "ItemInfo") FItemDataInfo ItemDataToItemInformation(FItemData ItemData);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item", meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
	static UPARAM(DisplayName = "ItemData") FItemData SocketToItemData(const UObject* WorldContextObject, FSocketItem Socket);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item", meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
	static UPARAM(DisplayName = "ItemInfo") FItemDataInfo SocketToItemInfo(const UObject* WorldContextObject, FSocketItem Socket);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item", meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
	static UPARAM(DisplayName = "ItemInfo") FSocketItem ItemDataToSocket(const UObject* WorldContextObject, FItemData ItemData);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item", meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
	static UPARAM(DisplayName = "ItemSave") TArray<FItem> MakeItemSaveFromInventory(const UObject* WorldContextObject, UInventoryComponent* Inventory, bool InEquipement = false, bool InHotbar = false);

	// ---------------------- FUNCTION ITEM SCRIPT ---------------------- 

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|ItemScripts", meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
	static UPARAM(DisplayName = "Out ItemScrips") TArray<UItemScript*> GetItemScriptsFromPrimaryAssetId(const UObject* WorldContextObject, FPrimaryAssetId PrimaryAssetId);
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|ItemScripts", meta = (DeterminesOutputType = "ClassItemScript", WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
	static UPARAM(DisplayName = "ItemScript")	UItemScript* GetItemScriptOfClassFromPrimaryAssetId(const UObject* WorldContextObject, FPrimaryAssetId PrimaryAssetId, TSubclassOf<UItemScript> ClassItemScript);
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|ItemScripts", meta = (DeterminesOutputType = "ClassItemScript", WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
	static UPARAM(DisplayName = "ItemScript")	UItemScript* GetItemScriptOfClassFromItem(const UObject* WorldContextObject, FItemData Item, TSubclassOf<UItemScript> ClassItemScript);
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|ItemScripts", BlueprintAuthorityOnly, meta = (DeterminesOutputType = "ClassItemScript", WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
	static UPARAM(DisplayName = "ItemScript")	UItemScript* GetItemScriptInstanceOfClassFromItem(FItemData Item, TSubclassOf<UItemScript> ClassItemScript);

	// ---------------------- FUNCTION ATTRIBUTES ---------------------- 

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Attributes", meta = (DisplayName = "Contains Attribute", KeyWords="Contains Attribute"))
	static bool ContaintAttribute(FGameplayTagAttributeContainer Attributes,FGameplayTag AttributeTag);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Attributes")
	static int32 GetNumberAttributes(FGameplayTagAttributeContainer Attributes);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Attributes")
	static float GetAttributeValue(FGameplayTagAttributeContainer Attributes, FGameplayTag AttributeTag);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Attributes")
	static FGameplayTagAttributeContainer GetAttributesWithExclude(FGameplayTagAttributeContainer Attributes, FGameplayTagContainer ExcludeTag);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Attributes")
	static FGameplayTagAttributeContainer GetAttributesWithInclude(FGameplayTagAttributeContainer Attributes, FGameplayTagContainer IncludeTag);

	/* return value of Attribute 1 - Attribute 2 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Attributes")
	static float CompareTwoAttribute(FGameplayTagAttribute Attribute1, FGameplayTagAttribute Attribute2) { return Attribute1.Value - Attribute2.Value; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Attributes", meta = (CompactNodeTitle = "FIND"))
	static FGameplayTagAttribute FindAttribute(FGameplayTagAttributeContainer Attributes, FGameplayTag Tag, bool& Success);

	UFUNCTION(BlueprintCallable, Category = "Chestory|Attributes")
	static void Add_Attribute(UPARAM(ref) FGameplayTagAttributeContainer& InAttributes, FGameplayTag Tag, float Value);

	UFUNCTION(BlueprintCallable, Category = "Chestory|Attributes")
	static void Remove_Attribute(UPARAM(ref) FGameplayTagAttributeContainer& InAttributes, FGameplayTag Tag, float Value);

	UFUNCTION(BlueprintCallable, Category = "Chestory|Attributes")
	static void Set_Attribute(UPARAM(ref) FGameplayTagAttributeContainer& InAttributes, FGameplayTag Tag, float Value);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Attributes", meta = (CompactNodeTitle = "MAP"))
	static TMap<FGameplayTag, float> GetAttributeToMapFormat(FGameplayTagAttributeContainer InAttributes) { return InAttributes.GetAttributesToMapFormat(); }
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Attributes", meta = (CompactNodeTitle = "ARRAY"))
	static TArray<FGameplayTagAttribute> GetAttributesToArray(FGameplayTagAttributeContainer InAttributes) {return InAttributes.GetAttributes();}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Attributes")
	static bool Has_Attribute(FGameplayTagAttributeContainer InAttributes, FGameplayTag Tag);

	// ---------------------- GETTER FROM SOCKET ---------------------- 

	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item", meta = (DisplayName = "Get Primary Asset Id"))
	static UPARAM(DisplayName = "PrimaryAssetId") FPrimaryAssetId GetPrimaryAssetIdFromSocket( FSocketItem Socket) { return Socket.PrimaryAssetId; }

	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item", meta = (DisplayName = "Get Attributes"))
	static UPARAM(DisplayName = "Attributes") FGameplayTagAttributeContainer GetAttributesFromSocket( FSocketItem Socket) { return Socket.Attributes; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory|Item", meta = (DisplayName = "Get Is Valid"))
	static UPARAM(DisplayName = "Is Valid") bool GetIsValidFromSocket(FSocketItem Socket) { return Socket.PrimaryAssetId.IsValid(); }

	// ---------------------- TRANSFER ----------------------

	static FItemTiny MakeItemTiny(FItemData ItemData);
	static FItemTiny MakeItemTiny(FItemDataInfo ItemInfo);
	static FItemDataInfoCompressed MakeItemTransfer(FItemData ItemData);
	static FItemDataInfoCompressed MakeItemTransfer(FItemDataInfo ItemInfo);
	static FItemDataInfoCraftCompressed MakeCraftTransfer(class UItemDataAsset* ItemDataAsset);
	static FItemDataInfoCraftCompressed MakeCraftTransfer(FItemDataInfoCraft InfoCraft);
	static FItemData MakeItemDataFromItemTiny(const UObject* WorldContextObject, FItemTiny ItemTiny);



	UFUNCTION(meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
	static FItemData MakeItemDataFromItem(const UObject* WorldContextObject, FItem Item);

	UFUNCTION(meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
	static FItemDataInfo MakeItemDataFromTransfer(const UObject* WorldContextObject, FItemDataInfoCompressed Transfer);
	
	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintCosmetic, Category = "Chestory", meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
	static FItemDataInfo MakeItemDataInfo(const UObject* WorldContextObject, FItemData ItemData);

	UFUNCTION(meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
	static FItemDataInfoCraft MakeItemCraftFromTransfer(const UObject* WorldContextObject, FItemDataInfoCraftCompressed CraftCompressed);
	static FItemDataInfoCraft MakeItemInfoCraft(const UObject* WorldContextObject, class UItemDataAsset* ItemDataAsset);





	// ---------------------- CRAFTING ----------------------


	static bool IsValidRecipe(UItemDataAsset* ItemDataAsset, bool PrintLog = false);


	// ---------------------- INVENTORY SUBSYSTEM ----------------------

	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintCosmetic, Category = "Chestory", meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
	static bool ContainsGrid(const UObject* WorldContextObject, FName GridKey);

	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintCosmetic, Category = "Chestory", meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
	static class UInventoryUniformGridPanel* GetGrid(const UObject* WorldContextObject, FName GridKey);

	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintCosmetic, Category = "Chestory", meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
	static	TArray<class UInventorySlotUserWidget*> GetGridSlots(const UObject* WorldContextObject, FName GridKey);

	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintCosmetic, Category = "Chestory", meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
	static bool ContainsEquipmentSlot(const UObject* WorldContextObject, FName Context, FName SlotKey);

	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintCosmetic, Category = "Chestory", meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
	static	class UInventorySlotUserWidget* GetEquipmentSlot(const UObject* WorldContextObject, FName Context, FName SlotKey);

	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintCosmetic, Category = "Chestory", meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
	static	TArray<class UInventorySlotUserWidget*> GetEquipmentSlots(const UObject* WorldContextObject, FName Context);

	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintCosmetic, Category = "Chestory", meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
	static bool ContainsHotbarSlot(const UObject* WorldContextObject, FName SlotKey);

	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintCosmetic, Category = "Chestory", meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
	static  int32 GetHotbarSlotNum(const UObject* WorldContextObject);
	
	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintCosmetic, Category = "Chestory", meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
	static bool HasHotbars(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintCosmetic, Category = "Chestory", meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
	static	class UInventorySlotUserWidget* GetHotbarSlot(const UObject* WorldContextObject, FName SlotKey);

	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintCosmetic, Category = "Chestory", meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
	static	TArray<class UInventorySlotUserWidget*> GetHotbarSlots(const UObject* WorldContextObject);
	
	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintAuthorityOnly, Category = "Chestory", meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
	static bool  ContainsInventory(const UObject* WorldContextObject, FGuid InventoryId);

	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintAuthorityOnly, Category = "Chestory", meta = (DefaultToSelf = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
	static class UInventoryComponent* GetInventory(const UObject* WorldContextObject, FGuid InventoryId);
	
	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintAuthorityOnly, Category = "Chestory", meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
	static bool  ContainsCrafting(const UObject* WorldContextObject, FGuid CraftingId);

	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintAuthorityOnly, Category = "Chestory", meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
	static class UCraftingComponent* GetCrafting(const UObject* WorldContextObject, FGuid CraftingId);

	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintCosmetic, Category = "Chestory|UserInterface", meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
	static TArray<UCraftingSlotUserWidget*> GetCraftingSlots(const UObject* WorldContextObject, FName GridKey);



	/*
	* Return the InventoryManager of the GameMode if it is the Server.
	* Return the Player InventoryManager if it is the client.
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory", meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
	static UInventoryManagerComponent* GetInventoryManagerInstance(const UObject* WorldContextObject,bool &Success);

	/*
	* Try to found InventoryManager in Target
	* Return the first InventoryManager found
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory", meta = (DefaultToSelf=Target))
	static UInventoryManagerComponent* GetInventoryManager(AActor* Target);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory", meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
	static TArray<FName> GetItemsIdWithGameplayTag(const UObject* WorldContextObject, FGameplayTag Tag);


	UFUNCTION(meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
	static void RemakeItemDataFromSaveGame(const UObject* WorldContextObject,UInventoryManagerComponent* InventoryManager, FItemData& Item);


	/*
	* Update data available in the ActorInWorld in the TArray<uint8> DataBytesActor
	* Used before a World to Inventory switch
	*/
	UFUNCTION(BlueprintCallable, Category = "Chestory|Bytes", meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
	static void UpdateDataBytesActor(const UObject* WorldContextObject, UPARAM(ref)FItemData& Item, bool bBeforDestoryed = false);



	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chestory", meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
	static bool IsRunningInEditor(const UObject* WorldContextObject);


	// ---------------------- SIZE FUNCTION ----------------------

	static FIntPoint GetRowColumnFromIndex(int32 index, int32 MaxPerRow);

	static int32 GetIndexFromRowColumn(FIntPoint Point,int32 MaxPerRow, int32 StartIndex);
};
