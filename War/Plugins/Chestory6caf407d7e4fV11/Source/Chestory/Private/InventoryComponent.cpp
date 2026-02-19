// Copyright 2024 Arkai Interactive. All Rights Reserved.


#include "InventoryComponent.h"
#include "Chestory.h"
#include "InventoryManagerComponent.h"
#include "GameFramework/GameModeBase.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "ChestoryFunctionLibrary.h"
#include "Serialization/MemoryWriter.h"
#include "GameFramework/PlayerState.h"

#include "Serialization/ObjectAndNameAsStringProxyArchive.h"


// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	SetNetAddressable();
	SetIsReplicated(true);

	// ...
}

UInventoryComponent::UInventoryComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
	SetIsReplicated(true);
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UInventoryComponent, InventoryID, COND_InitialOnly);
	DOREPLIFETIME(UInventoryComponent, CraftingIdLinked);

	//DOREPLIFETIME(UInventoryComponent, InventoryID);

	DOREPLIFETIME(UInventoryComponent, SlotsIndexKey);

	DOREPLIFETIME(UInventoryComponent, DecayFactor);
	DOREPLIFETIME(UInventoryComponent, DecayTickTime);
	DOREPLIFETIME(UInventoryComponent, bCanRepair);
	//DOREPLIFETIME(UInventoryComponent, Restriction);
	DOREPLIFETIME(UInventoryComponent, RestrictionPossibleItems);
	DOREPLIFETIME(UInventoryComponent, RestrictionImpossibleItems);
	DOREPLIFETIME(UInventoryComponent, InventoryType);
	DOREPLIFETIME(UInventoryComponent, DefaultDecayFactor);
	DOREPLIFETIME(UInventoryComponent, CurrentWeight);

}

// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	//SetIsReplicated(true);

	InitializeSubsystem();
	if (!IsValid(InventorySubsystem)) return;

	if (OwnerInventoryManager == nullptr)
	{
		AutoAddTag();
	}

	if (!SpawnDefinition.GridKey.IsNone())
	{
		ApplyInventoryDefinition(SpawnDefinition);
		bIsContainer = true; //TODO Warning ???
	}
	
	
	if (bIsContainer)
	{
		if (GetOwner()->HasAuthority() && Inventory.IsEmpty())
		{
			Server_InitializeInventory(ContainerSize);
		}
	}

	
	//if (GetOwner()->HasAuthority())
	if (GetOwnerRole() == ENetRole::ROLE_Authority)
	{
		if (DefaultDecayFactor == -1)
		{
			DefaultDecayFactor = DecayFactor;
		}


		//Server: empty from save create a new Id 
		if (!InventoryID.IsValid())
		{
			//Server Generate a New ID
			Server_GenerateId();
		}

	
		//Start Decay On the Server
		TryToFoundDecaySlot(); //Found default value from SaveGame
		StartDecayTimer();




		//Refresh Blueprint Item
	/*	OnItemUpdate.AddDynamic(this, &UInventoryComponent::OnInventoryUpdate);
		OnItemClear.AddDynamic(this, &UInventoryComponent::OnInventoryUpdate);*/

		OnItemEnterInventory.AddDynamic(this, &UInventoryComponent::OnInventoryUpdate);
		OnItemLeaveInventory.AddDynamic(this, &UInventoryComponent::OnInventoryUpdate);

		//Weight
		if (EnableWeight)
		{
			OnItemEnterInventory.AddDynamic(this, &UInventoryComponent::OnIncreaseWeight);
			OnItemLeaveInventory.AddDynamic(this, &UInventoryComponent::OnDecreaseWeight);

			//Start Weight value
			RefreshWeight();
		}



		//Bind delgate Tirgger Script on item
		OnContainerIsOpen.AddDynamic(this, &UInventoryComponent::NotifyScript_InventoryOpen);
		OnContainerIsClose.AddDynamic(this, &UInventoryComponent::NotifyScript_InventoryClose);

	
	}



	if (InventoryID.IsValid())
	{
		InventorySubsystem->RegisterInventory(InventoryID, this);
	}
	// ...
	
}

void UInventoryComponent::OnComponentCreated()
{
	Super::OnComponentCreated();

	if (GetOwner()->HasAuthority())
	{
		//Server: empty from save create a new Id 
		if (!InventoryID.IsValid())
		{
			//Server Generate a New ID
			InventoryID = FGuid::NewGuid();

		}
	}


}

void UInventoryComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	Super::OnComponentDestroyed(bDestroyingHierarchy);

	//IsValid(GetOwner()) && 


	if (IsValid(InventorySubsystem) && InventoryID.IsValid())
	{
		//UE_LOG(LogInventory, Display, TEXT("[UInventoryComponent::OnComponentDestroyed] Owner %s"), *GetOwner()->GetFName().ToString())
		InventorySubsystem->UnregisterInventory(InventoryID);
	}


}


// Called every frame
void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UInventoryComponent::OnObjectPreLoad_Implementation()
{

	if (GetInventoryType() == ESlotType::EEquipement)
	{
		if (IsValid(GetInventoryManager()))
		{
			if (!GetInventoryItems().IsEmpty())
			{
				for (int32 i = 0; i < GetInventoryItems().Num(); i++)
				{
					if (GetInventoryItem(i).IsValid())
					{
						GetInventoryManager()->DestoryEquipmentItem(this, i);
					}
				}
			}
		}
	}
	

	if (InventoryID.IsValid())
	{
		if (!IsValid(InventorySubsystem))
		{
			InitializeSubsystem();
		}

		InventorySubsystem->UnregisterInventory(InventoryID);
	}
}

void UInventoryComponent::OnObjectPostLoad_Implementation()
{
	//UE_LOG(LogInventory, Warning, TEXT("[UInventoryComponent::OnObjectPostLoad] Server : [%s] "), *FString(GetOwner()->GetFName().ToString() + "." + GetFName().ToString()));
	ReconstructitemsFromSave();
}

