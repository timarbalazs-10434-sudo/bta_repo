// Copyright 2024 Arkai Interactive. All Rights Reserved.


#include "ChestoryFunctionLibrary.h"

#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "InventoryManagerComponent.h"
#include "ItemDataAsset.h"
#include "Chestory.h"

UInventorySubsystem* UChestoryFunctionLibrary::GetInventorySubsystem(const UObject* WorldContextObject)
{
	if (!WorldContextObject) return nullptr;


	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
	if (IsValid(GameInstance))
	{
		UInventorySubsystem* InventorySubsystem = GameInstance->GetSubsystem<UInventorySubsystem>();

		if (IsValid(InventorySubsystem))
		{
			return InventorySubsystem;
		}
	}
	return nullptr;
}

FPrimaryAssetId UChestoryFunctionLibrary::GetPrimaryAssetIdFromItemData(const UObject* WorldContextObject, FItemData Item)
{
	if (Item.IsEmpty()) { return FPrimaryAssetId(); }

	if (!WorldContextObject) { FPrimaryAssetId(); }

	return IdToPrimaryAssetId(WorldContextObject, Item.ID);
}

FPrimaryAssetId UChestoryFunctionLibrary::GetPrimaryAssetIdFromInfo(const UObject* WorldContextObject, FItemDataInfo Item)
{
	if (Item.IsEmpty()) { return FPrimaryAssetId(); }

	if (!WorldContextObject) { FPrimaryAssetId(); }

	return IdToPrimaryAssetId(WorldContextObject, Item.ID);
}

FItemData UChestoryFunctionLibrary::GetItemDataFromPrimaryAssetId(const UObject* WorldContextObject, FPrimaryAssetId PrimaryAssetId)
{

	if (!PrimaryAssetId.IsValid())
	{
		return FItemData();
	}
	if (!WorldContextObject) { return FItemData(); }
	if (IsValid(GetInventorySubsystem(WorldContextObject)))
	{
		return	GetInventorySubsystem(WorldContextObject)->GetItemDataFromPrimaryAssetId(PrimaryAssetId);
	}
	
	return FItemData();
	
}

FItemData UChestoryFunctionLibrary::GetItemDataFromID(const UObject* WorldContextObject, FName ID)
{
	if(ID == "None") {return  FItemData();}

	if (!WorldContextObject) { return FItemData();}

	if (IsValid(GetInventorySubsystem(WorldContextObject)))
	{
		return	GetInventorySubsystem(WorldContextObject)->GetItemDataFromPrimaryAssetId(GetInventorySubsystem(WorldContextObject)->GetPrimaryAssetIdFromName(ID));
	}

	return FItemData();
}

FItemData UChestoryFunctionLibrary::MakeItemAmount(const UObject* WorldContextObject, FPrimaryAssetId PrimaryAssetId, int32 Amount)
{
	FItemData Item = GetItemDataFromPrimaryAssetId(WorldContextObject, PrimaryAssetId);

	if (Item.ItemType == EItemType::ECurrency)
	{
		Item.Amount = Amount;
	}
	else
	{
		Item.Amount = Item.bIsStackable ? FMath::Clamp(Amount, 0, Item.MaxStackable) : 1;
	}
	
	

	return Item;
}

FItemData UChestoryFunctionLibrary::MakeItemDecay(const UObject* WorldContextObject, FPrimaryAssetId PrimaryAssetId, float Decay)
{
	FItemData Item = GetItemDataFromPrimaryAssetId(WorldContextObject, PrimaryAssetId);
	
	if (Decay == -1.0f)
	{
		Item.Decay = Item.DecayTime;
	}
	else
	{
		Item.Decay = Item.bCanDecay ? FMath::Clamp(Decay, 1.0f, Item.DecayTime) : 0.0f;
	}
	

	return Item;
}

FItemData UChestoryFunctionLibrary::MakeItemDurability(const UObject* WorldContextObject, FPrimaryAssetId PrimaryAssetId, float Durability)
{
	FItemData Item = GetItemDataFromPrimaryAssetId(WorldContextObject, PrimaryAssetId);

	if (Durability == -1.0f)
	{
		Item.Durability = Item.MaxDurability;
	}
	else
	{
		Item.Durability = (Item.MaxDurability > 1) ? FMath::Clamp(Durability, 0.0f, Item.MaxDurability) : 0.0f;
	}

	

	return Item;
}

FItemData UChestoryFunctionLibrary::MakeItemAttributes(const UObject* WorldContextObject, FPrimaryAssetId PrimaryAssetId, FGameplayTagAttributeContainer Attributes)
{
	FItemData Item = GetItemDataFromPrimaryAssetId(WorldContextObject, PrimaryAssetId);

	if (!Item.bCanAttributes)
	{
		return Item;
	}

	//UE_LOG(LogInventory, Warning, TEXT("[UChestoryFunctionLibrary::MakeItemAttributes] Add Attribute on Item[%s]"), *Item.ID.ToString());

	//Override Default Attribute
	FGameplayTagAttributeContainer UseAttributes = (Attributes.GetNum() > 0) ? Attributes : Item.Attributes;

	if (UseAttributes.GetNum() > 0)
	{

		Item.Attributes.ClearAttributes(); // Clear default attribute for Load from Save
		//Load Attribute
		for (auto Att : UseAttributes.GetAttributes())
		{
			if (Att.AttributeTag.IsValid() && Att.Value > 0)
			{
				//UE_LOG(LogInventory, Warning, TEXT("[UChestoryFunctionLibrary::MakeItemAttributes] Add Attribute on Item[%s] Att:[%s] Value:[%f]"), *Item.ID.ToString(), *Att.AttributeTag.ToString(), Att.Value);
				Item.Attributes.AddAttribute(Att.AttributeTag, Att.Value);
			}
		}
	}

	return Item;
}

