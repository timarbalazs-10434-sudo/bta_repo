// Copyright 2024 Arkai Interactive. All Rights Reserved.


#include "InventorySubsystem.h"
#include "ItemDataAsset.h"
#include "UObject/Class.h"

//#include "JsonObjectConverter.h"
#include "ItemScript.h"
#include "Serialization/JsonSerializerMacros.h"
#include "Kismet/GameplayStatics.h"
#include "CraftingComponent.h"
#include "Engine/AssetManagerSettings.h"

//Save
#include "Online/CoreOnline.h"
#include "EngineUtils.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Engine/LocalPlayer.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

#include "Interface/ChestorySaveGameInterface.h"


const FPrimaryAssetType	UInventorySubsystem::ItemDataType = TEXT("ItemDataAsset");



void UInventorySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{

	Super::Initialize(Collection);
	//	AddRuleInAssetManager();
	
	InitializeStoreItems();



//	GetWorld()->OnLevelsChanged().AddUObject(this,&UInventorySubsystem::ResetDataLevelV2);
	//FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UInventorySubsystem::ResetDataLevel);
	FCoreUObjectDelegates::PreLoadMapWithContext.AddUObject(this, &UInventorySubsystem::ResetDataLevelV3);
//	GetDefault<UAssetManagerSettings>()->PrimaryAssetTypesToScan = DefautList;

	
}

void UInventorySubsystem::Deinitialize()
{
	ClearStoreItems();
}

UItemDataAsset* UInventorySubsystem::ForceLoadItem(const FPrimaryAssetId& PrimaryAssetId, bool bLogWarning)
{
	UAssetManager& AssetManager = UAssetManager::Get();
	// Optional "bundles" like "UI"
	TArray<FName> Bundles;
	FStreamableDelegate Delegate = FStreamableDelegate::CreateUObject(this, &UInventorySubsystem::EndLoadingItems);
	AssetManager.LoadPrimaryAsset(PrimaryAssetId, Bundles, Delegate);
	UItemDataAsset* Item = Cast<UItemDataAsset>(AssetManager.GetPrimaryAssetObject(PrimaryAssetId));

	if (bLogWarning && !IsValid(Item))
	{
		UE_LOG(LogInventory, Warning, TEXT("Failed to load item for identifier %s!"), *PrimaryAssetId.ToString());
	}

	return Item;
}

void UInventorySubsystem::AddRuleInAssetManager()
{

	//Look on ChestoryEditor
}

UItemDataAsset* UInventorySubsystem::GetItemDataAssetFromPrimaryAssetId(FPrimaryAssetId PrimaryAssetId)
{

	if (UAssetManager* AssetManager = UAssetManager::GetIfInitialized())
	{
		return Cast<UItemDataAsset>(AssetManager->GetPrimaryAssetObject(PrimaryAssetId));
	}
	else
	{
		return nullptr;
	}
}

FItemData UInventorySubsystem::GetItemDataFromPrimaryAssetId(FPrimaryAssetId PrimaryAssetId)
{
	if (GetItemDataAssetFromPrimaryAssetId(PrimaryAssetId))
	{
		return GetItemDataAssetFromPrimaryAssetId(PrimaryAssetId)->Item;
	}
	else
	{
		return FItemData();
	}
}

FPrimaryAssetId UInventorySubsystem::GetPrimaryAssetIdFromName(FName NameAsset)
{
	if (UAssetManager* AssetManager = UAssetManager::GetIfInitialized())
	{
		FPrimaryAssetId MyAsset(ItemDataType, NameAsset);
		return MyAsset;
	}
	return FPrimaryAssetId();
}

TArray<FPrimaryAssetId> UInventorySubsystem::FindItemsWithCraftingTag(FGameplayTagContainer CraftingTags)
{
	TArray<FPrimaryAssetId> ItemFound = TArray<FPrimaryAssetId>();

	if(!CraftingTags.IsValid()) 	return ItemFound;

	if(CraftingTags.IsEmpty()) 	return ItemFound;

	for (const auto *Item : StoreItems)
	{
		if (Item->IsCraftable && Item->EnableCanBeFound && Item->CraftingCategoryTag.IsValid())
		{
			if (CraftingTags.HasTag(Item->CraftingCategoryTag))
			{
				ItemFound.Add(FPrimaryAssetId("ItemDataAsset", Item->Item.ID));
			}
		}
	}

	return ItemFound;
}

TArray<FPrimaryAssetId> UInventorySubsystem::FindItemsWithItemsGameplayTags(FGameplayTagQuery TagQuery)
{
	TArray<FPrimaryAssetId> ItemFound = TArray<FPrimaryAssetId>();

	for (const auto* Item : StoreItems)
	{
		if (!IsValid(Item)) continue;

		if (Item->IsCraftable && Item->EnableCanBeFound)
		{
			if (TagQuery.Matches(Item->Item.GameplayTagContainer))
			{
				ItemFound.Add(FPrimaryAssetId("ItemDataAsset", Item->Item.ID));

			}
		}
	}

	return ItemFound;
}

bool UInventorySubsystem::CheckIfDataIsAlreadyLoaded()
{
	UAssetManager& AssetManager = UAssetManager::Get();
	TArray<FPrimaryAssetId> TmpListPrimaryAssetIdOut;
	AssetManager.GetPrimaryAssetIdList(ItemDataType, TmpListPrimaryAssetIdOut);

	if (TmpListPrimaryAssetIdOut.Num() > ListPrimaryAssetIdOut.Num())
	{
		return false;
	}
	else
	{
		for (auto AssedId : TmpListPrimaryAssetIdOut)
		{
			if (!ListPrimaryAssetIdOut.Contains(AssedId))
			{
				return false;
			}
		}
		// all asset found do not neccesary re load asset
		return true;
	}
}

bool UInventorySubsystem::CheskHasSetting()
{
	UAssetManagerSettings* Settings = GetMutableDefault<UAssetManagerSettings>();

	for (auto Scan : Settings->PrimaryAssetTypesToScan)
	{
		if (Scan.PrimaryAssetType == ItemDataType.GetName())
		{
			return true;
		}
	}

	return false;
}



TArray<FName> UInventorySubsystem::GetItemsIdWithGameplayTag(FGameplayTag Tag)
{
	if (ItemsIdCache.Contains(Tag))
	{
		return *ItemsIdCache.Find(Tag);
	}

	TArray<FName> ItemsFound;
	for (auto ItemRef : GetStoreItems())
	{
		if (ItemRef->Item.GameplayTagContainer.HasTag(Tag))
		{
			ItemsFound.Add(ItemRef->Item.ID);
		}
	}

	if (!ItemsFound.IsEmpty())
	{
		ItemsIdCache.Add(Tag, ItemsFound);
	}

	return ItemsFound;
}

