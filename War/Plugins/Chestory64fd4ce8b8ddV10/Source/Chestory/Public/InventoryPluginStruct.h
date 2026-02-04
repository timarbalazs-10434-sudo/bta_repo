// Copyright 2024 Arkai Interactive. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/Class.h"
#include "Engine/EngineTypes.h"
#include "Engine/DataTable.h"
#include "GameplayTagAttribute.h"
#include "Misc/Guid.h"
#include "ChestorySaveGame.h"
#include "Math/IntPoint.h"
#include "InventoryPluginStruct.generated.h"



/**
 * 
 */
class CHESTORY_API InventoryPluginStruct
{
public:
	InventoryPluginStruct();
	~InventoryPluginStruct();
};


UENUM(BlueprintType)
enum class EItemType : uint8
{
	/*
	* No rules 
	*/
	EMiscellaneous UMETA(DisplayName = "Miscellaneous"),
	
	/*
	* During the "UseItem" action this item will try to equip itself on an open Equipment inventory, it will also manage the Actor Equipment system
	*/
	EEquipement UMETA(DisplayName = "Equipement"),
	
	/*
	* During the "UseItem" action this item will trigger the "Decrement Item" function 
	*/
	EConsumbale UMETA(DisplayName = "Consumable"),

	/*
	* This item will be considered as a currency and a "Virtual" item  
	*/
	ECurrency UMETA(DisplayName = "Currency"),

	/*
	* During the "UseItem" action it will trigger the craft associated to it, 
	* it also tells the system that it must send the craft information to the data change in the inventory 
	*/
	EBlueprint UMETA(DisplayName = "Blueprint"),

	/*
	* No Impl
	*/
	ESkill UMETA(DisplayName = "Skill"),

	/*
	* This item will be considered as a "gem" and can be placed in the Socket  
	*/
	ESocket UMETA(DisplayName = "Socket")
};



UENUM(BlueprintType)
enum class ESlotType : uint8
{
	EInventory UMETA(DisplayName = "Inventory"),
	EEquipement UMETA(DisplayName = "Equipement"),
	EContainer UMETA(DisplayName = "Container"),
	EHotBar UMETA(DisplayName = "HotBar")
};

UENUM(BlueprintType)
enum class EGridType : uint8
{
	EInventory UMETA(DisplayName = "Inventory"),
	EContainer UMETA(DisplayName = "Container")
};

UENUM(BlueprintType)
enum class ECloseInventoryPreset : uint8
{
	/* Use only Array for close*/
	OnlyArray,
	
	/* Use Array and First inventory player in Array Inventorys Definition*/
	FirstInventory,
	
	/* Use Array and Equipment player*/
	Equipment,

	/* Use Array and First inventory player in Array Inventorys Definition and Equipment player*/
	FirstInventoryAndEquipment,
	
	/* Use Array and All inventories player in Array Inventorys Definition*/
	AllInventories,
	
	/* Use Array and All inventories player in Array Inventorys Definition and Equipment player*/
	AllInventoriesAndEquipment,
	
	/* Use Array and Last container open*/
	LastContainer,

	/* Use Array and All containers open*/
	AllContainers,

	/* All containers open and All inventories player in Array Inventorys Definition and Equipment player */
	AllContainersAndAllInventoriesAndEquipment
};

UENUM(BlueprintType)
enum class EUsedInputTrigger : uint8
{
	EUndefine UMETA(DisplayName = "Undefine"),
	EPressed UMETA(DisplayName = "Pressed"),
	EReleased UMETA(DisplayName = "Released")
};

//USTRUCT(BlueprintType)
//struct FInventoryID
//{
//	GENERATED_BODY()
//
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "InventoryID", meta = (ShowOnlyInnerProperties))
//	EGridType Type;
//
//	//Using the key to find this inventory later
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "InventoryID")
//	FName Key = "Default";
//
//	//Unique ID
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "InventoryID",meta = (HideInDetailPanel))
//	FGuid ID;
//
//	FInventoryID()
//		: FInventoryID(EGridType::EInventory,TEXT("Default"), FGuid::NewGuid())
//	{}
//
//	FInventoryID(EGridType InType,FName InKey, FGuid InID)
//		: Type(InType),
//		  Key(InKey),
//		  ID(InID)
//	{}
//
//	FInventoryID(const FInventoryID& Other)
//		: FInventoryID(Other.Type,Other.Key,Other.ID)
//	{}
//
//	bool operator==(const FInventoryID& Other) const
//	{
//		return Equals(Other);
//	}
//
//	bool Equals(const FInventoryID& Other) const
//	{
//		return (Type == Other.Type && Key == Other.Key && ID == Other.ID);
//	}
//
//public:
//
//
//};
//
//#if UE_BUILD_DEBUG
//uint32 GetTypeHash(const FInventoryID& Thing);
//#else // optimize by inlining in shipping and development builds
//FORCEINLINE uint32 GetTypeHash(const FInventoryID& Thing)
//{
//	uint32 Hash = FCrc::MemCrc32(&Thing, sizeof(FInventoryID));
//	return Hash;
//}
//#endif