FItemData UChestoryFunctionLibrary::MakeItemSockets(const UObject* WorldContextObject, FPrimaryAssetId PrimaryAssetId, TArray<FSocketItem> Sockets)
{
	FItemData Item = GetItemDataFromPrimaryAssetId(WorldContextObject, PrimaryAssetId);


	if (Item.bCanSockets)
	{
		if (!Sockets.IsEmpty())
		{
			Item.Sockets.Empty();
			for (auto socket : Sockets)
			{

				if (socket.PrimaryAssetId.IsValid())
				{
					FGameplayTagAttributeContainer Attributes;
					for (auto att : socket.Attributes.GetAttributes())
					{
						Attributes.AddAttribute(att.AttributeTag, att.Value);
					}
					socket.Attributes = Attributes;
					Item.Sockets.Add(socket);
				}
				else
				{
					Item.Sockets.Add(socket);
				}

				//Clamp Max Sockets
				if (Item.Sockets.Num() == Item.MaxSocket)
				{
					break;
				}
			}
		}
		else //MaxSockets default
		{
			TArray<FSocketItem> TmpSockets = Item.Sockets;
			Item.Sockets.Empty();

			for (auto socket : TmpSockets)
			{
				Item.Sockets.Add(socket);

				//Clamp Max Sockets
				if (Item.Sockets.Num() == Item.MaxSocket)
				{
					break;
				}
			}
		}
		
	}

	return Item;
}

FItemData UChestoryFunctionLibrary::MakeItem(const UObject* WorldContextObject, FPrimaryAssetId PrimaryAssetId, int32 Amount, float Decay, float Durability, FGameplayTagAttributeContainer Attributes, TArray<FSocketItem> Sockets, FString JsonParameter)
{

	FItemData Item = GetItemDataFromPrimaryAssetId(WorldContextObject, PrimaryAssetId);

	Item.Amount = MakeItemAmount(WorldContextObject, PrimaryAssetId, Amount).Amount;
	Item.Decay = MakeItemDecay(WorldContextObject, PrimaryAssetId, Decay).Decay;
	Item.Durability = MakeItemDurability(WorldContextObject, PrimaryAssetId, Durability).Durability;
	Item.Attributes = MakeItemAttributes(WorldContextObject, PrimaryAssetId, Attributes).Attributes;
	Item.Sockets = MakeItemSockets(WorldContextObject, PrimaryAssetId, Sockets).Sockets;
	Item.JsonParameter = JsonParameter.IsEmpty() ? Item.JsonParameter : JsonParameter; //Take default value if empty JsonParam

	return Item;
}

FItemData UChestoryFunctionLibrary::MakeItemByID(const UObject* WorldContextObject, FName ID, int32 Amount, float Decay, float Durability, FGameplayTagAttributeContainer Attributes, TArray<FSocketItem> Sockets, FString JsonParameter)
{
	if (ID == "None") { return  FItemData(); }

	if (!WorldContextObject) { return FItemData(); }

	if (IsValid(GetInventorySubsystem(WorldContextObject)))
	{
		FPrimaryAssetId Asset =	GetInventorySubsystem(WorldContextObject)->GetPrimaryAssetIdFromName(ID);

		return MakeItem(WorldContextObject, Asset, Amount, Decay, Durability, Attributes, Sockets, JsonParameter);
	}

	return FItemData();
}

FPrimaryAssetId UChestoryFunctionLibrary::IdToPrimaryAssetId(const UObject* WorldContextObject, FName ID)
{
	if (ID == "None") { return  FPrimaryAssetId(); }

	if (!WorldContextObject) { FPrimaryAssetId(); }

	if (IsValid(GetInventorySubsystem(WorldContextObject)))
	{
		FPrimaryAssetId Asset = GetInventorySubsystem(WorldContextObject)->GetPrimaryAssetIdFromName(ID);
		return Asset;
	}
	
	return FPrimaryAssetId();
}


FItemData UChestoryFunctionLibrary::GetItemFromInventoryId(const UObject* WorldContextObject, FGuid InventoryId, int32 Slot)
{
	if (GetInventorySubsystem(WorldContextObject)->ContainsInventory(InventoryId))
	{
		return GetInventorySubsystem(WorldContextObject)->GetInventory(InventoryId)->GetInventoryItem(Slot);
	}

	return FItemData();
}

ESlotType UChestoryFunctionLibrary::GetTypeFromInventoryId(const UObject* WorldContextObject, FGuid InventoryId)
{
	if (GetInventorySubsystem(WorldContextObject)->ContainsInventory(InventoryId))
	{
		return GetInventorySubsystem(WorldContextObject)->GetInventory(InventoryId)->GetInventoryType();
	}
	return ESlotType();
}

FItemDataInfo UChestoryFunctionLibrary::ItemDataToItemInformation(FItemData ItemData)
{

	FItemDataInfo ReturnItemInfo;

	ReturnItemInfo.ID = ItemData.ID;
	ReturnItemInfo.Icon = ItemData.Icon;
	ReturnItemInfo.Name = ItemData.Name;
	ReturnItemInfo.ItemType = ItemData.ItemType;
	ReturnItemInfo.GameplayTagContainer = ItemData.GameplayTagContainer;
	ReturnItemInfo.Amount = ItemData.Amount;
	ReturnItemInfo.JsonParameter = ItemData.JsonParameter;
	ReturnItemInfo.bIsDraggable = ItemData.bIsDraggable;
	ReturnItemInfo.bIsDroppable = ItemData.bIsDroppable;
	ReturnItemInfo.bIsStackable = ItemData.bIsStackable;
	ReturnItemInfo.bIsVisible = ItemData.bIsVisible;
	ReturnItemInfo.bCanDecay = ItemData.bCanDecay;
	ReturnItemInfo.Decay = ItemData.Decay;
	ReturnItemInfo.DecayTime = ItemData.DecayTime;
	ReturnItemInfo.Durability = ItemData.Durability;
	ReturnItemInfo.MaxDurability = ItemData.MaxDurability;
	ReturnItemInfo.bCanRepairable = ItemData.bCanRepairable;
	ReturnItemInfo.Attributes = ItemData.Attributes;
	ReturnItemInfo.bCanSockets = ItemData.bCanSockets;
	ReturnItemInfo.Sockets = ItemData.Sockets;

	return ReturnItemInfo;
}

UPARAM(DisplayName = "ItemInfo")FItemData UChestoryFunctionLibrary::SocketToItemData(const UObject* WorldContextObject, FSocketItem Socket)
{
	
	FItemData R_Info = MakeItemDataFromItemTiny(WorldContextObject,Socket.SocketTiny);
	R_Info.Attributes = Socket.Attributes;
	R_Info.Amount = 1;

	return R_Info;
}

