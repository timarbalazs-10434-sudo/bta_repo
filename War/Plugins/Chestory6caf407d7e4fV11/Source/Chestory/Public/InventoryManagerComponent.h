// Copyright 2024 Arkai Interactive. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryPluginStruct.h"
#include "InventoryComponent.h"
#include "DroppedComponent.h"
#include "InventoryUniformGridPanel.h"
#include "InventorySlotUserWidget.h"
#include "Blueprint/UserWidget.h"
#include "Engine/DataTable.h"
#include "ItemDataAsset.h"
#include "Socket/SocketUserWidget.h"
#include "TimerManager.h"
#include "Interface/ChestorySaveGameInterface.h"

#include "InputActionValue.h"
//#include "InputMappingContext.h"
//#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"

#include "InventoryManagerComponent.generated.h"


class UCraftingComponent;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUpdateCurrency, FName, ItemId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FUpdateInventory, FGuid, InInventoryId,int32,InSlot,bool,IsAdd,FItemData,ItemData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLeftMousePressedOnSlot);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRightMousePressedOnSlot);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEndBeginPLay);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEquipementChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FAttributeChanged, FGuid, InInventoryId, int32, InSlot, bool, IsAdd, FGameplayTagAttribute, Attribute);
DECLARE_DYNAMIC_DELEGATE(FLoadCustomCraftingSlot);
DECLARE_DYNAMIC_DELEGATE(FUnPossesseOwner);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FEquip, FName, EquipmentKey,FItemData, ItemEquip, FItemData, ItemUnequip, AActor*, ActorEquip);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FUnequip, FName, EquipmentKey,FItemData, ItemUnequip,AActor*,ActorDropped);



class UItemScript;
class UInventorySubsystem;
class AItemActor;




UCLASS(Blueprintable, ClassGroup=(Inventory), meta=(BlueprintSpawnableComponent) )
class CHESTORY_API UInventoryManagerComponent : public UActorComponent, public IChestorySaveGameInterface
{
	GENERATED_BODY()

	

public:	
	// Sets default values for this component's properties
	UInventoryManagerComponent();

	UInventoryManagerComponent(const FObjectInitializer& ObjectInitializer);

	virtual void OnComponentCreated();
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy);


#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	TArray<UInventoryComponent*> CacheOldInventory;
	TMap<FName, FInventoryDefinition> &InventorysConstructor = Inventorys;

#endif

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

//Inteface
	UFUNCTION()
	void OnObjectPreLoad_Implementation();

	UFUNCTION()
	void OnObjectPostLoad_Implementation();

	UFUNCTION()
	void OnObjectPreDestoryed_Implementation();

//-------------- Initialize
#pragma region /** Initialize */

	void InitializeSubsystem();
	// Get Default Currency
	UFUNCTION()
	void InitializeStoreItems();
	UFUNCTION(BlueprintCallable, Category="InventoryManager")
	void InitializeInventoryManager();

	void InitPlayerInventory(FName InventoryKey, UInventoryComponent* Inventory, FGuid InventoryId);
	void InitEquipementPlayerInventory();
	void InitHotbarInventory();
#pragma endregion

	//-------------- Input
#pragma region /** Input */

	/*
	* InputMappingContext si Add if any inventory is open or Remove if all inventory is close
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	bool bEnableAutoGestionInputMapping = true;


	/** MappingContext 
	* This mapping context used in inventory
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* InputMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	int32 Priority = 0;

	/** Use Item Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Action Inventory", meta = (AllowPrivateAccess = "true"))
	class UInputAction* UseAction;
	uint32 TriggeredEventHandle_UseAction;

	/** Transfer Item Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Action Inventory", meta = (AllowPrivateAccess = "true"))
	class UInputAction* TransferAction;
	uint32 TriggeredEventHandle_TransferAction;

	/** Transfer Item by Id Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Action Inventory", meta = (AllowPrivateAccess = "true"))
	class UInputAction* TransferByIdAction;
	uint32 TriggeredEventHandle_TransferByIdAction;

	/** Drop Item Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Action Inventory", meta = (AllowPrivateAccess = "true"))
	class UInputAction* DropAction;
	uint32 TriggeredEventHandle_DropAction;

	/** Split Item when Drag Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Action Inventory", meta = (AllowPrivateAccess = "true"))
	class UInputAction* SplitAction;
	uint32 TriggeredEventHandle_SplitAction;

	/** Repair Item Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Action Inventory", meta = (AllowPrivateAccess = "true"))
	class UInputAction* RepairAction;
	uint32 TriggeredEventHandle_RepairAction;

	/** Open Contexual menu Input Action */
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Action Inventory", meta = (AllowPrivateAccess = "true"))
	//class UInputAction* ContextualMenu;

	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Action Inventory", meta = (AllowPrivateAccess = "true"))
	//class UInputAction* NavigationUp;

	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Action Inventory", meta = (AllowPrivateAccess = "true"))
	//class UInputAction* NavigationDown;

	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Action Inventory", meta = (AllowPrivateAccess = "true"))
	//class UInputAction* NavigationLeft;

	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Action Inventory", meta = (AllowPrivateAccess = "true"))
	//class UInputAction* NavigationRight;
	
	/*
	* Will use the crafting system to repair the item
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Action", meta = (AllowPrivateAccess = "true"))
	bool bRepairWithCraftingSystem;


	/** MappingContext 
	* This mapping context is apply when InventoryManager is create. Used in the world
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Hotbar", meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* InputMappingContextHotbar;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Hotbar", meta = (AllowPrivateAccess = "true"))
	int32 PriorityHotbar = 0;

	/** Hotbar Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Hotbar|Action", meta = (AllowPrivateAccess = "true"))
	TMap<FName, class UInputAction*> HotbarActions;

	/*
	* A double left mouse click will be used for the "UseItem" interaction
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input|Action", meta = (DisplayName = "LMB Double use item"))
	bool LeftMouseButtonDoubleClickForUseItem = true;

	/*
	* The right mouse button will be used for the "UseItem" interaction 
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input|Action", meta = (DisplayName = "RMB use item"))
	bool bRightMouseButtonForUseItem = false;

	class UEnhancedInputLocalPlayerSubsystem* GetInputSystem();

	UFUNCTION(BlueprintCallable, Category = "InventoryManager|Input")
	void SetupInventoryManagerInput();
	UFUNCTION(BlueprintCallable, Category = "InventoryManager|Input")
	void RemoveInventoryManagerInput(bool InventoryContext = true);

	void SetupInventoryManagerInputHotbar();

	void InputCallback_UseItem(const FInputActionValue& Value);
	void InputCallback_TransferItem(const FInputActionValue& Value);

	void TryToTransferById();
	void InputCallback_TransferIdItem(const FInputActionValue& Value);
	bool bLastActionValueTransferIdItem = false;

	void InputCallback_SplitItem(const FInputActionValue& Value);
	void InputCallback_DropItem(const FInputActionValue& Value);
	void InputCallback_RepairItem(const FInputActionValue& Value);
	void InputCallback_HotbarAction(const FInputActionValue& Value,FName HotbarKey);

	void InputCallback_ContextualMenu(const FInputActionValue& Value);
	void InputCallback_NavigationUp(const FInputActionValue& Value);
	void InputCallback_NavigationDown(const FInputActionValue& Value);
	void InputCallback_NavigationLeft(const FInputActionValue& Value);
	void InputCallback_NavigationRight(const FInputActionValue& Value);


#pragma endregion


	//-------------- Inventory
	/*
	* Declare the inventory that the player will have at the beginning of the game
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory",meta = (ExposeOnSpawn))
	TMap<FName, FInventoryDefinition> Inventorys;

	UPROPERTY()
	TMap<FName, UInventoryComponent*> PlayerInventorys;
	
	UPROPERTY(Replicated, SaveGame)
	TArray<FGuid> PlayerInventoryIds;


	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "InventoryManager|Inventory")
	UInventoryComponent* GetFirstPlayerInventory();
	
	/*
	* Return the first inventory can be transfer this Item
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "InventoryManager|Inventory")
	UInventoryComponent* GetSmartPlayerInventory(FItemData Item, bool OnlyOpen = true);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "InventoryManager|Inventory")
	FGuid GetSmartPlayerInventoryId(FItemData Item, bool OnlyOpen = true,FGuid FromInventory = FGuid(), bool StartLast = false);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "InventoryManager|Inventory")
	FGuid GetFirstPlayerInventoryId();


	//-------------- Equipment
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment", meta = (ExposeOnSpawn))
	TMap<FName, FSlotDefinition> Equipments;

	UPROPERTY()
	TMap<FGuid, FName> MatchContextEquipments;


	/*
	* -------------- Hotbar
	* Only available on Player
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hotbar", meta = (ExposeOnSpawn))
	TMap<FName, FSlotDefinition> Hotbars;

	/*
	* InventorySlotUserWidget class used Default_InventorySlot or create your Widget
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (DisplayName = "Widget InventorySlot"))
	TSubclassOf<UInventorySlotUserWidget> CustomClassSlot;
	/*
	* The widget that will be created and added in the PanelSocket located in the Inventory slot widget
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (DisplayName = "Widget Socket"))
	TSubclassOf<USocketUserWidget> SocketWidget;

	/*
	* GeneralItemScript Call this items script on all item used by this InventoryManager
	*/
	UPROPERTY(Instanced, EditAnywhere, BlueprintReadWrite, Category = "Settings")
	TArray<class UItemScript*> GeneralItemScript;

	// Relative Location
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FVector DroppedLocation;

	/*
	* If true when using the item and the container is open, then the item will be transferred 
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (DisplayName = "Enable Transfer when use Item"))
	bool bTransferWhenUseItem = true;

	/*
	* The player's hotbar will be used as a shortcut to items in the player's inventory
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (DisplayName = "Enable Hotbar Shortcut"))
	bool bUseHotbarAsShortcut = false;

	/*
	* If the value is false, you can't swap sockets, you must remove the gem
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (DisplayName = "Enable Swap Socket"))
	bool bCanSwapSocket = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Decay")
	float DecayFactor = 1.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Decay")
	float DecayTickTime = 1.0f;

	/*
	* When the component is created, it will look for the starting elements in RowStarter.
	* You can also use the "LoadInventoryManagerFromDataTable" function if you want to make a choice after the creation 
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Starter", meta = (DisplayName = "Enable starting items"))
	bool bStartingWithItem = false;

	/*Use ItemStarter structure in the data table*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Starter", meta = (EditCondition = "bStartingWithItem"))
	FDataTableRowHandle RowStarter;

	/*You can drag and drop Item*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Drag", meta = (DisplayName = "Enable Drag"))
	bool bDragItem = true;

	/*
	* Keep button pressed for Drag, realse button for drop
	* else One click for take item and another click for drop
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Drag", meta = (EditCondition = "bDragItem"))
	bool bEnableHoldButton = true;

	/*default divides to half item*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Drag", meta = (DisplayName = "Enable Half Split",EditCondition = "bDragItem"))
	bool bHalfSplitItem = true;

	/*Custom default split*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Drag", meta = (EditCondition = "bDragItem && bHalfSplitItem == false"))
	int32 DefaultAmountToSplit = 1;

	/*
	* Widget class Drag slot 
	* Use class UDraggedItemSlot
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Drag", meta = (EditCondition = "bDragItem"))
	class TSubclassOf<UDraggedItemSlot> WidgetDragSlot;

	
	/* You can repair object in Player/Equipement inventory*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|Repair", meta = (DisplayName = "Enable Repair")) 
	bool bCanRepair = true;

	/*Percentage of cost compared to craft*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|Repair", meta = (ClampMin = "0.0", ClampMax = "100.0", UIMin = "0.0", UIMax = "100.0", Units="Percent"))
	float RepairPercentage = 50;

	// Split On Drag Activate/Desactivate
	bool bUseSplitOnDrag = false;




	// ------------------------ WEIGHT SYSTEM ------------------------	
#pragma region /** Weight */

	/*
	* Active Weight System
	*/
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, SaveGame, Category = "Settings|Weight")
	bool EnableWeight = false;

	/*
	* if Total Weight is > MaxWeight can't add item on any inventory ownered of this InventoryManager
	*/
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, SaveGame, Category = "Settings|Weight", meta = (EditCondition = "EnableWeight"))
	bool EnableWeightRestriction = true;
	
	/*
	* Maximum weight avaiable of this InventoryManager
	*/
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, SaveGame, Category = "Settings|Weight", meta = (EditCondition = "EnableWeight", ClampMin = "0.0", UIMin = "0.0"))
	float MaxWeight = 300.f;

	/*
	* Factor apply on Total Weight 
	* x = GetTotalWeight(), y = Factor
	* x * y = New TotalWeight
	*/
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, SaveGame, Category = "Settings|Weight", meta = (EditCondition = "EnableWeight", ClampMin = "0.01", UIMin = "0.01"))
	float WeightFactor = 1.f;

	/*
	* Will try to retrieve all child inventories of the owner actor (Ex:Bag,Weapond...) 
	*/
	//UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, SaveGame, Category = "Settings|Weight", meta = (EditCondition = "EnableWeight"))
	//bool EnableWeightChildren = true;


	UPROPERTY(ReplicatedUsing = OnRep_CurrentWeight, BlueprintReadOnly, SaveGame, Category = "Inventory|Weight")
	float CurrentWeight = 0.f;

	UFUNCTION()
	virtual void OnRep_CurrentWeight();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_UpdateWeightValue(float NewTotalWeight);

	UPROPERTY(BlueprintAssignable, Category = "Container|Event")
	FOnWeightChanged OnWeightChanged;

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Inventory|Weight")
	void SetMaxWeight(float NewMaxWeight);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|Weight")
	float GetMaxWeight() { return MaxWeight; };

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Inventory|Weight")
	void SetWeightFactor(float NewWeightFactor);

	UFUNCTION(Server, Reliable)
	void SetTotalWeight(float NewTotalWeight);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|Weight")
	float GetTotalWeight() { return CurrentWeight * WeightFactor; };

	UFUNCTION()
	void OnChildrenWeightChanged(float TotalWeight, float NewMaxWeight);

	TArray<UInventoryComponent*> WeightInventorys;

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Inventory|Weight")
	void AddWeightInventorys(UInventoryComponent* Inventory, bool RefreshWeight = false);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Inventory|Weight")
	void RemoveWeightInventorys(UInventoryComponent* Inventory);

