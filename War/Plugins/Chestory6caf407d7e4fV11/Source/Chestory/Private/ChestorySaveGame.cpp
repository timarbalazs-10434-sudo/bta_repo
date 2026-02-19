// Copyright 2024 Arkai Interactive. All Rights Reserved.


#include "ChestorySaveGame.h"

void UChestorySaveGame::AddPlayer(FInventorysSaveData AddData)
{
	for (auto&& Saved : SavedInventory)
	{
		if (Saved.InventoryName == AddData.InventoryName)
		{
			Saved.ByteData = AddData.ByteData;
			return;
		}
	}

	SavedInventory.Add(AddData);
}
