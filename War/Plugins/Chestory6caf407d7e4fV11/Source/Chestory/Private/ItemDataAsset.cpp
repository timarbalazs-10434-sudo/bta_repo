// Copyright 2024 Arkai Interactive. All Rights Reserved.


#include "ItemDataAsset.h"

void UItemDataAsset::PostLoad()
{
	Super::PostLoad();

	Item.ID = GetFName();
}
