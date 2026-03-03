// Copyright 2024 Arkai Interactive. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "AssetTypeActions_Base.h"
#include "AssetTypeActions/AssetTypeActions_Blueprint.h"
#include "Factories/BlueprintFactory.h"

#include "WidgetBlueprint.h"

#include "ItemDataAsset.h"
#include "ItemScript.h"
#include "InventorySlotUserWidget.h"
#include "CraftingSlotUserWidget.h"
#include "Socket/SocketUserWidget.h"

#include "ChestoryEditoFactory.generated.h"

/**
 * 
 */
UCLASS()
class CHESTORYEDITOR_API UChestoryEditoFactory : public UFactory
{
	GENERATED_BODY()
	
};


////////////////////////////////////////////
////////---Asset Factories---//////////////
///////////////////////////////////////////

//ItemDataAsset
UCLASS()
class CHESTORYEDITOR_API UItemDataAsset_Factory : public UFactory
{
	GENERATED_UCLASS_BODY()
public:

	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
};

class FAssetTypeActions_ItemDataAsset : public FAssetTypeActions_Base
{
public:
	FAssetTypeActions_ItemDataAsset(EAssetTypeCategories::Type InAssetCategory) : ChestoryAssetCategory(InAssetCategory){};
	
	virtual FText GetName() const override { return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_ItemDataAsset", "Item"); }
	virtual uint32 GetCategories() override { return ChestoryAssetCategory; }
	virtual FColor GetTypeColor() const override { return FColor(201, 29, 85); }
	virtual FText GetAssetDescription(const FAssetData& AssetData) const override { return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_ItemDataAssetDesc", "Item Data"); }
	virtual UClass* GetSupportedClass() const override { return UItemDataAsset::StaticClass(); }
private:
	EAssetTypeCategories::Type ChestoryAssetCategory;
};


//ItemScript
UCLASS()
class CHESTORYEDITOR_API UItemScript_Factory : public UBlueprintFactory
{
	GENERATED_UCLASS_BODY()
public:
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext) override;
	virtual bool ConfigureProperties() override {return true;};
};

class FAssetTypeActions_ItemScript : public FAssetTypeActions_Blueprint
{
public:
	FAssetTypeActions_ItemScript(EAssetTypeCategories::Type InAssetCategory) : ChestoryAssetCategory(InAssetCategory){};
	
	virtual FText GetName() const override { return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_ItemScript", "Item Script"); }
	virtual uint32 GetCategories() override { return ChestoryAssetCategory; }
	virtual FColor GetTypeColor() const override { return FColor( 63, 126, 255 ); }
	virtual FText GetAssetDescription(const FAssetData& AssetData) const override { return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_AbilityContextDesc", "Script for Items"); }
	virtual UClass* GetSupportedClass() const override { return UItemScript::StaticClass(); }
private:
	EAssetTypeCategories::Type ChestoryAssetCategory;
};



//InventorySlot Widget
UCLASS()
class CHESTORYEDITOR_API UInventorySlotUserWidget_Factory : public UBlueprintFactory
{
	GENERATED_UCLASS_BODY()
public:

	// The type of blueprint that will be created
	/*UPROPERTY(EditAnywhere, Category=WidgetBlueprintFactory)
	TEnumAsByte<enum EBlueprintType> BlueprintType;*/

	UClass* RootWidgetClass;
	
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext) override;
	virtual bool ConfigureProperties() override {return true;};
};

class FAssetTypeActions_InventorySlotUserWidget : public FAssetTypeActions_Blueprint
{
public:
	FAssetTypeActions_InventorySlotUserWidget(EAssetTypeCategories::Type InAssetCategory) : ChestoryAssetCategory(InAssetCategory){};
	
	virtual FText GetName() const override { return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_InventorySlotUserWidget", "Widget InventorySlot"); }
	virtual uint32 GetCategories() override { return ChestoryAssetCategory; }
	virtual FColor GetTypeColor() const override { return FColor( 63, 126, 255 ); }
	virtual FText GetAssetDescription(const FAssetData& AssetData) const override { return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_AbilityContextDesc", "Widget Inventory Slot Style"); }
	virtual UClass* GetSupportedClass() const override { return UInventorySlotUserWidget::StaticClass(); }
private:
	EAssetTypeCategories::Type ChestoryAssetCategory;
};


//CraftingSlot Widget
UCLASS()
class CHESTORYEDITOR_API UCraftingSlotUserWidget_Factory : public UBlueprintFactory
{
	GENERATED_UCLASS_BODY()
public:

	// The type of blueprint that will be created
	/*UPROPERTY(EditAnywhere, Category=WidgetBlueprintFactory)
	TEnumAsByte<enum EBlueprintType> BlueprintType;*/

	UClass* RootWidgetClass;
	
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext) override;
	virtual bool ConfigureProperties() override {return true;};
};

class FAssetTypeActions_CraftingSlotUserWidget : public FAssetTypeActions_Blueprint
{
public:
	FAssetTypeActions_CraftingSlotUserWidget(EAssetTypeCategories::Type InAssetCategory) : ChestoryAssetCategory(InAssetCategory){};
	
	virtual FText GetName() const override { return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_CraftingSlotUserWidget", "Widget CraftingSlot"); }
	virtual uint32 GetCategories() override { return ChestoryAssetCategory; }
	virtual FColor GetTypeColor() const override { return FColor( 63, 126, 255 ); }
	virtual FText GetAssetDescription(const FAssetData& AssetData) const override { return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_AbilityContextDesc", "Widget Crafting/Waiting Slot Style"); }
	virtual UClass* GetSupportedClass() const override { return UCraftingSlotUserWidget::StaticClass(); }
private:
	EAssetTypeCategories::Type ChestoryAssetCategory;
};

//Socket Widget
UCLASS()
class CHESTORYEDITOR_API USocketUserWidget_Factory : public UBlueprintFactory
{
	GENERATED_UCLASS_BODY()
public:

	// The type of blueprint that will be created
	//UPROPERTY(EditAnywhere, Category = WidgetBlueprintFactory)
	//	TEnumAsByte<enum EBlueprintType> BlueprintType;

	UClass* RootWidgetClass;

	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext) override;
	virtual bool ConfigureProperties() override { return true; };
};

class FAssetTypeActions_SocketUserWidget : public FAssetTypeActions_Blueprint
{
public:
	FAssetTypeActions_SocketUserWidget(EAssetTypeCategories::Type InAssetCategory) : ChestoryAssetCategory(InAssetCategory) {};

	virtual FText GetName() const override { return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_SocketUserWidget", "Widget Socket"); }
	virtual uint32 GetCategories() override { return ChestoryAssetCategory; }
	virtual FColor GetTypeColor() const override { return FColor(63, 126, 255); }
	virtual FText GetAssetDescription(const FAssetData& AssetData) const override { return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_AbilityContextDesc", "Widget Socket Style"); }
	virtual UClass* GetSupportedClass() const override { return USocketUserWidget::StaticClass(); }
private:
	EAssetTypeCategories::Type ChestoryAssetCategory;
};