USTRUCT(BlueprintType)
struct FInventoryDefinition
{
	GENERATED_BODY()

	FInventoryDefinition() {}

public:

	// Default GridKey
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Definition")
	FName GridKey;

	// Number of slots created
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Definition")
	int32 InventorySize = 40;

	//Number of slots maximum per row created
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Definition")
	int32 InventorySlotsPerRow = 5;

	/*
	* Using size Item system for this inventory
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container")
	bool EnableSizeItem = true;

	/*
	* If true, all items will be added in sequence.
	* Not recommended for use with item size system
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Definition")
	bool EnableAutoFill = false;

	/*
	* EnableAutoFill need to be True value
	* Automatically adjusts the number of available slots so that there are always empty slots.
	* The "InventorySize" value will be the minimum number of slots available
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Definition", meta = (EditCondition = "EnableAutoFill"))
	bool EnableDynamicSizeInventory = false;

	/*
	* The minimum number of rows that will always be available
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Definition", meta = (EditCondition = "EnableDynamicSizeInventory && EnableAutoFill"))
	int32 InventoryMiniRow = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Definition")
	float DecayFactor = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Definition")
	float DecayTickTime = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Definition")
	bool bCanRepair = true;

	/*
	* Override Weight System InventoryManager
	*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Definition", meta = (InlineEditConditionToggle))
	bool OverrideWeight = false;

	/*
	* Override Weight System InventoryManager
	* Enable Weight System for this inventory
	*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Definition", meta = (EditCondition = "OverrideWeight"))
	bool EnableWeight = false;

	//Only items with this tag are allowed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Definition")
	FGameplayTagQuery Restriction;

	//List Items possible to transfer in this inventory
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Definition", meta = (AllowedTypes = "ItemDataAsset"))
	TArray<FPrimaryAssetId> RestrictionPossibleItems;

	//List Items possible to transfer in this inventory
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Definition", meta = (AllowedTypes = "ItemDataAsset"))
	TArray<FPrimaryAssetId> RestrictionImpossibleItems;
};

USTRUCT(BlueprintType)
struct FSlotDefinition
{
	GENERATED_BODY()

		FSlotDefinition() {}

public:

	//Only items with this tag are allowed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Definition")
	FGameplayTagQuery Restriction;
};

USTRUCT()
struct FItemTiny : public FTableRowBase
{
	GENERATED_BODY()

public:


	UPROPERTY()
	FName ID = "None";

	UPROPERTY()
	FGameplayTagContainer GameplayTagContainer;

	UPROPERTY()
	int32 Amount = 1;

	UPROPERTY()
	FString JsonParameter = "";

	UPROPERTY()
	bool bIsDroppable = true;

	UPROPERTY()
	bool bIsDraggable = true;

	UPROPERTY()
	bool bIsStackable = true;

	UPROPERTY()
	bool bIsVisible = true;

	UPROPERTY()
	float Decay = 0.0f;

	UPROPERTY()
	float Durability = 0.0f;

	UPROPERTY()
	FGameplayTagAttributeContainer Attributes;

	UPROPERTY()
	bool bIsFilled = false;


	bool IsValid() const { return ID != "None"; }
	bool IsEmpty() const { return ID == "None"; }
};

USTRUCT(BlueprintType)
struct FSocketItem : public FTableRowBase
{
	GENERATED_BODY()

	FSocketItem() {}
	FSocketItem(FGameplayTagContainer InRestriction)
		: Restriction(InRestriction)
	{}


public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Item", meta = (AllowedTypes = "ItemDataAsset"))
	FPrimaryAssetId PrimaryAssetId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Item")
	FGameplayTagAttributeContainer Attributes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Item")
	FGameplayTagContainer Restriction;

	UPROPERTY()
	FItemTiny SocketTiny;


	void ClearSocket();
};

// Structur Complete
USTRUCT(BlueprintType, meta =(IsBlueprintBase="true"))
struct FItemData : public FTableRowBase
{
	GENERATED_BODY()


public :
	FItemData() : ID(TEXT("None")) {}
	FItemData(FName InID)
		: ID(InID)
	{}

	UPROPERTY(SaveGame)
	FGuid UniqueId;

	/*
	* Filled automatically by the Plugin
	* Name of the item to find ItemDataAsset associated to it
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame, Category = "ID")
	FName ID;

//---------- UI	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	class UTexture2D* Icon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI", meta = (MultiLine="true"))
	FText Description;

//---------- Definition
	/*
	* Defines the default action for the "UseItem" event and tells the system how to use the item
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parametre")
	EItemType ItemType = EItemType::EMiscellaneous;
	
	/*
	* Tags applied to this item
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Parametre")
    FGameplayTagContainer GameplayTagContainer;

	/*
	* Size of the item in inventory slot
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parametre", meta = (ClampMin = 1, ClampMax = 5, UIMin = 1, UIMax = 5))
	FIntPoint Size = FIntPoint(1,1);
	
	/*
	* Weight per Unit
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parametre")
	float Weight = 0.f;

//---------- Drag
	/*
	* Indicates that this item can be moved 
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Parametre", meta = (DisplayName = "Enable Drag"))
	bool bIsDraggable = true;


	/*
	* The string can be used with Json, for example to add a parameter to your Item.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Parametre|Json", meta = (MultiLine = "true"))
	FString JsonParameter = "";

//---------- Stack
	/*
	* Indicates that this item can be stacked
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Stack", meta = (DisplayName = "Enable Stack"))
	bool bIsStackable = false;

	/*
	* Indicates the maximum that can be stacked
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stack", meta = (EditCondition = "bIsStackable"))
	int32 MaxStackable = 1;

//---------- Drop
	/*
	* Indicates that this item can be deposited in the world,
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Drop", meta = (DisplayName = "Enable Drop"))
	bool bIsDroppable = false;

	/*
	* This actor will be created at the time of the drop 
	* The actor must have the DroppedComponent
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop", meta = (EditCondition = "bIsDroppable", DisplayName = "ActorDropped"))
	TSubclassOf<class AActor> ActorDroped;

//---------- Hide Param		
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Parametre", meta = (EditCondition = "false"))
	bool bIsVisible = true;	

	UPROPERTY(BlueprintReadWrite, SaveGame, Category = "Parametre", meta = (EditCondition = "bIsStackable"))
	int32 Amount = 1;

//---------- Attributes
	/*
	* Indicates that this item can have attributes 
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	bool bCanAttributes = false;

	/*
	* List of default attributes 
	* Use an ItemScript to generate random attributes 
	* 
	* NOTE : Attribute values that are equal to or less than 0 are removed so as not to overload replication
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Attributes", meta = (EditCondition = "bCanAttributes"))
	FGameplayTagAttributeContainer Attributes;

//---------- Socket
	/*
	* Indicates that this item can have sockets
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Socket")
	bool bCanSockets = false;

	/*
	* Indicates the maximum number of possible sockets
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Socket", meta = (EditCondition = "bCanSockets"))
	int32 MaxSocket = 4;

	/*
	* Apply default sockets to this item
	* You can leave the values empty to initialize empty sockets 
	* Use an ItemScript to generate random sockets 
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Socket", meta = (EditCondition = "bCanSockets"))
	TArray<FSocketItem> Sockets;

//---------- Durability	
	/*
	* Current Durability Item
	*/
	UPROPERTY(BlueprintReadWrite, SaveGame, Category = "Durability")
	float Durability = 0.0f;