UPARAM(DisplayName = "ItemInfo")FItemDataInfo UChestoryFunctionLibrary::SocketToItemInfo(const UObject* WorldContextObject, FSocketItem Socket)
{
	FItemDataInfoCompressed InfoComprssed;
	InfoComprssed.ItemTiny = Socket.SocketTiny;

	FItemDataInfo R_Info = MakeItemDataFromTransfer(WorldContextObject, InfoComprssed);
	R_Info.Attributes = Socket.Attributes; //TODO Delet Attribute
	R_Info.Amount = 1;

	return R_Info;
}

FSocketItem UChestoryFunctionLibrary::ItemDataToSocket(const UObject* WorldContextObject, FItemData ItemData)
{
	FSocketItem ConstructSocket;
	ItemData.Amount = 1;
	ConstructSocket.PrimaryAssetId = GetPrimaryAssetIdFromItemData(WorldContextObject,ItemData);
	ConstructSocket.SocketTiny = MakeItemTiny(ItemData);

	//TODO Delet Attribute
	for (auto att : ItemData.Attributes.GetAttributes())
	{
		ConstructSocket.Attributes.AddAttribute(att.AttributeTag, att.Value);
	}

	return ConstructSocket;
}

TArray<FItem> UChestoryFunctionLibrary::MakeItemSaveFromInventory(const UObject* WorldContextObject, UInventoryComponent* Inventory, bool InEquipement, bool InHotbar)
{

	if (IsValid(Inventory))
	{
		TArray<FItem> OutResult;
		for (int32 i = 0; i < Inventory->Inventory.Num(); i++)
		{
			FItemData ItemData = Inventory->GetInventoryItem(i);
			ItemData = ItemData.NotifyScript_ItemPreSave(Inventory->GetInventoryManager(), Inventory->GetId(), i);
			FItem Item;
			Item.PrimaryAssetId = GetPrimaryAssetIdFromItemData(WorldContextObject,ItemData);
			Item.Amount = ItemData.Amount;
			Item.JsonParameter = ItemData.JsonParameter;
			Item.Decay = ItemData.Decay;
			Item.Durability = ItemData.Durability;
			Item.bInEquipement = InEquipement;
			Item.bInHotbar = InHotbar;
			Item.IndexSlot = i;
			Item.Attributes = ItemData.Attributes;
			Item.Sockets = ItemData.Sockets;
			OutResult.Add(Item);
		}

		return OutResult;
	}

	return TArray<FItem>();
}

TArray<UItemScript*> UChestoryFunctionLibrary::GetItemScriptsFromPrimaryAssetId(const UObject* WorldContextObject, FPrimaryAssetId PrimaryAssetId)
{
	if (PrimaryAssetId.IsValid())
	{
		if (IsValid(GetInventorySubsystem(WorldContextObject)))
		{
			if (GetInventorySubsystem(WorldContextObject)->GetItemDataAssetFromPrimaryAssetId(PrimaryAssetId)->ListScriptsItem.IsValidIndex(0))
			{
				return GetInventorySubsystem(WorldContextObject)->GetItemDataAssetFromPrimaryAssetId(PrimaryAssetId)->ListScriptsItem;
			}
		}		
	}
	return TArray<UItemScript*>();
}

UItemScript* UChestoryFunctionLibrary::GetItemScriptOfClassFromPrimaryAssetId(const UObject* WorldContextObject, FPrimaryAssetId PrimaryAssetId, TSubclassOf<UItemScript> ClassItemScript)
{
	if (PrimaryAssetId.IsValid())
	{
		
		//UE_LOG(LogInventory, Warning, TEXT("Get script in Asset : %s"), *PrimaryAssetId.PrimaryAssetName.ToString());
		TArray<UItemScript*> ListScript = GetItemScriptsFromPrimaryAssetId(WorldContextObject,PrimaryAssetId);
		if (ListScript.IsValidIndex(0))
		{
			for (int32 i = 0; i < ListScript.Num(); i++)
			{
				if (IsValid(ListScript[i]))
				{
					if (ListScript[i]->GetClass() == ClassItemScript)
					{
						return ListScript[i];
					}
				}

			}
			return nullptr;
		}
		return nullptr;
	}
	return nullptr;
}

UPARAM(DisplayName = "ItemScript")UItemScript* UChestoryFunctionLibrary::GetItemScriptOfClassFromItem(const UObject* WorldContextObject, FItemData Item, TSubclassOf<UItemScript> ClassItemScript)
{
		
	return GetItemScriptOfClassFromPrimaryAssetId(WorldContextObject, UChestoryFunctionLibrary::GetPrimaryAssetIdFromItemData(WorldContextObject, Item),ClassItemScript);
}

UPARAM(DisplayName = "ItemScript")UItemScript* UChestoryFunctionLibrary::GetItemScriptInstanceOfClassFromItem(FItemData Item, TSubclassOf<UItemScript> ClassItemScript)
{
	if (Item.IsValid()) return Item.GetItemScriptInstanceFromClass(ClassItemScript);
	return nullptr;
}


bool UChestoryFunctionLibrary::ContaintAttribute(FGameplayTagAttributeContainer Attributes, FGameplayTag AttributeTag)
{
	for (const auto att : Attributes.Attributes)
	{
		if (att.AttributeTag == AttributeTag)
		{
			return true;
		}
	}
	return false;
}

int32 UChestoryFunctionLibrary::GetNumberAttributes(FGameplayTagAttributeContainer Attributes)
{
	return Attributes.Attributes.Num();
}

float UChestoryFunctionLibrary::GetAttributeValue(FGameplayTagAttributeContainer Attributes, FGameplayTag AttributeTag)
{
	if (ContaintAttribute(Attributes, AttributeTag))
	{
		for (const auto att : Attributes.Attributes)
		{
			if (att.AttributeTag == AttributeTag)
			{
				return att.Value;
			}
		}
	}
	return 0.0f;
}

FGameplayTagAttributeContainer UChestoryFunctionLibrary::GetAttributesWithExclude(FGameplayTagAttributeContainer Attributes, FGameplayTagContainer ExcludeTag)
{
	FGameplayTagAttributeContainer Out_Attributes;

	for (auto Attribute : Attributes.GetAttributes())
	{
		if (!ExcludeTag.HasTag(Attribute.AttributeTag))
		{
			Out_Attributes.AddAttribute(Attribute.AttributeTag, Attribute.Value);
		}
	}

	return Out_Attributes;
}