void UInventoryComponent::OnObjectPreSave_Implementation()
{
	//Save DataByte and Notify PreSave
	int32 Index = 0;
	for (auto&& Item : Inventory)
	{
		Item = Item.NotifyScript_ItemPreSave(GetInventoryManager(), GetId(), Index);
		UChestoryFunctionLibrary::UpdateDataBytesActor(this, Item);

		Index++;
	}
}

void UInventoryComponent::OnObjectPreDestoryed_Implementation()
{
	if (GetOwner()->HasAuthority())
	{
		//Auto Close Viwers 
		if (HasViwers())
		{
			TMap<class UInventoryManagerComponent*, FName> Copy_Viewers = GetViewers();

			for (auto& Viewer : Copy_Viewers)
			{
				Viewer.Key->Server_CloseInventory(InventoryID);
			}
		}
	}
}

void UInventoryComponent::TryToFoundDecaySlot()
{
	if (GetInventoryItems().IsEmpty())
	{
		return;
	}

	//Clear Data
	MDecaySlots.Empty();

	for (int32 i = 0; i < GetInventoryItems().Num(); i++)
	{
		if (GetInventoryItem(i).bCanDecay)
		{
			MDecaySlots.Add(i, false);
		}
	}

}

void UInventoryComponent::StartDecayTimer()
{
	GetWorld()->GetTimerManager().SetTimer(DecayTimer, this, &UInventoryComponent::DecayTick, DecayTickTime, true, 0.0f);

}

void UInventoryComponent::Server_GenerateId_Implementation()
{
	InventoryID = FGuid::NewGuid();
	UE_LOG(LogInventory, VeryVerbose, TEXT("[UInventoryComponent::Server_GenerateId] Server : [%s] NEW Inventory ID : [%s]"), *FString(GetOwner()->GetFName().ToString() + "." + GetFName().ToString()), *InventoryID.ToString());
	OnRep_InventoryID();
	Server_UpdateId();
}


void UInventoryComponent::Server_SetId_Implementation(FGuid Id)
{
	InventoryID = Id;
	UE_LOG(LogInventory, VeryVerbose, TEXT("[UInventoryComponent::Server_SetId] Server : [%s] Set Inventory ID : [%s]"), *FString(GetOwner()->GetFName().ToString() + "." + GetFName().ToString()), *InventoryID.ToString());
	InventorySubsystem->RegisterInventory(InventoryID, this);
	OnRep_InventoryID();
}

void UInventoryComponent::Server_UpdateId_Implementation()
{
	Server_SetId(InventoryID);
}

void UInventoryComponent::Multicast_UpdateId_Implementation(FGuid Id)
{
	InventoryID = Id;
	InventorySubsystem->RegisterInventory(Id, this);
}

void UInventoryComponent::OnRep_InventoryID()
{

	if (!IsValid(InventorySubsystem))
	{
		InitializeSubsystem();
	}

	FString Net = GetOwner()->HasAuthority() ? FString("Server") : FString("Client");

	InventorySubsystem->RegisterInventory(InventoryID, this);

	UE_LOG(LogInventory, Verbose, TEXT("[UInventoryComponent::OnRep_InventoryID] OnRep net :[%s] Owner:[%s] Inventory ID:[%s]"), *Net, *FString(GetOwner()->GetFName().ToString() + "." + GetFName().ToString()), *InventoryID.ToString());
}

FGuid UInventoryComponent::GetId()
{
	return InventoryID;
}


void UInventoryComponent::NotifyScript_InventoryOpen(FName OnGridKey)
{
	RefreshWeight();

	for (int32 Index = 0; Index < GetInventoryItems().Num(); Index++)
	{

		if (GetInventoryItem(Index).IsEmpty())
		{
			continue;
		}

		GetInventoryItem(Index).NotifyScript_InventoryOpen(GetInventoryManager(), GetId(), Index);
	}
}

void UInventoryComponent::NotifyScript_InventoryClose(FName OnGridKey)
{
	for (int32 Index = 0; Index < GetInventoryItems().Num(); Index++)
	{

		if (GetInventoryItem(Index).IsEmpty())
		{
			continue;
		}

		GetInventoryItem(Index).NotifyScript_InventoryClose(GetInventoryManager(), GetId(), Index);
	}
}

void UInventoryComponent::AddViewer(UInventoryManagerComponent* InventoryManager, FName OnGridKey)
{
	Viewers.Add(InventoryManager, OnGridKey);
}

void UInventoryComponent::RemoveViewer(UInventoryManagerComponent* InventoryManager)
{
	Viewers.Remove(InventoryManager);
}



FName UInventoryComponent::GetGridFromInventoryManager(UInventoryManagerComponent* InventoryManager)
{
	if (!GetViewers().Contains(InventoryManager))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryComponent::GetGridFromInventoryManager] Return DefaultValue Do not contain this InventoryManager:[%s] Owner:[%s]"),
			*InventoryManager->GetFName().ToString(), *InventoryManager->GetOwner()->GetFName().ToString());
		return GridKey;
	}

	return *GetViewers().Find(InventoryManager);
}

void UInventoryComponent::OnInventoryUpdate(FPrimaryAssetId PrimaryAssetId, int32 Amount)
{

	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	if (GetViewers().IsEmpty())
	{
		return;
	}

	if (EnableDynamicSizeInventory && EnableAutoFill)
	{
		AutoInventorySize();
	}

	if (IsValid(InventorySubsystem->GetInventoryManagerInstance()))
	{
		InventorySubsystem->GetInventoryManagerInstance()->Server_UpdateItemBlueprint(GetId());
	}
}

// ------------------------ DECAY SYSTEM ------------------------	
#pragma region /** Decay */

void UInventoryComponent::ResetDecayFactor_Implementation()
{
	DecayFactor = DefaultDecayFactor;
}



void UInventoryComponent::SetDecayFactor_Implementation(float NewDecayFactor)
{
	DecayFactor = NewDecayFactor;
}

