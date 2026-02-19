// Copyright 2024 Arkai Interactive. All Rights Reserved.


#include "InventoryPluginStruct.h"
#include "InventoryManagerComponent.h"
#include "ItemScript.h"
#include "Chestory.h"
#include "UObject/Object.h"
#include "Misc/Crc.h"


InventoryPluginStruct::InventoryPluginStruct()
{
}

InventoryPluginStruct::~InventoryPluginStruct()
{
}

//#if UE_BUILD_DEBUG
//uint32 GetTypeHash(const FInventoryID& Thing)
//{
//    uint32 Hash = FCrc::MemCrc32(&Thing, sizeof(FInventoryID));
//    return Hash;
//}
//#endif

FItemData FItemDataSlotSaved::ToItemData() const
{
    FItemData OutItemData;

    OutItemData.ID = ID;
    OutItemData.UniqueId = FGuid(UniqueId);
    OutItemData.GameplayTagContainer.FromExportString(GameplayTagContainerExport);
    OutItemData.Amount = Amount;
    OutItemData.bIsDroppable = bIsDroppable;
    OutItemData.bIsDraggable = bIsDraggable;
    OutItemData.bIsStackable = bIsStackable;
    OutItemData.bIsVisible = bIsVisible;
    OutItemData.Decay = Decay;
    OutItemData.Durability = Durability;

    for (auto att : Attributes.Attributes)
    {
        OutItemData.Attributes.AddAttribute(att.AttributeTag, att.Value);
    }

    OutItemData.Sockets = Sockets;
    OutItemData.bIsFilled = bIsFilled;

    return OutItemData;
}

void FItemData::SetLocationItem(FGuid InventoryId, int32 InventorySlot)
{
    LocationItemInventoryId = InventoryId;
    LocationItemInventoryIndex = InventorySlot;
}

TArray<UItemScript*> FItemData::GetItemScriptCombined(UInventoryManagerComponent* InventoryManager, bool IncludeGeneralScript/* = true*/)
{
    TArray<UItemScript*> ItemScripts = ItemScriptInstance;
    ItemScripts.Append(InventoryManager->GetItemScripts(ID, IncludeGeneralScript,true));
    return ItemScripts;
}

void FItemData::NotifyScript_ItemAdd(UInventoryManagerComponent* InventoryManager, FGuid InInventoryId, int32 IndexSlot, UInventoryComponent* InInventory)
{
    
    if (IsEmpty() || InventoryManager == nullptr) return; 

    for (UItemScript* Script : GetItemScriptCombined(InventoryManager))
    {
        if (Script)
        {
            Script->Init(InventoryManager, *this);
            Script->ItemAddInInventory(InventoryManager, InInventoryId, IndexSlot, InInventory);
        }
    }
}

void FItemData::NotifyScript_ItemRemove(UInventoryManagerComponent* InventoryManager, FGuid FromInventoryId, int32 IndexSlot, UInventoryComponent* FromInventory)
{
    if (IsEmpty() || InventoryManager == nullptr) return;

    for (UItemScript* Script : GetItemScriptCombined(InventoryManager))
    {
        if (Script)
        {
            Script->Init(InventoryManager, *this);
            Script->ItemRemoveInInventory(InventoryManager, FromInventoryId, IndexSlot, FromInventory);
        }
    }
}

void FItemData::NotifyScript_ItemUsed(UInventoryManagerComponent* InventoryManager, FGuid InventoryId, int32 IndexSlot, EUsedInputTrigger InputTrigger)
{
    if (IsEmpty() || InventoryManager == nullptr) return;

    if (!InventoryManager->GetInvSubsystem()) return;

 /*   if (NotifyScript_GetCanUse(InventoryManager,InventoryId,IndexSlot,InputTrigger))
    {

       
    }*/

    for (UItemScript* Script : GetItemScriptCombined(InventoryManager))
    {
        if (Script)
        {
            Script->Init(InventoryManager, *this);

            if (!Script->CanUse(InventoryManager, InventoryId, IndexSlot, InputTrigger)) continue;

            if (Script->UsedInputTriggerAuthorised.Contains(InputTrigger))
            {

                Script->ItemUsed(InventoryManager, InventoryId, IndexSlot, InputTrigger);
            }
        }
    }


    if (UInventoryComponent* OnInventory = InventoryManager->GetInvSubsystem()->GetInventory(InventoryId); OnInventory)
    {
        for (const auto Viewer : OnInventory->GetViewers())
        {
            Viewer.Key->Client_OnItemUsed(InventoryId, Viewer.Value, IndexSlot, InputTrigger);
        }
    }
}