	/*
	* If the value is greater than 0 then this will activate the durability for this item
	* It is only possible to change this value if the item cannot be stacked
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Durability", meta = (EditCondition = "!bIsStackable"))
	float MaxDurability = 0.0f;

	/*
	* Indicates if this item can be repaired 
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Durability", meta = (EditCondition = "MaxDurability > 0"))
	bool bCanRepairable = true;

//---------- Decay	
	/*
	* Activate the Decay system for this item
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decay")
		bool bCanDecay = false;

	/*
	* Current Decay Value
	*/
	UPROPERTY(BlueprintReadWrite, SaveGame, Category = "Decay")
	float Decay = 0.0f;
	
	/*
	* Indicate the maximum time that this item can last before being decay
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decay", meta = (EditCondition = "bCanDecay", Units="Seconds"))
	float DecayTime = 0.0f;
	
	/*
	* This item will be added to the inventory each time the item reaches the end of its time
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decay", meta = (EditCondition = "bCanDecay", AllowedTypes="ItemDataAsset"))
	FPrimaryAssetId DecayItem;


//---------- Equipment
	/*
	* This Actor will be created at the time of the Equip event and will load the variables with the "SaveGame" Flag
	* At the time of the Unequip event this Actor will be destroyed and will save all the variables in a
	* TArray<uint8> with the Flag "SaveGame" in ItemData 
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment", meta = (EditCondition = "ItemType == EItemType::EEquipement"))
	TSubclassOf<class AActor> ActorEquipment;
	
	/*
	* If this item can be dropped then it will use the actor equipment 
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment", meta = (EditCondition = "ItemType == EItemType::EEquipement"))
	bool bDropActorEquipment = false;


	UPROPERTY(NotReplicated, SaveGame)
	TArray<FInventorysSaveData> DataBytesActor;

	UPROPERTY()
	AActor* ActorInWorld = nullptr;

	UPROPERTY(SaveGame)
	bool bIsFilled = false;

	UPROPERTY(SaveGame)
	TArray<int32> FilledSlotsIndex;

	UPROPERTY()
	TArray<class UItemScript*> ItemScriptInstance;

	UPROPERTY(SaveGame)
	FGuid LocationItemInventoryId;

	UPROPERTY(SaveGame)
	int32 LocationItemInventoryIndex = -1;



	bool IsValid() const { return ID != "None" || bIsFilled; }
	bool IsEmpty() const { return ID == "None";}

	
	bool IsEquipment() const { return ItemType == EItemType::EEquipement; }
	bool IsCurrency() const { return ItemType == EItemType::ECurrency; }
	bool IsBlueprint() const { return ItemType == EItemType::EBlueprint; }
	bool IsSocket() const { return ItemType == EItemType::ESocket; }
	bool IsConsumbale() const { return ItemType == EItemType::EConsumbale; }

	float GetAmount() const { return Amount; }

	//Size
	bool IsMiniSize() const { return Size == FIntPoint(1, 1); }
	int32 GetTotalSize() const { return Size.X * Size.Y; }

	//Weight 
	//Return Weight * Amount
	float GetWeight() const { return Weight * GetAmount(); }

	void SetLocationItem(FGuid InventoryId, int32 InventorySlot);

	TArray<UItemScript*> GetItemScriptCombined(class UInventoryManagerComponent* InventoryManager,bool IncludeGeneralScript = true);

	/* Start Notify Item Script*/
	void NotifyScript_ItemAdd(class UInventoryManagerComponent* InventoryManager, FGuid InInventoryId, int32 IndexSlot,class UInventoryComponent* InInventory);
	void NotifyScript_ItemRemove(class UInventoryManagerComponent* InventoryManager, FGuid FromInventoryId, int32 IndexSlot, class UInventoryComponent* FromInventory);
	void NotifyScript_ItemUsed(class UInventoryManagerComponent* InventoryManager, FGuid InventoryId, int32 IndexSlot, EUsedInputTrigger InputTrigger = EUsedInputTrigger::EUndefine);
	void NotifyScript_ItemEquip(class UInventoryManagerComponent* InventoryManager, FGuid InInventoryId, int32 IndexSlot);
	void NotifyScript_ItemUnequip(class UInventoryManagerComponent* InventoryManager,FGuid InInventoryId, int32 IndexSlot);
	void NotifyScript_ItemCreated(class UInventoryManagerComponent* InventoryManager,  FGuid InInventoryId, int32 IndexSlot, class UInventoryComponent* InInventory);
	void NotifyScript_ItemDropped(class UInventoryManagerComponent* InventoryManager, AActor* ActorDropped, FGuid FromInventoryId, int32 FromSlot);
	void NotifyScript_ItemModifier(class UInventoryManagerComponent* InventoryManager, FGuid FromInventoryId, int32 FromInventorySlot, FGuid ToInventoryId, int32 ToInventorySlot);
	void NotifyScript_SocketItemAdded(class UInventoryManagerComponent* InventoryManager, FGuid InInventoryId, int32 InSlot, FSocketItem NewSocket, int32 InSocketSlot);
	void NotifyScript_SocketItemRemoved(class UInventoryManagerComponent* InventoryManager, FGuid FromInventoryId, int32 FromSlot, FSocketItem RemovedSocket, int32 FromSocketSlot);
	bool NotifyScript_GetIsItemModifier(class UInventoryManagerComponent* InventoryManager);
	bool NotifyScript_GetCanMove(class UInventoryManagerComponent* InventoryManager, FGuid FromInventoryId, int32 FromInventorySlot, FGuid ToInventoryId, int32 ToInventorySlot);
	bool NotifyScript_GetCanDrop(class UInventoryManagerComponent* InventoryManager, FGuid FromInventoryId, int32 FromInventorySlot);
	bool NotifyScript_GetCanUnequip(class UInventoryManagerComponent* InventoryManager, FGuid FromInventoryId, int32 FromIventorySlot, FGuid InInventoryId, int32 IndexSlot, AActor* InActorEquipment, bool DropRequest);
	bool NotifyScript_GetCanEquip(class UInventoryManagerComponent* InventoryManager, FGuid FromInventoryId, int32 FromInventorySlot, FGuid ToInventoryId, int32 ToInventorySlot);
	bool NotifyScript_GetCanCraft(class UInventoryManagerComponent* InventoryManager, FGuid FromCraftingId, FItemDataInfoCraft ItemDataInfoCraft, int32 InAmount);
	bool NotifyScript_GetCanUse(class UInventoryManagerComponent* InventoryManager, FGuid InInventoryId, int32 InInventorySlot, EUsedInputTrigger InputTrigger = EUsedInputTrigger::EUndefine);
	FItemData NotifyScript_ItemCreation(class UInventoryManagerComponent* InventoryManager, FGuid FromInventoryId, int32 FromIventorySlot);
	FItemData NotifyScript_ItemPostLoad(class UInventoryManagerComponent* InventoryManager, FGuid FromInventoryId, int32 FromIventorySlot);
	FItemData NotifyScript_ItemPreSave(class UInventoryManagerComponent* InventoryManager, FGuid FromInventoryId, int32 FromIventorySlot);
	void NotifyScript_InventoryOpen(class UInventoryManagerComponent* InventoryManager, FGuid InInventoryId, int32 InventorySlot);
	void NotifyScript_InventoryClose(class UInventoryManagerComponent* InventoryManager, FGuid InInventoryId, int32 InventorySlot);
	/* End Notify Item Script*/


