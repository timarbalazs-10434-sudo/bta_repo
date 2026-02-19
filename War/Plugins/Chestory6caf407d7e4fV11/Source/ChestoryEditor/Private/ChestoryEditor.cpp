// Copyright 2024 Arkai Interactive. All Rights Reserved.

#include "ChestoryEditor.h"

#include "AssetToolsModule.h"
#include "ChestoryEditoFactory.h"
#include "Modules/ModuleManager.h"
#include "Modules/ModuleInterface.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleRegistry.h"


//#include "ItemScript.h"
//#include "InventorySlotUserWidget.h"
//#include "CraftingSlotUserWidget.h"
//#include "Socket/SocketUserWidget.h"


#include "Misc/Paths.h"
#include "Engine/AssetManagerSettings.h"


DEFINE_LOG_CATEGORY(LogInventory);

class IAssetTools;

#define LOCTEXT_NAMESPACE "FChestoryEditor"

const FPrimaryAssetType	FChestoryEditor::ItemDataType = TEXT("ItemDataAsset");

void FChestoryEditor::StartupModule()
{

	//------REGISTER DEFAULT EVENTS-----////
	
	//Item Script
	RegisterDefaultEvent(UItemScript,ItemAddInInventory);
	RegisterDefaultEvent(UItemScript,ItemRemoveInInventory);
	RegisterDefaultEvent(UItemScript,ItemUsed);
	RegisterDefaultEvent(UItemScript,ItemDropped);
	RegisterDefaultEvent(UItemScript,ItemCreated);

	// InventorySlot Widget
	RegisterDefaultEvent(UInventorySlotUserWidget, UpdateItemInformation);
	RegisterDefaultEvent(UInventorySlotUserWidget, UpdateItemRepairInformation);
	RegisterDefaultEvent(UInventorySlotUserWidget, UpdateItemBlueprintInformation);
	RegisterDefaultEvent(UInventorySlotUserWidget, OnDurabilityChanged);
	RegisterDefaultEvent(UInventorySlotUserWidget, OnDecayChanged);

	// CraftingSlot Widget
	RegisterDefaultEvent(UCraftingSlotUserWidget,UpdateCraftingData);
	// If Waiting Slot
	RegisterDefaultEvent(UCraftingSlotUserWidget,CraftingStart);
	RegisterDefaultEvent(UCraftingSlotUserWidget,CraftingEnd);
	RegisterDefaultEvent(UCraftingSlotUserWidget,UpdateCraftingProgress);

	// Socket Widget
	RegisterDefaultEvent(USocketUserWidget, SocketUpdate);
	



	// ------- SETUP CATEGORY FACTORY ------- /////

	IAssetTools &AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	ChestoryAssetCategory = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("Chestory")),LOCTEXT("ChestoryCategory","Chestory"));

	// --- ItemData
	AssetTools.RegisterAssetTypeActions(MakeShareable(new FAssetTypeActions_ItemDataAsset(ChestoryAssetCategory)));

	// --- ItemScript
	AssetTools.RegisterAssetTypeActions(MakeShareable(new FAssetTypeActions_ItemScript(ChestoryAssetCategory)));

	// --- InventorySlot Widget
	AssetTools.RegisterAssetTypeActions(MakeShareable(new FAssetTypeActions_InventorySlotUserWidget(ChestoryAssetCategory)));

	// --- Craftinglot Widget
	AssetTools.RegisterAssetTypeActions(MakeShareable(new FAssetTypeActions_CraftingSlotUserWidget(ChestoryAssetCategory)));

	// --- Socket Widget
	AssetTools.RegisterAssetTypeActions(MakeShareable(new FAssetTypeActions_SocketUserWidget(ChestoryAssetCategory)));


	
	///////////////////////////////////////////////////////
	//------SETUP ASSET THUMBNAILS-----//// 
	StyleSet = MakeShareable(new FSlateStyleSet("ChestoryStyle"));
	FString ContentDir = IPluginManager::Get().FindPlugin("Chestory")->GetBaseDir(); 	//Content path of this plugin
	StyleSet->SetContentRoot(ContentDir);

	//--------CREATE ASSET BRUSH ICONS-----------//

	//Thanks OmegaGameFramework Plugin !  by Caius
	// Add Basic Thumbnail Location List
	TArray<FString> ThumnbailNames;
	ThumnbailNames.Add(TEXT("ItemDataAsset"));
	ThumnbailNames.Add(TEXT("ItemScript"));
	ThumnbailNames.Add(TEXT("InventoryUniformGridPanel"));
	ThumnbailNames.Add(TEXT("InventorySlotUserWidget"));
	ThumnbailNames.Add(TEXT("CraftingSlotUserWidget"));
	ThumnbailNames.Add(TEXT("SocketUserWidget"));

	FSlateImageBrush* ThumbnailTemp;
	FSlateImageBrush* IconTemp;

	FString ThumbanilPrefex;
	FString IconPrefex;

	FString DirecPrefex;
	FName IcoName;

	TMap<FString, FSlateImageBrush*> AssetImages;
	for (FString TempString : ThumnbailNames)
	{
		ThumbanilPrefex = "ClassThumbnail.";
		ThumbanilPrefex.Append(TempString);

		IconPrefex = "ClassIcon.";
		IconPrefex.Append(TempString);

		//Get Image Directory
		DirecPrefex = "Resources/Icons/";
		DirecPrefex.Append(TempString);

		//Create and set Thumbnail
		ThumbnailTemp = new FSlateImageBrush(StyleSet->RootToContentDir(DirecPrefex, TEXT(".png")), FVector2D(128.f, 128.f));
		IcoName = FName(*ThumbanilPrefex);
		StyleSet->Set(IcoName, ThumbnailTemp);

		DirecPrefex.Append("_16");
		//Create and set Icon
		IconTemp = new FSlateImageBrush(StyleSet->RootToContentDir(DirecPrefex, TEXT(".png")), FVector2D(16, 16.f));
		IcoName = FName(*IconPrefex);
		StyleSet->Set(IcoName, IconTemp);
	};

	//Reguster the created style
	FSlateStyleRegistry::RegisterSlateStyle(*StyleSet);
	//_______________________

	AddRuleInAssetManager();
	
}