void UInventorySubsystem::InitializeStoreItems()
{
	if (!CheckIfDataIsAlreadyLoaded())
	{
		UAssetManager& AssetManager = UAssetManager::Get();
		//TArray<FPrimaryAssetId> ListPrimaryAssetIdOut;
		ClearStoreItems();

		AssetManager.GetPrimaryAssetIdList(ItemDataType, ListPrimaryAssetIdOut);
		// Optional "bundles" like "UI"
		TArray<FName> Bundles;
		FStreamableDelegate Delegate = FStreamableDelegate::CreateUObject(this, &UInventorySubsystem::EndLoadingItems);
		UE_LOG(LogInventory, Display, TEXT("[InventorySubsystem] START LOADING ITEMS"));
		AssetManager.LoadPrimaryAssets(ListPrimaryAssetIdOut, Bundles, Delegate);
	}
	else
	{
		OnLoadingStoreItemsCompleted.Broadcast();
	}
}

void UInventorySubsystem::EndLoadingItems()
{

	UAssetManager& AssetManager = UAssetManager::Get();
	int32 TotalItems = 0;
	for (auto AssetID : ListPrimaryAssetIdOut)
	{
		//	UItemDataAsset* Item = ForceLoadItem(AssetID, true);
		UItemDataAsset* Item = Cast<UItemDataAsset>(AssetManager.GetPrimaryAssetObject(AssetID));

		if (IsValid(Item))
		{
			Item->Item.ID = AssetID.PrimaryAssetName;
			Item->Item.Decay = Item->Item.DecayTime;
			Item->Item.Durability = Item->Item.MaxDurability;

			// Initialize Script in ItemDataAsset
			for (UItemScript* script : Item->ListScriptsItem)
			{
				if (IsValid(script))
				{
					script->SetItem(Item->Item); //Init Default Item					
					script->ObjectForWorld = this;
					//script->GetWorld();
				}
			}


			// Add CurrencyType
			if (Item->Item.ItemType == EItemType::ECurrency)
			{
				FItemCurrency Currency;
				Currency.ID = Item->Item.ID;
				Currency.Amount = Item->Item.Amount;
				StoreCurrency.Add(Currency);
				//	UE_LOG(LogInventory, Verbose, TEXT("New Currency Add %s"), *Currency.ID.ToString());
			}



			AddItemInStore(Item);
			TotalItems++;
		}
	}
	UE_LOG(LogInventory, Display, TEXT("[InventorySubsystem] END LOADING ITEMS TOTAL [%d]"), TotalItems);
	bItemsCompletedLoad = true;
	OnLoadingStoreItemsCompleted.Broadcast();
}



void UInventorySubsystem::ResetDataLevel(const FString& MapName)
{

	UE_LOG(LogInventory, Warning, TEXT("[UInventorySubsystem::ResetDataLevel] -------------------------------- Call Reste DATA --------------------------------"));
	//PlayerGrids.Empty();
	//EquipementSlots.Empty();
	//HotbarSlots.Empty();
	//InventorysData.Empty();

}

void UInventorySubsystem::ResetDataLevelV2()
{
	UE_LOG(LogInventory, Warning, TEXT("[UInventorySubsystem::ResetDataLevelV2] Call Reste DATA V2"));

}

void UInventorySubsystem::ResetDataLevelV3(const FWorldContext& WorldContext, const FString& MapName)
{
	if (WorldContext.World() == GetWorld())
	{
		UE_LOG(LogInventory, Display, TEXT("[UInventorySubsystem::ResetDataLevelV3] Reset Data in Instance PIE:[%d] Prefix:[%s]"), WorldContext.PIEInstance, *WorldContext.PIEPrefix);
		
		
		ResetUIRegister();
		//InventorysData.Empty();
	}

}

void UInventorySubsystem::ResetUIRegister()
{
	PlayerGrids.Empty();
	EquipmentSlots.Empty();
	HotbarSlots.Empty();
}

void UInventorySubsystem::SetInventoryManagerInstance(UInventoryManagerComponent* InventoryManager)
{
	if (!IsValid(InventoryManager))
	{
		return;
	}

	if (InventoryManager->GetOwner()->HasAuthority())
	{
		if (!IsValid(InventoryManagerInstance))
		{
			InventoryManagerInstance = InventoryManager;
		}
	}
}

void UInventorySubsystem::ClearInventoryManagerInstance(UInventoryManagerComponent* InventoryManager)
{
	if (InventoryManager == InventoryManagerInstance)
	{
		InventoryManagerInstance = nullptr;
	}
}

void UInventorySubsystem::RegisterInventoryGrid(FName GridKey, UInventoryUniformGridPanel* Grid)
{
	if (GridKey.IsNone()) {
		UE_LOG(LogInventory, Warning, TEXT("[UInventorySubsystem::RegisterInventoryGrid] GridKey is None [%s]"), *GridKey.ToString());
		return; }

	if (!IsValid(Grid)) {
		UE_LOG(LogInventory, Warning, TEXT("[UInventorySubsystem::RegisterInventoryGrid] Grid is nullptr"));
		return; }


	UE_LOG(LogInventory, Display, TEXT("[UInventorySubsystem::RegisterInventoryGrid]  Add Grid [%s]"), *GridKey.ToString());

	PlayerGrids.Add(GridKey, Grid);

	OnPlayerGridAdded.Broadcast(GridKey, Grid);
}

void UInventorySubsystem::UnregisterInventoryGrid(FName GridKey)
{
	if (GridKey.IsNone()) { 
		UE_LOG(LogInventory, Warning, TEXT("[UInventorySubsystem::UnregisterInventoryGrid] GridKey is None [%s]"), *GridKey.ToString());
		return; }

	if (!PlayerGrids.Contains(GridKey)) { return; }

	PlayerGrids.Remove(GridKey);

	UE_LOG(LogInventory, Verbose, TEXT("[UInventorySubsystem::UnregisterInventoryGrid]  Remove player grid : [%s]"), *GridKey.ToString());

	OnPlayerGridRemoved.Broadcast(GridKey);
}