	UItemScript* GetItemScriptInstanceFromClass(TSubclassOf<UItemScript> ClassItemScript);
	bool ContaintInstanceScriptClass(UItemScript* ItemScript);


	FInventorysSaveData GetDataBytesActorByName(FName NameId);

};

//Filter item Sort by Size
struct FSortBySizeItem
{
	FORCEINLINE bool operator()(const FItemData& A, const FItemData& B) const
	{
		// Sort descending
		return B.GetTotalSize() < A.GetTotalSize();
	}
};

//Filter item Sort by Name
struct FSortByNameItem
{
	FORCEINLINE bool operator()(const FItemData& A, const FItemData& B) const
	{
		// Sort descending
		return A.Name.ToString()  < B.Name.ToString();
	}
};

//Filter item Sort by Attribute
struct FSortByAttributeItem
{

	FSortByAttributeItem() {}
	FSortByAttributeItem(FGameplayTag InTag)
		: Tag(InTag)
	{}

	FGameplayTag Tag;

	FORCEINLINE bool operator()(const FItemData& A, const FItemData& B) const
	{
		float ValueB = 0;
		float ValueA = 0;

		if (B.Attributes.ContainsAttributeTag(Tag))
		{
			ValueB = B.Attributes.GetAttributeValue(Tag);
		}

		if(A.Attributes.ContainsAttributeTag(Tag))
		{
			ValueA = A.Attributes.GetAttributeValue(Tag);

		}

		// Sort descending
		return ValueB < ValueA;
	}
};