FGameplayTagAttributeContainer UChestoryFunctionLibrary::GetAttributesWithInclude(FGameplayTagAttributeContainer Attributes, FGameplayTagContainer IncludeTag)
{

	FGameplayTagAttributeContainer Out_Attributes;

	for (auto Attribute : Attributes.GetAttributes())
	{
		if (IncludeTag.HasTag(Attribute.AttributeTag))
		{
			Out_Attributes.AddAttribute(Attribute.AttributeTag, Attribute.Value);
		}
	}

	return Out_Attributes;
}

FGameplayTagAttribute UChestoryFunctionLibrary::FindAttribute(FGameplayTagAttributeContainer Attributes, FGameplayTag Tag, bool& Success)
{
	
	if (ContaintAttribute(Attributes,Tag))
	{
		Success = true;
		return FGameplayTagAttribute(Tag, GetAttributeValue(Attributes,Tag));
	}
	

	Success = false;
	return FGameplayTagAttribute();
}



void UChestoryFunctionLibrary::Add_Attribute(UPARAM(ref)FGameplayTagAttributeContainer& InAttributes,FGameplayTag Tag, float Value)
{
	InAttributes.AddAttribute(Tag, Value);
}

void UChestoryFunctionLibrary::Remove_Attribute(UPARAM(ref)FGameplayTagAttributeContainer& InAttributes, FGameplayTag Tag, float Value)
{
	InAttributes.RemoveAttribute(Tag, Value);

}

void UChestoryFunctionLibrary::Set_Attribute(UPARAM(ref)FGameplayTagAttributeContainer& InAttributes, FGameplayTag Tag, float Value)
{
	InAttributes.SetAttribute(Tag, Value);

}

bool UChestoryFunctionLibrary::Has_Attribute(FGameplayTagAttributeContainer InAttributes, FGameplayTag Tag)
{
	return InAttributes.ContainsAttributeTag(Tag);
}

FItemTiny UChestoryFunctionLibrary::MakeItemTiny(FItemData ItemData)
{
	if (ItemData.IsEmpty())
	{
		if (!ItemData.bIsFilled)
		{
			return FItemTiny();
		}
	}

	FItemTiny ReturnData;

	ReturnData.ID = ItemData.ID;
	ReturnData.GameplayTagContainer = ItemData.GameplayTagContainer;
	ReturnData.Amount = ItemData.Amount;
	ReturnData.JsonParameter = ItemData.JsonParameter;
	ReturnData.bIsDraggable = ItemData.bIsDraggable;
	ReturnData.bIsDroppable = ItemData.bIsDroppable;
	ReturnData.bIsStackable = ItemData.bIsStackable;
	ReturnData.bIsVisible = ItemData.bIsVisible;
	ReturnData.Decay = ItemData.Decay;
	ReturnData.Durability = ItemData.Durability;
	ReturnData.Attributes = ItemData.Attributes;
	ReturnData.bIsFilled = ItemData.bIsFilled;

	return ReturnData;
}

FItemTiny UChestoryFunctionLibrary::MakeItemTiny(FItemDataInfo ItemInfo)
{
	if (ItemInfo.IsEmpty())
	{
		return FItemTiny();
	}

	FItemTiny ReturnData;

	ReturnData.ID = ItemInfo.ID;
	ReturnData.GameplayTagContainer = ItemInfo.GameplayTagContainer;
	ReturnData.Amount = ItemInfo.Amount;
	ReturnData.JsonParameter = ItemInfo.JsonParameter;
	ReturnData.bIsDraggable = ItemInfo.bIsDraggable;
	ReturnData.bIsDroppable = ItemInfo.bIsDroppable;
	ReturnData.bIsStackable = ItemInfo.bIsStackable;
	ReturnData.bIsVisible = ItemInfo.bIsVisible;
	ReturnData.Attributes = ItemInfo.Attributes;
	ReturnData.bIsFilled = ItemInfo.bIsFilled;

	return ReturnData;
}

FItemDataInfoCompressed UChestoryFunctionLibrary::MakeItemTransfer(FItemData ItemData)
{
	if (ItemData.IsEmpty())
	{
		if (!ItemData.bIsFilled)
		{
			return FItemDataInfoCompressed();
		}
	}

	FItemDataInfoCompressed ReturnData;

	ReturnData.ItemTiny = MakeItemTiny(ItemData);
	ReturnData.Sockets = ItemData.Sockets;

	return ReturnData;
}

FItemDataInfoCompressed UChestoryFunctionLibrary::MakeItemTransfer(FItemDataInfo ItemInfo)
{
	if (ItemInfo.IsEmpty())
	{
		return FItemDataInfoCompressed();
	}

	FItemDataInfoCompressed ReturnData;

	ReturnData.ItemTiny = MakeItemTiny(ItemInfo);
	ReturnData.Sockets = ItemInfo.Sockets;

	return ReturnData;
}

FItemDataInfoCraftCompressed UChestoryFunctionLibrary::MakeCraftTransfer(UItemDataAsset* ItemDataAsset)
{
	if (!IsValid(ItemDataAsset))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UChestoryFunctionLibrary::MakeCraftTransfer] ItemDataAsset Invalid"));
		return FItemDataInfoCraftCompressed();
	}

	FItemDataInfoCraftCompressed ReturnInfo;

	ReturnInfo.ItemInformationCompressed = MakeItemTransfer(ItemDataAsset->Item);

	TArray<int32> ConstructNeededCurrent;
	TArray<int32> ConstructAmount;
	ConstructNeededCurrent.AddDefaulted(ItemDataAsset->Recipe.IngredientNeeded.Num());
	ConstructAmount.AddDefaulted(ItemDataAsset->Recipe.IngredientNeeded.Num());

	for (int32 i = 0; i < ConstructNeededCurrent.Num(); i++)
	{
		ConstructAmount[i] = ItemDataAsset->Recipe.IngredientNeeded[i].Amount;
		ConstructNeededCurrent[i] = ItemDataAsset->Recipe.IngredientNeeded[i].Current;
	}

	ReturnInfo.Amount = ConstructAmount;
	ReturnInfo.IngredientNeededCurrent = ConstructNeededCurrent;

	return ReturnInfo;
}

