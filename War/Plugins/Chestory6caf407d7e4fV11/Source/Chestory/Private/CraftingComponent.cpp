// Copyright 2024 Arkai Interactive. All Rights Reserved.


#include "CraftingComponent.h"
#include "InventorySubsystem.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetMathLibrary.h"
#include "ChestoryFunctionLibrary.h"
// Sets default values for this component's properties
UCraftingComponent::UCraftingComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	SetNetAddressable();
	SetIsReplicated(true);
	// ...

}

UCraftingComponent::UCraftingComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
	SetIsReplicated(true);
}

void UCraftingComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UCraftingComponent, CraftingID, COND_InitialOnly);
	//DOREPLIFETIME(UInventoryComponent, InventoryID);
 
	DOREPLIFETIME(UCraftingComponent, InventoryIdLinked);
	DOREPLIFETIME(UCraftingComponent, CraftingFactor);
	DOREPLIFETIME(UCraftingComponent, CraftingTickTime);

}

// Called when the game starts
void UCraftingComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	InitializeSubsystem();

	if (!IsValid(InventorySubsystem)) return;

	//Server Instance
	if (GetOwner()->HasAuthority())
	{

		if (!CraftingID.IsValid())
		{
			Server_GenerateId();
		}

		if (TryToResolveInventory())
		{
			InventoryIdLinked = CraftingInventory->GetId();
			CraftingInventory->CraftingIdLinked = CraftingID;
			UE_LOG(LogInventory, Display, TEXT("[UCraftingComponent::BeginPlay] Success to linked Invntory with Owner:[%s] InventoryId:[%s]"), *GetOwner()->GetFName().ToString(), *InventoryIdLinked.ToString())

			//Detect Change Data
			CraftingInventory->OnItemEnterInventory.AddDynamic(this, &UCraftingComponent::OnOwerInventoryDataChanged);
			CraftingInventory->OnItemLeaveInventory.AddDynamic(this, &UCraftingComponent::OnOwerInventoryDataChanged);
			
		}
	}

	//Register Crafting Component
	if (CraftingID.IsValid())
	{
		InventorySubsystem->RegisterCrafting(CraftingID, this);
	}
}

void UCraftingComponent::OnComponentCreated()
{
	Super::OnComponentCreated();

	if (GetOwner()->HasAuthority())
	{
		//Server: empty from save create a new Id 
		if (!CraftingID.IsValid())
		{
			//Server Generate a New ID
			CraftingID = FGuid::NewGuid();
		}
	}

	
}

void UCraftingComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	Super::OnComponentDestroyed(bDestroyingHierarchy);

	//Maybe check autority
	if (IsValid(InventorySubsystem) && CraftingID.IsValid())
	{
		InventorySubsystem->UnregisterCrafting(CraftingID);
	}
}

void UCraftingComponent::PostLoad()
{
	Super::PostLoad();

	//Transfer Old Data to New Data
	if (DefaultItemsCrafts.IsEmpty() && !BlueprintCrafts.IsEmpty())
	{
		for (auto Blueprint : BlueprintCrafts)
		{
			FPrimaryAssetId NewFPrimaryAssetId = FPrimaryAssetId("ItemDataAsset", Blueprint->Item.ID);
			if (NewFPrimaryAssetId.IsValid())
			{
				DefaultItemsCrafts.Add(NewFPrimaryAssetId);
			}
		}
	}
}


// Called every frame
void UCraftingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UCraftingComponent::OnObjectPreLoad_Implementation()
{
	if (CraftingID.IsValid())
	{
		if (!IsValid(InventorySubsystem))
		{
			InitializeSubsystem();
		}

		InventorySubsystem->UnregisterCrafting(CraftingID);
	}
}

void UCraftingComponent::OnObjectPostLoad_Implementation()
{
	if (GetOwner()->HasAuthority())
	{

		//Server: empty from save create a new Id 
		if (!CraftingID.IsValid())
		{
			//Server Generate a New ID
			Server_GenerateId();
			UE_LOG(LogInventory, Display, TEXT("[UCraftingComponent::OnObjectPostLoad_Implementation] GenerateId Owner:[%s] Id:[%s]"), *GetOwner()->GetFName().ToString(), *CraftingID.ToString())

		}
		else
		{
			Multicast_UpdateId(CraftingID);
		}


		//Try Restart Crafting
		OnCraftingLoaded();
	}
}


