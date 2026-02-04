// Copyright 2024 Arkai Interactive. All Rights Reserved.


#include "ChestoryEditoFactory.h"
#include "ChestoryEditor.h"

#include "UMGEditorProjectSettings.h"
#include "Kismet2/KismetEditorUtilities.h"

#include "BaseWidgetBlueprint.h"
#include "Blueprint/WidgetTree.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"



///////////////////////////////////////////////////
//////////////// Asset factories ////////////////
///////////////////////////////////////////////////



//////// ItemDataAsset ////////
UItemDataAsset_Factory::UItemDataAsset_Factory(const class FObjectInitializer& OBJ) : Super(OBJ) {
	SupportedClass = UItemDataAsset::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UItemDataAsset_Factory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	check(Class->IsChildOf(UItemDataAsset::StaticClass()));
	return NewObject<UItemDataAsset>(InParent, Class, Name, Flags | RF_Transactional);
}


//////// ItemScript ////////
UItemScript_Factory::UItemScript_Factory(const class FObjectInitializer& OBJ) : Super(OBJ) {
	SupportedClass = UItemScript::StaticClass();
	ParentClass = UItemScript::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}
UObject* UItemScript_Factory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags,
	UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	check(Class->IsChildOf(UItemScript::StaticClass()));
	
	if (ParentClass != UItemScript::StaticClass())
	{
		FFormatNamedArguments Args;
		Args.Add(TEXT("ClassName"), (ParentClass != nullptr) ? FText::FromString(ParentClass->GetName()) : LOCTEXT("Null", "(null)"));
		FMessageDialog::Open(EAppMsgType::Ok, FText::Format(LOCTEXT("CannotCreateBlueprintFromClass", "Cannot create a blueprint based on the class '{0}'."), Args));
		return nullptr;
	}
	else
	{
		return FKismetEditorUtilities::CreateBlueprint(ParentClass, InParent, Name, BPTYPE_Normal, UBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass(), CallingContext);
	}
}



//////// InventorySlot Widget ////////
UInventorySlotUserWidget_Factory::UInventorySlotUserWidget_Factory(const class FObjectInitializer& OBJ) : Super(OBJ) {
	SupportedClass = UInventorySlotUserWidget::StaticClass();
	ParentClass = UInventorySlotUserWidget::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}
UObject* UInventorySlotUserWidget_Factory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags,
	UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	check(Class->IsChildOf(UInventorySlotUserWidget::StaticClass()));

	// If they selected an interface, force the parent class to be UInterface
	if (BlueprintType == BPTYPE_Interface)
	{
		ParentClass = UInterface::StaticClass();
	}
	
	if (ParentClass != UInventorySlotUserWidget::StaticClass())
	{
		FFormatNamedArguments Args;
		Args.Add(TEXT("ClassName"), (ParentClass != nullptr) ? FText::FromString(ParentClass->GetName()) : LOCTEXT("Null", "(null)"));
		FMessageDialog::Open(EAppMsgType::Ok, FText::Format(LOCTEXT("CannotCreateBlueprintFromClass", "Cannot create a blueprint based on the class '{0}'."), Args));
		return nullptr;
	}
	else
	{
		if (!GetDefault<UUMGEditorProjectSettings>()->bUseWidgetTemplateSelector)
		{
			RootWidgetClass = GetDefault<UUMGEditorProjectSettings>()->DefaultRootWidget;
		}

	        UWidgetBlueprint* NewBP = CastChecked<UWidgetBlueprint>(FKismetEditorUtilities::CreateBlueprint(ParentClass, InParent, Name, BlueprintType, UWidgetBlueprint::StaticClass(), UWidgetBlueprintGeneratedClass::StaticClass(), CallingContext));
     
     		// Create the desired root widget specified by the project
     		if ( NewBP->WidgetTree->RootWidget == nullptr )
     		{
     			if (TSubclassOf<UPanelWidget> RootWidgetPanel = RootWidgetClass)
     			{
     				UWidget* Root = NewBP->WidgetTree->ConstructWidget<UWidget>(RootWidgetPanel);
     				NewBP->WidgetTree->RootWidget = Root;
     			}
     		}
     
     		return NewBP;
	}
}