#pragma endregion

	/*
	* At initialization time, if the Owner can be Controller by a player (Client or ListenServer). 
	* It will ask the server to return the Hotbar Data
	*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Net")
	bool bEnableHotbarRefreshOnInit = true;

	/*
	* At initialization time, if the Owner can be Controller by a player (Client or ListenServer).
	* It will ask the server to return the Equipment Data
	*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Net")
	bool bEnableEquipmentRefreshOnInit = false;


	UFUNCTION(BlueprintCallable, Category = "InventoryManager|DragAndDrop")
	void ActivateSplitOnDrag() { bUseSplitOnDrag = true; };
	
	UFUNCTION(BlueprintCallable, Category = "InventoryManager|DragAndDrop", meta = (DisplayName="DeactivateSplitOnDrag"))
	void DesactivateSplitOnDrag() { bUseSplitOnDrag = false; };

	//Check if Owner is Localy Controlled
	bool GetIsLocallyControlled();

	// Subsystem
	UInventorySubsystem* InventorySubsystem;
	UInventorySubsystem* GetInvSubsystem();

	// Use GetIsRadyToUse Function
	UPROPERTY(BlueprintReadOnly, replicated, Category = "InventoryManager")
	bool bIsReadyToUse = false;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "InventoryManager|Inventory")
	bool GetIsReadyToUse() { return bIsReadyToUse; }




	//Client Pending Data UI (Grid Not Impl just HotbarSlot)
	TMap<FName,FGuid> RequestGrid;

	UFUNCTION()
	void OnPlayerGridAdded(FName GridKey, UInventoryUniformGridPanel* Grid);

	UFUNCTION()
	void OnHotbarSlotAdded(FName SlotKey);

	bool bClientPendingHotbar = false;
	void Client_PendingHotbar();



	/*-- Update Crafting & ItemBlueprint Amount/Max
	 * Stop On Decay TryToAdd
	 * Stop On UseTransferAll
	 */
	bool bCanUpdateCraft = true;

	UFUNCTION()
	void EquipementChanged(FPrimaryAssetId PrimaryAssetId, int32 Amount);


	// Inventory Component --------- With Function GET
	UInventoryComponent* EquipementPlayerInventory = nullptr;
	UInventoryComponent* HotbarInventory = nullptr;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "InventoryManager|Inventory", meta = (DisplayName = "Get Player Inventory"))
	UInventoryComponent* GetInventory(FName InventoryKey) { return *PlayerInventorys.Find(InventoryKey); };
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "InventoryManager|Inventory")
	UInventoryComponent* GetEquipementInventory() { return EquipementPlayerInventory; };
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "InventoryManager|Inventory")
	UInventoryComponent* GetHotbarInventory() { return HotbarInventory; };


	UFUNCTION(BlueprintCallable, Category = "InventoryManager|DragAndDrop")
	void PickupItemActorDropped(UDroppedComponent* DroppedComponent);
	UFUNCTION(Server,Reliable)
	void Server_PickupItemActorDropped(UDroppedComponent* DroppedComponent);


	/* Return All ItemData valid who is current inventory player*/
	UFUNCTION(BlueprintCallable, Category = "InventoryManager|Inventory")
	TArray<FItemData> GetAllItemsDataInPlayer(bool IncludePlayerInventory, bool IncludeEquipement, bool IncludeHotbar);

