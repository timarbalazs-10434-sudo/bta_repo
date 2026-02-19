// Copyright 2024 Arkai Interactive. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "UObject/Object.h"
#include "UObject/NoExportTypes.h"
#include "Tickable.h"

#include "InventoryPluginStruct.h"
#include "InventoryManagerComponent.h"
#include "ItemScript.generated.h"

/**
 * Script for items 
 */
UCLASS(DefaultToInstanced, EditInlineNew, BlueprintType, Blueprintable)
class CHESTORY_API UItemScript : public UObject, public FTickableGameObject
{
	GENERATED_BODY()


public:

	void Init(UInventoryManagerComponent* InstigatorInventoryManager, FItemData Item);
	UWorld* GetWorld() const override;


	// -------- FTickableGameObject Begin --------
	virtual void Tick(float DeltaTime) override;
	virtual ETickableTickType GetTickableTickType() const override
	{
		return bEnableTick ? ETickableTickType::Always : ETickableTickType::Never;
	}
	virtual bool IsTickable() const override
	{ 
		return bEnableTick;
	}
	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(UItemScript, STATGROUP_Tickables);
	}
	virtual bool IsTickableWhenPaused() const
	{
		return true;
	}
	virtual bool IsTickableInEditor() const
	{
		return false;
	}
	// -------- FTickableGameObject End --------


	virtual void BeginDestroy() override;

