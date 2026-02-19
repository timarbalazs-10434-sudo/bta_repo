// Copyright 2024 Arkai Interactive. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InventoryPluginStruct.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/AssetManager.h"
#include "Engine/AssetManagerSettings.h"
#include "ChestorySaveGame.h"
#include "Engine/World.h"
#include "InventorySubsystem.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLoadingStoreItemsCompleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGridAdded,FName,GridKey, UInventoryUniformGridPanel*,Grid);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGridRemoved,FName,GridKey);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHotbarSlotAdded,FName,SlotKey);


class UItemDataAsset;
class UInventoryUniformGridPanel;
class UInventoryManagerComponent;
class UInventorySlotUserWidget;
class UInventoryComponent;


UENUM(BlueprintType)
enum class ETokenPolicy : uint8
{

  /*Using NetUniqueId Token*/
  OnlineSubsystem,

  /*Using AccountId Token*/
  OnlineServices,

  /*Using Player name Token*/
  PlayerName

};

/**
 * 
 */
UCLASS()
class CHESTORY_API UInventorySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
    // Begin USubsystem
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    
    // End USubsystem

    UItemDataAsset* ForceLoadItem(const FPrimaryAssetId& PrimaryAssetId, bool bLogWarning = true);

    static const FPrimaryAssetType	ItemDataType;

    // List all items loaded in game
    TArray<UItemDataAsset*> StoreItems;

    // All currency loaded in game
    TArray<FItemCurrency> StoreCurrency;

    UPROPERTY(BlueprintAssignable, Category = "InventorySubsytem|Event")
    FLoadingStoreItemsCompleted OnLoadingStoreItemsCompleted;

    // Current in Dev for Auto Rule AssetManagerSetting 
  //  UFUNCTION(BlueprintCallable, Category = "InventorySubsytem")
    void AddRuleInAssetManager();

public: 
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "InventorySubsytem")
    UItemDataAsset* GetItemDataAssetFromPrimaryAssetId(FPrimaryAssetId PrimaryAssetId);
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "InventorySubsytem")
    FItemData GetItemDataFromPrimaryAssetId(FPrimaryAssetId PrimaryAssetId);
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "InventorySubsytem")
    FPrimaryAssetId GetPrimaryAssetIdFromName(FName NameAsset);

    // Return all Item Data Asset available in project
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "InventorySubsytem")
    TArray<UItemDataAsset*>GetStoreItems() { return StoreItems; };

    // Return all currency available in project
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "InventorySubsytem")
    TArray<FItemCurrency>GetStoreCurrency() { return StoreCurrency; };

    UFUNCTION(BlueprintCallable, Category = "InventorySubsytem")
    TArray<FPrimaryAssetId> FindItemsWithCraftingTag(FGameplayTagContainer CraftingTags);

    UFUNCTION(BlueprintCallable, Category = "InventorySubsytem")
    TArray<FPrimaryAssetId> FindItemsWithItemsGameplayTags(FGameplayTagQuery TagQuery);

    /* Return true if the data of items is loaded
    *else you have to use bind event OnLoadingStoreItemsComplete */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "InventorySubsytem", meta = (DisplayName = "IsLoaded"))
    bool CheckIfDataIsAlreadyLoaded();

    bool CheskHasSetting();

    // IS UI has Alerady loaded clear old UI Data
    bool CompletedUI;
    // All items is load 
    bool bItemsCompletedLoad = false;

    bool bUIEquipementIsCompleted = false;


    //----------------- ItemId List Cache
    
    TMap<FGameplayTag, TArray<FName>> ItemsIdCache;

    TArray<FName> GetItemsIdWithGameplayTag(FGameplayTag Tag);


private : 

    TArray<FPrimaryAssetId> ListPrimaryAssetIdOut;

    UFUNCTION()
    void InitializeStoreItems();
    void EndLoadingItems();

    void ClearStoreItems() { StoreItems.Empty(); };
    void AddItemInStore(UItemDataAsset* Item) { StoreItems.Add(Item); };


    //****************************** NEW INIT SYSTEM ******************************//