void UCraftingComponent::OnCraftingLoaded()
{
	if (GetWaitList().IsEmpty())
	{
		return;
	}
	if (!IsValid(InventorySubsystem))
	{
		InitializeSubsystem();
	}

	if (!InventorySubsystem->bItemsCompletedLoad)
	{
		InventorySubsystem->OnLoadingStoreItemsCompleted.AddDynamic(this, &UCraftingComponent::OnCraftingLoaded);
		return;
	}

	//try to restart Crafting
	StartCraftingTimer();
}

void UCraftingComponent::Server_GenerateId_Implementation()
{
	CraftingID = FGuid::NewGuid();
	UE_LOG(LogInventory, VeryVerbose, TEXT("[UCraftingComponent::Server_GenerateId] Server : [%s] NEW Crafting ID : [%s]"), *FString(GetOwner()->GetFName().ToString() + "." + GetFName().ToString()), *CraftingID.ToString());
	OnRep_CraftingID();
}

void UCraftingComponent::Server_SetId_Implementation(FGuid Id)
{
	CraftingID = Id;
	UE_LOG(LogInventory, VeryVerbose, TEXT("[UCraftingComponent::Server_SetId] Server : [%s] Set Crafting ID : [%s]"), *FString(GetOwner()->GetFName().ToString() + "." + GetFName().ToString()), *CraftingID.ToString());
	InventorySubsystem->RegisterCrafting(CraftingID, this);
	OnRep_CraftingID();
}

void UCraftingComponent::Multicast_UpdateId_Implementation(FGuid Id)
{
	CraftingID = Id;
	InventorySubsystem->RegisterCrafting(CraftingID, this);

	if (IsValid(CraftingInventory))
	{
		CraftingInventory->CraftingIdLinked = CraftingID;
	}

}

void UCraftingComponent::OnRep_CraftingID()
{
	if (!IsValid(InventorySubsystem))
	{
		InitializeSubsystem();
	}

	FString Net = GetOwner()->HasAuthority() ? FString("Server") : FString("Client");

	InventorySubsystem->RegisterCrafting(CraftingID, this);
	UE_LOG(LogInventory, Verbose, TEXT("[UCraftingComponent::OnRep_CraftingID] OnRep net :[%s] Owner:[%s] Inventory ID:[%s]"), *Net, *FString(GetOwner()->GetFName().ToString() + "." + GetFName().ToString()), *CraftingID.ToString());


}



void UCraftingComponent::InitializeSubsystem()
{
	const UGameInstance* GameInstance = GetOwner()->GetGameInstance();
	if (IsValid(GameInstance))
	{
		InventorySubsystem = GameInstance->GetSubsystem<UInventorySubsystem>();
	}
}

void UCraftingComponent::SetDynamicsCrafts(TArray<FPrimaryAssetId> Crafts)
{
	Server_SetDynamicsCraft(Crafts);
}

UInventoryManagerComponent* UCraftingComponent::GetInventoryManager()
{
	if (HasInventoryManager())
	{
		return GetOwnerManager();
	}

	return InventorySubsystem->GetInventoryManagerInstance();
}