FItemDataInfoCraftCompressed UChestoryFunctionLibrary::MakeCraftTransfer(FItemDataInfoCraft InfoCraft)
{
	if (InfoCraft.ItemInformation.IsEmpty())
	{
		UE_LOG(LogInventory, Warning, TEXT("[UChestoryFunctionLibrary::MakeCraftTransfer] InfoCraft is Empty"));
		return FItemDataInfoCraftCompressed();
	}

	FItemDataInfoCraftCompressed ReturnInfo;

	ReturnInfo.ItemInformationCompressed = MakeItemTransfer(InfoCraft.ItemInformation);

	TArray<int32> ConstructNeededCurrent;
	TArray<int32> ConstructAmount;
	ConstructNeededCurrent.AddDefaulted(InfoCraft.Recipe.IngredientNeeded.Num());
	ConstructAmount.AddDefaulted(InfoCraft.Recipe.IngredientNeeded.Num());

	for (int32 i = 0; i < ConstructNeededCurrent.Num(); i++)
	{
		ConstructNeededCurrent[i] = InfoCraft.Recipe.IngredientNeeded[i].Current;
		ConstructAmount[i] = InfoCraft.Recipe.IngredientNeeded[i].Amount;
	}

	ReturnInfo.Amount = ConstructAmount;
	ReturnInfo.IngredientNeededCurrent = ConstructNeededCurrent;
	ReturnInfo.MaxCountCraft = InfoCraft.MaxCountCraft;

	return ReturnInfo;
}

FItemData UChestoryFunctionLibrary::MakeItemDataFromItemTiny(const UObject* WorldContextObject, FItemTiny ItemTiny)
{
	if (!ItemTiny.IsValid())
	{
		return FItemData();
	}
	if (!IsValid(GetInventorySubsystem(WorldContextObject)))
	{
		return FItemData();
	}
	
	FItemData ReturnData = GetInventorySubsystem(WorldContextObject)->GetItemDataFromPrimaryAssetId(GetInventorySubsystem(WorldContextObject)->GetPrimaryAssetIdFromName(ItemTiny.ID));

	ReturnData.GameplayTagContainer = ItemTiny.GameplayTagContainer;
	ReturnData.Amount = ItemTiny.Amount;
	ReturnData.JsonParameter = ItemTiny.JsonParameter;
	ReturnData.bIsDraggable = ItemTiny.bIsDraggable;
	ReturnData.bIsDroppable = ItemTiny.bIsDroppable;
	ReturnData.bIsStackable = ItemTiny.bIsStackable;
	ReturnData.bIsVisible = ItemTiny.bIsVisible;
	ReturnData.Decay = ItemTiny.Decay;
	ReturnData.Durability = ItemTiny.Durability;
	ReturnData.Attributes = ItemTiny.Attributes;
	ReturnData.bIsFilled = ItemTiny.bIsFilled;


	return ReturnData;
}



FItemData UChestoryFunctionLibrary::MakeItemDataFromItem(const UObject* WorldContextObject, FItem Item)
{
	FItemData ItemData = MakeItem(
		WorldContextObject,
		Item.PrimaryAssetId,
		Item.Amount,
		Item.Decay,
		Item.Durability,
		Item.Attributes,
		Item.Sockets,
		Item.JsonParameter);

	return ItemData;
}

FItemDataInfo UChestoryFunctionLibrary::MakeItemDataFromTransfer(const UObject* WorldContextObject, FItemDataInfoCompressed Transfer)
{
	if (Transfer.IsEmpty() && !Transfer.ItemTiny.bIsFilled)
	{
		return FItemDataInfo();
	}
	if (!IsValid(GetInventorySubsystem(WorldContextObject)))
	{
		return FItemDataInfo();
	}

	FItemDataInfo ReturnItemInfo;
	FItemData ItemData = GetInventorySubsystem(WorldContextObject)->GetItemDataFromPrimaryAssetId(GetInventorySubsystem(WorldContextObject)->GetPrimaryAssetIdFromName(Transfer.ItemTiny.ID));

	ReturnItemInfo.ID = Transfer.ItemTiny.ID;

	// Get data on the client instance (Default Value)
	ReturnItemInfo.Icon = ItemData.Icon;
	ReturnItemInfo.Name = ItemData.Name;
	ReturnItemInfo.ItemType = ItemData.ItemType;
	ReturnItemInfo.bCanDecay = ItemData.bCanDecay;
	ReturnItemInfo.DecayTime = ItemData.DecayTime;
	ReturnItemInfo.bCanRepairable = ItemData.bCanRepairable;
	ReturnItemInfo.bCanSockets = ItemData.bCanSockets;
	ReturnItemInfo.MaxSocket = ItemData.MaxSocket;
	ReturnItemInfo.Size = ItemData.Size;
	ReturnItemInfo.Weight = ItemData.Weight;


	// Get Value MaxDurability
	FGameplayTag MaxDurability = FGameplayTag::RequestGameplayTag(FName("MaxDurability"), false);
	if (MaxDurability.IsValid(); Transfer.ItemTiny.Attributes.ContainsAttributeTag(MaxDurability))
	{
		ReturnItemInfo.MaxDurability = Transfer.ItemTiny.Attributes.GetAttributeValue(MaxDurability);
	}
	else // Not found use default value
	{
		ReturnItemInfo.MaxDurability = ItemData.MaxDurability;
	}


	ReturnItemInfo.GameplayTagContainer = Transfer.ItemTiny.GameplayTagContainer;
	ReturnItemInfo.Amount = Transfer.ItemTiny.Amount;
	ReturnItemInfo.JsonParameter = Transfer.ItemTiny.JsonParameter;
	ReturnItemInfo.bIsDraggable = Transfer.ItemTiny.bIsDraggable;
	ReturnItemInfo.bIsDroppable = Transfer.ItemTiny.bIsDroppable;
	ReturnItemInfo.bIsStackable = Transfer.ItemTiny.bIsStackable;
	ReturnItemInfo.bIsVisible = Transfer.ItemTiny.bIsVisible;
	ReturnItemInfo.Decay = Transfer.ItemTiny.Decay;
	ReturnItemInfo.Durability = Transfer.ItemTiny.Durability;
	ReturnItemInfo.Attributes = Transfer.ItemTiny.Attributes;
	ReturnItemInfo.Sockets = Transfer.Sockets;
	ReturnItemInfo.bIsFilled = Transfer.ItemTiny.bIsFilled;

	return ReturnItemInfo;
}