UInventoryUniformGridPanel* UInventorySubsystem::GetInventoryGrid(FName GridKey)
{
	if (GridKey.IsNone()) { 
		UE_LOG(LogInventory, Warning, TEXT("[UInventorySubsystem::GetInventoryGrid] GridKey is None [%s]"), *GridKey.ToString());
		return nullptr; }

	if (!PlayerGrids.Contains(GridKey)) { return nullptr; }

	if (!IsValid(*PlayerGrids.Find(GridKey)))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventorySubsystem::GetInventoryGrid] Remove Grid Invalid Ptr GridKey[%s]"), *GridKey.ToString());

		PlayerGrids.Remove(GridKey);
		return nullptr;
	}

	return *PlayerGrids.Find(GridKey);
}

void UInventorySubsystem::RegisterEquipmentSlot(FName Context, FName SlotKey, UInventorySlotUserWidget* Slot)
{
	if (Context.IsNone()) {
		UE_LOG(LogInventory, Warning, TEXT("[UInventorySubsystem::RegisterEquipmentSlot] Context is None [%s]"), *SlotKey.ToString());
		return;
	}

	if (SlotKey.IsNone()) {
		UE_LOG(LogInventory, Warning, TEXT("[UInventorySubsystem::RegisterEquipmentSlot] SlotKey is None [%s]"), *SlotKey.ToString());
		return;
	}

	if (!IsValid(Slot)) {
		UE_LOG(LogInventory, Warning, TEXT("[UInventorySubsystem::RegisterEquipmentSlot] Slot is nullptr"));
		return;
	}

	
	EquipmentSlots.Add(TPair<FName, FName>(Context, SlotKey), Slot);

	UE_LOG(LogInventory, Display, TEXT("[UInventorySubsystem::RegisterEquipmentSlot]  Slot Equipment  Added [%s]"), *SlotKey.ToString());
}

void UInventorySubsystem::UnregisterEquipmentSlot(FName Context, FName SlotKey)
{
	if (Context.IsNone()) {
		UE_LOG(LogInventory, Warning, TEXT("[UInventorySubsystem::UnregisterEquipmentSlot] Context is None [%s]"), *SlotKey.ToString());
		return;
	}

	if (SlotKey.IsNone()) {
		UE_LOG(LogInventory, Warning, TEXT("[UInventorySubsystem::UnregisterEquipmentSlot] SlotKey is None [%s]"), *SlotKey.ToString());
		return;
	}

	if (!ContainsEquipmentSlot(Context, SlotKey))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventorySubsystem::UnregisterEquipmentSlot] Slot Equipment  not found with key: [%s]"), *SlotKey.ToString());
		return;
	}

	EquipmentSlots.Remove(TPair<FName, FName>(Context, SlotKey));
	UE_LOG(LogInventory, Verbose, TEXT("[UInventorySubsystem::UnregisterEquipmentSlot]  Slot Equipment Remove [%s]"), *SlotKey.ToString());
}

UInventorySlotUserWidget* UInventorySubsystem::GetEquipmentSlot(FName Context, FName SlotKey)
{
	if (Context.IsNone()) {
		UE_LOG(LogInventory, Warning, TEXT("[UInventorySubsystem::GetEquipmentSlot] Context is None [%s]"), *SlotKey.ToString());
		return nullptr;
	}

	if (SlotKey.IsNone()) {
		UE_LOG(LogInventory, Warning, TEXT("[UInventorySubsystem::GetEquipmentSlot] SlotKey is None [%s]"), *SlotKey.ToString());
		return nullptr;
	}

	if (!ContainsEquipmentSlot(Context,SlotKey))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventorySubsystem::GetEquipmentSlot] Slot Equipement not found with key: [%s]"), *SlotKey.ToString());
		return nullptr;
	}

	//Check if Widegt Valid
	if (!IsValid(*EquipmentSlots.Find(TPair<FName, FName>(Context, SlotKey))))
	{
		EquipmentSlots.Remove(TPair<FName, FName>(Context, SlotKey));
		return nullptr;
	}

	return *EquipmentSlots.Find(TPair<FName, FName>(Context, SlotKey));
}

TArray<UInventorySlotUserWidget*> UInventorySubsystem::GetEquipmentSlots(FName Context)
{
	TArray<UInventorySlotUserWidget*> OutData;

	for (auto& Equipment : EquipmentSlots)
	{
		if (Equipment.Key.Key == Context)
		{
			OutData.Add(Equipment.Value);
		}
	}

	return OutData;
}

void UInventorySubsystem::RegisterHotbarSlot(FName SlotKey, UInventorySlotUserWidget* Slot)
{
	if (SlotKey.IsNone()) {
		UE_LOG(LogInventory, Warning, TEXT("[UInventorySubsystem::RegisterHotbarSlot] SlotKey is None [%s]"), *SlotKey.ToString());
		return;
	}

	if (!IsValid(Slot)) {
		UE_LOG(LogInventory, Warning, TEXT("[UInventorySubsystem::RegisterHotbarSlot] Slot is nullptr"));
		return;
	}

	HotbarSlots.Add(SlotKey, Slot);

	OnHotbarSlotAdded.Broadcast(SlotKey);

	UE_LOG(LogInventory, Display, TEXT("[UInventorySubsystem::RegisterHotbarSlot]  Slot Hotbar Added [%s]"), *SlotKey.ToString());
}

void UInventorySubsystem::UnregisterHotbarSlot(FName SlotKey)
{
	if (SlotKey.IsNone()) {
		UE_LOG(LogInventory, Warning, TEXT("[UInventorySubsystem::UnregisterHotbarSlot] SlotKey is None [%s]"), *SlotKey.ToString());
		return;
	}

	if (!ContainsHotbarSlot(SlotKey))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventorySubsystem::UnregisterHotbarSlot] Slot Equipement not found with key: [%s]"), *SlotKey.ToString());
		return;
	}

	HotbarSlots.Remove(SlotKey);
	UE_LOG(LogInventory, Verbose, TEXT("[UInventorySubsystem::UnregisterHotbarSlot]  Slot Hotbar Remove [%s]"), *SlotKey.ToString());
}

UInventorySlotUserWidget* UInventorySubsystem::GetHotbarSlot(FName SlotKey)
{
	if (SlotKey.IsNone()) {
		UE_LOG(LogInventory, Warning, TEXT("[UInventorySubsystem::GetHotbarSlot] SlotKey is None [%s]"), *SlotKey.ToString());
		return nullptr;
	}


	if (!ContainsHotbarSlot(SlotKey))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventorySubsystem::GetHotbarSlot] Slot Hotbar not found with key: [%s]"), *SlotKey.ToString());
		return nullptr;
	}

	//Check if Widegt Valid
	if (!IsValid(*HotbarSlots.Find(SlotKey)))
	{
		HotbarSlots.Remove(SlotKey);
		return nullptr;
	}

	return *HotbarSlots.Find(SlotKey);
}