public : 
	// ---------------------- MANAGEMENT GET HOVER ITEM ---------------------- 

	// Variable use for return of data at hover
	FItemDataInfo HoverItemInformation;
	ESlotType HoverType;
	int32 HoverInventorySlot;
	bool HoverCurrentIsValid;
	FPrimaryAssetId HoverPrimaryAssetId;
	UInventorySlotUserWidget* HoverInventorySlotUserWidget = nullptr;
	FGuid HoverInventoryId;

	void SetCurrentItemOnMouseHover(bool CurrentIsValid, FPrimaryAssetId PrimaryAssetId, FItemDataInfo ItemInformation, ESlotType Type, int32 InventorySlot, UInventorySlotUserWidget* InventorySlotUserWidget, FGuid InventoryId);

	// Get Current hover item
	//if success is true then item is Valid 
	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintCosmetic, Category = "InventoryManager|Inventory",meta = (AdvancedDisplay=3))
	void GetCurrentItemOnMouseHover(bool& Success, FGuid& InventoryId, int32& InventorySlot, FPrimaryAssetId& PrimaryAssetId,FItemDataInfo& ItemInformation,ESlotType& Type, UInventorySlotUserWidget*& HoverSlotUserWidget);

	void LMBPressedOnSlot();
	void RMBPressedOnSlot();

	UPROPERTY(BlueprintAssignable, Category = "InventoryManager|Event")
	FLeftMousePressedOnSlot OnLeftMousePressedOnSlot;

	UPROPERTY(BlueprintAssignable, Category = "InventoryManager|Event")
	FRightMousePressedOnSlot OnRightMousePressedOnSlot;

public: 

	// ---------------------- SAVE AND LOAD ---------------------- 



	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "InventoryManager|SaveAndLoad")
	void LoadItemsOnInventory(UInventoryComponent* InInventory,const TArray<FItem>& Items);

	
	/*Assembly Data for save on variable*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "InventoryManager|SaveAndLoad")
	FItemForSave GetItemsForSave();
	
	TArray<FItemCurrency> GetCurrencyForSave() const { return AllCurrency; };

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "InventoryManager|SaveAndLoad")
	FInventoryManagerSaved GetInventoryManagerSave();

	UFUNCTION(BlueprintCallable, Category = "InventoryManager|SaveAndLoad")
	void LoadInventoryManagerFromSave(FInventoryManagerSaved InventoryManagerSaved);

// ------------------------ MANAGER SYSTEM ------------------------

public : 



	/*Event call end initialize inventory after the Owner is controlled
	* And/Or Loading Asset is Completed*/
	UPROPERTY(BlueprintAssignable, Category = "InventoryManager|Event")
	FEndBeginPLay EndBeginPlay;

	// Call when Equipement changed 
	UPROPERTY(BlueprintAssignable, Category = "InventoryManager|Event")
	FOnEquipementChanged OnEquipementChanged;
	
	UPROPERTY(BlueprintAuthorityOnly, BlueprintAssignable, Category = "InventoryManager|Event")
	FEquip OnEquip;

	UPROPERTY(BlueprintAuthorityOnly, BlueprintAssignable, Category = "InventoryManager|Event")
	FUnequip OnUnequip;
	
	// Create a New item and Try to add in inventory 
	// @OutIndex  Returns the last Index if there was more than one stack.
	UFUNCTION(BlueprintCallable, Category = "InventoryManager|Public", meta = (PrimaryAssetId = "ItemDataAsset"))
	void TryToAddItemToInventory(bool& Success, FItemData& OutItem, int32& OutIndex, UInventoryComponent* Inventory, FPrimaryAssetId PrimaryAssetId, int32 CustomAmount = -1, bool IsNewItem = false);

	// Try to add ItemData in inventory 
	// @OutIndex  Returns the last Index if there was more than one stack.
	UFUNCTION(BlueprintCallable, Category = "InventoryManager|Public")
	void TryToAddItemDataToInventory(bool& Success, FItemData& OutItem, int32 &OutIndex, UInventoryComponent* Inventory, FItemData ItemData, int32 CustomAmount = -1,bool IsNewItem = false);

	/*Try to remove this Item in Inventory*/
	UFUNCTION(BlueprintCallable, Category = "InventoryManager|Public")
	void TryToRemoveItemToInventory(bool& Success, int32& RemainingToRemove, UInventoryComponent* Inventory,FPrimaryAssetId PrimaryAssetId, int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "InventoryManager|Public")
	void CheckInventoryHasItemAmount(bool& HasItemAmount, int32& TotalAmountItem, UInventoryComponent* Inventory, FPrimaryAssetId PrimaryAssetId, int32 Amount);


	UFUNCTION(BlueprintCallable, Category = "InventoryManager|Public")
	bool CheckContainerHasItemCraft(TArray<FItemCraft> ItemsCraft, UInventoryComponent* Container);

	// ------- Item Script Event
	const TArray<UItemScript*> GetItemScripts(FName ItemID, bool AddGeneralItemScript = true, bool ExcludeInstance = true);

	// -- Init Inventory
	
	UFUNCTION(BlueprintAuthorityOnly,Server, Reliable)
	void Server_InitializeInventory(UInventoryComponent* Inventory);

	UFUNCTION(BlueprintAuthorityOnly,Server, Reliable)
	void Server_InitializeEquipement(int32 LengthFounds);

	UFUNCTION(BlueprintAuthorityOnly,Server, Reliable)
	void Server_InitializeHotbar(int32 LengthFounds);
	


	/*
	* Load Inventoryies - Equipment - Hotbar for this InventoryManager
	*/
	UFUNCTION(BlueprintCallable,BlueprintAuthorityOnly, Server, Reliable, Category = "InventoryManager|Load")
	void LoadInventoryManagerFromDataTable(FDataTableRowHandle RowHandle);

	void Server_Start_Inventorys(FName InventoryKey, FDataTableRowHandle RowHandle);
	void Server_Start_Equipments(FDataTableRowHandle RowHandle);
	void Server_Start_Hotbars(FDataTableRowHandle RowHandle);

	void BuildInstanceItemScript(FItemData& Item);

private:

	FItemData GenerateItem(FItemData& Item);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "InventoryManager|Server|Inventory")
	void AddItem(UInventoryComponent* Inventory, int32 InventorySlot, FItemData InventoryItem, bool IsNewItem = false);
public:
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "InventoryManager|Server|Inventory")
	void RemoveItem(UInventoryComponent* Inventory, int32 InventorySlot, bool Unfilled = true);
	
	/*
	* UnequipItem and Destory this use before load Inventory
	* After this function the item is lost
	*/
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "InventoryManager|Server|Inventory")
	void DestoryEquipmentItem(UInventoryComponent* FromInventory, int32 FromInventorySlot);

