// Copyright 2024 Arkai Interactive. All Rights Reserved.


#include "ItemScript.h"


void UItemScript::Init(UInventoryManagerComponent* InstigatorInventoryManager, FItemData Item)
{
    InventoryManager = InstigatorInventoryManager;
    OwnerItem = Item;
    GetWorld();
}

UWorld* UItemScript::GetWorld() const
{

   UObject* outer;
   outer = InventoryManager;
    if (outer != nullptr)
    {
        if (!outer->HasAnyFlags(RF_ClassDefaultObject))
        {
            //UE_LOG(LogTemp, Warning, TEXT("[ItemScript] Return World InventoryManager"))
            return outer->GetWorld();
        }
    }
  
    //If no inventory manager use InventorySubsystem 
    outer = ObjectForWorld;
    if (outer !=nullptr)
    {
        if (!outer->HasAnyFlags(RF_ClassDefaultObject))
        {
            //UE_LOG(LogTemp,Warning,TEXT("[ItemScript] Return World Subsystem"))
            return outer->GetWorld();
        }

    }

    return nullptr;
}

void UItemScript::Tick(float DeltaTime)
{
    if(this->IsUnreachable())   return;

    if (LastFrameNumberWeTicked == GFrameCounter)
        return;

    if (!IsValid(InventoryManager)) return;
    if (GetWorld() == nullptr) return;
    if(!bEnableTick) return;
    if(!IsValid(this)) return;


    ItemTick(DeltaTime);

    LastFrameNumberWeTicked = GFrameCounter;
}

void UItemScript::BeginDestroy()
{
    bEnableTick = false;
    SetTickableTickType(ETickableTickType::Never);

    Super::BeginDestroy();
}

bool UItemScript::IsItemModifier_Implementation()
{
    return bEnableModifier;
}

bool UItemScript::CanMove_Implementation(UInventoryManagerComponent* InInventoryManager, FGuid FromInventoryId, int32 FromInventorySlot, FGuid ToInventoryId, int32 ToInventorySlot)
{
    return true;
}

bool UItemScript::CanDrop_Implementation(UInventoryManagerComponent* InInventoryManager, FGuid FromInventoryId, int32 FromInventorySlot)
{
    return true;
}

bool UItemScript::CanUnequip_Implementation(UInventoryManagerComponent* InInventoryManager, FGuid FromInventoryId, int32 FromIventorySlot, FGuid InInventoryId, int32 InventorySlot, AActor* ActorEquipment, bool DropRequest)
{
    return true;
}

bool UItemScript::CanEquip_Implementation(UInventoryManagerComponent* InInventoryManager, FGuid FromInventoryId, int32 FromIventorySlot, FGuid ToInventoryId, int32 ToIventorySlot)
{
    return true;
}

bool UItemScript::CanCraft_Implementation(UInventoryManagerComponent* InInventoryManager, FGuid FromCraftingId, FItemDataInfoCraft ItemDataInfoCraft, int32 Amount)
{
    return true;
}

bool UItemScript::CanUse_Implementation(UInventoryManagerComponent* InInventoryManager, FGuid InInventoryId, int32 InIventorySlot, EUsedInputTrigger InputTrigger)
{
    return UsedInputTriggerAuthorised.Contains(InputTrigger);
}

FItemData UItemScript::ItemCreation_Implementation(UInventoryManagerComponent* InInventoryManager, FGuid FromInventoryId, int32 FromIventorySlot, FItemData NewItem)
{
    return NewItem;
}

FItemData UItemScript::ItemPostLoad_Implementation(UInventoryManagerComponent* InInventoryManager, FGuid InventoryId, int32 IventorySlot, FItemData ItemLoad)
{
    return ItemLoad;
}

FItemData UItemScript::ItemPreSave_Implementation(UInventoryManagerComponent* InInventoryManager, FGuid InventoryId, int32 IventorySlot, FItemData ItemSave)
{
    return ItemSave;
}

AActor* UItemScript::GetOwner()
{
    if (!IsValid(InventoryManager))
    {
       return nullptr;
    }
  
    return InventoryManager->GetOwner();
}

void UItemScript::SetTickEnable(bool Enable)
{
    if (Enable && bEnableItemScriptInstance)
    {
        SetTickableTickType(ETickableTickType::Always);
        bEnableTick = true;
        return;
    }

    SetTickableTickType(ETickableTickType::Never);
    bEnableTick = false;
}

bool UItemScript::IsInstanced() const
{
    return GetOuter() != nullptr && GetOuter()->IsA<UItemDataAsset>();
}