void FItemData::NotifyScript_ItemEquip(UInventoryManagerComponent* InventoryManager, FGuid InInventoryId, int32 IndexSlot)
{
    if (IsEmpty() || InventoryManager == nullptr) return;

    for (UItemScript* Script : GetItemScriptCombined(InventoryManager))
    {
        if (Script)
        {
            Script->Init(InventoryManager, *this);
            Script->ItemEquip(InventoryManager, InInventoryId, IndexSlot);
        }
    }

}

void FItemData::NotifyScript_ItemUnequip(UInventoryManagerComponent* InventoryManager, FGuid InInventoryId, int32 IndexSlot)
{
    if (IsEmpty() || InventoryManager == nullptr) return;

    for (UItemScript* Script : GetItemScriptCombined(InventoryManager))
    {
        if (Script)
        {
            Script->Init(InventoryManager, *this);
            Script->ItemUnequip(InventoryManager, InInventoryId, IndexSlot);
        }
    }
}

void FItemData::NotifyScript_ItemCreated(UInventoryManagerComponent* InventoryManager, FGuid InInventoryId, int32 IndexSlot, UInventoryComponent* InInventory)
{
    if (IsEmpty() || InventoryManager == nullptr) return;

    for (UItemScript* Script : GetItemScriptCombined(InventoryManager))
    {
        if (Script)
        {
            Script->Init(InventoryManager, *this);
            Script->ItemCreated(InventoryManager, InInventoryId, IndexSlot, InInventory);
        }
    }
}

void FItemData::NotifyScript_ItemDropped(UInventoryManagerComponent* InventoryManager, AActor* ActorDropped, FGuid FromInventoryId, int32 FromSlot)
{
    if (IsEmpty() || InventoryManager == nullptr) return;

    for (UItemScript* Script : GetItemScriptCombined(InventoryManager))
    {
        if (Script)
        {
            Script->Init(InventoryManager, *this);
            Script->ItemDropped(InventoryManager, ActorDropped, FromInventoryId, FromSlot);
        }
    }
}

void FItemData::NotifyScript_ItemModifier(UInventoryManagerComponent* InventoryManager, FGuid FromInventoryId, int32 FromInventorySlot, FGuid ToInventoryId, int32 ToInventorySlot)
{
    if (IsEmpty() || InventoryManager == nullptr) return;

    for (UItemScript* Script : GetItemScriptCombined(InventoryManager))
    {
        if (Script)
        {
            Script->Init(InventoryManager, *this);
            Script->ItemModifier(InventoryManager, FromInventoryId, FromInventorySlot, ToInventoryId, ToInventorySlot);
        }
    }
}

void FItemData::NotifyScript_SocketItemAdded(UInventoryManagerComponent* InventoryManager, FGuid InInventoryId, int32 InSlot, FSocketItem NewSocket, int32 InSocketSlot)
{
    if (IsEmpty() || InventoryManager == nullptr) return;

    for (UItemScript* Script : GetItemScriptCombined(InventoryManager))
    {
        if (Script)
        {
            Script->Init(InventoryManager, *this);
            Script->SocketItemAdded(InventoryManager, InInventoryId, InSlot, NewSocket, InSocketSlot);
        }
    }
}

void FItemData::NotifyScript_SocketItemRemoved(UInventoryManagerComponent* InventoryManager, FGuid FromInventoryId, int32 FromSlot, FSocketItem RemovedSocket, int32 FromSocketSlot)
{
    if (IsEmpty() || InventoryManager == nullptr) return;

    for (UItemScript* Script : GetItemScriptCombined(InventoryManager))
    {
        if (Script)
        {
            Script->Init(InventoryManager, *this);
            Script->SocketItemRemoved(InventoryManager, FromInventoryId, FromSlot, RemovedSocket, FromSocketSlot);
        }
    }
}

bool FItemData::NotifyScript_GetIsItemModifier(UInventoryManagerComponent* InventoryManager)
{
    if (IsEmpty() || InventoryManager == nullptr) return false;

    for (UItemScript* Script : GetItemScriptCombined(InventoryManager,false)) //Exclude General ItemScript for IsModifier
    {
        if (Script)
        {
            Script->Init(InventoryManager, *this);
            if (Script->IsItemModifier())
            {
                return true;
            }
        }
    }
    return false;
}