//Filter item Sort by GameplayTag
struct FSortByGameplayTagItem
{

	FSortByGameplayTagItem() {}
	FSortByGameplayTagItem(FGameplayTag InTag)
		: Tag(InTag)
	{}

	FGameplayTag Tag;

	FORCEINLINE bool operator()(const FItemData& A, const FItemData& B) const
	{
		float ValueB = 0;
		float ValueA = 0;

		if (B.GameplayTagContainer.HasTag(Tag))
		{
			ValueB = 1;
		}

		if (A.GameplayTagContainer.HasTag(Tag))
		{
			ValueA = 1;

		}

		// Sort descending
		return ValueB < ValueA;
	}
};

//Filter item Sort by Weight
struct FSortByWeightItem
{
	FORCEINLINE bool operator()(const FItemData& A, const FItemData& B) const
	{
		// Sort descending
		return B.GetWeight() < A.GetWeight();
	}
};

//Filter item Sort by Weight
struct FSortByAmountItem
{
	FORCEINLINE bool operator()(const FItemData& A, const FItemData& B) const
	{
		// Sort descending
		return B.Amount < A.Amount;
	}
};

//Filter item Sort by Socket
struct FSortBySocketItem
{
	FORCEINLINE bool operator()(const FItemData& A, const FItemData& B) const
	{
		// Sort descending
		return B.Sockets.Num() < A.Sockets.Num();
	}
};

//Filter item Sort by Durability
struct FSortByDurabilityItem
{
	FORCEINLINE bool operator()(const FItemData& A, const FItemData& B) const
	{
		// Sort descending
		return B.Durability < A.Durability;
	}
};

