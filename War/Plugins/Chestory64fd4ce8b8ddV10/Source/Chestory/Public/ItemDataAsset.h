// Copyright 2024 Arkai Interactive. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "InventoryPluginStruct.h"
#include "ItemDataAsset.generated.h"

/**
 * Item Inventory
 */

UCLASS(BlueprintType)
class CHESTORY_API UItemDataAsset : public UDataAsset
{
	GENERATED_BODY()

public :

	FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId("ItemDataAsset", GetFName());
	}

	virtual void PostLoad() override;

	//UItemDataAsset(const FObjectInitializer& ObjectInitializer) { Item.ID = GetFName(); };



	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data", meta = (ShowOnlyInnerProperties))
	FItemData Item;
	
	/*
	* Only Server Side
	* List of scripts that will be triggered for these items
	* By default the scripts are not unique per item, all items of this class will use the same ItemScript  
	*
	* For example my apple item with a script "IS_eat" and a value of 5, all the apple item will have the value 5
	* But a banana item with the script "IS_eat" and a value of 8 is possible
	* 
	* If you want ItemScript per Item use EnableInstanceItemScript in the script
	*/
	UPROPERTY(Instanced, EditAnywhere, BlueprintReadWrite, Category = "Item Script", meta = (TitleProperty = "IsInstance:{bEnableItemScriptInstance}"))
	TArray<class UItemScript*> ListScriptsItem; 

	/*
	* Indicates that this item can be used in a CraftingComponent
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Craft")
	bool IsCraftable;

	/*
	* Data sent to the client to display the recip craft as he wishes 
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Craft", meta = (EditCondition = "IsCraftable", DisplayName = "DEPRECATED_Tag", DeprecatedProperty, DeprecationMessage = "Use CraftingCategoryTag"))
	FName Tag;

	//Enable this item can be found by the crafting component
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Craft", meta = (EditCondition = "IsCraftable"))
	bool EnableCanBeFound = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Craft", meta = (EditCondition = "IsCraftable"))
	FGameplayTag CraftingCategoryTag;

	/*
	* Data sent to the client to display the recip craft as he wishes
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Craft", meta = (EditCondition = "IsCraftable"))
	FName Category;

	/*
	* Data sent to the client to display the recip craft as he wishes
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Craft", meta = (EditCondition = "IsCraftable"))
	FText NameCategory;

	/*
	* Data sent to the client to display the recip craft as he wishes
	* can be used to prioritize display in a list
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Craft", meta = (EditCondition = "IsCraftable"))
	int32 Priority;

	/*
	* The recipe linked to this item
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Craft", meta = (EditCondition = "IsCraftable", ShowOnlyInnerProperties))
	FRecipeCraft Recipe;
};