bool FItemData::NotifyScript_GetCanMove(UInventoryManagerComponent* InventoryManager, FGuid FromInventoryId, int32 FromInventorySlot, FGuid ToInventoryId, int32 ToInventorySlot)
{
    if (IsEmpty() || InventoryManager == nullptr) return true;

    for (UItemScript* Script : GetItemScriptCombined(InventoryManager))
    {
        if (Script)
        {
            Script->Init(InventoryManager, *this);
            if (!Script->CanMove(InventoryManager, FromInventoryId, FromInventorySlot, ToInventoryId, ToInventorySlot))
            {
                UE_LOG(LogInventory, Display, TEXT("[NotifyScript_GetCanMove] Item:[%s] ItemScript:[%s] return false"), *ID.ToString(), *Script->GetFName().ToString());
                return false;
            }
        }
    }

    return true;
}

bool FItemData::NotifyScript_GetCanDrop(UInventoryManagerComponent* InventoryManager, FGuid FromInventoryId, int32 FromInventorySlot)
{
    if (IsEmpty() || InventoryManager == nullptr) return true;

    for (UItemScript* Script : GetItemScriptCombined(InventoryManager))
    {
        if (Script)
        {
            Script->Init(InventoryManager, *this);
            if (!Script->CanDrop(InventoryManager, FromInventoryId, FromInventorySlot))
            {
                UE_LOG(LogInventory, Display, TEXT("[NotifyScript_GetCanDrop] Item:[%s] ItemScript:[%s] return false"), *ID.ToString(), *Script->GetFName().ToString());
                return false;
            }
        }
    }

    return true;
}

bool FItemData::NotifyScript_GetCanUnequip(UInventoryManagerComponent* InventoryManager, FGuid FromInventoryId, int32 FromIventorySlot, FGuid InInventoryId, int32 IndexSlot, AActor* InActorEquipment, bool DropRequest)
{
    if (IsEmpty() || InventoryManager == nullptr) return true;


    for (UItemScript* Script : GetItemScriptCombined(InventoryManager))
    {
        if (Script)
        {
            Script->Init(InventoryManager, *this);
            if (!Script->CanUnequip(InventoryManager, FromInventoryId, FromIventorySlot, InInventoryId, IndexSlot, InActorEquipment, DropRequest))
            {
                return false;
            }
        }
    }

    return true;
}

bool FItemData::NotifyScript_GetCanEquip(UInventoryManagerComponent* InventoryManager, FGuid FromInventoryId, int32 FromInventorySlot, FGuid ToInventoryId, int32 ToInventorySlot)
{
    if (IsEmpty() || InventoryManager == nullptr) return true;

    for (UItemScript* Script : GetItemScriptCombined(InventoryManager))
    {
        if (Script)
        {
            Script->Init(InventoryManager, *this);
            if (!Script->CanEquip(InventoryManager, FromInventoryId, FromInventorySlot, ToInventoryId, ToInventorySlot))
            {
                UE_LOG(LogInventory, Display, TEXT("[NotifyScript_GetCanEquip] Item:[%s] ItemScript:[%s] return false"), *ID.ToString(), *Script->GetFName().ToString());
                return false;
            }
        }
    }

    return true;
}

bool FItemData::NotifyScript_GetCanCraft(UInventoryManagerComponent* InventoryManager, FGuid FromCraftingId, FItemDataInfoCraft ItemDataInfoCraft, int32 InAmount)
{
    if (IsEmpty() || InventoryManager == nullptr) return true;


    for (UItemScript* Script : GetItemScriptCombined(InventoryManager))
    {
        if (Script)
        {
            Script->Init(InventoryManager, *this);
            if (!Script->CanCraft(InventoryManager, FromCraftingId, ItemDataInfoCraft, InAmount))
            {
                UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::GetCanCraft] Item:[%s] ItemScript:[%s] return false"), *ID.ToString(), *Script->GetFName().ToString());
                return false;
            }
        }
    }

    return true;
}