//Filter item Sort by Decay
struct FSortByDecayItem
{
	FORCEINLINE bool operator()(const FItemData& A, const FItemData& B) const
	{
		float ValueB = 0;
		float ValueA = 0;

		if (B.bCanDecay)
		{
			ValueB = B.Decay / B.DecayTime;
		}

		if (A.bCanDecay)
		{
			ValueA = A.Decay / A.DecayTime;

		}

		// Sort descending
		return ValueB < ValueA;
	}
};


UENUM(BlueprintType)
enum class ESortItemRule : uint8
{
	/* Sort by alphabetically Name */
	Name,

	/* Sort by GameplayTag 
	*  Need input Tag
	*/
	GameplayTag,

	/* Sort by Size Item*/
	Size,

	/* Sort by Total Weight Item*/
	Weight,

	/* Sort by Amount*/
	Amount,

	/* Sort by Num Sockets */
	Socket,

	/* Sort by Durability*/
	Durability,

	/* Sort by Ratio Decay */
	Decay,

	/* Sort by Attribute Value 
	* Need input Tag
	*/
	Attribute
};





USTRUCT(BlueprintType)
struct FItemDataInfo : public FTableRowBase
{
	GENERATED_BODY()

public:
	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "ItemDataInfo")
		FName ID = "None";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "ItemDataInfo")
		class UTexture2D* Icon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "ItemDataInfo")
		FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "ItemDataInfo")
		EItemType ItemType = EItemType::EMiscellaneous;
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "ItemDataInfo")
        FGameplayTagContainer GameplayTagContainer;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemDataInfo")
		FIntPoint Size = FIntPoint(1, 1);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Parametre")
		float Weight = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Parametre")
		FString JsonParameter = "";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "ItemDataInfo")
		int32 Amount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "ItemDataInfo")
		bool bIsDroppable = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "ItemDataInfo")
		bool bIsDraggable = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "ItemDataInfo")
		bool bIsStackable = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame, Category = "ItemDataInfo")
		bool bIsVisible = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "ItemDataInfo")
	bool bCanDecay = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame, Category = "ItemDataInfo")
	float Decay = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "ItemDataInfo")
	float DecayTime = 0.0f;

	UPROPERTY(EditAnywhere,BlueprintReadOnly, SaveGame, Category = "ItemDataInfo")
	float Durability = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "ItemDataInfo")
	float MaxDurability = 0.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "ItemDataInfo")
	bool bCanRepairable = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "ItemDataInfo")
	FGameplayTagAttributeContainer Attributes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "ItemDataInfo")
	bool bCanSockets = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "ItemDataInfo")
	int32 MaxSocket = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "ItemDataInfo")
	TArray<FSocketItem> Sockets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "ItemDataInfo")
	bool bIsFilled = false;


	bool IsValid() const { return ID != "None"; }
	bool IsEmpty() const { return ID == "None"; }


	bool IsEquipment() const { return ItemType == EItemType::EEquipement; }
	bool IsCurrency() const { return ItemType == EItemType::ECurrency; }
	bool IsBlueprint() const { return ItemType == EItemType::EBlueprint; }
	bool IsSocket() const { return ItemType == EItemType::ESocket; }
	bool IsConsumbale() const { return ItemType == EItemType::EConsumbale; }

	//Weight
	float GetWeight() const { return Weight * Amount; }
};



USTRUCT()
struct FItemDataInfoCompressed : public FTableRowBase
{
	GENERATED_BODY()

public:
	
	UPROPERTY()
	FItemTiny ItemTiny;

	//UPROPERTY()
	//FName ID = "None";
	//		
	//UPROPERTY()
	//FGameplayTagContainer GameplayTagContainer;

	//UPROPERTY()
	//int32 Amount = 1;

	//UPROPERTY()
	//FString JsonParameter = "";

	//UPROPERTY()
	//bool bIsDroppable = true;

	//UPROPERTY()
	//bool bIsDraggable = true;

	//UPROPERTY()
	//bool bIsStackable = true;

	//UPROPERTY()
	//bool bIsVisible = true;

	//UPROPERTY()
	//float Decay = 0.0f;

	//UPROPERTY()
	//float Durability = 0.0f;

	//UPROPERTY()
	//FGameplayTagAttributeContainer Attributes;

	UPROPERTY()
	TArray<FSocketItem> Sockets;

	//UPROPERTY()
	//bool bIsFilled = false;


	bool IsValid() const { return ItemTiny.ID != "None"; }
	bool IsEmpty() const { return ItemTiny.ID == "None"; }


};