void UInventoryComponent::DecayTick()
{
	if (!MDecaySlots.IsEmpty())
	{
		// Reset DecaysSlots MaxRefresh to false
		ResetDecaySlots();
		const TMap<int32,bool> LocalDecaySlots = MDecaySlots;
		TMap<FPrimaryAssetId,int32> TmpAssetToAdd;

		UInventoryManagerComponent* TmpManager = GetInventoryManager();
		
		
		for(int32 i = 0; i < MDecaySlots.Num(); i++)
		{
			TArray<int32> Keys;
			MDecaySlots.GetKeys(Keys);

			if (int32 SlotItem = Keys[i]; GetItemIsValid(Inventory[SlotItem]))
			{
				if (Inventory[SlotItem].bCanDecay)
				{
					Inventory[SlotItem].Decay -= (DecayTickTime * DecayFactor);
					if (Inventory[SlotItem].Decay <= 0.0f)
					{
						Inventory[SlotItem].Decay = Inventory[SlotItem].DecayTime;
						Inventory[SlotItem].Amount -= 1;

						//Update Maps with true for max Refresh
						MDecaySlots.Add(SlotItem,true);
						
						if (Inventory[SlotItem].DecayItem.IsValid())
						{
							if (TmpManager != nullptr)
							{
								
								//Check for Disable Crafting update
								// 
								// Stack Decay asset for TryToAdd of the end function
								if(TmpAssetToAdd.Contains(Inventory[SlotItem].DecayItem))
								{
									
									int tmpAmount = *TmpAssetToAdd.Find(Inventory[SlotItem].DecayItem);
									tmpAmount++;
									TmpAssetToAdd.Add(Inventory[SlotItem].DecayItem,tmpAmount);
								}
								else
								{
									//Add First
									TmpAssetToAdd.Add(Inventory[SlotItem].DecayItem,1);
								}
							}
						}
						
						if (Inventory[SlotItem].Amount == 0)
						{
							if (TmpManager != nullptr)
							{
								TmpManager->RemoveItem(this,SlotItem);
							}
							else
							{
								TArray<int32> OutArrayUnfilled;
								ClearInventoryItem(SlotItem, OutArrayUnfilled);
							}
						
						}
					}
				}
			}
		}

		//Update Widget Slot Need InventoryManager Valid
		//Is Inventory Player
		
		TArray<int32> Keys;
		TArray<bool> Values;
		MDecaySlots.GetKeys(Keys);
		MDecaySlots.GenerateValueArray(Values);

		if (InventorySubsystem->ContainsInventory(InventoryID))
		{
			for (auto& Viewer : GetViewers())
			{
				Viewer.Key->Server_UpdateDecaySlots(InventoryID, Viewer.Value, Keys, Values);
			}
		}
		else
		{
			Server_SetId(InventoryID);
		}
	


		// Check if need to refresh Craft slot & Item Blueprint
		if (TmpManager != nullptr)
		{

			// Need to Add Asset 
			if(!TmpAssetToAdd.IsEmpty())
			{
				for(auto& Asset : TmpAssetToAdd)
				{
					bool success;
					FItemData OutItem;
					int32 IndexOut;
					UE_LOG(LogInventory,Verbose,TEXT("[UInventoryComponent::DecayTick] End Decay Tick TryToAdd %s Amount %d"), *Asset.Key.ToString(),Asset.Value);
					UInventoryComponent* TargetInventory = (GetIsEquipmentInventory() || GetIsHotbarInventory()) ? InventorySubsystem->GetInventory(GetInventoryManager()->GetFirstPlayerInventoryId()) : this;
					GetInventoryManager()->TryToAddItemToInventory(success,OutItem,IndexOut,TargetInventory,Asset.Key, Asset.Value,true);
				}
			}

		}
	}
}

#pragma endregion




// ------------------------ WEIGHT SYSTEM ------------------------	
#pragma region /** Weight */

void UInventoryComponent::SetMaxWeight_Implementation(float NewMaxWeight)
{
	MaxWeight = NewMaxWeight;
}

void UInventoryComponent::SetWeightFactor_Implementation(float NewWeightFactor)
{
	WeightFactor = NewWeightFactor;
}


void UInventoryComponent::SetTotalWeight_Implementation(float NewTotalWeight)
{
	CurrentWeight = NewTotalWeight;

	OnRep_CurrentWeight();

	//UE_LOG(LogInventory, Warning, TEXT("SetTotalWeight : %f"), NewTotalWeight)
}

void UInventoryComponent::OnRep_CurrentWeight()
{

	Multicast_UpdateWeightValue(CurrentWeight);
}

void UInventoryComponent::Multicast_UpdateWeightValue_Implementation(float NewTotalWeight)
{
	CurrentWeight = NewTotalWeight;

	OnWeightChanged.Broadcast(GetInventoryWeight(), GetMaxWeight());
}


float UInventoryComponent::GetMaxWeight()
{
	if (IsValid(OwnerInventoryManager))
	{
		if (OwnerInventoryManager->EnableWeight)
		{
			return	OwnerInventoryManager->GetMaxWeight();
		}
	}

	return MaxWeight;
}

float UInventoryComponent::GetInventoryWeight()
{
	if (IsValid(OwnerInventoryManager))
	{
		if (OwnerInventoryManager->EnableWeight)
		{
			return OwnerInventoryManager->GetTotalWeight();
		}
	}

	return CurrentWeight * WeightFactor;
}




void UInventoryComponent::IncreaseWeight(float Amount)
{
	//UE_LOG(LogInventory, Warning, TEXT("IncreaseWeight : %f"), Amount)

	//TODO Clamp Max ?
	SetTotalWeight(CurrentWeight + Amount);
}

void UInventoryComponent::DecreaseWeight(float Amount)
{
	//UE_LOG(LogInventory, Warning, TEXT("DecreaseWeight : %f Current:%f "), Amount, CurrentWeight)

	SetTotalWeight(FMath::Max(CurrentWeight - Amount, 0));
}

void UInventoryComponent::RefreshWeight()
{
	if (!EnableWeight) { return; }

	float NewTotalWeight = 0.f;
	for (const auto& Item : Inventory)
	{
		if (!Item.IsEmpty())
		{
			NewTotalWeight = NewTotalWeight + Item.GetWeight();
		}
	}

	SetTotalWeight(NewTotalWeight);
}