private:
	void HasItem(UInventoryComponent* Inventory, FName ItemID, bool& Success, int32& InventorySlot);
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "InventoryManager|Server|Inventory")
	void DropItemData(UInventoryComponent* Inventory,int32 InventorySlot, FItemData ItemData);
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "InventoryManager|Server|Inventory")
	void DropItem(UInventoryComponent* Inventory, int32 InventorySlot);
	
	// --------------------- Move Item --------------------- //
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "InventoryManager|Server|Inventory")
	void MoveItem(UInventoryComponent* FromInventory, int32 FromInventorySlot, UInventoryComponent* ToInventory, int32 ToInventorySlot);
	void HandleItemStack(UInventoryComponent* FromInventory, int32 FromInventorySlot, UInventoryComponent* ToInventory, int32 ToInventorySlot, FItemData& LocalInventoryItem);
	void HandleItemSwap(UInventoryComponent* FromInventory, int32 FromInventorySlot, UInventoryComponent* ToInventory, int32 ToInventorySlot, FItemData& LocalInventoryItem, FItemData& LocalSwapInventoryItem);
	void HandleItemMove(UInventoryComponent* FromInventory, int32 FromInventorySlot, UInventoryComponent* ToInventory, int32 ToInventorySlot, FItemData& LocalInventoryItem);


	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "InventoryManager|Server|Inventory")
	void EquipItem(UInventoryComponent* FromInventory, int32 FromInventorySlot, UInventoryComponent* ToInventory, int32 ToInventorySlot);
	void HandleItemEquip(UInventoryComponent* FromInventory, int32 FromInventorySlot, UInventoryComponent* ToInventory, int32 ToInventorySlot, FItemData& LocalInventoryItem);
	void HandleItemSwapEquip(UInventoryComponent* FromInventory, int32 FromInventorySlot, UInventoryComponent* ToInventory, int32 ToInventorySlot, FItemData& LocalInventoryItem, FItemData& LocalSwapEquipementItem);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "InventoryManager|Server|Inventory")
	void UnequipItem(UInventoryComponent* FromInventory, int32 FromInventorySlot, UInventoryComponent* ToInventory, int32 ToInventorySlot);
	void HandleItemUnequip(UInventoryComponent* FromInventory, int32 FromInventorySlot, UInventoryComponent* ToInventory, int32 ToInventorySlot, FItemData& LocalInventoryItem);
	void HandleItemSwapUnequip(UInventoryComponent* FromInventory, int32 FromInventorySlot, UInventoryComponent* ToInventory, int32 ToInventorySlot, FItemData& LocalInventoryItem, FItemData& LocalSwapEquipementItem);

	UFUNCTION(Server, Reliable,BlueprintCallable, Category = "InventoryManager|Server|Inventory")
	void SplitItem(UInventoryComponent* FromInventory, int32 FromInventorySlot, UInventoryComponent* ToInventory, int32 ToInventorySlot, int32 Amount);

	void BroadcastItemMovement(UInventoryComponent* Inventory, const FItemData& ItemLeaving, const FItemData& ItemEntering);

	bool CheckAutoFillInventory(FItemData ForItem, UInventoryComponent* ToInventory, int32& ToInventorySlot);

	// ------------ Actor In World
	/*
	* Spawn Actor Equipment and Load DataBytes
	*/
	UFUNCTION()
	void Server_SpawnActorEquiment(UInventoryComponent* EquipmentInventory, int32 EquipmentSlot);

	/*
	* Save Actor Equipment on DataBytes and destory Actor
	*/
	UFUNCTION(Server, Reliable)
	void Server_DestoryActorEquipment(UInventoryComponent* EquipmentInventory, int32 EquipmentSlot, bool IsDrop = false);




	/*
	* Checks if the slots we remove are empty
	* Return true if has item
	*/
	UFUNCTION(BlueprintCallable, Category = "InventoryManager|Public")
	bool CheckSlotsForItems(UInventoryComponent* Inventory, int32 Amount);


public:
	// ---------------------- INVENTORY SIZE ----------------------

	UFUNCTION(Server, Reliable,BlueprintCallable, Category = "InventoryManager|Server|Inventory")
	void Server_IncreaseInventorySize(FGuid InventoryId, int32 Amount);
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "InventoryManager|Server|Inventory")
	void Server_DecreaseInventorySize(FGuid InventoryId, int32 Amount);

	// ---------------------- FILTER MANAGEMENT ----------------------

	UFUNCTION(Server, Reliable,BlueprintCallable, Category = "InventoryManager|Filter")
	void EnableVisibilityFilter(FGuid InventoryId, FGameplayTagContainer Tags);
	UFUNCTION(Server, Reliable,BlueprintCallable, Category = "InventoryManager|Filter")
	void DisableVisibilityFilter(FGuid InventoryId, bool FillInventory);
	UFUNCTION(Server, Reliable,BlueprintCallable, Category = "InventoryManager|Filter")
	void FillInventorySlotVisible(FGuid InventoryId,int32 StartIndex = 0);
	UFUNCTION(Server, Reliable)
	void RefreshClientSlots(FGuid InventoryId);

	// ---------------------- SORT MANAGEMENT ----------------------
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "InventoryManager|Filter")
	void SortInventory(FGuid InventoryId, ESortItemRule SortRule,bool Invert, FGameplayTag Tag);



	// ---------------------- ATTIBUTES MANAGEMENT ----------------------

	/*
	* Add a new value or Update value with this tag
	* Add value on current attribute value or add new attriube with this value
	*/
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "InventoryManager|Attribute")
	void AddTagAttribute(UInventoryComponent* Inventory, int32 Slot, FGameplayTag AttributeTag, float Value);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "InventoryManager|Attribute")
	void RemoveTagAttribute(UInventoryComponent* Inventory, int32 Slot, FGameplayTag AttributeTag, float Value);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "InventoryManager|Attribute")
	float GetAttributeValue(UInventoryComponent* Inventory, int32 Slot, FGameplayTag AttributeTag) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "InventoryManager|Attribute")
	bool HasAttributeTag(UInventoryComponent* Inventory, int32 Slot, FGameplayTag AttributeTag) const;

	// Equipement
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "InventoryManager|Attribute")
	float GetEquipementAttributeValue(FName EquipmentKey, FGameplayTag AttributeTag);
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "InventoryManager|Attribute")
	float GetTotalAttributeValueEquipement(FGameplayTag AttributeTag);
	// Return Attribute Container for all equipements 
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "InventoryManager|Attribute")
	FGameplayTagAttributeContainer GetAttributesEquipement(bool IncludeSocket);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "InventoryManager|Attribute")
	FGameplayTagAttributeContainer GetAttributesEquipementByTag(FName EquipmentKey, bool IncludeSocket);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "InventoryManager|Attribute")
	FGameplayTagAttributeContainer GetAttributesEquipementByIndex(int32 IndexSlot, bool IncludeSocket);


	//@TODO maybe can delet
	FGameplayTagAttributeContainer ReconstrcutAttibutes(TMap<FGameplayTag, float> AttributesMap);

	UPROPERTY(BlueprintAuthorityOnly, BlueprintAssignable, Category = "InventoryManager|Event")
	FAttributeChanged OnAttributeChanged;


private:

	// ---------------------- STACK MANAGEMENT ---------------------- 

	int32 AddItemToStack(UInventoryComponent* Inventory, int32 InventorySlot, int32 AmountToAdd, float Decay);
	static FItemData AddToItemAmount(FItemData ItemData, int32 AmountToAdd);
	static void RemoveToItemAmount(bool& SlotIsEmpty, FItemData& OutItem, int32& AmountRemoved, FItemData Item, int32 AmountToRemove);
	int32 FindAndAddAmountToStack(UInventoryComponent* Inventory, FName ItemID, int32 Amount, float Decay);
	//Force Equipment Not Equal
	static bool EqualItem(FItemData Item1, FItemData Item2) { return (Item1.ID == Item2.ID && Item1.Icon == Item2.Icon && !(Item1.ItemType == EItemType::EEquipement && Item2.ItemType == EItemType::EEquipement)); }; 

	// ---------------------- CRAFTING MANAGEMENT ---------------------- 
public:

	UFUNCTION(Server,Reliable)
	void Server_OpenCrafting(UCraftingComponent* Crafting, UInventoryComponent* InInventory,FName GridKey);
	UFUNCTION(Server, Reliable)
	void Server_CloseCrafting(UCraftingComponent* Crafting, FName GridKey);

	static bool IsValidRecipe(UItemDataAsset* ItemDataAsset, bool PrintLog);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "InventoryManager|Crafting")
	int32 GetMaxCountCanCraft(TArray<FItemCraft> ItemHasNeed, FGuid InventoryId);

	/*
	* Adds to the variable InfoCraft :
	* The maximum number of Craft 
	* Ingredients available in the inventory
	*/
	void ApplyAdditionalCraftData(FGuid InventoryId, FItemDataInfoCraft& InfoCraft, TArray<int32>& RecipeAmount);

	UFUNCTION(Server, Reliable,BlueprintCallable, Category = "InventoryManager|Crafting")
	void Server_UpdateDataCraft(FGuid CraftingId, bool Limited = true);



	// ---------------------- WAIT LIST MANAGEMENT ---------------------- 
	UFUNCTION(BlueprintCallable, Category = "InventoryManager|Crafting")
	void AddCraftToWaitlist(FGuid CraftingId, FItemDataInfoCraft AddInfo, int32 Amount);
	
	UFUNCTION(Server, Reliable)
	void Server_AddCraftToWaitlist(FGuid CraftingId, FItemDataInfoCraft AddInfo, int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "InventoryManager|Crafting")
	void RemoveCraftInWaitlist(FGuid CraftingId,int32 IndexSlot);

	UFUNCTION(Server, Reliable)
	void Server_RemoveCraftInWaitList(FGuid CraftingId, int32 IndexSlot);
	UFUNCTION(Server, Reliable)
	void Server_RemoveCraftInWaitListForAllViewers(FGuid CraftingId, int32 IndexSlot);

	UFUNCTION(BlueprintCallable, Category = "InventoryManager|Crafting")
	void RefundRecipeByPercent(UInventoryComponent* ToInventory,FRecipeCraft RecipeToBeRefund, float Percent);
	UFUNCTION(BlueprintCallable, Category = "InventoryManager|Crafting")
	void RemoveIngredientNeededInInventory(UInventoryComponent* Inventory, FRecipeCraft Recipe);