TArray<FItemDataInfoCraftCompressed> UCraftingComponent::GetDataForTransfer()
{
	TArray<FItemDataInfoCraftCompressed> OutData;

	if(!IsValid(GetInventorySubsystem())) 	return OutData;

	for (const auto Craft : GetCombinedItemsCraft())
	{
		UItemDataAsset* DataAsset = GetInventorySubsystem()->GetItemDataAssetFromPrimaryAssetId(Craft);

		if (IsValid(DataAsset))
		{
			// is Valid add in craft list
			if (UChestoryFunctionLibrary::IsValidRecipe(DataAsset, true))
			{
				//Apply data
				FItemDataInfoCraft InfoCraft = UChestoryFunctionLibrary::MakeItemInfoCraft(this, DataAsset);
				TArray<int32> OutRecipe;
				GetInventoryManager()->ApplyAdditionalCraftData(InventoryIdLinked, InfoCraft, OutRecipe);

				OutData.Add(UChestoryFunctionLibrary::MakeCraftTransfer(InfoCraft));
			}
		}
	}

	//for (int32 i = 0; i < BlueprintCrafts.Num(); i++)
	//{
	//	// is Valid add in craft list
	//	if (UChestoryFunctionLibrary::IsValidRecipe(BlueprintCrafts[i], true))
	//	{
	//		//Apply data
	//		FItemDataInfoCraft InfoCraft = UChestoryFunctionLibrary::MakeItemInfoCraft(this, BlueprintCrafts[i]);
	//		TArray<int32> OutRecipe;
	//		GetInventoryManager()->ApplyAdditionalCraftData(InventoryIdLinked, InfoCraft, OutRecipe);

	//		OutData.Add(UChestoryFunctionLibrary::MakeCraftTransfer(InfoCraft));
	//	}
	//}

	return OutData;
}

TArray<FItemDataInfoCraftCompressed> UCraftingComponent::GetDataWaitingForTransfer()
{
	TArray<FItemDataInfoCraftCompressed> OutData;

	for (auto& Wait : WaitList)
	{
		OutData.Add(UChestoryFunctionLibrary::MakeCraftTransfer(Wait));
	}

	return OutData;
}

bool UCraftingComponent::FindIndexWaitListSameItem(int32 &IndexFound,FItemDataInfoCraft WaitListItem)
{
	IndexFound = -1;
	for (auto &SlotWait : WaitList)
	{
		IndexFound++;
		if (SlotWait.ItemInformation.ID == WaitListItem.ItemInformation.ID)
		{
			return true;
		}
	}

	IndexFound = -1;
	return false;
}

void UCraftingComponent::CheckFailureCarft(bool& Failure, FPrimaryAssetId& PrimaryAssetId, FItemDataInfoCraft Craft)
{

	Failure = false;
	if(Craft.Recipe.bCanFailure)
	{
		Failure = UKismetMathLibrary::RandomBoolWithWeight(Craft.Recipe.PercentFailure / 100);
	}

	if (Failure)
	{
		PrimaryAssetId = Craft.Recipe.PrimaryAssetIdFailure;
	}
	else
	{
		PrimaryAssetId = Craft.Recipe.ReturnSelf ? InventorySubsystem->GetPrimaryAssetIdFromName(Craft.ItemInformation.ID) : Craft.Recipe.PrimaryAssetIdReturn;
	}

	return;	
}


void UCraftingComponent::Server_RemoveWaitListItem_Implementation(int32 IndexSlot)
{
	WaitList.RemoveAt(IndexSlot);
	OnUpdateWaitingList.Broadcast("None", -1);
}

void UCraftingComponent::StartCraftingTimer_Implementation()
{
	if (GetWorld()->GetTimerManager().IsTimerActive(TimerCrafting)) // Check si pas déjà en coure de craft
	{
		return;
	}

	//Ensure no ressrouce
	CraftInfo.bHasResources = false;

	//bHaveRessourceForCraft = false;
	for (int32 i = 0; i < WaitList.Num(); i++)
	{
		if (CanStartCrafting(WaitList[i]))
		{
			CraftInfo = FCraftingInfo(WaitList[i], i, true, WaitList[i].bIsRepair, CraftInfo.CraftingCurrentTime);
			CraftInfo.ItemInfo.InventoryId = CraftingInventory->GetId();

			GetInventoryManager()->RemoveIngredientNeededInInventory(CraftingInventory, WaitList[i].Recipe);

			// Call Event Server 
			StartCrafting.Broadcast(CraftInfo.ItemInfo);
			//Server Send data to Client
			GetInventoryManager()->StartViewersWaitingSlot(CraftingID, CraftingInventory->GetViewers(), CraftInfo.WaitListIndex, CraftInfo.ItemInfo.Recipe.TimeToCraft);
			GetWorld()->GetTimerManager().SetTimer(TimerCrafting, this, &UCraftingComponent::CurrentCraftingTimer, CraftingTickTime, true, -1.0f);

			break;
		}
	}
}