void UInventoryComponent::OnIncreaseWeight(FPrimaryAssetId PrimaryAssetId, int32 Amount)
{
	FItemData Item = UChestoryFunctionLibrary::GetItemDataFromPrimaryAssetId(this, PrimaryAssetId);
	if (!Item.IsEmpty())
	{
		IncreaseWeight(Item.Weight * Amount);
	}
}

void UInventoryComponent::OnDecreaseWeight(FPrimaryAssetId PrimaryAssetId, int32 Amount)
{
	FItemData Item = UChestoryFunctionLibrary::GetItemDataFromPrimaryAssetId(this, PrimaryAssetId);
	if (!Item.IsEmpty())
	{
		DecreaseWeight(Item.Weight * Amount);
	}
}


#pragma endregion


bool UInventoryComponent::CheckItemInRestriction(FItemData Item, int32 IndexSlot)
{

	if (!Item.bIsDraggable && !Item.bIsDroppable)
	{
		return false;
	}

	//Check restriction by slot 
	if (IndexSlot != -1 && (GetIsEquipmentInventory() || GetIsHotbarInventory()))
	{
		if (SlotsRestriction.Find(SlotsIndexKey[IndexSlot])->IsEmpty())
		{
			return true;
		}

		UE_LOG(LogTemp, Verbose, TEXT("[UInventoryComponent::CheckItemInRestriction] Check SlotKey:[%s]"), *SlotsIndexKey[IndexSlot].ToString());
		return Item.GameplayTagContainer.MatchesQuery(*SlotsRestriction.Find(SlotsIndexKey[IndexSlot]));
	}


	// No rule restriction return true
	if (Restriction.IsEmpty() && RestrictionImpossibleItems.IsEmpty() && RestrictionPossibleItems.IsEmpty() && (EnableWeight && !EnableWeightRestriction))
	{
		return true;
	}

	//If use Weight check 
	if (EnableWeight && EnableWeightRestriction)
	{
		if (Item.GetWeight() != 0 && GetWeightAvaiable() < Item.GetWeight())
		{
			UE_LOG(LogTemp, Verbose, TEXT("[UInventoryComponent::CheckItemInRestriction] Weight Restriction"));
			return false;
		}
	}

	const FPrimaryAssetId LocalAsset = InventorySubsystem->GetPrimaryAssetIdFromName(Item.ID);

	// Check if item is in Impossible list if true return false
	if (!RestrictionImpossibleItems.IsEmpty())
	{
		if(RestrictionImpossibleItems.Contains(LocalAsset))
		{
			UE_LOG(LogTemp, Verbose, TEXT("[UInventoryComponent::CheckItemInRestriction] Item is in Restriction Impossible Items"));
			return false;
		}
	}

	

	// Check if item is in Possible List if not then return false
	if (!RestrictionPossibleItems.IsEmpty())
	{
		if(RestrictionPossibleItems.Contains(LocalAsset))
		{
			return true;
		}
	} 

	if (!Restriction.IsEmpty())
	{
		// Here Has restriction if not in impossible list and it is in possible (or possible is empty) check if has Tag
		return Item.GameplayTagContainer.MatchesQuery(Restriction);
	}

	return true;
}

bool UInventoryComponent::HaveSapceInSlot(FItemData Item, int32 FromSlot, int32 ToCheckedSlot, bool ForSwap)
{
	// Ignore MiniSize or if this inventory does not use SizeItem
	if (Item.IsMiniSize() || !EnableSizeItem)
	{
		if (ForSwap)
		{
			// If swapping, check if the target slot is also MiniSize
			if (GetInventoryItem(ToCheckedSlot).IsMiniSize())
			{
				return true;
			}
		}
		else
		{
			// If not swapping, always return true
			return true;
		}
	}

	// Get the points that need to be checked for the item
	TArray<FIntPoint> PointsToCheck = GetPointsFromItemSlot(Item, ToCheckedSlot);
	UE_LOG(LogInventory, Verbose, TEXT("[UInventoryComponent::HaveSapceInSlot] Total points to check: [%d]"), PointsToCheck.Num());

	// Get the points that would be occupied if the item were moved to the target slot
	TArray<FIntPoint> SimulatePoint = GetPointsFromItemSlot(GetInventoryItem(ToCheckedSlot), FromSlot);
	UE_LOG(LogInventory, Verbose, TEXT("[UInventoryComponent::HaveSapceInSlot] SimulateTargetPoint to check: [%d]"), SimulatePoint.Num());

	// Calculate the indices of the simulated target points
	TArray<int32> SimulateTargetIndex;
	if (ForSwap)
	{
		for (const auto& SimPoint : SimulatePoint)
		{
			int32 SimIndexChild = UChestoryFunctionLibrary::GetIndexFromRowColumn(SimPoint, ContainerSlotsPerRow, 0);
			SimulateTargetIndex.Add(SimIndexChild);
		}
	}

	// Check each point to see if it is available
	for (const auto& Point : PointsToCheck)
	{
		int32 Column = Point.X;
		int32 Row = Point.Y;
		int32 IndexChild = UChestoryFunctionLibrary::GetIndexFromRowColumn(Point, ContainerSlotsPerRow, 0);

		// Check if the column is within the valid range
		if (Column >= ContainerSlotsPerRow)
		{
			UE_LOG(LogInventory, Warning, TEXT("[UInventoryComponent::HaveSapceInSlot] Column out of range: Row:[%d] Column:[%d] Index:[%d]"), Row, Column, ToCheckedSlot);
			return false;
		}

		// Check if the index is within the inventory bounds
		if (IndexChild >= Inventory.Num())
		{
			UE_LOG(LogInventory, Warning, TEXT("[UInventoryComponent::HaveSapceInSlot] Index out of bounds: Row:[%d] Column:[%d] Index:[%d]"), Row, Column, IndexChild);
			return false;
		}

		// Check if the slot is already occupied
		if (GetInventoryItem(IndexChild).IsValid())
		{
			// Ignore the slot if it is the same as the source slot
			if (IndexChild == FromSlot)
			{
				continue;
			}

			// If swapping, ignore the slot if it is part of the target item's occupied slots
			if (ForSwap && (Inventory[ToCheckedSlot].FilledSlotsIndex.Contains(IndexChild) || IndexChild == ToCheckedSlot))
			{
				if (!SimulateTargetIndex.Contains(IndexChild))
				{
					continue;
				}
			}

			UE_LOG(LogInventory, Warning, TEXT("[UInventoryComponent::HaveSapceInSlot] Slot not available: Row:[%d] Column:[%d] Index:[%d]"), Row, Column, IndexChild);
			return false;
		}
	}
	
	//UE_LOG(LogInventory, Warning, TEXT("[UInventoryComponent::HaveSapceInSlot] Success SpaceFound for ToSlot Row:[%d] Column:[%d] Index:[%d]"), ItemPoint.Y, ItemPoint.X, ToCheckedSlot);

	return true;
}