TArray<UInventorySlotUserWidget*> UInventorySubsystem::GetHotbarSlots()
{
	TArray<UInventorySlotUserWidget*> OutHotbar;
	HotbarSlots.GenerateValueArray(OutHotbar);

	return OutHotbar;
}


void UInventorySubsystem::RegisterInventory(FGuid InventoryId, UInventoryComponent* Inventory)
{
	if (!InventoryId.IsValid())
	{
		UE_LOG(LogInventory, Warning, TEXT("UInventorySubsystem::RegisterInventory] InventoryId Invalid"));
		return;
	}

	if (!IsValid(Inventory))
	{
		UE_LOG(LogInventory, Warning, TEXT("UInventorySubsystem::RegisterInventory] InventoryComponent Invalid"));
		return;
	}

	FString Net = Inventory->GetOwner()->HasAuthority() ? FString("Server") : FString("Client");


	if (InventorysData.Contains(InventoryId))
	{
		if (IsValid(*InventorysData.Find(InventoryId)))
		{
			UE_LOG(LogInventory, VeryVerbose, TEXT("UInventorySubsystem::RegisterInventory] This inventory already exists Owner :[%s] InventoryId: [%s] NetRole :[%s] "), * FString(Inventory->GetOwner()->GetFName().ToString() + "." + Inventory->GetFName().ToString()), *InventoryId.ToString(), *Net);
			return;
		}
		//else update data with the new InventoryComp
	}
	
	UE_LOG(LogInventory, Verbose, TEXT("UInventorySubsystem::RegisterInventory] Add Owner:[%s] InventoryId:[%s] NetRole:[%s] "), *FString(Inventory->GetOwner()->GetFName().ToString() + "." + Inventory->GetFName().ToString()),*InventoryId.ToString(),*Net);
	InventorysData.Add(InventoryId, Inventory);
}

void UInventorySubsystem::UnregisterInventory(FGuid InventoryId)
{
	if (InventorysData.Contains(InventoryId))
	{
		UInventoryComponent* InventoryDebug = GetInventory(InventoryId);
		FString Net = InventoryDebug->GetOwner()->HasAuthority() ? FString("Server") : FString("Client");

		UE_LOG(LogInventory, Verbose, TEXT("UInventorySubsystem::UnregisterInventory] Remove Owner :[%s] InventoryId: [%s] NetRole :[%s]  "), *FString(InventoryDebug->GetOwner()->GetFName().ToString() + "." + InventoryDebug->GetFName().ToString()), *InventoryId.ToString(), *Net);
		InventorysData.Remove(InventoryId);
		return;
	}

	UE_LOG(LogInventory, VeryVerbose, TEXT("UInventorySubsystem::UnregisterInventory] This inventory cannot be found InventoryId: [%s]"),*InventoryId.ToString());
	return;
}

UInventoryComponent* UInventorySubsystem::GetInventory(FGuid InventoryId)
{
	if (InventorysData.Contains(InventoryId))
	{
		return *InventorysData.Find(InventoryId);
	}

	return nullptr;
}

void UInventorySubsystem::Debug_PrintFromInventory(UInventoryComponent* Inventory)
{
	if (!IsValid(Inventory))
	{
		return;
	}

	FString Net = Inventory->GetOwner()->HasAuthority() ? FString("Server") : FString("Client");


	UE_LOG(LogInventory, Warning, TEXT("UInventorySubsystem::Debug_PrintFromInventor] Id on Owner[%s] Inventory:[%s] NetMode:[%s]"), *FString(Inventory->GetOwner()->GetFName().ToString() + "." + Inventory->GetFName().ToString()),*Inventory->GetId().ToString(), *Net);


	for (auto& Inv : InventorysData)
	{
		if (Inv.Value == Inventory)
		{
			UE_LOG(LogInventory, Warning, TEXT("UInventorySubsystem::Debug_PrintFromInventor]  on Subsystem for this inventory  Id:[%s]"), *Inventory->GetId().ToString());

		}
	}

	if (UInventoryComponent* OnInventory = GetInventory(Inventory->GetId()))
	{
		UE_LOG(LogInventory, Warning, TEXT("UInventorySubsystem::Debug_PrintFromInventor]  on Subsystem for found Id:[%s]"), *OnInventory->GetId().ToString());

	}

}

void UInventorySubsystem::Debug_PrintAllInventorys()
{
	for (auto& Inv : InventorysData)
	{
		UInventoryComponent* Inventory = Inv.Value;

		FString Net = Inventory->GetOwner()->HasAuthority() ? FString("Server") : FString("Client");
		UE_LOG(LogInventory, Warning, TEXT("UInventorySubsystem::Debug_PrintAllInventorys] Id on Owner[%s] ONInventoryID:[%s] InventoryID:[%s] NetMode:[%s]"), *FString(Inventory->GetOwner()->GetFName().ToString() + "." + Inventory->GetFName().ToString()), *Inventory->GetId().ToString(), *Inv.Key.ToString(), *Net);

	}
}

void UInventorySubsystem::RegisterCrafting(FGuid CraftingId, UCraftingComponent* Crafting)
{
	if (!CraftingId.IsValid())
	{
		UE_LOG(LogInventory, Warning, TEXT("UInventorySubsystem::RegisterCrafting] CraftingId Invalid"));
		return;
	}

	if (!IsValid(Crafting))
	{
		UE_LOG(LogInventory, Warning, TEXT("UInventorySubsystem::RegisterCrafting] CraftingComponent Invalid"));
		return;
	}

	FString Net = Crafting->GetOwner()->HasAuthority() ? FString("Server") : FString("Client");


	if (CraftingsData.Contains(CraftingId))
	{
		if (IsValid(*CraftingsData.Find(CraftingId)))
		{
			UE_LOG(LogInventory, VeryVerbose, TEXT("UInventorySubsystem::RegisterCrafting] This inventory already exists Owner :[%s] CraftingId: [%s] NetRole :[%s] "), *FString(Crafting->GetOwner()->GetFName().ToString() + "." + Crafting->GetFName().ToString()), *CraftingId.ToString(), *Net);
			return;
		}
		//else update data with the new InventoryComp
	}

	UE_LOG(LogInventory, Verbose, TEXT("UInventorySubsystem::RegisterCrafting] Add Owner:[%s] CraftingId:[%s] NetRole:[%s] "), *FString(Crafting->GetOwner()->GetFName().ToString() + "." + Crafting->GetFName().ToString()), *CraftingId.ToString(), *Net);
	CraftingsData.Add(CraftingId, Crafting);
}