public:

	

	UInventoryManagerComponent* InventoryManager = nullptr;
	AActor* InventoryManagerOwner = nullptr;
	UObject* ObjectForWorld = nullptr;

	class UInventorySlotUserWidget* SlotWidget = nullptr;

	void SetInventoryManager(UInventoryManagerComponent* InventoryManagerUpdate) { InventoryManager = InventoryManagerUpdate; };

	/*
	* Enable Modifier triggers the ItemModifier event when the element is dropped onto another.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemScript", meta = (EditCondition = "!IsInstanced()", EditConditionHides))
	bool bEnableModifier = false;

	/*
	* Generate Instance of this ItemScript by Item
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemScript", meta = (EditCondition = "!IsInstanced()", EditConditionHides))
	bool bEnableItemScriptInstance = false;

	/*
	* Input Type Authorised for trigger CanUse and ItemUsed function
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemScript", meta = (EditCondition = "!IsInstanced()", EditConditionHides, NoElementDuplicate))
	TArray<EUsedInputTrigger> UsedInputTriggerAuthorised = {EUsedInputTrigger::EUndefine, EUsedInputTrigger::EPressed};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemScript", meta = (EditCondition = "!IsInstanced() && bEnableItemScriptInstance", EditConditionHides))
	bool bEnableTick = false;
	
public:

	UFUNCTION(BlueprintNativeEvent, Category = "Param")
	bool IsItemModifier();

	UFUNCTION(BlueprintNativeEvent, Category = "Param")
	bool CanMove(UInventoryManagerComponent* InInventoryManager, FGuid FromInventoryId, int32 FromInventorySlot, FGuid ToInventoryId, int32 ToInventorySlot);

	UFUNCTION(BlueprintNativeEvent, Category = "Param")
	bool CanDrop(UInventoryManagerComponent* InInventoryManager, FGuid FromInventoryId, int32 FromInventorySlot);

	UFUNCTION(BlueprintNativeEvent, Category = "Param")
	bool CanUnequip(UInventoryManagerComponent* InInventoryManager, FGuid FromInventoryId, int32 FromInventorySlot, FGuid InInventoryId, int32 InventorySlot, AActor* ActorEquipment,bool DropRequest);

	UFUNCTION(BlueprintNativeEvent, Category = "Param")
	bool CanEquip(UInventoryManagerComponent* InInventoryManager, FGuid FromInventoryId, int32 FromInventorySlot, FGuid ToInventoryId, int32 ToInventorySlot);

	UFUNCTION(BlueprintNativeEvent, Category = "Param")
	bool CanCraft(UInventoryManagerComponent* InInventoryManager, FGuid FromCraftingId, FItemDataInfoCraft ItemDataInfoCraft, int32 Amount);

	UFUNCTION(BlueprintNativeEvent, Category = "Param")
	bool CanUse(UInventoryManagerComponent* InInventoryManager, FGuid InInventoryId, int32 InInventorySlot, EUsedInputTrigger InputTrigger = EUsedInputTrigger::EUndefine);

	UFUNCTION(BlueprintNativeEvent, Category = "Param")
	FItemData ItemCreation(UInventoryManagerComponent* InInventoryManager, FGuid FromInventoryId, int32 FromInventorySlot, FItemData NewItem);

	UFUNCTION(BlueprintNativeEvent, Category = "Param")
	FItemData ItemPostLoad(UInventoryManagerComponent* InInventoryManager, FGuid InventoryId, int32 InventorySlot, FItemData ItemLoad);

	UFUNCTION(BlueprintNativeEvent, Category = "Param")
	FItemData ItemPreSave(UInventoryManagerComponent* InInventoryManager, FGuid InventoryId, int32 InventorySlot, FItemData ItemSave);



	// ------------------------ EVENT BASE ------------------------

public:
	UFUNCTION(BlueprintImplementableEvent)
	void ItemTick(float DeltaTime);

	UFUNCTION(BlueprintImplementableEvent)
	void ItemUsed(UInventoryManagerComponent* InInventoryManager, FGuid InInventoryId,int32 InventorySlot, EUsedInputTrigger InputTrigger);

	UFUNCTION(BlueprintAuthorityOnly,BlueprintImplementableEvent)
	void ItemDropped(UInventoryManagerComponent* FromInventoryManager,AActor* ActorDropped, FGuid FromInventoryId,int32 FromSlot);

	UFUNCTION(BlueprintAuthorityOnly,BlueprintImplementableEvent)
	void ItemAddInInventory(UInventoryManagerComponent* InInventoryManager, FGuid InInventoryId, int32 InSlot,UInventoryComponent* InInventory);

	UFUNCTION(BlueprintAuthorityOnly,BlueprintImplementableEvent)
	void ItemRemoveInInventory(UInventoryManagerComponent* FromInventoryManager, FGuid FromInventoryId, int32 FromSlot,UInventoryComponent* FromInventory);
	/*
	* Called end creation Item
	*/
	UFUNCTION(BlueprintAuthorityOnly, BlueprintImplementableEvent)
	void ItemCreated(UInventoryManagerComponent* InInventoryManager, FGuid InInventoryId, int32 InSlot, UInventoryComponent* InInventory);


	/* This Function is Trigger if the function IsItemModifier is overided to true or default variable EnableModifier is true
	* Trigger befor swap Item
	* */
	UFUNCTION(BlueprintAuthorityOnly,BlueprintImplementableEvent)
	void ItemModifier(UInventoryManagerComponent* InInventoryManager, FGuid FromInventoryId,  int32 FromInventorySlot, FGuid ToInventoryId, int32 ToInventorySlot);


	// ------------------------ EVENT SOCKET ------------------------

	UFUNCTION(BlueprintAuthorityOnly, BlueprintImplementableEvent)
	void SocketItemAdded(UInventoryManagerComponent* InInventoryManager, FGuid InInventoryId, int32 InSlot, FSocketItem NewSocket, int32 InSocketSlot);

	UFUNCTION(BlueprintAuthorityOnly, BlueprintImplementableEvent)
	void SocketItemRemoved(UInventoryManagerComponent* FromInventoryManager, FGuid FromInventoryId, int32 FromSlot, FSocketItem RemovedSocket, int32 FromSocketSlot);


	// ------------------------ EQUIPMENT------------------------
	/*
	* Event trigger when Actor Equipment is spawned
	*/
	UFUNCTION(BlueprintAuthorityOnly, BlueprintImplementableEvent)
	void ItemEquip(UInventoryManagerComponent* InInventoryManager, FGuid InInventoryId, int32 InventorySlot);

	/*
	* Event trigger before Actor Equipment is Destory or Dropped
	*/
	UFUNCTION(BlueprintAuthorityOnly, BlueprintImplementableEvent)
	void ItemUnequip(UInventoryManagerComponent* InInventoryManager, FGuid InInventoryId, int32 InventorySlot);


	// ------------------------ INVENTORY------------------------
	
	/*
	* Event trigger when Owner Inventory is Open
	*/
	UFUNCTION(BlueprintAuthorityOnly, BlueprintImplementableEvent)
	void OwnerInventoryOpen(UInventoryManagerComponent* InInventoryManager, FGuid InInventoryId, int32 InventorySlot);

	/*
	* Event trigger when Owner Inventory is Close
	*/
	UFUNCTION(BlueprintAuthorityOnly, BlueprintImplementableEvent)
	void OwnerInventoryClose(UInventoryManagerComponent* InInventoryManager, FGuid InInventoryId, int32 InventorySlot);

	// ------------------------ FUNCTION BASE------------------------
public:

	/*
	* Return the InventoryManager that uses the script
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Item Script")
	UInventoryManagerComponent * GetInventoryManager() { return InventoryManager; };

	/*
	* Return Owner InventoryManager that uses the script
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Item Script")
	AActor* GetOwner();


	/*
	* Return the ItemData called by this ItemScript
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Item Script")
	FItemData GetItemData() { return OwnerItem; };


	UFUNCTION(BlueprintCallable, Category = "Item Script")
	void SetTickEnable(bool Enable);


	UFUNCTION()
	bool IsInstanced() const;

	void SetItem(FItemData Item) { OwnerItem = Item; }


private:

	// The last frame number we were ticked.
	// We don't want to tick multiple times per frame 
	uint32 LastFrameNumberWeTicked = INDEX_NONE;


	FItemData OwnerItem;
};