TArray<FIntPoint> UInventoryComponent::GetPointsFromItemSlot(FItemData Item, int32 Slot)
{
	TArray<FIntPoint> Points;


	// Get the starting point (row and column) from the given slot index
	FIntPoint ItemPoint = UChestoryFunctionLibrary::GetRowColumnFromIndex(Slot, ContainerSlotsPerRow);
	// Get the size of the item
	FIntPoint ItemSize = Item.Size;

	// Reserve memory for the points to avoid frequent reallocations
	Points.Reserve(ItemSize.X * ItemSize.Y);

	// Single loop to generate all points for the item
	for (int32 i = 0; i < ItemSize.X * ItemSize.Y; i++)
	{
		// Calculate the X coordinate using modulo operation
		int32 X = ItemPoint.X + (i % ItemSize.X);
		// Calculate the Y coordinate using integer division
		int32 Y = ItemPoint.Y + (i / ItemSize.X);

		// Add the calculated point to the array
		Points.AddUnique(FIntPoint(X, Y));
	}

	return Points;
}

TArray<int32> UInventoryComponent::GetFilledIndexFromItemSlot(FItemData Item, int32 Slot)
{
	TArray<int32> OutIndexs;
	for (auto Point : GetPointsFromItemSlot(Item, Slot))
	{
		int32 Index = UChestoryFunctionLibrary::GetIndexFromRowColumn(Point, ContainerSlotsPerRow, Slot);
		if (Index <= Inventory.Num())
		{
				OutIndexs.Add(Index);
		}
	}

	return OutIndexs;
}

void UInventoryComponent::InitializeSubsystem()
{
	const UGameInstance* GameInstance = GetOwner()->GetGameInstance();
	if (IsValid(GameInstance))
	{
		InventorySubsystem = GameInstance->GetSubsystem<UInventorySubsystem>();
	}
}

void UInventoryComponent::ApplyInventoryDefinition(FInventoryDefinition Definition)
{
	bIsContainer = false;
	GridKey = Definition.GridKey;
	ContainerSize = Definition.InventorySize;
	ContainerSlotsPerRow = Definition.InventorySlotsPerRow;
	EnableSizeItem = Definition.EnableSizeItem;
	EnableAutoFill = Definition.EnableAutoFill;
	EnableDynamicSizeInventory = Definition.EnableDynamicSizeInventory;
	ContainerMiniRow = Definition.InventoryMiniRow;
	DecayFactor = Definition.DecayFactor;
	DecayTickTime = Definition.DecayTickTime;
	bCanRepair = Definition.bCanRepair;
	Restriction = Definition.Restriction;
	RestrictionImpossibleItems = Definition.RestrictionImpossibleItems;
	RestrictionPossibleItems = Definition.RestrictionPossibleItems;
	
	if (IsValid(OwnerInventoryManager))
	{
		EnableWeight = Definition.OverrideWeight ? Definition.EnableWeight : OwnerInventoryManager->EnableWeight;
	}

}

void UInventoryComponent::SetOwnerInventoryManager(UInventoryManagerComponent* Owner)
{
	OwnerInventoryManager = Owner;
}

void UInventoryComponent::ClearOwnerInventoryManager()
{	
	OwnerInventoryManager = nullptr; 
}



UInventoryManagerComponent* UInventoryComponent::GetInventoryManager()
{
	//return InventorySubsystem->GetInventoryManagerInstance();

	if (IsValid(OwnerInventoryManager))
	{
		return OwnerInventoryManager;
	}

	if (IsValid(InventorySubsystem->GetInventoryManagerInstance()))
	{
		return InventorySubsystem->GetInventoryManagerInstance();
	}

	
	return nullptr;
}

AActor* UInventoryComponent::GetOwnerForLocation()
{
	if (APlayerState* PS = Cast<APlayerState>(GetOwner()))
	{
		if (PS->GetPlayerController())
		{
			return PS->GetPlayerController()->GetPawn();
		}
	}

	return GetOwner();
}