void UInventorySubsystem::UnregisterCrafting(FGuid CraftingId)
{
	if (CraftingsData.Contains(CraftingId))
	{
		UE_LOG(LogInventory, Verbose, TEXT("UInventorySubsystem::UnregisterCrafting] Remove CraftingId: [%s] "), *CraftingId.ToString());
		CraftingsData.Remove(CraftingId);
		return;
	}

	UE_LOG(LogInventory, VeryVerbose, TEXT("UInventorySubsystem::UnregisterCrafting] This inventory cannot be found CraftingId: [%s]"), *CraftingId.ToString());
	return;
}

UCraftingComponent* UInventorySubsystem::GetCrafting(FGuid CraftingId)
{
	if (CraftingsData.Contains(CraftingId))
	{
		return *CraftingsData.Find(CraftingId);
	}

	return nullptr;
}


//****************************** NEW SAVE SYSTEM ******************************//

void UInventorySubsystem::WriteInventorySaveGame(FString SlotName, int32 UserIndex)
{
	if (GetWorld()->GetNetMode() == ENetMode::NM_Client)
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventorySubsystem::WriteInventorySaveGame] Working only on the Server"));
		return;
	}

	if (UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex))
	{
		UE_LOG(LogInventory, Display, TEXT("[UInventorySubsystem::WriteInventorySaveGame]Try to load the existing SaveGame Object"));
		CurrentWorldSaveGame = Cast<UChestorySaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex));
	}
	else
	{
		UE_LOG(LogInventory, Display, TEXT("[UInventorySubsystem::WriteInventorySaveGame] Create a new SaveGame Object"));
		CurrentWorldSaveGame = Cast<UChestorySaveGame>(UGameplayStatics::CreateSaveGameObject(UChestorySaveGame::StaticClass()));
	}
	//Maybe create a new SaveGame object

	if (!IsValid(CurrentWorldSaveGame))
	{
		UE_LOG(LogInventory, Display, TEXT("[UInventorySubsystem::WriteInventorySaveGame] Failed to create a new SaveGame Object"));
		return;
	}

	if (!CurrentWorldSaveGame->SavedInventory.IsEmpty())
	{
		//Clear Data for apply a new version 
		CurrentWorldSaveGame->ClearData();
	}

	TArray<FInventorysSaveData> NewSavedInventory;

	for (FActorIterator It(GetWorld()); It; ++It)
	{
		AActor* Actor = *It;
		if (!IsValid(Actor))
		{
			continue;
		}

		TArray<UActorComponent*> Inventorys = GetChestoryComponent(Actor);
		//Actor->GetComponents(Inventorys);
		if (Inventorys.IsEmpty())
		{
			continue;
		}

		//Not Save data player )
		if (IsPlayer(Actor) || IsPlayer(Actor->GetOwner()))
		{
			continue;
		}

		for (auto& Inventory : Inventorys)
		{
			FString ConstName = Actor->GetFName().ToString() + "." + Inventory->GetFName().ToString();

			CurrentWorldSaveGame->SavedInventory.Add(SavedActorComponent(FName(ConstName),Inventory,"WriteInventorySaveGame"));
		}
	}

	UE_LOG(LogInventory, Display, TEXT("[UInventorySubsystem::WriteInventorySaveGame] SaveGame Inventory To Slot : [%s] Total inventory save : [%d]"), *SlotName, CurrentWorldSaveGame->SavedInventory.Num());
	UGameplayStatics::SaveGameToSlot(CurrentWorldSaveGame, SlotName, UserIndex);
}

void UInventorySubsystem::LoadInventorySaveGame(FString SlotName, int32 UserIndex)
{
	if (GetWorld()->GetNetMode() == ENetMode::NM_Client)
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventorySubsystem::LoadInventorySaveGame] Working only on the Server"));
		return;
	}

	if (UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex))
	{
		CurrentWorldSaveGame = Cast<UChestorySaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex));
		if (CurrentWorldSaveGame == nullptr)
		{
			UE_LOG(LogInventory, Warning, TEXT("[UInventorySubsystem::LoadInventorySaveGame] Failed to load SaveGame Data."));
			return;
		}

		UE_LOG(LogInventory, Warning, TEXT("[UInventorySubsystem::LoadInventorySaveGame] Success to load SaveGameSlot Object."));


		for (FActorIterator It(GetWorld()); It; ++It)
		{

			AActor* Actor = *It;
			if (!IsValid(Actor))
			{
				continue;
			}

			TArray<UActorComponent*> Inventorys = GetChestoryComponent(Actor);
			//Actor->GetComponents(Inventorys);
			if (Inventorys.IsEmpty())
			{
				continue;
			}

			//Ignore data player )
			if (IsPlayer(Actor) || IsPlayer(Actor->GetOwner()))
			{
				continue;
			}
			
			for (auto& InventoryData : CurrentWorldSaveGame->SavedInventory)
			{
				if (InventoryData.InventoryName.IsNone()) { continue; }

				for (auto& InInventory : Inventorys)
				{
					if (!IsValid(InInventory)) { continue; }

					FString ConstName = Actor->GetFName().ToString() + "." + InInventory->GetFName().ToString();

					if (LoadActorComponent(InventoryData, FName(ConstName), InInventory, "LoadInventorySaveGame"))
					{
						break;
					}
				}
			}
		}
		UE_LOG(LogInventory, Display, TEXT("[UInventorySubsystem::LoadInventorySaveGame] Load Inventorys Completed"));
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("[UInventorySubsystem::LoadInventorySaveGame] Create a new SaveGame Object"));
		CurrentWorldSaveGame = Cast<UChestorySaveGame>(UGameplayStatics::CreateSaveGameObject(UChestorySaveGame::StaticClass()));
	}
}