public:
	// ---------------------- INVENTORY MANAGEMENT ---------------------- 
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "InventoryManager|Inventory", meta = (DisplayName = "Open Inventory"))
	void Server_OpenInventory(UInventoryComponent* Inventory, UCraftingComponent* Crafting,FName OverrideGridKey);
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "InventoryManager|Inventory", meta = (DisplayName = "Close Inventory"))
	void Server_CloseInventory(FGuid InventoryId);

	//void Server_OpenMultipleInventories()
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "InventoryManager|Inventory", meta = (DisplayName = "Close Multiple Inventory", AutoCreateRefTerm = "InventoryIds"))
	void Server_CloseMultipleInventories(ECloseInventoryPreset Preset, const TArray<FGuid>& InventoryIds);

	//Array for save the current inventory player opens by the players
	UPROPERTY(Replicated)
	TArray<FGuid> InventoriesOpen;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "InventoryManager|Inventory")
	FGuid GetLastInventoryOpen() { return InventoriesOpen.IsEmpty() ? FGuid() : InventoriesOpen.Last(); }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "InventoryManager|Inventory")
	TArray<FGuid> GetInventoriesOpen() { return InventoriesOpen; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "InventoryManager|Inventory")
	bool GetHasInventoryOpen() { return !InventoriesOpen.IsEmpty(); }

	//Array for save the current container opens by the players
	UPROPERTY(Replicated)
	TArray<FGuid> ContainersOpen;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "InventoryManager|Inventory")
	FGuid GetLastContainerOpen() { return ContainersOpen.IsEmpty() ? FGuid() : ContainersOpen.Last(); }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "InventoryManager|Inventory")
	TArray<FGuid> GetContainersOpen() { return ContainersOpen; }
	
	/*
	* Return the first Container open can be transfer this Item
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "InventoryManager|Inventory")
	FGuid GetSmartContainerOpen(FItemData Item);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "InventoryManager|Inventory")
	bool GetHasContainerOpen() { return !ContainersOpen.IsEmpty(); }

	//Array for save the current Equipment opens by the players
	UPROPERTY(Replicated)
	TArray<FGuid> EquipmentsOpen;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "InventoryManager|Inventory")
	FGuid GetLastEquipmentsOpen() { return EquipmentsOpen.IsEmpty() ? FGuid() : EquipmentsOpen.Last(); }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "InventoryManager|Inventory")
	TArray<FGuid> GetEquipmentsOpen() { return EquipmentsOpen; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "InventoryManager|Inventory")
	bool GetHasEquipmentsOpen() { return !EquipmentsOpen.IsEmpty(); }

	/*
	* Return the first Player Inventory or Last Container Open
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "InventoryManager|Inventory")
	FGuid GetSmartToInventory(FGuid FromInventory);

	/*
	* Return the first Player Inventory or Container Open with restriction possible
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "InventoryManager|Inventory")
	FGuid GetSmartToInventoryWithItem(FGuid FromInventory,FItemData Item);

	// ---------------------- EQUIPMENT MANAGEMENT ---------------------- 
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "InventoryManager|Inventory", meta = (DisplayName = "Open Equipment"))
	void Server_OpenEquipment(UInventoryComponent* Inventory, FName Context);
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "InventoryManager|Inventory", meta = (DisplayName = "Close Equipment"))
	void Server_CloseEquipment(FGuid EquipmentInventoryId);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "InventoryManager|Inventory", meta = (DisplayName = "Load Hotbar"))
	void Server_LoadHotbar();


public:
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "InventoryManager|Server|Inventory")
	void SetIsDraggable(FGuid InventoryId, int32 Slot, bool IsDraggable);
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "InventoryManager|Server|Inventory")
	void SetIsDroppable(FGuid InventoryId,  int32 Slot, bool IsDroppable);
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "InventoryManager|Server|Inventory")
	void SetDurability(FGuid InventoryId, int32 Slot,bool Increase, float Durability);
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "InventoryManager|Server|Inventory")
	void AddGameplayTagOnItem(FGuid InventoryId, int32 Slot, FGameplayTag Tag);
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "InventoryManager|Server|Inventory")
	void RemoveGameplayTagOnItem(FGuid InventoryId, int32 Slot, FGameplayTag Tag);
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "InventoryManager|Server|Inventory")
	void SetJsonParameter(FGuid InventoryId, int32 Slot, const FString& JsonParameter);

		//TODO SetGameplayTag

//-------- Lock Equipment

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "InventoryManager|Server|Inventory")
	void LockSlot(FGuid InventoryId, int32 Index);
	
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "InventoryManager|Server|Inventory")
	void UnlockSlot(FGuid InventoryId, int32 Index);

	UPROPERTY(replicated)
	FGameplayTagContainer EquipementTypeDisable;

	/*
	* Enable Slots containing the Tag
	*/
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "InventoryManager|Server|Inventory")
	void EnableEquipementSlot(FGameplayTag Tag);
	
	/*
	* Disable Slots containing the Tag
	*/
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "InventoryManager|Server|Inventory")
	void DisableEquipementSlot(FGameplayTag Tag, bool Unequip);
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "InventoryManager|Server|Inventory")
	FGameplayTagContainer GetEquiptementTypeDisable() { return EquipementTypeDisable; }
	
	bool CheckIsDisableSlot(FGameplayTagContainer Tags) { return GetEquiptementTypeDisable().HasAny(Tags); }

	bool CanBeEquip(FGameplayTagContainer Tags) { return !CheckIsDisableSlot(Tags); }


public:
	// ---------------------- MOVING MANAGEMENT ---------------------- 
	bool bCanMoveItem(FGuid FromInventoryId, int32 FromInventorySlot, FGuid ToInventoryId, int32 ToInventorySlot);

	UFUNCTION(Server, Reliable)
	void Server_MoveInventoryToInventory(FGuid FromInventoryId, int32 FromInventorySlot, FGuid ToInventoryId, int32 ToInventorySlot,bool bSplit);

	UFUNCTION(Server, Reliable)
	void Server_CancelMoveInventory(FGuid FromInventoryId, int32 FromInventorySlot,FGuid ToInventory, int32 ToInventorySlot);
	
	UFUNCTION(Server, Reliable)
	void Server_DropItem(FGuid InventoryId, int32 InventorySlot);

	UFUNCTION(Server, Reliable,BlueprintCallable, Category = "InventoryManager|Public")
	void Server_CustomSplitItem(FGuid InventoryId, int32 InventorySlot, int32 Amount);


	// ---------------------- SOCKET MANAGEMENT ---------------------- 
	void AddSocketToItem(UInventoryComponent* Inventory, int32 IndexSlot, FSocketItem Socket, int32 SocketSlot);
	void RemoveSocketToitem(UInventoryComponent* Inventory, int32 IndexSlot, int32 SocketSlot);
	void MoveSocketToSocket(UInventoryComponent* Inventory, int32 IndexSlot, int32 FromSocketSlot, int32 ToSocketSlot);
	void MoveSocketToAnotherItemSocket(UInventoryComponent* FromInventory, int32 FromIndexSlot, int32 FromSocketSlot, UInventoryComponent* ToInventory, int32 ToIndexSlot,  int32 ToSocketSlot);
	bool CheckSocketRestrictionSlot(UInventoryComponent* ToInventory, int32 ToIndexSlot, int32 ToSocketSlot, FSocketItem SocketCheck);

	UFUNCTION(Server, Reliable)
	void Server_MoveSocketItem(FGuid FromInventoryId, int32 FromSlot, int32 FromSocketSlot, FGuid ToInventoryId, int32 ToSlot, int32 ToSocketSlot);
	
	UFUNCTION(Server, Reliable)
	void Server_TakeSocketItem(FGuid FromInventoryId, int32 FromSlot, int32 FromSocketSlot, FGuid ToInventoryId, int32 ToSlot);
	
	UFUNCTION(Server, Reliable)
	void Server_DepositSocketItem(FGuid FromInventoryId, int32 FromSlot, FGuid ToInventoryId, int32 ToSlot, int32 ToSocketSlot);
	
	UFUNCTION(Server, Reliable)
	void Server_DropSocketItem(FGuid FromInventoryId, int32 FromSlot, int32 FromSocketSlot);

	UFUNCTION(Server, Reliable,BlueprintCallable, Category = "InventoryManager|Socket")
	void AddSocketOnItem(UInventoryComponent* Inventory, int32 InventorySlot, int32 Amount);

	// ---------------------- REPAIR ITEM ----------------------
	UFUNCTION(Server, Reliable)
	void Server_RepairItemWithRecipe(FGuid Type, int32 FromSlot,bool UseCraft);
	
	UFUNCTION(BlueprintCallable,BlueprintPure, Category = "InventoryManager|Repair")
	bool CanBeRepair(FItemDataInfoCraft& ItemDataInfoCraft,FItemDataInfo DataInfo);

	UFUNCTION(BlueprintCallable,BlueprintPure, Category = "InventoryManager|Repair")
	FItemDataInfoCraft ConvertRecipeToRepairCost(bool &HasResource, FGuid InventoryId, FItemDataInfoCraft ItemInfoCraft,int32 IndexSlot);

	// ---------------------- FILLED ITEM ----------------------
	UFUNCTION(Server, Reliable)
	void Server_FilledItemInventory(FGuid InvenotyId, int32 Index);

	UFUNCTION(Server, Reliable)
	void Server_UnfilledItemInventory(FGuid InvenotyId, int32 Index, int32 IgnoreItemSlot = -1);