void UCraftingComponent::CurrentCraftingTimer_Implementation()
{
	CraftInfo.CraftingCurrentTime += GetWorld()->GetTimerManager().GetTimerElapsed(TimerCrafting) * CraftingFactor;
	CurrentCrafting.Broadcast(CraftInfo.CraftingCurrentTime);
	GetInventoryManager()->UpdateProgressViewersWaitingSlot(CraftingID, CraftingInventory->GetViewers(), CraftInfo.WaitListIndex, CraftInfo.CraftingCurrentTime);
	
	if (IsFinishedCrafting())
	{
		ClearTimerCrafting();
		EndCraftingTimer();
	}
}

void UCraftingComponent::EndCraftingTimer_Implementation()
{

	if (!IsValid(GetInventoryManager()))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UCraftingComponent::EndCraftingTimer] InventoryManager Invalid"));
		return;
	}

	if (CraftInfo.bIsRepair)
	{
		HandleRepairSuccess();
	}
	else
	{
		HandleCraftingSuccess();
	}

	UpdateWaitList();
}





bool UCraftingComponent::CanStartCrafting(const FItemDataInfoCraft& CraftItem)
{
	if (!IsValid(GetInventoryManager()) || !IsValid(CraftingInventory))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UCraftingComponent::CanStartCrafting] Inventory Manager or Crafting Inventory is invalid."));
		return false;
	}

	// Check Inventory Space
	if (CraftingInventory->GetAmountEmptyInventorySpace() < CraftItem.Recipe.ItemReturnAmount)
	{
		UE_LOG(LogInventory, Warning, TEXT("[UCraftingComponent::CanStartCrafting] Not enough space in inventory for crafting."));
		return false;
	}

	// Check inventory ressrouce
	if (!GetInventoryManager()->CheckContainerHasItemCraft(CraftItem.Recipe.IngredientNeeded, CraftingInventory))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UCraftingComponent::CanStartCrafting] Not enough resources for crafting."));
		return false;
	}

	return true;
}

bool UCraftingComponent::IsFinishedCrafting()
{
	return CraftInfo.CraftingCurrentTime >= CraftInfo.ItemInfo.Recipe.TimeToCraft;
}

void UCraftingComponent::ClearTimerCrafting()
{
	GetWorld()->GetTimerManager().ClearTimer(TimerCrafting);
	CraftInfo.CraftingCurrentTime = 0;
}

void UCraftingComponent::HandleCraftingSuccess()
{
	bool bIsFailure = false;
	FPrimaryAssetId RealPrimaryAssetId;
	CheckFailureCarft(bIsFailure, RealPrimaryAssetId, CraftInfo.ItemInfo);

	int32 AmountItem = CraftInfo.ItemInfo.Recipe.ItemReturnAmount;
	FItemData ItemAdd;
	ItemAdd.Amount = AmountItem;

	bool bSuccessAdd;
	int32 IndexOut;
	GetInventoryManager()->TryToAddItemToInventory(bSuccessAdd, ItemAdd, IndexOut, CraftingInventory, RealPrimaryAssetId, AmountItem, true);

	// Server
	EndCrafting.Broadcast(ItemAdd, bIsFailure, false, FItemDataInfoCraft(), CraftInfo.ItemInfo.InventoryId, IndexOut);
}

void UCraftingComponent::HandleRepairSuccess()
{
	if (IsValid(GetInventoryManager()))
	{
		GetInventoryManager()->SetDurability(CraftingInventory->InventoryID, CraftInfo.ItemInfo.InventorySlot, false, CraftInfo.ItemInfo.ItemInformation.MaxDurability);
		GetInventoryManager()->SetIsDraggable(CraftingInventory->InventoryID, CraftInfo.ItemInfo.InventorySlot, true);
	}

	// Server
	EndCrafting.Broadcast(FItemData(), false, true, CraftInfo.ItemInfo, CraftInfo.ItemInfo.InventoryId, CraftInfo.ItemInfo.InventorySlot);
}


