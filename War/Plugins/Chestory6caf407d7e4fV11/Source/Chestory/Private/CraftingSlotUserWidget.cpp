// Copyright 2024 Arkai Interactive. All Rights Reserved.


#include "CraftingSlotUserWidget.h"

#include "Chestory.h"
#include "InventorySubsystem.h"

#define LOCTEXT_NAMESPACE "UMG"

#if WITH_EDITOR
// Set Catagory in Widget Blueprint
const FText UCraftingSlotUserWidget::GetPaletteCategory()
{
	return LOCTEXT("Inventory", "Inventory");
}

#endif

void UCraftingSlotUserWidget::UpdateCraftingProgress_Implementation(float CurrentTime)
{
}

void UCraftingSlotUserWidget::CraftingStart_Implementation(float Time)
{
}

void UCraftingSlotUserWidget::CraftingEnd_Implementation()
{
}


void UCraftingSlotUserWidget::UpdateCraftingData_Implementation()
{

	if (IsValid(InventorySubsystem))
	{
		if (ItemInformationCraft.ItemInformation.IsValid())
		{
			FPrimaryAssetId AssetId = InventorySubsystem->GetPrimaryAssetIdFromName(ItemInformationCraft.ItemInformation.ID);
			FItemData ItemDataFromObject = InventorySubsystem->GetItemDataAssetFromPrimaryAssetId(AssetId)->Item;

			ItemInformationCraft.ItemInformation.Icon = ItemDataFromObject.Icon;
			ItemInformationCraft.ItemInformation.Name = ItemDataFromObject.Name;
			ItemInformationCraft.ItemInformation.ItemType = ItemDataFromObject.ItemType;
		}

	}
}

void UCraftingSlotUserWidget::InitializeSubsystem()
{
	if (IsValid(GetOwningPlayer()))
	{
		UGameInstance* GameInstance = GetOwningPlayer()->GetGameInstance();
		if (IsValid(GameInstance))
		{
			InventorySubsystem = GameInstance->GetSubsystem<UInventorySubsystem>();
		}
	}
}

void UCraftingSlotUserWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	InitializeSubsystem();
}

void UCraftingSlotUserWidget::InitCraft(FGuid CraftingId, int32 MySlot, UInventoryManagerComponent* Manger, bool IsWaiting, FItemDataInfoCraft DefaultValue)
{
	OwnerCraftingId = CraftingId;
	IndexSlot = MySlot;
	InventoryManager = Manger;
	bIsWaitingSlot = IsWaiting;
	ItemInformationCraft = DefaultValue;

	UpdateCraftingData();
}

void UCraftingSlotUserWidget::SetCraft(FItemDataInfoCraft InfoCraft)
{
	ItemInformationCraft = InfoCraft;
	UpdateCraftingData();
}

void UCraftingSlotUserWidget::UpdateIngredientNeeded(const TArray<int32>& ItemCraftAmount, int32 MaxAmount)
{
	for (int32 i = 0; i < ItemInformationCraft.Recipe.IngredientNeeded.Num(); i++)
	{
		ItemInformationCraft.Recipe.IngredientNeeded[i].Current = ItemCraftAmount[i];
	}
	ItemInformationCraft.MaxCountCraft = MaxAmount;
	UpdateCraftingData();
}
