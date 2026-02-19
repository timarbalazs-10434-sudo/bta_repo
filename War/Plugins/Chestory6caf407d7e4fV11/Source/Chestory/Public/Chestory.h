// Copyright 2024 Arkai Interactive. All Rights Reserved.

#pragma once
#include "ItemDataAsset.h"
#include "Modules/ModuleManager.h"
#include "Engine/GameInstance.h"

DECLARE_LOG_CATEGORY_EXTERN(LogInventory, Log, All);

class FChestoryModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

//develop branch test 5

//Je veut cette modfi sur tout les version
};