USTRUCT(BlueprintType)
struct FItem : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item", meta=(AllowedTypes = "ItemDataAsset"))
	FPrimaryAssetId PrimaryAssetId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FName InventoryKey = "DefaultPlayer";

	/*
	* Noitfy function ItemCreation & ItemCreated in ItemScript
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	bool bNotifyItemCreation = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 Amount = 1;

	// Juste for Save and Load Operation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 IndexSlot = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	bool bInEquipement = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	bool bInHotbar = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FName SlotKey = "";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	float Decay = 0.0f;

	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category = "Item")
	float Durability = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FGameplayTagAttributeContainer Attributes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TArray<FSocketItem> Sockets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Json", meta = (MultiLine = "true"))
	FString JsonParameter = "";
};

USTRUCT(BlueprintType)
struct FItemStarter : public FTableRowBase
{
	GENERATED_BODY()

public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemStarter", meta = (TitleProperty = "{PrimaryAssetId} Amount :{Amount}"))
	TArray<FItem> Items;
};


USTRUCT(BlueprintType)
struct FContainerSetting : public FTableRowBase
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ContainerSetting")
	FName NameContainer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ContainerSetting")
	int32 ContainerSize = 6;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ContainerSetting")
	int32 ContainerSlotsPerRow = 6;
	// If False then juste for Loot
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ContainerSetting")
	bool bIsStorageContainer = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ContainerSetting", Meta = (MakeEditWidget = true))
	FVector DroppedLocation = FVector(0,0,0);
};


USTRUCT(BlueprintType)
struct FItemCurrency : public FTableRowBase
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "ItemCurrency")
	FName ID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "ItemCurrency")
	int32 Amount = 0;

	bool IsValid() const { return ID != "None"; }
	bool IsEmpty() const { return ID == "None"; }
};

USTRUCT(BlueprintType)
struct FItemForSave : public FTableRowBase
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemForSave")
	TArray<FItem> ItemsInventory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemForSave")
	TArray<FItemCurrency> Currency;

};


USTRUCT(BlueprintType)
struct FItemCraft : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "ItemCraft", meta = (AllowedTypes = "ItemDataAsset"))
	FPrimaryAssetId PrimaryAssetId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "ItemCraft")
	int32 Amount = 0; 

	// Variable calculate on server for client 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "ItemCraft")
	int32 Current = -1; 
};



USTRUCT(BlueprintType)
struct FRecipeCraft : public FTableRowBase
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Item Craft Ingredient", meta = (TitleProperty = "{PrimaryAssetId} Amount :{Amount}"))
	TArray<FItemCraft> IngredientNeeded;

	/*In seconds. If equal 0 then instant craft*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Item Craft", meta= (Units="Seconds"))
	float TimeToCraft = 1.0f;

	/*
	* If true this recipe return that ItemDataAsset
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Item Craft Return")
	bool ReturnSelf = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Item Craft Return", meta = (EditCondition = "!ReturnSelf", AllowedTypes = "ItemDataAsset"))
	FPrimaryAssetId PrimaryAssetIdReturn;

	/*
	* Indicates if this recipe can have a failure 
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Item Craft Failure")
	bool bCanFailure = false;

	/*
	* Percentage chance of triggering a failure
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Item Craft Failure",meta = (EditCondition = "bCanFailure", ClampMin = "0.0", ClampMax = "100.0", UIMin = "0.0", UIMax = "100.0"))
	float PercentFailure = 0.0f;

	/*
	* In case of failure this item will be added to the inventory
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Item Craft Failure", meta = (EditCondition = "bCanFailure", AllowedTypes = "ItemDataAsset"))
	FPrimaryAssetId PrimaryAssetIdFailure;

	/*
	* Number of items to be added at the end of the craft
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Item Craft Return")
	int32 ItemReturnAmount = 1;
};


USTRUCT(BlueprintType)
struct FItemDataInfoCraft : public FTableRowBase
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "ItemDataInfoCraft")
	FItemDataInfo ItemInformation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "ItemDataInfoCraft")
	FName Tag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "ItemDataInfoCraft")
	FName Category;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "ItemDataInfoCraft")
	FText NameCategory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "ItemDataInfoCraft")
	int32 Priority = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "ItemDataInfoCraft")
	FRecipeCraft Recipe;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "ItemDataInfoCraft")
	int32 MaxCountCraft = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "ItemDataInfoCraft")
	bool bIsRepair = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "ItemDataInfoCraft")
	ESlotType InType = ESlotType::EInventory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "ItemDataInfoCraft")
	FGuid InventoryId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "ItemDataInfoCraft")
	int InventorySlot = 0;
};

USTRUCT()
struct FItemDataInfoCraftCompressed : public FTableRowBase
{
	GENERATED_BODY()

public:

	UPROPERTY()
	FItemDataInfoCompressed ItemInformationCompressed;

	UPROPERTY()
	TArray<int32> Amount;
	
	UPROPERTY()
	TArray<int32> IngredientNeededCurrent;

	UPROPERTY()
	int32 MaxCountCraft = 0;
};


//-------------------------------------------- INVENTORY SAVED IN STRUCTURE -----------------------------------//


USTRUCT(BlueprintType)
struct FItemDataSlotSaved : public FTableRowBase
{
	GENERATED_BODY()

public:

	FItemDataSlotSaved() : ID(TEXT("None")) {}
	FItemDataSlotSaved(FItemData ItemData, int32 InIndexSlot)
		: UniqueId(ItemData.UniqueId.ToString())
		, ID(ItemData.ID)
		, GameplayTagContainerExport(ItemData.GameplayTagContainer.ToString())
		, Amount(ItemData.Amount)
		, JsonParameter(ItemData.JsonParameter)
		, bIsDroppable(ItemData.bIsDroppable)
		, bIsDraggable(ItemData.bIsDraggable)
		, bIsStackable(ItemData.bIsStackable)
		, bIsVisible(ItemData.bIsVisible)
		, Decay(ItemData.Decay)
		, Durability(ItemData.Durability)
		, Attributes(ItemData.Attributes)
		, Sockets(ItemData.Sockets)
		, bIsFilled(ItemData.bIsFilled)
		, IndexSlot(InIndexSlot)
	{
	}
	UPROPERTY(BlueprintReadWrite, Category="ItemDataSlotSaved")
	FString UniqueId = "";

	UPROPERTY(BlueprintReadWrite, Category = "ItemDataSlotSaved")
	FName ID = "None";

	UPROPERTY(BlueprintReadWrite, Category = "ItemDataSlotSaved")
	FString GameplayTagContainerExport = "";

	UPROPERTY(BlueprintReadWrite, Category = "ItemDataSlotSaved")
	int32 Amount = 1;

	UPROPERTY(BlueprintReadWrite, Category = "ItemDataSlotSaved")
	FString JsonParameter = "";

	UPROPERTY(BlueprintReadWrite, Category = "ItemDataSlotSaved")
	bool bIsDroppable = true;

	UPROPERTY(BlueprintReadWrite, Category = "ItemDataSlotSaved")
	bool bIsDraggable = true;

	UPROPERTY(BlueprintReadWrite, Category = "ItemDataSlotSaved")
	bool bIsStackable = true;

	UPROPERTY(BlueprintReadWrite, Category = "ItemDataSlotSaved")
	bool bIsVisible = true;

	UPROPERTY(BlueprintReadWrite, Category = "ItemDataSlotSaved")
	float Decay = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "ItemDataSlotSaved")
	float Durability = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "ItemDataSlotSaved")
	FGameplayTagAttributeContainer Attributes;

	UPROPERTY(BlueprintReadWrite, Category = "ItemDataSlotSaved")
	TArray<FSocketItem> Sockets;

	UPROPERTY(BlueprintReadWrite, Category = "ItemDataSlotSaved")
	bool bIsFilled = false;

	UPROPERTY(BlueprintReadWrite, Category = "ItemDataSlotSaved")
	int32 IndexSlot  = 0;

	FItemData ToItemData() const;

};

USTRUCT(BlueprintType)
struct FInventorySaved : public FTableRowBase
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, Category = "InventorySaved")
	int32 InventorySize = 0;

	UPROPERTY(BlueprintReadWrite, Category = "InventorySaved")
	TArray<FName> SlotsIndexKey;

	UPROPERTY(BlueprintReadWrite, Category = "InventorySaved")
	TArray<FItemDataSlotSaved> SlotsSaved;

};

USTRUCT(BlueprintType)
struct FPlayerInventorySaved : public FTableRowBase
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, Category = "PlayerInventorySaved")
	TArray<FInventorySaved> ArrayPlayerInventory;
};


USTRUCT(BlueprintType)
struct FInventoryManagerSaved : public FTableRowBase
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, Category = "InventoryManagerSaved")
	TArray<FName> NamePlayerInventory;

	UPROPERTY(BlueprintReadWrite, Category = "InventoryManagerSaved")
	TArray<FInventorySaved> PlayerInventory;


	UPROPERTY(BlueprintReadWrite, Category = "InventoryManagerSaved")
	FInventorySaved EquipmentInventory;

	UPROPERTY(BlueprintReadWrite, Category = "InventoryManagerSaved")
	FInventorySaved HotbarInventory;

	UPROPERTY(BlueprintReadWrite, Category = "InventoryManagerSaved")
	TArray<FItemCurrency> Currency;


};