void UCraftingComponent::UpdateWaitList()
{
	GetInventoryManager()->EndViewersWaitingSlot(CraftingID, CraftingInventory->GetViewers(), CraftInfo.WaitListIndex);

	WaitList[CraftInfo.WaitListIndex].ItemInformation.Amount -= 1;
	CraftInfo.ItemInfo = WaitList[CraftInfo.WaitListIndex];
	OnUpdateWaitingList.Broadcast(CraftInfo.ItemInfo.ItemInformation.ID, CraftInfo.ItemInfo.ItemInformation.Amount);

	if (WaitList[CraftInfo.WaitListIndex].ItemInformation.Amount <= 0)
	{
		GetInventoryManager()->Server_RemoveCraftInWaitList(CraftingID, CraftInfo.WaitListIndex);
	}
	else if (bCallCancelCraft)
	{
		GetInventoryManager()->Server_RemoveCraftInWaitList(CraftingID, CraftInfo.WaitListIndex);
	}
	else
	{
		GetInventoryManager()->SetViewersWaitingSlot(CraftingID, CraftingInventory->GetViewers(), CraftInfo.WaitListIndex, UChestoryFunctionLibrary::MakeCraftTransfer(CraftInfo.ItemInfo));
	}

	// Chack hace craft
	if (WaitList.Num() > 0)
	{
		StartCraftingTimer();
	}
}


void UCraftingComponent::OnOwerInventoryDataChanged(FPrimaryAssetId PrimaryAssetId, int32 Amount)
{
	if (!IsValid(CraftingInventory))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UCraftingComponent::OnOwerInventoryDataChanged] CraftingInventory Invalid"));
		return;
	}


	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	if (CraftingInventory->HasViwers())
	{
		if (IsValid(GetInventoryManager()))
		{

			GetInventoryManager()->Server_UpdateDataCraft(CraftingID, true);

		}
	}

	//Has item in waitlist but can't craft have no ressource befor this update, then try to restart Crafting
	if (!GetWaitList().IsEmpty() && !GetIsCurrentCrafting() && !CraftInfo.bHasResources)
	{
		StartCraftingTimer();
	}
}




TArray<FPrimaryAssetId> UCraftingComponent::GetCombinedItemsCraft()
{

	TArray<FPrimaryAssetId> Result;
	TArray<FPrimaryAssetId> Result2;

	TotalItemsCrafts.Empty();

	for (const auto Craft : DefaultItemsCrafts)
	{
		TotalItemsCrafts.AddUnique(Craft);
	}

	if (!DynamicsItemsCrafts.IsEmpty())
	{
		for (const auto Craft : DynamicsItemsCrafts)
		{
			TotalItemsCrafts.AddUnique(Craft);
		}
	}

	if (IsValid(GetInventorySubsystem()))
	{
		for (auto const Craft : GetInventorySubsystem()->FindItemsWithCraftingTag(ItemsCraftingTags))
		{
			TotalItemsCrafts.AddUnique(Craft);
		}

		for (auto const Craft : GetInventorySubsystem()->FindItemsWithItemsGameplayTags(ItemsGameplayTags))
		{
			TotalItemsCrafts.AddUnique(Craft);
		}
	}


	return TotalItemsCrafts;
}

UInventoryManagerComponent* UCraftingComponent::GetOwnerManager()
{
	if (!IsValid(GetOwner()))
	{
		return nullptr;
	}

	TArray<UInventoryManagerComponent*> InventoryManagers;
	GetOwner()->GetComponents(InventoryManagers);

	if (!InventoryManagers.IsEmpty() && IsValid(InventoryManagers[0]))
	{
		return InventoryManagers[0];

	}
	else
	{
		return nullptr;
	}
}

bool UCraftingComponent::HasInventoryManager()
{
	return IsValid(GetOwnerManager());
}

void UCraftingComponent::PendingToResolve()
{
	TryToResolveInventory();
}