public:
	// ---------------------- USE ITEM ---------------------- 
	UFUNCTION(Server, Reliable)
	void UseInventoryItem(FGuid InventoryId, int32 InventorySlot, EUsedInputTrigger InputTrigger = EUsedInputTrigger::EUndefine);
	
	UFUNCTION(Server, Reliable)
	void UseEquipementItem(FGuid FromInventoryId, int32 InventorySlot, FGuid ToInventoryId, EUsedInputTrigger InputTrigger = EUsedInputTrigger::EUndefine);
	
	UFUNCTION(Server, Reliable)
	void UseConsumableItemInInventory(UInventoryComponent* Inventory, int32 InventorySlot, EUsedInputTrigger InputTrigger = EUsedInputTrigger::EUndefine);
		
	UFUNCTION(Server, Reliable)
	void UseBlueprintItem(FGuid InventoryId, int32 InventorySlot,int32 Amount, EUsedInputTrigger InputTrigger = EUsedInputTrigger::EUndefine);

	UFUNCTION(Server, Reliable)
	void UseHotbarAsShortuct(FGuid InventoryId, FName ItemId, FName SlotKey, EUsedInputTrigger InputTrigger = EUsedInputTrigger::EUndefine);

	/*
	* Transfer item from Inventory To Auto found Inventory
	* Try to found Container Open if from Player Inventory and check if Inventory is full and Restriction 
	* Try to found Player Inventory if from Container Inventory and check if Inventory is full and Restriction 
	*/
	UFUNCTION(Server, Reliable,BlueprintCallable, Category = "InventoryManager|Use")
	void UseTransferItem(FGuid FromInventoryId, int32 FromSlot, FGuid OverrideToInventoryId);


	/* 
	* Transfer All items With  PrimaryAssetId from Inventory To Auto found Inventory
	* Try to found Container Open if from Player Inventory and check if Inventory is full and Restriction 
	* Try to found Player Inventory if from Container Inventory and check if Inventory is full and Restriction 
	* 
	* BETA 
	* If Quick is true, then do not try to find and fill stack
	* For multiplayer game maybe need to use Quick Transfer
	*/
	UFUNCTION(Server, Reliable,BlueprintCallable, Category = "InventoryManager|Use")
	void UseTransferAllItemById(FGuid FromInventoryId, FPrimaryAssetId PrimaryAssetId, FGuid OverrideToInventoryId,bool Quick = false);
	
	UFUNCTION(Server, Reliable,BlueprintCallable, Category = "InventoryManager|Use")
	void UseItemFromType(FGuid InventoryId, int32 InventorySlot, EUsedInputTrigger InputTrigger = EUsedInputTrigger::EUndefine);
	
	UFUNCTION(BlueprintCallable, Category = "InventoryManager|Use")
	void UseDropItemFromType(FGuid InventoryId, int32 FromSlot);
	/*If UseCraftingSystem is true then try to call AddCraftToWaitList with recipe repair*/

	UFUNCTION(BlueprintCallable, Category = "InventoryManager|Use")
	void UseRepairItemFromType(FGuid InventoryId, int32 FromSlot,bool UseCraftingSystem);
	
	UFUNCTION(BlueprintCallable, Category = "InventoryManager|Use")
	void UseBlueprintItemFromType(FGuid InventoryId, int32 InventorySlot,int32 Amount);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "InventoryManager|Use")
	void DecrementItem(UInventoryComponent* Inventory, int32 InventorySlot, int32 Amount = 1);
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "InventoryManager|Use")
	void IncrementItem(UInventoryComponent* Inventory, int32 InventorySlot, int32 Amount = 1);


	//Return Current value Add & update ToAdd with the remaining
	static void VirtualAddToStack(int32& Current,int32 MaxStack,int32& ToAdd);


public:
	UPROPERTY(BlueprintAuthorityOnly, BlueprintAssignable, Category = "InventoryManager|Event")
	FUpdateInventory OnUpdateInventory;

private:
	// ---------------------- CURRENCY MANAGEMENT ---------------------- 
	UPROPERTY(replicated, SaveGame)
	TArray<FItemCurrency> AllCurrency;

	UFUNCTION(BlueprintAuthorityOnly,Server, Reliable)
	void SetCurrencyByItemId(FName Id, int32 Amount);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "InventoryManager|Currency")
	void UpdateAllCurrency();

	bool GetIsCurrencyFromItemID(FName ItemID);

	UFUNCTION(Server, Reliable)
	void UpdateCurrency(FName Id);


	// Return -1 if not found
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "InventoryManager|Currency")
	int32 GetCurrencyByType(FName Id);

	UPROPERTY(BlueprintAuthorityOnly, BlueprintAssignable, Category = "InventoryManager|Event")
	FUpdateCurrency OnUpdateCurrency;

	// ------------------------------------------------ TRANSFER DATA ------------------------------------------------

	//V2
	void SetViewersInventorySlot(FGuid InventoryId, TMap<class UInventoryManagerComponent*, FName> Viewers, int32 InventorySlot, FItemDataInfoCompressed ItemTransfer);
	void ClearViewersInventorySlot(FGuid InventoryId, TMap<class UInventoryManagerComponent*, FName> Viewers, int32 InventorySlot);
	void SetViewersFilledInventorySlot(FGuid InventoryId, TMap<class UInventoryManagerComponent*, FName> Viewers, const TArray<int32>& IndexFilled);
	void SetViewersUnfilledInventorySlot(FGuid InventoryId, TMap<class UInventoryManagerComponent*, FName> Viewers, const TArray<int32>& IndexUnfilled);
	void SetViewersCancelOverSlot(FGuid InventoryId, TMap<class UInventoryManagerComponent*, FName> Viewers, const TArray<int32>& IndexCancel);
	void SetViewersRepairSlot(FGuid InventoryId, TMap<class UInventoryManagerComponent*, FName> Viewers, int32 InventorySlot, FItemDataInfoCraftCompressed Repair);
	void SetViewersBlueprintSlot(FGuid InventoryId, TMap<class UInventoryManagerComponent*, FName> Viewers, int32 InventorySlot, FItemDataInfoCraftCompressed Blueprint);
	void SetViewersDurabilitySlot(FGuid InventoryId, TMap<class UInventoryManagerComponent*, FName> Viewers, int32 InventorySlot, float NewDurability);
	void SetViewersAttributesSlot(FGuid InventoryId, TMap<class UInventoryManagerComponent*, FName> Viewers, int32 InventorySlot, FGameplayTagAttributeContainer Attributes);
	void SetViewersGameplayTagSlot(FGuid InventoryId, TMap<class UInventoryManagerComponent*, FName> Viewers, int32 InventorySlot, FGameplayTag Tag, bool IsAdd);
	void SetViewersJsonParameterSlot(FGuid InventoryId, TMap<class UInventoryManagerComponent*, FName> Viewers, int32 InventorySlot, FString JsonParameter);
	void SetViewersSocketSlot(FGuid InventoryId, TMap<class UInventoryManagerComponent*, FName> Viewers, int32 InventorySlot, int32 NewSocketSlot, FSocketItem NewSocket);
	void SetViewersLockSlot(FGuid InventoryId, TMap<class UInventoryManagerComponent*, FName> Viewers, int32 InventorySlot);
	void SetViewersUnlockSlot(FGuid InventoryId, TMap<class UInventoryManagerComponent*, FName> Viewers, int32 InventorySlot);
	void SetViewersItemCrafting(FGuid InventoryId, TMap<class UInventoryManagerComponent*, FName> Viewers, int32 InventorySlot, FItemDataInfoCraftCompressed Info);
	void SetViewersItemAmountCrafting(FGuid InventoryId, TMap<class UInventoryManagerComponent*, FName> Viewers, int32 InventorySlot, const TArray<int32>& ItemCraftAmount, int32 MaxAmount);
	void AddViewersWaitingSlot(FGuid CraftingId, TMap<class UInventoryManagerComponent*, FName> Viewers, int32 InventorySlot, FItemDataInfoCraftCompressed Info, TSubclassOf<UCraftingSlotUserWidget> ClassWaiting);