void UInventorySubsystem::WritePlayerInventorySaveGame(APlayerController* Player, ETokenPolicy TokenPolicy, FString OverrideId, FString SlotName, int32 UserIndex)
{
	
	//if (GetWorld()->GetNetMode() == ENetMode::NM_Client)
	//{
	//	UE_LOG(LogInventory, Warning, TEXT("[UInventorySubsystem::WritePlayerInventorySaveGame] Working only on the Server"));
	//	return;
	//}

	if(!IsValid(Player)) 
	{ 
		UE_LOG(LogInventory, Warning, TEXT("[UInventorySubsystem::WritePlayerInventorySaveGame] Player Invalid"));
		return;
	}

	if (UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex))
	{
		UE_LOG(LogInventory, Display, TEXT("[UInventorySubsystem::WritePlayerInventorySaveGame]Try to load the existing SaveGame Object"));
		CurrentPlayersSaveGame = Cast<UChestorySaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex));
	}

	if (!IsValid(CurrentPlayersSaveGame))
	{
		UE_LOG(LogInventory, Display, TEXT("[UInventorySubsystem::WritePlayerInventorySaveGame] Failed to create a new SaveGame Object"));
		return;
	}

	if (!CurrentPlayersSaveGame->SavedInventory.IsEmpty())
	{
		//Clear Data for apply a new version 
		//CurrentPlayersSaveGame->ClearData();
	}
	
	TArray<UActorComponent*> Inventorys = GetChestoryComponent(Player);
	//Player->GetComponents(Inventorys);
	if (Inventorys.IsEmpty())
	{
		if (!IsValid(Player->K2_GetPawn()))
		{
			UE_LOG(LogInventory, Warning, TEXT("[UInventorySubsystem::WritePlayerInventorySaveGame] no Pawn valid"));
			return;
		}

		//try found on Pawn
		Inventorys = GetChestoryComponent(Player->K2_GetPawn());
		Inventorys.Append(GetChestoryComponent(Player->PlayerState)); //Get Data in PlayerState
		
		//Add Child Player
		for (auto& Child : Player->K2_GetPawn()->Children)
		{
			Inventorys.Append(GetChestoryComponent(Child.Get()));
		}

		if (Inventorys.IsEmpty())
		{
			//Not inventory found for this player
			UE_LOG(LogInventory, Warning, TEXT("[UInventorySubsystem::WritePlayerInventorySaveGame] no Chestory Component found on the player"));
			return;
		}
	}

	for (auto& Inventory : Inventorys)
	{

		FString ClassName = Inventory->GetOwner()->GetClass()->GetFName().ToString();
		FString ConstName = (OverrideId.Len() == 0 ? GetTokenID(Player, TokenPolicy) : OverrideId) + "." + ClassName + "." + Inventory->GetFName().ToString();

		CurrentPlayersSaveGame->AddPlayer(SavedActorComponent(FName(ConstName),Inventory,"WritePlayerInventorySaveGame"));
	}


	UE_LOG(LogInventory, Display, TEXT("[UInventorySubsystem::WritePlayerInventorySaveGame] SaveGame Inventory To Slot : [%s] Total inventory save : [%d]"), *SlotName, CurrentPlayersSaveGame->SavedInventory.Num());
	UGameplayStatics::SaveGameToSlot(CurrentPlayersSaveGame, SlotName, UserIndex);
}

void UInventorySubsystem::LoadPlayerInventorySaveGame(APlayerController* Player, ETokenPolicy TokenPolicy, FString OverrideId, FString SlotName, int32 UserIndex, bool& Success)
{
	
	//if (GetWorld()->GetNetMode() == ENetMode::NM_Client)
	//{
	//	UE_LOG(LogInventory, Warning, TEXT("[UInventorySubsystem::LoadPlayerInventorySaveGame] Working only on the Server"));
	//	return;
	//}
	Success = false;
	if (!IsValid(Player))
	{ 
		UE_LOG(LogInventory, Warning, TEXT("[UInventorySubsystem::LoadPlayerInventorySaveGame] Player Invalid"))
		return; 
	}

	if (UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex))
	{
		CurrentPlayersSaveGame = Cast<UChestorySaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex));
		if (CurrentPlayersSaveGame == nullptr)
		{
			UE_LOG(LogInventory, Warning, TEXT("[UInventorySubsystem::LoadPlayerInventorySaveGame] Failed to load SaveGame Data."));
			return;
		}

		TArray<UActorComponent*> Inventorys = GetChestoryComponent(Player);

		if (Inventorys.IsEmpty())
		{
			if (!IsValid(Player->K2_GetPawn()))
			{
				UE_LOG(LogInventory, Warning, TEXT("[UInventorySubsystem::WritePlayerInventorySaveGame] Pawn Invalid"));
				return;
			}

			//try found on Pawn
			Inventorys = GetChestoryComponent(Player->K2_GetPawn());
			Inventorys.Append(GetChestoryComponent(Player->PlayerState)); //Get Data in PlayerState

			//Add Child Player
			for (auto& Child : Player->K2_GetPawn()->Children)
			{
				Inventorys.Append(GetChestoryComponent(Child.Get()));
			}
			//Player->K2_GetPawn()->GetComponents(Inventorys);
			{
				if (Inventorys.IsEmpty())
				{
					//Not inventory found for this player
					UE_LOG(LogInventory, Warning, TEXT("[UInventorySubsystem::WritePlayerInventorySaveGame] no Chestory Component found on the player"));
					return;
				}
			}
		}

		for (auto& InventoryData : CurrentPlayersSaveGame->SavedInventory)
		{
			if (InventoryData.InventoryName.IsNone()) { continue; }

			for (auto& InInventory : Inventorys)
			{
				if (!IsValid(InInventory)) { continue; }

				FString ClassName = InInventory->GetOwner()->GetClass()->GetFName().ToString();
				FString ConstName = (OverrideId.Len() == 0 ? GetTokenID(Player, TokenPolicy) : OverrideId) + "." + ClassName + "." + InInventory->GetFName().ToString();

				UE_LOG(LogInventory, VeryVerbose, TEXT("[UInventorySubsystem::LoadPlayerInventorySaveGame] Try ID : [%s] For : [%s]"), *ConstName, *InventoryData.InventoryName.ToString());

				if(LoadActorComponent(InventoryData,FName(ConstName),InInventory,"WritePlayerInventorySaveGame"))
				{
					Success = true;
					break;
				}
			}
		}

		if (!Success)
		{
			UE_LOG(LogInventory, Warning, TEXT("[UInventorySubsystem::WritePlayerInventorySaveGame] no DataSave found for player:[%s]"),*GetTokenID(Player, TokenPolicy));
		}
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("[UInventorySubsystem::LoadPlayerInventorySaveGame] Create a new SaveGame Object"));
		CurrentPlayersSaveGame = Cast<UChestorySaveGame>(UGameplayStatics::CreateSaveGameObject(UChestorySaveGame::StaticClass()));
	}
}