void UInventoryComponent::ReconstructitemsFromSave()
{
	if (!IsValid(InventorySubsystem))
	{
		InitializeSubsystem();
	}

	//if item is not ready wait loading
	if (!InventorySubsystem->bItemsCompletedLoad)
	{
		InventorySubsystem->OnLoadingStoreItemsCompleted.AddDynamic(this, &UInventoryComponent::ReconstructitemsFromSave);
		
		return;
	}

	UE_LOG(LogInventory, Display, TEXT("[UInventoryComponent::ReconstructItemsFromSave] Owner:[%s].[%s]"), *GetOwner()->GetFName().ToString(),*GetFName().ToString())

	//Server: empty from save create a new Id 
	if (!InventoryID.IsValid())
	{
		//Server Generate a New ID
		Server_GenerateId();
		UE_LOG(LogInventory, Display, TEXT("[UInventoryComponent::ReconstructitemsFromSave] GenerateId Owner:[%s] Id:[%s]"), *GetOwner()->GetFName().ToString(),*InventoryID.ToString())

	}
	else
	{
		Multicast_UpdateId(InventoryID);
	}

	//Reconstrue Item variable with data missing 
	int32 Index = 0;
	for (auto&& Item : Inventory)
	{
		Inventory[Index].SetLocationItem(GetId(), Index);
		UChestoryFunctionLibrary::RemakeItemDataFromSaveGame(this, GetInventoryManager(),Item);
		Item = Item.NotifyScript_ItemPostLoad(GetInventoryManager(), GetId(), Index);
		Index++;
	}

	RefreshWeight();


	//Refresh data on Client with the new Data
	if (IsValid(OwnerInventoryManager))
	{
		if (GetIsEquipmentInventory())
		{
			UE_LOG(LogInventory, Display, TEXT("[UInventoryComponent::ReconstructitemsFromSave] Refresh Equipment Slots %s"), *GetOwner()->GetFName().ToString())

			OwnerInventoryManager->RefreshEquipementSlots();
		}

		if (GetIsHotbarInventory())
		{
			OwnerInventoryManager->RefreshHotbarSlots();
		}

	
	}

	//Found default Decay value from SaveGame
	TryToFoundDecaySlot(); 
	//Start Decay On the Server
	GetWorld()->GetTimerManager().SetTimer(DecayTimer, this, &UInventoryComponent::DecayTick, DecayTickTime, true, 0.0f);


	InventorySubsystem->OnLoadingStoreItemsCompleted.RemoveDynamic(this, &UInventoryComponent::ReconstructitemsFromSave);
}

FItemForSave UInventoryComponent::GetItemsForSave()
{
	FItemForSave ItemsSave;
	TArray<FItem> ListToSave;
	TArray<FItemCurrency> CurrencySave;

	for (int32 i = 0; i < Inventory.Num(); i++)
	{
		Inventory[i] = Inventory[i].NotifyScript_ItemPreSave(GetInventoryManager(), GetId(), i);
		FItem item;
		item.PrimaryAssetId = InventorySubsystem->GetPrimaryAssetIdFromName(Inventory[i].ID);
		item.Amount = Inventory[i].Amount;
		item.Decay =Inventory[i].Decay;
		item.Durability = Inventory[i].Durability;
		item.bInEquipement = false;
		item.bInHotbar = false;
		item.IndexSlot = i;
		item.Attributes = Inventory[i].Attributes;
		item.Sockets = Inventory[i].Sockets;
		ListToSave.Add(item);
	}
	ItemsSave.ItemsInventory = ListToSave;
	ItemsSave.Currency = CurrencySave;

	return ItemsSave;
}

FInventorySaved UInventoryComponent::GetInventorySave()
{
	FInventorySaved OutInventorySave;
	OutInventorySave.InventorySize = Inventory.Num();
	OutInventorySave.SlotsIndexKey = SlotsIndexKey;

	int32 LoopIndex = 0;
	for (auto Slot : Inventory)
	{
		if (!Slot.IsEmpty())
		{
			OutInventorySave.SlotsSaved.Add(FItemDataSlotSaved(Slot.NotifyScript_ItemPreSave(GetInventoryManager(),GetId(),LoopIndex), LoopIndex));
		}
		LoopIndex++;
	}

	return OutInventorySave;
}

void UInventoryComponent::LoadInventoryFromSave(FInventorySaved InventorySave)
{

	SlotsIndexKey = InventorySave.SlotsIndexKey;

	Server_InitializeInventory(InventorySave.InventorySize);

	for (const auto& SlotSaved : InventorySave.SlotsSaved)
	{
		Inventory[SlotSaved.IndexSlot] = SlotSaved.ToItemData();
	}

	IChestorySaveGameInterface::Execute_OnObjectPostLoad(this);
}

bool UInventoryComponent::GetItemIsValid(FItemData Item)
{
	// Check if Item as Default ID if true that item is Invalid
	return (Item.IsValid());
}

 bool UInventoryComponent::InitializeInventory(int32 InventorySize)
{

	//Clear Array
	Inventory.Empty();

	//Add Default item
	Inventory.AddDefaulted(InventorySize);

	// Debug be careful if fonction use then not a container
	FString MyName = "None";
	GetOwner()->GetName(MyName);
	UE_LOG(LogInventory, Verbose, TEXT("[UInventoryComponent::InitializeInventory] Initialize Name: [%s] Size: [%d]"), *MyName, InventorySize);
	//Check if the good lenht of array
	bool bSuccess = false;
	if (Inventory.Num() == InventorySize)
	{
		bSuccess = true;
	}

	return bSuccess;
}

void UInventoryComponent::Server_InitializeInventory_Implementation(int32 InventorySize)
{
	InventorySize = (InventorySize == -1) ? ContainerSize : InventorySize;
	UE_LOG(LogInventory, Verbose, TEXT("[UInventoryComponent::Server_InitializeInventory] Server Initialize Size : %d"), InventorySize);
	InitializeInventory(InventorySize);
}

UPARAM(DisplayName = "InventoryItem")FItemData UInventoryComponent::GetInventoryItem(int32 InventorySlot)
{
	if (Inventory.IsValidIndex(InventorySlot))
	{
		// Check if Item is valid if true then return the slot Data
		if (GetItemIsValid(Inventory[InventorySlot]))
		{
			return UPARAM(DisplayName = "InventoryItem")Inventory[InventorySlot];
		}
		else
		{
			return UPARAM(DisplayName = "InventoryItem")FItemData();
		}
	}
	else
	{
		return UPARAM(DisplayName = "InventoryItem")FItemData();
	}
		
}

FItemData& UInventoryComponent::GetItemByRef(int32 InventorySlot)
{
	// TODO: insérer une instruction return ici	
	return Inventory[InventorySlot];
}