//////// CraftingSlot Widget ////////
UCraftingSlotUserWidget_Factory::UCraftingSlotUserWidget_Factory(const class FObjectInitializer& OBJ) : Super(OBJ) {
	SupportedClass = UCraftingSlotUserWidget::StaticClass();
	ParentClass = UCraftingSlotUserWidget::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}
UObject* UCraftingSlotUserWidget_Factory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags,
	UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	check(Class->IsChildOf(UCraftingSlotUserWidget::StaticClass()));

	// If they selected an interface, force the parent class to be UInterface
	if (BlueprintType == BPTYPE_Interface)
	{
		ParentClass = UInterface::StaticClass();
	}
	
	if (ParentClass != UCraftingSlotUserWidget::StaticClass())
	{
		FFormatNamedArguments Args;
		Args.Add(TEXT("ClassName"), (ParentClass != nullptr) ? FText::FromString(ParentClass->GetName()) : LOCTEXT("Null", "(null)"));
		FMessageDialog::Open(EAppMsgType::Ok, FText::Format(LOCTEXT("CannotCreateBlueprintFromClass", "Cannot create a blueprint based on the class '{0}'."), Args));
		return nullptr;
	}
	else
	{
		if (!GetDefault<UUMGEditorProjectSettings>()->bUseWidgetTemplateSelector)
		{
			RootWidgetClass = GetDefault<UUMGEditorProjectSettings>()->DefaultRootWidget;
		}

		UWidgetBlueprint* NewBP = CastChecked<UWidgetBlueprint>(FKismetEditorUtilities::CreateBlueprint(ParentClass, InParent, Name, BlueprintType, UWidgetBlueprint::StaticClass(), UWidgetBlueprintGeneratedClass::StaticClass(), CallingContext));
     
		// Create the desired root widget specified by the project
		if ( NewBP->WidgetTree->RootWidget == nullptr )
		{
			if (TSubclassOf<UPanelWidget> RootWidgetPanel = RootWidgetClass)
			{
				UWidget* Root = NewBP->WidgetTree->ConstructWidget<UWidget>(RootWidgetPanel);
				NewBP->WidgetTree->RootWidget = Root;
			}
		}
     
		return NewBP;
	}
}


//////// Socket Widget ////////
USocketUserWidget_Factory::USocketUserWidget_Factory(const class FObjectInitializer& OBJ) : Super(OBJ) {
	SupportedClass = USocketUserWidget::StaticClass();
	ParentClass = USocketUserWidget::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}
UObject* USocketUserWidget_Factory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags,
	UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	check(Class->IsChildOf(USocketUserWidget::StaticClass()));

	// If they selected an interface, force the parent class to be UInterface
	if (BlueprintType == BPTYPE_Interface)
	{
		ParentClass = UInterface::StaticClass();
	}

	if (ParentClass != USocketUserWidget::StaticClass())
	{
		FFormatNamedArguments Args;
		Args.Add(TEXT("ClassName"), (ParentClass != nullptr) ? FText::FromString(ParentClass->GetName()) : LOCTEXT("Null", "(null)"));
		FMessageDialog::Open(EAppMsgType::Ok, FText::Format(LOCTEXT("CannotCreateBlueprintFromClass", "Cannot create a blueprint based on the class '{0}'."), Args));
		return nullptr;
	}
	else
	{
		if (!GetDefault<UUMGEditorProjectSettings>()->bUseWidgetTemplateSelector)
		{
			RootWidgetClass = GetDefault<UUMGEditorProjectSettings>()->DefaultRootWidget;
		}

		UWidgetBlueprint* NewBP = CastChecked<UWidgetBlueprint>(FKismetEditorUtilities::CreateBlueprint(ParentClass, InParent, Name, BlueprintType, UWidgetBlueprint::StaticClass(), UWidgetBlueprintGeneratedClass::StaticClass(), CallingContext));

		// Create the desired root widget specified by the project
		if (NewBP->WidgetTree->RootWidget == nullptr)
		{
			if (TSubclassOf<UPanelWidget> RootWidgetPanel = RootWidgetClass)
			{
				UWidget* Root = NewBP->WidgetTree->ConstructWidget<UWidget>(RootWidgetPanel);
				NewBP->WidgetTree->RootWidget = Root;
			}
		}

		return NewBP;
	}
}

#undef LOCTEXT_NAMESPACE