bool UInventorySubsystem::IsPlayer(AActor* Actor)
{


	//Check if is a Human controller
	if (APlayerController* Controller = Cast<APlayerController>(Actor))
	{
		UpdatePlayersList();
		UE_LOG(LogTemp, Verbose, TEXT("[UInventorySubsystem::IsPlayer] this actor is a controller : [%s]"), *Controller->GetFName().ToString());
		return PlayersController.Contains(Controller);
	}

	//Check if is a Human Pawn
	if (APawn* Pawn = Cast<APawn>(Actor))
	{
		UpdatePlayersList();
		UE_LOG(LogTemp, Verbose, TEXT("[UInventorySubsystem::IsPlayer] this actor is a Pawn Name : [%s]"),*Pawn->GetFName().ToString());
		return PlayersPawn.Contains(Pawn);
	}


	//is't a human pawn or controller
	return false;
}

void UInventorySubsystem::UpdatePlayersList()
{
	AGameStateBase* GS = GetWorld()->GetGameState();
	if (GS == nullptr)
	{
		// Warn about failure to save?
		UE_LOG(LogTemp, Warning, TEXT("[UInventorySubsystem::UpdatePlayersList] Failed to found GameState"));
		return;
	}

	EditorMaxPlayer = GS->PlayerArray.Num() < EditorMaxPlayer ? GS->PlayerArray.Num() : EditorMaxPlayer;

	//Check if need to update array else do noting
	if (PlayersPawn.Num() != GS->PlayerArray.Num() || PlayersController.Num() != GS->PlayerArray.Num())
	{
		//Clear Data
		PlayersPawn.Empty();
		PlayersController.Empty();

		for (auto& PS : GS->PlayerArray)
		{
			APlayerState* State = PS.Get();
			if (State != nullptr)
			{	
				if (IsValid(State->GetPlayerController()))
				{
					if (IsValid(State->GetPlayerController()->K2_GetPawn()))
					{
						PlayersPawn.AddUnique(State->GetPlayerController()->K2_GetPawn());
						UE_LOG(LogTemp, VeryVerbose, TEXT("[UInventorySubsystem::UpdatePlayersList] Add Player Pawn cache list: [%s]"),*PlayersPawn.Last()->GetFName().ToString());
					}

					PlayersController.AddUnique(State->GetPlayerController());
					UE_LOG(LogTemp, VeryVerbose, TEXT("[UInventorySubsystem::UpdatePlayersList] Add Player Controller cache list : [%s]"), *PlayersController.Last()->GetFName().ToString());
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("[UInventorySubsystem::UpdatePlayersList] Pawn or Controller Invalid"));

				}
				
			}
		}
		UE_LOG(LogTemp, Verbose, TEXT("[UInventorySubsystem::UpdatePlayersList] Fnish to update player list Controller : [%d] Pawn : [%d]"),PlayersController.Num(),PlayersPawn.Num());
		return;
	}

	return;;
}

FString UInventorySubsystem::GetTokenID(APlayerController* Player, ETokenPolicy TokenPolicy)
{
	if(!IsValid(Player)) { return FString("InvalidTokenPlayerWrong"); }




	ULocalPlayer* LocalPlayer = (Player != nullptr) ? Player->GetLocalPlayer() : nullptr;
	if (!IsValid(LocalPlayer))
	{
		return Player->PlayerState->GetPlayerName();
	}
	switch (TokenPolicy)
	{
	case ETokenPolicy::OnlineSubsystem:
	
		if (UKismetSystemLibrary::IsLoggedIn(Player))
		{
			return LocalPlayer->GetPreferredUniqueNetId().GetV1().Get()->ToString();
		}
		
		break;
	case ETokenPolicy::OnlineServices:
		
		if (UKismetSystemLibrary::IsLoggedIn(Player))
		{
			return UE::Online::ToLogString(LocalPlayer->GetPreferredUniqueNetId().GetV2());
			//Online Service ?? ? Who .?
			//return Player->PlayerState->GetPlayerName();

		}
		
		break;
	case ETokenPolicy::PlayerName:
		return Player->PlayerState->GetPlayerName();
		break;
	default:
		break;
	}

	return Player->PlayerState->GetPlayerName();


//#if WITH_EDITOR
//
//	if (GetWorld()->WorldType != EWorldType::Type::Game)
//	{
//		//return FString("Player" + FString::FromInt(GPlayInEditorID));
//
//		AGameStateBase* GS = GetWorld()->GetGameState();
//		if (GS != nullptr)
//		{
//			for (int32 i = 0; i < GS->PlayerArray.Num(); i++)
//			{
//				if (GS->PlayerArray[i].Get()->GetPlayerController() == Player)
//				{
//					FString Out = "Player" + FString::FromInt(i);
//					return Out;
//				}
//			}
//		}
//	}
//
//#endif


	////TODO need to check with OnlineServices
	//if (UKismetSystemLibrary::IsLoggedIn(Player))
	//{
	//	//Who to get LocalPlayer for Server ?????
	//	ULocalPlayer* LocalPlayer = (Player != nullptr) ? Player->GetLocalPlayer() : nullptr;
	//	if (IsValid(LocalPlayer))
	//	{
	//	
	//		return LocalPlayer->GetPreferredUniqueNetId().ToString();
	//	}
	//}

	//return Player->PlayerState->GetPlayerName();
}

TArray<UActorComponent*> UInventorySubsystem::GetChestoryComponent(AActor* Actor)
{
	TArray<UActorComponent*> OutData;
	
	OutData = Actor->GetComponentsByInterface(UChestorySaveGameInterface::StaticClass());
	return OutData;

}

FInventorysSaveData UInventorySubsystem::SavedActorComponent(FName Key, UActorComponent* ActorComponent, FString DebugFunc /*= ""*/, bool bBeforDestoryed /* = false*/)
{

	//Trigger Interface
	IChestorySaveGameInterface::Execute_OnObjectPreSave(ActorComponent);

	FInventorysSaveData InventoryData;

	InventoryData.InventoryName = Key;
	UE_LOG(LogInventory, Verbose, TEXT("[UInventorySubsystem::SavedActorComponent::%s] Save ActorComponent:[%s]"), *DebugFunc, *Key.ToString());

	FMemoryWriter MemWriter(InventoryData.ByteData);
	FObjectAndNameAsStringProxyArchive Ar(MemWriter, true);
	Ar.ArIsSaveGame = true;

	ActorComponent->Serialize(Ar);

	IChestorySaveGameInterface::Execute_OnObjectPostSave(ActorComponent);

	if (bBeforDestoryed)
	{
		IChestorySaveGameInterface::Execute_OnObjectPreDestoryed(ActorComponent);
	}


	return InventoryData;
}