public:
	void AddViewersItemCrafting(FGuid CraftingId, int32 InventorySlot, TMap<class UInventoryManagerComponent*, FName> Viewers, FItemDataInfoCraftCompressed Info);
	void RemoveViewersItemCrafting(FGuid CraftingId, int32 InventorySlot, TMap<class UInventoryManagerComponent*, FName> Viewers);

	void SetViewersWaitingSlot(FGuid CraftingId, TMap<class UInventoryManagerComponent*, FName> Viewers, int32 InventorySlot, FItemDataInfoCraftCompressed Info);
	void RemoveViewersWaitingSlot(FGuid CraftingId, TMap<class UInventoryManagerComponent*, FName> Viewers, int32 InventorySlot);
	void StartViewersWaitingSlot(FGuid CraftingId, TMap<class UInventoryManagerComponent*, FName> Viewers, int32 InventorySlot, float TimeOfCraft);
	void UpdateProgressViewersWaitingSlot(FGuid CraftingId, TMap<class UInventoryManagerComponent*, FName> Viewers, int32 InventorySlot, float Current);
	void EndViewersWaitingSlot(FGuid CraftingId, TMap<class UInventoryManagerComponent*, FName> Viewers, int32 InventorySlot);

// ------------------------ USER INTERFACE ------------------------

	// Initialisation inventory UI 
public:

	void InitializeInventoryPanelUI(FGuid InventoryId, FName GridKey,bool UsingSizeItem,int32 SlotPerRow, TSubclassOf<UInventorySlotUserWidget> OverrideInventoryWidgetSlot, const TArray<FItemDataInfoCompressed>& DefaultValue = TArray<FItemDataInfoCompressed>());


private:
	// ---------------------- INVENTORY USER INTERFACE ---------------------- 
// ---------- CLIENT ----------

	// --- Create Inventory Slots 
	void CreateInventorySlots(FGuid InventoryId,UInventoryUniformGridPanel* Grid, bool UsingSizeItem, TSubclassOf<UInventorySlotUserWidget> OverrideInventoryWidgetSlot, int32 InventorySizeTotal, int32 InventorySlotPerRow, TArray<FItemDataInfoCompressed> DefaultValue,bool Increment = false);
	void AddInventorySlot(FGuid InventoryId, UInventoryUniformGridPanel* Grid, bool UsingSizeItem, TSubclassOf<UInventorySlotUserWidget> OverrideInventoryWidgetSlot, int32 Row, int32 Column, int32 Slot, FItemDataInfoCompressed DefaultValue);

	/*Open Inventory*/
	UFUNCTION(Client, Reliable,BlueprintCallable, Category = "InventoryManager|Inventory", meta = (DisplayName = "Open Inventory (Deprecated)", DeprecatedFunction, DeprecationMessage = "Use the New OpenInventory function"))
	void Client_OpenInventory(FGuid InventoryId,FName GridKey, bool UsingSizeItem, int32 SlotPerRow, TSubclassOf<UInventorySlotUserWidget> OverrideInventoryWidgetSlot, const TArray<FItemDataInfoCompressed>& DefaultValue);
	/*Close Inventory*/
	UFUNCTION(Client, Reliable, BlueprintCallable, Category = "InventoryManager|Inventory", meta = (DisplayName = "Close Inventory (Deprecated)", DeprecatedFunction, DeprecationMessage = "Use the New CloseInventory function"))
	void Client_CloseInventory(FGuid InventoryId, FName GridKey,bool RemoveInput);

	UFUNCTION(Client, Reliable)
	void Client_LoadInventory(FGuid InventoryId, FName GridKey, bool UsingSizeItem, int32 SlotPerRow, TSubclassOf<UInventorySlotUserWidget> OverrideInventoryWidgetSlot, const TArray<FItemDataInfoCompressed>& DefaultValue);

	UFUNCTION(Client, Reliable)
	void Client_ClearInventory(FName GridKey);

	UFUNCTION(Client, Reliable)
	void Client_SetInventorySlotItem(FGuid InventoryId, FName GridKey,int32 InventorySlot, FItemDataInfoCompressed ItemInformationCompressed);
	
	UFUNCTION(Client, Reliable)
	void Client_ClearInventorySlotItem(FGuid InventoryId, FName GridKey, int32 InventorySlot,bool CloseInventory = false);

	UFUNCTION(Client, Reliable)
	void Client_FilledInventorySlots(FGuid InventoryId, FName GridKey, const TArray<int32>& InventorySlots);

	UFUNCTION(Client, Reliable)
	void Client_UnfilledInventorySlots(FGuid InventoryId, FName GridKey, const TArray<int32>& InventorySlots);

	UFUNCTION(Client, Reliable)
	void Client_CancelOverSlots(FGuid InventoryId, FName GridKey, const TArray<int32>& InventorySlots);
	
	UFUNCTION(Client, Reliable)
	void Client_SetInventoryRepairSlotItem(FGuid InventoryId, FName GridKey,int32 InventorySlot, FItemDataInfoCraftCompressed ItemInformationCraftCompressed);

	UFUNCTION(Client, Reliable)
	void Client_SetInventoryBlueprintSlotItem(FGuid InventoryId, FName GridKey, int32 InventorySlot, FItemDataInfoCraftCompressed ItemInformationCraftCompressed);

	UFUNCTION(Client, Reliable)
	void Client_IncreaseInventorySlots(FGuid InventoryId, FName GridKey, bool UsingSizeItem, TSubclassOf<UInventorySlotUserWidget> OverrideInventoryWidgetSlot, int32 Amount);
	UFUNCTION(Client, Reliable)
	void Client_DecreaseInventorySlots(FGuid InventoryId, FName GridKey, int32 Amount);
public:

	UFUNCTION(Client, Reliable)
	void Client_OnItemUsed(FGuid OnInventoryId,FName GridKey, int32 OnSlot,EUsedInputTrigger InputTrigger);

private:

	// ---------------------- DURABILITY USER INTERFACE ----------------------	
	UFUNCTION(Client, Reliable)
	void Client_SetDurabilitySlotItem(FGuid InventoryId, FName GridKey, int32 InventorySlot, float Durability);

	UFUNCTION(Client, Reliable)
	void Client_SetDecaySlotItem(FGuid InventoryId, FName GridKey, int32 InventorySlot, float Decay,float OnDecayFactor);

	// ---------------------- ATTRIBUTES USER INTERFACE ----------------------
	UFUNCTION(Client, Reliable)
	void Client_SetAttributesSlotItem(FGuid InventoryId, FName GridKey, int32 InventorySlot,FGameplayTagAttributeContainer Attributes);

	// ---------------------- GameplayTag USER INTERFACE ----------------------
	UFUNCTION(Client, Reliable)
	void Client_SetGameplayTagSlotItem(FGuid InventoryId, FName GridKey, int32 InventorySlot, FGameplayTag Tag, bool IsAdd);

	// ---------------------- JSON PARAMATER USER INTERFACE ----------------------
	UFUNCTION(Client, Reliable)
	void Client_SetJsonParameterSlotItem(FGuid InventoryId, FName GridKey, int32 InventorySlot,const FString& JsonParameter);


	// ---------------------- SOCKET USER INTERFACE ----------------------
	UFUNCTION(Client, Reliable)
	void Client_SetSocketSlotItem(FGuid InventoryId, FName GridKey, int32 InventorySlot, int32 NewSocketSlot, FSocketItem NewSocket);

	// ---------------------- LOCK USER INTERFACE ----------------------
	UFUNCTION(Client, Reliable)
	void Client_LockSlot(FGuid InventoryId, FName GridKey, int32 Index);

	UFUNCTION(Client, Reliable)
	void Client_UnlockSlot(FGuid InventoryId, FName GridKey, int32 Index);

	// ---------------------- EQUIPEMENT USER INTERFACE ---------------------- 
	UFUNCTION(Client, Reliable)
	void Client_LoadEquipement(FGuid InventoryId, FName Context, const TArray<FItemDataInfoCompressed>& DefaultValue);

	UFUNCTION(Client, Reliable)
	void Client_ClearEquipement(FGuid InventoryId);

	UFUNCTION(Client, Reliable)
	void Client_SetEquipementSlotItem(FGuid InventoryId,int32 Index, FItemDataInfoCompressed ItemToTransfer);

	UFUNCTION(Client, Reliable)
	void Client_ClearEquipementSlotItem(FGuid InventoryId, int32 Index,bool CloseInventory = false);

	UFUNCTION(Client, Reliable)
	void Client_EnableEquipementSlot(FGuid InventoryId, FGameplayTag Tag);

	UFUNCTION(Client, Reliable)
	void Client_DisableEquipementSlot(FGuid InventoryId, FGameplayTag Tag);


	// ---------------------- HOTBAR USER INTERFACE ---------------------- 
private:
	FItemDataInfo GetHotBarSlotItem(FName SlotKey);