UPARAM(DisplayName = "Success") bool UInventoryComponent::SetInventoryItem(int32 InventorySlot, FItemData InventoryItem,TArray<int32>& Filled)
{
	if (Inventory.IsValidIndex(InventorySlot))
	{
		Inventory[InventorySlot] = InventoryItem;
		Inventory[InventorySlot].SetLocationItem(GetId(), InventorySlot);

		//Filled Slot only on Grid Inventory 
		if (GetIsPlayerInventory() || GetIsContainerInventory() && EnableSizeItem)
		{
			//Check if is visible then do not filled inventory
			if (Inventory[InventorySlot].bIsVisible)
			{
				Filled = GetFilledIndexFromItemSlot(InventoryItem, InventorySlot);
				Filled.Remove(InventorySlot);
				if (!Filled.IsEmpty())
				{
					//Update Filled
					Inventory[InventorySlot].FilledSlotsIndex = Filled;

					//Lock slot 
					for (auto IndexFilled : Filled)
					{
						UE_LOG(LogInventory, VeryVerbose, TEXT("[ UInventoryComponent::SetInventoryItem]For:[%d] Filled Index:[%d]"), InventorySlot, IndexFilled)
						Inventory[IndexFilled].bIsFilled = true;
					}
				}
			}			
		}
	

		OnItemUpdate.Broadcast(InventorySlot);

		// Check if this Item has decay option
		if (Inventory[InventorySlot].bCanDecay)
		{
		//	UE_LOG(LogInventory,Verbose,TEXT("Add Decay Item"));
			MDecaySlots.Add(InventorySlot, false);
		}
		else
		{
			MDecaySlots.Remove(InventorySlot);
		}
		return UPARAM(DisplayName = "Success") true;
	}
	else
	{
		return UPARAM(DisplayName = "Success") false;
	}
	//Replace Element on index
	

	return UPARAM(DisplayName = "Success") true;
}

void UInventoryComponent::ClearInventoryItem(int32 InventorySlot,TArray<int32>& Unfilled /*= TArray<int32>()*/)
{
	if (Inventory.IsValidIndex(InventorySlot))
	{
		//Unfilled Slot only on Grid Inventory
		if (GetIsPlayerInventory() || GetIsContainerInventory() && EnableSizeItem)
		{
			Unfilled = GetFilledIndexFromItemSlot(GetInventoryItem(InventorySlot), InventorySlot);
			Unfilled.Remove(InventorySlot);
			//Unlock slot 
			for (auto IndexFilled : Unfilled)
			{
				Inventory[IndexFilled].bIsFilled = false;
			}
		}

		OnItemClear.Broadcast(InventorySlot);

		Inventory[InventorySlot] = FItemData();
		MDecaySlots.Remove(InventorySlot);
		
	}
}

int32 UInventoryComponent::FindInventoryItem(bool& Success, FName ItemId)
{
	Success = false;

	for (int32 Index = 0; Index < Inventory.Num(); Index++)
	{
		if (!GetInventoryItem(Index).IsEmpty())
		{
			if (GetInventoryItem(Index).ID == ItemId)
			{
				Success = true;
				return Index;
			}
		}
	}

	return -1;
}

void UInventoryComponent::ClearInventory()
{
	for (int Index = 0; Index < Inventory.Num(); Index++)
	{
		if (GetInventoryItem(Index).IsValid())
		{
			TArray<int32> Unfilled;
			ClearInventoryItem(Index, Unfilled);
		}
	}
}

void UInventoryComponent::GetEmptyInventorySpace(bool& Success, int32& IndexSlot, FItemData ForItem)
{
	Success = false;
	IndexSlot = -1;

	for (int32 i = 0; i < Inventory.Num(); i++)
	{

		if (ForItem.IsMiniSize() || !GetIsGirdInventory() || !EnableSizeItem)
		{
			// Check if item is Invalid
			if (!GetItemIsValid(Inventory[i]))
			{
				//Item is Invalid so break and return index empty 
				Success = true;
				IndexSlot = i;
				break;
			}
			else
			{
				// Not slot empty found 
				Success = false;
				IndexSlot = -1;
			}
		}
		else
		{
			// don't check this slot if has item
			if (GetInventoryItem(i).IsValid())
			{
				continue;
			}

			if (HaveSapceInSlot(ForItem, -1, i))
			{
				Success = true;
				IndexSlot = i;
				break;
			}
			else
			{
				// Not slot empty found 
				Success = false;
				IndexSlot = -1;
			}
		}
	
	}
}

int32 UInventoryComponent::GetAmountEmptyInventorySpace()
{
	int32 AmountEmpty = 0;
	for(auto Slot : GetInventoryItems())
	{
		if (Slot.IsEmpty())
		{
			AmountEmpty++;
		}
	}

	return AmountEmpty;
}

bool UInventoryComponent::IsEmptyInventory()
{
	for (auto& Slot :GetInventoryItems())
	{
		if (!Slot.IsEmpty()) { return false; }
	}
	return true;
}

bool UInventoryComponent::IsFull()
{
	if (EnableDynamicSizeInventory)
	{
		return false;
	}
	bool HaveSpace;
	int32 Index;

	GetEmptyInventorySpace(HaveSpace, Index);

	return !HaveSpace;
}

UPARAM(DisplayName = "ItemCount")int32 UInventoryComponent::GetInventoryItemCount()
{
	int32 tmpCount = 0;

	for (int32 i = 0; i < Inventory.Num(); i++)
	{
		if (GetItemIsValid(Inventory[i]))
		{
			tmpCount++;
		}
	}
	return UPARAM(DisplayName = "ItemCount")tmpCount;
}

void UInventoryComponent::IncreaseInventorySize(int32 Amount)
{

	Inventory.AddDefaulted(Amount);
}

UPARAM(DisplayName = "Success") bool UInventoryComponent::DecreaseInventorySize(int32 Amount)
{
	Inventory.RemoveAt((Inventory.Num() - Amount), Amount, EAllowShrinking::Yes);

	return UPARAM(DisplayName = "Success") bool();
}