bool FItemData::NotifyScript_GetCanUse(UInventoryManagerComponent* InventoryManager, FGuid InInventoryId, int32 InInventorySlot, EUsedInputTrigger InputTrigger)
{
    if (IsEmpty() || InventoryManager == nullptr) return true;

    bool bUsedSuccess = true;

    for (UItemScript* Script : GetItemScriptCombined(InventoryManager,false))
    {
        if (Script)
        {
            if (!Script->UsedInputTriggerAuthorised.Contains(InputTrigger))
            {
                bUsedSuccess = false;
                UE_LOG(LogInventory, Display, TEXT("[NotifyScript_GetCanUse] Reject ItemUse Item:[%s] ItemScript:[%s] InputTrigger is not Authorised"), *ID.ToString(), *Script->GetFName().ToString());
                //return bUsedSuccess;
            }

            Script->Init(InventoryManager, *this);
            if (!Script->CanUse(InventoryManager, InInventoryId, InInventorySlot, InputTrigger))
            {
                bUsedSuccess = false;
                UE_LOG(LogInventory, Display, TEXT("[NotifyScript_GetCanUse] Reject ItemUse Item:[%s] ItemScript:[%s] "), *ID.ToString(), *Script->GetFName().ToString());
            }
        }
    }
    return bUsedSuccess;
}

FItemData FItemData::NotifyScript_ItemCreation(UInventoryManagerComponent* InventoryManager, FGuid FromInventoryId, int32 FromIventorySlot)
{
    if (IsEmpty() || InventoryManager == nullptr) return FItemData();

    //Get Data
    FItemData OutItemData = *this;
    for (UItemScript* Script : GetItemScriptCombined(InventoryManager))
    {
        if (Script)
        {
            Script->Init(InventoryManager, *this);
            OutItemData = Script->ItemCreation(InventoryManager, FromInventoryId, FromIventorySlot, OutItemData);
        }
    }
    return OutItemData;
}

FItemData FItemData::NotifyScript_ItemPostLoad(UInventoryManagerComponent* InventoryManager, FGuid FromInventoryId, int32 FromIventorySlot)
{
    if (!IsValid() || InventoryManager == nullptr) return FItemData();

    for (UItemScript* Script : GetItemScriptCombined(InventoryManager))
    {
        if (Script)
        {
            Script->Init(InventoryManager, *this);
            *this = Script->ItemPostLoad(InventoryManager, FromInventoryId, FromIventorySlot, *this);
        }
    }
    return  *this;
}

FItemData FItemData::NotifyScript_ItemPreSave(UInventoryManagerComponent* InventoryManager, FGuid FromInventoryId, int32 FromIventorySlot)
{
    if (!IsValid() || InventoryManager == nullptr) return FItemData();

  
    for (UItemScript* Script : GetItemScriptCombined(InventoryManager))
    {
        if (Script)
        {
            Script->Init(InventoryManager, *this);
            *this = Script->ItemPreSave(InventoryManager, FromInventoryId, FromIventorySlot, *this);
        }
    }
    return *this;
}

void FItemData::NotifyScript_InventoryOpen(UInventoryManagerComponent* InventoryManager, FGuid InInventoryId, int32 InventorySlot)
{
    if (IsEmpty() || InventoryManager == nullptr) return;

    for (UItemScript* Script : GetItemScriptCombined(InventoryManager))
    {
        if (Script)
        {
            Script->Init(InventoryManager, *this);
            Script->OwnerInventoryOpen(InventoryManager, InInventoryId, InventorySlot);
        }
    }
}

void FItemData::NotifyScript_InventoryClose(UInventoryManagerComponent* InventoryManager, FGuid InInventoryId, int32 InventorySlot)
{
    if (IsEmpty() || InventoryManager == nullptr) return;

    for (UItemScript* Script : GetItemScriptCombined(InventoryManager))
    {
        if (Script)
        {
            Script->Init(InventoryManager, *this);
            Script->OwnerInventoryClose(InventoryManager, InInventoryId, InventorySlot);
        }
    }
}

UItemScript* FItemData::GetItemScriptInstanceFromClass(TSubclassOf<UItemScript> ClassItemScript)
{
    if (ItemScriptInstance.IsEmpty())
    {
        return nullptr;
    }

    for (auto* InstanceScript : ItemScriptInstance)
    {
        if (!InstanceScript)
        {
            continue;
        }

        if (InstanceScript->GetClass() == ClassItemScript) return InstanceScript;
    }

    return nullptr;
}

bool FItemData::ContaintInstanceScriptClass(UItemScript* ItemScript)
{  
    return (GetItemScriptInstanceFromClass(ItemScript->GetClass()) != nullptr);
}

FInventorysSaveData FItemData::GetDataBytesActorByName(FName NameId)
{
    for (auto Save : DataBytesActor)
    {
        if (Save.InventoryName == NameId) return Save;
    }
    return FInventorysSaveData();
}

void FSocketItem::ClearSocket()
{
    PrimaryAssetId = FPrimaryAssetId();
    Attributes = FGameplayTagAttributeContainer();
    SocketTiny = FItemTiny();
}