FItemDataInfo UChestoryFunctionLibrary::MakeItemDataInfo(const UObject* WorldContextObject, FItemData ItemData)
{

	FItemDataInfo ReturnItemInfo;

	ReturnItemInfo.ID = ItemData.ID;
	ReturnItemInfo.Icon = ItemData.Icon;
	ReturnItemInfo.Name = ItemData.Name;
	ReturnItemInfo.ItemType = ItemData.ItemType;
	ReturnItemInfo.GameplayTagContainer = ItemData.GameplayTagContainer;
	ReturnItemInfo.Size = ItemData.Size;
	ReturnItemInfo.Amount = ItemData.Amount;
	ReturnItemInfo.JsonParameter = ItemData.JsonParameter;
	ReturnItemInfo.bIsDraggable = ItemData.bIsDraggable;
	ReturnItemInfo.bIsDroppable = ItemData.bIsDroppable;
	ReturnItemInfo.bIsStackable = ItemData.bIsStackable;
	ReturnItemInfo.bIsVisible = ItemData.bIsVisible;
	ReturnItemInfo.bCanDecay = ItemData.bCanDecay;
	ReturnItemInfo.Decay = ItemData.Decay;
	ReturnItemInfo.DecayTime = ItemData.DecayTime;
	ReturnItemInfo.Durability = ItemData.Durability;
	ReturnItemInfo.MaxDurability = ItemData.MaxDurability;
	ReturnItemInfo.bCanRepairable = ItemData.bCanRepairable;
	ReturnItemInfo.Attributes = ItemData.Attributes;
	ReturnItemInfo.bCanSockets = ItemData.bCanSockets;
	ReturnItemInfo.MaxSocket = ItemData.MaxSocket;
	ReturnItemInfo.Sockets = ItemData.Sockets;
	ReturnItemInfo.bIsFilled = ItemData.bIsFilled;
	ReturnItemInfo.Weight = ItemData.Weight;

	return ReturnItemInfo;
}

FItemDataInfoCraft UChestoryFunctionLibrary::MakeItemCraftFromTransfer(const UObject* WorldContextObject, FItemDataInfoCraftCompressed CraftCompressed)
{
	if (CraftCompressed.ItemInformationCompressed.IsEmpty())
	{
		UE_LOG(LogInventory, Warning, TEXT("[UChestoryFunctionLibrary::MakeItemCraftFromTransfer] CraftCompressed is Empty"));
		return FItemDataInfoCraft();
	}

	
	if (!IsValid(GetInventorySubsystem(WorldContextObject)))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UChestoryFunctionLibrary::MakeItemCraftFromTransfer] InventorySubsystem Invalid"));
		return FItemDataInfoCraft();
	}
	
	if (UItemDataAsset* OriginAsset = GetInventorySubsystem(WorldContextObject)->GetItemDataAssetFromPrimaryAssetId(GetInventorySubsystem(WorldContextObject)->GetPrimaryAssetIdFromName(CraftCompressed.ItemInformationCompressed.ItemTiny.ID)); IsValid(OriginAsset))
	{
		FItemDataInfoCraft ReturnInfo;

		ReturnInfo.ItemInformation = MakeItemDataFromTransfer(WorldContextObject,CraftCompressed.ItemInformationCompressed);

		ReturnInfo.Tag = OriginAsset->Tag;
		ReturnInfo.Category = OriginAsset->Category;
		ReturnInfo.NameCategory = OriginAsset->NameCategory;
		ReturnInfo.Priority = OriginAsset->Priority;


		FRecipeCraft RecipeInfo;
		RecipeInfo.ReturnSelf = OriginAsset->Recipe.ReturnSelf;
		RecipeInfo.ItemReturnAmount = OriginAsset->Recipe.ItemReturnAmount;
		RecipeInfo.TimeToCraft = OriginAsset->Recipe.TimeToCraft;
		RecipeInfo.PrimaryAssetIdReturn = OriginAsset->Recipe.PrimaryAssetIdReturn;
		RecipeInfo.bCanFailure = OriginAsset->Recipe.bCanFailure;
		RecipeInfo.PercentFailure = OriginAsset->Recipe.PercentFailure;
		RecipeInfo.PrimaryAssetIdFailure = OriginAsset->Recipe.PrimaryAssetIdFailure;


		TArray<FItemCraft> ItemCraftInfo;
		ItemCraftInfo.AddDefaulted(OriginAsset->Recipe.IngredientNeeded.Num());

		for (int32 i = 0; i < OriginAsset->Recipe.IngredientNeeded.Num(); i++)
		{
			ItemCraftInfo[i].Amount = CraftCompressed.Amount[i];
			ItemCraftInfo[i].PrimaryAssetId = OriginAsset->Recipe.IngredientNeeded[i].PrimaryAssetId;
			ItemCraftInfo[i].Current = CraftCompressed.IngredientNeededCurrent[i];
		}
		RecipeInfo.IngredientNeeded = ItemCraftInfo;

		ReturnInfo.Recipe = RecipeInfo;
		ReturnInfo.MaxCountCraft = CraftCompressed.MaxCountCraft;

		return ReturnInfo;
	}

	UE_LOG(LogInventory, Warning, TEXT("[UChestoryFunctionLibrary::MakeItemCraftFromTransfer] Can't Found ItemDataAsset %s :"), *CraftCompressed.ItemInformationCompressed.ItemTiny.ID.ToString());
	return FItemDataInfoCraft();

}

FItemDataInfoCraft UChestoryFunctionLibrary::MakeItemInfoCraft(const UObject* WorldContextObject, UItemDataAsset* ItemDataAsset)
{

	FItemDataInfoCraft InfoReturn;
	if(!IsValid(ItemDataAsset)) return InfoReturn;

	InfoReturn.ItemInformation = MakeItemDataInfo(WorldContextObject, ItemDataAsset->Item);
	InfoReturn.ItemInformation.Icon = ItemDataAsset->Item.Icon;
	InfoReturn.ItemInformation.Name = ItemDataAsset->Item.Name;
	InfoReturn.ItemInformation.ItemType = ItemDataAsset->Item.ItemType;

	InfoReturn.Tag = ItemDataAsset->Tag;
	InfoReturn.Category = ItemDataAsset->Category;
	InfoReturn.NameCategory = ItemDataAsset->NameCategory;
	InfoReturn.Priority = ItemDataAsset->Priority;
	InfoReturn.Recipe = ItemDataAsset->Recipe;


	return InfoReturn;
}