bool UCraftingComponent::TryToResolveInventory()
{
	//Has Manager
	if (HasInventoryManager())
	{
		if (InventoryPlayerLinked.IsNone())
		{
			UE_LOG(LogInventory, Warning, TEXT("[UCraftingComponent::TryResolveInventory] InventoryPlayerLinked Invalid in Actor:[%s]"), *GetOwner()->GetFName().ToString())
			return false;
		}

		if (!GetOwnerManager()->PlayerInventorys.Contains(InventoryPlayerLinked))
		{
			UE_LOG(LogInventory, Warning, TEXT("[UCraftingComponent::TryResolveInventory] InventoryPlayerLinked:[%s] is't in the Inventorys list in Actor:[%s]"),*InventoryPlayerLinked.ToString(), *GetOwner()->GetFName().ToString())
			return false;
		}

		CraftingInventory = GetOwnerManager()->GetInventory(InventoryPlayerLinked);
		return IsValid(CraftingInventory);
	}
	else
	{
		TArray<UInventoryComponent*> InventorysFound;
		GetOwner()->GetComponents(InventorysFound);

		if (InventorysFound.IsEmpty())
		{
			UE_LOG(LogInventory,Warning,TEXT("[UCraftingComponent::TryResolveInventory] Inventory not found in Actor:[%s]"),*GetOwner()->GetFName().ToString())
			return false;
		}

		CraftingInventory = InventorysFound[0];
		return IsValid(CraftingInventory);

	}
}



void UCraftingComponent::Server_AddCraft_Implementation(FPrimaryAssetId ItemAdd)
{

	if (!IsValid(GetInventorySubsystem()))
	{
		return;
	}

	if (DynamicsItemsCrafts.Contains(ItemAdd)) return;

	if (TotalItemsCrafts.Contains(ItemAdd)) return;

	UItemDataAsset* Asset = GetInventorySubsystem()->GetItemDataAssetFromPrimaryAssetId(ItemAdd);

	if (!IsValid(Asset))
	{
		return;
	}

	// is Valid add in craft list
	if (UChestoryFunctionLibrary::IsValidRecipe(Asset, true))
	{


		//Apply data
		FItemDataInfoCraft InfoCraft = UChestoryFunctionLibrary::MakeItemInfoCraft(this, Asset);
		TArray<int32> OutRecipe;
		GetInventoryManager()->ApplyAdditionalCraftData(InventoryIdLinked, InfoCraft, OutRecipe);

	


		DynamicsItemsCrafts.Add(ItemAdd);

		UInventoryComponent* Inventory = GetInventorySubsystem()->GetInventory(GetInventorIdLinked());
		if (IsValid(Inventory))
		{
			GetInventoryManager()->AddViewersItemCrafting(GetId(), GetCombinedItemsCraft().Num() + 1, Inventory->GetViewers(), UChestoryFunctionLibrary::MakeCraftTransfer(InfoCraft));
		}
	}
}

void UCraftingComponent::Server_RemoveCraft_Implementation(FPrimaryAssetId ItemRemove)
{

	if (!IsValid(GetInventorySubsystem()))
	{
		return;
	}


	if (DynamicsItemsCrafts.Contains(ItemRemove))
	{
		DynamicsItemsCrafts.Remove(ItemRemove);
	}

	int32 IndexToRemove = -1;


	if (TotalItemsCrafts.Contains(ItemRemove))
	{
		TotalItemsCrafts.Find(ItemRemove, IndexToRemove);

		TotalItemsCrafts.Remove(ItemRemove);
	}

	if (IndexToRemove != -1)
	{
		UInventoryComponent* Inventory = GetInventorySubsystem()->GetInventory(GetInventorIdLinked());
		if (IsValid(Inventory))
		{
			GetInventoryManager()->RemoveViewersItemCrafting(GetId(), IndexToRemove, Inventory->GetViewers());
		}
	}
}


void UCraftingComponent::Server_SetDynamicsCraft_Implementation(const TArray<FPrimaryAssetId>& Crafts)
{
	DynamicsItemsCrafts = Crafts;
}






