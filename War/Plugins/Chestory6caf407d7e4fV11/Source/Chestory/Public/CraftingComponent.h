// Copyright 2024 Arkai Interactive. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Chestory.h"
#include "InventoryManagerComponent.h"
#include "TimerManager.h"
#include "Interface/ChestorySaveGameInterface.h"
#include "CraftingComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FStartCrafting,FItemDataInfoCraft, ItemToCraft);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FEndCrafting, FItemData, ItemDataCraft, bool, Failure, bool, IsRepair, FItemDataInfoCraft, ItemRepairInfo, FGuid, InInventory, int32,InSlot);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRemoveInWaitlist);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCurrentCrafting, float, CurrentCraftTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FUpdateWaitingList, FName, IDNewBlueprint,int32,AmountRemaining);

USTRUCT()
struct FCraftingInfo
{
	FCraftingInfo() {}
	FCraftingInfo(FItemDataInfoCraft InItemInfo, int32 InWaitListIndex, bool InbHasRessource, bool InbIsRepair, float StartTime)
		:ItemInfo(InItemInfo)
		, WaitListIndex(InWaitListIndex)
		, bHasResources(InbHasRessource)
		, bIsRepair(InbIsRepair)
		, CraftingCurrentTime(StartTime)
	{}

	GENERATED_BODY()

public:	
	UPROPERTY(SaveGame)
	FItemDataInfoCraft ItemInfo;
	UPROPERTY(SaveGame)
	int32 WaitListIndex = -1;
	UPROPERTY(SaveGame)
	bool bHasResources = false ;
	UPROPERTY(SaveGame)
	bool bIsRepair = false;
	UPROPERTY(SaveGame)
	float CraftingCurrentTime = 0;
};

UCLASS(Abstract, Blueprintable, ClassGroup=(Inventory), meta=(BlueprintSpawnableComponent) )
class CHESTORY_API UCraftingComponent : public UActorComponent, public IChestorySaveGameInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCraftingComponent();
	UCraftingComponent(const FObjectInitializer& ObjectInitializer);


protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void OnComponentCreated();
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy);
	virtual void PostLoad() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

//Inteface
	UFUNCTION()
	void OnObjectPreLoad_Implementation();
	UFUNCTION()
	void OnObjectPostLoad_Implementation();


	UFUNCTION()
	void OnCraftingLoaded();
public:	

	//---------- Event
	UPROPERTY(BlueprintAssignable, BlueprintAuthorityOnly, Category = "Crafting|Event")
	FStartCrafting StartCrafting;
	
	UPROPERTY(BlueprintAssignable, BlueprintAuthorityOnly, Category = "Crafting|Event")
	FEndCrafting EndCrafting;

	UPROPERTY(BlueprintAssignable, BlueprintAuthorityOnly, Category = "Crafting|Event")
	FRemoveInWaitlist RemoveInWaitlist;

	UPROPERTY(BlueprintAssignable, BlueprintAuthorityOnly, Category = "Crafting|Event")
	FCurrentCrafting CurrentCrafting;

	UPROPERTY(BlueprintAssignable, BlueprintAuthorityOnly,Category = "Crafting|Event")
	FUpdateWaitingList OnUpdateWaitingList;