public:
    //ULevel* level,UWorld* World
    UFUNCTION()
    void ResetDataLevel(const FString& MapName);

    UFUNCTION()
    void ResetDataLevelV2();
    FWorldDelegates::FOnLevelChanged LevelChanged;

    UFUNCTION()
    void ResetDataLevelV3(const FWorldContext& WorldContext, const FString&  MapName );

    UFUNCTION(BlueprintCallable, Category = "InventorySubsytem|Debug")
    void ResetUIRegister();

    //***************** InventoryManagerComponent  *****************//


    void SetInventoryManagerInstance(UInventoryManagerComponent* InventoryManager);

    UFUNCTION(BlueprintCallable, Category = "InventorySubsytem|Debug")
    UInventoryManagerComponent* GetInventoryManagerInstance() { return InventoryManagerInstance; }

    void ClearInventoryManagerInstance(UInventoryManagerComponent* InventoryManager);
   
    //***************** GRID  *****************//

    void RegisterInventoryGrid(FName GridKey, UInventoryUniformGridPanel* Grid);
    void UnregisterInventoryGrid(FName GridKey);
    bool ContainsGrid(FName GridKey) { return PlayerGrids.Contains(GridKey); };

    UInventoryUniformGridPanel* GetInventoryGrid(FName GridKey);

    FGridAdded OnPlayerGridAdded;
    FGridRemoved OnPlayerGridRemoved;

    //***************** SLOT  *****************//

    void RegisterEquipmentSlot(FName Context, FName SlotKey, UInventorySlotUserWidget* Slot);
    void UnregisterEquipmentSlot(FName Context, FName SlotKey);
    bool ContainsEquipmentSlot(FName Context, FName SlotKey) {return EquipmentSlots.Contains(TPair<FName, FName>(Context, SlotKey)); }
    UInventorySlotUserWidget* GetEquipmentSlot(FName Context, FName SlotKey);
    TArray<UInventorySlotUserWidget*> GetEquipmentSlots(FName Context);

    void RegisterHotbarSlot(FName SlotKey, UInventorySlotUserWidget* Slot);
    void UnregisterHotbarSlot(FName SlotKey);
    bool ContainsHotbarSlot(FName SlotKey) { return HotbarSlots.Contains(SlotKey); }
    int32 GetHotbarSlotNum() { return HotbarSlots.Num(); }
    bool HasHotbars() { return !HotbarSlots.IsEmpty(); }
    UInventorySlotUserWidget* GetHotbarSlot(FName SlotKey);
    TArray<UInventorySlotUserWidget*> GetHotbarSlots();


    FHotbarSlotAdded OnHotbarSlotAdded;

    //***************** InventoryComponent  *****************//

    void RegisterInventory(FGuid InventoryId, UInventoryComponent* Inventory);
    void UnregisterInventory(FGuid InventoryId);
    bool ContainsInventory(FGuid InventoryId) { return InventorysData.Contains(InventoryId); };
    UInventoryComponent* GetInventory(FGuid InventoryId);

    UFUNCTION(BlueprintCallable, Category = "InventorySubsytem|Debug")
    void  Debug_PrintFromInventory(UInventoryComponent* Inventory);

    UFUNCTION(BlueprintCallable, Category = "InventorySubsytem|Debug")
    void  Debug_PrintAllInventorys();

    //***************** CraftingComponent  *****************//

    void RegisterCrafting(FGuid CraftingId, class UCraftingComponent* Crafting);
    void UnregisterCrafting(FGuid CraftingId);
    bool ContainsCrafting(FGuid CraftingId) { return CraftingsData.Contains(CraftingId); };
    class  UCraftingComponent* GetCrafting(FGuid CraftingId);