bool UInventorySubsystem::LoadActorComponent(FInventorysSaveData InventoryData, FName Key, UActorComponent* ActorComponent, FString DebugFunc)
{
	if (!IsValid(ActorComponent))
	{
		return false;
	}

	if (InventoryData.InventoryName == FName(Key))
	{
		if (ActorComponent->Implements<UChestorySaveGameInterface>())
		{
			IChestorySaveGameInterface::Execute_OnObjectPreLoad(ActorComponent);
		}

		FMemoryReader MemReader(InventoryData.ByteData);
		FObjectAndNameAsStringProxyArchive Ar(MemReader, false);
		Ar.ArIsSaveGame = true;

		// Convert binary array back into actor's variables
		ActorComponent->Serialize(Ar);
		UE_LOG(LogInventory, Verbose, TEXT("[UInventorySubsystem::LoadActorComponent::%s] Load ActorComponent:[%s]"), *DebugFunc, *Key.ToString());

		//Trigger Interface
		if (ActorComponent->Implements<UChestorySaveGameInterface>())
		{
			IChestorySaveGameInterface::Execute_OnObjectPostLoad(ActorComponent);
		}


		return true;
	}

	return false;
}

FInventorysSaveData UInventorySubsystem::SavedActor(FName Key, AActor* Actor, FString DebugFunc, bool bBeforDestoryed /*= false*/)
{
	//Trigger Interface
	if (Actor->Implements<UChestorySaveGameInterface>())
	{			
		IChestorySaveGameInterface::Execute_OnObjectPreSave(Actor);
	}
			 
	FInventorysSaveData InventoryData;

	InventoryData.InventoryName = Key;
	UE_LOG(LogInventory, Verbose, TEXT("[UInventorySubsystem::SavedActorComponent::%s] Save Actor:[%s]"), *DebugFunc, *Key.ToString());

	FMemoryWriter MemWriter(InventoryData.ByteData);
	FObjectAndNameAsStringProxyArchive Ar(MemWriter, true);
	Ar.ArIsSaveGame = true;

	Actor->Serialize(Ar);

	//Trigger Interface
	if (Actor->Implements<UChestorySaveGameInterface>())
	{
		IChestorySaveGameInterface::Execute_OnObjectPostSave(Actor);

		if (bBeforDestoryed)
		{
			IChestorySaveGameInterface::Execute_OnObjectPreDestoryed(Actor);
		}
	}

	return InventoryData;
}

bool UInventorySubsystem::LoadActor(FInventorysSaveData InventoryData, FName Key, AActor* Actor, FString DebugFunc)
{
	if (!IsValid(Actor))
	{
		return false;
	}

	if (InventoryData.InventoryName == FName(Key))
	{
		//Trigger Interface
		if (Actor->Implements<UChestorySaveGameInterface>())
		{
			IChestorySaveGameInterface::Execute_OnObjectPreLoad(Actor);
		}

		FMemoryReader MemReader(InventoryData.ByteData);
		FObjectAndNameAsStringProxyArchive Ar(MemReader, false);
		Ar.ArIsSaveGame = true;

		// Convert binary array back into actor's variables
		Actor->Serialize(Ar);
		UE_LOG(LogInventory, Verbose, TEXT("[UInventorySubsystem::LoadActorComponent::%s] Load Actor: [%s]"), *DebugFunc, *Key.ToString());

		//Trigger Interface
		if (Actor->Implements<UChestorySaveGameInterface>())
		{
			IChestorySaveGameInterface::Execute_OnObjectPostLoad(Actor);
		}

		return true;
	}

	return false;
}

FInventorysSaveData UInventorySubsystem::SavedItemScript(FName Key, UItemScript* ItemScript, FString DebugFunc, bool bBeforDestoryed)
{

	FInventorysSaveData InventoryData;

	InventoryData.InventoryName = Key;
	UE_LOG(LogInventory, Verbose, TEXT("[UInventorySubsystem::SavedItemScript::%s] Save ItemScript:[%s]"), *DebugFunc, *Key.ToString());

	FMemoryWriter MemWriter(InventoryData.ByteData);
	FObjectAndNameAsStringProxyArchive Ar(MemWriter, true);
	Ar.ArIsSaveGame = true;

	ItemScript->Serialize(Ar);

	//Trigger Interface
	if (ItemScript->Implements<UChestorySaveGameInterface>())
	{
		IChestorySaveGameInterface::Execute_OnObjectPostSave(ItemScript);

		if (bBeforDestoryed)
		{
			IChestorySaveGameInterface::Execute_OnObjectPreDestoryed(ItemScript);
		}
	}

	return InventoryData;
}

bool UInventorySubsystem::LoadItemScript(FInventorysSaveData InventoryData, FName Key, UItemScript* ItemScript, FString DebugFunc)
{
	if (!IsValid(ItemScript))
	{
		UE_LOG(LogInventory,Warning,TEXT("[UInventorySubsystem::LoadItemScript::%s] ItemScript Invalid"),*DebugFunc)
		return false;
	}

	if (InventoryData.InventoryName == FName(Key))
	{
		//Trigger Interface
		if (ItemScript->Implements<UChestorySaveGameInterface>())
		{
			IChestorySaveGameInterface::Execute_OnObjectPreLoad(ItemScript);
		}

		FMemoryReader MemReader(InventoryData.ByteData);
		FObjectAndNameAsStringProxyArchive Ar(MemReader, false);
		Ar.ArIsSaveGame = true;

		// Convert binary array back into actor's variables
		ItemScript->Serialize(Ar);
		UE_LOG(LogInventory, Verbose, TEXT("[UInventorySubsystem::LoadItemScript::%s] Load ItemScript: [%s]"), *DebugFunc, *Key.ToString());

		//Trigger Interface
		if (ItemScript->Implements<UChestorySaveGameInterface>())
		{
			IChestorySaveGameInterface::Execute_OnObjectPostLoad(ItemScript);
		}

		return true;
	}

	return false;
}