void UInventoryComponent::AutoInventorySize()
{
	int32 TotalMiniSlot = ContainerSlotsPerRow * ContainerMiniRow;
	int32 TotalEmptySlot = 0;
	int32 TotalSlotToChange = 0;


	for (int32 i = Inventory.Num() - 1; i != 0; i--)
	{
		if (!GetItemByRef(i).IsValid())
		{
			TotalEmptySlot++;
		}
		else
		{

			if (TotalEmptySlot < TotalMiniSlot)
			{
				TotalSlotToChange = ContainerSlotsPerRow * FMath::CeilToInt32(float((TotalMiniSlot - TotalEmptySlot) / 5 + 1));
				break;
			}
			else if (TotalEmptySlot > TotalMiniSlot)
			{

				TotalSlotToChange = ContainerSlotsPerRow * FMath::CeilToInt32(float((TotalMiniSlot - TotalEmptySlot) / 5));
				break;
			}
		}
	}

	//Increase Size
	if (TotalSlotToChange > 0)
	{
		UE_LOG(LogInventory, VeryVerbose, TEXT("[UInventoryComponent::AutoInventorySize] Increment:[%d]"), TotalSlotToChange)

		if (IsValid(GetInventoryManager()))
		{
			GetInventoryManager()->Server_IncreaseInventorySize(GetId(), TotalSlotToChange);
		}
	}

	//Decrease Size
	if (TotalSlotToChange < 0)
	{
		if (Inventory.Num() != ContainerSize)
		{
			UE_LOG(LogInventory, VeryVerbose, TEXT("[UInventoryComponent::AutoInventorySize] Decrement:[%d]"), TotalSlotToChange)

			if (IsValid(GetInventoryManager()))
			{
				GetInventoryManager()->Server_DecreaseInventorySize(GetId(), FMath::Abs(TotalSlotToChange));
			}
		}
	}
}

bool UInventoryComponent::CanBeDecremented(int32 Amount)
{
	for (int32 i = (Inventory.Num() - Amount); i < Inventory.Num(); i++)
	{
		if (GetItemIsValid(GetInventoryItem(i)))
		{
			return false; //Item found can remove slot
		}
	}
	
	//no  Item found 
	return true;
}

void UInventoryComponent::LoadInventoryItems(int32 InventorySize, TArray<FItemData> InventoryItems)
{
	//Clear Array
	Inventory.Empty();
	// Set Lenght Inventory
	Inventory.AddDefaulted(InventorySize);

	for (int32 i = 0; i < InventoryItems.Num(); i++)
	{
		// Init Decay if not value 
		if(InventoryItems[i].bCanDecay && InventoryItems[i].Decay == 0)
		{
			InventoryItems[i].Decay = InventoryItems[i].DecayTime;
		}
		InventoryItems[i].NotifyScript_ItemPostLoad(GetInventoryManager(), GetId(), i);
		TArray<int32> FilledIndex;
		SetInventoryItem(i, InventoryItems[i], FilledIndex);
	}
}

FContainerSetting UInventoryComponent::GetContainerSetting()
{
	FContainerSetting OutData;
	OutData.NameContainer = NameContainer;
	OutData.ContainerSize = ContainerSize;
	OutData.ContainerSlotsPerRow = ContainerSlotsPerRow;
	OutData.bIsStorageContainer = !EnableLootContainer;
	OutData.DroppedLocation = DroppedLocation;

	return OutData;
}

TArray<FItemDataInfoCompressed> UInventoryComponent::GetDataForTransfer()
{
	TArray<FItemDataInfoCompressed> Data;

	for (auto& Item : GetInventoryItems())
	{
		Data.Add(UChestoryFunctionLibrary::MakeItemTransfer(Item));
	}

	return Data;
}



void UInventoryComponent::ApplyNewFilter(FGameplayTagContainer Tags)
{
	for(int32 i = 0; i < Inventory.Num(); i++)
	{
		FItemData& itemRef = GetItemByRef(i);
		if(GetItemIsValid(GetItemByRef(i)))
		{
			itemRef.bIsVisible = itemRef.GameplayTagContainer.HasAny(Tags);
		}
				
	}
	CurrentTagsApply = Tags;
	SetUseFilter(true);
	
}

void UInventoryComponent::DisableFilter()
{
	for(int32 i = 0; i < Inventory.Num(); i++)
	{
		FItemData& itemRef = GetItemByRef(i);
		itemRef.bIsVisible = true;
	}

	SetUseFilter(false);
}

void UInventoryComponent::AutoAddTag()
{
	if (GetOwner() != UGameplayStatics::GetPlayerPawn(GetWorld(), 0) && GetOwner() != UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		GetOwner()->Tags.AddUnique(TEXT("HasInventory"));
	}
}

int32 UInventoryComponent::GetIndexAtKey(FName Key)
{
	int32 OutIndex;

	if (SlotsIndexKey.Find(Key, OutIndex))
	{
		return OutIndex;
	}

	return int32();
}

void UInventoryComponent::FindIndexFromRestriction(FGameplayTagContainer CheckRestriction, bool& Success, int32& Index)
{
	Success = false;
	Index = -1;

	if (GetIsEquipmentInventory() || GetIsHotbarInventory())
	{
		if (SlotsRestriction.IsEmpty())
		{
			UE_LOG(LogInventory, Verbose, TEXT("[UInventoryComponent::FindIndexFromRestriction] Slots Restriction Is Empty"));
			return;
		}

		for (auto& Restri : SlotsRestriction)
		{			
			if (CheckRestriction.MatchesQuery(Restri.Value))
			{

				Index = GetIndexAtKey(Restri.Key);			
				//UE_LOG(LogInventory, Verbose, TEXT("[UInventoryComponent::FindIndexFromRestriction] GetIndexAtKey Key:%s  Index:%d Found"), *Restri.Key.ToString(), Index);
				Success = true;
				//If Empty return now else continu to found un empty slot
				if (GetInventoryItem(Index).IsEmpty())
				{
					return;
				}
			}
		}
	}
}


void UInventoryComponent::ResetDecaySlots()
{

	TArray<int32> Keys;
	MDecaySlots.GetKeys(Keys);

	for(auto Key : Keys)
	{
		MDecaySlots.Add(Key,false);
	}
}