bool UChestoryFunctionLibrary::IsValidRecipe(UItemDataAsset* ItemDataAsset, bool PrintLog)
{

	if (!ItemDataAsset->IsCraftable)
	{
		if (PrintLog)
		{
			UE_LOG(LogInventory, Warning, TEXT("[IsValidRecipe] %s : Recipe Invalid IsCraftable is False"), *ItemDataAsset->GetFName().ToString());
		}
		return false;
	}

	if (ItemDataAsset->Recipe.IngredientNeeded.IsEmpty())
	{
		if (PrintLog)
		{
			UE_LOG(LogInventory, Warning, TEXT("[IsValidRecipe] %s : Recipe Invalid IngredientNeeded is Empty"), *ItemDataAsset->GetFName().ToString());
		}
		return false;
	}

	// Check if Recipe is Valid
	for (FItemCraft Ingredient : ItemDataAsset->Recipe.IngredientNeeded)
	{
		if (!Ingredient.PrimaryAssetId.IsValid())
		{
			if (PrintLog)
			{
				UE_LOG(LogInventory, Warning, TEXT("[IsValidRecipe] %s : Recipe Invalid PrimaryAssetId is not found"), *ItemDataAsset->GetFName().ToString());
			}
			return false;
		}
	}

	return true;
}

bool UChestoryFunctionLibrary::ContainsGrid(const UObject* WorldContextObject, FName GridKey)
{
	return GetInventorySubsystem(WorldContextObject)->ContainsGrid(GridKey);
}

UInventoryUniformGridPanel* UChestoryFunctionLibrary::GetGrid(const UObject* WorldContextObject, FName GridKey)
{
	return GetInventorySubsystem(WorldContextObject)->GetInventoryGrid(GridKey);
}

TArray<class UInventorySlotUserWidget*> UChestoryFunctionLibrary::GetGridSlots(const UObject* WorldContextObject, FName GridKey)
{
	return GetInventorySubsystem(WorldContextObject)->GetInventoryGrid(GridKey)->GetInventorySlots();
}

bool UChestoryFunctionLibrary::ContainsEquipmentSlot(const UObject* WorldContextObject, FName Context, FName SlotKey)
{
	return GetInventorySubsystem(WorldContextObject)->ContainsEquipmentSlot(Context,SlotKey);
}

UInventorySlotUserWidget* UChestoryFunctionLibrary::GetEquipmentSlot(const UObject* WorldContextObject, FName Context, FName SlotKey)
{
	return GetInventorySubsystem(WorldContextObject)->GetEquipmentSlot(Context,SlotKey);
}

TArray<class UInventorySlotUserWidget*> UChestoryFunctionLibrary::GetEquipmentSlots(const UObject* WorldContextObject, FName Context)
{
	return GetInventorySubsystem(WorldContextObject)->GetEquipmentSlots(Context);
}

bool UChestoryFunctionLibrary::ContainsHotbarSlot(const UObject* WorldContextObject, FName SlotKey)
{
	return GetInventorySubsystem(WorldContextObject)->ContainsHotbarSlot(SlotKey);
}

int32 UChestoryFunctionLibrary::GetHotbarSlotNum(const UObject* WorldContextObject)
{
	return GetInventorySubsystem(WorldContextObject)->GetHotbarSlotNum();
}

bool UChestoryFunctionLibrary::HasHotbars(const UObject* WorldContextObject)
{
	return GetInventorySubsystem(WorldContextObject)->HasHotbars();
}

UInventorySlotUserWidget* UChestoryFunctionLibrary::GetHotbarSlot(const UObject* WorldContextObject, FName SlotKey)
{
	return GetInventorySubsystem(WorldContextObject)->GetHotbarSlot(SlotKey);
}

TArray<class UInventorySlotUserWidget*> UChestoryFunctionLibrary::GetHotbarSlots(const UObject* WorldContextObject)
{
	return GetInventorySubsystem(WorldContextObject)->GetHotbarSlots();
}

bool UChestoryFunctionLibrary::ContainsInventory(const UObject* WorldContextObject, FGuid InventoryId)
{
	return GetInventorySubsystem(WorldContextObject)->ContainsInventory(InventoryId);
}

UInventoryComponent* UChestoryFunctionLibrary::GetInventory(const UObject* WorldContextObject, FGuid InventoryId)
{
	return GetInventorySubsystem(WorldContextObject)->GetInventory(InventoryId);
}

bool UChestoryFunctionLibrary::ContainsCrafting(const UObject* WorldContextObject, FGuid CraftingId)
{

	return GetInventorySubsystem(WorldContextObject)->ContainsCrafting(CraftingId);
}

UCraftingComponent* UChestoryFunctionLibrary::GetCrafting(const UObject* WorldContextObject, FGuid CraftingId)
{
	return GetInventorySubsystem(WorldContextObject)->GetCrafting(CraftingId);
}

TArray<UCraftingSlotUserWidget*> UChestoryFunctionLibrary::GetCraftingSlots(const UObject* WorldContextObject, FName GridKey)
{
  if (IsValid(GetGrid(WorldContextObject,GridKey)))
  {
	  return  GetGrid(WorldContextObject,GridKey)->CraftingSlots;
  } else 
  { 
	  return TArray<UCraftingSlotUserWidget*>(); 
  }; 
}

UInventoryManagerComponent* UChestoryFunctionLibrary::GetInventoryManagerInstance(const UObject* WorldContextObject, bool& Success)
{
	UInventoryManagerComponent* Manager = GetInventorySubsystem(WorldContextObject)->GetInventoryManagerInstance();
	Success = IsValid(Manager);

	return Manager;
}