public : 

	//---------- Id

	/*
	* Server Genereate a New Id for this CraftingComponent
	* and Call OnRep_InventoryID for replicated the new Id
	*/
	UFUNCTION(Server, Reliable)
	void Server_GenerateId();

	UFUNCTION(Server, Reliable)
	void Server_SetId(FGuid Id);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_UpdateId(FGuid Id);

	UPROPERTY(ReplicatedUsing = OnRep_CraftingID, BlueprintReadOnly, SaveGame, Category = "Crafting")
	FGuid CraftingID;

	UFUNCTION()
	virtual void OnRep_CraftingID();

	/*
	* Get the CraftingId for this CraftingComponent
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crafting")
	FGuid GetId() { return CraftingID; }



	//---------- Inventory

	/*
	 * If this component is located next to an InventoryManager indicate here 
	 * the Inventory that it must be used for these Craft
	 *
	 * Otherwise it takes the first InventoryComponent found in this Actor
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	FName InventoryPlayerLinked;

	/*
	* Current InventoryIdLinked, Apply in BeginPlay on this Component 
	* After TryToResolveInventory
	*/
	UPROPERTY(Replicated, SaveGame)
	FGuid InventoryIdLinked;

	/*
	* Get the inventoryId current linked with this Crafting Component
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crafting")
	FGuid GetInventorIdLinked() { return InventoryIdLinked; }


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	FName DefaultGridKey = FName("DefaultGrid");


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting|Widget", meta = (DisplayName = "Widget Crafting Slot"))
	TSubclassOf<UCraftingSlotUserWidget> ClassCraftingSlot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting|Widget", meta = (DisplayName = "Widget Waiting Slot"))
	TSubclassOf<UCraftingSlotUserWidget> ClassWaitingSlot;

	/*If true the crafting slots will be automatically created */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Crafting|Widget", meta = (DisplayName = "Enable Auto Creation"))
	bool bAutoCreationCraftingSlot = true;

	/* List ItemDataAsset to type Blueprint for list crafting*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Crafting|Items", meta = (DisplayName = "DEPRECATED_BlueprintCrafts", DeprecatedProperty, DeprecationMessage = "For default Item use DefaultItemsCrafts Variable, for Dynamics use function Add/Remove Craft"))
	TArray<UItemDataAsset*> BlueprintCrafts;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Crafting|Items", meta=(NoElementDuplicate, AllowedTypes = "ItemDataAsset"))
	TArray<FPrimaryAssetId> DefaultItemsCrafts;

	// Find Item with GameplayTagContrainer in the ItemDataAsset and add in crafting list
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting|Items")
	FGameplayTagQuery ItemsGameplayTags;

	// Find Item with CraftingCategoryTag to add in crafting list
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting|Items")
	FGameplayTagContainer ItemsCraftingTags;

	/* Try to stack the Item on the waiting list if they are the same Item*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Crafting")
	bool bEnableStackWaitlist = false;

	/* Size maxi of the waiting list of crafting system*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Crafting")
	int32 MaxWaitingList = 1;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Crafting")
	float FailureFactor = 1.0f;
	
	// Speed crafting
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Crafting")
	float CraftingFactor = 1.0f;

	// Refresh tick crafting
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Crafting")
	float CraftingTickTime = 0.1f;

	/*If true is possible to cancel current craft*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Crafting|Cancel")
	bool bCancelDuringCraft = false;

	/*Refund ressource after Cancel current craft*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Crafting|Cancel", meta = (EditCondition = "bCancelDuringCraft"))
	bool bRefundRessource = false;

	/*Percentage refund of craft cancel*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Crafting|Cancel", meta = (EditCondition = "bCancelDuringCraft && bRefundRessource", ClampMin = "0.0", ClampMax = "100.0", UIMin = "0.0", UIMax = "100.0"))
	float RefundPercentage = 100;
	
	/*
	* Init Inventory Subsystem
	*/
	void InitializeSubsystem();
	UInventorySubsystem* InventorySubsystem;

	UInventorySubsystem* GetInventorySubsystem() { return InventorySubsystem; };



	/*
	* Current InventoryComponent Used with tihs CraftingComponent
	*/
	UInventoryComponent* CraftingInventory;

	UInventoryManagerComponent* InventoryManager;

	/*
	* Return InventoryComponent used with tihs CraftingComponent
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crafting|Inventory")
	UInventoryComponent * GetCraftingInventory() { return CraftingInventory; };


	UFUNCTION(BlueprintCallable, Category = "Crafting|Inventory")
	void AddCraft(FPrimaryAssetId ItemAdd) { Server_AddCraft(ItemAdd); };

	UFUNCTION(BlueprintCallable, Category = "Crafting|Inventory")
	void RemoveCraft(FPrimaryAssetId ItemRemove) { Server_RemoveCraft(ItemRemove); };

	/*
	* Replace DynamicsCraft Array, use this function befor Open Crafting
	*/
	UFUNCTION(BlueprintCallable, Category = "Crafting|Inventory")
	void SetDynamicsCrafts(TArray<FPrimaryAssetId> Crafts);

	/*
	* Return Blueprint Crafts available on this CrafingComponent
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crafting|Inventory")
	TArray<UItemDataAsset*> GetBlueprintCrafts() { return BlueprintCrafts; };

	/*
	* Return the Owner InventoryManager if Owner Actor has a InventoryManager
	* or Return the InventoryManager in the GameMode
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crafting|Inventory")
	UInventoryManagerComponent* GetInventoryManager();

	/*
	* Return the Wait list craft
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crafting|Inventory")
	TArray<FItemDataInfoCraft> GetWaitList() { return WaitList; };

	/*
	* Server Compressed Data befor send to the Client 
	*/
	TArray<FItemDataInfoCraftCompressed> GetDataForTransfer();

	// ------------------------ WAITING ------------------------

	UPROPERTY(SaveGame)
	TArray<FItemDataInfoCraft> WaitList;

	TArray<FItemDataInfoCraftCompressed> GetDataWaitingForTransfer();

	// Cancel variable
	bool bCallCancelCraft = false;
	int32 IndexSlotToCancel;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crafting|Inventory")
	FItemDataInfoCraft GetWaitListItem(int32 IndexSlot) { if (WaitList.IsValidIndex(IndexSlot)) { return WaitList[IndexSlot]; } else { return FItemDataInfoCraft(); } };
	
	void SetWaitListItem(int32 IndexSlot, FItemDataInfoCraft WaitListItem) { WaitList[IndexSlot] = WaitListItem; };
	void ClearWaitListItem(int32 IndexSlot) { FItemDataInfoCraft ClearItem; WaitList[IndexSlot] = ClearItem; };
	void AddWaitListItem(FItemDataInfoCraft AddInfo) { WaitList.Add(AddInfo); };
	void RemoveWaitlistItem(int32 IndexSlot) { WaitList.RemoveAt(IndexSlot); RemoveInWaitlist.Broadcast(); };
	bool FindIndexWaitListSameItem(int32 &IndexFound,FItemDataInfoCraft WaitListItem);
	FItemDataInfoCraft GetWaitSlotAtIndex(int32 Index) {return WaitList[Index]; }
	void CheckFailureCarft(bool &Failure, FPrimaryAssetId &PrimaryAssetId, FItemDataInfoCraft Craft);

	UFUNCTION(Server, Reliable)
	void Server_RemoveWaitListItem(int32 IndexSlot);

	// ------------------------ CRAFTING ------------------------

	UPROPERTY(SaveGame)
	FCraftingInfo CraftInfo;

	int32 GetWaitlistIndex() { return CraftInfo.WaitListIndex; }	
	FTimerHandle TimerCrafting;


	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crafting|Inventory")
	bool GetIsCurrentCrafting() { return GetWorld()->GetTimerManager().IsTimerActive(TimerCrafting); };
	bool CanStartCrafting(const FItemDataInfoCraft& CraftItem);
	bool IsFinishedCrafting();


	UFUNCTION(Server, Reliable)
	void StartCraftingTimer();
	UFUNCTION(Server, Reliable)
	void CurrentCraftingTimer();
	UFUNCTION(Server, Reliable)
	void EndCraftingTimer();
	void ClearTimerCrafting();

	void HandleCraftingSuccess();
	void HandleRepairSuccess();
	void UpdateWaitList();

	/*
	* Bind Linked Inventory event ItemEnter / ItemLeave for Check Update can Craft
	*/
	UFUNCTION()
	void OnOwerInventoryDataChanged(FPrimaryAssetId PrimaryAssetId, int32 Amount);

protected:


	TArray<FPrimaryAssetId> GetCombinedItemsCraft();

private:

	UInventoryManagerComponent* GetOwnerManager();
	bool HasInventoryManager();

	UFUNCTION()
	void PendingToResolve();

	/*
	* Try to found Inventory player on the Owner Player 
	* or Check in Owner Actor and take the First InventoryComponent found
	*/
	bool TryToResolveInventory();


	UFUNCTION(Server, Reliable)
	void Server_AddCraft(FPrimaryAssetId ItemAdd);

	UFUNCTION(Server, Reliable)
	void Server_RemoveCraft(FPrimaryAssetId ItemRemove);

	UFUNCTION(Server, Reliable)
	void Server_SetDynamicsCraft(const TArray<FPrimaryAssetId>& Crafts);

	UPROPERTY(SaveGame)

	TArray<FPrimaryAssetId> DynamicsItemsCrafts;

	TArray<FPrimaryAssetId> TotalItemsCrafts;

};