void FChestoryEditor::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FKismetEditorUtilities::UnregisterAutoBlueprintNodeCreation(this);
	FSlateStyleRegistry::UnRegisterSlateStyle((StyleSet->GetStyleSetName()));
}

bool FChestoryEditor::CheskHasSetting()
{
	UAssetManagerSettings* Settings = GetMutableDefault<UAssetManagerSettings>();

	for (auto Scan : Settings->PrimaryAssetTypesToScan)
	{
		if (Scan.PrimaryAssetType == ItemDataType.GetName())
		{
			return true;
		}
	}

	return false;
}

void FChestoryEditor::AddRuleInAssetManager()
{
	//----------------------------------------------------------------------- TEST 3
	// TODO Gestion SourceControl Look GameFeaturesEditorModule.cpp

	if (!CheskHasSetting())
	{
		UE_LOG(LogInventory, Warning, TEXT("ItemDataAsset AssetManager rule is not Found"))
		// Check out the ini or make it writable
		UAssetManagerSettings* Settings = GetMutableDefault<UAssetManagerSettings>();

		FDirectoryPath DummyPath;
		DummyPath.Path = TEXT("/Game");
		FName NameClass = UItemDataAsset::StaticClass()->GetFName();
		
		
		// UE5 contructor
		FPrimaryAssetTypeInfo NewTypeInfo(
			NameClass,
			UItemDataAsset::StaticClass(),
			false,
			false,
			{ DummyPath },
			{});
			
		NewTypeInfo.Rules.CookRule = EPrimaryAssetCookRule::AlwaysCook;
		
		Settings->Modify(true);

		Settings->PrimaryAssetTypesToScan.Add(NewTypeInfo);

		bool Validate = false;
		bool Other = false;
		NewTypeInfo.FillRuntimeData(Validate, Other);

		UE_LOG(LogInventory, Display, TEXT("Try to add Default AssetManager Path: %s"), *NewTypeInfo.AssetScanPaths[0]);

		bool bSuccessUpdate = false;
		Settings->PostEditChange();
		bSuccessUpdate = Settings->TryUpdateDefaultConfigFile();

		UAssetManager::Get().ReinitializeFromConfig();

		
		if (bSuccessUpdate)
		{
			UE_LOG(LogInventory, Display, TEXT("AssetManager add rule Success"));
		}
		else
		{
			UE_LOG(LogInventory, Warning, TEXT("AssetManager add rule Failed"));
		}
		
	}
	else
	{
		UE_LOG(LogInventory, Display, TEXT("AssetManager rule has found"))
	}
}


#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FChestoryEditor, ChestoryEditor)