UInventoryManagerComponent* UChestoryFunctionLibrary::GetInventoryManager(AActor* Target)
{

	if (!IsValid(Target))
	{
		return nullptr;

	}

	TArray<UInventoryManagerComponent*> IMCFound;
	Target->GetComponents(IMCFound);
	if (IMCFound.IsValidIndex(0))
	{
		if (IsValid(IMCFound[0]))
		{
			return IMCFound[0];
		}
	}

	return nullptr;
}

TArray<FName> UChestoryFunctionLibrary::GetItemsIdWithGameplayTag(const UObject* WorldContextObject, FGameplayTag Tag)
{
	if (GetInventorySubsystem(WorldContextObject))
	{
		return GetInventorySubsystem(WorldContextObject)->GetItemsIdWithGameplayTag(Tag);
	}
	return TArray<FName>();
}

bool UChestoryFunctionLibrary::IsRunningInEditor(const UObject* WorldContextObject)
{

#if WITH_EDITOR
	//Standalone
	if (WorldContextObject->GetWorld()->WorldType == EWorldType::Type::Game)
	{
		return false;
	}

	return true;
#else
	return false;
#endif
}


void UChestoryFunctionLibrary::RemakeItemDataFromSaveGame(const UObject* WorldContextObject, UInventoryManagerComponent* InventoryManager, FItemData& Item)
{
	if (!Item.IsValid())
	{
		return;
	}


	if (!IsValid(GetInventorySubsystem(WorldContextObject)))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UChestoryFunctionLibrary::RemakeItemDataFromSaveGame] InventorySubsystem Invalid"));
		return;
	}

	if (!IsValid(InventoryManager))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UChestoryFunctionLibrary::RemakeItemDataFromSaveGame] InventoryManager Invalid"));
	}


	//Get Data not Save
	FItemData DefaultItem = GetInventorySubsystem(WorldContextObject)->GetItemDataFromPrimaryAssetId(GetPrimaryAssetIdFromItemData(WorldContextObject, Item));

	Item.Icon = DefaultItem.Icon;
	Item.Name = DefaultItem.Name;
	Item.Description = DefaultItem.Description;
	Item.ItemType = DefaultItem.ItemType;
	Item.bCanAttributes = DefaultItem.bCanAttributes;
	Item.bCanSockets = DefaultItem.bCanSockets;
	Item.Size = DefaultItem.Size;
	Item.MaxSocket = DefaultItem.MaxSocket;
	Item.MaxStackable = DefaultItem.MaxStackable;
	Item.MaxDurability = DefaultItem.MaxDurability;
	Item.bCanRepairable = DefaultItem.bCanRepairable;
	Item.bCanDecay = DefaultItem.bCanDecay;
	Item.DecayTime = DefaultItem.DecayTime;
	Item.DecayItem = DefaultItem.DecayItem;
	Item.ActorEquipment = DefaultItem.ActorEquipment;
	Item.bDropActorEquipment = DefaultItem.bDropActorEquipment;
	Item.ActorDroped = DefaultItem.ActorDroped;
	Item.Weight = DefaultItem.Weight;

	InventoryManager->BuildInstanceItemScript(Item);
	int32 Index = 0;
	for (auto Instance : Item.ItemScriptInstance)
	{
		FString NameInstance = FString::Printf(TEXT("%s_Instance_%d_%s"), *Instance->GetClass()->GetFName().ToString(), Index, *Item.UniqueId.ToString());
		FInventorysSaveData DataSave = Item.GetDataBytesActorByName(FName(NameInstance));
		if (DataSave.ByteData.IsEmpty())
		{
			UE_LOG(LogInventory, Warning, TEXT("[%hs] Load ItemScipt:[%s] Failed Empty DataByte"),__FUNCTION__, *NameInstance)
			continue;
		}

		if (GetInventorySubsystem(WorldContextObject)->LoadItemScript(Item.GetDataBytesActorByName(FName(NameInstance)), FName(NameInstance), Instance))
		{
			UE_LOG(LogInventory, VeryVerbose, TEXT("[%hs] Load ItemScipt:[%s] Success"), __FUNCTION__, *NameInstance)

		}
		Index++;
	}

	return;
}


void UChestoryFunctionLibrary::UpdateDataBytesActor(const UObject* WorldContextObject, FItemData& Item, bool bBeforDestoryed)
{
	UInventorySubsystem* InventorySubsystem = GetInventorySubsystem(WorldContextObject);
	if (!IsValid(InventorySubsystem))
	{
		return;
	}

	//Clear old Data
	Item.DataBytesActor.Empty();

	if (IsValid(Item.ActorInWorld))
	{
		
		//Save Actor
		Item.DataBytesActor.Add(InventorySubsystem->SavedActor(FName("Actor"), Item.ActorInWorld, "UChestoryFunctionLibrary::UpdateDataBytesActor", bBeforDestoryed));

		//Save ActorComponent
		TArray<UActorComponent*> ChestoryComponent = InventorySubsystem->GetChestoryComponent(Item.ActorInWorld);
		for (auto& ChestComp : ChestoryComponent)
		{
			Item.DataBytesActor.Add(InventorySubsystem->SavedActorComponent(ChestComp->GetFName(), ChestComp, "UChestoryFunctionLibrary::UpdateDataBytesActor", bBeforDestoryed));
		}
	}

	int32 Index = 0;
	if (!Item.ItemScriptInstance.IsEmpty())
	{
		for (auto Instance : Item.ItemScriptInstance)
		{
			if (Instance)
			{
				FString NameInstance = FString::Printf(TEXT("%s_Instance_%d_%s"), *Instance->GetClass()->GetFName().ToString(), Index, *Item.UniqueId.ToString());
				Item.DataBytesActor.Add(InventorySubsystem->SavedItemScript(FName(NameInstance), Instance, __FUNCTION__, bBeforDestoryed));
			}
			Index++;
		}
	}
	
}



// ---------------------- SIZE FUNCTION ----------------------

FIntPoint UChestoryFunctionLibrary::GetRowColumnFromIndex(int32 index, int32 MaxPerRow)
{
	int32 Row = index / MaxPerRow;
	int32 Column = index % MaxPerRow;

	return FIntPoint(Column, Row);
}

int32 UChestoryFunctionLibrary::GetIndexFromRowColumn(FIntPoint Point, int32 MaxPerRow, int32 StartIndex)
{
	//return Point.Y * MaxPerRow + Point.X + StartIndex;
	return Point.Y * MaxPerRow + Point.X;
}