public:

	UFUNCTION(Client, Reliable)
	void Client_LoadHotbar(FGuid InventoryId, const TArray<FItemDataInfoCompressed>& DefaultValue);
	
	UFUNCTION(Client, Reliable)
	void Client_ClearHotbar(FGuid InventoryId);

	UFUNCTION(Client, Reliable)
	void Client_SetHotbarSlotItem(FGuid InventoryId, int32 InventorySlot, FItemDataInfoCompressed ItemToTransfer);
	
	UFUNCTION(Client, Reliable)
	void Client_ClearHotBarSlotItem(FGuid InventoryId, int32 HotbarSlot);
	
	UFUNCTION(Client, Reliable)
	void Client_MoveHotbarSlotItem(int32 FromSlot, int32 ToHotbarSlot, bool FromInventory, bool FromHotbar, FItemDataInfoCompressed FromItem);
	
	UFUNCTION(BlueprintCallable, Category = "InventoryManager|Use", meta = (DisplayName = "Use HotBar Slot"))
	void Client_UseHotBarSlot(FName SlotKey,EUsedInputTrigger InputTrigger = EUsedInputTrigger::EUndefine);


	// ---------------------- CRAFTING USER INTERFACE ---------------------- 
private:
	UFUNCTION(Client, Reliable)
	void Client_LoadCrafting(FGuid CraftingId, UCraftingComponent* Crafting, const TArray<FItemDataInfoCraftCompressed>& ListItemCraftToLoad, const TArray<FItemDataInfoCraftCompressed>& WaitlistToLoad, FName GridKey);

	UFUNCTION(Client, Reliable)
	void Client_UnloadCrafting(FGuid CraftingId, FName GridKey);
	
	// ----------------------Crafting
	void CreateCraftingSlot(FGuid CraftingId, int32 Count, TSubclassOf<UCraftingSlotUserWidget> Class, FName GridKey, TArray<FItemDataInfoCraftCompressed> DefaultValue, UCraftingComponent* CraftingComp);
	
	UFUNCTION(Client, Reliable)
	void Client_AddCraftingSlot(FGuid CraftingId, int32 Slot, TSubclassOf<UCraftingSlotUserWidget> Class, FName GridKey, FItemDataInfoCraftCompressed DefaultValue, UCraftingComponent* CraftingComp);

	UFUNCTION(Client, Reliable)
	void Client_RemoveCraftingSlot(FGuid CraftingId, int32 Slot, FName GridKey);
	
	UFUNCTION(BlueprintCallable, Category = "InventoryManager|Crafting")
	void AddCustomCraftingSlot(UCraftingComponent* Crafting, UCraftingSlotUserWidget* CraftingSlot, FName Gridkey, UItemDataAsset* ItemDataAsset);
	UFUNCTION(Client, Reliable)
	void Client_AddCustomCraftingSlot(UCraftingComponent* Crafting, UCraftingSlotUserWidget* CraftingSlot, FName Gridkey, UItemDataAsset* ItemDataAsset);

	UFUNCTION(Client, Reliable)
	void Client_ClearCraftingSlot(FName GridKey);
	
	UFUNCTION(Client, Reliable)
	void Client_SetItemCrafting(FName GridKey, int32 Slot, FItemDataInfoCraftCompressed Info);

	UFUNCTION(Client, Reliable)
	void Client_SetItemCraftingAmount(FName GridKey, int32 Slot, const TArray<int32>& ItemCraftAmount, int32 MaxAmount);

	//----------------------Waiting
	void CreateWaitingSlot(FGuid CraftingId, TSubclassOf<UCraftingSlotUserWidget> ClassWaiting, FName GridKey, TArray<FItemDataInfoCraftCompressed> DefaultValue);
	
	UFUNCTION(Client, Reliable)
	void Client_AddWaitingSlot(FGuid CraftingId, int32 Slot, TSubclassOf<UCraftingSlotUserWidget> ClassWaiting, FName GridKey, FItemDataInfoCraftCompressed DefaultValue);

	UFUNCTION(Client, Reliable)
	void Client_RemoveWaitingSlot(int32 Slot, FName GridKey);

	UFUNCTION(Client, Reliable)
	void Client_SetItemWaiting(FName GridKey, int32 Slot, FItemDataInfoCraftCompressed NewInfo);

	UFUNCTION(Client, Reliable)
	void Client_ClearWaitingSlot(FName GridKey);

	UFUNCTION(Client, Reliable)
	void CLient_Start_Crafting(FName GridKey, int32 Slot, float TimeOfCraft);

	UFUNCTION(Client, Reliable)
	void Client_Update_Crafting(FName GridKey, int32 Slot, float CurrentTimer);
	
	UFUNCTION(Client, Reliable)
	void Client_End_Crafting(FName GridKey, int32 Slot);


public:

	// ---------------------- FILTER CRAFTING MANAGEMENT ----------------------
	UFUNCTION(Client, Reliable,BlueprintCallable, Category = "InventoryManager|Filter")
	void EnableVisibilityCraftingFilter(FName GridKey, FGameplayTagContainer Tags);
	
	UFUNCTION(Client, Reliable,BlueprintCallable, Category = "InventoryManager|Filter")
	void DisableVisibilityCraftingFilter(FName GridKey);
	
private:
	
	//Refresh only data needed on ItemType Blueprint 
	UFUNCTION(Client, Reliable)
	void Client_RefreshItemBlueprintInfoSlots(ESlotType Type);



public:

	// ---------- SERVER ----------
	/*Server RefreshSlot Data Inventory on Client UI*/
	UFUNCTION(BlueprintAuthorityOnly, Server, Reliable, BlueprintCallable,Category = "InventoryManager|Server")
	void RefreshInventorySlots(FGuid InventoryId,FName GridKey);
	
	/*Server RefreshSlot Data Equipement on Client UI*/
	UFUNCTION(BlueprintAuthorityOnly, Server, Reliable, BlueprintCallable, Category = "InventoryManager|Server")
	void RefreshEquipementSlots(bool TriggerScript = true, FName Context = FName("None"));
	
	/*Server RefreshSlot Data Hotbar on Client UI*/
	UFUNCTION(BlueprintAuthorityOnly, Server, Reliable, BlueprintCallable, Category = "InventoryManager|Server")
	void RefreshHotbarSlots();
	


	// ---------- SERVER STACK INIT SLOT PLAYER INVENTORY ---------- //

	void SendDataInventorySlots(FGuid InventoryId,FName GridKey,int32 StartStack, int32 EndStack);

	//Smooth data send to the client at startup (Only on Player Inventory) 
	UPROPERTY(BlueprintReadWrite,EditAnywhere,Category= "Net")
	bool bEnableSmoothNet = false;
	// Number of slot loading by stack
	UPROPERTY(BlueprintReadWrite,EditAnywhere,Category= "Net", meta = (EditCondition = "bEnableSmoothNet"))
	int32 StackSize = 70;
	UPROPERTY(BlueprintReadWrite,EditAnywhere,Category= "Net", meta = (EditCondition = "bEnableSmoothNet"))
	float DelayBetweenStack = 0.35f;

	int32 CurrentSlotStack = 0;
	FGuid NetCurrentInventoryId;
	FName NetCurrentGridKey;

	UFUNCTION(BlueprintCallable,BlueprintPure, Category = "InventoryManager|Net")
	int32 GetCurrentSlotStack(){return CurrentSlotStack;};
	
	FTimerHandle TimerStackNet;

	UFUNCTION(Server,Reliable)
	void StartTimerStackNet();
	UFUNCTION(Server,Reliable)
	void CurrentTimerStackNet();


	// Decay
	UFUNCTION(Server, Reliable)
	void Server_UpdateDecaySlots(FGuid InventoryId,FName GridKey, const TArray<int32>& DecaySlots, const TArray<bool>& MaxRefresh);

	//Repair
	UFUNCTION(Server, Reliable)
	void Server_SetRepairInfoSlot(FGuid InventoryId, int32 IndexSlot);


	// Item Blueprint Type
	UFUNCTION(Server, Reliable)
	void Server_SetItemBlueprintInfoSlot(FGuid InventoryId, int32 IndexSlot);
	//Function trigger by the InventoryComponent
	UFUNCTION(Server, Reliable)
	void Server_UpdateItemBlueprint(FGuid InventoryId);
	

public:
	// For Debug
	void PrintHasAuthority(FName print);
	void PrintPlayerHasAuthority(FString print);
	FString NetModeToString(ENetMode NetMode);

private:

	UInventoryComponent* GetInventory(FGuid InventoryId);
	UCraftingComponent* GetCrafting(FGuid CraftingId);
	UInventoryUniformGridPanel* GetGrid(FName GridKey);
	UInventorySlotUserWidget* GetEquipmentSlot(FName Context, FName SlotKey);
	UInventorySlotUserWidget* GetHotbarSlot(FName SlotKey);
	UInventorySlotUserWidget* GetAdvancedSlot(FGuid InventoryId, FName GridKey, int32 InventorySlot,FString DebugText = "");
};














