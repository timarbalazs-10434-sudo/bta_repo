// Copyright 2024 Arkai Interactive. All Rights Reserved.

#include "Chestory.h"
#include "Misc/Paths.h"
#include "Engine/AssetManagerSettings.h"

DEFINE_LOG_CATEGORY(LogInventory);

#define LOCTEXT_NAMESPACE "FChestoryModule"



//const FPrimaryAssetType	FChestoryModule::ItemDataType = TEXT("ItemDataAsset");
//const UClass* FChestoryModule::ClassDataAsset = UItemDataAsset::StaticClass();

void FChestoryModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FChestoryModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	
}


#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FChestoryModule, Chestory)