private:


    TMap<FName, UInventoryUniformGridPanel*> PlayerGrids;

    TMap<TPair<FName, FName>, UInventorySlotUserWidget*> EquipmentSlots;
    TMap<FName, UInventorySlotUserWidget*> HotbarSlots;


    TMap<FGuid, UInventoryComponent*> InventorysData;

    TMap<FGuid,class UCraftingComponent*> CraftingsData;

    UInventoryManagerComponent* InventoryManagerInstance = nullptr;



    //****************************** NEW SAVE SYSTEM ******************************//

public: 

     /**
     * Save the data of all the inventories of the world
     * @param SlotName			Name of save game slot to save to.
	 * @param UserIndex			The platform user index that identifies the user doing the saving, ignored on some platforms.
     */
     UFUNCTION(BlueprintAuthorityOnly,BlueprintCallable, Category = "InventorySubsytem|SaveGame")
     void WriteInventorySaveGame(FString SlotName,int32 UserIndex);

     /**
     * Load the data of all the inventories of the world
     * @param SlotName			Name of save game slot to save to.
	 * @param UserIndex			The platform user index that identifies the user doing the saving, ignored on some platforms.
     */
     UFUNCTION(BlueprintAuthorityOnly,BlueprintCallable, Category = "InventorySubsytem|SaveGame")
     void LoadInventorySaveGame(FString SlotName, int32 UserIndex);

     /**
     * Save the player's inventory data
     * @param Player           The player's controller uses to find the Id and the inventory.
     * @param OverrideId       Overrides the PlayerController token Id.
     * @param SlotName			Name of save game slot to save to.
	 * @param UserIndex			The platform user index that identifies the user doing the saving, ignored on some platforms.
     */
     UFUNCTION(BlueprintCallable, Category = "InventorySubsytem|SaveGame")
     void WritePlayerInventorySaveGame(APlayerController* Player, ETokenPolicy TokenPolicy, FString OverrideId, FString SlotName, int32 UserIndex);

     /**
     * Load the player's inventory data
     * @param Player            The player's controller uses to find the Id and the inventory.
     * @param OverrideId        Overrides the PlayerController token Id.
     * @param SlotName			Name of save game slot to save to.
	 * @param UserIndex			The platform user index that identifies the user doing the saving, ignored on some platforms.
     */
     UFUNCTION(BlueprintCallable, Category = "InventorySubsytem|SaveGame")
     void LoadPlayerInventorySaveGame(APlayerController* Player, ETokenPolicy TokenPolicy, FString OverrideId, FString SlotName, int32 UserIndex,bool& Success);


     TArray<UActorComponent*> GetChestoryComponent(AActor* Actor);


     FInventorysSaveData SavedActorComponent(FName Key, UActorComponent* ActorComponent,FString DebugFunc = "", bool bBeforDestoryed = false);
     bool LoadActorComponent(FInventorysSaveData InventoryData,FName Key, UActorComponent* ActorComponent,FString DebugFunc = "");

     FInventorysSaveData SavedActor(FName Key, AActor* Actor, FString DebugFunc = "", bool bBeforDestoryed = false);
     bool LoadActor(FInventorysSaveData InventoryData, FName Key, AActor* Actor, FString DebugFunc = "");


     FInventorysSaveData SavedItemScript(FName Key,class UItemScript* ItemScript, FString DebugFunc = "", bool bBeforDestoryed = false);
     bool LoadItemScript(FInventorysSaveData InventoryData, FName Key, class UItemScript* ItemScript, FString DebugFunc = "");

     UFUNCTION(BlueprintCallable, BlueprintPure, Category = "InventorySubsytem|SaveGame")
     FString GetTokenID(APlayerController* Player, ETokenPolicy TokenPolicy);

protected:

    UPROPERTY()
    UChestorySaveGame* CurrentWorldSaveGame;

    UPROPERTY()
    UChestorySaveGame* CurrentPlayersSaveGame;



private:

    bool IsPlayer(AActor* Actor);


    void UpdatePlayersList();
    TArray<APawn*> PlayersPawn;
    TArray<APlayerController*> PlayersController;


    int32 EditorMaxPlayer = 0;

};

