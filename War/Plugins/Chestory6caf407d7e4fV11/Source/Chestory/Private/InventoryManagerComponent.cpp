// Copyright 2024 Arkai Interactive. All Rights Reserved.


#include "InventoryManagerComponent.h"
#include "Chestory.h"
#include "Blueprint/WidgetTree.h"
#include "Math/UnrealMathUtility.h"
#include "Components/UniformGridSlot.h"
#include "GenericPlatform/GenericPlatformMath.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "ItemDataAsset.h"
#include "ItemScript.h"
#include "Net/UnrealNetwork.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "CraftingComponent.h"
#include "InventorySubsystem.h"
#include "CraftingSlotUserWidget.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerState.h"
#include "ChestoryFunctionLibrary.h"

#include "ChestorySaveGame.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include "EnhancedInputSubsystems.h"

#include "Algo/Reverse.h"

#include "DroppedComponent.h"


// Sets default values for this component's properties
UInventoryManagerComponent::UInventoryManagerComponent()
{
	RegisterComponent();
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	SetComponentTickEnabled(true);
		
	SetNetAddressable();
	SetIsReplicatedByDefault(true);
	
	// ...
}


UInventoryManagerComponent::UInventoryManagerComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	if (IsValid(GetOwner()))
	{


		// EQUIPEMENT
		EquipementPlayerInventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("EquipementInventoryAuto"));
		EquipementPlayerInventory->bIsContainer = false;
		EquipementPlayerInventory->SetInventoryType(ESlotType::EEquipement);
		EquipementPlayerInventory->DecayFactor = DecayFactor;
		EquipementPlayerInventory->DecayTickTime = DecayTickTime;
		EquipementPlayerInventory->bCanRepair = bCanRepair;
		EquipementPlayerInventory->EnableWeight = EnableWeight;
		EquipementPlayerInventory->EnableSizeItem = false;
		EquipementPlayerInventory->SetOwnerInventoryManager(this);
		GetOwner()->AddOwnedComponent(EquipementPlayerInventory);

		// HOTBAR
		HotbarInventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("HotbarInventoryAuto"));
		HotbarInventory->bIsContainer = false;
		HotbarInventory->SetInventoryType(ESlotType::EHotBar);
		HotbarInventory->DecayFactor = DecayFactor;
		HotbarInventory->DecayTickTime = DecayTickTime;
		HotbarInventory->bCanRepair = bCanRepair;
		HotbarInventory->EnableWeight = bUseHotbarAsShortcut ? false : EnableWeight; 
		HotbarInventory->EnableSizeItem = false;
		HotbarInventory->SetOwnerInventoryManager(this);
		GetOwner()->AddOwnedComponent(HotbarInventory);

	}
}
void UInventoryManagerComponent::OnComponentCreated()
{

	Super::OnComponentCreated();


	//Create Inventory
	for (auto& DefInventory : Inventorys)
	{
		UInventoryComponent* NewInventory = NewObject<UInventoryComponent>(this, *DefInventory.Key.ToString());
		NewInventory->CreationMethod = EComponentCreationMethod::Native;
		NewInventory->bIsContainer = false;
		NewInventory->SetInventoryType(ESlotType::EInventory);
		NewInventory->SetOwnerInventoryManager(this);
		NewInventory->ApplyInventoryDefinition(DefInventory.Value);
		NewInventory->SetOwnerInventoryManager(this); //Update Weight Value Enable
		NewInventory->RegisterComponent();

		GetOwner()->AddOwnedComponent(NewInventory);
		PlayerInventorys.Add(DefInventory.Key, NewInventory);

		UE_LOG(LogInventory, VeryVerbose, TEXT("[UInventoryManagerComponent::OnComponentCreated]  Add Inventory [%s] on [%s]"), *DefInventory.Key.ToString(),*GetOwner()->GetFName().ToString());
	}

	//Def Equipments SlotKey
	EquipementPlayerInventory->SetInventoryType(ESlotType::EEquipement);
	EquipementPlayerInventory->EnableWeight = EnableWeight;
	EquipementPlayerInventory->EnableSizeItem = false;
	EquipementPlayerInventory->SetOwnerInventoryManager(this);
	for (auto& Equipment : Equipments)
	{
		EquipementPlayerInventory->SlotsIndexKey.Add(Equipment.Key);
		EquipementPlayerInventory->SlotsRestriction.Add(Equipment.Key, Equipment.Value.Restriction);
	}

	//Def Hotbar SlotKey
	HotbarInventory->SetInventoryType(ESlotType::EHotBar);
	HotbarInventory->EnableWeight = bUseHotbarAsShortcut ? false : EnableWeight;
	HotbarInventory->EnableSizeItem = false;
	HotbarInventory->SetOwnerInventoryManager(this);
	for (auto& HotSlot : Hotbars)
	{
		HotbarInventory->SlotsIndexKey.Add(HotSlot.Key);
		HotbarInventory->SlotsRestriction.Add(HotSlot.Key, HotSlot.Value.Restriction);
	}



}
void UInventoryManagerComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	Super::OnComponentDestroyed(bDestroyingHierarchy);




	
	if (IsValid(InventorySubsystem))
	{
		InventorySubsystem->ClearInventoryManagerInstance(this);
	}




}
#if WITH_EDITOR
void UInventoryManagerComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.Property == nullptr)
	{
		Super::PostEditChangeProperty(PropertyChangedEvent);
		return;
	}
	if (PropertyChangedEvent.Property->GetFName() == FName(TEXT("Inventorys")))
	{
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);

}
#endif
void UInventoryManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);



	DOREPLIFETIME(UInventoryManagerComponent, bIsReadyToUse);

	DOREPLIFETIME(UInventoryManagerComponent, AllCurrency);

	DOREPLIFETIME(UInventoryManagerComponent, EquipementTypeDisable);

	DOREPLIFETIME(UInventoryManagerComponent, InventoriesOpen);
	DOREPLIFETIME(UInventoryManagerComponent, ContainersOpen);
	DOREPLIFETIME(UInventoryManagerComponent, EquipmentsOpen);
	DOREPLIFETIME(UInventoryManagerComponent, PlayerInventoryIds);

	DOREPLIFETIME(UInventoryManagerComponent, CurrentWeight);


}


// Called when the game starts
void UInventoryManagerComponent::BeginPlay()
{


	// Get Subsystem Inventory
	InitializeSubsystem();

	Super::BeginPlay();
	//is Invalid return now
	if (!IsValid(GetInvSubsystem())) return;


	const FString InstanceName = GetOwner()->HasAuthority() ? TEXT("Server") : TEXT("Client");
	UE_LOG(LogInventory, Verbose, TEXT("------------------------------------- BEGIN PLAY Instance : %s"), *InstanceName);


	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;



	//Bind Equipement inventory for trigger EquipementChanged
	if (IsValid(EquipementPlayerInventory))
	{
		EquipementPlayerInventory->OnItemEnterInventory.AddDynamic(this, &UInventoryManagerComponent::EquipementChanged);
		EquipementPlayerInventory->OnItemLeaveInventory.AddDynamic(this, &UInventoryManagerComponent::EquipementChanged);
	}


	//Init Inventory Manager Component
	InitializeInventoryManager();


	//Disable Tick TODO 
	SetComponentTickEnabled(false);



	UE_LOG(LogInventory, Verbose, TEXT("------------------------------------- END BEGIN PLAY INVENTORY MANAGER------------------------------------- Instance : %s"), *InstanceName);
}

void UInventoryManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	//UE_LOG(LogInventory, Warning, TEXT("------------------------------------- EndPlay INVENTORY MANAGER----------------------------- Owner:[%s]"),*GetOwner()->GetFName().ToString());


}



// Called every frame
void UInventoryManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UInventoryManagerComponent::OnObjectPreLoad_Implementation()
{

	/*UnequipItem*/
	//Now is in InventoryComponent Type Equipment

}

void UInventoryManagerComponent::OnObjectPostLoad_Implementation()
{
	//Hotbar
	//RefreshHotbarSlots();

	//Update Currency
	UpdateAllCurrency();

}

void UInventoryManagerComponent::OnObjectPreDestoryed_Implementation()
{
	UE_LOG(LogInventory, Warning, TEXT("[ UInventoryManagerComponent::OnObjectPreDestoryed_Implementation] Trigger"));

	if (GetOwner()->HasAuthority())
	{
		TArray<FGuid> Copy_Open = EquipmentsOpen;

		for (auto Equipment : Copy_Open)
		{
			if (!Equipment.IsValid())
			{
				UE_LOG(LogInventory, Warning, TEXT("[ UInventoryManagerComponent::OnObjectPreDestoryed_Implementation] EquipmentId Invalid"));

				continue;
			}

			Server_CloseEquipment(Equipment);

		}

		Copy_Open = ContainersOpen;

		for (auto Container : ContainersOpen)
		{
			if (!Container.IsValid())
			{
				continue;
			}

		
			Server_CloseInventory(Container);
		}
	}
}

void UInventoryManagerComponent::InitializeSubsystem()
{
	const UGameInstance* GameInstance = GetOwner()->GetGameInstance();
	if (IsValid(GameInstance))
	{
		InventorySubsystem = GameInstance->GetSubsystem<UInventorySubsystem>();

		InventorySubsystem->SetInventoryManagerInstance(this);
	}
}

void UInventoryManagerComponent::InitializeStoreItems()
{

	AllCurrency = GetInvSubsystem()->StoreCurrency;
}


void UInventoryManagerComponent::InitializeInventoryManager()
{
	//Check if Items is Completed Load else pending load is completed
	if (!GetInvSubsystem()->bItemsCompletedLoad)
	{
		GetInvSubsystem()->OnLoadingStoreItemsCompleted.AddDynamic(this, &UInventoryManagerComponent::InitializeInventoryManager);
		return;
	}


	UE_LOG(LogInventory, Display, TEXT("[UInventoryManagerComponent::InitializeInventoryManager] Start Init Inventory Manager"));

	//if is GameMode don't go
	if (AGameModeBase* IsGameMode = Cast<AGameModeBase>(GetOwner()); IsGameMode)
	{
		UE_LOG(LogInventory, Display, TEXT("[UInventoryManagerComponent::InitializeInventoryManager] GameMode Inventory Manager Load completed"));
		EndBeginPlay.Broadcast();
		bIsReadyToUse = true;
		SetComponentTickEnabled(false);
		return;
	}


	// Server
	if (GetOwner()->HasAuthority())
	{
		InitializeStoreItems();


		//Init Size Inventory
		for (const auto& Inv : PlayerInventorys)
		{
			InitPlayerInventory(Inv.Key, Inv.Value, Inv.Value->InventoryID);
		}

		InitEquipementPlayerInventory();


		InitHotbarInventory();



		bIsReadyToUse = true;
	}

	//Client
	if (GetOwnerRole() > ROLE_SimulatedProxy)
	{
		UpdateAllCurrency();
	}



	SetComponentTickEnabled(false);

	
	
	/*
	* the owner is not sure if he is already controlled at the begin play, so we check the ROLE  
	* ROLE_AutonomousProxy means that the actor can take input, so this is a Client that will have a replication from a controlled actor
	* If it's a Listen Server then it will be controlled 
	*/
	if (GetOwnerRole() == ROLE_AutonomousProxy || GetIsLocallyControlled())
	{
		if(bEnableHotbarRefreshOnInit)
		{
			UE_LOG(LogInventory, Display, TEXT("[UInventoryManagerComponent::InitializeInventoryManager] IsLocallyControlled Client requests Data Hotbar from the Server"));
			//If the client is not ready, he will wait for the widgets
			RefreshHotbarSlots();
		}
		
		if (bEnableEquipmentRefreshOnInit)
		{
			UE_LOG(LogInventory, Display, TEXT("[UInventoryManagerComponent::InitializeInventoryManager] IsLocallyControlled Client requests Data Equipment from the Server"));
			RefreshEquipementSlots();
		}
	}


	SetupInventoryManagerInputHotbar();


	UE_LOG(LogInventory, Display, TEXT("[Controlled] --------------------------------- Inventory Manager Load completed ----------------------------------"));
	EndBeginPlay.Broadcast();
}

void UInventoryManagerComponent::InitPlayerInventory(FName InventoryKey, UInventoryComponent* Inventory, FGuid InventoryId)
{
	if (!InventoryId.IsValid())
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::InitPlayerInventory] InventoryId Invalid [%s]"), *InventoryId.ToString())
			return;
	}

	if (!GetInvSubsystem()->ContainsInventory(InventoryId)) {
		UE_LOG(LogInventory, Display, TEXT("[UInventoryManagerComponent::InitPlayerInventory] Inventory not found with InventoryId: [%s]"), *InventoryId.ToString())

			//Ensure Register Inventory Player
			GetInvSubsystem()->RegisterInventory(InventoryId, Inventory);
	}

	//UInventoryComponent* Inventory = GetInventory(InventoryId);
	if (IsValid(Inventory)) // Player Inventory 
	{
		FName GridKey = Inventory->GridKey;


		UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::InitPlayerInventory] InventoryId:[%s] GridKey[%s] NetMode: [%s]"), *InventoryId.ToString(), *GridKey.ToString(), *NetModeToString(GetNetMode()));
		//Step 1 : Server Try to init inventory
		if (GetOwnerRole() == ROLE_Authority)
		{
			//Init Inventory only if is empty (no save loaded)
			if (Inventory->GetInventoryItems().IsEmpty())
			{
				Server_InitializeInventory(Inventory); //Init with empty slots

				//Load Item from DataTable
				if (bStartingWithItem)
				{
					Server_Start_Inventorys(InventoryKey, RowStarter);
				}
			}

			Inventory->SetOwnerInventoryManager(this);
			HotbarInventory->SetInventoryType(ESlotType::EInventory);

			AddWeightInventorys(Inventory);
			PlayerInventoryIds.AddUnique(InventoryId);
		}

		//Step 2 : Client Try to found panel and load inventory
		if (GetIsLocallyControlled() && false)
		{
			// Has panel go Init or else wait panel added
			if (GetInvSubsystem()->ContainsGrid(GridKey))
			{
				UE_LOG(LogInventory, Display, TEXT("[UInventoryManagerComponent::InitPlayerInventory] Has Panel UI"));
				//	InitializeInventoryPanelUI(InventoryId, GridKey);

			}
			else
			{
				//TODO Add TMap<FGuid,FName> InventoryId,GridKey Request
				UE_LOG(LogInventory, Display, TEXT("[UInventoryManagerComponent::InitPlayerInventory] Pending panel UI (BindEvent OnPlayerGridAdded)"));
				RequestGrid.Add(GridKey, InventoryId);
				GetInvSubsystem()->OnPlayerGridAdded.AddDynamic(this, &UInventoryManagerComponent::OnPlayerGridAdded);
			}

		}
	}
}

void UInventoryManagerComponent::InitEquipementPlayerInventory()
{
	if (!IsValid(EquipementPlayerInventory)) // Player Equipement 
	{
		return;
	}

	if (Equipments.IsEmpty())
	{
		return;
	}


	//Init Inventory only if is empty (no save loaded)
	if (EquipementPlayerInventory->GetInventoryItems().IsEmpty())
	{
		if (GetOwnerRole() == ROLE_Authority)
		{
			Server_InitializeEquipement(Equipments.Num());

			//Load Item from DataTable
			if (bStartingWithItem)
			{
				Server_Start_Equipments(RowStarter);
			}
		}
	}


	if (GetOwnerRole() == ROLE_Authority)
	{
		EquipementPlayerInventory->SetOwnerInventoryManager(this);
		HotbarInventory->SetInventoryType(ESlotType::EEquipement);


		UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::InitEquipementPlayerInventory] Server try to send data to the Client"));
		//Trigger Script
		RefreshEquipementSlots();

		AddWeightInventorys(EquipementPlayerInventory);
	}
}

void UInventoryManagerComponent::InitHotbarInventory()
{
	if (!IsValid(HotbarInventory))
	{
		return;
	}

	if (Hotbars.IsEmpty())
	{
		return;
	}

	if (HotbarInventory->GetInventoryItems().IsEmpty())
	{
		if (GetOwnerRole() == ROLE_Authority)
		{

			Server_InitializeHotbar(Hotbars.Num());

			//Load Item from DataTable
			if (bStartingWithItem)
			{
				Server_Start_Hotbars(RowStarter);
			}
		}
	}

	if (GetOwnerRole() == ROLE_Authority)
	{
		HotbarInventory->SetOwnerInventoryManager(this);
		HotbarInventory->SetInventoryType(ESlotType::EHotBar);
		HotbarInventory->AddViewer(this, FName());

		if (!bUseHotbarAsShortcut)
		{
			AddWeightInventorys(HotbarInventory);
		}


		//Expetion for Listen server can refresh only instance if is controlled !
		if (GetNetMode() == ENetMode::NM_ListenServer && !GetIsLocallyControlled())
		{
			//Can't refresh if is not Localy Player
			return;
		}

		//RefreshHotbarSlots();
	}
}


//-------------- Input
#pragma region 
UEnhancedInputLocalPlayerSubsystem* UInventoryManagerComponent::GetInputSystem()
{
	ULocalPlayer* LocalPlayerFound = nullptr;

	if (APlayerController* Controller = Cast<APlayerController>(GetOwner()))
	{
		LocalPlayerFound = Controller->GetLocalPlayer();
	}

	if (!IsValid(LocalPlayerFound))
	{
		if (APawn* Pawn = Cast<APawn>(GetOwner()))
		{
			if (APlayerController* Controller = Cast<APlayerController>(Pawn->GetController()))
			{
				LocalPlayerFound = Controller->GetLocalPlayer();
			}

		}
	}

	if (!IsValid(LocalPlayerFound))
	{
		if (const APlayerState* PS = Cast<APlayerState>(GetOwner()))
		{
			if (IsValid(PS->GetPlayerController()))
			{
				LocalPlayerFound = PS->GetPlayerController()->GetLocalPlayer();
			}
		}
	}


	//Local Player Found
	if (IsValid(LocalPlayerFound))
	{
		UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::SetupInventoryManagerInput] LocalPlayer Found"))
		return LocalPlayerFound->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	}

	UE_LOG(LogInventory, VeryVerbose, TEXT("[UInventoryManagerComponent::GetInputSystem] LocalPlayer Not Found"))
	return nullptr;
}

/** Input */
void UInventoryManagerComponent::SetupInventoryManagerInput()
{

	// Get the local player subsystem
	if (UEnhancedInputLocalPlayerSubsystem* InputSystem = GetInputSystem())
	{
		if (InputMappingContext != nullptr)
		{
			if (InputSystem->HasMappingContext(InputMappingContext))
			{
				UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::SetupInventoryManagerInput] Has already  Mapping Context"))
				return;
			}

			InputSystem->AddMappingContext(InputMappingContext, Priority);			
			UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::SetupInventoryManagerInput] Add Mapping Context"))

			UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(InputSystem->GetLocalPlayer()->GetPlayerController(this->GetWorld())->InputComponent);
			if (IsValid(Input))
			{
				UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::SetupInventoryManagerInput] Enhanced Input Component Found"))

				//Bind
				TriggeredEventHandle_UseAction = Input->BindAction(UseAction, ETriggerEvent::Triggered, this, &UInventoryManagerComponent::InputCallback_UseItem).GetHandle();
				TriggeredEventHandle_TransferAction = Input->BindAction(TransferAction, ETriggerEvent::Started, this, &UInventoryManagerComponent::InputCallback_TransferItem).GetHandle();
				TriggeredEventHandle_TransferByIdAction = Input->BindAction(TransferByIdAction, ETriggerEvent::Triggered, this, &UInventoryManagerComponent::InputCallback_TransferIdItem).GetHandle();
				TriggeredEventHandle_SplitAction = Input->BindAction(SplitAction, ETriggerEvent::Triggered, this, &UInventoryManagerComponent::InputCallback_SplitItem).GetHandle();
				TriggeredEventHandle_DropAction = Input->BindAction(DropAction, ETriggerEvent::Started, this, &UInventoryManagerComponent::InputCallback_DropItem).GetHandle();
				TriggeredEventHandle_RepairAction = Input->BindAction(RepairAction, ETriggerEvent::Started, this, &UInventoryManagerComponent::InputCallback_RepairItem).GetHandle();




			/*	Input->BindAction(ContextualMenu, ETriggerEvent::Started, this, &UInventoryManagerComponent::InputCallback_ContextualMenu);
				Input->BindAction(NavigationUp, ETriggerEvent::Started, this, &UInventoryManagerComponent::InputCallback_NavigationUp);
				Input->BindAction(NavigationDown, ETriggerEvent::Started, this, &UInventoryManagerComponent::InputCallback_NavigationDown);
				Input->BindAction(NavigationLeft, ETriggerEvent::Started, this, &UInventoryManagerComponent::InputCallback_NavigationLeft);
				Input->BindAction(NavigationRight, ETriggerEvent::Started, this, &UInventoryManagerComponent::InputCallback_NavigationRight);*/


			}
			else
			{
				UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::SetupInventoryManagerInput] Owner Enhanced Input Component Invalid"))
			}
		}
	}
}

void UInventoryManagerComponent::RemoveInventoryManagerInput(bool InventoryContext)
{
	// Get the local player subsystem
	if (UEnhancedInputLocalPlayerSubsystem* InputSystem = GetInputSystem())
	{
		InputSystem->RemoveMappingContext(InputMappingContext);
		UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::RemoveInventoryManagerInput] Remove InventoryManagerInput"))
		//TODO Remove Binding

		UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(InputSystem->GetLocalPlayer()->GetPlayerController(this->GetWorld())->InputComponent);
		if (IsValid(Input))
		{
			Input->RemoveBindingByHandle(TriggeredEventHandle_UseAction);
			Input->RemoveBindingByHandle(TriggeredEventHandle_TransferAction);
			Input->RemoveBindingByHandle(TriggeredEventHandle_TransferByIdAction);
			Input->RemoveBindingByHandle(TriggeredEventHandle_SplitAction);
			Input->RemoveBindingByHandle(TriggeredEventHandle_DropAction);
			Input->RemoveBindingByHandle(TriggeredEventHandle_RepairAction);
		}
	}
}

void UInventoryManagerComponent::SetupInventoryManagerInputHotbar()
{
	// Get the local player subsystem
	if (UEnhancedInputLocalPlayerSubsystem* InputSystem = GetInputSystem())
	{
		if (InputMappingContextHotbar != nullptr)
		{
			InputSystem->AddMappingContext(InputMappingContextHotbar, PriorityHotbar);

			UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::SetupInventoryManagerInputHotbar] Add Mapping Context"))


				UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(InputSystem->GetLocalPlayer()->GetPlayerController(this->GetWorld())->InputComponent);
			if (IsValid(Input))
			{
				UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::SetupInventoryManagerInputHotbar] Enhanced Input Component Found"))

				//Bind Hotbar with Key
				for (auto& HotbarAction : HotbarActions)
				{
					UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::SetupInventoryManagerInputHotbar]Bind Hotbar:[%s]"), *HotbarAction.Key.ToString());
					Input->BindAction(HotbarAction.Value, ETriggerEvent::Triggered, this, &UInventoryManagerComponent::InputCallback_HotbarAction, HotbarAction.Key);
				}
			}
			else
			{
				UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::SetupInventoryManagerInputHotbar] Owner Enhanced Input Component Invalid"))
			}
		}
	}
}

void UInventoryManagerComponent::InputCallback_UseItem(const FInputActionValue& Value)
{
	if (Value.GetValueType() == EInputActionValueType::Boolean)
	{
		const bool IsPressed = Value.Get<bool>();
		EUsedInputTrigger Trigger = IsPressed ? EUsedInputTrigger::EPressed : EUsedInputTrigger::EReleased;

		if (HoverCurrentIsValid) { UseInventoryItem(HoverInventoryId, HoverInventorySlot, Trigger); }
	}
	else
	{
		if (HoverCurrentIsValid) { UseInventoryItem(HoverInventoryId, HoverInventorySlot); }

	}
}

void UInventoryManagerComponent::InputCallback_TransferItem(const FInputActionValue& Value)
{
	if (HoverCurrentIsValid) { UseTransferItem(HoverInventoryId, HoverInventorySlot,FGuid()); }
}

void UInventoryManagerComponent::TryToTransferById()
{
	if (bLastActionValueTransferIdItem)
	{
		if (HoverCurrentIsValid) { UseTransferAllItemById(HoverInventoryId, HoverPrimaryAssetId, FGuid()); }
	}
}

void UInventoryManagerComponent::InputCallback_TransferIdItem(const FInputActionValue& Value)
{
	bool BoolValue = Value.Get<bool>();
	bLastActionValueTransferIdItem = BoolValue;
}
void UInventoryManagerComponent::InputCallback_SplitItem(const FInputActionValue& Value)
{
	if (Value.GetValueType() != EInputActionValueType::Boolean) return;

	bool BoolValue = Value.Get<bool>();

	UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::InputCallback_SplitItem] Input Split IsPressed:[%s]"), BoolValue ? TEXT("True") : TEXT("False"));

	BoolValue ? ActivateSplitOnDrag() : DesactivateSplitOnDrag();

}

void UInventoryManagerComponent::InputCallback_DropItem(const FInputActionValue& Value)
{	
	//if (HoverCurrentIsValid) HoverInventorySlotUserWidget->RightMouseButtonPressed();
	if (HoverCurrentIsValid){UseDropItemFromType(HoverInventoryId, HoverInventorySlot);}
}

void UInventoryManagerComponent::InputCallback_RepairItem(const FInputActionValue& Value)
{
	if (HoverCurrentIsValid) { UseRepairItemFromType(HoverInventoryId, HoverInventorySlot, bRepairWithCraftingSystem); }
}

void UInventoryManagerComponent::InputCallback_HotbarAction(const FInputActionValue& Value, FName HotbarKey)
{
	UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::SetupInventoryManagerInput] HotbarTrigger %s"), *HotbarKey.ToString())
	if (Value.GetValueType() == EInputActionValueType::Boolean)
	{
		const bool IsPressed = Value.Get<bool>();
		EUsedInputTrigger Trigger = IsPressed ? EUsedInputTrigger::EPressed : EUsedInputTrigger::EReleased;
		Client_UseHotBarSlot(HotbarKey, Trigger);

	}
	else
	{
		Client_UseHotBarSlot(HotbarKey);
	}	
}

void UInventoryManagerComponent::InputCallback_ContextualMenu(const FInputActionValue& Value)
{
}

void UInventoryManagerComponent::InputCallback_NavigationUp(const FInputActionValue& Value)
{
	UE_LOG(LogInventory,Warning,TEXT("Up"))
}

void UInventoryManagerComponent::InputCallback_NavigationDown(const FInputActionValue& Value)
{
	UE_LOG(LogInventory, Warning, TEXT("Down"))
}

void UInventoryManagerComponent::InputCallback_NavigationLeft(const FInputActionValue& Value)
{
	UE_LOG(LogInventory, Warning, TEXT("Left"))
}

void UInventoryManagerComponent::InputCallback_NavigationRight(const FInputActionValue& Value)
{
	UE_LOG(LogInventory, Warning, TEXT("Right"))
}

#pragma endregion

UInventoryComponent* UInventoryManagerComponent::GetFirstPlayerInventory()
{
	if (PlayerInventorys.IsEmpty())
	{
		return nullptr;

	}

	for (auto& Inventory : PlayerInventorys)
	{
		return Inventory.Value;
	}

	return nullptr;
}

UInventoryComponent* UInventoryManagerComponent::GetSmartPlayerInventory(FItemData Item, bool OnlyOpen/* = true*/)
{
	if (!Item.IsValid())
	{
		return nullptr;

	}

	return GetInventory(GetSmartPlayerInventoryId(Item, OnlyOpen));

}

FGuid UInventoryManagerComponent::GetSmartPlayerInventoryId(FItemData Item, bool OnlyOpen/* = true*/, FGuid FromInventory/* = FGuid()*/, bool StartLast /*= false*/)
{
	if (!Item.IsValid())
	{
		return FGuid();
	}


	
	TArray<FGuid> IdToCheck;

	if (InventoriesOpen.IsEmpty())
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::GetSmartPlayerInventoryId] InventoriesOpen is Empty"))
			//return FGuid();
	}

	for (auto& IdOpen : InventoriesOpen)
	{
		IdToCheck.AddUnique(IdOpen);
	}


	if (PlayerInventoryIds.IsEmpty())
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::GetSmartPlayerInventoryId] PlayerInventoryIds is Empty"))
		return FGuid();
	}

	//Add Default player Inventory
	if (!OnlyOpen)
	{
		for (auto& IdOpen : PlayerInventoryIds)
		{
			IdToCheck.AddUnique(IdOpen);
		}
	}

	//Start to check by last Open

	if (StartLast)
	{
		Algo::Reverse(IdToCheck);
	}

	for (auto InventoryId : IdToCheck)
	{
		if (UInventoryComponent* Inventory = GetInventory(InventoryId); IsValid(Inventory))
		{
			//Check if have space in this inventory then continu for check another inventory
			if (Inventory->IsFull())
			{
				continue;
			}
			if (FromInventory.IsValid(); InventoryId == FromInventory)
			{
				continue;
			}

			if (Inventory->CheckItemInRestriction(Item))
			{
				UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::GetSmartPlayerInventoryId] InventoryId Found :[%s]"), *Inventory->GetId().ToString())

				return Inventory->GetId();
			}
		}
		else
		{
			UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::GetSmartPlayerInventoryId] InventoryComponent Invalid"))

		}
	}

	return FGuid();

}

FGuid UInventoryManagerComponent::GetFirstPlayerInventoryId()
{
	if (PlayerInventoryIds.IsEmpty())
	{
		return FGuid();

	}

	return PlayerInventoryIds[0];
}



void UInventoryManagerComponent::OnPlayerGridAdded(FName GridKey, UInventoryUniformGridPanel* Grid)
{
	//@TODO Check Request Grid with InventoryID
	UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::OnPlayerGridAdded] --------------- Recived event Player Added %s"), *GridKey.ToString());
	if (RequestGrid.Contains(GridKey))
	{
	//	InitializeInventoryPanelUI(*RequestGrid.Find(GridKey), GridKey);
		RequestGrid.Remove(GridKey);
	}
	//Added new Grid init client grid and call server refesh
}

void UInventoryManagerComponent::OnHotbarSlotAdded(FName SlotKey)
{
	if (GetInvSubsystem()->GetHotbarSlotNum() == Hotbars.Num())
	{
		// Client have all slot widget, request to the server to re send data UI
		Server_LoadHotbar();
		GetInvSubsystem()->OnHotbarSlotAdded.RemoveDynamic(this,&UInventoryManagerComponent::OnHotbarSlotAdded);
		bClientPendingHotbar = false;
	}
}

void UInventoryManagerComponent::Client_PendingHotbar()
{
	if (!GetIsLocallyControlled())
	{
		return;
	}
	if (!bClientPendingHotbar)
	{
		//No Hotbar found pending widget create
		GetInvSubsystem()->OnHotbarSlotAdded.AddDynamic(this, &UInventoryManagerComponent::OnHotbarSlotAdded);
		bClientPendingHotbar = true;
	}
}



void UInventoryManagerComponent::EquipementChanged(FPrimaryAssetId PrimaryAssetId, int32 Amount)
{
	OnEquipementChanged.Broadcast();
}


// ------------------------ WEIGHT SYSTEM ------------------------	
#pragma region /** Weight */


void UInventoryManagerComponent::OnRep_CurrentWeight()
{
	Multicast_UpdateWeightValue(CurrentWeight);

}

void UInventoryManagerComponent::Multicast_UpdateWeightValue_Implementation(float NewTotalWeight)
{
	CurrentWeight = NewTotalWeight;

	OnWeightChanged.Broadcast(GetTotalWeight(),GetMaxWeight());
}

void UInventoryManagerComponent::SetMaxWeight_Implementation(float NewMaxWeight)
{
	MaxWeight = NewMaxWeight;
}

void UInventoryManagerComponent::SetWeightFactor_Implementation(float NewWeightFactor)
{
	WeightFactor = NewWeightFactor;
}

void UInventoryManagerComponent::SetTotalWeight_Implementation(float NewTotalWeight)
{
	CurrentWeight = NewTotalWeight;

	//UE_LOG(LogInventory, Warning, TEXT("SetTotalWeight_Implementation :%f"), CurrentWeight);


	OnRep_CurrentWeight();
}


#pragma endregion

void UInventoryManagerComponent::OnChildrenWeightChanged(float TotalWeight, float NewMaxWeight)
{
	if (!GetOwner()->HasAuthority()) { return; }
	//UE_LOG(LogInventory, Warning, TEXT("OnChildrenWeightChanged :%f"), TotalWeight);
	float FindTotal = 0;
	for (const auto& Inventory : WeightInventorys)
	{
		//UE_LOG(LogInventory, Warning, TEXT("OnChildrenWeightChanged GetCurrentWeightValue :%f"), Inventory->GetCurrentWeightValue());

		FindTotal = FindTotal + Inventory->GetCurrentWeightValue();
	}

	SetTotalWeight(FindTotal);
}

void UInventoryManagerComponent::AddWeightInventorys_Implementation(UInventoryComponent* Inventory, bool RefreshWeight/* = false*/)
{
	if (!IsValid(Inventory)) { return; }

	if (EnableWeight && Inventory->EnableWeight)
	{

		if (GetOwner()->HasAuthority())
		{
			WeightInventorys.AddUnique(Inventory);
			Inventory->OnWeightChanged.AddDynamic(this, &UInventoryManagerComponent::OnChildrenWeightChanged);
			if (RefreshWeight)
			{
				Inventory->RefreshWeight();
			}
			//UE_LOG(LogInventory, Warning, TEXT("Add Owner %s"), *Inventory->GetOwner()->GetFName().ToString())

			//OnChildrenWeightChanged(0.f, 0.f);

		}
	}


}

void UInventoryManagerComponent::RemoveWeightInventorys_Implementation(UInventoryComponent* Inventory)
{
	if (!IsValid(Inventory)) { return; }


	if (EnableWeight && Inventory->EnableWeight)
	{
		if (GetOwner()->HasAuthority())
		{
			WeightInventorys.Remove(Inventory);
			Inventory->OnWeightChanged.RemoveDynamic(this, &UInventoryManagerComponent::OnChildrenWeightChanged);
			//UE_LOG(LogInventory, Warning, TEXT("Remove Owner %s"), *Inventory->GetOwner()->GetFName().ToString())

			OnChildrenWeightChanged(0.f, 0.f);
		}
	}
}

bool UInventoryManagerComponent::GetIsLocallyControlled()
{
	if (const APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		bool IsLocalPlayerPawn = Pawn->IsLocallyControlled() && Pawn->IsPlayerControlled();
		const FString IsLocal = IsLocalPlayerPawn ? TEXT("True") : TEXT("False");
		UE_LOG(LogInventory, Verbose, TEXT("----------------------------------- Type [Pawn] %s Is Local Controller: %s"), *GetOwner()->GetFName().ToString(), *IsLocal);
		return IsLocalPlayerPawn;
	}
	else if (const AController* Controller = Cast<AController>(GetOwner()))
	{
		bool IsLocalPlayerController = Controller->IsLocalController() && Controller->IsPlayerController();
		const FString IsLocal = IsLocalPlayerController ? TEXT("True") : TEXT("False");
		UE_LOG(LogInventory, Verbose, TEXT("----------------------------------- Type [Controller] %s Is Local Controller ? : %s"), *GetOwner()->GetFName().ToString(), *IsLocal);
		return IsLocalPlayerController;
	}
	else if (const APlayerState* PS = Cast<APlayerState>(GetOwner()))
	{
		if (IsValid(PS->GetPlayerController()))
		{
			bool IsLocalPlayerController = PS->GetPlayerController()->IsLocalController() && PS->GetPlayerController()->IsPlayerController();
			const FString IsLocal = IsLocalPlayerController ? TEXT("True") : TEXT("False");
			UE_LOG(LogInventory, Verbose, TEXT("----------------------------------- Type [PlayerState] %s Is Local Controller ? : %s"), *GetOwner()->GetFName().ToString(), *IsLocal);
			return IsLocalPlayerController;
		}
	}

	return false;
}



UInventorySubsystem* UInventoryManagerComponent::GetInvSubsystem()
{
	if (IsValid(InventorySubsystem))
	{
		return InventorySubsystem;
	}

	InitializeSubsystem();

	return InventorySubsystem;
}


void UInventoryManagerComponent::PickupItemActorDropped(UDroppedComponent* DroppedComponent)
{
	if (DroppedComponent)
	{
		Server_PickupItemActorDropped(DroppedComponent);		
	}
}

void UInventoryManagerComponent::Server_PickupItemActorDropped_Implementation(UDroppedComponent* DroppedComponent)
{
	if (!IsValid(DroppedComponent))
	{
		return;
	}

		
	FItemData Item	= DroppedComponent->GetItemData();
	
	//Apply modification data on Item
	UChestoryFunctionLibrary::UpdateDataBytesActor(this, Item, true);
                					
	if (Item.IsValid())
	{
		bool Success;
		FItemData OutItem;
		int32 IndexOut;
		UE_LOG(LogInventory, Verbose, TEXT("[PickupItemActorDropped]Try to AddItemData"));
		TryToAddItemDataToInventory(Success,OutItem, IndexOut, GetInventory(GetFirstPlayerInventoryId()), Item, -1, false);
		
		if (Success)
		{
			UE_LOG(LogInventory,Verbose,TEXT("[PickupItemActorDropped] Success Destroyed Owner DroppedComponent with ActorChildren"));
			//Destroy Attached Actor
			TArray<AActor*> ActorChildren;
			DroppedComponent->GetOwner()->GetAttachedActors(ActorChildren);
			for (auto& Child : ActorChildren)
			{
				Child->Destroy();
			}

			DroppedComponent->GetOwner()->Destroy();
		}
		else
		{
			UE_LOG(LogInventory,Verbose,TEXT("[PickupItemActorDropped] Reamaning Item update ItemData on DroppedComponent"));
			DroppedComponent->Server_SetItemData(Item,false);
		}
	}
	else
	{
		UE_LOG(LogInventory,Warning,TEXT("[PickupItemActorDropped] ItemData Invalid"));
	}							
}

TArray<FItemData> UInventoryManagerComponent::GetAllItemsDataInPlayer(bool IncludePlayerInventory, bool IncludeEquipement, bool IncludeHotbar)
{
	TArray<FItemData> AllItemDataFound;

	if (IncludePlayerInventory)
	{
		for (auto& InvPlayer : PlayerInventoryIds)
		{
			if (UInventoryComponent* Inv = GetInventory(InvPlayer); IsValid(Inv))
			{
				for (auto Item : Inv->GetInventoryItems())
				{
					if (Item.IsValid())
					{
						AllItemDataFound.Add(Item);
					}
				}
			}
		}
		
		
	}
	if (IncludeEquipement)
	{
		for (auto Item : EquipementPlayerInventory->GetInventoryItems())
		{
			if (Item.IsValid())
			{
				AllItemDataFound.Add(Item);
			}
		}
	}
	if (IncludeHotbar)
	{
		for (auto Item : HotbarInventory->GetInventoryItems())
		{
			if (Item.IsValid())
			{
				AllItemDataFound.Add(Item);
			}
		}
	}
	return AllItemDataFound;
}



// ------------------------ MANAGER SYSTEM ------------------------




void UInventoryManagerComponent::TryToAddItemToInventory(bool& Success, FItemData& OutItem, int32& OutIndex,
	UInventoryComponent* Inventory, FPrimaryAssetId PrimaryAssetId, int32 CustomAmount, bool IsNewItem)
{
	
	// Check if asset is Valid 
	if (PrimaryAssetId.IsValid())
	{
		UE_LOG(LogInventory, Verbose, TEXT("[TryToAddItemToInventory] Try to add (Create New Item) : %s"), *PrimaryAssetId.PrimaryAssetName.ToString());
		FItemData Item = GetInvSubsystem()->GetItemDataFromPrimaryAssetId(PrimaryAssetId);
		// Init Decay
		if(Item.bCanDecay)
		{
			Item.Decay = Item.DecayTime;
		}
		TryToAddItemDataToInventory(Success,OutItem, OutIndex,Inventory,Item,CustomAmount, true);
	}
}

void UInventoryManagerComponent::TryToAddItemDataToInventory(bool& Success, FItemData& OutItem, int32& OutIndex,
	UInventoryComponent* Inventory, FItemData ItemData, int32 CustomAmount, bool IsNewItem)
{
	if (!IsValid(Inventory))
	{
		UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::TryToAddItemDataToInventory] InventoryComponent Invalid"));
		return;
	}


	// Check if asset is Valid 
	if (!ItemData.IsEmpty())
	{
		UE_LOG(LogInventory, Verbose, TEXT("[TryToAddItemDataToInventory] Try to add:[%s]"), *ItemData.ID.ToString());
		FItemData Item = ItemData;

		
		if (!Inventory->CheckItemInRestriction(Item))
		{
			return;
		}
		
		FName LocalItemID = Item.ID;
		if (CustomAmount != -1)
		{
			Item.Amount = CustomAmount;
			//UE_LOG(LogInventory, Warning, TEXT("[TryToAddItemDataToInventory] override Item Amount"));

		}
		int32 LocalItemAmount = Item.Amount;
		int32 DefaultAmount = LocalItemAmount;

		if (Item.IsCurrency())
		{
			// Management currency 
			SetCurrencyByItemId(Item.ID, GetCurrencyByType(Item.ID) + CustomAmount);
			Success = true;
			

		}
		else if (IsValid(Inventory))
		{
			// Calculate the number of turns to fill the inventory with the number of items you want to add
			int32 Ceil = (FMath::CeilToInt(float(Item.Amount) / float(Item.MaxStackable)));
			int32 NbLoop = FMath::Max(Ceil, 0);
			//UE_LOG(LogInventory, Warning, TEXT("NbLoop:  %d"), NbLoop);
			Success = false;
			for (int32 i = 0; i < NbLoop; i++)
			{
				if (Item.bIsStackable) // Check si Stackable
				{
					//UE_LOG(LogInventory, Warning, TEXT("tryToAdd:  %d"), LocalItemAmount);
					int32 TmpItemAmount = LocalItemAmount;
					LocalItemAmount = FindAndAddAmountToStack(Inventory, ItemData.ID, LocalItemAmount,ItemData.Decay);
					//UE_LOG(LogInventory, Warning, TEXT("tryToAdd Reaming :  %d"), LocalItemAmount);

					if (LocalItemAmount == TmpItemAmount) // No stack found then create a new stack if inventory is not full
					{
						bool InventorySpace;
						int32 IndexInventorySpace;
						Inventory->GetEmptyInventorySpace(InventorySpace, IndexInventorySpace, Item);
						if (InventorySpace && !Success)
						{
							// Calculates if he should fill the stack with his Max or just create a stack with the rest
							int32 NewAmount;
							if (LocalItemAmount <= Item.MaxStackable)
							{
								NewAmount = LocalItemAmount;
							}
							else
							{
								NewAmount = Item.MaxStackable;
							}
							// Add Item with New Value Amount
							Item.Amount = NewAmount;
							AddItem(Inventory, IndexInventorySpace, Item, IsNewItem); //@TODO A voir pour gestion des Attributes sur les Stack la je sais pas encore comment faire
							Item = Inventory->GetInventoryItem(IndexInventorySpace); // Update Data Item after ItemScript
							OutIndex = IndexInventorySpace;
							//Success = true;
							UE_LOG(LogInventory, Verbose, TEXT("[TryToAddItemDataToInventory] No Stack AddItem:[%d]"), Item.Amount);
							LocalItemAmount = LocalItemAmount - Item.Amount;
						}
						else
						{
							// Inventory full 
							OutItem = Item;
							Success = false;
							UE_LOG(LogInventory, Verbose, TEXT("[TryToAddItemDataToInventory] Inventory is Full"));
							break;
						}
					}
					if (LocalItemAmount == 0)
					{
						OutItem = Item;
						Success = true;
						UE_LOG(LogInventory, VeryVerbose, TEXT("[TryToAddItemDataToInventory] Reaming:0"));
					}
					else
					{	
						Item.Amount = LocalItemAmount;
						i--; //go back into the loop to create a new slot
						//UE_LOG(LogInventory, Warning, TEXT("Stack Found Pas tout pris il rest : %d"), Item.Amount);
					}
				}
				else // Not stackable so create a new item if there is room in the inventory
				{
					bool InventorySpace;
					int32 IndexInventorySpace;
					Inventory->GetEmptyInventorySpace(InventorySpace, IndexInventorySpace,Item); // Get inventory index where there is space
					if (InventorySpace && !Success)
					{
						Item.Amount = 1;
						AddItem(Inventory, IndexInventorySpace, Item,IsNewItem);
						Item = Inventory->GetInventoryItem(IndexInventorySpace); // Update Data Item after ItemScript
						OutIndex = IndexInventorySpace;
						UE_LOG(LogInventory, Verbose, TEXT("[TryToAddItemDataToInventory] New Stack AddItem :  %d"), Item.Amount);

						if (i == NbLoop - 1)
						{
							OutItem = Item;
							Success = true;
							LocalItemAmount = 0; // I create a stack so I indicate that there is nothing more to add
						}
						else
						{
							Success = false;
						}

					}
					else
					{
						// Inventory full 
					//	UE_LOG(LogInventory, Warning, TEXT("Inventory FULL"));
						OutItem = Item;

					}
				}
			}

			if (LocalItemAmount == 0)
			{
				OutItem = Item;
				Success = true;
				//UE_LOG(LogInventory, Warning, TEXT("All Item In Inventory Reaming 0"));
			}
			else
			{
				Success = false;
			}

			Inventory->OnItemEnterInventory.Broadcast(UChestoryFunctionLibrary::GetPrimaryAssetIdFromItemData(GetWorld(), OutItem), DefaultAmount - LocalItemAmount);
			
			if (Inventory->GetUseFilter())
			{
				FillInventorySlotVisible(Inventory->GetId());
			}
			
		}
		else
		{
			UE_LOG(LogInventory, Warning, TEXT("[TryToAddItemDataToInventory] Inventory NotValid"));
		}

	}
}

void UInventoryManagerComponent::TryToRemoveItemToInventory(bool& Success, int32& RemainingToRemove, UInventoryComponent* Inventory, FPrimaryAssetId PrimaryAssetId, int32 Amount)
{
	if (IsValid(Inventory) && PrimaryAssetId.IsValid())
	{
		//Success = false;
		bool HasItemAmount;
		int32 TotalAmountItem;
		int32 SaveAmount = Amount;
		CheckInventoryHasItemAmount(HasItemAmount, TotalAmountItem, Inventory, PrimaryAssetId, Amount);

		if (!GetIsCurrencyFromItemID(PrimaryAssetId.PrimaryAssetName))
		{
			for (int32 i = 0; i < Inventory->Inventory.Num(); i++)
			{
				if (Inventory->Inventory[i].ID == PrimaryAssetId.PrimaryAssetName)
				{
					bool SlotIsEmpty;
					FItemData OutItem;
					int32 AmountRemoved = 0;
					RemoveToItemAmount(SlotIsEmpty, OutItem, AmountRemoved, Inventory->Inventory[i], Amount);
					Amount = Amount - AmountRemoved;
					//	UE_LOG(LogInventory, Warning, TEXT("IdSlot: %d SlotIsEmpty: %s, AmountRemoved: %d"), i, (SlotIsEmpty ? TEXT("true") : TEXT("false")), AmountRemoved);
					RemainingToRemove = Amount - AmountRemoved;
					if (SlotIsEmpty)
					{
						RemoveItem(Inventory, i);
					}
					else
					{
						AddItem(Inventory, i, OutItem);
					}
					if (Amount <= 0)
					{
						Success = true;
						RemainingToRemove = 0;
						break;
					}
				}
			}
			if (Amount != 0)
			{
				Success = false;
			}
			
			Inventory->OnItemLeaveInventory.Broadcast(PrimaryAssetId, Success ? SaveAmount : SaveAmount - RemainingToRemove);
		}
		else
		{
			FItemData Item = GetInvSubsystem()->GetItemDataFromPrimaryAssetId(PrimaryAssetId);
			if (GetCurrencyByType(Item.ID) >= Amount)
			{
				SetCurrencyByItemId(Item.ID, GetCurrencyByType(Item.ID) - Amount);
				Success = true;
				RemainingToRemove = 0;
			}
			else
			{
				Success = false;
				//TODO Voir comment on fait sur le pas asser d'argent ?? 
			}
		}

	}
}

void UInventoryManagerComponent::CheckInventoryHasItemAmount(bool& HasItemAmount, int32& TotalAmountItem, UInventoryComponent* Inventory, FPrimaryAssetId PrimaryAssetId, int32 Amount)
{
	if (!GetIsCurrencyFromItemID(PrimaryAssetId.PrimaryAssetName))
	{
		//if not currency then check Item
		if (IsValid(Inventory))
		{
			if (Inventory->Inventory.Num() != 0)
			{
				TotalAmountItem = 0;
				for (int32 i = 0; i < Inventory->Inventory.Num(); i++)
				{
					if (Inventory->Inventory[i].ID == PrimaryAssetId.PrimaryAssetName)
					{
						TotalAmountItem = (TotalAmountItem + Inventory->Inventory[i].Amount);
					}
				}
				if (TotalAmountItem >= Amount)
				{
					HasItemAmount = true;
				}
				else
				{
					HasItemAmount = false;
				}
			}	
		}
	}
	else
	{
		const FItemData Item = GetInvSubsystem()->GetItemDataFromPrimaryAssetId(PrimaryAssetId);
		TotalAmountItem = GetCurrencyByType(Item.ID);
		if (GetCurrencyByType(Item.ID) >= Amount)
		{
			HasItemAmount = true;	
		}
		else
		{
			HasItemAmount = false;
		}
	}
}



bool UInventoryManagerComponent::CheckContainerHasItemCraft(TArray<FItemCraft> ItemsCraft, UInventoryComponent* Container)
{
	bool HasTotalItemsCraft = false;
	bool HasItemAmount = false;

	TArray<bool> HasItemsCraft;
	HasItemsCraft.AddDefaulted(ItemsCraft.Num());

	int32 TotalAmountItem = 0;
	int32 CurrentAmountCheck = 0;


	for (int32 i = 0; i < ItemsCraft.Num(); i++)
	{
		if (IsValid(Container))
		{
			CheckInventoryHasItemAmount(HasItemAmount, TotalAmountItem, Container, ItemsCraft[i].PrimaryAssetId, ItemsCraft[i].Amount);

			if (HasItemAmount)
			{
				HasItemsCraft[i] = true;
			}
			else
			{
				CurrentAmountCheck += TotalAmountItem;

				if (CurrentAmountCheck >= ItemsCraft[i].Amount)
				{
					HasItemsCraft[i] = true;
				}
			}
		}
	}

	for (int32 i = 0; i < HasItemsCraft.Num(); i++)
	{
		if (!HasItemsCraft[i])
		{
			return false;
			//	UE_LOG(LogInventory, Warning, TEXT("No Item: %s"), *ItemsCraft[i].ID.ToString());
		}
	}

	// If all list a check is true
	return true;
}

const TArray<UItemScript*> UInventoryManagerComponent::GetItemScripts(FName ItemID, bool AddGeneralItemScript /*= true*/, bool ExcludeInstance /*= true*/)
{

	if (ItemID == "None") { return TArray<UItemScript*>(); }

	TArray<UItemScript*> DefaultScript = UChestoryFunctionLibrary::GetItemScriptsFromPrimaryAssetId(this, GetInvSubsystem()->GetPrimaryAssetIdFromName(ItemID));
	TArray<UItemScript*> ListScripts;

	if (ExcludeInstance)
	{
		//Exclude Instance Script
		for (auto Script : DefaultScript)
		{
			if (!Script->bEnableItemScriptInstance)
			{
				ListScripts.Add(Script);
			}
		}
	}
	else
	{
		ListScripts = DefaultScript;
	}
	


	//Adding General ItemScript
	if (AddGeneralItemScript)
	{
		ListScripts.Append(GeneralItemScript);
	}

	if (ListScripts.IsEmpty())
	{
		return TArray<UItemScript*>();
	}
	else
	{	

		return ListScripts;
	}
}



void UInventoryManagerComponent::SetCurrentItemOnMouseHover(bool CurrentIsValid, FPrimaryAssetId PrimaryAssetId, FItemDataInfo ItemInformation, ESlotType Type, int32 InventorySlot, UInventorySlotUserWidget* InventorySlotUserWidget, FGuid InventoryId)
{

	if (CurrentIsValid)
	{
		HoverCurrentIsValid = CurrentIsValid;
		HoverItemInformation = ItemInformation;
		HoverType = Type;
		HoverInventorySlot = InventorySlot;
		HoverPrimaryAssetId = PrimaryAssetId;
		HoverInventorySlotUserWidget = InventorySlotUserWidget;
		HoverInventoryId = InventoryId;

		Server_SetRepairInfoSlot(InventoryId,HoverInventorySlot);
		Server_SetItemBlueprintInfoSlot(InventoryId,HoverInventorySlot);

		
		//Call function Update Repair Data
	}
	else
	{
		HoverCurrentIsValid = false;
		HoverItemInformation = FItemDataInfo();
		HoverType = ESlotType::EInventory;
		HoverInventorySlot = -1;
		PrimaryAssetId = FPrimaryAssetId();
		HoverInventorySlotUserWidget = nullptr;
		HoverInventoryId = FGuid();
	}
}

void UInventoryManagerComponent::GetCurrentItemOnMouseHover(bool& Success, FGuid& InventoryId, int32& InventorySlot, FPrimaryAssetId& PrimaryAssetId, FItemDataInfo& ItemInformation, ESlotType& Type,  UInventorySlotUserWidget*& HoverSlotUserWidget)
{
	Success = HoverCurrentIsValid;
	ItemInformation = HoverItemInformation;
	Type = HoverType;
	InventorySlot = HoverInventorySlot;
	PrimaryAssetId = HoverPrimaryAssetId;
	HoverSlotUserWidget = HoverInventorySlotUserWidget;
	InventoryId = HoverInventoryId;
}

void UInventoryManagerComponent::LMBPressedOnSlot()
{
	OnLeftMousePressedOnSlot.Broadcast();
	TryToTransferById();

}

void UInventoryManagerComponent::RMBPressedOnSlot()
{
	OnRightMousePressedOnSlot.Broadcast();
}



void UInventoryManagerComponent::LoadItemsOnInventory_Implementation(UInventoryComponent* InInventory, const TArray<FItem>& Items)
{

	for (int32 i = 0; i < Items.Num(); i++)
	{
		FItemData LocalItemData = UChestoryFunctionLibrary::MakeItem(
			this, 
			Items[i].PrimaryAssetId, 
			Items[i].Amount, 
			Items[i].Decay, 
			Items[i].Durability, 
			Items[i].Attributes, 
			Items[i].Sockets,
			Items[i].JsonParameter);

		LocalItemData = LocalItemData.NotifyScript_ItemPostLoad(this, InInventory->GetId(), i);

		UE_LOG(LogInventory, Warning, TEXT("[LoadItemsOnInventory] -------------------------------------------- Trigger Event ItemAddInInventory in ItemScript"));
		TArray<int32> FilledIndex;
		InInventory->SetInventoryItem(Items[i].IndexSlot, LocalItemData,FilledIndex);
		LocalItemData.NotifyScript_ItemAdd(this, InInventory->GetId(), i, InInventory);
	}
}



FItemForSave UInventoryManagerComponent::GetItemsForSave()
{
	FItemForSave ItemsSave;
	TArray<FItem> ListToSave;
	TArray<FItemCurrency> CurrencySave;

	for (auto& Inventory : PlayerInventorys)
	{
		if (IsValid(Inventory.Value))
		{
			ListToSave.Append(UChestoryFunctionLibrary::MakeItemSaveFromInventory(GetWorld(), Inventory.Value));
		}
	}
	
	
	if (IsValid(EquipementPlayerInventory))
	{
		ListToSave.Append(UChestoryFunctionLibrary::MakeItemSaveFromInventory(GetWorld(), EquipementPlayerInventory,true));
	}
	if (IsValid(HotbarInventory))
	{
		ListToSave.Append(UChestoryFunctionLibrary::MakeItemSaveFromInventory(GetWorld(), HotbarInventory, false,true));
	}

	ItemsSave.ItemsInventory = ListToSave;
	ItemsSave.Currency = GetCurrencyForSave();

	return ItemsSave;
}

FInventoryManagerSaved UInventoryManagerComponent::GetInventoryManagerSave()
{
	FInventoryManagerSaved OutInventoryManagerSaved;
	OutInventoryManagerSaved.Currency = GetCurrencyForSave();

	for (TPair<FName,UInventoryComponent*> Inv : PlayerInventorys)
	{
		OutInventoryManagerSaved.NamePlayerInventory.Add(Inv.Key);
		OutInventoryManagerSaved.PlayerInventory.Add(Inv.Value->GetInventorySave());
	}

	OutInventoryManagerSaved.EquipmentInventory = EquipementPlayerInventory->GetInventorySave();
	OutInventoryManagerSaved.HotbarInventory = HotbarInventory->GetInventorySave();

	//TODO PlayerInventoryIds ??

	return OutInventoryManagerSaved;
}

void UInventoryManagerComponent::LoadInventoryManagerFromSave(FInventoryManagerSaved InventoryManagerSaved)
{
	//player


	int32 LoopIndex = -1;
	for (const auto& IMCS : InventoryManagerSaved.PlayerInventory)
	{
		LoopIndex++;
		if (!PlayerInventorys.Contains(InventoryManagerSaved.NamePlayerInventory[LoopIndex]))
		{
			continue;
		}

		if (UInventoryComponent* Inv = *PlayerInventorys.Find(InventoryManagerSaved.NamePlayerInventory[LoopIndex]))
		{
			Inv->LoadInventoryFromSave(IMCS);
		}
	}

	EquipementPlayerInventory->LoadInventoryFromSave(InventoryManagerSaved.EquipmentInventory);
	HotbarInventory->LoadInventoryFromSave(InventoryManagerSaved.HotbarInventory);

	AllCurrency = InventoryManagerSaved.Currency;
}

void UInventoryManagerComponent::Server_InitializeInventory_Implementation(UInventoryComponent* Inventory)
{
	Inventory->Server_InitializeInventory();
}

void UInventoryManagerComponent::Server_InitializeEquipement_Implementation(int32 LengthFounds)
{
	EquipementPlayerInventory->Server_InitializeInventory(LengthFounds);
}

void UInventoryManagerComponent::Server_InitializeHotbar_Implementation(int32 LengthFounds)
{
	HotbarInventory->Server_InitializeInventory(LengthFounds);
}



void UInventoryManagerComponent::LoadInventoryManagerFromDataTable_Implementation(FDataTableRowHandle RowHandle)
{
	if (!IsValid(RowHandle.DataTable))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::LoadInventoryManagerFromDataTable] DataTable Invalid"));
		return;
	}

	for (auto& Inventory : Inventorys)
	{
		Server_Start_Inventorys(Inventory.Key, RowHandle);
	}

	Server_Start_Equipments(RowHandle);
	Server_Start_Hotbars(RowHandle);


	RefreshEquipementSlots();
	RefreshHotbarSlots();

	//UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::LoadInventoryManagerFromDataTable] End Load"));

}


void UInventoryManagerComponent::Server_Start_Inventorys(FName InventoryKey, FDataTableRowHandle RowHandle)
{
	FItemStarter* Items = new FItemStarter();

	if (IsValid(RowHandle.DataTable))
	{
		FString Context;
		Items = RowHandle.DataTable->FindRow<FItemStarter>(RowHandle.RowName, Context, false);
	}

	if (Items == nullptr)
	{
		return;
	}

	for (auto& Item : Items->Items)
	{
		//Pass other Inventory
		if (Item.bInEquipement || Item.bInHotbar)
		{
			continue;
		}

		if (Item.InventoryKey.IsNone())
		{
			UE_LOG(LogInventory, Display, TEXT("[UInventoryManagerComponent::Server_Start_Inventorys] InventoryKey Invalid [%s]"), *Item.InventoryKey.ToString())
			continue;
		}

		if (Item.InventoryKey != InventoryKey)
		{
			continue;
		}

		if (UInventoryComponent* Inventory = *PlayerInventorys.Find(Item.InventoryKey))
		{
			bool bSuccess = false;
			int32 IndexFound = 0;
			FItemData Data = UChestoryFunctionLibrary::MakeItemDataFromItem(this, Item);
			Inventory->GetEmptyInventorySpace(bSuccess, IndexFound, Data);
			
			if (bSuccess)
			{
				if (Data.IsCurrency())
				{
					SetCurrencyByItemId(Data.ID, Data.Amount);
				}
				else
				{
					AddItem(Inventory, IndexFound, Data, Item.bNotifyItemCreation);
					
					/*TArray<int32> FilledIndex;
					Inventory->SetInventoryItem(IndexFound, Data,FilledIndex);*/
				}
			}
			else
			{
				UE_LOG(LogInventory, Display, TEXT("[UInventoryManagerComponent::Server_Start_Inventorys] No space found in this inventory InventoryKey:[%s]"), *Item.InventoryKey.ToString())
			}
		}
		else
		{
			UE_LOG(LogInventory, Display, TEXT("[UInventoryManagerComponent::Server_Start_Inventorys] InventoryComponent Invalid"))
		}
	}
}

void UInventoryManagerComponent::Server_Start_Equipments(FDataTableRowHandle RowHandle)
{
	FItemStarter* Items = new FItemStarter();

	if (IsValid(RowHandle.DataTable))
	{
		FString Context;
		Items = RowHandle.DataTable->FindRow<FItemStarter>(RowHandle.RowName, Context, false);
	}

	if (Items == nullptr)
	{
		return;
	}

	for (auto& Item : Items->Items)
	{
		//Pass other Inventory
		if (!Item.bInEquipement)
		{
			continue;
		}

		if (Item.SlotKey.IsNone())
		{
			UE_LOG(LogInventory, Display, TEXT("[UInventoryManagerComponent::Server_Start_Equipments] Invalid SlotKey:[%s]"), *Item.SlotKey.ToString())
			continue;
		}

		if (IsValid(EquipementPlayerInventory))
		{
			if (!EquipementPlayerInventory->SlotsIndexKey.Contains(Item.SlotKey))
			{
				UE_LOG(LogInventory,Display,TEXT("[UInventoryManagerComponent::Server_Start_Equipments] Equipment slot not found with SlotKey:[%s]"),*Item.SlotKey.ToString())
				continue;
			}

			int32 Index = EquipementPlayerInventory->GetIndexAtKey(Item.SlotKey);
			UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::Server_Start_Equipments] Equipment slot SlotKey:[%s] At Index:[%d]"), *Item.SlotKey.ToString(), Index)
			AddItem(EquipementPlayerInventory, Index, UChestoryFunctionLibrary::MakeItemDataFromItem(this, Item), Item.bNotifyItemCreation);
			
		/*	TArray<int32> FilledIndex;
			EquipementPlayerInventory->SetInventoryItem(Index, UChestoryFunctionLibrary::MakeItemDataFromItem(this, Item),FilledIndex);*/
		}
	}
}

void UInventoryManagerComponent::Server_Start_Hotbars(FDataTableRowHandle RowHandle)
{
	FItemStarter* Items = new FItemStarter();

	if (IsValid(RowHandle.DataTable))
	{
		FString Context;
		Items = RowHandle.DataTable->FindRow<FItemStarter>(RowHandle.RowName, Context, false);
	}

	if (Items == nullptr)
	{
		return;
	}

	for (auto& Item : Items->Items)
	{
		//Pass other Inventory
		if (!Item.bInHotbar)
		{
			continue;
		}

		if (Item.SlotKey.IsNone())
		{
			UE_LOG(LogInventory, Display, TEXT("[UInventoryManagerComponent::Server_Start_Hotbars] Invalid SlotKey:[%s]"), *Item.SlotKey.ToString())
			continue;
		}

		if (IsValid(HotbarInventory))
		{
			if (!HotbarInventory->SlotsIndexKey.Contains(Item.SlotKey))
			{
				UE_LOG(LogInventory, Display, TEXT("[UInventoryManagerComponent::Server_Start_Hotbars] Hotbar slot not found with SlotKey:[%s]"), *Item.SlotKey.ToString())
				continue;
			}

			int32 Index = HotbarInventory->GetIndexAtKey(Item.SlotKey);
			UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::Server_Start_Hotbars] Hotbar slot SlotKey:[%s] At Index:[%d]"), *Item.SlotKey.ToString(), Index)
			AddItem(HotbarInventory, Index, UChestoryFunctionLibrary::MakeItemDataFromItem(this, Item), Item.bNotifyItemCreation);		
				
			/*TArray<int32> FilledIndex;
			HotbarInventory->SetInventoryItem(Index, UChestoryFunctionLibrary::MakeItemDataFromItem(this, Item),FilledIndex);*/
			
		}
	}
}


void UInventoryManagerComponent::BuildInstanceItemScript(FItemData& Item)
{
	Item.ItemScriptInstance.Empty();

	for (auto script : GetItemScripts(Item.ID, true, false))
	{
		if (script->bEnableItemScriptInstance)
		{
			UItemScript* ScriptInstance = DuplicateObject(script, this);
			ScriptInstance->Init(this, Item);
			Item.ItemScriptInstance.Add(ScriptInstance);
			UE_LOG(LogInventory,Verbose,TEXT("[%hs] Build ItemScript Instance Item:[%s] UniqueId:[%s] ItemScript:[%s]"),__FUNCTION__, *Item.ID.ToString(), *Item.UniqueId.ToString(),*ScriptInstance->GetFName().ToString())
		}
	}
}

FItemData UInventoryManagerComponent::GenerateItem(FItemData& Item)
{

	if (Item.UniqueId.IsValid())
	{
		//UE_LOG(LogInventory,Warning,TEXT("Item has already Unique Id"))
		return FItemData();
	}

	Item.UniqueId = FGuid::NewGuid();
	//UE_LOG(LogInventory, Warning, TEXT("Item Generate new Unique Id"))


	


	return Item;
}


void UInventoryManagerComponent::AddItem_Implementation(UInventoryComponent* Inventory, int32 InventorySlot, FItemData InventoryItem, bool IsNewItem)
{
	if (!IsValid(Inventory))
	{	
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::AddItem]  InventoryComponent Invalid"));
	}

	// Set Visibility slot if Filter is True 
	if (Inventory->GetUseFilter())
	{
		InventoryItem.bIsVisible = InventoryItem.GameplayTagContainer.HasAny(Inventory->GetFilterTags());
	}

	GenerateItem(InventoryItem);

	// Call ItemCreation for PreBuild item
	if (IsNewItem)
	{
		BuildInstanceItemScript(InventoryItem);

		FItemData CopyItem = InventoryItem;
		InventoryItem = CopyItem.NotifyScript_ItemCreation(this, Inventory->GetId(), InventorySlot);
	}

	TArray<int32> FilledIndex;
	//Server change data
	Inventory->SetInventoryItem(InventorySlot, InventoryItem, FilledIndex);


	// Call Event ItemAddInInventory in script and Update Index InventorySlot
	InventoryItem.NotifyScript_ItemAdd(this, Inventory->GetId(), InventorySlot, Inventory);

	//Get Item with new value if is modified with Item Script AddInInventory, send default greate value
	FItemDataInfoCompressed LocalItemInfoCompressed = UChestoryFunctionLibrary::MakeItemTransfer(Inventory->GetInventoryItem(InventorySlot));

	//Server Send Data to Client
	SetViewersInventorySlot(Inventory->GetId(), Inventory->GetViewers(), InventorySlot, LocalItemInfoCompressed);

	if (Inventory->EnableSizeItem)
	{
		SetViewersFilledInventorySlot(Inventory->GetId(), Inventory->GetViewers(), FilledIndex);
	}

	//Check if Can Repair this Item
	Server_SetRepairInfoSlot(Inventory->GetId(), InventorySlot);

	//Check If New item
	if (IsNewItem)
	{
		InventoryItem.NotifyScript_ItemCreated(this, Inventory->GetId(), InventorySlot, Inventory);
	}

	OnUpdateInventory.Broadcast(Inventory->GetId(), InventorySlot, true, InventoryItem);
}

void UInventoryManagerComponent::RemoveItem_Implementation(UInventoryComponent* Inventory, int32 InventorySlot, bool Unfilled /*= true*/)
{
	if (IsValid(Inventory))
	{
		FItemData ItemRemoved = Inventory->GetInventoryItem(InventorySlot);
		// Call Event ItemRemoveInInventory in script and Update Index InventorySlot
		//Trigger_Script_ItemRemoveInInventory(this,ItemRemoved.ID, Inventory->GetId(),InventorySlot,Inventory);
		ItemRemoved.NotifyScript_ItemRemove(this, Inventory->GetId(), InventorySlot, Inventory);
		
		TArray<int32> UnfilledSlot;
		Inventory->ClearInventoryItem(InventorySlot, UnfilledSlot);

		//Server Send Data to Client
		ClearViewersInventorySlot(Inventory->InventoryID, Inventory->GetViewers(), InventorySlot);

		if (Unfilled && Inventory->EnableSizeItem)
		{
			SetViewersUnfilledInventorySlot(Inventory->InventoryID, Inventory->GetViewers(), UnfilledSlot);
		}


		OnUpdateInventory.Broadcast(Inventory->GetId(), InventorySlot, false, ItemRemoved);

		if (Inventory->EnableAutoFill)
		{
			FillInventorySlotVisible(Inventory->GetId(), InventorySlot);
		}

	}
}

void UInventoryManagerComponent::HasItem(UInventoryComponent* Inventory, FName ItemID, bool& Success, int32& InventorySlot)
{
	if (IsValid(Inventory))
	{
		for (int32 i = 0; i < Inventory->Inventory.Num(); i++)
		{
			if (Inventory->Inventory[i].ID == ItemID)
			{
				Success = true;
				InventorySlot = i;
				break;
			}
			else
			{
				Success = false;
				InventorySlot = -1;
			}
		}
	}
}

void UInventoryManagerComponent::DropItemData_Implementation(UInventoryComponent* Inventory, int32 InventorySlot, FItemData ItemData)
{
	if (ItemData.bIsDroppable)
	{
		UClass* MyClass = ItemData.ActorDroped;
		FVector SpawnLocation = Inventory->GetOwnerForLocation()->GetActorLocation();
		FRotator SpawnRotator = Inventory->GetOwnerForLocation()->GetActorRotation();

		FPrimaryAssetId MyPrimaryAsset = GetInvSubsystem()->GetPrimaryAssetIdFromName(ItemData.ID);

	
		Inventory->bIsContainer ? SpawnLocation += Inventory->DroppedLocation : SpawnLocation += DroppedLocation;

		auto MyItemActor = Cast<AActor>(UGameplayStatics::BeginDeferredActorSpawnFromClass(
			GetWorld(),
			MyClass,
			FTransform(SpawnRotator, SpawnLocation),
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn,
			GetOwner()));
		//UE_LOG(LogInventory, Warning, TEXT("SpawnActor"));
		if (IsValid(MyItemActor))
		{
			//UE_LOG(LogInventory, Warning, TEXT("Init new Item Actor"));
			MyItemActor->SetReplicates(true);
			MyItemActor->SetOwner(GetOwner());

			UGameplayStatics::FinishSpawningActor(MyItemActor, FTransform(SpawnRotator, SpawnLocation));

			// Apply Data In dropped Component
			TArray<UDroppedComponent*> Comps;
			MyItemActor->GetComponents(Comps);
			if (Comps.IsValidIndex(0))
			{
				UDroppedComponent* DroppedComponent = Comps[0];
				if (DroppedComponent)
				{
					DroppedComponent->Server_SetItemData(ItemData, true);
				}
			}
			else
			{
				UE_LOG(LogInventory, Warning, TEXT("[DropItem] DroppedComponent no found"));
			}

			//Notify Script Dropped
			ItemData.NotifyScript_ItemDropped(this, MyItemActor, Inventory->GetId(), InventorySlot);
		}
	}
}

void UInventoryManagerComponent::DropItem_Implementation(UInventoryComponent* Inventory, int32 InventorySlot)
{
	if (IsValid(Inventory))
	{
		FItemData ItemData;

		ItemData = Inventory->GetInventoryItem(InventorySlot);

		if (ItemData.bIsDroppable)
		{
			FPrimaryAssetId MyPrimaryAsset = GetInvSubsystem()->GetPrimaryAssetIdFromName(ItemData.ID);


			if (ItemData.IsEquipment() && Inventory->GetIsEquipmentInventory() && ItemData.bDropActorEquipment)
			{
				if (!ItemData.NotifyScript_GetCanUnequip(this, Inventory->GetId(), InventorySlot,FGuid(),-1, ItemData.ActorInWorld, true))
				{
					return;
				}

				if (IsValid(ItemData.ActorInWorld))
				{
					FDetachmentTransformRules Rules = FDetachmentTransformRules::KeepWorldTransform;
					ItemData.ActorInWorld->DetachFromActor(Rules);
				}

				//Save Data and Update Variable
				Server_DestoryActorEquipment(Inventory, InventorySlot, true);
				ItemData = Inventory->GetInventoryItem(InventorySlot);

				// Apply Data In dropped Component
				TArray<UDroppedComponent*> Comps;
				ItemData.ActorInWorld->GetComponents(Comps);
				if (Comps.IsValidIndex(0))
				{
					UDroppedComponent* DroppedComponent = Comps[0];
					if (DroppedComponent)
					{
						UE_LOG(LogInventory, Warning, TEXT("[DropItem] DroppedComponent found Set Init Drop Comp"));

						DroppedComponent->Server_SetItemData(ItemData, true);
					}
				}
				else
				{
					UE_LOG(LogInventory, Warning, TEXT("[DropItem] DroppedComponent no found"));
				}


				//Notify Script Dropped
				ItemData.NotifyScript_ItemDropped(this, ItemData.ActorInWorld, Inventory->GetId(), InventorySlot);

				OnUnequip.Broadcast(Inventory->GetKeyAtIndex(InventorySlot), Inventory->GetInventoryItem(InventorySlot), ItemData.ActorInWorld);

				ItemData.ActorInWorld = nullptr;


			}
			else
			{

				if (!ItemData.NotifyScript_GetCanDrop(this, Inventory->GetId(), InventorySlot))
				{
					return;
				}

				UClass* MyClass = ItemData.ActorDroped;
				FVector SpawnLocation = Inventory->GetOwnerForLocation()->GetActorLocation();
				FRotator SpawnRotator = Inventory->GetOwnerForLocation()->GetActorRotation();

				Inventory->bIsContainer ? SpawnLocation += Inventory->DroppedLocation : SpawnLocation += DroppedLocation;

				auto MyItemActor = Cast<AActor>(UGameplayStatics::BeginDeferredActorSpawnFromClass(
					GetWorld(),
					MyClass,
					FTransform(SpawnRotator, SpawnLocation),
					ESpawnActorCollisionHandlingMethod::AlwaysSpawn,
					GetOwner()));
				//UE_LOG(LogInventory, Warning, TEXT("SpawnActor"));
				if (IsValid(MyItemActor))
				{
					//UE_LOG(LogInventory, Warning, TEXT("Init new Item Actor"));
					MyItemActor->SetReplicates(true);
					MyItemActor->SetOwner(GetOwner());

					UGameplayStatics::FinishSpawningActor(MyItemActor, FTransform(SpawnRotator, SpawnLocation));

					// Apply Data In dropped Component
					TArray<UDroppedComponent*> Comps;
					MyItemActor->GetComponents(Comps);
					if (Comps.IsValidIndex(0))
					{
						UDroppedComponent* DroppedComponent = Comps[0];
						if (DroppedComponent)
						{
							DroppedComponent->Server_SetItemData(ItemData, true);
						}
					}
					else
					{
						UE_LOG(LogInventory, Warning, TEXT("[DropItem] DroppedComponent no found"));
					}


					//Notify Script	Dropped
					ItemData.NotifyScript_ItemDropped(this, MyItemActor, Inventory->GetId(), InventorySlot);
				}
			

			}



			RemoveItem(Inventory, InventorySlot);

			Inventory->OnItemLeaveInventory.Broadcast(MyPrimaryAsset, ItemData.Amount);
		}
	}
	
}

void UInventoryManagerComponent::MoveItem_Implementation(UInventoryComponent* FromInventory, int32 FromInventorySlot, UInventoryComponent* ToInventory, int32 ToInventorySlot)
{
	// Check if the inventories and slots are valid
	if (!FromInventory || !ToInventory || (FromInventory == ToInventory && FromInventorySlot == ToInventorySlot))
	{
		return;
	}

	// Check if the destination inventory is a container and if it is enabled for looting
	if (ToInventory->bIsContainer && ToInventory->EnableLootContainer)
	{
		return;
	}

	// Get the item data from the source inventory slot
	FItemData LocalInventoryItem = FromInventory->GetInventoryItem(FromInventorySlot);

	// Check if the item can be placed in the destination inventory slot
	if (!ToInventory->CheckItemInRestriction(LocalInventoryItem, ToInventorySlot))
	{
		Server_CancelMoveInventory(FromInventory->GetId(), FromInventorySlot, ToInventory->GetId(), ToInventorySlot);
		return;
	}



	// Handle the item movement if it is draggable
	if (LocalInventoryItem.bIsDraggable)
	{
		if (LocalInventoryItem.ItemType == EItemType::ECurrency)
		{
			// Handle currency items (implementation not provided)
		}
		else
		{
			// Notify CanMove ItemScript
			if (!LocalInventoryItem.NotifyScript_GetCanMove(this, FromInventory->GetId(), FromInventorySlot, ToInventory->GetId(), ToInventorySlot))
			{
				return;
			}

			// Get the item data from the destination inventory slot
			FItemData LocalSwapInventoryItem = ToInventory->GetInventoryItem(ToInventorySlot);

			// Check if the destination slot has a valid item
			if (ToInventory->GetItemIsValid(LocalSwapInventoryItem))
			{
				if (!LocalSwapInventoryItem.bIsDraggable)
				{
					Server_CancelMoveInventory(FromInventory->GetId(), FromInventorySlot, ToInventory->GetId(), ToInventorySlot);
					return;
				}				

				// Check if the items are the same for stacking
				if (EqualItem(LocalInventoryItem, LocalSwapInventoryItem))
				{
					HandleItemStack(FromInventory, FromInventorySlot, ToInventory, ToInventorySlot, LocalInventoryItem);
				}
				else
				{
					HandleItemSwap(FromInventory, FromInventorySlot, ToInventory, ToInventorySlot, LocalInventoryItem, LocalSwapInventoryItem);
				}
			}
			else
			{
				HandleItemMove(FromInventory, FromInventorySlot, ToInventory, ToInventorySlot, LocalInventoryItem);
			}
		}
	}
}

void UInventoryManagerComponent::HandleItemStack(UInventoryComponent* FromInventory, int32 FromInventorySlot, UInventoryComponent* ToInventory, int32 ToInventorySlot, FItemData& LocalInventoryItem)
{
	// Add the item to the stack in the destination inventory
	int32 AmountRemaining = AddItemToStack(ToInventory, ToInventorySlot, LocalInventoryItem.Amount, LocalInventoryItem.Decay);
	if (AmountRemaining > 0)
	{
		LocalInventoryItem.Amount = AmountRemaining;
		AddItem(FromInventory, FromInventorySlot, LocalInventoryItem);
	}
	else
	{
		RemoveItem(FromInventory, FromInventorySlot);
	}

	// Broadcast the item movement
	BroadcastItemMovement(FromInventory, LocalInventoryItem, FItemData());
	BroadcastItemMovement(ToInventory, FItemData(), LocalInventoryItem);
}

void UInventoryManagerComponent::HandleItemSwap(UInventoryComponent* FromInventory, int32 FromInventorySlot, UInventoryComponent* ToInventory, int32 ToInventorySlot, FItemData& LocalInventoryItem, FItemData& LocalSwapInventoryItem)
{
	// Check if the item has a modifier script
	if (LocalInventoryItem.NotifyScript_GetIsItemModifier(this))
	{
		LocalInventoryItem.NotifyScript_ItemModifier(this, FromInventory->GetId(), FromInventorySlot, ToInventory->GetId(), ToInventorySlot);
	}
	else
	{
		//Check FromInventory Have restriction
		if (!FromInventory->CheckItemInRestriction(LocalSwapInventoryItem, FromInventorySlot))
		{
			Server_CancelMoveInventory(FromInventory->GetId(), FromInventorySlot, ToInventory->GetId(), ToInventorySlot);
			return;
		}


		// Check if there is enough space in both inventories for the swap
		if (!ToInventory->HaveSapceInSlot(LocalInventoryItem, FromInventorySlot, ToInventorySlot, true) ||
			!FromInventory->HaveSapceInSlot(LocalSwapInventoryItem, ToInventorySlot, FromInventorySlot, true))
		{
			Server_CancelMoveInventory(FromInventory->GetId(), FromInventorySlot, ToInventory->GetId(), ToInventorySlot);
			return;
		}

		// Unfill the slots in both inventories
		Server_UnfilledItemInventory(FromInventory->GetId(), FromInventorySlot);
		Server_UnfilledItemInventory(ToInventory->GetId(), ToInventorySlot);

		// Swap the items between the inventories
		AddItem(ToInventory, ToInventorySlot, LocalInventoryItem);
		AddItem(FromInventory, FromInventorySlot, LocalSwapInventoryItem);

		// Broadcast the item movement if the inventories are different
		if (FromInventory != ToInventory)
		{
			BroadcastItemMovement(FromInventory, LocalInventoryItem, LocalSwapInventoryItem);
			BroadcastItemMovement(ToInventory, LocalSwapInventoryItem, LocalInventoryItem);
		}
	}
}

void UInventoryManagerComponent::HandleItemMove(UInventoryComponent* FromInventory, int32 FromInventorySlot, UInventoryComponent* ToInventory, int32 ToInventorySlot, FItemData& LocalInventoryItem)
{
	// Check if the item can be auto-filled in the destination inventory
	if (!CheckAutoFillInventory(LocalInventoryItem, ToInventory, ToInventorySlot))
	{
		Server_CancelMoveInventory(FromInventory->GetId(), FromInventorySlot, ToInventory->GetId(), ToInventorySlot);
		return;
	}

	// Check if there is enough space in the destination inventory
	if (!ToInventory->HaveSapceInSlot(LocalInventoryItem, FromInventorySlot, ToInventorySlot))
	{
		Server_CancelMoveInventory(FromInventory->GetId(), FromInventorySlot, ToInventory->GetId(), ToInventorySlot);
		return;
	}

	// Remove the item from the source inventory and add it to the destination inventory
	RemoveItem(FromInventory, FromInventorySlot, false);
	CheckAutoFillInventory(LocalInventoryItem, ToInventory, ToInventorySlot);
	AddItem(ToInventory, ToInventorySlot, LocalInventoryItem);
	Server_UnfilledItemInventory(FromInventory->GetId(), FromInventorySlot, ToInventorySlot);

	// Broadcast the item movement if the inventories are different
	if (FromInventory != ToInventory)
	{
		BroadcastItemMovement(FromInventory, LocalInventoryItem, FItemData());
		BroadcastItemMovement(ToInventory, FItemData(), LocalInventoryItem);
	}
}


void UInventoryManagerComponent::EquipItem_Implementation(UInventoryComponent* FromInventory, int32 FromInventorySlot, UInventoryComponent* ToInventory, int32 ToInventorySlot)
{
	// Check if the inventories and slots are valid
	if (!IsValid(FromInventory) || !IsValid(ToInventory))
	{
		return;
	}

	if (FromInventory == ToInventory && FromInventorySlot == ToInventorySlot)
	{
		return;
	}

	// Get the item data from the source inventory slot
	FItemData LocalInventoryItem = FromInventory->GetInventoryItem(FromInventorySlot);

	// Check if the item can be equipped based on restrictions
	if (!ToInventory->CheckItemInRestriction(LocalInventoryItem, ToInventorySlot) && !LocalInventoryItem.NotifyScript_GetIsItemModifier(this))
	{
		UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::EquipItem] Cannot be equipped, restrictions not met"));
		Server_CancelMoveInventory(FromInventory->GetId(), FromInventorySlot, ToInventory->GetId(), ToInventorySlot);
		return;
	}

	// Check if the item can be equipped based on item scripts
	if (!LocalInventoryItem.NotifyScript_GetCanEquip(this, FromInventory->GetId(), FromInventorySlot, ToInventory->GetId(), ToInventorySlot))
	{
		UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::EquipItem] Cannot be equipped, item script returned false"));
		Server_CancelMoveInventory(FromInventory->GetId(), FromInventorySlot, ToInventory->GetId(), ToInventorySlot);
		return;
	}

	// Check if the slot is disabled
	if (!CanBeEquip(LocalInventoryItem.GameplayTagContainer) && !LocalInventoryItem.NotifyScript_GetIsItemModifier(this))
	{
		UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::EquipItem] Cannot be equipped, type tag is in DisableSlot"));
		Server_CancelMoveInventory(FromInventory->GetId(), FromInventorySlot, ToInventory->GetId(), ToInventorySlot);
		return;
	}

	// Handle equipment items
	if (LocalInventoryItem.IsEquipment())
	{
		// Get the item data from the target inventory slot
		FItemData LocalSwapEquipementItem = ToInventory->GetInventoryItem(ToInventorySlot);

		// Check if the target slot has a valid item
		if (ToInventory->GetItemIsValid(LocalSwapEquipementItem))
		{
			HandleItemSwapEquip(FromInventory, FromInventorySlot, ToInventory, ToInventorySlot, LocalInventoryItem, LocalSwapEquipementItem);
		}
		else
		{
			HandleItemEquip(FromInventory, FromInventorySlot, ToInventory, ToInventorySlot, LocalInventoryItem);
		}
	}

	// If the item is a modifier, trigger the modifier event
	if (LocalInventoryItem.NotifyScript_GetIsItemModifier(this))
	{
		LocalInventoryItem.NotifyScript_ItemModifier(this, FromInventory->GetId(), FromInventorySlot, ToInventory->GetId(), ToInventorySlot);
	}
}

void UInventoryManagerComponent::HandleItemEquip(UInventoryComponent* FromInventory, int32 FromInventorySlot, UInventoryComponent* ToInventory, int32 ToInventorySlot, FItemData& LocalInventoryItem)
{
	// Move the item to the target slot
	AddItem(ToInventory, ToInventorySlot, LocalInventoryItem);
	RemoveItem(FromInventory, FromInventorySlot);

	// Spawn the equipment actor
	Server_SpawnActorEquiment(ToInventory, ToInventorySlot);
	LocalInventoryItem = ToInventory->GetInventoryItem(ToInventorySlot);

	// Check if the equipment actor is valid
	if (IsValid(LocalInventoryItem.ActorInWorld))
	{
		UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::EquipItem] Equipment actor valid, spawned successfully"));
	}
	else
	{
		if (LocalInventoryItem.ActorEquipment != NULL)
		{
			UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::EquipItem] Equipment actor invalid, spawn failed"));
		}
	}

	// Broadcast the equipment event
	OnEquip.Broadcast(ToInventory->GetKeyAtIndex(ToInventorySlot), LocalInventoryItem, FItemData(), LocalInventoryItem.ActorInWorld);

	// Broadcast the item movement events if the inventories are different
	if (FromInventory != ToInventory)
	{
		BroadcastItemMovement(FromInventory, LocalInventoryItem, FItemData());
		BroadcastItemMovement(ToInventory, FItemData(), LocalInventoryItem);
	}
}

void UInventoryManagerComponent::HandleItemSwapEquip(UInventoryComponent* FromInventory, int32 FromInventorySlot, UInventoryComponent* ToInventory, int32 ToInventorySlot, FItemData& LocalInventoryItem, FItemData& LocalSwapEquipementItem)
{
	// Check if the target slot is disabled
	if (!CanBeEquip(LocalSwapEquipementItem.GameplayTagContainer))
	{
		UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::EquipItem] Cannot be equipped, type tag is in DisableSlot on target slot"));
		Server_CancelMoveInventory(FromInventory->GetId(), FromInventorySlot, ToInventory->GetId(), ToInventorySlot);
		return;
	}

	// Check if the item can be equipped based on restrictions
	if (!ToInventory->CheckItemInRestriction(LocalSwapEquipementItem, ToInventorySlot) && !LocalSwapEquipementItem.NotifyScript_GetIsItemModifier(this))
	{
		UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::EquipItem] Cannot be equipped, restrictions not met"));
		Server_CancelMoveInventory(FromInventory->GetId(), FromInventorySlot, ToInventory->GetId(), ToInventorySlot);
		return;
	}

	//Check FromInventory Have restriction
	if (!FromInventory->CheckItemInRestriction(LocalSwapEquipementItem, FromInventorySlot))
	{
		Server_CancelMoveInventory(FromInventory->GetId(), FromInventorySlot, ToInventory->GetId(), ToInventorySlot);
		return;
	}


	// Destroy the old equipment
	Server_DestoryActorEquipment(ToInventory, ToInventorySlot, false);

	// Notify the item removal event in the inventory
	LocalInventoryItem.NotifyScript_ItemRemove(this, FromInventory->GetId(), FromInventorySlot, FromInventory);
	LocalSwapEquipementItem.NotifyScript_ItemRemove(this, ToInventory->GetId(), ToInventorySlot, ToInventory);

	// Swap the items
	AddItem(ToInventory, ToInventorySlot, LocalInventoryItem);
	AddItem(FromInventory, FromInventorySlot, LocalSwapEquipementItem);

	// Spawn the equipment actor
	Server_SpawnActorEquiment(ToInventory, ToInventorySlot);
	LocalInventoryItem = ToInventory->GetInventoryItem(ToInventorySlot);

	// Check if the equipment actor is valid
	if (IsValid(LocalInventoryItem.ActorInWorld))
	{
		UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::EquipItem] Equipment actor valid, spawned successfully"));
	}
	else
	{
		if (LocalInventoryItem.ActorEquipment != NULL)
		{
			UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::EquipItem] Equipment actor invalid, spawn failed"));
		}
	}

	// Broadcast the equipment event
	OnEquip.Broadcast(ToInventory->GetKeyAtIndex(ToInventorySlot), LocalInventoryItem, LocalSwapEquipementItem, LocalInventoryItem.ActorInWorld);

	// Broadcast the item movement events if the inventories are different
	if (FromInventory != ToInventory)
	{
		BroadcastItemMovement(FromInventory, LocalInventoryItem, LocalSwapEquipementItem);
		BroadcastItemMovement(ToInventory, LocalSwapEquipementItem, LocalInventoryItem);
	}
}

void UInventoryManagerComponent::UnequipItem_Implementation(UInventoryComponent* FromInventory, int32 FromInventorySlot, UInventoryComponent* ToInventory, int32 ToInventorySlot)
{
	// Check if the inventories and slots are valid
	if (!IsValid(FromInventory) || !IsValid(ToInventory))
	{
		return;
	}

	if (FromInventory == ToInventory && FromInventorySlot == ToInventorySlot)
	{
		return;
	}

	FItemData LocalInventoryItem = FromInventory->GetInventoryItem(FromInventorySlot);

	// Check if the item can be unequipped
	if (!LocalInventoryItem.NotifyScript_GetCanUnequip(this, FromInventory->GetId(), FromInventorySlot, ToInventory->GetId(), ToInventorySlot, LocalInventoryItem.ActorInWorld, false))
	{
		UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::UnequipItem] Cannot unequip item: %s"), *LocalInventoryItem.ID.ToString());
		Server_CancelMoveInventory(FromInventory->GetId(), FromInventorySlot, ToInventory->GetId(), ToInventorySlot);
		return;
	}

	// Check if the item can be placed in the destination inventory slot
	if (!ToInventory->CheckItemInRestriction(LocalInventoryItem, ToInventorySlot))
	{
		UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::UnequipItem] Item cannot be placed in the destination slot: %s"), *LocalInventoryItem.ID.ToString());
		Server_CancelMoveInventory(FromInventory->GetId(), FromInventorySlot, ToInventory->GetId(), ToInventorySlot);
		return;
	}

	if (LocalInventoryItem.IsEquipment())
	{
		FItemData LocalSwapEquipementItem = ToInventory->GetInventoryItem(ToInventorySlot);

		if (ToInventory->GetItemIsValid(LocalSwapEquipementItem))
		{
			// Swap
			HandleItemSwapUnequip(FromInventory, FromInventorySlot, ToInventory, ToInventorySlot, LocalInventoryItem, LocalSwapEquipementItem);
		}
		else
		{
			// Move
			HandleItemUnequip(FromInventory, FromInventorySlot, ToInventory, ToInventorySlot, LocalInventoryItem);
		}

		// Broadcast the unequip event
		OnUnequip.Broadcast(FromInventory->GetKeyAtIndex(FromInventorySlot), LocalInventoryItem, nullptr);
	}
}

void UInventoryManagerComponent::HandleItemSwapUnequip(UInventoryComponent* FromInventory, int32 FromInventorySlot, UInventoryComponent* ToInventory, int32 ToInventorySlot, FItemData& LocalInventoryItem, FItemData& LocalSwapEquipementItem)
{
	// Check if the item can be placed in the source inventory slot
	if (!ToInventory->CheckItemInRestriction(LocalSwapEquipementItem, FromInventorySlot))
	{
		Server_CancelMoveInventory(FromInventory->GetId(), FromInventorySlot, ToInventory->GetId(), ToInventorySlot);
		return;
	}

	// Check if there is enough space for the move item
	if (!ToInventory->HaveSapceInSlot(LocalInventoryItem, FromInventorySlot, ToInventorySlot, true))
	{
		Server_CancelMoveInventory(FromInventory->GetId(), FromInventorySlot, ToInventory->GetId(), ToInventorySlot);
		return;
	}

	//Check FromInventory Have restriction
	if (!FromInventory->CheckItemInRestriction(LocalSwapEquipementItem, FromInventorySlot))
	{
		Server_CancelMoveInventory(FromInventory->GetId(), FromInventorySlot, ToInventory->GetId(), ToInventorySlot);
		return;
	}

	// Destroy the actor equipment
	Server_DestoryActorEquipment(FromInventory, FromInventorySlot);

	// Update data after unequip
	LocalInventoryItem = FromInventory->GetInventoryItem(FromInventorySlot);

	// Notify remove in inventory
	LocalInventoryItem.NotifyScript_ItemRemove(this, FromInventory->GetId(), FromInventorySlot, FromInventory);
	LocalSwapEquipementItem.NotifyScript_ItemRemove(this, ToInventory->GetId(), ToInventorySlot, ToInventory);

	// Swap items
	AddItem(ToInventory, ToInventorySlot, LocalInventoryItem);
	AddItem(FromInventory, FromInventorySlot, LocalSwapEquipementItem);

	// Broadcast item movement
	BroadcastItemMovement(FromInventory, LocalInventoryItem, LocalSwapEquipementItem);
	BroadcastItemMovement(ToInventory, LocalSwapEquipementItem, LocalInventoryItem);
}

void UInventoryManagerComponent::HandleItemUnequip(UInventoryComponent* FromInventory, int32 FromInventorySlot, UInventoryComponent* ToInventory, int32 ToInventorySlot, FItemData& LocalInventoryItem)
{
	// Auto fill inventory
	if (!CheckAutoFillInventory(LocalInventoryItem, ToInventory, ToInventorySlot))
	{
		Server_CancelMoveInventory(FromInventory->GetId(), FromInventorySlot, ToInventory->GetId(), ToInventorySlot);
		return;
	}

	// Check if there is enough space for the move item
	if (!ToInventory->HaveSapceInSlot(LocalInventoryItem, FromInventorySlot, ToInventorySlot))
	{
		Server_CancelMoveInventory(FromInventory->GetId(), FromInventorySlot, ToInventory->GetId(), ToInventorySlot);
		return;
	}

	// Destroy the actor equipment
	Server_DestoryActorEquipment(FromInventory, FromInventorySlot);

	// Update data after unequip
	LocalInventoryItem = FromInventory->GetInventoryItem(FromInventorySlot);

	// Move item
	AddItem(ToInventory, ToInventorySlot, LocalInventoryItem);
	RemoveItem(FromInventory, FromInventorySlot);

	// Broadcast item movement
	BroadcastItemMovement(FromInventory, LocalInventoryItem, FItemData());
	BroadcastItemMovement(ToInventory, FItemData(), LocalInventoryItem);
}

void UInventoryManagerComponent::SplitItem_Implementation(UInventoryComponent* FromInventory, int32 FromInventorySlot, UInventoryComponent* ToInventory, int32 ToInventorySlot, int32 Amount)
{
	if (IsValid(FromInventory) && IsValid(ToInventory))
	{
		if (!(FromInventory == ToInventory && FromInventorySlot == ToInventorySlot)) // Is same slot and same inventory
		{
			if (ToInventory->bIsContainer == true && ToInventory->EnableLootContainer != true || ToInventory->bIsContainer == false) // @TODO Gestion COntainer 
			{
				FItemData LocalInventoryItem = FromInventory->GetInventoryItem(FromInventorySlot);


				if (!ToInventory->CheckItemInRestriction(LocalInventoryItem, ToInventorySlot))
				{
					return;
				}
				
				// Check is Stackable and if that I want to move equal Amount Item
				if (!LocalInventoryItem.bIsStackable == true || LocalInventoryItem.Amount == Amount || Amount == 0)
				{
					// if true juste MoveItem
				//	UE_LOG(LogInventory, Warning, TEXT("Split : Juste Move BaseAmount : %d -> TryAmountSplit : %d"), LocalInventoryItem.Amount,Amount);
					MoveItem(FromInventory, FromInventorySlot, ToInventory, ToInventorySlot);
				}
				else
				{
					int32 OutAmountRemoved;
					FItemData OutItemData;
					bool OutFullAmountRemoved;
					//Check if Item is Visible, if not then move to the end Inventory if has Space
					if (!ToInventory->GetInventoryItem(ToInventorySlot).bIsVisible)
					{
						//UE_LOG(LogInventory,Warning,TEXT("Move Item"));
						bool HaveSpace = false;
						int32 IndexFound;
						ToInventory->GetEmptyInventorySpace(HaveSpace,IndexFound, LocalInventoryItem);
						if (HaveSpace)
						{
							MoveItem(ToInventory,ToInventorySlot,ToInventory,IndexFound);
						}
							
					}
					
					FItemData LocalSwapInventoryItem = ToInventory->GetInventoryItem(ToInventorySlot);
					bool bSameInventory = FromInventory == ToInventory;
					int32 AmountToRemoved = Amount;
					
					if (ToInventory->GetItemIsValid(LocalSwapInventoryItem))
					{
						// Swap Item or stack
						if (EqualItem(LocalInventoryItem, LocalSwapInventoryItem)) // Same Item try to stack
						{
							int32 AmountRemaining = AddItemToStack(ToInventory, ToInventorySlot, Amount,LocalInventoryItem.Decay);
							AmountToRemoved = Amount - AmountRemaining;

							RemoveToItemAmount(OutFullAmountRemoved, OutItemData, OutAmountRemoved, LocalInventoryItem, AmountToRemoved);

							if (OutFullAmountRemoved) //Slot is Empty then remove
							{
								RemoveItem(FromInventory, FromInventorySlot);
							}
							else
							{
								AddItem(FromInventory, FromInventorySlot, OutItemData); // if not empty Update Value InventorySlot
							}


							if (!bSameInventory)
							{
								FromInventory->OnItemLeaveInventory.Broadcast(UChestoryFunctionLibrary::GetPrimaryAssetIdFromItemData(GetWorld(), LocalInventoryItem), AmountToRemoved);
								ToInventory->OnItemEnterInventory.Broadcast(UChestoryFunctionLibrary::GetPrimaryAssetIdFromItemData(GetWorld(), OutItemData), AmountToRemoved);
							}
						}
						
					}
					else
					{
						//Auto Fill Inventory  /!\ ToInventorySlot byRef
						if (!CheckAutoFillInventory(LocalInventoryItem, ToInventory, ToInventorySlot))
						{
							Server_CancelMoveInventory(FromInventory->GetId(), FromInventorySlot, ToInventory->GetId(), ToInventorySlot);
							return;
						}

						RemoveToItemAmount(OutFullAmountRemoved, OutItemData, OutAmountRemoved, LocalInventoryItem, Amount);
						LocalInventoryItem.Amount = OutAmountRemoved;
						LocalInventoryItem.UniqueId = FGuid();
						AddItem(ToInventory, ToInventorySlot, LocalInventoryItem);

						if (OutFullAmountRemoved)
						{
							RemoveItem(FromInventory, FromInventorySlot);
						}
						else
						{
							
							//Reset Decay
							OutItemData.Decay = OutItemData.DecayTime;
							AddItem(FromInventory, FromInventorySlot, OutItemData);
						}

						if (!bSameInventory)
						{
							FromInventory->OnItemLeaveInventory.Broadcast(UChestoryFunctionLibrary::GetPrimaryAssetIdFromItemData(GetWorld(),OutItemData), LocalInventoryItem.Amount);
							ToInventory->OnItemEnterInventory.Broadcast(UChestoryFunctionLibrary::GetPrimaryAssetIdFromItemData(GetWorld(),LocalInventoryItem), LocalInventoryItem.Amount);
						}
					}
				}
			}
		}
	}
}

void UInventoryManagerComponent::BroadcastItemMovement(UInventoryComponent* Inventory, const FItemData& ItemLeaving, const FItemData& ItemEntering)
{
	// Broadcast the item leaving the inventory
	if (ItemLeaving.IsValid())
	{
		Inventory->OnItemLeaveInventory.Broadcast(UChestoryFunctionLibrary::GetPrimaryAssetIdFromItemData(GetWorld(), ItemLeaving), ItemLeaving.Amount);
	}

	// Broadcast the item entering the inventory
	if (ItemEntering.IsValid())
	{
		Inventory->OnItemEnterInventory.Broadcast(UChestoryFunctionLibrary::GetPrimaryAssetIdFromItemData(GetWorld(), ItemEntering), ItemEntering.Amount);
	}
}

bool UInventoryManagerComponent::CheckAutoFillInventory(FItemData ForItem, UInventoryComponent* ToInventory, int32& ToInventorySlot)
{
	if (ToInventory->EnableAutoFill)
	{
		bool Success = false;
		int32 IndexFound = -1;
		ToInventory->GetEmptyInventorySpace(Success, IndexFound, ForItem);

		if (Success)
		{
			ToInventorySlot = IndexFound;
		}

		return Success;

	}

	return true;
}


void UInventoryManagerComponent::DestoryEquipmentItem_Implementation(UInventoryComponent* FromInventory, int32 FromInventorySlot)
{
	if (!IsValid(FromInventory))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::DestoryEquipmentItem] InventoryComponent Invalid"));
		return;
	}

	FItemData LocalInventoryItem = FromInventory->GetInventoryItem(FromInventorySlot);

	if (LocalInventoryItem.IsEmpty())
	{
		UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::DestoryEquipmentItem] InventoryComponent:[%s] Index:[%d] is Empty"),*FromInventory->GetFName().ToString(),FromInventorySlot);
		return;
	}

	if (LocalInventoryItem.ItemType != EItemType::EEquipement)
	{
		UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::DestoryEquipmentItem] InventoryComponent:[%s] Index:[%d] is not ItemType Equipment"), *FromInventory->GetFName().ToString(), FromInventorySlot);
		return;
	}



	//Destory Acotr in world
	Server_DestoryActorEquipment(FromInventory, FromInventorySlot);
	//Update Data after OnUnequip
	LocalInventoryItem = FromInventory->GetInventoryItem(FromInventorySlot);

	RemoveItem(FromInventory, FromInventorySlot);

	OnUnequip.Broadcast(FromInventory->GetKeyAtIndex(FromInventorySlot), LocalInventoryItem, nullptr);

	FromInventory->OnItemLeaveInventory.Broadcast(UChestoryFunctionLibrary::GetPrimaryAssetIdFromItemData(GetWorld(),
		LocalInventoryItem),
		LocalInventoryItem.Amount);

	UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::DestoryEquipmentItem] InventoryComponent:[%s] Index:[%d] Destory Equipment Success"), *FromInventory->GetFName().ToString(), FromInventorySlot);

}


void UInventoryManagerComponent::Server_SpawnActorEquiment(UInventoryComponent* EquipmentInventory, int32 EquipmentSlot)
{


	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	if (!IsValid(EquipmentInventory))
	{
		return;
	}

	FItemData Item = EquipmentInventory->GetInventoryItem(EquipmentSlot);

	if (IsValid(Item.ActorInWorld))
	{
		UE_LOG(LogInventory, Display, TEXT("[UInventoryManagerComponent::Server_SpawnActorEquiment] Already has an Actor Equipment in the world, ActorEquipment:[%s]"), *Item.ID.ToString());
		return;
	}

	if (Item.IsValid() && Item.ActorEquipment != NULL)
	{
		FVector SpawnLocation = EquipmentInventory->GetOwnerForLocation()->GetActorLocation();
		FRotator SpawnRotator = EquipmentInventory->GetOwnerForLocation()->GetActorRotation();

		UE_LOG(LogInventory, Display, TEXT("[UInventoryManagerComponent::Server_SpawnActorEquiment] Spawn ActorEquipment:[%s]"),*Item.ID.ToString());


		auto MyItemActor = Cast<AActor>(UGameplayStatics::BeginDeferredActorSpawnFromClass(
			GetWorld(),
			Item.ActorEquipment,
			FTransform(SpawnRotator, SpawnLocation),
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn,
			GetOwner()));


		//Spawn Component child befor Load DataComp
		UGameplayStatics::FinishSpawningActor(MyItemActor, FTransform(SpawnRotator, SpawnLocation));


		if (IsValid(MyItemActor))
		{

			//Load DataBytes on Actor in World
			if (!Item.DataBytesActor.IsEmpty())
			{
				for (auto& Data : Item.DataBytesActor)
				{
					if (GetInvSubsystem()->LoadActor(Data, FName("Actor"), MyItemActor, "Server_SpawnActorEquiment"))
					{
						break;
					}
				}


				//Load DataBytes on Chestory Component in World
				TArray<UActorComponent*> ChestoryComp = GetInvSubsystem()->GetChestoryComponent(MyItemActor);
				for (auto& Comp : ChestoryComp)
				{
					for (auto& Data : Item.DataBytesActor)
					{
						if (GetInvSubsystem()->LoadActorComponent(Data, Comp->GetFName(), Comp, "Server_SpawnActorEquiment"))
						{
							break;
						}
					}
				}
			}
			else
			{
				UE_LOG(LogInventory, VeryVerbose, TEXT("[UInventoryManagerComponent::Server_SpawnActorEquiment] No Data found on Equipment :[%s]"),*Item.ID.ToString());
			}


			// Apply Data In dropped Component
			TArray<UDroppedComponent*> Comps;
			MyItemActor->GetComponents(Comps);
			if (Comps.IsValidIndex(0))
			{
				UDroppedComponent* DroppedComponent = Comps[0];
				if (DroppedComponent)
				{
					DroppedComponent->Server_SetItemData(Item, true);
				}
			}

			//Apply Actor
			Item.ActorInWorld = MyItemActor;

			TArray<int32> FilledIndex;
			//Update Item in Invetory
			EquipmentInventory->SetInventoryItem(EquipmentSlot, Item,FilledIndex);

			// Notify Script ItemEquip
			Item.NotifyScript_ItemEquip(this, EquipmentInventory->GetId(), EquipmentSlot);
		}


	}

}

void UInventoryManagerComponent::Server_DestoryActorEquipment_Implementation(UInventoryComponent* EquipmentInventory, int32 EquipmentSlot, bool IsDrop)
{

	if (!IsValid(EquipmentInventory))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Server_DestoryActorEquipment] EquipmentInventory Component Invalid"));
		return;
	}

	FItemData Item = EquipmentInventory->GetInventoryItem(EquipmentSlot);
	
	if (!Item.IsValid())
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Server_DestoryActorEquipment] Item Invalid"));
		return;
	}

	//No equipment Actor Manage
	if (Item.ActorEquipment == NULL)
	{
		return;
	}

	if (!IsValid(Item.ActorInWorld))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Server_DestoryActorEquipment] ActorInWorld Invalid"));
		return;
	}

	//Apply data Actor on Slot
	UChestoryFunctionLibrary::UpdateDataBytesActor(this,Item,true);
	
	TArray<int32> FilledIndex;
	EquipmentInventory->SetInventoryItem(EquipmentSlot, Item,FilledIndex);

	//Notify Script Unequip
	Item.NotifyScript_ItemUnequip(this, EquipmentInventory->GetId(), EquipmentSlot);


	if (!IsDrop)
	{
		TArray<AActor*> ActorChildren;
		//Item.ActorInWorld->GetAllChildActors(ActorChildren);
		Item.ActorInWorld->GetAttachedActors(ActorChildren);
		for (auto& Child : ActorChildren)
		{
			Child->Destroy();
		}

		//Not destory
		Item.ActorInWorld->Destroy();
		//Item Clear un DropItem Function
		Item.ActorInWorld = nullptr;
	}

	EquipmentInventory->SetInventoryItem(EquipmentSlot, Item,FilledIndex);



}

bool UInventoryManagerComponent::CheckSlotsForItems(UInventoryComponent* Inventory, int32 Amount)
{
	if (IsValid(Inventory))
	{
		for (int32 i = (Inventory->Inventory.Num() - Amount); i < Inventory->Inventory.Num(); i++)
		{
			if (Inventory->GetItemIsValid(Inventory->GetInventoryItem(i)))
			{
				return true;
			}
		}
		return false;
	}
	else
	{
		return false;
	}
	
}


void UInventoryManagerComponent::Server_IncreaseInventorySize_Implementation(FGuid InventoryId, int32 Amount)
{

	if (!InventoryId.IsValid())
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Server_IncreaseInventorySize] InventoryId Invalid"));
		return;
	}

	if (UInventoryComponent* Inventory = GetInventory(InventoryId); IsValid(Inventory))
	{
		Inventory->IncreaseInventorySize(Amount);

		TSubclassOf<UInventorySlotUserWidget> Widget = Inventory->OverrideInventoryWidgetSlot != NULL ? Inventory->OverrideInventoryWidgetSlot : CustomClassSlot;

		for (auto& Viewer : Inventory->GetViewers())
		{
			Viewer.Key->Client_IncreaseInventorySlots(InventoryId, Viewer.Value, Inventory->EnableSizeItem, Widget, Amount);
		}
	}
}

void UInventoryManagerComponent::Server_DecreaseInventorySize_Implementation(FGuid InventoryId, int32 Amount)
{
	
	if (!InventoryId.IsValid())
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Server_DecreaseInventorySize] InventoryId Invalid"));
		return;
	}
	if (Amount <= 0)
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Server_DecreaseInventorySize] Can Decrease  with Amount:[%d]"), Amount);
		return;
	}

	if (UInventoryComponent* Inventory = GetInventory(InventoryId); IsValid(Inventory))
	{
		if (!Inventory->CanBeDecremented(Amount))
		{
			UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Server_DecreaseInventorySize] Can Decrease  with Amount:[%d] you have item on Slot"), Amount);
			return;
		}

		Inventory->DecreaseInventorySize(Amount);
		for (auto& Viewer : Inventory->GetViewers())
		{
			Viewer.Key->Client_DecreaseInventorySlots(InventoryId, Viewer.Value, Amount);
		}

	}

}

// ---------------------- FILTER MANAGEMENT ---------------------- 


void UInventoryManagerComponent::EnableVisibilityFilter_Implementation(FGuid InventoryId,	FGameplayTagContainer Tags)
{
	if (!InventoryId.IsValid())
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::EnableVisibilityFilter] InventoryId Invalid [%s]"),*InventoryId.ToString());
		return;
	}

	if(!IsValid(GetInventory(InventoryId)))
	{
		UE_LOG(LogInventory,Warning,TEXT("[UInventoryManagerComponent::EnableVisibilityFilter] InventoryComponent Invalid with InventoryId:[%s]"),*InventoryId.ToString());
		return;
	}
	
	
	GetInventory(InventoryId)->ApplyNewFilter(Tags);
	FillInventorySlotVisible(InventoryId);
	//RefreshClientSlots(InventoryId);

}

void UInventoryManagerComponent::DisableVisibilityFilter_Implementation(FGuid InventoryId, bool FillInventory)
{

	if (!InventoryId.IsValid())
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::DisableVisibilityFilter] InventoryId Invalid [%s]"), *InventoryId.ToString());
		return;
	}

	if (!IsValid(GetInventory(InventoryId)))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::DisableVisibilityFilter] InventoryComponent Invalid with InventoryId:[%s]"),*InventoryId.ToString());
		return;
	}
	
	GetInventory(InventoryId)->DisableFilter();

	if (FillInventory)
	{
		FillInventorySlotVisible(InventoryId);
	}

	RefreshClientSlots(InventoryId);
		
}


void UInventoryManagerComponent::FillInventorySlotVisible_Implementation(FGuid InventoryId, int32 StartIndex/* = 0*/)
{

	if (!InventoryId.IsValid())
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::FillInventorySlotVisible] InventoryId Invalid [%s]"), *InventoryId.ToString());
		return;
	}

	if (!IsValid(GetInventory(InventoryId)))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::FillInventorySlotVisible] InventoryComponent Invalid with InventoryId:[%s]"), *InventoryId.ToString());
		return;
	}
	
	UInventoryComponent* Inventory = GetInventory(InventoryId);

	//Save item Map
	TMap<int32, FItemData> MapItem;

	//For all item in inventory save the item dans remove temp
	for (int32 _Index = StartIndex; _Index < Inventory->GetInventoryItems().Num(); _Index++)
	{
		if (!Inventory->GetInventoryItem(_Index).IsEmpty())
		{
			
			//Save Item
			MapItem.Add(_Index, Inventory->GetInventoryItem(_Index));

			//Remove Item
			TArray<int32> Unfilled;
			Inventory->ClearInventoryItem(_Index, Unfilled);
			//RemoveItem(Inventory, _Index);
		}
	}
	
	//Inventory->ClearInventory();

	//Sort MapItem by Size object in inventory
	MapItem.ValueSort(FSortBySizeItem());

	//Add Item Visible
	for(const auto Item : MapItem)
	{
		if (Item.Value.bIsVisible)
		{
			bool HaveSpace = false;
			int32 IndexFound = 0;
			Inventory->GetEmptyInventorySpace(HaveSpace, IndexFound, Item.Value);
			if (HaveSpace)
			{
				TArray<int32> Filled;
				Inventory->SetInventoryItem(IndexFound, Item.Value, Filled);
				//AddItem(Inventory, IndexFound, Item.Value);
			}
		}
	}

	//Add Item Invisible
	for (const auto Item : MapItem)
	{
		if (!Item.Value.bIsVisible)
		{
			bool HaveSpace = false;
			int32 IndexFound = 0;
			Inventory->GetEmptyInventorySpace(HaveSpace, IndexFound, Item.Value);
			if (HaveSpace)
			{
				TArray<int32> Filled;
				Inventory->SetInventoryItem(IndexFound, Item.Value, Filled);
				//AddItem(Inventory, IndexFound, Item.Value);
			}
		}
	}

	RefreshClientSlots(InventoryId);
}


void UInventoryManagerComponent::SortInventory_Implementation(FGuid InventoryId, ESortItemRule SortRule, bool Invert, FGameplayTag Tag)
{
	if (!InventoryId.IsValid())
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::SortInventory] InventoryId Invalid [%s]"), *InventoryId.ToString());
		return;
	}

	

	if (!IsValid(GetInventory(InventoryId)))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::SortInventory] InventoryComponent Invalid with InventoryId:[%s]"), *InventoryId.ToString());
		return;
	}

	UInventoryComponent* Inventory = GetInventory(InventoryId);

	//Save item Array
	TArray<FItemData> DataInventory;

	//For all item in inventory save the item dans remove temp
	for (int32 _Index = 0; _Index < Inventory->GetInventoryItems().Num(); _Index++)
	{
		if (!Inventory->GetInventoryItem(_Index).IsEmpty())
		{

			//Save Item
			DataInventory.Add(Inventory->GetInventoryItem(_Index));

			//Remove Item
			TArray<int32> Unfilled;
			Inventory->ClearInventoryItem(_Index, Unfilled);
		}
	}

	//Sort DataInventory object in inventory

	switch (SortRule)
	{
	case ESortItemRule::Name:
		DataInventory.Sort(FSortByNameItem());
		break;
	case ESortItemRule::GameplayTag:
		DataInventory.Sort(FSortByGameplayTagItem(Tag));
		break;
	case ESortItemRule::Size:
		DataInventory.Sort(FSortBySizeItem());
		break;
	case ESortItemRule::Weight:
		DataInventory.Sort(FSortByWeightItem());
		break;
	case ESortItemRule::Amount:
		DataInventory.Sort(FSortByAmountItem());
		break;
	case ESortItemRule::Socket:
		DataInventory.Sort(FSortBySocketItem());
		break;
	case ESortItemRule::Durability:
		DataInventory.Sort(FSortByDurabilityItem());
		break;
	case ESortItemRule::Decay:
		DataInventory.Sort(FSortByDecayItem());
		break;
	case ESortItemRule::Attribute:
		DataInventory.Sort(FSortByAttributeItem(Tag));
		break;
	default:
		DataInventory.Sort(FSortByNameItem());
		break;
	}

	if (Invert)
	{
		Algo::Reverse(DataInventory);
	}

	//Add Item Visible
	for (const auto Item : DataInventory)
	{
		bool HaveSpace = false;
		int32 IndexFound = 0;
		Inventory->GetEmptyInventorySpace(HaveSpace, IndexFound, Item);
		if (HaveSpace)
		{
			TArray<int32> Filled;
			Inventory->SetInventoryItem(IndexFound, Item, Filled);
		}
	}

	RefreshClientSlots(InventoryId);

}

void UInventoryManagerComponent::RefreshClientSlots_Implementation(FGuid InventoryId)
{
	if (!GetInvSubsystem()->ContainsInventory(InventoryId))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::RefreshClientSlots] Inventory Not found with InventoryId:[%s]"),*InventoryId.ToString());
		return;
	}

	UInventoryComponent* Inventory = GetInventory(InventoryId);

	if(!IsValid(Inventory))
	{
		UE_LOG(LogInventory,Warning,TEXT("[RefreshClientSlots] InventoryComponent Invalid"));
		return;
	}
	
	TArray<FItemDataInfoCompressed> NewData = Inventory->GetDataForTransfer();
	
	for (int32 i = 0; i < Inventory->GetDataForTransfer().Num(); i++)
	{		
		SetViewersInventorySlot(InventoryId, Inventory->GetViewers(), i, NewData[i]);	
	}
}

// ---------------------- ATTIBUTES MANAGEMENT ----------------------

void UInventoryManagerComponent::AddTagAttribute_Implementation(UInventoryComponent* Inventory, int32 Slot, FGameplayTag AttributeTag, float Value)
{
	if (!IsValid(Inventory))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::AddTagAttribute] Inventory Invalid"));
		return;
	}

	FItemData Item = Inventory->GetInventoryItem(Slot);
	if (Item.IsValid())
	{
		Item.Attributes.AddAttribute(AttributeTag, Value);
		TArray<int32> FilledIndex;
		Inventory->SetInventoryItem(Slot, Item,FilledIndex);

		SetViewersAttributesSlot(Inventory->GetId(), Inventory->GetViewers(), Slot, Item.Attributes);

		OnAttributeChanged.Broadcast(Inventory->GetId(), Slot, true, FGameplayTagAttribute(AttributeTag, Value));
		
	}
	else
	{
		UE_LOG(LogInventory,Warning,TEXT("[UInventoryManagerComponent::AddTagAttribute] Item no valid"))
	}
}

void UInventoryManagerComponent::RemoveTagAttribute_Implementation(UInventoryComponent* Inventory, int32 Slot, FGameplayTag AttributeTag, float Value)
{
	if (!IsValid(Inventory))
	{
		return;
	}

	FItemData Item = Inventory->GetInventoryItem(Slot);
	if (Item.IsValid())
	{
		Item.Attributes.RemoveAttribute(AttributeTag, Value);
		TArray<int32> FilledIndex;
		Inventory->SetInventoryItem(Slot, Item,FilledIndex);

		SetViewersAttributesSlot(Inventory->GetId(), Inventory->GetViewers(), Slot, Item.Attributes);


		OnAttributeChanged.Broadcast(Inventory->GetId(), Slot, false, FGameplayTagAttribute(AttributeTag, Value));
	}
}

float UInventoryManagerComponent::GetAttributeValue(UInventoryComponent* Inventory, int32 Slot, FGameplayTag AttributeTag) const
{
	if (!IsValid(Inventory))
	{
		return 0.0f;
	}

	FItemData Item = Inventory->GetInventoryItem(Slot);
	if (Item.IsValid())
	{
		return Item.Attributes.GetAttributeValue(AttributeTag);
	}

	return 0.0f;
}

bool UInventoryManagerComponent::HasAttributeTag(UInventoryComponent* Inventory, int32 Slot, FGameplayTag AttributeTag) const
{

	if (!IsValid(Inventory))
	{
		return false;
	}

	FItemData Item = Inventory->GetInventoryItem(Slot);
	if (Item.IsValid())
	{
		return Item.Attributes.ContainsAttributeTag(AttributeTag);
	}

	return false;
}

float UInventoryManagerComponent::GetEquipementAttributeValue(FName EquipmentKey, FGameplayTag AttributeTag)
{
	if (!IsValid(EquipementPlayerInventory))
	{
		UE_LOG(LogInventory, Warning, TEXT("[GetEquipementAttributeValue] EquipementPlayerInventory Invalid"));
		return 0.0f;
	}

	if (!AttributeTag.IsValid())
	{
		UE_LOG(LogInventory, Warning, TEXT("[GetEquipementAttributeValue] AttributeTag Invalid"));
		return 0.0f;
	}



	if (!Equipments.Contains(EquipmentKey))
	{
		UE_LOG(LogInventory, Warning, TEXT("[GetEquipementAttributeValue] EquipmentKey Not found"));
		return 0.0f;
	}

	int32 Index = EquipementPlayerInventory->GetIndexAtKey(EquipmentKey);
	if (EquipementPlayerInventory->GetInventoryItem(Index).IsValid())
	{
		if (HasAttributeTag(EquipementPlayerInventory, Index, AttributeTag))
		{
			return GetAttributeValue(EquipementPlayerInventory, Index, AttributeTag);
		}
	}

	UE_LOG(LogInventory, Verbose, TEXT("[GetEquipementAttributeValue] Attribute tag not found in equipement"));
	return 0.0f;
}

float UInventoryManagerComponent::GetTotalAttributeValueEquipement(FGameplayTag AttributeTag)
{
	if (!IsValid(EquipementPlayerInventory))
	{
		UE_LOG(LogInventory, Warning, TEXT("[GetTotalAttributeValueEquipement] EquipementPlayerInventory Invalid"));
		return 0.0f;
	}

	if (!AttributeTag.IsValid())
	{
		UE_LOG(LogInventory, Warning, TEXT("[GetTotalAttributeValueEquipement] AttributeTag Invalid"));
		return 0.0f;
	}

	float TotalAttribute = 0.0f;

	for (int32 i = 0; i < EquipementPlayerInventory->GetInventoryItems().Num(); i++)
	{
		
		if (HasAttributeTag(EquipementPlayerInventory, i, AttributeTag))
		{
			TotalAttribute += GetAttributeValue(EquipementPlayerInventory, i, AttributeTag);
		}
	}


	return TotalAttribute;
}

FGameplayTagAttributeContainer UInventoryManagerComponent::GetAttributesEquipement(bool IncludeSocket)
{

	if (!IsValid(EquipementPlayerInventory))
	{
		UE_LOG(LogInventory, Warning, TEXT("[GetAttributesEquipement] EquipementPlayerInventory Invalid"));
		return FGameplayTagAttributeContainer();
	}


	FGameplayTagAttributeContainer Attrbitues;
	float TotalAttribute = 0.0f;

	for (auto Item : EquipementPlayerInventory->GetInventoryItems())
	{
		for (auto Attribute : Item.Attributes.GetAttributes())
		{
			Attrbitues.AddAttribute(Attribute.AttributeTag, Item.Attributes.GetAttributeValue(Attribute.AttributeTag));
		}
	
		if (IncludeSocket && Item.bCanSockets)
		{
			for (auto socket : Item.Sockets)
			{
				if (socket.PrimaryAssetId.IsValid())
				{
					
					for (auto AttributeSocket : socket.Attributes.GetAttributes())
					{
						UE_LOG(LogInventory, Verbose, TEXT("[GetAttributesEquipement] Socket Tag %s"), *AttributeSocket.AttributeTag.GetTagName().ToString());
						Attrbitues.AddAttribute(AttributeSocket.AttributeTag, socket.Attributes.GetAttributeValue(AttributeSocket.AttributeTag));
						UE_LOG(LogInventory, Verbose, TEXT("[GetAttributesEquipement] Socket Value %f"), socket.Attributes.GetAttributeValue(AttributeSocket.AttributeTag));
					}
				}
			}
		}
	}

	

	return Attrbitues;
}

FGameplayTagAttributeContainer UInventoryManagerComponent::GetAttributesEquipementByTag(FName EquipmentKey, bool IncludeSocket)
{
	if (!IsValid(EquipementPlayerInventory))
	{
		UE_LOG(LogInventory, Warning, TEXT("[GetAttributesEquipementByTag] EquipementPlayerInventory Invalid"));
		return FGameplayTagAttributeContainer();
	}

	FGameplayTagAttributeContainer Attrbitues;


	FItemData Item = EquipementPlayerInventory->GetInventoryItem(EquipementPlayerInventory->GetIndexAtKey(EquipmentKey));
	if (Item.IsValid())
	{
		for (auto Attribute : Item.Attributes.GetAttributes())
		{
			Attrbitues.AddAttribute(Attribute.AttributeTag, Item.Attributes.GetAttributeValue(Attribute.AttributeTag));
		}

		if (IncludeSocket && Item.bCanSockets)
		{
			for (auto socket : Item.Sockets)
			{
				if (socket.PrimaryAssetId.IsValid())
				{
					for (auto AttributeSocket : socket.Attributes.GetAttributes())
					{
						Attrbitues.AddAttribute(AttributeSocket.AttributeTag, socket.Attributes.GetAttributeValue(AttributeSocket.AttributeTag));
					}
				}
			}
		}
	}

	return Attrbitues;
}

FGameplayTagAttributeContainer UInventoryManagerComponent::GetAttributesEquipementByIndex(int32 IndexSlot, bool IncludeSocket)
{
	if (!IsValid(EquipementPlayerInventory))
	{
		UE_LOG(LogInventory, Warning, TEXT("[GetAttributesEquipementByTag] EquipementPlayerInventory Invalid"));
		return FGameplayTagAttributeContainer();
	}

	FGameplayTagAttributeContainer Attrbitues;
	FItemData Item = EquipementPlayerInventory->GetInventoryItem(IndexSlot);

	if (!Item.IsValid())
	{
		UE_LOG(LogInventory, Warning, TEXT("[GetAttributesEquipementByTag] Item Invalid with tihs index"));
		return FGameplayTagAttributeContainer();
	}


	for (auto Attribute : Item.Attributes.GetAttributes())
	{
		Attrbitues.AddAttribute(Attribute.AttributeTag, Item.Attributes.GetAttributeValue(Attribute.AttributeTag));
	}

	if (IncludeSocket && Item.bCanSockets)
	{
		for (auto socket : Item.Sockets)
		{
			if (socket.PrimaryAssetId.IsValid())
			{
				for (auto AttributeSocket : socket.Attributes.GetAttributes())
				{
					Attrbitues.AddAttribute(AttributeSocket.AttributeTag, socket.Attributes.GetAttributeValue(AttributeSocket.AttributeTag));
				}
			}
		}
	}

	return Attrbitues;
}

FGameplayTagAttributeContainer UInventoryManagerComponent::ReconstrcutAttibutes(TMap<FGameplayTag, float> AttributesMap)
{
	FGameplayTagAttributeContainer Attributes;

	for (auto Attribute : AttributesMap)
	{
		Attributes.AddAttribute(Attribute.Key, Attribute.Value);
	}
	return Attributes;
}



// ---------------------- STACK MANAGEMENT ---------------------- 



int32 UInventoryManagerComponent::AddItemToStack(UInventoryComponent* Inventory, int32 InventorySlot, int32 AmountToAdd, float Decay)
{
	FItemData LocalInventoryItem = Inventory->GetInventoryItem(InventorySlot);
	int32 ReturnRemainingAmount = AmountToAdd;

	if (LocalInventoryItem.Amount < LocalInventoryItem.MaxStackable)
	{
		int32 freeStackSpace = LocalInventoryItem.MaxStackable - LocalInventoryItem.Amount;

		if (AmountToAdd <= freeStackSpace)
		{
			ReturnRemainingAmount = 0;
			FItemData RecombineItem = AddToItemAmount(LocalInventoryItem, AmountToAdd);
			RecombineItem.Decay = FMath::Min(Decay,LocalInventoryItem.Decay); //Select Mini Decay
			AddItem(Inventory, InventorySlot, RecombineItem);
			return ReturnRemainingAmount;
		}
		else
		{
			ReturnRemainingAmount = AmountToAdd - freeStackSpace;
			AddItem(Inventory, InventorySlot, AddToItemAmount(LocalInventoryItem, freeStackSpace));
			return ReturnRemainingAmount;
		}
	}
	else
	{
		return ReturnRemainingAmount;
	}
}

FItemData UInventoryManagerComponent::AddToItemAmount(FItemData ItemData, int32 AmountToAdd)
{
	//FItemData DataReturn = ItemData; A TESTER 
	ItemData.Amount = ItemData.Amount + AmountToAdd;

	return ItemData;
}

void UInventoryManagerComponent::RemoveToItemAmount(bool& SlotIsEmpty, FItemData& OutItem, int32& AmountRemoved, FItemData Item, int32 AmountToRemove)
{
//	UE_LOG(LogInventory, Warning, TEXT("Current: %d / wanna Remove : %d"), Item.Amount, AmountToRemove);
	if (Item.Amount <= AmountToRemove)
	{
		AmountRemoved = Item.Amount;
		SlotIsEmpty = true;

	}
	else
	{
		AmountRemoved = AmountToRemove;
		Item.Amount = Item.Amount - AmountToRemove;
		SlotIsEmpty = false;
		OutItem = Item;
	}
}


int32 UInventoryManagerComponent::FindAndAddAmountToStack(UInventoryComponent* Inventory, FName ItemID, int32 Amount, float Decay)
{
	int32 LocalAmount = Amount;
	for (int32 i = 0; i < Inventory->Inventory.Num(); i++) //For All inventory
	{
		if (Inventory->Inventory[i].ID == ItemID && Inventory->Inventory[i].bIsStackable) //Found item and is Stackable
		{
			LocalAmount = AddItemToStack(Inventory, i, Amount, Decay); // AddItemToStack
			Amount = LocalAmount;
			if (LocalAmount == 0)
			{
				break; // Amount equal 0 then end function 
			}
		}
	}
	return LocalAmount;
}

// ---------------------- CRAFTING MANAGEMENT ---------------------- 

void UInventoryManagerComponent::Server_OpenCrafting_Implementation(UCraftingComponent* Crafting, UInventoryComponent* InInventory, FName GridKey)
{
	if (!IsValid(Crafting))
	{
		return;
	}

	Crafting->InventoryManager = this;

	if (!GetInvSubsystem()->ContainsCrafting(Crafting->GetId()))
	{
		Crafting->Multicast_UpdateId(Crafting->GetId());
	}

	// Server Send Data to client
	Client_LoadCrafting(Crafting->GetId(), Crafting, Crafting->GetDataForTransfer(), Crafting->GetDataWaitingForTransfer(), GridKey);

	
	//Client_UnloadCrafting(Crafting);
	
}

void UInventoryManagerComponent::Server_CloseCrafting_Implementation(UCraftingComponent* Crafting, FName GridKey)
{
	if (!IsValid(Crafting))
	{
		return;
	}

	Client_UnloadCrafting(Crafting->GetId(),GridKey);
	

}

bool UInventoryManagerComponent::IsValidRecipe(UItemDataAsset* ItemDataAsset, bool PrintLog)
{
	if (!ItemDataAsset->IsCraftable)
	{
		if (PrintLog)
		{
			UE_LOG(LogInventory, Warning, TEXT("[IsValidRecipe] %s : Recipe Invalid IsCraftable is False"), *ItemDataAsset->GetFName().ToString());
		}
		return false;
	}
	
	if (ItemDataAsset->Recipe.IngredientNeeded.IsEmpty())
	{
		if (PrintLog)
		{
			UE_LOG(LogInventory, Warning, TEXT("[IsValidRecipe] %s : Recipe Invalid IngredientNeeded is Empty"), *ItemDataAsset->GetFName().ToString());
		}
		return false;
	}
	
	// Check if Recipe is Valid
	for (FItemCraft Ingredient : ItemDataAsset->Recipe.IngredientNeeded)
	{
		if (!Ingredient.PrimaryAssetId.IsValid())
		{
			if (PrintLog)
			{
				UE_LOG(LogInventory, Warning, TEXT("[IsValidRecipe] %s : Recipe Invalid PrimaryAssetId is not found"), *ItemDataAsset->GetFName().ToString());
			}
			return false;
		}
	}

	return true;
}


int32 UInventoryManagerComponent::GetMaxCountCanCraft(TArray<FItemCraft> ItemHasNeed, FGuid InventoryId)
{
	if (!IsValid(GetInventory(InventoryId)))
	{
		return 0;
	}

	bool HasItemAmount = false;
	int32 MaxItemCount = 99999999;
	int32 TotalAmountItem = 0;

	for (const auto Item : ItemHasNeed)
	{
		CheckInventoryHasItemAmount(HasItemAmount, TotalAmountItem, GetInventory(InventoryId), Item.PrimaryAssetId, Item.Amount);

		const float Value = static_cast<float>(TotalAmountItem) / static_cast<float>(Item.Amount);

		if (const int32 ValueInt = FMath::TruncToInt(Value); ValueInt < MaxItemCount)
		{
			MaxItemCount = ValueInt;
		}

	}
	return MaxItemCount;
}

void UInventoryManagerComponent::ApplyAdditionalCraftData(FGuid InventoryId, FItemDataInfoCraft& InfoCraft, TArray<int32>& RecipeAmount)
{
	if (!InventoryId.IsValid())
	{
		return;
	}

	if (!GetInvSubsystem()->ContainsInventory(InventoryId))
	{
		return;
	}

	InfoCraft.MaxCountCraft = GetMaxCountCanCraft(InfoCraft.Recipe.IngredientNeeded, InventoryId);
	RecipeAmount.AddDefaulted(InfoCraft.Recipe.IngredientNeeded.Num());

	for (int32 y = 0; y < InfoCraft.Recipe.IngredientNeeded.Num(); y++)
	{
		FItemCraft& ItemCraft = InfoCraft.Recipe.IngredientNeeded[y];
		bool OutSuccess;
		CheckInventoryHasItemAmount(OutSuccess, ItemCraft.Current, GetInventory(InventoryId), ItemCraft.PrimaryAssetId, ItemCraft.Amount);
		RecipeAmount[y] = ItemCraft.Current;
	}
}

void UInventoryManagerComponent::Server_UpdateDataCraft_Implementation(FGuid CraftingId,bool Limited)
{
	if (!CraftingId.IsValid())
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Server_UpdateDataCraft] CraftingId Invalid [%s]"),*CraftingId.ToString());
		return;
	}

	if (!GetInvSubsystem()->ContainsCrafting(CraftingId))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Server_UpdateDataCraft] Can't found CraftingComponent with CraftingId [%s]"), *CraftingId.ToString());
		return;
	}

	UCraftingComponent* CraftingComp = GetCrafting(CraftingId);

	UInventoryComponent* InventoryComp = GetInventory(CraftingComp->InventoryIdLinked);
	if (!IsValid(InventoryComp))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Server_UpdateDataCraft]  InventoryComponent Linked with CraftingId [%s] Invalid"), *CraftingId.ToString());
		return;
	}

	int32 IndexSlotValid = 0;
	for (int32 i = 0; i < CraftingComp->BlueprintCrafts.Num(); i++)
	{
		if (IsValidRecipe(CraftingComp->BlueprintCrafts[i], false))
		{
			FItemDataInfoCraft CurrentInfoCraft = UChestoryFunctionLibrary::MakeItemInfoCraft(this,CraftingComp->BlueprintCrafts[i]);
			TArray<int32> RecipeAmount;

			ApplyAdditionalCraftData(CraftingComp->InventoryIdLinked, CurrentInfoCraft, RecipeAmount);

			if (Limited)
			{
				SetViewersItemAmountCrafting(InventoryComp->GetId(), InventoryComp->GetViewers(), IndexSlotValid, RecipeAmount, CurrentInfoCraft.MaxCountCraft);
			}
			else
			{
				SetViewersItemCrafting(InventoryComp->GetId(), InventoryComp->GetViewers(), IndexSlotValid, UChestoryFunctionLibrary::MakeCraftTransfer(CurrentInfoCraft));
			}

			IndexSlotValid++;

		}
	}


}


void UInventoryManagerComponent::AddCraftToWaitlist(FGuid CraftingId, FItemDataInfoCraft AddInfo, int32 Amount)
{


	if (!AddInfo.ItemInformation.IsValid())
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::AddCraftToWaitlist] Item to Craft Invalid"));
		return;
	}


	Server_AddCraftToWaitlist(CraftingId, AddInfo, Amount);
}

void UInventoryManagerComponent::Server_AddCraftToWaitlist_Implementation(FGuid CraftingId, FItemDataInfoCraft AddInfo, int32 Amount)
{
	if (!CraftingId.IsValid())
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Server_AddCraftToWaitlist] CraftingId Invalid"));
		return;
	}

	if (!GetInvSubsystem()->ContainsCrafting(CraftingId))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Server_AddCraftToWaitlist] CraftingComponent not found with CraftingId:[%s]"), *CraftingId.ToString());
		return;
	}

	if (!AddInfo.ItemInformation.IsValid())
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Server_AddCraftToWaitlist] Item to Craft Invalid"));
		return;
	}

	if (UCraftingComponent* CraftingComp = GetCrafting(CraftingId); IsValid(CraftingComp))
	{
		
		// --  Step 1 : Check Can Craft
		if (!UChestoryFunctionLibrary::GetItemDataFromID(this, AddInfo.ItemInformation.ID).NotifyScript_GetCanCraft(this, CraftingId, AddInfo, Amount))
		{
			UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::Server_AddCraftToWaitlist] ItemScript Function:[GetCanCraft] ItemCraft:[%s] return False"), *AddInfo.ItemInformation.ID.ToString());
			return;
		}

		// --  Step 2 : Try Stack if Enable (Pass if not Init)
		if (CraftingComp->bEnableStackWaitlist)
		{
			int32 IndexFound = -1;
			if (CraftingComp->FindIndexWaitListSameItem(IndexFound, AddInfo))
			{
				FItemDataInfoCraft WaitSlot = CraftingComp->GetWaitSlotAtIndex(IndexFound);
				WaitSlot.ItemInformation.Amount += Amount;
				CraftingComp->SetWaitListItem(IndexFound, WaitSlot);
				CraftingComp->OnUpdateWaitingList.Broadcast(WaitSlot.ItemInformation.ID, WaitSlot.ItemInformation.Amount);

				//Server Send update to Client
				if (IsValid(GetInventory(CraftingComp->InventoryIdLinked)))
				{
					SetViewersWaitingSlot(CraftingId, GetInventory(CraftingComp->InventoryIdLinked)->GetViewers(), IndexFound, UChestoryFunctionLibrary::MakeCraftTransfer(WaitSlot));
				}

				//Success stack End Function
				return;
			}
		}

		// --  Step 3 : Try Add Wait Slot (Init if first WaitSlot)
		int32 Index = CraftingComp->GetWaitList().Num();
		if (Index > CraftingComp->MaxWaitingList)
		{
			UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Server_AddCraftToWaitlist] Can't add Waitlist is full Index found:[%d] Max:[%d] "), Index, CraftingComp->MaxWaitingList);
			return;
		}

		AddInfo.ItemInformation.Amount = Amount;

		CraftingComp->AddWaitListItem(AddInfo);

		CraftingComp->OnUpdateWaitingList.Broadcast(AddInfo.ItemInformation.ID, Amount);

		if(IsValid(GetInventory(CraftingComp->InventoryIdLinked)))
		{
			//Server Send data to Client
			AddViewersWaitingSlot(
				CraftingId, 
				GetInventory(CraftingComp->InventoryIdLinked)->GetViewers(), 
				Index, 
				UChestoryFunctionLibrary::MakeCraftTransfer(AddInfo), 
				CraftingComp->ClassWaitingSlot);
		}
		else
		{
			UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Server_AddCraftToWaitlist] InventoryComponent Invalid"));
			return;
		}
		CraftingComp->StartCraftingTimer();
	}
	else
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Server_AddCraftToWaitlist] CraftingComponent Invalid"));
		return;
	}

}

void UInventoryManagerComponent::RemoveCraftInWaitlist(FGuid CraftingId, int32 IndexSlot)
{
	Server_RemoveCraftInWaitList(CraftingId, IndexSlot);
}


void UInventoryManagerComponent::Server_RemoveCraftInWaitList_Implementation(FGuid CraftingId, int32 IndexSlot)
{
	if (!CraftingId.IsValid())
	{
		return;
	}

	if (!GetInvSubsystem()->ContainsCrafting(CraftingId))
	{
		return;
	}

	UE_LOG(LogInventory, Display, TEXT("[UInventoryManagerComponent::Server_RemoveCraftInWaitList] Try Remove craft in waitlist"));

	UCraftingComponent* LocalCrafting = GetCrafting(CraftingId);
	
	if (!IsValid(LocalCrafting))
	{
		UE_LOG(LogInventory, Display, TEXT("[UInventoryManagerComponent::Server_RemoveCraftInWaitList] UCraftingComponent Invalid"));
		return;
	}

	// If not being crafted, it does what it wants
	if (!LocalCrafting->GetIsCurrentCrafting())
	{
		Server_RemoveCraftInWaitListForAllViewers(CraftingId, IndexSlot);
		LocalCrafting->bCallCancelCraft = false;
		UE_LOG(LogInventory, Verbose, TEXT("[Server_RemoveCraftInWaitList] Not current Crafting Free Remove"));
	}
	// It's the same slot it's currently being crafted but it has the right to cancel because bCancelDuringCraft is true
	else if (IndexSlot == LocalCrafting->GetWaitlistIndex() && LocalCrafting->GetIsCurrentCrafting() && LocalCrafting->bCancelDuringCraft)
	{
		// if refund resource is true 
		if (LocalCrafting->bRefundRessource)
		{
			RefundRecipeByPercent(LocalCrafting->CraftingInventory, LocalCrafting->CraftInfo.ItemInfo.Recipe, LocalCrafting->RefundPercentage);
		}
		UE_LOG(LogInventory, Verbose, TEXT("[Server_RemoveCraftInWaitList] Current Crafting but Free Remove Clear Timer befor"));
		// Clear Timer so that we can remove the current craft without any problem
		LocalCrafting->ClearTimerCrafting();
		Server_RemoveCraftInWaitListForAllViewers(CraftingId, IndexSlot);

		//Restart Craft 
		if (LocalCrafting->WaitList.Num() > 0) // If there is still crafts we continue
		{
			LocalCrafting->StartCraftingTimer();
		}
	}
	/* It's the same slot he's currently crafting and the remaining is > 1, but he doesn't have the right to undo
	* because bCancelDuringCraft is false
	* He will launch the bCallCancelCraft, on the next passage in the EndCraftingTimer function before restarting the craft he will check if he must Remove */
	else if (IndexSlot == LocalCrafting->GetWaitlistIndex() && LocalCrafting->WaitList[LocalCrafting->GetWaitlistIndex()].ItemInformation.Amount > 1 && LocalCrafting->GetIsCurrentCrafting() && !LocalCrafting->bCancelDuringCraft)
	{
		UE_LOG(LogInventory, Verbose, TEXT("[Server_RemoveCraftInWaitList] Current Crafting don't remove if is not last, lunch delay function for remove"));
		LocalCrafting->bCallCancelCraft = true;
		LocalCrafting->IndexSlotToCancel = IndexSlot;
	}
	else if (IndexSlot > LocalCrafting->GetWaitlistIndex() || !LocalCrafting->GetIsCurrentCrafting())
	{
		UE_LOG(LogInventory, Verbose, TEXT("[Server_RemoveCraftInWaitList] Normalement on arrive jamais ici mais au cas ou  Start Server_RemoveCraftInWaitListForAllViewers"));
		Server_RemoveCraftInWaitListForAllViewers(CraftingId, IndexSlot);
	}
	// It is necessarily lower than the current index, no choice to pause in the craft, to delete the indexes which of course changes the index of the current slot
	// and restart the timer
	else
	{
		GetWorld()->GetTimerManager().PauseTimer(LocalCrafting->TimerCrafting);
		Server_RemoveCraftInWaitListForAllViewers(CraftingId, IndexSlot);
		LocalCrafting->bCallCancelCraft = false;
		LocalCrafting->CraftInfo.WaitListIndex -= 1;
		GetWorld()->GetTimerManager().UnPauseTimer(LocalCrafting->TimerCrafting);
		UE_LOG(LogInventory, Verbose, TEXT("[Server_RemoveCraftInWaitList] Stop Timer Change Index  Cancel and Restart Timer"));
	}
}

void UInventoryManagerComponent::Server_RemoveCraftInWaitListForAllViewers_Implementation(FGuid CraftingId, int32 IndexSlot)
{
	if (UCraftingComponent* LocalCrafting = GetCrafting(CraftingId); IsValid(LocalCrafting))
	{
		LocalCrafting->RemoveWaitlistItem(IndexSlot);
		LocalCrafting->OnUpdateWaitingList.Broadcast("None", -1);

		// Call for all player in this container update UI
		RemoveViewersWaitingSlot(CraftingId, GetInventory(GetCrafting(CraftingId)->InventoryIdLinked)->GetViewers(), IndexSlot);
	}
}




void UInventoryManagerComponent::RefundRecipeByPercent(UInventoryComponent* ToInventory, FRecipeCraft RecipeToBeRefund, float Percent)
{
	if (Percent != 0.0f)
	{
		for (auto CraftNeed : RecipeToBeRefund.IngredientNeeded)
		{
			bool bSuccessAdd;
			FItemData ItemToAdd;
			int32 Index;
			float Refund = (Percent / 100) * CraftNeed.Amount;
			int32 ReturnRefund = static_cast<int32>(Refund);
			TryToAddItemToInventory(bSuccessAdd,ItemToAdd,Index,ToInventory,CraftNeed.PrimaryAssetId,ReturnRefund);
		}
	}
}

void UInventoryManagerComponent::RemoveIngredientNeededInInventory(UInventoryComponent* Inventory, FRecipeCraft Recipe)
{
	if (Inventory)
		for (const auto Resource : Recipe.IngredientNeeded)
		{
			bool bSuccess;
			int32 AmountReaming;
			TryToRemoveItemToInventory(bSuccess, AmountReaming, Inventory, Resource.PrimaryAssetId, Resource.Amount);
		}
}




// ---------------------- CONTAINER MANAGEMENT ---------------------- 


void UInventoryManagerComponent::Server_OpenInventory_Implementation(UInventoryComponent* Inventory, UCraftingComponent* Crafting, FName OverrideGridKey)
{
	if (!IsValid(Inventory))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Server_OpenInventory] Inventory Invalid"));
		return;
	}
//	Inventory->ListInventoryManagerViewers.Add(this);

	if (!GetOwner()->HasAuthority())
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Server_OpenInventory] Need to run on the server"));
		return;
	}


	if (GetInvSubsystem()->ContainsInventory(Inventory->GetId()))
	{
		UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::Server_OpenInventory] InventoryId add on Instance Subsystem"));
		//Ensure instance has this inventory
		Inventory->Multicast_UpdateId(Inventory->GetId());
	}


	FName UseGirdKey = OverrideGridKey.IsNone() ? Inventory->GridKey : OverrideGridKey;
	UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::Server_OpenInventory] Using GridKey:[%s]"),*UseGirdKey.ToString());

	Inventory->AddViewer(this, UseGirdKey);

	//Select Widget Slot Class
	TSubclassOf<UInventorySlotUserWidget> Widget = Inventory->OverrideInventoryWidgetSlot != NULL ? Inventory->OverrideInventoryWidgetSlot : CustomClassSlot;
	//Ensure in inventory is empty
	Client_ClearInventory(UseGirdKey);
	//Server transfer data to client
	Client_OpenInventory(Inventory->GetId(), UseGirdKey, Inventory->EnableSizeItem, Inventory->ContainerSlotsPerRow, Widget, Inventory->GetDataForTransfer());

	if (IsValid(Crafting))
	{
		//Ensure Crafting Id Linked
		Inventory->CraftingIdLinked = Crafting->CraftingID;

		Server_OpenCrafting(Crafting, Inventory, UseGirdKey);
	}

	//Save open container
	if (Inventory->GetInventoryType() == ESlotType::EContainer)
	{
		ContainersOpen.Add(Inventory->GetId());
	}


	//Save open Inventory
	if (Inventory->GetInventoryType() == ESlotType::EInventory)
	{
		InventoriesOpen.Add(Inventory->GetId());
	}


	//Update item blueprint
	Server_UpdateItemBlueprint(Inventory->GetId());


	//Trigger on server side
	Inventory->OnContainerIsOpen.Broadcast(UseGirdKey);

}

void UInventoryManagerComponent::Server_CloseInventory_Implementation(FGuid InventoryId)
{
	if (!InventoryId.IsValid())
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Server_CloseInventory] InventoryId Invalid"));
		return;
	}
	if (!IsValid(GetInventory(InventoryId)))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Server_CloseInventory] Inventory Invalid"));
		return;
	}

	UInventoryComponent* Inventory = GetInventory(InventoryId);

	//Find Grid Key used by this inventory manager if not found use Default Value on InventoryComp
	FName UseGirdKey = GetInventory(InventoryId)->GetGridFromInventoryManager(this);

	UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::Server_CloseInventory] Using GridKey:[%s]"), *UseGirdKey.ToString());


	Inventory->RemoveViewer(this);

	Server_CloseCrafting(GetCrafting(Inventory->CraftingIdLinked), UseGirdKey);



	//Remove save open container
	if (Inventory->GetInventoryType() == ESlotType::EContainer)
	{
		ContainersOpen.Remove(InventoryId);
	}


	//Remove save open Inventory
	if (Inventory->GetInventoryType() == ESlotType::EInventory)
	{
		InventoriesOpen.Remove(InventoryId);
	}

	const bool NeedRemoveInput = !GetHasInventoryOpen() && !GetHasContainerOpen();
	Client_CloseInventory(InventoryId, UseGirdKey, NeedRemoveInput);


	//Trigger on server side
	Inventory->OnContainerIsClose.Broadcast(UseGirdKey);
	
}

void UInventoryManagerComponent::Server_CloseMultipleInventories_Implementation(ECloseInventoryPreset Preset, const TArray<FGuid>& InventoryIds)
{
	TArray<FGuid> IdsInventoryClose = InventoryIds;
	TArray<FGuid> IdsEquipmentClose;


	switch (Preset)
	{
	case ECloseInventoryPreset::OnlyArray:
		break;
	case ECloseInventoryPreset::FirstInventory:
		IdsInventoryClose.Add(GetFirstPlayerInventoryId());
		break;
	case ECloseInventoryPreset::Equipment:
		IdsEquipmentClose.Add(EquipementPlayerInventory->GetId());
		break;
	case ECloseInventoryPreset::FirstInventoryAndEquipment:
		IdsInventoryClose.Add(GetFirstPlayerInventoryId());
		IdsEquipmentClose.Add(EquipementPlayerInventory->GetId());
		break;
	case ECloseInventoryPreset::AllInventories:
		IdsInventoryClose.Append(GetInventoriesOpen());
		break;
	case ECloseInventoryPreset::AllInventoriesAndEquipment:
		IdsInventoryClose.Append(GetInventoriesOpen());
		IdsEquipmentClose.Add(EquipementPlayerInventory->GetId());
		break;
	case ECloseInventoryPreset::LastContainer:
		IdsInventoryClose.Add(GetLastContainerOpen());
		break;
	case ECloseInventoryPreset::AllContainers:
		IdsInventoryClose.Append(GetContainersOpen());
		break;
	case ECloseInventoryPreset::AllContainersAndAllInventoriesAndEquipment:
		IdsInventoryClose.Append(GetInventoriesOpen());
		IdsInventoryClose.Append(GetContainersOpen());
		IdsEquipmentClose.Append(GetEquipmentsOpen());
		break;
	default:
		break;
	}

	for (auto& Inv : IdsInventoryClose)
	{
		Server_CloseInventory(Inv);
	}

	for (auto& Equi : IdsEquipmentClose)
	{
		Server_CloseEquipment(Equi);
	}
}

FGuid UInventoryManagerComponent::GetSmartContainerOpen(FItemData Item)
{
	if (!Item.IsValid())
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::GetSmartContainerOpen] Item Invalid"))
		return FGuid();
	}

	for (auto InventoryId : ContainersOpen)
	{
		if (UInventoryComponent* Inventory = GetInventory(InventoryId); IsValid(Inventory))
		{
			if (Inventory->IsFull())
			{
				continue;
			}

			if (Inventory->CheckItemInRestriction(Item))
			{
				return Inventory->GetId();
			}
		}
		else
		{
			UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::GetSmartContainerOpen] InventoryComponent Containers Invalid"))

		}
	}

	UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::GetSmartContainerOpen] Containers open with restriction can't find"))

	return FGuid();
}

FGuid UInventoryManagerComponent::GetSmartToInventory(FGuid FromInventory)
{
	if (!FromInventory.IsValid())
	{
		UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::GetSmartToInventory]FromInventory Invalid"))
		return FGuid();
	}

	if (UInventoryComponent* Inventory = GetInventory(FromInventory); IsValid(Inventory))
	{
		if (Inventory->GetInventoryType() == ESlotType::EInventory)
		{
			
			UE_LOG(LogInventory,Verbose,TEXT("[UInventoryManagerComponent::GetSmartToInventory] Return Last Container Open InventoryId:[%s]"), *GetLastContainerOpen().ToString())
			if (!GetContainersOpen().IsEmpty())
			{
				if (GetLastContainerOpen().IsValid())
				{
					return GetLastContainerOpen();

				}
			}
		}
		else
		{
			UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::GetSmartToInventory] Return the First Player Inventory InventoryId:[%s] Owner:[%s]"), *GetFirstPlayerInventoryId().ToString(),*Inventory->GetOwner()->GetFName().ToString())
			return GetFirstPlayerInventoryId();
		}
	}

	return FGuid();
}

FGuid UInventoryManagerComponent::GetSmartToInventoryWithItem(FGuid FromInventory, FItemData Item)
{
	if (!FromInventory.IsValid())
	{
		UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::GetSmartToInventoryWithItem]FromInventory Invalid"))
			return FGuid();
	}

	if (UInventoryComponent* Inventory = GetInventory(FromInventory); IsValid(Inventory))
	{

		if (Inventory->GetInventoryType() == ESlotType::EInventory)
		{
			if (!GetContainersOpen().IsEmpty())
			{
				FGuid ContainerFound = GetSmartContainerOpen(Item);
				if (!ContainerFound.IsValid())
				{
					UE_LOG(LogInventory, Display, TEXT("[UInventoryManagerComponent::GetSmartToInventoryWithItem] Containers Found Invalid Id:[%s]"), *ContainerFound.ToString())
					return FGuid();
				}

				UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::GetSmartToInventoryWithItem] Containers open Found :[%s]"),*ContainerFound.ToString())

				return ContainerFound;
			}
			else
			{
				UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::GetSmartToInventoryWithItem]  Can't find Containers open, Try to found other Player Inventory"))
				return GetSmartPlayerInventoryId(Item,true,FromInventory,true);
			}

		}
		else
		{
	
			UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::GetSmartToInventoryWithItem] Try to found Player Inventory"))
			return GetSmartPlayerInventoryId(Item);
		}
	}
	else
	{
		UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::GetSmartToInventoryWithItem] InventoryComponent Invalid"))

	}

	return FGuid();
}

void UInventoryManagerComponent::Server_OpenEquipment_Implementation(UInventoryComponent* Inventory, FName Context)
{
	/*if (!EquipmentInventoryId.IsValid())
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Server_OpenEquipment] Invalid EquipmentInventoryId:[%s]"),*EquipmentInventoryId.ToString());
		return;
	}*/


	if (!GetOwner()->HasAuthority())
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Server_OpenEquipment] Need to run on the server"));
		return;
	}


	//UInventoryComponent* Inventory = GetInventory(EquipmentInventoryId);

	if (!IsValid(Inventory))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Server_OpenEquipment] EquipmentInventoryComponent Invalid"));
		return;
	}

	if(Context.IsNone())
	{
		return;
	}


	if (GetInvSubsystem()->ContainsInventory(Inventory->GetId()))
	{
		UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::Server_OpenEquipment] InventoryId add on Instance Subsystem"));
		//Ensure instance has this inventory
		Inventory->Multicast_UpdateId(Inventory->GetId());
	}


	//Inventory->ListInventoryManagerViewers.Add(this);
	Inventory->AddViewer(this, FName());

	EquipmentsOpen.Add(Inventory->GetId());

	//Server transfer data to client
	Client_LoadEquipement(Inventory->InventoryID, Context, Inventory->GetDataForTransfer());

	//Trigger on server side
	Inventory->OnContainerIsOpen.Broadcast(Context);

}

void UInventoryManagerComponent::Server_CloseEquipment_Implementation(FGuid EquipmentInventoryId)
{
	if (!EquipmentInventoryId.IsValid())
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Server_CloseEquipment] Invalid EquipmentInventoryId:[%s]"), *EquipmentInventoryId.ToString());
		return;
	}

	/*if (Context.IsNone())
	{
		return;
	}*/

	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	//Check local instance if has context
	//if (GetNetMode() == ENetMode::NM_ListenServer || GetNetMode() == ENetMode::NM_Standalone)
	//{
	//	if (!MatchContextEquipments.Contains(EquipmentInventoryId))
	//	{
	//		return;
	//	}
	//}

	//UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Server_CloseEquipment]  Close EquipmentInventoryId:[%s] Owner:[%s]"), *EquipmentInventoryId.ToString(),*GetOwner()->GetFName().ToString());


	UInventoryComponent* Inventory = GetInventory(EquipmentInventoryId);
	if (!IsValid(Inventory))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Server_CloseEquipment] EquipmentInventoryComponent Invalid Can't remove Viwers EquipmentInventoryId:[%s]"), *EquipmentInventoryId.ToString());
		return;
	}
	
	Client_ClearEquipement(EquipmentInventoryId);

	EquipmentsOpen.Remove(EquipmentInventoryId);

	Inventory->RemoveViewer(this);

	//Trigger on server side
	Inventory->OnContainerIsClose.Broadcast(FName());
}

void UInventoryManagerComponent::Server_LoadHotbar_Implementation()
{
	if (!IsValid(HotbarInventory))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Server_LoadHotbar] HotbarInventory Component Invalid"));
		return;
	}

	if (!HotbarInventory->GetId().IsValid())
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Server_LoadHotbar] HotbarInventory InventoryId Invalid"));
		return;
	}


	if (!GetOwner()->HasAuthority())
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Server_LoadHotbar] need to Run On Server"));
		return;
	}

	if (!GetInvSubsystem()->ContainsInventory(HotbarInventory->GetId()))
	{
		UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::Server_LoadHotbar] InventoryId add on Instance Subsystem"));
		//Ensure instance has this inventory
		HotbarInventory->Multicast_UpdateId(HotbarInventory->GetId());
	}



	Client_ClearHotbar(HotbarInventory->GetId());

	UE_LOG(LogInventory, Display, TEXT("[UInventoryManagerComponent::Server_LoadHotbar] Lenght Hotbar Data:[%d]"), HotbarInventory->GetDataForTransfer().Num());
	Client_LoadHotbar(HotbarInventory->GetId(), HotbarInventory->GetDataForTransfer());
}

void UInventoryManagerComponent::SetIsDraggable_Implementation(FGuid InventoryId, int32 Slot, bool IsDraggable)
{
	UInventoryComponent* LocalInventory = GetInventory(InventoryId);
	if (!IsValid(LocalInventory))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::SetIsDraggable] InventoryComponent Invalid"));
		return;
	}

	FItemData ModifyItem = LocalInventory->GetInventoryItem(Slot);
	if (!ModifyItem.IsValid())
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::SetIsDraggable] Item Invalid on InventoryId:[%s] InventorySlot:[%d]"), *InventoryId.ToString(), Slot);
		return;
	}

	ModifyItem.bIsDraggable = IsDraggable;
	TArray<int32> FilledIndex;
	LocalInventory->SetInventoryItem(Slot, ModifyItem,FilledIndex);
	FItemDataInfoCompressed LocalItemInfoCompressed = UChestoryFunctionLibrary::MakeItemTransfer(ModifyItem);

	if (!GetInvSubsystem()->ContainsInventory(InventoryId))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::SetIsDraggable] Inventory not found with InventoryId:[%s]"), *InventoryId.ToString())
		return;
	}

	SetViewersInventorySlot(InventoryId,LocalInventory->GetViewers(), Slot, LocalItemInfoCompressed);
}


void UInventoryManagerComponent::SetIsDroppable_Implementation(FGuid InventoryId, int32 Slot, bool IsDroppable)
{
	UInventoryComponent* LocalInventory = GetInventory(InventoryId);
	if (!IsValid(LocalInventory))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::SetIsDroppable] InventoryComponent Invalid"));
		return;
	}

	FItemData ModifyItem = LocalInventory->GetInventoryItem(Slot);
	if (!ModifyItem.IsValid())
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::SetIsDroppable] Item Invalid on InventoryId:[%s] InventorySlot:[%d]"), *InventoryId.ToString(), Slot);
		return;
	}

	ModifyItem.bIsDroppable = IsDroppable;
	TArray<int32> FilledIndex;
	LocalInventory->SetInventoryItem(Slot, ModifyItem,FilledIndex);
	FItemDataInfoCompressed LocalItemInfoCompressed = UChestoryFunctionLibrary::MakeItemTransfer(ModifyItem);

	if (!GetInvSubsystem()->ContainsInventory(InventoryId))
	{
		UE_LOG(LogInventory,Warning,TEXT("[UInventoryManagerComponent::SetIsDroppable] Inventory not found with InventoryId:[%s]"),*InventoryId.ToString())
		return;
	}

	//Server Send data to client
	SetViewersInventorySlot(InventoryId, LocalInventory->GetViewers(), Slot, LocalItemInfoCompressed);
}

void UInventoryManagerComponent::SetDurability_Implementation(FGuid InventoryId, int32 Slot,bool Increase, float Durability)
{
	UInventoryComponent* LocalInventory = GetInventory(InventoryId);
	if (!IsValid(LocalInventory))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::SetDurability] InventoryComponent Invalid"));
		return;
	}

	FItemData ModifyItem = LocalInventory->GetInventoryItem(Slot);
	if (!ModifyItem.IsValid())
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::SetDurability] Item Invalid on InventoryId:[%s] InventorySlot:[%d]"), *InventoryId.ToString(), Slot);
		return;
	}
	
	
	if (Increase)
	{
		ModifyItem.Durability += Durability;
	}
	else //Set
	{
		ModifyItem.Durability = Durability;	
	}

	//Clamp
	ModifyItem.Durability =	FMath::Clamp(ModifyItem.Durability,0.0f,ModifyItem.MaxDurability);
	
	TArray<int32> FilledIndex;
	LocalInventory->SetInventoryItem(Slot, ModifyItem,FilledIndex);

	SetViewersDurabilitySlot(InventoryId, LocalInventory->GetViewers(), Slot, ModifyItem.Durability);
}

void UInventoryManagerComponent::AddGameplayTagOnItem_Implementation(FGuid InventoryId, int32 Slot, FGameplayTag Tag)
{
	UInventoryComponent* LocalInventory = GetInventory(InventoryId);

	if (!IsValid(LocalInventory))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::AddGameplayTagOnItem] InventoryComponent Invalid"));
		return;
	}

	FItemData ModifyItem = LocalInventory->GetInventoryItem(Slot);

	if (!ModifyItem.IsValid())
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::AddGameplayTagOnItem] Item Invalid on InventoryId:[%s] InventorySlot:[%d]"),*InventoryId.ToString(),Slot);
		return;
	}


	ModifyItem.GameplayTagContainer.AddTag(Tag);
	TArray<int32> FilledIndex;
	LocalInventory->SetInventoryItem(Slot, ModifyItem,FilledIndex);

	UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::AddGameplayTagOnItem] Add Tag:[%s] on InventoryId:[%s] InventorySlot:[%d]"),*Tag.GetTagName().ToString(), *InventoryId.ToString(), Slot);

	//Send to client
	SetViewersGameplayTagSlot(InventoryId, LocalInventory->GetViewers(), Slot, Tag, true);
}

void UInventoryManagerComponent::RemoveGameplayTagOnItem_Implementation(FGuid InventoryId, int32 Slot, FGameplayTag Tag)
{
	UInventoryComponent* LocalInventory = GetInventory(InventoryId);

	if (!IsValid(LocalInventory))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::RemoveGameplayTagOnItem] InventoryComponent Invalid"));
		return;
	}

	FItemData ModifyItem = LocalInventory->GetInventoryItem(Slot);

	if (!ModifyItem.IsValid())
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::RemoveGameplayTagOnItem] Item Invalid on InventoryId:[%s] InventorySlot:[%d]"), *InventoryId.ToString(), Slot);
		return;
	}

	if (!ModifyItem.GameplayTagContainer.HasTag(Tag))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::RemoveGameplayTagOnItem] Tag:[%s] not found on GameplayTagContainer InventoryId:[%s] InventorySlot:[%d]"), *Tag.GetTagName().ToString(), *InventoryId.ToString(), Slot);
		return;
	}

	ModifyItem.GameplayTagContainer.RemoveTag(Tag);

	TArray<int32> FilledIndex;
	LocalInventory->SetInventoryItem(Slot, ModifyItem,FilledIndex);

	UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::RemoveGameplayTagOnItem] Remove Tag:[%s] on InventoryId:[%s] InventorySlot:[%d]"), *Tag.GetTagName().ToString(), *InventoryId.ToString(), Slot);

	//Send to client
	SetViewersGameplayTagSlot(InventoryId, LocalInventory->GetViewers(), Slot, Tag, false);
}


void UInventoryManagerComponent::SetJsonParameter_Implementation(FGuid InventoryId, int32 Slot, const FString& JsonParameter)
{
	UInventoryComponent* LocalInventory = GetInventory(InventoryId);
	if (!IsValid(LocalInventory))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::SetJsonParameter] InventoryComponent Invalid"));
		return;
	}

	FItemData ModifyItem = LocalInventory->GetInventoryItem(Slot);
	if (!ModifyItem.IsValid())
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::SetJsonParameter] Item Invalid on InventoryId:[%s] InventorySlot:[%d]"), *InventoryId.ToString(), Slot);
		return;
	}


	ModifyItem.JsonParameter = JsonParameter;


	TArray<int32> FilledIndex;
	LocalInventory->SetInventoryItem(Slot, ModifyItem, FilledIndex);

	//TODO ItemScript OnJsonParameterChanged here ??

	//Send Client 
	SetViewersJsonParameterSlot(InventoryId, LocalInventory->GetViewers(), Slot, JsonParameter);
}



void UInventoryManagerComponent::LockSlot_Implementation(FGuid InventoryId, int32 Index)
{
	if (!InventoryId.IsValid())
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::LockSlot] InventoryId Invalid"));
		return;
	}

	if (UInventoryComponent* Inventory = GetInventory(InventoryId); IsValid(Inventory))
	{
		if (!Inventory->GetInventoryItem(Index).IsEmpty())
		{
			SetIsDraggable(InventoryId, Index, false);
			SetIsDroppable(InventoryId, Index, false);

			//Inventory->GetItemByRef(Index).bIsLock = true;

			//Client
			SetViewersLockSlot(InventoryId, Inventory->GetViewers(), Index);
		}
	}
	else
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::LockSlot] InventoryComponent Invalid"));
		return;
	}
}

void UInventoryManagerComponent::UnlockSlot_Implementation(FGuid InventoryId, int32 Index)
{
	if (!InventoryId.IsValid())
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::UnlockSlot] InventoryId Invalid"));
		return;
	}

	if (UInventoryComponent* Inventory = GetInventory(InventoryId); IsValid(Inventory))
	{
		if (!Inventory->GetInventoryItem(Index).IsEmpty())
		{
			SetIsDraggable(InventoryId, Index, true);
			SetIsDroppable(InventoryId, Index, true);

			//Inventory->GetItemByRef(Index).bIsLock = false;

			//Client
			SetViewersUnlockSlot(InventoryId, Inventory->GetViewers(), Index);
		}
	}
	else
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::UnlockSlot] InventoryComponent Invalid"));
		return;
	}
}

void UInventoryManagerComponent::EnableEquipementSlot_Implementation(FGameplayTag Tag)
{
	EquipementTypeDisable.RemoveTag(Tag);

	for (int32 i = 0; i < EquipementPlayerInventory->GetInventoryItems().Num(); i++)
	{
		if (EquipementPlayerInventory->GetInventoryItem(i).GameplayTagContainer.HasTag(Tag))
		{
			SetIsDraggable(EquipementPlayerInventory->InventoryID, i, true);
			SetIsDroppable(EquipementPlayerInventory->InventoryID, i, true);
		}
	}

	Client_EnableEquipementSlot(EquipementPlayerInventory->GetId(),Tag);
}

void UInventoryManagerComponent::DisableEquipementSlot_Implementation(FGameplayTag Tag, bool Unequip)
{
	EquipementTypeDisable.AddTag(Tag);


	// Unequip equipement with same type tag
	
		for (int32 i = 0; i < EquipementPlayerInventory->GetInventoryItems().Num(); i++)
		{
			if (EquipementPlayerInventory->GetInventoryItem(i).GameplayTagContainer.HasTag(Tag))
			{
				if (Unequip)
				{
					bool HasSpace = false;
					int32 SlotEmpty;
					GetInventory(GetFirstPlayerInventoryId())->GetEmptyInventorySpace(HasSpace, SlotEmpty, EquipementPlayerInventory->GetInventoryItem(i));
					if (HasSpace)
					{
						UnequipItem(EquipementPlayerInventory, i, GetInventory(GetFirstPlayerInventoryId()), SlotEmpty);
						
					}
					else
					{
						UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::DisableEquipementSlot] Can't unequip Inventory is Full"));
					}
				}

				SetIsDraggable(EquipementPlayerInventory->InventoryID, i, false);
				SetIsDroppable(EquipementPlayerInventory->InventoryID, i, false);
			}

			//Client ça ne marche obliger de passer par le Tag Dommage pas la fonction universel
			Client_DisableEquipementSlot(EquipementPlayerInventory->GetId(), Tag);
		}
}






void UInventoryManagerComponent::Server_DropItem_Implementation(FGuid InventoryId, int32 InventorySlot)
{
	if (UInventoryComponent* Inventory = GetInventory(InventoryId); IsValid(Inventory))
	{
		if (Inventory->GetIsHotbarInventory() && bUseHotbarAsShortcut)
		{
			Client_ClearHotBarSlotItem(InventoryId, InventorySlot);
		}

	

		DropItem(Inventory, InventorySlot);

	}
}

void UInventoryManagerComponent::Server_CustomSplitItem_Implementation(FGuid InventoryId, int32 InventorySlot, int32 Amount)
{
	UInventoryComponent* LocalInventory = GetInventory(InventoryId);
	
	if (IsValid(LocalInventory))
	{
		bool InventoryIsNotFull = false;
		int32 IndexSlotEmpty;

		LocalInventory->GetEmptyInventorySpace(InventoryIsNotFull, IndexSlotEmpty, LocalInventory->GetInventoryItem(InventorySlot));

		if (InventoryIsNotFull)
		{
			SplitItem(LocalInventory, InventorySlot, LocalInventory, IndexSlotEmpty, Amount);
		}
		
	}
}

// ---------------------- SOCKET MANAGEMENT ----------------------

void UInventoryManagerComponent::AddSocketToItem(UInventoryComponent* Inventory, int32 IndexSlot, FSocketItem Socket, int32 SocketSlot)
{
	if (!IsValid(Inventory)) {return;}

	FItemData Item = Inventory->GetInventoryItem(IndexSlot);
	if (Item.IsValid() && Item.bCanSockets)
	{
		if (Item.Sockets.IsValidIndex(SocketSlot))
		{

			Item.Sockets[SocketSlot].SocketTiny = Socket.SocketTiny;
			Item.Sockets[SocketSlot].PrimaryAssetId = Socket.PrimaryAssetId;
			Item.Sockets[SocketSlot].Attributes = Socket.Attributes; //TODO Delet Attribute


			TArray<int32> FilledIndex;
			Inventory->SetInventoryItem(IndexSlot, Item,FilledIndex);

			//Replication Client
			SetViewersSocketSlot(Inventory->GetId(), Inventory->GetViewers(), IndexSlot, SocketSlot, Item.Sockets[SocketSlot]);

			OnEquipementChanged.Broadcast();

			Item.NotifyScript_SocketItemAdded(this, Inventory->GetId(), IndexSlot, Socket, SocketSlot);
			
		}
	}

}

void UInventoryManagerComponent::RemoveSocketToitem(UInventoryComponent* Inventory, int32 IndexSlot, int32 SocketSlot)
{
	if (!IsValid(Inventory)) { return; }

	FItemData Item = Inventory->GetInventoryItem(IndexSlot);
	if (Item.IsValid() && Item.bCanSockets)
	{
		if (Item.Sockets.IsValidIndex(SocketSlot))
		{
			Item.NotifyScript_SocketItemRemoved(this, Inventory->GetId(), IndexSlot, Item.Sockets[SocketSlot], SocketSlot);

			//Clear Data Socket
			Item.Sockets[SocketSlot].ClearSocket();
			TArray<int32> FilledIndex;
			Inventory->SetInventoryItem(IndexSlot, Item,FilledIndex);


			//Replication Client
			SetViewersSocketSlot(Inventory->GetId(), Inventory->GetViewers(), IndexSlot, SocketSlot, FSocketItem());

			//@TODO A check OnItemLeaveInventory
			Inventory->OnItemLeaveInventory.Broadcast(UChestoryFunctionLibrary::GetPrimaryAssetIdFromItemData(GetWorld(), Inventory->GetInventoryItem(IndexSlot)), 0);
			OnEquipementChanged.Broadcast();
		}
	}
}

void UInventoryManagerComponent::MoveSocketToSocket(UInventoryComponent* Inventory, int32 IndexSlot, int32 FromSocketSlot, int32 ToSocketSlot)
{
	if (!IsValid(Inventory)) { return; }

	FItemData Item = Inventory->GetInventoryItem(IndexSlot);
	if (Item.IsValid() && Item.bCanSockets)
	{
		FSocketItem FromSocket = Item.Sockets[FromSocketSlot];
		FSocketItem ToSocket = Item.Sockets[ToSocketSlot];

		if (!CheckSocketRestrictionSlot(Inventory, IndexSlot, ToSocketSlot, FromSocket) || !CheckSocketRestrictionSlot(Inventory, IndexSlot, FromSocketSlot, ToSocket))
		{
			UE_LOG(LogInventory, Verbose, TEXT("[%hs] Can't deposit Socket restriction return false"), __FUNCTION__);
			return;
		}

		TArray<FSocketItem> SocketsItem = Item.Sockets;
		SocketsItem[FromSocketSlot].PrimaryAssetId = ToSocket.PrimaryAssetId;
		SocketsItem[FromSocketSlot].Attributes = ToSocket.Attributes;
		SocketsItem[FromSocketSlot].SocketTiny = ToSocket.SocketTiny;

		SocketsItem[ToSocketSlot].PrimaryAssetId = FromSocket.PrimaryAssetId;
		SocketsItem[ToSocketSlot].Attributes = FromSocket.Attributes;
		SocketsItem[ToSocketSlot].SocketTiny = FromSocket.SocketTiny;


		Item.Sockets = SocketsItem;

	//	UE_LOG(LogInventory, Warning, TEXT("Set Inventory Item Slot : %d"), IndexSlot);
		TArray<int32> FilledIndex;
		Inventory->SetInventoryItem(IndexSlot, Item,FilledIndex);


		//Replication Client
		SetViewersSocketSlot(Inventory->GetId(), Inventory->GetViewers(), IndexSlot, ToSocketSlot, FromSocket);
		SetViewersSocketSlot(Inventory->GetId(), Inventory->GetViewers(), IndexSlot, FromSocketSlot, ToSocket);
	}
}

void UInventoryManagerComponent::MoveSocketToAnotherItemSocket(UInventoryComponent* FromInventory, int32 FromIndexSlot, int32 FromSocketSlot, UInventoryComponent* ToInventory, int32 ToIndexSlot, int32 ToSocketSlot)
{
	if (!IsValid(FromInventory) && !IsValid(ToInventory)) {return;}

	FItemData FromItem = FromInventory->GetInventoryItem(FromIndexSlot);
	FItemData ToItem = ToInventory->GetInventoryItem(ToIndexSlot);

	if (FromItem.Sockets.IsValidIndex(FromSocketSlot) && ToItem.Sockets.IsValidIndex(ToSocketSlot))
	{
		FSocketItem FromSocket = FromItem.Sockets[FromSocketSlot];
		FSocketItem ToScket = ToItem.Sockets[ToSocketSlot];

		if (!CheckSocketRestrictionSlot(ToInventory, ToIndexSlot, ToSocketSlot, FromSocket) || !CheckSocketRestrictionSlot(FromInventory, FromIndexSlot, FromSocketSlot, ToScket))
		{
			UE_LOG(LogInventory, Verbose, TEXT("[%hs] Can't deposit Socket restriction return false"), __FUNCTION__);
			return;
		}

		//Empty To Slot Socket just Add
		if (!ToItem.Sockets[ToSocketSlot].PrimaryAssetId.IsValid())
		{
			AddSocketToItem(ToInventory, ToIndexSlot, FromSocket, ToSocketSlot);
			RemoveSocketToitem(FromInventory, FromIndexSlot, FromSocketSlot);
		}
		else
		{
			if (bCanSwapSocket)
			{
				UE_LOG(LogInventory, Verbose, TEXT("[MoveSocketToAnotherItemSocket] Swap Socket Item From/To"));
				FromItem.NotifyScript_SocketItemRemoved(this, FromInventory->GetId(), FromIndexSlot, FromSocket, FromSocketSlot);
				ToItem.NotifyScript_SocketItemRemoved(this, ToInventory->GetId(), ToIndexSlot, ToScket, ToSocketSlot);

				//Swap Socket item
				AddSocketToItem(ToInventory, ToIndexSlot, FromSocket, ToSocketSlot);
				AddSocketToItem(FromInventory, FromIndexSlot, ToScket, FromSocketSlot);
			}
		}
	}
}

bool UInventoryManagerComponent::CheckSocketRestrictionSlot(UInventoryComponent* ToInventory, int32 ToIndexSlot, int32 ToSocketSlot, FSocketItem SocketCheck)
{
	if (!IsValid(ToInventory))
	{
		UE_LOG(LogInventory, Warning, TEXT("[%hs] Inventory Invalid"), __FUNCTION__);
		return false;
	}

	FItemData ItemSlot = ToInventory->GetInventoryItem(ToIndexSlot);

	if(!ItemSlot.IsValid())
	{
		UE_LOG(LogInventory, Verbose, TEXT("[%hs] Item Slot Invalid"), __FUNCTION__);
		return false;
	}

	if (!ItemSlot.Sockets.IsValidIndex(ToSocketSlot))
	{
		UE_LOG(LogInventory, Verbose, TEXT("[%hs] Socket Slot Index Invalid"), __FUNCTION__);
		return false;
	}


	FSocketItem ToSocketItem = ItemSlot.Sockets[ToSocketSlot];

	FItemData FromSocketItemData = UChestoryFunctionLibrary::SocketToItemData(GetWorld(), SocketCheck);
	if (!FromSocketItemData.IsValid())
	{
		return true;
	}

	if (ToSocketItem.Restriction.IsEmpty())
	{
		UE_LOG(LogInventory, Verbose, TEXT("[%hs] Success Restriction slot is Empty"), __FUNCTION__);
		return true;
	}

	if (!ToSocketItem.Restriction.HasAnyExact(FromSocketItemData.GameplayTagContainer))
	{
		UE_LOG(LogInventory, Verbose, TEXT("[%hs] Restriction Matche Query return false"), __FUNCTION__);
		return false;
	}

	UE_LOG(LogInventory, VeryVerbose, TEXT("[%hs] Success Restriction SocketCheckId:[%s]"), __FUNCTION__, *SocketCheck.SocketTiny.ID.ToString());

	return true;
}

void UInventoryManagerComponent::Server_MoveSocketItem_Implementation(FGuid FromInventoryId, int32 FromSlot, int32 FromSocketSlot, FGuid ToInventoryId, int32 ToSlot, int32 ToSocketSlot)
{
	UInventoryComponent* FromInventory = GetInventory(FromInventoryId);
	UInventoryComponent* ToInventory = GetInventory(ToInventoryId);

	if (!IsValid(FromInventory) && !IsValid(ToInventory)) { return; }

	//Same item Juste move inner
	if (FromInventoryId == ToInventoryId && FromSlot == ToSlot)
	{
		UE_LOG(LogInventory, Verbose, TEXT("[Server_MoveSocketItem] Same Slot just swap Socket"));
		MoveSocketToSocket(FromInventory, FromSlot, FromSocketSlot, ToSocketSlot);
	}
	else
	{
		MoveSocketToAnotherItemSocket(FromInventory, FromSlot, FromSocketSlot, ToInventory, ToSlot, ToSocketSlot);
	}
}

void UInventoryManagerComponent::Server_TakeSocketItem_Implementation(FGuid FromInventoryId, int32 FromSlot, int32 FromSocketSlot, FGuid ToInventoryId, int32 ToSlot)
{
	UInventoryComponent* FromInventory = GetInventory(FromInventoryId);
	UInventoryComponent* ToInventory = GetInventory(ToInventoryId);

	if (!IsValid(FromInventory) && !IsValid(ToInventory)) {return;}

	FSocketItem TakeSocket = FromInventory->GetInventoryItem(FromSlot).Sockets[FromSocketSlot];

	// Have Space juste create a new Item
	if (ToInventory->GetInventoryItem(ToSlot).IsEmpty())
	{
		AddItem(ToInventory, ToSlot, UChestoryFunctionLibrary::SocketToItemData(GetWorld(), TakeSocket));
		RemoveSocketToitem(FromInventory, FromSlot, FromSocketSlot);

		FromInventory->OnItemLeaveInventory.Broadcast(TakeSocket.PrimaryAssetId, 1);
		ToInventory->OnItemEnterInventory.Broadcast(TakeSocket.PrimaryAssetId, 1);
	}
	else if (ToInventory->GetInventoryItem(ToSlot).ID == TakeSocket.PrimaryAssetId.PrimaryAssetName
		&& ToInventory->GetInventoryItem(ToSlot).Amount != ToInventory->GetInventoryItem(ToSlot).MaxStackable) //Same Item and is not full
	{
		FItemData Item = ToInventory->GetInventoryItem(ToSlot);
		Item.Amount += 1;
		AddItem(ToInventory, ToSlot, Item);
		RemoveSocketToitem(FromInventory, FromSlot, FromSocketSlot);

		FromInventory->OnItemLeaveInventory.Broadcast(TakeSocket.PrimaryAssetId, 1);
		ToInventory->OnItemEnterInventory.Broadcast(TakeSocket.PrimaryAssetId, 1);
	}
	else
	{
		bool FoundSpace = false;
		int32 IndexFound;
		ToInventory->GetEmptyInventorySpace(FoundSpace, IndexFound, UChestoryFunctionLibrary::SocketToItemData(GetWorld(), TakeSocket));
		if (FoundSpace)
		{
			//AddItem(ToInventory, IndexFound, UChestoryFunctionLibrary::SocketToItemData(GetWorld(), TakeSocket));
			FItemData Out;
			int32 IndexOut;
			TryToAddItemDataToInventory(FoundSpace, Out,IndexOut, ToInventory, UChestoryFunctionLibrary::SocketToItemData(GetWorld(), TakeSocket), 1); // Cal OnItemEnter
			RemoveSocketToitem(FromInventory, FromSlot, FromSocketSlot);

			FromInventory->OnItemLeaveInventory.Broadcast(TakeSocket.PrimaryAssetId, 1);
			
		}
		else
		{
			UE_LOG(LogInventory, Verbose, TEXT("[Server_TakeSocketItem] Inventory is full can't Take Socket"));
			return;
		}
	}

}
void UInventoryManagerComponent::Server_DepositSocketItem_Implementation(FGuid FromInventoryId, int32 FromSlot, FGuid ToInventoryId, int32 ToSlot, int32 ToSocketSlot)
{

	UInventoryComponent* FromInventory = GetInventory(FromInventoryId);
	UInventoryComponent* ToInventory = GetInventory(ToInventoryId);
	
	if (!IsValid(FromInventory) || !IsValid(ToInventory)) {return; }

	FItemData FromItem = FromInventory->GetInventoryItem(FromSlot);

	UE_LOG(LogInventory, Verbose, TEXT("[Server_DepositSocketItem] Start Function  FromItem:[%s]  FromIndex:[%d]"),*FromItem.ID.ToString(), FromSlot);
	if (!UChestoryFunctionLibrary::GetIsValidFromItemData(FromItem))
	{
		UE_LOG(LogInventory, Verbose, TEXT("[Server_DepositSocketItem] Item Invalid From InventoryId:[%s] Index:[%d] Item:[%s]"),*FromInventoryId.ToString(), FromSlot, *FromInventory->Inventory[1].ID.ToString());
		return;
	}

	if (FromItem.ItemType == EItemType::ESocket)
	{
		FSocketItem MakeSocket = UChestoryFunctionLibrary::ItemDataToSocket(GetWorld(), FromItem);

		//Check Socket Restriction
		if (!CheckSocketRestrictionSlot(ToInventory, ToSlot, ToSocketSlot, MakeSocket))
		{
			UE_LOG(LogInventory, Verbose, TEXT("[%hs] Can't deposit Socket restriction return false"), __FUNCTION__);
			return;
		}
		
		// Is Empty Slot Socket
		if (!ToInventory->GetInventoryItem(ToSlot).Sockets[ToSocketSlot].PrimaryAssetId.IsValid())
		{
			UE_LOG(LogInventory, Verbose, TEXT("[Server_DepositSocketItem] Is Empty Slot RemoveSlot:[%d]  Update AddSocket ToSlot:[%d] ToSocketSlot:[%d]"), FromSlot, ToSlot, ToSocketSlot);
			if (!FromItem.bIsStackable || FromItem.Amount <= 1)
			{
				RemoveItem(FromInventory, FromSlot);
			}
			else
			{
				FromItem.Amount -= 1;
				AddItem(FromInventory, FromSlot, FromItem);
			}
			
			AddSocketToItem(ToInventory, ToSlot, MakeSocket, ToSocketSlot);

			FromInventory->OnItemLeaveInventory.Broadcast(MakeSocket.PrimaryAssetId, 1);
			ToInventory->OnItemEnterInventory.Broadcast(MakeSocket.PrimaryAssetId, 1);
			

		}
		else // Socket has aleready Item Juste Add Item with the ToSocket
		{
			UE_LOG(LogInventory, Verbose, TEXT("[Server_DepositSocketItem] Has Already Item Try to swap Socket To Item"));
			FItemData ItemSocket = UChestoryFunctionLibrary::SocketToItemData(GetWorld(), ToInventory->GetInventoryItem(ToSlot).Sockets[ToSocketSlot]);
			if (bCanSwapSocket)
			{
				if (ItemSocket.ID != FromItem.ID) // If same Gem socket make nothing 
				{
					bool Success;
					FItemData Out;
					int32 IndexOut;
					TryToAddItemDataToInventory(Success, Out,IndexOut, FromInventory, ItemSocket, 1); //Trigger Event OnItemEnter
					if (!FromItem.bIsStackable || FromItem.Amount <= 1)
					{
						RemoveItem(FromInventory, FromSlot);
					}
					else
					{
						FromItem.Amount -= 1;
						AddItem(FromInventory, FromSlot, FromItem);
					}


				}
				
				//Notify Script Socket Remove
				ToInventory->GetInventoryItem(ToSlot).NotifyScript_SocketItemRemoved(this, ToInventory->GetId(), ToSlot, ToInventory->GetInventoryItem(ToSlot).Sockets[ToSocketSlot], ToSocketSlot);

				AddSocketToItem(ToInventory, ToSlot, MakeSocket, ToSocketSlot);
				//Triger leave
				FromInventory->OnItemLeaveInventory.Broadcast(MakeSocket.PrimaryAssetId, 1);
			}
		}
	}
}

void UInventoryManagerComponent::Server_DropSocketItem_Implementation(FGuid FromInventoryId, int32 FromSlot, int32 FromSocketSlot)
{
	UInventoryComponent* Inventory = GetInventory(FromInventoryId);
	if (!IsValid(Inventory)) 
	{ 
		UE_LOG(LogInventory,Warning,TEXT("[UInventoryManagerComponent::Server_DropSocketItem] InventoryComponent Invalid with InventoryId:[%s]"),*FromInventoryId.ToString())
		return; 
	}

	UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Server_DropSocketItem] InventoryId:[%s] FromSlot:[%d] FromSocketSlot:[%d]"), *FromInventoryId.ToString(), FromSlot, FromSocketSlot)

	DropItemData(Inventory,FromSlot, UChestoryFunctionLibrary::SocketToItemData(GetWorld(),Inventory->GetInventoryItem(FromSlot).Sockets[FromSocketSlot]));

	RemoveSocketToitem(Inventory, FromSlot, FromSocketSlot);
}

void UInventoryManagerComponent::AddSocketOnItem_Implementation(UInventoryComponent* Inventory, int32 InventorySlot, int32 Amount)
{
	if (!IsValid(Inventory)) { return; }

	FItemData Item = Inventory->GetInventoryItem(InventorySlot);


	if (Item.IsValid(); Item.bCanSockets && Amount > 0 && (Item.Sockets.Num() + Amount) <= Item.MaxSocket)
	{
		TArray<FSocketItem> AddedSockets;
		AddedSockets.AddDefaulted(Amount);
		Item.Sockets.Append(AddedSockets);

		AddItem(Inventory, InventorySlot, Item);
	}

}

// ---------------------- REPAIR ITEM ----------------------

void UInventoryManagerComponent::Server_RepairItemWithRecipe_Implementation(FGuid InventoryId, int32 FromSlot,bool UseCraft)
{
	
	if (UInventoryComponent* Inventory = GetInventory(InventoryId); IsValid(Inventory))
	{
		if (!Inventory->bCanRepair )
		{
			return;
		}

		const FItemData ItemData = Inventory->GetInventoryItem(FromSlot);

		if (FItemDataInfoCraft InfoCraft; CanBeRepair(InfoCraft, UChestoryFunctionLibrary::MakeItemDataInfo(this, ItemData)))
		{
			//Update Recipe with repair rule
			bool OutHasResource;
			FItemDataInfoCraft	RepairCraft =	ConvertRecipeToRepairCost(OutHasResource, InventoryId, InfoCraft,FromSlot);

			if (OutHasResource)
			{
				if (!IsValid(GetCrafting(Inventory->CraftingIdLinked)) && UseCraft)
				{
					UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Server_RepairItemWithRecipe] CraftingComponent Invalid with CraftingId:[%s]"),*Inventory->CraftingIdLinked.ToString());
					return;

				}

				if (UseCraft)
				{
					SetIsDraggable(InventoryId,FromSlot,false);
					Server_AddCraftToWaitlist(Inventory->CraftingIdLinked, RepairCraft, 1);
					return;
				}
				else 
				{
					bool CheckSuccess = true;
					for(auto Resource : RepairCraft.Recipe.IngredientNeeded)
					{
						bool OutSuccess;
						int32 OutRemainingToRemove;
						TryToRemoveItemToInventory(OutSuccess,OutRemainingToRemove,Inventory,Resource.PrimaryAssetId,Resource.Amount);

						if (!OutSuccess)
						{
							CheckSuccess = false;
						}
					}
				
					if(CheckSuccess)
					{
						SetDurability(Inventory->GetId(),FromSlot,false,ItemData.MaxDurability);
					}
				}
			
				
			}

			
		}
	}
	else
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Server_RepairItemWithRecipe] InventoryComponent Invalid"));
	}
}


bool UInventoryManagerComponent::CanBeRepair(FItemDataInfoCraft& ItemDataInfoCraft,FItemDataInfo DataInfo)
{
	ItemDataInfoCraft = UChestoryFunctionLibrary::MakeItemInfoCraft(this,GetInvSubsystem()->GetItemDataAssetFromPrimaryAssetId(GetInvSubsystem()->GetPrimaryAssetIdFromName(DataInfo.ID)));
	ItemDataInfoCraft.ItemInformation = DataInfo;
	
	if(ItemDataInfoCraft.ItemInformation.IsEmpty())
	{
		return false;
	}
	if (!ItemDataInfoCraft.ItemInformation.bCanRepairable)
	{
		return false;
	}
	if (ItemDataInfoCraft.ItemInformation.MaxDurability == 0)
	{
		return false;
	}
	if (!IsValidRecipe(GetInvSubsystem()->GetItemDataAssetFromPrimaryAssetId(GetInvSubsystem()->GetPrimaryAssetIdFromName(ItemDataInfoCraft.ItemInformation.ID)), false))
	{
		return false;
	}
	if (ItemDataInfoCraft.ItemInformation.Durability == ItemDataInfoCraft.ItemInformation.MaxDurability)
	{
		return false;
	}

	return true;
	
}

FItemDataInfoCraft UInventoryManagerComponent::ConvertRecipeToRepairCost(bool &HasResource, FGuid InventoryId, FItemDataInfoCraft ItemInfoCraft,int32 IndexSlot)
{

	if (UInventoryComponent* Inventory = GetInventory(InventoryId); IsValid(Inventory))
	{
		const float RatioDurability = ItemInfoCraft.ItemInformation.Durability / ItemInfoCraft.ItemInformation.MaxDurability;


		bool InitResource = true;

		for (auto&& In : ItemInfoCraft.Recipe.IngredientNeeded)
		{
			In.Amount = FMath::CeilToInt((In.Amount * (1 - RatioDurability)) * (RepairPercentage / 100));
			In.Amount = In.Amount == 0 ? 1 : In.Amount; // Clamp Mini 1 value


			bool HasItem;
			CheckInventoryHasItemAmount(HasItem, In.Current, Inventory, In.PrimaryAssetId, In.Amount);
			if (!HasItem)
			{
				InitResource = false;
			}
		}

		HasResource = InitResource;
		ItemInfoCraft.Recipe.TimeToCraft *= (RepairPercentage / 100);
		ItemInfoCraft.bIsRepair = true;
		//ItemInfoCraft.InType = FromType;
		ItemInfoCraft.InventorySlot = IndexSlot;

		return ItemInfoCraft;
	}

	return FItemDataInfoCraft();
}

void UInventoryManagerComponent::Server_FilledItemInventory_Implementation(FGuid InvenotyId, int32 Index)
{
	if (UInventoryComponent* Inventory = GetInventory(InvenotyId))
	{
		if (!Inventory->GetIsGirdInventory() || !Inventory->GetInventoryItem(Index).bIsVisible || !Inventory->EnableSizeItem)
		{
			return;
		}

		TArray<int32> Filled = Inventory->GetFilledIndexFromItemSlot(Inventory->GetInventoryItem(Index), Index);
		Filled.Remove(Index);
		//Inventory->Inventory[Index].FilledSlotsIndex = Filled;
		for (auto const& I : Filled)
		{
			Inventory->Inventory[I].bIsFilled = true;
		}

		SetViewersFilledInventorySlot(InvenotyId, Inventory->GetViewers(), Filled);
	}
}

void UInventoryManagerComponent::Server_UnfilledItemInventory_Implementation(FGuid InvenotyId, int32 Index, int32 IgnoreItemSlot/* = -1*/)
{
	if (UInventoryComponent* Inventory = GetInventory(InvenotyId))
	{
		if (!Inventory->GetIsGirdInventory() || !Inventory->EnableSizeItem)
		{
			return;
		}

		TArray<int32> Unfilled = Inventory->GetFilledIndexFromItemSlot(Inventory->GetInventoryItem(Index), Index);

		//Ignore Slot filled shared 
		if (IgnoreItemSlot != -1)
		{
			TArray<int32> Filled = Inventory->GetFilledIndexFromItemSlot(Inventory->GetInventoryItem(IgnoreItemSlot), IgnoreItemSlot);
			for (auto& IndexRemove : Filled)
			{
				Unfilled.Remove(IndexRemove);
			}
		}

		//Inventory->Inventory[Index].FilledSlotsIndex.Empty();
		//Filled.Remove(Index);
		for (auto const& I : Unfilled)
		{
			Inventory->Inventory[I].bIsFilled = false;
		}

		SetViewersUnfilledInventorySlot(InvenotyId, Inventory->GetViewers(), Unfilled);
	}
}

// ---------------------- USE ITEM ----------------------

void UInventoryManagerComponent::UseInventoryItem_Implementation(FGuid InventoryId, int32 InventorySlot, EUsedInputTrigger InputTrigger)
{
	//yahouuuuuu 
	//UE_LOG(LogInventory, Verbose, TEXT("UseInventoryItem"));

	UInventoryComponent* Inventory = GetInventory(InventoryId);	
	UInventoryComponent* ToInventoryTransfer = nullptr;


	if (!IsValid(Inventory))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::UseInventoryItem] Inventory Invalid"));
		return;
	}

	FItemData ItemUsed = Inventory->GetInventoryItem(InventorySlot);
	if (!ItemUsed.IsValid())
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::UseInventoryItem] Item Invalid"));
		return;
	}

	if (bTransferWhenUseItem)
	{
		ToInventoryTransfer = Inventory->GetIsContainerInventory() ? GetSmartPlayerInventory(ItemUsed, false) : GetInventory(GetSmartContainerOpen(ItemUsed));
	}
	const bool bHasInventoryTransfer = IsValid(ToInventoryTransfer);


	switch (Inventory->GetInventoryType())
	{
	case ESlotType::EEquipement:
		if (bHasInventoryTransfer && !ToInventoryTransfer->EnableLootContainer && bTransferWhenUseItem)
		{
			// Unequip Go Equipement in Inventory
			UseTransferItem(InventoryId, InventorySlot, GetLastContainerOpen());
		}
		else UseEquipementItem(InventoryId, InventorySlot, GetSmartPlayerInventoryId(ItemUsed, false), InputTrigger);
		break;
	case ESlotType::EHotBar:
		if (!bUseHotbarAsShortcut)
		{
			switch (ItemUsed.ItemType)
			{
			case EItemType::EConsumbale: UseConsumableItemInInventory(Inventory, InventorySlot, InputTrigger);
			default:
				//Notify Script ItemUsed
				ItemUsed.NotifyScript_ItemUsed(this, InventoryId, InventorySlot,InputTrigger);
				break;
			}
		}
		break;
	default:
		if (bHasInventoryTransfer && bTransferWhenUseItem)
		{
			//Check loot Container
			if (ToInventoryTransfer->GetIsContainerInventory() && !ToInventoryTransfer->EnableLootContainer || ToInventoryTransfer->GetIsPlayerInventory())
			{
				UseTransferItem(InventoryId, InventorySlot, ToInventoryTransfer->GetId());
				return;
			}
		}
		else
		{
			switch (ItemUsed.ItemType)
			{
			case EItemType::EEquipement: if (IsValid(EquipementPlayerInventory)) UseEquipementItem(InventoryId, InventorySlot, EquipementPlayerInventory->GetId(), InputTrigger);
				break;
			case EItemType::EConsumbale: UseConsumableItemInInventory(Inventory, InventorySlot, InputTrigger);
				break;
			case EItemType::EBlueprint: UseBlueprintItem(InventoryId, InventorySlot, 1, InputTrigger);
				break;
			default:
				//Notify Script UseItem
				ItemUsed.NotifyScript_ItemUsed(this, InventoryId, InventorySlot,InputTrigger);
				break;
			}	
		}
		break;
	}	
}

void UInventoryManagerComponent::UseEquipementItem_Implementation(FGuid FromInventoryId, int32 InventorySlot, FGuid ToInventoryId, EUsedInputTrigger InputTrigger)
{
	if (!FromInventoryId.IsValid() || !ToInventoryId.IsValid())
	{
		return;
	}

	UInventoryComponent* FromInventory = GetInventory(FromInventoryId);
	UInventoryComponent* ToInventory = GetInventory(ToInventoryId);

	if(!IsValid(FromInventory) || !IsValid(ToInventory))
	{
		return;
	}

	if (!FromInventory->GetInventoryItem(InventorySlot).NotifyScript_GetCanUse(this, FromInventoryId, InventorySlot, InputTrigger))
	{
		return;
	}

	FromInventory->GetInventoryItem(InventorySlot).NotifyScript_ItemUsed(this, FromInventoryId, InventorySlot, InputTrigger);

	if (FromInventory->GetIsEquipmentInventory())
	{
		bool HaveSpace = false; int ToIndex;		
		ToInventory->GetEmptyInventorySpace(HaveSpace, ToIndex);
		if (HaveSpace)
		{
			UnequipItem(FromInventory, InventorySlot, ToInventory, ToIndex);
		}
	}
	else
	{
		bool HaveSlot = false; int ToIndex;		
		ToInventory->FindIndexFromRestriction(FromInventory->GetInventoryItem(InventorySlot).GameplayTagContainer, HaveSlot, ToIndex);
		if (HaveSlot)
		{
			EquipItem(FromInventory, InventorySlot, ToInventory, ToIndex);

		}

	}
}


void UInventoryManagerComponent::UseConsumableItemInInventory_Implementation(UInventoryComponent* Inventory, int32 InventorySlot, EUsedInputTrigger InputTrigger)
{
	if (!IsValid(Inventory))
	{	
		return;
	}

	if (!Inventory->GetInventoryItem(InventorySlot).NotifyScript_GetCanUse(this, Inventory->GetId(), InventorySlot,InputTrigger))
	{
		return;
	}

	bool IsEmptySlot; FItemData OutItem; int32 AmountRemoved;

	Inventory->GetInventoryItem(InventorySlot).NotifyScript_ItemUsed(this, Inventory->GetId(), InventorySlot,InputTrigger);

	RemoveToItemAmount(IsEmptySlot, OutItem, AmountRemoved, Inventory->GetInventoryItem(InventorySlot), 1);
	if (IsEmptySlot)
	{
		RemoveItem(Inventory, InventorySlot);
	}
	else
	{
		AddItem(Inventory, InventorySlot, OutItem);
	}	
}


void UInventoryManagerComponent::UseBlueprintItem_Implementation(FGuid InventoryId, int32 InventorySlot,int32 Amount, EUsedInputTrigger InputTrigger)
{
	UInventoryComponent* Inventory = GetInventory(InventoryId);
	
	if (!IsValid(Inventory))
	{
		return;
	}

	// Check has crafting system
	if (!IsValid(GetCrafting(Inventory->CraftingIdLinked)))
	{
		return;
	}

	FItemData ItemData = Inventory->GetInventoryItem(InventorySlot);
	if (!ItemData.IsValid())
	{
		return;
	}

	if (!ItemData.NotifyScript_GetCanUse(this, Inventory->GetId(), InventorySlot,InputTrigger))
	{
		return;
	}

	Inventory->GetInventoryItem(InventorySlot).NotifyScript_ItemUsed(this, Inventory->GetId(), InventorySlot, InputTrigger);

	UItemDataAsset* ItemAsset = GetInvSubsystem()->GetItemDataAssetFromPrimaryAssetId(GetInvSubsystem()->GetPrimaryAssetIdFromName(ItemData.ID));
	if (IsValidRecipe(ItemAsset, true))
	{
		Server_AddCraftToWaitlist(Inventory->CraftingIdLinked, UChestoryFunctionLibrary::MakeItemInfoCraft(this, ItemAsset), Amount);
	}
}


void UInventoryManagerComponent::UseHotbarAsShortuct_Implementation(FGuid InventoryId, FName ItemId, FName SlotKey, EUsedInputTrigger InputTrigger)
{

	UInventoryComponent* Inventory = GetInventory(InventoryId);

	bool HasItem = false; int32 TotalAmount = 0;
	CheckInventoryHasItemAmount(HasItem, TotalAmount, Inventory, GetInvSubsystem()->GetPrimaryAssetIdFromName(ItemId), 1);

	if (!HasItem)
	{
		UE_LOG(LogInventory,Display,TEXT("[UInventoryManagerComponent::UseHotbarAsShortuct] No item found with ID:[%s]"),*ItemId.ToString())
		return;
	}

	for (int32 i = 0; i < Inventory->GetInventoryItems().Num(); i++)
	{
		if (Inventory->GetInventoryItem(i).ID == ItemId)
		{
			UseItemFromType(InventoryId, i,InputTrigger);

			//Update Hotbar value
			Client_SetHotbarSlotItem(HotbarInventory->GetId(), HotbarInventory->GetIndexAtKey(SlotKey), UChestoryFunctionLibrary::MakeItemTransfer(Inventory->GetInventoryItem(i)));
			break;
		}
	}
}



void UInventoryManagerComponent::UseTransferItem_Implementation(FGuid FromInventoryId, int32 FromSlot, FGuid OverrideToInventoryId)
{

	if (!FromInventoryId.IsValid())
	{
		UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::UseTransferItem] FromInventoryId Invalid [%s]"), *FromInventoryId.ToString());
		return;
	}


	UInventoryComponent* FromInventory = GetInventory(FromInventoryId);

	if (!OverrideToInventoryId.IsValid())
	{
		UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::UseTransferItem] OverrideToInventoryId Invalid Try to Found auto Target"));

		OverrideToInventoryId = GetSmartToInventoryWithItem(FromInventoryId, FromInventory->GetInventoryItem(FromSlot));

		if (!OverrideToInventoryId.IsValid())
		{
			UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::UseTransferItem] ToInventoryId not found"));
			return;
		}

	}


	UInventoryComponent* ToInventory = GetInventory(OverrideToInventoryId);

	if (IsValid(FromInventory) && IsValid(ToInventory))
	{
		UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::UseTransferItem] UseTransferItem"));
		if (ToInventory->bIsContainer == true && ToInventory->EnableLootContainer != true || ToInventory->bIsContainer == false)
		{
			FItemData ItemToTransfer = FromInventory->GetInventoryItem(FromSlot);

			if (!ToInventory->CheckItemInRestriction(ItemToTransfer))
			{
				return;
			}

			if (ItemToTransfer.IsValid())
			{
				if (ItemToTransfer.bIsDraggable)
				{
					bool success;
					FItemData ItemDataOut;
					
					//If is equipment Use call Unequip 
					if (FromInventory->GetIsEquipmentInventory())
					{
						UseEquipementItem(FromInventoryId, FromSlot, OverrideToInventoryId);
						return;
					}

					if (!ItemToTransfer.NotifyScript_GetCanMove(this, FromInventoryId, FromSlot, OverrideToInventoryId, -1))
					{
						return;
					}

					int32 IndexOut;
					TryToAddItemDataToInventory(success, ItemDataOut,IndexOut, ToInventory, ItemToTransfer, FromInventory->GetInventoryItem(FromSlot).Amount);
					if (success)
					{
						RemoveItem(FromInventory, FromSlot);
					}
					else
					{
						AddItem(FromInventory, FromSlot, ItemDataOut);
					}
					FromInventory->OnItemLeaveInventory.Broadcast(UChestoryFunctionLibrary::GetPrimaryAssetIdFromItemData(GetWorld(), ItemToTransfer), success ? ItemToTransfer.Amount : ItemDataOut.Amount);

					
				}
			}
		}
	}
}

void UInventoryManagerComponent::UseTransferAllItemById_Implementation(FGuid FromInventoryId, FPrimaryAssetId PrimaryAssetId, FGuid OverrideToInventoryId,bool Quick)
{
	if (!FromInventoryId.IsValid() )
	{
		UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::UseTransferAllItemById] InventoryId Invalid FromId:[%s]"), *FromInventoryId.ToString());
		return;
	}

	UInventoryComponent* FromInventory = GetInventory(FromInventoryId);

	if (!OverrideToInventoryId.IsValid())
	{
		UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::UseTransferAllItemById] OverrideToInventoryId Invalid Try to Found auto Target"));
		

		OverrideToInventoryId = GetSmartToInventoryWithItem(FromInventoryId, UChestoryFunctionLibrary::GetItemDataFromPrimaryAssetId(this, PrimaryAssetId));
		
		if (!OverrideToInventoryId.IsValid())
		{
			UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::UseTransferAllItemById] ToInventoryId not found"));
			return;
		}
	}




	UInventoryComponent* ToInventory = GetInventory(OverrideToInventoryId);

	if (ToInventory->bIsContainer == true && ToInventory->EnableLootContainer != true || ToInventory->bIsContainer == false)
	{
		TMap<int32, int32> FromSlotIdMap;
		TArray<int32> IndexSlots;
		TArray<int32> FromSlotToRemove;
		TArray<int32> FromSlotToUpdate;
		TArray<FItemData> FromItems = FromInventory->GetInventoryItems();
		int32 LimitSpaceToInventory = ToInventory->GetAmountEmptyInventorySpace();
		
		FItemData DefaultItem = GetInvSubsystem()->GetItemDataFromPrimaryAssetId(PrimaryAssetId);
		int32 ReamingResource = 0;
		int32 Reaming = 0;
		TMap<int32,int32> ToItemsSpace;
		TMap<int32,FItemData> ToItemsFill;
		TArray<FItemData> VirtualToItem;

		
		int32 CanSend = 0;
		int32 CanReceive = 0;
		int32 CanAmountTransfer = 0;

		int32 TotalTransfer = 0; // For Event LeaveInventory
		

		

		//@TODO Refaire fonction All Transfer by ID
		int32 ISlot =0;
		// Construct MaxCanBeTransfer
		for(auto Item : ToInventory->GetInventoryItems())
		{
		
			if (Item.ID == PrimaryAssetId.PrimaryAssetName)
			{
				const int32 Space = Item.MaxStackable - Item.Amount;
				if (Space != 0)
				{
					CanReceive += Space;
					ToItemsSpace.Add(ISlot,Space);
				}
			}
			else if(Item.IsEmpty()) //if true Empty SLot 
			{
				CanReceive += DefaultItem.MaxStackable;
				ToItemsSpace.Add(ISlot,DefaultItem.MaxStackable);
			}

			ISlot++;
		}
		
		UE_LOG(LogInventory, Verbose,TEXT("CanReceive Amount Space : %d"), CanReceive);

		// Construct Total Send And Reaming
		for (int32 i = 0; i < FromItems.Num(); i++)
		{
			if (FromItems[i].ID == PrimaryAssetId.PrimaryAssetName)
			{
				IndexSlots.Add(i);
				CanSend += FromInventory->GetInventoryItem(i).Amount;

				if (CanSend >= CanReceive)
				{
					ReamingResource = CanSend - CanReceive;
					UE_LOG(LogInventory,Verbose,TEXT("Max can be transfer achieved has : %d Max : %d Rem : %d"),CanSend,CanReceive,ReamingResource);
					break;
				}
				
				
			}
		}

		UE_LOG(LogInventory, Verbose,TEXT("CanSend Amount Space : %d"), CanSend);
		CanAmountTransfer = FMath::Min(CanSend,CanReceive);
		UE_LOG(LogInventory, Verbose,TEXT("CanAmountTransfer : %d"), CanAmountTransfer);
		
		
		if (Quick)
		{
			int32 SlotToRemove = -1;
			for(auto&& ItemSpace : ToItemsSpace)
			{
				//Check if need to Remove FromSlot
				if(SlotToRemove != -1)
				{
					IndexSlots.Remove(SlotToRemove);
					SlotToRemove = -1; //Reset Remove
				}
				if(IndexSlots.IsEmpty())
				{
					UE_LOG(LogInventory,Verbose,TEXT("End Transfer FromSlot Inventory is Empty"));
					break;
				}

				//int32 StartSpace = ItemSpace.Value
				for(auto Slot : IndexSlots)
				{
					FItemData ItemToAdd = FromItems[Slot];


					if (!ItemToAdd.NotifyScript_GetCanMove(this, FromInventoryId, Slot, OverrideToInventoryId, -1))
					{
						continue;
					}

					// Is Empty Slot && Space is MaxStackable
					UE_LOG(LogInventory,Verbose,TEXT("ToItemSpace :%d"),ItemSpace.Value);
					if(ItemSpace.Value == ItemToAdd.MaxStackable && ItemToAdd.Amount == ItemToAdd.MaxStackable)
					{
						
						UE_LOG(LogInventory,Verbose,TEXT("Amount == MaxStackable"));
						ToItemsFill.Add(ItemSpace.Key,ItemToAdd);
						FromSlotToRemove.Add(Slot);
						SlotToRemove = Slot;

						TotalTransfer += ItemToAdd.Amount;
						break;
						
					}
					
						//Need To Stack With ToSlot
						int32 TmpToAdd = FromItems[Slot].Amount;
						int32 TmpCurrent = ItemToAdd.MaxStackable - ItemSpace.Value;
						UE_LOG(LogInventory,Verbose,TEXT("BEFORE VirtualAddToStack Current %d ToAdd %d "),TmpCurrent,TmpToAdd);
						VirtualAddToStack(TmpCurrent,ItemToAdd.MaxStackable,TmpToAdd);
						FromItems[Slot].Amount = TmpToAdd;
						FromSlotToUpdate.Add(Slot);
						ItemToAdd.Amount = TmpCurrent;
						//ItemToAdd.Decay = FMath::Min(ItemToAdd.Decay, FromItems[Slot].Decay);
						ItemSpace.Value = ItemToAdd.MaxStackable - TmpCurrent;
						UE_LOG(LogInventory,Verbose,TEXT("AFTER VirtualAddToStack Current %d ToAdd %d "),ItemToAdd.Amount,FromItems[Slot].Amount);

						TotalTransfer += TmpCurrent;

						//Check if from == 0 and delet this slot if true
						if(FromItems[Slot].Amount == 0)
						{
							UE_LOG(LogInventory,Verbose,TEXT("From Stack == 0 Remove Slot EndLoop"));
							FromSlotToRemove.Add(Slot);
							SlotToRemove = Slot;
						}

						ToItemsFill.Add(ItemSpace.Key,ItemToAdd);
						// J'ai ajouté et je suis au MAxStackable donc j'ajoute 
						if(ItemToAdd.Amount == ItemToAdd.MaxStackable)
						{
							
							UE_LOG(LogInventory,Verbose,TEXT("Add Max Stack Break Loop From"));
							break;
						}
					
				}
				
			}

			

			for(auto ToAdd : ToItemsFill)
			{
				AddItem(ToInventory,ToAdd.Key,ToAdd.Value);
			}
			for(auto SlotRemove : FromSlotToRemove)
			{
				if(IsValid(FromInventory);FromInventory->GetInventoryItems().IsValidIndex(SlotRemove))
				{
					RemoveItem(FromInventory,SlotRemove);
				}
				
				
				//ClearSlotToUpdate
				if (FromSlotToUpdate.Contains(SlotRemove))
				{
					FromSlotToUpdate.Remove(SlotRemove);
				}
			}

			for(auto SlotUpdate : FromSlotToUpdate)
			{
				if(IsValid(FromInventory))
				{
					AddItem(FromInventory,SlotUpdate,FromItems[SlotUpdate]);
				}
			}
			
			if(false)
			{
				UE_LOG(LogInventory,Warning,TEXT("Try New Methode All Transfer %d"), CanSend);

				bool SuccessOut;
				FItemData ItemOut;
				int32 IndexOut;
				FItemData ItemToAdd = FromInventory->GetInventoryItem(IndexSlots[0]);
				int32 Remaining = 0;

				TryToAddItemDataToInventory(SuccessOut, ItemOut, IndexOut, ToInventory, ItemToAdd, CanSend);
				TryToRemoveItemToInventory(SuccessOut,Remaining,FromInventory,PrimaryAssetId,CanSend);	
			}


			if (false)
			{
				for(int32 i = 0; i < FMath::Min(LimitSpaceToInventory,IndexSlots.Num()) ;i++)
				{
					bool SuccessOut;
					// Check if there is space
					int32 IndexOut;
					ToInventory->GetEmptyInventorySpace(SuccessOut, IndexOut);
					FItemData ItemToAdd = FromInventory->GetInventoryItem(i);

					MoveItem(FromInventory,IndexSlots[i],ToInventory,IndexOut);
				}
			}
			
			
		}
		else
		{
			for (auto SlotMap : IndexSlots)
			{
				bool SuccessOut;
				FItemData ItemOut;
				int32 Fromindex = SlotMap;

				//Find Index if auto size inventory
				if (FromInventory->EnableAutoFill)
				{
					bool FoundSuccess = false;
					int32 IndexFound = -1;

					IndexFound = FromInventory->FindInventoryItem(FoundSuccess, DefaultItem.ID);
					if (FoundSuccess)
					{
						Fromindex = IndexFound;
					}
					else
					{
						return;
					}
				}

				FItemData ItemToAdd = FromInventory->GetInventoryItem(Fromindex);


				// Check if there is space
				int32 IndexOut;
				ToInventory->GetEmptyInventorySpace(SuccessOut, IndexOut, ItemToAdd);
				if (SuccessOut)
				{

					if (!ToInventory->CheckItemInRestriction(ItemToAdd))
					{
						return;
					}

					if (!ItemToAdd.NotifyScript_GetCanMove(this, FromInventoryId, Fromindex, OverrideToInventoryId, -1))
					{
						continue;
					}

				
					//TryToAddItemToInventory(SuccessOut, ItemOut, ToInventory, PrimaryAssetId, ItemToAdd.Amount);
					TryToAddItemDataToInventory(SuccessOut, ItemOut,IndexOut, ToInventory, ItemToAdd, ItemToAdd.Amount);
					if (SuccessOut)
					{
						TotalTransfer += ItemToAdd.Amount;
						RemoveItem(FromInventory, Fromindex);
					}
					else
					{
						TotalTransfer += ItemOut.Amount;
						AddItem(FromInventory, Fromindex, ItemOut);
					}
				}
				else
				{
					break;
				}
			}
		}
		FromInventory->OnItemLeaveInventory.Broadcast(PrimaryAssetId, Quick ? CanAmountTransfer :TotalTransfer);
		//Add is called by TryToAddItemDataToInventory
		//ToInventory->OnItemEnterInventory.Broadcast(PrimaryAssetId, Quick ? CanAmountTransfer :TotalTransfer);
	}
}



void UInventoryManagerComponent::UseItemFromType_Implementation(FGuid InventoryId, int32 InventorySlot, EUsedInputTrigger InputTrigger)
{
	UseInventoryItem(InventoryId, InventorySlot,InputTrigger);
}



void UInventoryManagerComponent::UseDropItemFromType(FGuid InventoryId, int32 FromSlot)
{
	Server_DropItem(InventoryId, FromSlot);
}

void UInventoryManagerComponent::UseRepairItemFromType(FGuid InventoryId, int32 FromSlot,bool UseCraftingSystem)
{
	Server_RepairItemWithRecipe(InventoryId,FromSlot,UseCraftingSystem);
}

void UInventoryManagerComponent::UseBlueprintItemFromType(FGuid InventoryId, int32 InventorySlot,int32 Amount)
{
	UseBlueprintItem(InventoryId,InventorySlot,Amount);
}

void UInventoryManagerComponent::DecrementItem_Implementation(UInventoryComponent* Inventory, int32 InventorySlot, int32 Amount)
{
	if (!IsValid(Inventory))
	{
		return;
	}

	if (Inventory->GetInventoryItem(InventorySlot).IsEmpty())
	{
		return;
	}

	FItemData InItem = Inventory->GetInventoryItem(InventorySlot);

	bool SlotIsEmpty;

	FItemData OutItem;
	int32 AmountRemoved;

	RemoveToItemAmount(SlotIsEmpty, OutItem, AmountRemoved, InItem, Amount);

	if (SlotIsEmpty)
	{
		RemoveItem(Inventory, InventorySlot);
	}
	else
	{
		AddItem(Inventory, InventorySlot, OutItem);
	}
}

void UInventoryManagerComponent::IncrementItem_Implementation(UInventoryComponent* Inventory, int32 InventorySlot, int32 Amount)
{
	if (!IsValid(Inventory))
	{
		return;
	}

	if (Inventory->GetInventoryItem(InventorySlot).IsEmpty())
	{
		return;
	}

	if (Inventory->GetInventoryItem(InventorySlot).bIsStackable)
	{
		int32 Reamaning = AddItemToStack(Inventory, InventorySlot, Amount, Inventory->GetInventoryItem(InventorySlot).Decay);
	}
}

void UInventoryManagerComponent::VirtualAddToStack(int32& Current, const int32 MaxStack, int32& ToAdd)
{
	Current += ToAdd;
	ToAdd = Current > MaxStack ? Current - MaxStack : 0;
	Current = FMath::Clamp(Current,1,MaxStack);

}

int32 UInventoryManagerComponent::GetCurrencyByType(FName Id)
{


	for (int32 i = 0; i < AllCurrency.Num(); i++)
	{
		if (AllCurrency[i].ID == Id)
		{
			return AllCurrency[i].Amount;
		}
	}
	return -1;
}


void UInventoryManagerComponent::SetCurrencyByItemId_Implementation(FName Id, int32 Amount)
{
	for (int32 i = 0; i < AllCurrency.Num(); i++)
	{
		if (AllCurrency[i].ID == Id)
		{
			AllCurrency[i].Amount = Amount;
			UpdateCurrency(Id);
		}
	}
}

void UInventoryManagerComponent::UpdateAllCurrency_Implementation()
{
	for (int32 i = 0; i < AllCurrency.Num(); i++)
	{
		if (GetOwnerRole() == ROLE_Authority)
		{
			UpdateCurrency(AllCurrency[i].ID);
		}
	}
}

bool UInventoryManagerComponent::GetIsCurrencyFromItemID(FName ItemID)
{
	for (int32 i = 0; i < AllCurrency.Num(); i++)
	{
		if (AllCurrency[i].ID == ItemID)
		{
			return true;
		}
	}
	return false;
}

void UInventoryManagerComponent::UpdateCurrency_Implementation(FName Id)
{
	OnUpdateCurrency.Broadcast(Id);
}

// ------------------------------------------------ TRANSFER DATA ------------------------------------------------


void UInventoryManagerComponent::SetViewersInventorySlot(FGuid InventoryId, TMap<class UInventoryManagerComponent*, FName> Viewers, int32 InventorySlot, FItemDataInfoCompressed ItemTransfer)
{

	const FItemDataInfoCompressed CheckItemInfo = ItemTransfer.ItemTiny.bIsVisible ? ItemTransfer : FItemDataInfoCompressed();

	if (!GetInvSubsystem()->ContainsInventory(InventoryId))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::SetViewersInventorySlot] InventoryComponent not found with InventoryId[%s]"), *InventoryId.ToString())
			return;
	}

	if (!IsValid(GetInventory(InventoryId)))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::SetViewersInventorySlot] InventoryComponent Invalid with InventoryId[%s]"), *InventoryId.ToString())
			return;
	}

	//Check if need to use Equipment methode
	if (GetInventory(InventoryId)->GetIsEquipmentInventory())
	{
		for (const auto Viewer : Viewers)
		{
			if (!IsValid(Viewer.Key))
			{
				continue;
			}
			Viewer.Key->Client_SetEquipementSlotItem(InventoryId, InventorySlot, ItemTransfer);

		}
	}
	else if (GetInventory(InventoryId)->GetIsHotbarInventory())
	{
		for (const auto Viewer : Viewers)
		{
			if (!IsValid(Viewer.Key))
			{
				continue;
			}
			Viewer.Key->Client_SetHotbarSlotItem(InventoryId, InventorySlot, ItemTransfer);
		}
	}
	else
	{
		for (const auto Viewer : Viewers)
		{
			if (!IsValid(Viewer.Key))
			{
				continue;
			}
			Viewer.Key->Client_SetInventorySlotItem(InventoryId, Viewer.Value, InventorySlot, ItemTransfer);
		}
	}

}

void UInventoryManagerComponent::ClearViewersInventorySlot(FGuid InventoryId, TMap<class UInventoryManagerComponent*, FName> Viewers, int32 InventorySlot)
{
	if (GetInventory(InventoryId)->GetIsEquipmentInventory())
	{
		for (const auto Viewer : Viewers)
		{
			Viewer.Key->Client_ClearEquipementSlotItem(InventoryId, InventorySlot);

		}
	}
	else if (GetInventory(InventoryId)->GetIsHotbarInventory())
	{
		for (const auto Viewer : Viewers)
		{
			Viewer.Key->Client_ClearHotBarSlotItem(InventoryId, InventorySlot);
		}
	}
	else
	{
		for (const auto Viewer : Viewers)
		{
			Viewer.Key->Client_ClearInventorySlotItem(InventoryId, Viewer.Value, InventorySlot);
		}
	}
}

void UInventoryManagerComponent::SetViewersFilledInventorySlot(FGuid InventoryId, TMap<class UInventoryManagerComponent*, FName> Viewers, const TArray<int32>& IndexFilled)
{
	if (!GetInvSubsystem()->ContainsInventory(InventoryId))
	{
		UE_LOG(LogInventory, Warning, TEXT("[InventoryManagerComponent::SetViewersFilledInventorySlot] InventoryComponent not found with InventoryId[%s]"), *InventoryId.ToString())
		return;
	}

	if (!IsValid(GetInventory(InventoryId)))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::SetViewersFilledInventorySlot] InventoryComponent Invalid with InventoryId[%s]"), *InventoryId.ToString())
		return;
	}

	for (const auto Viewer : Viewers)
	{
		Viewer.Key->Client_FilledInventorySlots(InventoryId, Viewer.Value, IndexFilled);
	}

}

void UInventoryManagerComponent::SetViewersUnfilledInventorySlot(FGuid InventoryId, TMap<class UInventoryManagerComponent*, FName> Viewers, const TArray<int32>& IndexUnfilled)
{
	if (!GetInvSubsystem()->ContainsInventory(InventoryId))
	{
		UE_LOG(LogInventory, Warning, TEXT("[InventoryManagerComponent::SetViewersUnfilledInventorySlot] InventoryComponent not found with InventoryId[%s]"), *InventoryId.ToString())
			return;
	}

	if (!IsValid(GetInventory(InventoryId)))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::SetViewersUnfilledInventorySlot] InventoryComponent Invalid with InventoryId[%s]"), *InventoryId.ToString())
			return;
	}

	for (const auto Viewer : Viewers)
	{
		Viewer.Key->Client_UnfilledInventorySlots(InventoryId, Viewer.Value, IndexUnfilled);
	}
}

void UInventoryManagerComponent::SetViewersCancelOverSlot(FGuid InventoryId, TMap<class UInventoryManagerComponent*, FName> Viewers, const TArray<int32>& IndexCancel)
{
	if (!GetInvSubsystem()->ContainsInventory(InventoryId))
	{
		UE_LOG(LogInventory, Warning, TEXT("[InventoryManagerComponent::SetViewersCancelOverSlot] InventoryComponent not found with InventoryId[%s]"), *InventoryId.ToString())
			return;
	}

	if (!IsValid(GetInventory(InventoryId)))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::SetViewersCancelOverSlot] InventoryComponent Invalid with InventoryId[%s]"), *InventoryId.ToString())
			return;
	}

	for (const auto Viewer : Viewers)
	{
		Viewer.Key->Client_CancelOverSlots(InventoryId, Viewer.Value, IndexCancel);
	}
}

void UInventoryManagerComponent::SetViewersRepairSlot(FGuid InventoryId, TMap<class UInventoryManagerComponent*, FName> Viewers, int32 InventorySlot, FItemDataInfoCraftCompressed Repair)
{
	if (!GetInvSubsystem()->ContainsInventory(InventoryId))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::SetViewersItemCrafting] InventoryComponent not found with InventoryId[%s]"), *InventoryId.ToString())
			return;
	}

	if (!IsValid(GetInventory(InventoryId)))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::SetViewersItemCrafting] InventoryComponent Invalid with InventoryId[%s]"), *InventoryId.ToString())
			return;
	}

	for (const auto Viewer : Viewers)
	{
		Viewer.Key->Client_SetInventoryRepairSlotItem(InventoryId, Viewer.Value, InventorySlot, Repair);
	}

}

void UInventoryManagerComponent::SetViewersBlueprintSlot(FGuid InventoryId, TMap<class UInventoryManagerComponent*, FName> Viewers, int32 InventorySlot, FItemDataInfoCraftCompressed Blueprint)
{
	if (!GetInvSubsystem()->ContainsInventory(InventoryId))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::SetViewersBlueprintSlot] InventoryComponent not found with InventoryId:[%s]"), *InventoryId.ToString())
			return;
	}

	if (!IsValid(GetInventory(InventoryId)))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::SetViewersBlueprintSlot] InventoryComponent Invalid with InventoryId:[%s]"), *InventoryId.ToString())
			return;
	}

	for (const auto Viewer : Viewers)
	{
		Viewer.Key->Client_SetInventoryBlueprintSlotItem(InventoryId, Viewer.Value, InventorySlot, Blueprint);
	}
}

void UInventoryManagerComponent::SetViewersDurabilitySlot(FGuid InventoryId, TMap<class UInventoryManagerComponent*, FName> Viewers, int32 InventorySlot, float NewDurability)
{
	if (!GetInvSubsystem()->ContainsInventory(InventoryId))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::SetViewersDurabilitySlot] InventoryComponent not found with InventoryId:[%s]"), *InventoryId.ToString())
			return;
	}

	if (!IsValid(GetInventory(InventoryId)))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::SetViewersDurabilitySlot] InventoryComponent Invalid with InventoryId:[%s]"), *InventoryId.ToString())
			return;
	}

	for (const auto Viewer : Viewers)
	{
		Viewer.Key->Client_SetDurabilitySlotItem(InventoryId, Viewer.Value, InventorySlot, NewDurability);
	}
}

void UInventoryManagerComponent::SetViewersAttributesSlot(FGuid InventoryId, TMap<class UInventoryManagerComponent*, FName> Viewers, int32 InventorySlot, FGameplayTagAttributeContainer Attributes)
{
	if (!GetInvSubsystem()->ContainsInventory(InventoryId))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::SetViewersAttributesSlot] InventoryComponent not found with InventoryId:[%s]"), *InventoryId.ToString())
		return;
	}

	if (!IsValid(GetInventory(InventoryId)))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::SetViewersAttributesSlot] InventoryComponent Invalid with InventoryId:[%s]"), *InventoryId.ToString())
			return;
	}

	for (const auto Viewer : Viewers)
	{
		Viewer.Key->Client_SetAttributesSlotItem(InventoryId, Viewer.Value, InventorySlot, Attributes);
	}
}

void UInventoryManagerComponent::SetViewersGameplayTagSlot(FGuid InventoryId, TMap<class UInventoryManagerComponent*, FName> Viewers, int32 InventorySlot, FGameplayTag Tag, bool IsAdd)
{
	if (!GetInvSubsystem()->ContainsInventory(InventoryId))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::SetViewersGameplayTagSlot] InventoryComponent not found with InventoryId:[%s]"), *InventoryId.ToString())
			return;
	}

	if (!IsValid(GetInventory(InventoryId)))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::SetViewersGameplayTagSlot] InventoryComponent Invalid with InventoryId:[%s]"), *InventoryId.ToString())
			return;
	}

	for (const auto Viewer : Viewers)
	{
		Viewer.Key->Client_SetGameplayTagSlotItem(InventoryId, Viewer.Value, InventorySlot, Tag,IsAdd);
	}
}

void UInventoryManagerComponent::SetViewersJsonParameterSlot(FGuid InventoryId, TMap<class UInventoryManagerComponent*, FName> Viewers, int32 InventorySlot, FString JsonParameter)
{
	if (!GetInvSubsystem()->ContainsInventory(InventoryId))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::SetViewersJsonParameterSlot] InventoryComponent not found with InventoryId:[%s]"), *InventoryId.ToString())
			return;
	}

	if (!IsValid(GetInventory(InventoryId)))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::SetViewersJsonParameterSlot] InventoryComponent Invalid with InventoryId:[%s]"), *InventoryId.ToString())
			return;
	}

	for (const auto Viewer : Viewers)
	{
		Viewer.Key->Client_SetJsonParameterSlotItem(InventoryId, Viewer.Value, InventorySlot,JsonParameter);
	}
}



void UInventoryManagerComponent::SetViewersSocketSlot(FGuid InventoryId, TMap<class UInventoryManagerComponent*, FName> Viewers, int32 InventorySlot, int32 NewSocketSlot, FSocketItem NewSocket)
{
	if (!GetInvSubsystem()->ContainsInventory(InventoryId))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::SetViewersSocketSlot] InventoryComponent not found with InventoryId:[%s]"), *InventoryId.ToString())
			return;
	}

	if (!IsValid(GetInventory(InventoryId)))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::SetViewersSocketSlot] InventoryComponent Invalid with InventoryId:[%s]"), *InventoryId.ToString())
			return;
	}

	for (const auto Viewer : Viewers)
	{
		Viewer.Key->Client_SetSocketSlotItem(InventoryId, Viewer.Value, InventorySlot, NewSocketSlot, NewSocket);
	}
}

void UInventoryManagerComponent::SetViewersLockSlot(FGuid InventoryId, TMap<class UInventoryManagerComponent*, FName> Viewers, int32 InventorySlot)
{
	if (!GetInvSubsystem()->ContainsInventory(InventoryId))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::SetViewersLockSlot] InventoryComponent not found with InventoryId:[%s]"), *InventoryId.ToString())
			return;
	}

	if (!IsValid(GetInventory(InventoryId)))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::SetViewersLockSlot] InventoryComponent Invalid with InventoryId:[%s]"), *InventoryId.ToString())
			return;
	}

	for (const auto Viewer : Viewers)
	{
		Viewer.Key->Client_LockSlot(InventoryId, Viewer.Value, InventorySlot);
	}
}

void UInventoryManagerComponent::SetViewersUnlockSlot(FGuid InventoryId, TMap<class UInventoryManagerComponent*, FName> Viewers, int32 InventorySlot)
{
	if (!GetInvSubsystem()->ContainsInventory(InventoryId))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::SetViewersUnlockSlot] InventoryComponent not found with InventoryId:[%s]"), *InventoryId.ToString())
			return;
	}

	if (!IsValid(GetInventory(InventoryId)))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::SetViewersUnlockSlot] InventoryComponent Invalid with InventoryId:[%s]"), *InventoryId.ToString())
			return;
	}

	for (const auto Viewer : Viewers)
	{
		Viewer.Key->Client_UnlockSlot(InventoryId, Viewer.Value, InventorySlot);
	}
}

void UInventoryManagerComponent::SetViewersItemCrafting(FGuid InventoryId, TMap<class UInventoryManagerComponent*, FName> Viewers, int32 InventorySlot, FItemDataInfoCraftCompressed Info)
{
	if (!GetInvSubsystem()->ContainsInventory(InventoryId))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::SetViewersItemCrafting] InventoryComponent not found with InventoryId[%s]"), *InventoryId.ToString())
			return;
	}

	if (!IsValid(GetInventory(InventoryId)))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::SetViewersItemCrafting] InventoryComponent Invalid with InventoryId[%s]"), *InventoryId.ToString())
			return;
	}

	for (const auto Viewer : Viewers)
	{
		Viewer.Key->Client_SetItemCrafting(Viewer.Value, InventorySlot, Info);
	}

}

void UInventoryManagerComponent::SetViewersItemAmountCrafting(FGuid InventoryId, TMap<class UInventoryManagerComponent*, FName> Viewers, int32 InventorySlot, const TArray<int32>& ItemCraftAmount, int32 MaxAmount)
{
	if (!GetInvSubsystem()->ContainsInventory(InventoryId))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::SetViewersItemAmountCrafting] InventoryComponent not found with InventoryId[%s]"), *InventoryId.ToString())
			return;
	}

	if (!IsValid(GetInventory(InventoryId)))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::SetViewersItemAmountCrafting] InventoryComponent Invalid with InventoryId[%s]"), *InventoryId.ToString())
			return;
	}

	for (const auto Viewer : Viewers)
	{
		Viewer.Key->Client_SetItemCraftingAmount(Viewer.Value, InventorySlot, ItemCraftAmount, MaxAmount);
	}
}

void UInventoryManagerComponent::AddViewersWaitingSlot(FGuid CraftingId, TMap<class UInventoryManagerComponent*, FName> Viewers, int32 InventorySlot, FItemDataInfoCraftCompressed Info, TSubclassOf<UCraftingSlotUserWidget> ClassWaiting)
{
	if (!GetInvSubsystem()->ContainsCrafting(CraftingId))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::AddViewersWaitingSlot] CraftingComponent not found with CraftingId:[%s]"), *CraftingId.ToString())
			return;
	}

	if (!IsValid(GetCrafting(CraftingId)))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::AddViewersWaitingSlot] CraftingComponent Invalid with CraftingId:[%s]"), *CraftingId.ToString())
			return;
	}


	for (const auto Viewer : Viewers)
	{
		Viewer.Key->Client_AddWaitingSlot(CraftingId, InventorySlot, ClassWaiting, Viewer.Value, Info);
	}
}

void UInventoryManagerComponent::AddViewersItemCrafting(FGuid CraftingId, int32 InventorySlot, TMap<class UInventoryManagerComponent*, FName> Viewers, FItemDataInfoCraftCompressed Info)
{
	if (!GetInvSubsystem()->ContainsCrafting(CraftingId))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::AddViewersItemCrafting] CraftingComponent not found with CraftingId:[%s]"), *CraftingId.ToString())
			return;
	}

	if (!IsValid(GetCrafting(CraftingId)))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::AddViewersItemCrafting] CraftingComponent Invalid with CraftingId:[%s]"), *CraftingId.ToString())
			return;
	}

	for (const auto Viewer : Viewers)
	{
		Viewer.Key->Client_AddCraftingSlot(CraftingId, InventorySlot, GetCrafting(CraftingId)->ClassCraftingSlot, Viewer.Value, Info, GetCrafting(CraftingId));
	}

}

void UInventoryManagerComponent::RemoveViewersItemCrafting(FGuid CraftingId, int32 InventorySlot, TMap<class UInventoryManagerComponent*, FName> Viewers)
{
	if (!GetInvSubsystem()->ContainsCrafting(CraftingId))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::RemoveViewersItemCrafting] CraftingComponent not found with CraftingId:[%s]"), *CraftingId.ToString())
			return;
	}

	if (!IsValid(GetCrafting(CraftingId)))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::RemoveViewersItemCrafting] CraftingComponent Invalid with CraftingId:[%s]"), *CraftingId.ToString())
			return;
	}

	for (const auto Viewer : Viewers)
	{
		Viewer.Key->Client_RemoveCraftingSlot(CraftingId, InventorySlot, Viewer.Value);
	}
}

void UInventoryManagerComponent::SetViewersWaitingSlot(FGuid CraftingId, TMap<class UInventoryManagerComponent*, FName> Viewers, int32 InventorySlot, FItemDataInfoCraftCompressed Info)
{
	if (!GetInvSubsystem()->ContainsCrafting(CraftingId))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::SetViewersWaitingSlot] CraftingComponent not found with CraftingId:[%s]"), *CraftingId.ToString())
			return;
	}

	if (!IsValid(GetCrafting(CraftingId)))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::SetViewersWaitingSlot] CraftingComponent Invalid with CraftingId:[%s]"), *CraftingId.ToString())
			return;
	}


	for (const auto Viewer : Viewers)
	{
		Viewer.Key->Client_SetItemWaiting(Viewer.Value, InventorySlot, Info);
	}
}

void UInventoryManagerComponent::RemoveViewersWaitingSlot(FGuid CraftingId, TMap<class UInventoryManagerComponent*, FName> Viewers, int32 InventorySlot)
{
	if (!GetInvSubsystem()->ContainsCrafting(CraftingId))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::SetViewersWaitingSlot] CraftingComponent not found with CraftingId:[%s]"), *CraftingId.ToString())
			return;
	}

	if (!IsValid(GetCrafting(CraftingId)))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::SetViewersWaitingSlot] CraftingComponent Invalid with CraftingId:[%s]"), *CraftingId.ToString())
			return;
	}


	for (const auto Viewer : Viewers)
	{
		Viewer.Key->Client_RemoveWaitingSlot( InventorySlot, Viewer.Value);
	}
}

void UInventoryManagerComponent::StartViewersWaitingSlot(FGuid CraftingId, TMap<class UInventoryManagerComponent*, FName> Viewers, int32 InventorySlot, float TimeOfCraft)
{
	if (!GetInvSubsystem()->ContainsCrafting(CraftingId))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::StartViewersWaitingSlot] CraftingComponent not found with CraftingId:[%s]"), *CraftingId.ToString())
			return;
	}

	if (!IsValid(GetCrafting(CraftingId)))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::StartViewersWaitingSlot] CraftingComponent Invalid with CraftingId:[%s]"), *CraftingId.ToString())
			return;
	}


	for (const auto Viewer : Viewers)
	{
		Viewer.Key->CLient_Start_Crafting(Viewer.Value, InventorySlot, TimeOfCraft);
	}
}

void UInventoryManagerComponent::UpdateProgressViewersWaitingSlot(FGuid CraftingId, TMap<class UInventoryManagerComponent*, FName> Viewers, int32 InventorySlot, float Current)
{
	if (!GetInvSubsystem()->ContainsCrafting(CraftingId))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::UpdateProgressViewersWaitingSlot] CraftingComponent not found with CraftingId:[%s]"), *CraftingId.ToString())
			return;
	}

	if (!IsValid(GetCrafting(CraftingId)))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::UpdateProgressViewersWaitingSlot] CraftingComponent Invalid with CraftingId:[%s]"), *CraftingId.ToString())
			return;
	}


	for (const auto Viewer : Viewers)
	{
		Viewer.Key->Client_Update_Crafting(Viewer.Value, InventorySlot, Current);
	}
}

void UInventoryManagerComponent::EndViewersWaitingSlot(FGuid CraftingId, TMap<class UInventoryManagerComponent*, FName> Viewers, int32 InventorySlot)
{
	if (!GetInvSubsystem()->ContainsCrafting(CraftingId))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::EndViewersWaitingSlot] CraftingComponent not found with CraftingId:[%s]"), *CraftingId.ToString())
			return;
	}

	if (!IsValid(GetCrafting(CraftingId)))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::EndViewersWaitingSlot] CraftingComponent Invalid with CraftingId:[%s]"), *CraftingId.ToString())
			return;
	}


	for (const auto Viewer : Viewers)
	{
		Viewer.Key->Client_End_Crafting(Viewer.Value, InventorySlot);
	}
}


// ------------------------ USER INTERFACE ------------------------


	// ---------------------- INVENTORY USER INTERFACE ---------------------- 

void UInventoryManagerComponent::InitializeInventoryPanelUI(FGuid InventoryId,FName GridKey, bool UsingSizeItem, int32 SlotPerRow, TSubclassOf<UInventorySlotUserWidget> OverrideInventoryWidgetSlot, const TArray<FItemDataInfoCompressed>& DefaultValue)
{
	if (!GetInvSubsystem()->ContainsGrid(GridKey)) 
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::InitializeInventoryPanelUI] Could not find a grid with this GridKey [%s]"), *GridKey.ToString());
		return; 
	}

	UInventoryUniformGridPanel* InventoryGridPanel = GetGrid(GridKey);

	UE_LOG(LogInventory, Display, TEXT("[UInventoryManagerComponent::InitializeInventoryPanelUI] Start Load Grid"));


	if (IsValid(GetInventory(InventoryId)))
	{
		if (GetOwnerRole() > ROLE_SimulatedProxy)
		{
			
		}

		//Create slot on grid
		Client_LoadInventory(InventoryId, GridKey, UsingSizeItem, SlotPerRow, OverrideInventoryWidgetSlot, DefaultValue);

		UpdateAllCurrency();
		UE_LOG(LogInventory, Display, TEXT("[UInventoryManagerComponent::InitializeInventoryPanelUI] End Load Grid"));
	}
	else
	{
		UE_LOG(LogInventory, Error, TEXT("[InitializeInventoryPanelUI] NEW UI CAN'T Auto Load Inventory PlayerInventory Invalid"));
	}
}




void UInventoryManagerComponent::CreateInventorySlots(FGuid InventoryId, UInventoryUniformGridPanel* Grid, bool UsingSizeItem, TSubclassOf<UInventorySlotUserWidget> OverrideInventoryWidgetSlot, int32 InventorySizeTotal, int32 InventorySlotPerRow, TArray<FItemDataInfoCompressed> DefaultValue, bool Increment)
{

	if (!IsValid(Grid))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::CreateInventorySlots] InventoryUniformGridPanel Invalid"));
	}

	if (InventorySizeTotal > 0)
	{
		int32 LocalSlotNumber = Increment ? Grid->GetInventorySlots().Num() + InventorySizeTotal : 0;
		int32 LocalLoopCount = Increment ? Grid->GetInventorySlots().Num() : 0;
		const int32 Ceil = (FMath::CeilToInt(static_cast<float>(Increment ? Grid->GetInventorySlots().Num() + InventorySizeTotal : InventorySizeTotal) / static_cast<float>(InventorySlotPerRow)) - 0);
		const int32 Loop = FMath::Max(Ceil, 0);

		int32 Row = 0;
		int32 Column = 0;


		Grid->SetMaxPerRow(InventorySlotPerRow);
		
		//If true get last Row and Colum on this grid
		if (Increment)
		{
			const int32 LastIndexSlotPanel = (Grid->GetChildrenCount() - 1);
			UPanelSlot* SlotACast = Grid->GetChildAt(LastIndexSlotPanel)->Slot;
			if (const UUniformGridSlot* LastSlot = Cast<UUniformGridSlot>(SlotACast)) //Slot is Valid get last row and column
			{
				Row = LastSlot->GetRow();
				Column = LastSlot->GetColumn() + 1;
			}

		}

		for (Row; Row < Loop; Row++)
		{
			for (Column; Column < InventorySlotPerRow; Column++)
			{
				//UE_LOG(LogInventory, Warning, TEXT("AddSlot Row:[%d] Column:[%d]"), Row,Column);
				FItemDataInfoCompressed Default = DefaultValue.IsValidIndex(LocalLoopCount) ? DefaultValue[LocalLoopCount] : FItemDataInfoCompressed();
				AddInventorySlot(InventoryId,Grid, UsingSizeItem, OverrideInventoryWidgetSlot, Row, Column, LocalLoopCount, Default);
				LocalSlotNumber++;
				LocalLoopCount++;

				if (LocalLoopCount == InventorySizeTotal)
				{
					UE_LOG(LogInventory, Display, TEXT("[UInventoryManagerComponent::CreateInventorySlots] UI  Total Slot Created : %d"), Grid->GetInventorySlots().Num());
					break;
				}
			}
			Column = 0; //Reset Column for the next row
		}		
	}
	else
	{
		UE_LOG(LogInventory, Warning, TEXT("InventorySize <= 0 Can't create Slot"), Grid->GetInventorySlots().Num());
	}
}

void UInventoryManagerComponent::AddInventorySlot(FGuid InventoryId, UInventoryUniformGridPanel* Grid, bool UsingSizeItem, TSubclassOf<UInventorySlotUserWidget> OverrideInventoryWidgetSlot, int32 Row, int32 Column, int32 Slot, FItemDataInfoCompressed DefaultValue)
{
	if (!IsValid(Grid))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::AddInventorySlot] Grid Invalid"));
		return;
	}

	// Initialize local slot

	if ((Grid->GetInventorySlots().Num()) < Slot) // Check if has already slots 
	{
		if (Grid->GetInventorySlots().IsValidIndex(Slot))
		{
			//Client_ClearInventorySlotItem(Slot);
			UInventorySlotUserWidget* HaveSlot = Grid->GetInventorySlot(Slot);
			if (IsValid(HaveSlot))
			{
				HaveSlot->SetItem(InventoryId, UChestoryFunctionLibrary::MakeItemDataFromTransfer(this, DefaultValue));
				UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::AddInventorySlot] UI  Slot is heare juste clear data"));
				return;
			}
		}
	}

	// else create and add new slot  || CustomClassSlot != NULL
	if (OverrideInventoryWidgetSlot != NULL)
	{
		UInventorySlotUserWidget* NewSlot = CreateWidget<UInventorySlotUserWidget>(GetWorld(), OverrideInventoryWidgetSlot);
		if (NewSlot)
		{
			NewSlot->InventorySlot = Slot;
			NewSlot->InventoryManager = this;
			NewSlot->OwnerInventoryUseSizeItem = UsingSizeItem;
			NewSlot->OwnerInventoryGrid = Grid;
			NewSlot->Type = ESlotType::EInventory;
			NewSlot->SetItem(InventoryId, UChestoryFunctionLibrary::MakeItemDataFromTransfer(this, DefaultValue));

			//UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::AddInventorySlot] New Widget Slot Created on Grid: [%s] Row:[%d] Column:[%d]"), *Grid->GridKey.ToString(), Row, Column);

			// Add slot on Panel Inventory
			Grid->AddInventorySlot(NewSlot, Row, Column);
		}
		else
		{
			UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::AddInventorySlot] Create new InventorySlotUserWidget Failed"));
		}

	}
	else
	{
		UE_LOG(LogInventory, Error, TEXT("[UInventoryManagerComponent::AddInventorySlot] No InventorySlotUserWidget Class indicated in CustomClassSlot Variable in InventoryManager"));
	}

	
}


void UInventoryManagerComponent::Client_OpenInventory_Implementation(FGuid InventoryId, FName GridKey, bool UsingSizeItem, int32 SlotPerRow, TSubclassOf<UInventorySlotUserWidget> OverrideInventoryWidgetSlot, const TArray<FItemDataInfoCompressed>& DefaultValue)
{

//	OpenInventoryWindow();
	if (!GetInvSubsystem()->ContainsInventory(InventoryId))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Client_OpenInventory] Inventory not found with InventoryId:[%s]"), *InventoryId.ToString());
		return;
	}

	if (!GetInvSubsystem()->ContainsGrid(GridKey))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Client_OpenInventory] Grid not found with GridKey:[%s]"), *GridKey.ToString());
		return;
	}

	//Construct Grid with Data
	InitializeInventoryPanelUI(InventoryId, GridKey,UsingSizeItem, SlotPerRow, OverrideInventoryWidgetSlot, DefaultValue);


	if(!IsValid(GetGrid(GridKey)->WidgetParentVisibility))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Client_OpenInventory] No WidgetParentVisibility in InventoryUniformGridPanel"));
	}
	

	//Setup input in Inventory
	if (bEnableAutoGestionInputMapping)
	{
		SetupInventoryManagerInput();
	}

	GetGrid(GridKey)->WidgetParentVisibility->SetVisibility(ESlateVisibility::Visible);

	//Trigger Client Side
	//GetInventory(InventoryId)->OnContainerIsOpen.Broadcast(GridKey);


}

void UInventoryManagerComponent::Client_CloseInventory_Implementation(FGuid InventoryId, FName GridKey, bool RemoveInput)
{
	//CloseInventoryWindow();
	if (!GetInvSubsystem()->ContainsInventory(InventoryId))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Client_CloseInventory] Inventory not found with InventoryId:[%s]"), *InventoryId.ToString());
		return;
	}

	if (!GetInvSubsystem()->ContainsGrid(GridKey))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Client_CloseInventory] Grid not found with GridKey:[%s]"), *InventoryId.ToString());
		return;
	}

	//Trigger on Client side
	//GetInventory(InventoryId)->OnContainerIsClose.Broadcast(GridKey);

	if (!IsValid(GetGrid(GridKey)->WidgetParentVisibility))
	{
		UE_LOG(LogInventory, Display, TEXT("[UInventoryManagerComponent::Client_CloseInventory] No WidgetParentVisibility in InventoryUniformGridPanel"));
	}

	GetGrid(GridKey)->WidgetParentVisibility->SetVisibility(ESlateVisibility::Hidden);
	
	Client_ClearInventory(GridKey);

	GetGrid(GridKey)->CloseContainer();

	//Clear input
	if (RemoveInput && bEnableAutoGestionInputMapping)
	{
		RemoveInventoryManagerInput();
	}
}

void UInventoryManagerComponent::Client_LoadInventory_Implementation(FGuid InventoryId, FName GridKey, bool UsingSizeItem, int32 SlotPerRow, TSubclassOf<UInventorySlotUserWidget> OverrideInventoryWidgetSlot, const TArray<FItemDataInfoCompressed>& DefaultValue)
{

	 if (!GetInvSubsystem()->ContainsInventory(InventoryId))
	 {
		 UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Client_LoadInventory] Could not find an inventory with this InventoryId [%s]"), *InventoryId.ToString());
		 return;
	 }

	 if (!GetInvSubsystem()->ContainsGrid(GridKey))
	 {
		 UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Client_LoadInventory] Could not find a grid with this GridKey [%s]"), *GridKey.ToString());
		 return;
	 }

	 if (UInventoryUniformGridPanel* Grid = GetGrid(GridKey); IsValid(Grid))
	 {
		 Grid->SetInventoryId(InventoryId);
		 Grid->OwnerInventoryUseSizeItem = UsingSizeItem;

		 //Take default size or lenght Data (increment or decrement inventory size support from save)
		 int32 Size = DefaultValue.IsEmpty() ? GetInventory(InventoryId)->ContainerSize : DefaultValue.Num();
		 //int32 PerRow = GetInventory(InventoryId)->ContainerSlotsPerRow;
		 CreateInventorySlots(InventoryId, Grid, UsingSizeItem, OverrideInventoryWidgetSlot, Size, SlotPerRow, DefaultValue);

		 Grid->OnOpenContainer.Broadcast(GetInventory(InventoryId)->GetContainerSetting());
	 }
	 else
	 {
		 UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Client_LoadInventory] UInventoryUniformGridPanel invalid with GridKey [%s]"), *GridKey.ToString());
		 return;
	 }

	

}

void UInventoryManagerComponent::Client_ClearInventory_Implementation(FName GridKey)
{
	if (!GetInvSubsystem()->ContainsGrid(GridKey))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Client_ClearInventory] Could not find a grid with this GridKey [%s]"), *GridKey.ToString());
		return;
	}

	//reset Grid
	GetGrid(GridKey)->ClearPanel();
}



void UInventoryManagerComponent::Client_SetInventorySlotItem_Implementation(FGuid InventoryId, FName GridKey, int32 InventorySlot,  FItemDataInfoCompressed ItemInformationCompressed)
{ 

	if (UInventorySlotUserWidget* Slot = GetAdvancedSlot(InventoryId, GridKey, InventorySlot, FString("Client_SetInventorySlotItem")))
	{
		const FItemDataInfo DataInfoUncompressed = UChestoryFunctionLibrary::MakeItemDataFromTransfer(this, ItemInformationCompressed);
		Slot->SetItem(InventoryId, DataInfoUncompressed.bIsVisible ? DataInfoUncompressed : FItemDataInfo());
	}
}

void UInventoryManagerComponent::Client_ClearInventorySlotItem_Implementation(FGuid InventoryId, FName GridKey, const int32 InventorySlot, bool CloseInventory/* = false*/)
{
	if (UInventorySlotUserWidget* Slot = GetAdvancedSlot(CloseInventory ? FGuid()  : InventoryId, GridKey, InventorySlot, FString("Client_ClearInventorySlotItem")))
	{
		Slot->ClearItem(InventoryId);
	}
}


void UInventoryManagerComponent::Client_FilledInventorySlots_Implementation(FGuid InventoryId, FName GridKey, const TArray<int32>& InventorySlots)
{
	if (UInventoryUniformGridPanel* Grid = GetGrid(GridKey))
	{
		Grid->FilledSlots(InventorySlots);
	}
}

void UInventoryManagerComponent::Client_UnfilledInventorySlots_Implementation(FGuid InventoryId, FName GridKey, const TArray<int32>& InventorySlots)
{
	if (UInventoryUniformGridPanel* Grid = GetGrid(GridKey))
	{
		Grid->UnfilledSlots(InventorySlots);
	}
}

void UInventoryManagerComponent::Client_CancelOverSlots_Implementation(FGuid InventoryId, FName GridKey, const TArray<int32>& InventorySlots)
{
	if (UInventoryUniformGridPanel* Grid = GetGrid(GridKey))
	{
		Grid->CancelOverSlots(InventorySlots);
	}
}


void UInventoryManagerComponent::Client_SetInventoryRepairSlotItem_Implementation(FGuid InventoryId, FName GridKey, int32 InventorySlot,FItemDataInfoCraftCompressed ItemInformationCraftCompressed)
{
	if (UInventorySlotUserWidget* Slot = GetAdvancedSlot(InventoryId, GridKey, InventorySlot, FString("Client_SetInventoryRepairSlotItem")))
	{
		const FItemDataInfoCraft DataInfoUncompressed = UChestoryFunctionLibrary::MakeItemCraftFromTransfer(this, ItemInformationCraftCompressed);
		Slot->SetRepair(DataInfoUncompressed.ItemInformation.bIsVisible ? DataInfoUncompressed : FItemDataInfoCraft());
	}
}

void UInventoryManagerComponent::Client_SetInventoryBlueprintSlotItem_Implementation(FGuid InventoryId, FName GridKey, int32 InventorySlot,	FItemDataInfoCraftCompressed ItemInformationCraftCompressed)
{

	if (UInventorySlotUserWidget* Slot = GetAdvancedSlot(InventoryId, GridKey, InventorySlot, FString("Client_SetInventoryBlueprintSlotItem")))
	{
		const FItemDataInfoCraft DataInfoUncompressed = UChestoryFunctionLibrary::MakeItemCraftFromTransfer(this,ItemInformationCraftCompressed);
		Slot->SetBlueprint(DataInfoUncompressed.ItemInformation.bIsVisible ? DataInfoUncompressed : FItemDataInfoCraft());
	}
}

void UInventoryManagerComponent::Client_IncreaseInventorySlots_Implementation(FGuid InventoryId, FName GridKey, bool UsingSizeItem, TSubclassOf<UInventorySlotUserWidget> OverrideInventoryWidgetSlot, int32 Amount)
{
	//IncreaseInventorySlots(Amount);

	if (!GetInvSubsystem()->ContainsInventory(InventoryId))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Client_IncreaseInventorySlots] Could not find an inventory with this InventoryId [%s]"), *InventoryId.ToString());
		return;
	}

	if (!GetInvSubsystem()->ContainsGrid(GridKey))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Client_IncreaseInventorySlots] Could not find a grid with this GridKey [%s]"), *GridKey.ToString());
		return;
	}

	if (!IsValid(GetGrid(GridKey)))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Client_IncreaseInventorySlots] UInventoryUniformGridPanel invalid with GridKey [%s]"), *GridKey.ToString());
		return;
	}

	GetGrid(GridKey)->SetInventoryId(InventoryId);

	//Take default size or lenght Data (increment or decrement inventory size support from save)
	int32 PerRow = GetInventory(InventoryId)->ContainerSlotsPerRow;
	UE_LOG(LogInventory, Display, TEXT("[UInventoryManagerComponent::Client_IncreaseInventorySlots] Add AmountSlot:[%d] on GridKey [%s]"), Amount, *GridKey.ToString());

	CreateInventorySlots(InventoryId, GetGrid(GridKey), UsingSizeItem, OverrideInventoryWidgetSlot, Amount, PerRow, TArray<FItemDataInfoCompressed>(),true);

}

void UInventoryManagerComponent::Client_DecreaseInventorySlots_Implementation(FGuid InventoryId, FName GridKey, int32 Amount)
{
	//DecreaseInventorySlots(Amount);
	
	if (!GetInvSubsystem()->ContainsInventory(InventoryId))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Client_IncreaseInventorySlots] Could not find an inventory with this InventoryId [%s]"), *InventoryId.ToString());
		return;
	}

	if (!GetInvSubsystem()->ContainsGrid(GridKey))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Client_IncreaseInventorySlots] Could not find a grid with this GridKey [%s]"), *GridKey.ToString());
		return;
	}

	if (!IsValid(GetGrid(GridKey)))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Client_IncreaseInventorySlots] UInventoryUniformGridPanel invalid with GridKey [%s]"), *GridKey.ToString());
		return;
	}

	GetGrid(GridKey)->SetInventoryId(InventoryId);


	GetGrid(GridKey)->DecreaseInventorySlots(Amount);
}


void UInventoryManagerComponent::Client_OnItemUsed_Implementation(FGuid OnInventoryId, FName GridKey, int32 OnSlot, EUsedInputTrigger InputTrigger)
{
	if (!OnInventoryId.IsValid())
	{
		return;
	}

	if (UInventorySlotUserWidget* Slot = GetAdvancedSlot(OnInventoryId, GridKey, OnSlot, FString("Client_OnItemUsed")))
	{
		Slot->OnItemUsed(InputTrigger);
	}
}


void UInventoryManagerComponent::Client_SetDurabilitySlotItem_Implementation(FGuid InventoryId, FName GridKey, int32 InventorySlot, float Durability)
{

	if (UInventorySlotUserWidget* Slot = GetAdvancedSlot(InventoryId, GridKey, InventorySlot, FString("Client_SetDurabilitySlotItem")))
	{

		Slot->SetDurability(Durability);
	}
}

void UInventoryManagerComponent::Client_SetDecaySlotItem_Implementation(FGuid InventoryId, FName GridKey, int32 InventorySlot,
	float Decay, float OnDecayFactor)
{
	if (UInventorySlotUserWidget* Slot = GetAdvancedSlot(InventoryId, GridKey, InventorySlot, FString("Client_SetDecaySlotItem")))
	{
		if (Slot->IsValidLowLevel())
		{
			Slot->SetDecay(InventoryId, Decay, OnDecayFactor);
		}
	}
}

void UInventoryManagerComponent::Client_SetAttributesSlotItem_Implementation(FGuid InventoryId, FName GridKey, int32 InventorySlot, FGameplayTagAttributeContainer Attributes)
{

	if (UInventorySlotUserWidget* Slot = GetAdvancedSlot(InventoryId, GridKey, InventorySlot, FString("Client_SetDecaySlotItem")))
	{
		Slot->SetAttributes(Attributes);
	}
}

void UInventoryManagerComponent::Client_SetGameplayTagSlotItem_Implementation(FGuid InventoryId, FName GridKey, int32 InventorySlot, FGameplayTag Tag, bool IsAdd)
{
	if (UInventorySlotUserWidget* Slot = GetAdvancedSlot(InventoryId, GridKey, InventorySlot, FString("Client_SetGameplayTagSlotItem")))
	{
		Slot->SetGameplayTag(Tag, IsAdd);
	}
}

void UInventoryManagerComponent::Client_SetJsonParameterSlotItem_Implementation(FGuid InventoryId, FName GridKey, int32 InventorySlot,const  FString& JsonParameter)
{
	if (UInventorySlotUserWidget* Slot = GetAdvancedSlot(InventoryId, GridKey, InventorySlot, FString("Client_SetJsonParameterSlotItem")))
	{
		Slot->SetJsonParameter(JsonParameter);
	}
}


void UInventoryManagerComponent::Client_SetSocketSlotItem_Implementation(FGuid InventoryId, FName GridKey, int32 InventorySlot, int32 NewSocketSlot, FSocketItem NewSocket)
{

	if (UInventorySlotUserWidget* Slot = GetAdvancedSlot(InventoryId, GridKey, InventorySlot, FString("Client_SetDecaySlotItem")))
	{
		Slot->SetSocketInformation(InventoryId,NewSocket, NewSocketSlot);
	}
}


// ---------------------- LOCK USER INTERFACE ----------------------

void UInventoryManagerComponent::Client_LockSlot_Implementation(FGuid InventoryId, FName GridKey, int32 Index)
{
	if (UInventorySlotUserWidget* Slot = GetAdvancedSlot(InventoryId, GridKey, Index, FString("Client_LockSlot")))
	{
		Slot->OnDisableSlot();
	}
}

void UInventoryManagerComponent::Client_UnlockSlot_Implementation(FGuid InventoryId, FName GridKey, int32 Index)
{
	if (UInventorySlotUserWidget* Slot = GetAdvancedSlot(InventoryId, GridKey, Index, FString("Client_UnlockSlot")))
	{
		Slot->OnEnableSlot();
	}
}


// ---------------------- EQUIPEMENT USER INTERFACE ---------------------- 

void UInventoryManagerComponent::Client_LoadEquipement_Implementation(FGuid InventoryId, FName Context, const TArray<FItemDataInfoCompressed>& DefaultValue)
{

	if (!InventoryId.IsValid())
	{
		return;
	}

	if (!GetInvSubsystem()->ContainsInventory(InventoryId))
	{
		return;
	}


	//if (GetInventory(InventoryId)->SlotsIndexKey.IsEmpty())
	//{
	//	UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Client_LoadEquipement] SlotsIndexKey is Empty On InventoryId:[%s] "), *InventoryId.ToString());
	//	return;
	//}


	//Add Cache Context
	MatchContextEquipments.Add(InventoryId, Context);
	UE_LOG(LogInventory,Verbose,TEXT("[UInventoryManagerComponent::Client_LoadEquipement] Add Context:[%s] On InventoryId:[%s] "),*Context.ToString(),*InventoryId.ToString())

	for (int32 i = 0; i < DefaultValue.Num(); i++)
	{
		Client_SetEquipementSlotItem(InventoryId, i, DefaultValue[i]);
	}

	//for (auto Equipment : GetInventory(InventoryId)->SlotsIndexKey)
	//{
	//	int32 Index = GetInventory(InventoryId)->GetIndexAtKey(Equipment);
	//	Client_SetEquipementSlotItem(InventoryId, Index, DefaultValue[Index]);

	//	//GetInventory(InventoryId)->SlotsIndexKey.Find(Equipment, Index);
	//	if (Index)
	//	{
	//	}
	//}

}

void UInventoryManagerComponent::Client_ClearEquipement_Implementation(FGuid InventoryId)
{
	if (GetWorld()->GetNetMode() == ENetMode::NM_DedicatedServer)
	{
		return;
	}

	//Warning is not possible normaly
	if (MatchContextEquipments.IsEmpty())
	{
		return;
	}

	if (!IsValid(GetInvSubsystem()->GetInventory(InventoryId)))
	{
		return;
	}

	for (auto Equipment : GetInvSubsystem()->GetInventory(InventoryId)->SlotsIndexKey)
	{
		
		//GetInvSubsystem()->GetInventory(InventoryId)->SlotsIndexKey.Find(Equipment, Index);
		int32 Index = GetInvSubsystem()->GetInventory(InventoryId)->GetIndexAtKey(Equipment);
		Client_ClearEquipementSlotItem(InventoryId, Index,true);
	}
	
	FName Context = MatchContextEquipments.Contains(InventoryId) ? *MatchContextEquipments.Find(InventoryId) : FName();
	UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::Client_ClearEquipement] Remove Context:[%s] On InventoryId:[%s] "), *Context.ToString(), *InventoryId.ToString())

	//Remove Context Cache
	MatchContextEquipments.Remove(InventoryId);
}

void UInventoryManagerComponent::Client_SetEquipementSlotItem_Implementation(FGuid InventoryId,int32 Index, FItemDataInfoCompressed ItemToTransfer)
{
	if (!GetInvSubsystem()->ContainsInventory(InventoryId))
	{
		return;
	}



	if (MatchContextEquipments.IsEmpty())
	{
		UE_LOG(LogInventory, Warning, TEXT("UInventoryManagerComponent::Client_ClearEquipementSlotItem] Context Equipments is Empty"));
		return;
	}

	if (!MatchContextEquipments.Contains(InventoryId))
	{
		UE_LOG(LogInventory, Warning, TEXT("UInventoryManagerComponent::Client_SetEquipementSlotItem] no Context found InventoryId:[%s]"),*InventoryId.ToString());
		return;
	}

	FName SlotKey = GetInvSubsystem()->GetInventory(InventoryId)->GetKeyAtIndex(Index);
	if (SlotKey.IsNone()) {
		UE_LOG(LogInventory, Warning, TEXT("UInventoryManagerComponent::Client_SetEquipementSlotItem] SlotKey is None [%s]"), *SlotKey.ToString());
		return;
	}

	if (FName Context = *MatchContextEquipments.Find(InventoryId); !Context.IsNone())
	{
		if (!GetInvSubsystem()->ContainsEquipmentSlot(Context, SlotKey))
		{
			UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Client_SetEquipementSlotItem] not found Equipement slot with Context:[%s] SlotKey:[%s]"), *Context.ToString(), *SlotKey.ToString());
			return;
		}

		if (!IsValid(GetEquipmentSlot(Context, SlotKey)))
		{
			UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Client_SetEquipementSlotItem] Widget EquipementSlot with Context:[%s] SlotKey:[%s] Invalid"), *Context.ToString(), *SlotKey.ToString());
			return;
		}

		GetEquipmentSlot(Context, SlotKey)->InventoryManager = this;
		GetEquipmentSlot(Context, SlotKey)->InventorySlot = Index;
		GetEquipmentSlot(Context, SlotKey)->SetItem(InventoryId, UChestoryFunctionLibrary::MakeItemDataFromTransfer(this,ItemToTransfer));
	}
}

void UInventoryManagerComponent::Client_ClearEquipementSlotItem_Implementation(FGuid InventoryId,int32 Index, bool CloseInventory/* = false*/)
{
	if (!GetInvSubsystem()->ContainsInventory(InventoryId))
	{
		return;
	}


	if (MatchContextEquipments.IsEmpty())
	{
		UE_LOG(LogInventory, Warning, TEXT("UInventoryManagerComponent::Client_ClearEquipementSlotItem] Context Equipments is Empty"));
		return;
	}

	if (!MatchContextEquipments.Contains(InventoryId))
	{
		UE_LOG(LogInventory, Warning, TEXT("UInventoryManagerComponent::Client_ClearEquipementSlotItem] no Context found InventoryId:[%s]"), *InventoryId.ToString());
		return;
	}

	FName SlotKey = GetInvSubsystem()->GetInventory(InventoryId)->GetKeyAtIndex(Index);


	if (SlotKey.IsNone()) {
		UE_LOG(LogInventory, Warning, TEXT("UInventoryManagerComponent::Client_ClearEquipementSlotItem] SlotKey is None [%s]"), *SlotKey.ToString());
		return;
	}

	if (FName Context = *MatchContextEquipments.Find(InventoryId); !Context.IsNone())
	{
		if (!GetInvSubsystem()->ContainsEquipmentSlot(Context, SlotKey))
		{
			UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Client_ClearEquipementSlotItem] not found Equipement slot with Context:[%s] SlotKey:[%s]"), *Context.ToString(), *SlotKey.ToString());
			return;
		}

		if (!IsValid(GetEquipmentSlot(Context, SlotKey)))
		{
			UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Client_ClearEquipementSlotItem] Widget EquipementSlot with Context:[%s] SlotKey:[%s] Invalid"), *Context.ToString(), *SlotKey.ToString());
			return;
		}

		GetEquipmentSlot(Context, SlotKey)->ClearItem(CloseInventory ? FGuid()  : InventoryId);

	}
}

void UInventoryManagerComponent::Client_EnableEquipementSlot_Implementation(FGuid InventoryId, FGameplayTag Tag)
{
	if (!MatchContextEquipments.Contains(InventoryId))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Client_EnableEquipementSlot] No Context found with InventoryId:[%s]"), *InventoryId.ToString())
			return;
	}

	FName Context = *MatchContextEquipments.Find(InventoryId);

	TArray<FName> Keys;
	for (auto& Equipment : Equipments)
	{
		FGameplayTagContainer Container;
		Container.AddTag(Tag);
		if (Container.MatchesQuery(Equipment.Value.Restriction))
		{
			Keys.Add(Equipment.Key);
		}
	}

	for (auto ESlot : UChestoryFunctionLibrary::GetEquipmentSlots(this, Context))
	{
		if (Keys.Contains(ESlot->SlotKey))
		{
			ESlot->OnEnableSlot();
		}
	}
}

void UInventoryManagerComponent::Client_DisableEquipementSlot_Implementation(FGuid InventoryId, FGameplayTag Tag)
{
	if (!MatchContextEquipments.Contains(InventoryId))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Client_DisableEquipementSlot] No Context found with InventoryId:[%s]"), *InventoryId.ToString())
			return;
	}

	FName Context = *MatchContextEquipments.Find(InventoryId);

	TArray<FName> Keys;
	for (auto& Equipment : Equipments)
	{
		FGameplayTagContainer Container;
		Container.AddTag(Tag);
		if (Container.MatchesQuery(Equipment.Value.Restriction))
		{
			Keys.Add(Equipment.Key);
		}
	}

	for (auto ESlot : UChestoryFunctionLibrary::GetEquipmentSlots(this, Context))
	{
		if (Keys.Contains(ESlot->SlotKey))
		{
			ESlot->OnDisableSlot();
		}
	}
}






// ---------------------- HOTBAR USER INTERFACE ---------------------- 


void UInventoryManagerComponent::Client_LoadHotbar_Implementation(FGuid InventoryId, const TArray<FItemDataInfoCompressed>& DefaultValue)
{
	if (!InventoryId.IsValid())
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Client_LoadHotbar] InventoryId Invalid"));
		return;
	}

	if (!GetInvSubsystem()->ContainsInventory(InventoryId))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Client_LoadHotbar] Inventory not fournd with InventoryId:[%s]"), *InventoryId.ToString());
		return;
	}


	if (!IsValid(GetInventory(InventoryId)))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Client_LoadHotbar] InventoryComponent Invalid"));
		return;
	}

	for (auto Hot : GetInventory(InventoryId)->SlotsIndexKey)
	{
		int32 Index;
		GetInventory(InventoryId)->SlotsIndexKey.Find(Hot, Index);
		if (DefaultValue[Index].IsValid())
		{
			Client_SetHotbarSlotItem(InventoryId, Index, DefaultValue[Index]);

		}
		else
		{
			Client_ClearHotBarSlotItem(InventoryId, Index);
		}
	}
}

void UInventoryManagerComponent::Client_ClearHotbar_Implementation(FGuid InventoryId)
{
	if (GetWorld()->GetNetMode() == ENetMode::NM_DedicatedServer)
	{
		return;
	}

	if (!InventoryId.IsValid())
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Client_ClearHotbar] InventoryId Invalid"));
		return;
	}

	if (!IsValid(GetInventory(InventoryId)))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Client_ClearHotbar] InventoryComponent Invalid"));
		return;
	}

	for (auto Hot : GetInventory(InventoryId)->SlotsIndexKey)
	{
		int32 Index;
		GetInventory(InventoryId)->SlotsIndexKey.Find(Hot, Index);
		Client_ClearHotBarSlotItem(InventoryId, Index);
	}
}



void UInventoryManagerComponent::Client_SetHotbarSlotItem_Implementation(FGuid InventoryId, int32 InventorySlot, FItemDataInfoCompressed ItemToTransfer)
{

	if (GetWorld()->GetNetMode() == ENetMode::NM_DedicatedServer)
	{
		return;
	}

	if (!GetInvSubsystem()->HasHotbars())
	{
		Client_PendingHotbar();
		UE_LOG(LogInventory, VeryVerbose, TEXT("[UInventoryManagerComponent::Client_SetHotbarSlotItem] no data Hotbars in Subsystem pending data"));
		return;
	}

	if (!InventoryId.IsValid())
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Client_SetHotbarSlotItem] InventoryId Invalid"));
		return;
	}


	if (!GetInvSubsystem()->ContainsInventory(InventoryId))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Client_SetHotbarSlotItem] Inventory not fournd with InventoryId:[%s]"), *InventoryId.ToString());
		return;
	}

	FName SlotKey = GetInvSubsystem()->GetInventory(InventoryId)->GetKeyAtIndex(InventorySlot);
	if (SlotKey.IsNone()) {
		UE_LOG(LogInventory, Warning, TEXT("UInventoryManagerComponent::Client_SetHotbarSlotItem] SlotKey is None [%s]"), *SlotKey.ToString());
		return;
	}

	if (!GetInvSubsystem()->ContainsHotbarSlot(SlotKey))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Client_SetHotbarSlotItem] Hotbar Widget Slot not found with SlotKey:[%s]"), *SlotKey.ToString());
		return;
	}

	if (!IsValid(GetHotbarSlot(SlotKey)))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Client_SetHotbarSlotItem] Hotbar Widget Slot Invalid"));
		return;
	}

	//
	//if (!GetHotbarSlot(SlotKey)->IsInViewport())
	//{
	//	UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Client_SetHotbarSlotItem] Hotbar Widget Slot IsInViewport not"));
	//	return;
	//}

	GetHotbarSlot(SlotKey)->InventoryManager = this;
	GetHotbarSlot(SlotKey)->InventorySlot = InventorySlot;
	FItemDataInfo Info = UChestoryFunctionLibrary::MakeItemDataFromTransfer(this, ItemToTransfer);
	GetHotbarSlot(SlotKey)->SetItem(InventoryId, Info);
}

void UInventoryManagerComponent::Client_ClearHotBarSlotItem_Implementation(FGuid InventoryId, int32 HotbarSlot)
{
	if (GetWorld()->GetNetMode() == ENetMode::NM_DedicatedServer)
	{
		return;
	}

	if (!GetInvSubsystem()->HasHotbars())
	{
		Client_PendingHotbar();
		UE_LOG(LogInventory, VeryVerbose, TEXT("[UInventoryManagerComponent::Client_ClearHotBarSlotItem] no data Hotbars in Subsystem pending data"));
		return;
	}

	if (!InventoryId.IsValid())
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Client_ClearHotBarSlotItem] InventoryId Invalid"));
		return;
	}


	if (!GetInvSubsystem()->ContainsInventory(InventoryId))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Client_ClearHotBarSlotItem] Inventory not fournd with InventoryId:[%s]"), *InventoryId.ToString());
		return;
	}

	FName SlotKey = GetInvSubsystem()->GetInventory(InventoryId)->GetKeyAtIndex(HotbarSlot);


	if (SlotKey.IsNone()) {
		UE_LOG(LogInventory, Warning, TEXT("UInventoryManagerComponent::Client_ClearHotBarSlotItem] SlotKey is None [%s]"), *SlotKey.ToString());
		return;
	}

	UInventorySlotUserWidget* SlotRef = GetHotbarSlot(SlotKey);

	if (!IsValid(SlotRef) || !SlotRef->IsValidLowLevel())
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Client_ClearHotBarSlotItem] Hotbar Widget Slot Invalid"));
		return;
	}

	SlotRef->InventoryManager = this;
	SlotRef->InventorySlot = HotbarSlot;
	SlotRef->ClearItem(InventoryId);
}


void UInventoryManagerComponent::Client_MoveHotbarSlotItem_Implementation(int32 FromSlot, int32 ToHotbarSlot, bool FromInventory, bool FromHotbar, FItemDataInfoCompressed FromItem)
{
	if (FromInventory)
	{
		
		Client_SetHotbarSlotItem(HotbarInventory->GetId(), ToHotbarSlot, FromItem);
	}
	else if (FromHotbar) //Swap hotbar item 
	{
		const FItemDataInfo LocalItemHotBar = GetHotBarSlotItem(HotbarInventory->GetKeyAtIndex(FromSlot));
		const FItemDataInfo LocalSwapItemHotBar = GetHotBarSlotItem(HotbarInventory->GetKeyAtIndex(ToHotbarSlot));
		if (LocalSwapItemHotBar.IsValid())
		{
			Client_SetHotbarSlotItem(HotbarInventory->GetId(), ToHotbarSlot, UChestoryFunctionLibrary::MakeItemTransfer(LocalItemHotBar));
			Client_SetHotbarSlotItem(HotbarInventory->GetId(), FromSlot, UChestoryFunctionLibrary::MakeItemTransfer(LocalSwapItemHotBar));
		}
		else
		{
			Client_SetHotbarSlotItem(HotbarInventory->GetId(), ToHotbarSlot, UChestoryFunctionLibrary::MakeItemTransfer(LocalItemHotBar));
			Client_ClearHotBarSlotItem(HotbarInventory->GetId(), FromSlot);
		}
	}
}


FItemDataInfo UInventoryManagerComponent::GetHotBarSlotItem(FName SlotKey)
{

	if (UInventorySlotUserWidget* Slot = GetHotbarSlot(SlotKey))
	{
		return Slot->ItemInformation;
	}

	UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::GetHotBarSlotItem] Hotbar ItemInfo Not found with SlotKey:[%s]"), *SlotKey.ToString());
	return FItemDataInfo();
}

void UInventoryManagerComponent::Client_UseHotBarSlot(FName SlotKey, EUsedInputTrigger InputTrigger)
{
	if (bUseHotbarAsShortcut)
	{
		const FItemDataInfo LocalInfoHotBar = GetHotBarSlotItem(SlotKey);
		if (!LocalInfoHotBar.IsValid())
		{
			UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Client_UseHotBarSlot] UI  Hotbar Item Invalid SlotKey:[%s] ItemId:[%s]."),*SlotKey.ToString(), *LocalInfoHotBar.ID.ToString());
			return;
		}

		UseHotbarAsShortuct(GetFirstPlayerInventoryId(), LocalInfoHotBar.ID, SlotKey,InputTrigger);
	}
	else
	{
		UseItemFromType(HotbarInventory->GetId(), HotbarInventory->GetIndexAtKey(SlotKey),InputTrigger);
	}
}

// ---------------------- CRAFTING USER INTERFACE ---------------------- 
void UInventoryManagerComponent::Client_LoadCrafting_Implementation(FGuid CraftingId, UCraftingComponent* Crafting, const TArray<FItemDataInfoCraftCompressed>& ListItemCraftToLoad, const TArray<FItemDataInfoCraftCompressed>& WaitlistToLoad, FName GridKey)
{

	if (!IsValid(Crafting))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Client_LoadCrafting] CraftingComponent Invalid"));
		return;
	}

	if (GridKey.IsNone())
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Client_LoadCrafting] GridKey is None"));
		return;
	}

	if (!GetInvSubsystem()->ContainsGrid(GridKey))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Client_LoadCrafting] Grid not found with GridKey:[%s]"), *GridKey.ToString());
		return;
	}

	UInventoryUniformGridPanel* InventoryGridPanel = GetGrid(GridKey);
	//Check before continue the load
	if (!IsValid(InventoryGridPanel))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Client_LoadCrafting] UInventoryUniformGridPanel Invalid with GridKey:[%s]"), *GridKey.ToString());
		return;
	}

	Crafting->DefaultGridKey = GridKey;

	//Load Crafting Slot
	if (Crafting->bAutoCreationCraftingSlot)
	{
		CreateCraftingSlot(CraftingId, ListItemCraftToLoad.Num(), Crafting->ClassCraftingSlot, GridKey, ListItemCraftToLoad, Crafting);
	}


	CreateWaitingSlot(CraftingId, Crafting->ClassWaitingSlot, GridKey, WaitlistToLoad);

}

void UInventoryManagerComponent::Client_UnloadCrafting_Implementation(FGuid CraftingId, FName GridKey)
{

	if (!CraftingId.IsValid())
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Client_UnloadCrafting] CraftingId Invalid"));
		return;
	}

	if (GridKey.IsNone())
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Client_UnloadCrafting] GridKey is None"));
		return;
	}

	if (!GetInvSubsystem()->ContainsGrid(GridKey))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Client_UnloadCrafting] Grid not found with GridKey:[%s]"), *GridKey.ToString());
		return;
	}

	UInventoryUniformGridPanel* InventoryGridPanel = GetGrid(GridKey);

	if (IsValid(InventoryGridPanel))
	{
		Client_ClearCraftingSlot(GridKey);
		Client_ClearWaitingSlot(GridKey);
	}
}

// ----------------------Crafting

void UInventoryManagerComponent::CreateCraftingSlot(FGuid CraftingId, int32 Count, TSubclassOf<UCraftingSlotUserWidget> Class, FName GridKey, TArray<FItemDataInfoCraftCompressed> DefaultValue, UCraftingComponent* CraftingComp)
{
	Client_ClearCraftingSlot(GridKey);

	if (Count > 0)
	{
		for (int32 i = 0; i < Count; i++)
		{
			Client_AddCraftingSlot(CraftingId,i, Class, GridKey, DefaultValue[i], CraftingComp);
		}
	}
}


void UInventoryManagerComponent::Client_AddCraftingSlot_Implementation(FGuid CraftingId, int32 Slot, TSubclassOf<UCraftingSlotUserWidget> Class, FName GridKey, FItemDataInfoCraftCompressed DefaultValue, UCraftingComponent* CraftingComp)
{
	if (Class == NULL)
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Client_AddCraftingSlot] Class CraftingSlot Invalid in Crafting Component"));
		return;
	}

	
	if (UInventoryUniformGridPanel* GridPanel = GetGrid(GridKey))
	{
		if (!IsValid(GridPanel->PanelCrafting))
		{
			return;
		}

		UCraftingSlotUserWidget* MyCraftSlot = CreateWidget<UCraftingSlotUserWidget>(GetWorld(), Class);
		MyCraftSlot->InitCraft(CraftingId, Slot, this, false, UChestoryFunctionLibrary::MakeItemCraftFromTransfer(this, DefaultValue));

		GridPanel->AddCraftingSlot(MyCraftSlot);
	}
}

void UInventoryManagerComponent::Client_RemoveCraftingSlot_Implementation(FGuid CraftingId, int32 Slot, FName GridKey)
{
	if (UInventoryUniformGridPanel* GridPanel = GetGrid(GridKey))
	{
		if (!IsValid(GridPanel->PanelCrafting))
		{
			return;
		}

		GridPanel->RemoveCraftingSlot(Slot);
	}
}

void UInventoryManagerComponent::AddCustomCraftingSlot(UCraftingComponent* Crafting, UCraftingSlotUserWidget* CraftingSlot, FName Gridkey, UItemDataAsset* ItemDataAsset)
{
	if (IsValid(CraftingSlot))
	{
		Client_AddCustomCraftingSlot(Crafting, CraftingSlot, Gridkey, ItemDataAsset);
	}
}

void UInventoryManagerComponent::Client_AddCustomCraftingSlot_Implementation(UCraftingComponent* Crafting, UCraftingSlotUserWidget* CraftingSlot, FName Gridkey, UItemDataAsset* ItemDataAsset)
{
	if (!IsValid(Crafting) || !IsValid(ItemDataAsset))
	{
		return;
	}

	if (!Crafting->bAutoCreationCraftingSlot)
	{
		if (UInventoryUniformGridPanel* Grid = GetGrid(Gridkey))
		{
			const FItemDataInfoCraft LocalItemInfoCraft = UChestoryFunctionLibrary::MakeItemInfoCraft(this, ItemDataAsset);
			const int32 LocalIndexSlot = Grid->GetCraftingSlots().Num();

			UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::Client_AddCustomCraftingSlot_] CraftingId:[%s] At Index:[%d]"), *Crafting->GetId().ToString(), LocalIndexSlot);

			CraftingSlot->InitCraft(Crafting->GetId(), LocalIndexSlot, this, false, LocalItemInfoCraft);
			Grid->AddCraftingSlot(CraftingSlot, false);
		}
	}
}

void UInventoryManagerComponent::Client_ClearCraftingSlot_Implementation(FName GridKey)
{
	if (UInventoryUniformGridPanel* GridPanel = GetGrid(GridKey))
	{
		GridPanel->ClearCraftingSlots();

	}
}

void UInventoryManagerComponent::Client_SetItemCrafting_Implementation(FName GridKey, int32 Slot, FItemDataInfoCraftCompressed Info)
{
	if (!GetInvSubsystem()->ContainsGrid(GridKey))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Client_SetItemCrafting] Not grid found with GridKey :[%s]"), *GridKey.ToString());
		return;
	}


	if (UInventoryUniformGridPanel* GridPanel = GetGrid(GridKey))
	{
		//Check if this client instance has Data (if opened this crafting)
		if (GridPanel->GetCraftingSlots().IsEmpty())
		{
			return;
		}

		GridPanel->GetCraftingSlot(Slot)->SetCraft(UChestoryFunctionLibrary::MakeItemCraftFromTransfer(this, Info));
	}
}

void UInventoryManagerComponent::Client_SetItemCraftingAmount_Implementation(FName GridKey, int32 Slot, const TArray<int32>& ItemCraftAmount, int32 MaxAmount)
{
	if (!GetInvSubsystem()->ContainsGrid(GridKey))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Client_SetItemCraftingAmount] Not grid found with GridKey :[%s]"), *GridKey.ToString());
		return;
	}

	if (UInventoryUniformGridPanel* GridPanel = GetGrid(GridKey))
	{
		//Check if this client instance has Data (if opened this crafting)
		if (GridPanel->GetCraftingSlots().IsEmpty())
		{
			return;
		}

		if (!GridPanel->GetCraftingSlots().IsValidIndex(Slot))
		{
			UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Client_SetItemCraftingAmount] Invalid Index CraftingSlot:[%d] On GridKey :[%s]"), Slot, *GridKey.ToString());
			return;
		}

		GridPanel->GetCraftingSlot(Slot)->UpdateIngredientNeeded(ItemCraftAmount, MaxAmount);
	}

}

void UInventoryManagerComponent::CreateWaitingSlot(FGuid CraftingId, TSubclassOf<UCraftingSlotUserWidget> ClassWaiting, FName GridKey, TArray<FItemDataInfoCraftCompressed> DefaultValue)
{
	if (ClassWaiting == NULL)
	{
		return;
	}

	//Not sure
	if (DefaultValue.IsEmpty())
	{
		return;
	}

	if (!IsValid(GetGrid(GridKey)))
	{
		return;

	}

	for (int32 i = 0; i < DefaultValue.Num(); i++)
	{
		Client_AddWaitingSlot(CraftingId, i, ClassWaiting, GridKey, DefaultValue[i]);
	}
	
}

void UInventoryManagerComponent::Client_RemoveWaitingSlot_Implementation(int32 Slot, FName GridKey)
{
	if (UInventoryUniformGridPanel* GridPanel = GetGrid(GridKey))
	{
		GridPanel->RemoveWaitingSlot(Slot);
	}
}

void UInventoryManagerComponent::Client_AddWaitingSlot_Implementation(FGuid CraftingId, int32 Slot, TSubclassOf<UCraftingSlotUserWidget> ClassWaiting, FName GridKey, FItemDataInfoCraftCompressed DefaultValue)
{

	if (ClassWaiting == NULL)
	{
		return;
	}

	if (UInventoryUniformGridPanel* GridPanel = GetGrid(GridKey))
	{
		if (!IsValid(GridPanel->PanelWaitingList))
		{
			return;
		}

		UCraftingSlotUserWidget* MyWaitingSlot = CreateWidget<UCraftingSlotUserWidget>(GetWorld(), ClassWaiting);
		MyWaitingSlot->InitCraft(CraftingId, Slot, this, true, UChestoryFunctionLibrary::MakeItemCraftFromTransfer(this, DefaultValue));

		GridPanel->AddWaitingSlot(MyWaitingSlot);
	}
}

void UInventoryManagerComponent::Client_ClearWaitingSlot_Implementation(FName GridKey)
{
	if (UInventoryUniformGridPanel* GridPanel = GetGrid(GridKey))
	{
		GridPanel->ClearWaitingSlots();
	}
}

void UInventoryManagerComponent::Client_SetItemWaiting_Implementation(FName GridKey, int32 Slot, FItemDataInfoCraftCompressed NewInfo)
{
	if (!GetInvSubsystem()->ContainsGrid(GridKey))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Client_SetItemWaiting] Not grid found with GridKey :[%s]"), *GridKey.ToString());
		return;
	}

	if (UInventoryUniformGridPanel* GridPanel = GetGrid(GridKey))
	{
		//Check if this client instance has Data (if opened this crafting)
		if (GridPanel->GetCraftingSlots().IsEmpty())
		{
			return;
		}

		GridPanel->GetWaitingSlot(Slot)->SetCraft(UChestoryFunctionLibrary::MakeItemCraftFromTransfer(this, NewInfo));
	}
}

void UInventoryManagerComponent::CLient_Start_Crafting_Implementation(FName GridKey, int32 Slot, float TimeOfCraft)
{
	if (UInventoryUniformGridPanel* GridPanel = GetGrid(GridKey))
	{
		GridPanel->StartCrafting(Slot, TimeOfCraft);
	}
}

void UInventoryManagerComponent::Client_Update_Crafting_Implementation(FName GridKey, int32 Slot, float CurrentTimer)
{
	if (UInventoryUniformGridPanel* GridPanel = GetGrid(GridKey))
	{

		GridPanel->UpdateCrafting(Slot, CurrentTimer);
	}
	else
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Client_Update_Crafting] GridPanel Invalid GridKey:[%s]"), *GridKey.ToString());

	}
}

void UInventoryManagerComponent::Client_End_Crafting_Implementation(FName GridKey, int32 Slot)
{
	if (UInventoryUniformGridPanel* GridPanel = GetGrid(GridKey))
	{
		GridPanel->EndCrafting(Slot);
	}
}




// ---------------------- FILTER CRAFTING MANAGEMENT ----------------------

void UInventoryManagerComponent::EnableVisibilityCraftingFilter_Implementation(FName GridKey,	FGameplayTagContainer Tags)
{

	if (!GridKey.IsValid())
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::EnableVisibilityCraftingFilter] GridKey Invalid GridKey:[%s]"), *GridKey.ToString());
		return;
	}

	if (!IsValid(GetGrid(GridKey)))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::EnableVisibilityCraftingFilter] UInventoryUniformGridPanel Invalid With GridKey:[%s]"), *GridKey.ToString());
		return;
	}


	for(const auto Slot : GetGrid(GridKey)->GetCraftingSlots())
	{
		bool HasTagCategory = false;
		HasTagCategory = Slot->ItemInformationCraft.ItemInformation.GameplayTagContainer.HasAllExact(Tags);
		Slot->SetVisibility(HasTagCategory ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

}

void UInventoryManagerComponent::DisableVisibilityCraftingFilter_Implementation(FName GridKey)
{
	if (!GridKey.IsValid())
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::DisableVisibilityCraftingFilter] GridKey Invalid GridKey:[%s]"), *GridKey.ToString());
		return;
	}

	if (!IsValid(GetGrid(GridKey)))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::DisableVisibilityCraftingFilter] UInventoryUniformGridPanel Invalid With GridKey:[%s]"), *GridKey.ToString());
		return;
	}

	for(const auto Slot : GetGrid(GridKey)->GetCraftingSlots())
	{
		Slot->SetVisibility(ESlateVisibility::Visible);
	}
}



void UInventoryManagerComponent::Client_RefreshItemBlueprintInfoSlots_Implementation(ESlotType Type)
{
	//RefreshItemBlueprintInfoSlots(Type);
}







void UInventoryManagerComponent::RefreshInventorySlots_Implementation(FGuid InventoryId, FName GridKey)
{
	if (bEnableSmoothNet)
	{
		//Smooth Data
		NetCurrentInventoryId = InventoryId;
		NetCurrentGridKey = GridKey;
		StartTimerStackNet();
	}
	else
	{
		//All data
		SendDataInventorySlots(InventoryId,GridKey,0, GetInventory(InventoryId)->GetInventoryItems().Num());
		
	}
}

void UInventoryManagerComponent::RefreshEquipementSlots_Implementation(bool TriggerScript, FName Context)
{
	if (IsValid(EquipementPlayerInventory))
	{
		if (EquipementPlayerInventory->GetInventoryItems().Num() > 0)
		{
			//if (!GetInvSubsystem()->bUIEquipementIsCompleted) { UE_LOG(LogInventory, Display, TEXT("[RefreshEquipementSlots] Can't refresh client is not ready whait UI "));  return; }

			TArray<FItemData> Inventory = EquipementPlayerInventory->GetInventoryItems();
			UE_LOG(LogInventory, Display, TEXT("[RefreshEquipementSlots] Total EquipementInventory : %d"), Inventory.Num());

			for (int32 i = 0; i < Inventory.Num(); i++)
			{
				// Update Item UI 
				if (!Context.IsNone())
				{
					Client_SetEquipementSlotItem(EquipementPlayerInventory->GetId(),i, UChestoryFunctionLibrary::MakeItemTransfer(Inventory[i]));
				}

				//Run Script
				if (EquipementPlayerInventory->GetItemIsValid(Inventory[i]) && TriggerScript)
				{
					// For All Script Call Event ItemAddInInventory
					UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::RefreshEquipementSlots] ItemScript ItemAddInInventory (For Load) : %s"), *Inventory[i].ID.ToString());
					Inventory[i].NotifyScript_ItemAdd(this, EquipementPlayerInventory->GetId(), i, EquipementPlayerInventory);

					Server_SpawnActorEquiment(EquipementPlayerInventory, i);	

					OnEquip.Broadcast(EquipementPlayerInventory->GetKeyAtIndex(i), Inventory[i], FItemData(), EquipementPlayerInventory->GetInventoryItem(i).ActorInWorld);
				}



			}
		}
	}
}


void UInventoryManagerComponent::RefreshHotbarSlots_Implementation()
{
	//Is Server
	if (!IsValid(HotbarInventory))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::RefreshHotbarSlots] HotbarInventory Invalid"));
		return;
	}

	if (HotbarInventory->GetInventoryItems().IsEmpty())
	{
		UE_LOG(LogInventory, Display, TEXT("[UInventoryManagerComponent::RefreshHotbarSlots] HotbarInventory is empty"));
		return;
	}

	if (!HotbarInventory->GetId().IsValid())
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::RefreshHotbarSlots] HotbarInventory InventorId Invalid"));
		return;
	}

	if (!GetInvSubsystem()->ContainsInventory(HotbarInventory->GetId()))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::RefreshHotbarSlots] Can't found HotbarInventory in Subsystem with InventoryId:[%s]"),*HotbarInventory->GetId().ToString());
		return;
	}

	//if (HotbarInventory->GetInventoryManager() != this)
	//{
	//	UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::RefreshHotbarSlots] Try to reshrech then not same instance Impossible for Hotbar InventoryId:[%s]"), *HotbarInventory->GetId().ToString());
	//	return;
	//}

	TArray<FItemData> Inventory = HotbarInventory->GetInventoryItems();
	UE_LOG(LogInventory, Display, TEXT("[UInventoryManagerComponent::RefreshHotbarSlots] Total:[%d] Owner:[%s] InventoryId:[%s]"), Inventory.Num(),*GetOwner()->GetFName().ToString(), *HotbarInventory->GetId().ToString());

	for (int32 i = 0; i < Inventory.Num(); i++)
	{
		//Server trigger Script
		if (HotbarInventory->GetInventoryItem(i).IsValid())
		{
			// UI 
			Client_SetHotbarSlotItem(HotbarInventory->GetId(), i, UChestoryFunctionLibrary::MakeItemTransfer(Inventory[i]));

			// For All Script Call Event ItemAddInInventory
			UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::RefreshHotbarSlots] ItemScript ItemAddInInventory (For Load) : %s"), *Inventory[i].ID.ToString());
			Inventory[i].NotifyScript_ItemAdd(this, HotbarInventory->GetId(), i, HotbarInventory);
		}
		else
		{
			UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::RefreshHotbarSlots] Item Clear Slot:[%d]"), i);

			Client_ClearHotBarSlotItem(HotbarInventory->GetId(), i);
		}
	}
}


void UInventoryManagerComponent::SendDataInventorySlots(FGuid InventoryId, FName GridKey, int32 StartStack, int32 EndStack)
{
	if (!GetInvSubsystem()->ContainsInventory(InventoryId))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::SendDataInventorySlots] Inventory not found with InventoryId: [%s]"), *InventoryId.ToString());
		return;
	}

	if (UInventoryComponent* InventoryComp = GetInventory(InventoryId))
	{
		if (InventoryComp->GetInventoryItems().Num() > 0)
		{
			TArray<FItemData> Inventory = InventoryComp->GetInventoryItems();
			UE_LOG(LogInventory, Display, TEXT("[RefreshInventorySlots] Total PlayerInventory : %d"), Inventory.Num());
			UE_LOG(LogInventory, Display, TEXT("[RefreshInventorySlots] New Stack Start to : %d"), StartStack);

			for (int32 i = StartStack; i < Inventory.Num(); i++)
			{
				
				if (InventoryComp->GetItemIsValid(Inventory[i]))
				{

					//UI
					Client_SetInventorySlotItem(InventoryId,GridKey,i, UChestoryFunctionLibrary::MakeItemTransfer(Inventory[i]));

					// For All Script Call Event ItemAddInInventory
					UE_LOG(LogInventory, Verbose, TEXT("[RefreshInventorySlots] ItemScript ItemAddInInventory (For Load) : %s"), *Inventory[i].ID.ToString());
					Inventory[i].NotifyScript_ItemAdd(this, InventoryId, i, InventoryComp);
				}
				else
				{
					Client_ClearInventorySlotItem(InventoryId, GridKey,i);
				}

				CurrentSlotStack++;
				
				if (i == EndStack || i == InventoryComp->Inventory.Num())
				{
					UE_LOG(LogInventory, Verbose, TEXT("[RefreshInventorySlots] End Refresh"));
					break;;
				}
			}
		}
	}
}



void UInventoryManagerComponent::StartTimerStackNet_Implementation()
{
	if(!GetWorld()->GetTimerManager().IsTimerActive(TimerStackNet))
	{
		Client_ClearInventory(NetCurrentGridKey);
		
		GetWorld()->GetTimerManager().SetTimer(TimerStackNet,this,&UInventoryManagerComponent::CurrentTimerStackNet,DelayBetweenStack,true,0.0f);
	}
}

void UInventoryManagerComponent::CurrentTimerStackNet_Implementation()
{

	SendDataInventorySlots(NetCurrentInventoryId,NetCurrentGridKey,CurrentSlotStack,CurrentSlotStack+StackSize);

	if (CurrentSlotStack == GetInventory(NetCurrentInventoryId)->GetInventoryItems().Num())
	{
		UE_LOG(LogInventory, Verbose, TEXT("[RefreshInventorySlots] Clear Timer"));
		CurrentSlotStack = 0;
		NetCurrentGridKey = FName();
		NetCurrentInventoryId = FGuid();
		GetWorld()->GetTimerManager().ClearTimer(TimerStackNet);
	}
	
}


void UInventoryManagerComponent::Server_UpdateDecaySlots_Implementation(FGuid InventoryId, FName GridKey,
	const TArray<int32>& DecaySlots, const TArray<bool>& MaxRefresh)
{
	if (!GetInvSubsystem()->ContainsInventory(InventoryId))
	{
		UE_LOG(LogInventory,Warning,TEXT("[UInventoryManagerComponent::UInventoryManagerComponent::Server_UpdateDecaySlots] Inventory not found with InventoryID:[%s]"),*InventoryId.ToString())
		return;
	}

	if (UInventoryComponent* Inventory = GetInventory(InventoryId))
	{
		for (int32 i = 0; i < DecaySlots.Num(); i++)
		{
			if (!IsValid(Inventory))
			{
				UE_LOG(LogInventory,Warning,TEXT("[UInventoryManagerComponent::Server_UpdateDecaySlots] Can't be decay Inventory Lost"))
				return;
			}
			if (Inventory->GetInventoryItem(DecaySlots[i]).IsValid() && Inventory->GetInventoryItem(DecaySlots[i]).bCanDecay)
			{
				//Check if need Max Refresh slot
				if (MaxRefresh[i])
				{
					Client_SetInventorySlotItem(InventoryId, GridKey, DecaySlots[i], UChestoryFunctionLibrary::MakeItemTransfer(Inventory->GetInventoryItem(DecaySlots[i])));
				}
				else //juste decay value 
				{
					Client_SetDecaySlotItem(InventoryId,GridKey, DecaySlots[i],Inventory->GetInventoryItem(DecaySlots[i]).Decay, Inventory->DecayFactor);
				}
			}
		}
	}
}

void UInventoryManagerComponent::Server_SetRepairInfoSlot_Implementation(FGuid InventoryId, int32 IndexSlot)
{
	if (!InventoryId.IsValid())
	{
		return;
	}

	;
	if (UInventoryComponent* Inventory = GetInventory(InventoryId); IsValid(Inventory) )
	{
		if (!Inventory->bCanRepair)
		{
			return;
		}

		FItemDataInfoCraft ItemInfoRepair;
		
		FItemData Item = Inventory->GetInventoryItem(IndexSlot);
		if (!Item.IsValid())
		{
			return;
		}
		
		FItemDataInfo ItemInfo = UChestoryFunctionLibrary::MakeItemDataInfo(this, Inventory->GetInventoryItem(IndexSlot));
		if (CanBeRepair(ItemInfoRepair,ItemInfo))
		{
			bool OutResource;
			FItemDataInfoCraft ItemInfoRepairToSend = ConvertRecipeToRepairCost(OutResource, InventoryId, ItemInfoRepair,IndexSlot);

			if (ItemInfoRepairToSend.ItemInformation.IsValid() && !Inventory->GetIsHotbarInventory())
			{
				SetViewersRepairSlot(InventoryId, Inventory->GetViewers(), IndexSlot, UChestoryFunctionLibrary::MakeCraftTransfer(ItemInfoRepairToSend));
			}
		}
	}
}

void UInventoryManagerComponent::Server_SetItemBlueprintInfoSlot_Implementation(FGuid InventoryId, const int32 IndexSlot)
{
	UInventoryComponent* Inventory = GetInventory(InventoryId);
	if (IsValid(Inventory))
	{
		const FItemData ItemData = Inventory->GetInventoryItem(IndexSlot);
		if (Inventory->GetItemIsValid(ItemData); ItemData.ItemType == EItemType::EBlueprint)
		{
			UItemDataAsset* ItemAsset = GetInvSubsystem()->GetItemDataAssetFromPrimaryAssetId(GetInvSubsystem()->GetPrimaryAssetIdFromName(ItemData.ID));
			if (IsValidRecipe(ItemAsset,false))
			{
				FItemDataInfoCraft CurrentInfoCraft = UChestoryFunctionLibrary::MakeItemInfoCraft(this,ItemAsset);
				TArray<int32> RecipeAmount;
				ApplyAdditionalCraftData(InventoryId, CurrentInfoCraft, RecipeAmount);
				

				SetViewersBlueprintSlot(InventoryId, Inventory->GetViewers(), IndexSlot, UChestoryFunctionLibrary::MakeCraftTransfer(CurrentInfoCraft));
			}
		}
	}
}

void UInventoryManagerComponent::Server_UpdateItemBlueprint_Implementation(FGuid InventoryId)
{
	UInventoryComponent* Inventory = GetInventory(InventoryId);
	if (IsValid(Inventory))
	{
		for(int32 i = 0; i < Inventory->GetInventoryItems().Num(); i++)
		{
			if (Inventory->GetInventoryItem(i).IsBlueprint())
			{
				Server_SetItemBlueprintInfoSlot(InventoryId,i);
			}
		}
	}
}

/// ------- For Debug Network
/// 
void UInventoryManagerComponent::PrintHasAuthority(FName print)
{
	const bool bHasAuthority = GetOwner()->HasAuthority();

	UE_LOG(LogInventory, Warning, TEXT("%s HasAuthority : %s"),*print.ToString(), bHasAuthority ? TEXT("Is Server True") : TEXT("Is Client False"));
	if (GEngine != nullptr)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow,TEXT(" HasAuthority : %s"), bHasAuthority ? TEXT("Is Server True") : TEXT("Is Client False"));
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, FString::Printf(TEXT("%s HasAuthority : %s"), *print.ToString(), bHasAuthority ? TEXT("Is Server True") : TEXT("Is Client False")));
	}
		
}

void UInventoryManagerComponent::PrintPlayerHasAuthority(FString print)
{
	FString Player = "None";
	GetOwner()->GetName(Player);
	//UE_LOG(LogInventory, Error, TEXT("INIT PLAYER : %s"), *Player);
	const FString Total = print.Append(Player);

	PrintHasAuthority(FName(*Total));
}

FString UInventoryManagerComponent::NetModeToString(ENetMode NetMode)
{
	switch (NetMode)
	{
	case NM_Standalone:
		return FString("Standalone");
	case NM_DedicatedServer:
		return FString("DedicatedServer");
	case NM_ListenServer:
		return FString("ListenServer");
	case NM_Client:
		return FString("Client");
	case NM_MAX:
		return FString("Max");
	default:
		return FString("Standalone");
	}
}

UInventoryComponent* UInventoryManagerComponent::GetInventory(FGuid InventoryId)
{
	if (!IsValid(InventorySubsystem))
	{
		return nullptr;

	}

	return GetInvSubsystem()->GetInventory(InventoryId);
}

UCraftingComponent* UInventoryManagerComponent::GetCrafting(FGuid CraftingId)
{
	if (!IsValid(InventorySubsystem))
	{
		return nullptr;

	}

	return  GetInvSubsystem()->GetCrafting(CraftingId);
}

UInventoryUniformGridPanel* UInventoryManagerComponent::GetGrid(FName GridKey)
{
	if (!IsValid(InventorySubsystem))
	{
		return nullptr;

	}

	return GetInvSubsystem()->GetInventoryGrid(GridKey);
}

UInventorySlotUserWidget* UInventoryManagerComponent::GetEquipmentSlot(FName Context, FName SlotKey)
{
	if (!IsValid(InventorySubsystem))
	{
		return nullptr;

	}

	return GetInvSubsystem()->GetEquipmentSlot(Context,SlotKey);
}

UInventorySlotUserWidget* UInventoryManagerComponent::GetHotbarSlot(FName SlotKey)
{
	if (!IsValid(InventorySubsystem))
	{
		return nullptr;

	}

	if (IsValid(GetInvSubsystem()->GetHotbarSlot(SlotKey)))
	{
		return GetInvSubsystem()->GetHotbarSlot(SlotKey);
	}

	Client_PendingHotbar();
	
	return nullptr;
}

UInventorySlotUserWidget* UInventoryManagerComponent::GetAdvancedSlot(FGuid InventoryId, FName GridKey, int32 InventorySlot, FString DebugText)
{
	if (GridKey.IsNone()) //Is Slot
	{
		if (!GetInvSubsystem()->ContainsInventory(InventoryId))
		{
			return nullptr;
		}

		if (!IsValid(GetInventory(InventoryId)))
		{
			return nullptr;
		}

		if (GetInventory(InventoryId)->GetIsHotbarInventory())
		{
			if (IsValid(GetHotbarSlot(GetInventory(InventoryId)->GetKeyAtIndex(InventorySlot))))
			{
				if (!GetHotbarSlot(GetInventory(InventoryId)->GetKeyAtIndex(InventorySlot))->IsValidLowLevel())
				{
					return nullptr;
				}
				//Return Hotbar
				return GetHotbarSlot(GetInventory(InventoryId)->GetKeyAtIndex(InventorySlot));

			}
		}

		if (GetInventory(InventoryId)->GetIsEquipmentInventory())
		{
			if (MatchContextEquipments.IsEmpty())
			{
				return nullptr;
			}

			if (!MatchContextEquipments.Contains(InventoryId))
			{
				return nullptr;
			}

			FName Context = *MatchContextEquipments.Find(InventoryId);
			FName SlotKey = GetInventory(InventoryId)->GetKeyAtIndex(InventorySlot);
			if (Context.IsNone() || SlotKey.IsNone())
			{
				return nullptr;
			}

			if (!IsValid(GetEquipmentSlot(Context, SlotKey)))
			{
				UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::GetAdvancedSlot::%s] Widget EquipementSlot with Context:[%s] SlotKey:[%s] Invalid"),*DebugText, *Context.ToString(), *SlotKey.ToString());
				return nullptr;
			}

			//Return Equipement
			return GetEquipmentSlot(Context, SlotKey);
		}
	}
	else // Updade on Grid
	{
		if (!GetInvSubsystem()->ContainsGrid(GridKey) || GridKey.IsNone())
		{
			UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::GetAdvancedSlot::%s] Not grid found with GridKey :[%s]"), *DebugText, *GridKey.ToString());
			return nullptr;
		}

		if (UInventoryUniformGridPanel* GridPanel = GetGrid(GridKey))
		{
			if (!GridPanel->GetInventorySlots().IsValidIndex(InventorySlot))
			{
				UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::GetAdvancedSlot::%s] Grid [%s] not index valid [%d] ArrayLenght:[%d]"),*DebugText, *GridKey.ToString(), InventorySlot, GridPanel->GetInventorySlots().Num());
				return nullptr;
			}

			if (!IsValid(GridPanel->GetInventorySlot(InventorySlot)))
			{
				UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::GetAdvancedSlot::%s] Grid [%s] Slot widget Invalid Index:[%d]"),*DebugText, *GridKey.ToString(), InventorySlot);
				return nullptr;
			}

			return GridPanel->GetInventorySlot(InventorySlot);
		}
	}

	return nullptr;
}



bool UInventoryManagerComponent::bCanMoveItem(FGuid FromInventoryId, int32 FromInventorySlot, FGuid ToInventoryId, int32 ToInventorySlot)
{
	if (!FromInventoryId.IsValid())
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::bCanMoveItem] FromInventoryId Invalid"));
		return false;
	}

	if (!ToInventoryId.IsValid())
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::bCanMoveItem] ToInventoryId Invalid"));
		return false;
	}

	if (!GetInvSubsystem()->ContainsInventory(FromInventoryId))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::bCanMoveItem] From Inventory not found InventoryId:[%s]"), *FromInventoryId.ToString());
		return false;
	}

	if (!GetInvSubsystem()->ContainsInventory(ToInventoryId))
	{
		UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::bCanMoveItem] To Inventory not found  InventoryId:[%s]"), *ToInventoryId.ToString());
		return false;
	}

	if (FromInventoryId == ToInventoryId && FromInventorySlot == ToInventorySlot)
	{
		UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::bCanMoveItem] Same Invetory and slot do noting"));
		return false;
	}

	return true;
}

void UInventoryManagerComponent::Server_CancelMoveInventory_Implementation(FGuid FromInventoryId, int32 FromInventorySlot, FGuid ToInventory, int32 ToInventorySlot)
{
	UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::Server_CancelMoveInventory] Cancel Move Item"));

	Server_FilledItemInventory(FromInventoryId, FromInventorySlot);


	if (ToInventory.IsValid())
	{
		if (UInventoryComponent* Inventory = GetInventory(ToInventory))
		{
			if (!Inventory->GetIsGirdInventory())
			{
				return;
			}

			TArray<int32> Filled = Inventory->GetFilledIndexFromItemSlot(Inventory->GetInventoryItem(FromInventorySlot), ToInventorySlot);
			//Filled.Remove(Index);
			SetViewersCancelOverSlot(ToInventory, Inventory->GetViewers(), Filled);
		}

	}



}


void UInventoryManagerComponent::Server_MoveInventoryToInventory_Implementation(FGuid FromInventoryId, int32 FromInventorySlot, FGuid ToInventoryId, int32 ToInventorySlot, bool bSplit)
{
	if (!bCanMoveItem(FromInventoryId,  FromInventorySlot,  ToInventoryId,  ToInventorySlot))
	{
		Server_CancelMoveInventory(FromInventoryId, FromInventorySlot, ToInventoryId, ToInventorySlot);
		return;
	}

	UInventoryComponent* FromInventory = GetInventory(FromInventoryId);
	UInventoryComponent* ToInventory = GetInventory(ToInventoryId);

	//UE_LOG(LogInventory, Warning, TEXT("[UInventoryManagerComponent::Server_MoveInventoryToInventory] Size Bytes:[%d]"), FromInventory->GetInventoryItem(FromInventorySlot).DataBytesActor.Num());


	if (FromInventory->GetIsEquipmentInventory())
	{
		//Unequipe item
		UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::Server_MoveInventoryToInventory] UnequipItem"));
		UnequipItem(FromInventory, FromInventorySlot, ToInventory, ToInventorySlot);
		return;
	}

	if (ToInventory->GetIsEquipmentInventory())
	{
	   //Equipe Item
		UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::Server_MoveInventoryToInventory] EquipItem"));
		EquipItem(FromInventory, FromInventorySlot, ToInventory, ToInventorySlot);
		return;

	}

	//Chesk Shorcut hotbar
	if (bUseHotbarAsShortcut)
	{
		//Is not from Container and go in Inventory to Hotbar
		if (!FromInventory->GetIsContainerInventory() && ToInventory->GetIsHotbarInventory())
		{
			Client_MoveHotbarSlotItem(
				FromInventorySlot,
				ToInventorySlot,
				FromInventory->GetIsPlayerInventory(), 
				FromInventory->GetIsHotbarInventory(),
				UChestoryFunctionLibrary::MakeItemTransfer(FromInventory->GetInventoryItem(FromInventorySlot)));
			return;
		}

		if (FromInventory->GetIsHotbarInventory())
		{
			Client_ClearHotBarSlotItem(FromInventoryId, FromInventorySlot);
			return;
		}
	}


	if (bSplit)
	{
		UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::Server_MoveInventoryToInventory] SplitItem"));
		int32 AmountSplit = bHalfSplitItem ? GetInventory(FromInventoryId)->GetInventoryItem(FromInventorySlot).Amount / 2 : DefaultAmountToSplit;
		SplitItem(FromInventory, FromInventorySlot, ToInventory, ToInventorySlot, AmountSplit);
		return;

	}
	else
	{
		

		UE_LOG(LogInventory, Verbose, TEXT("[UInventoryManagerComponent::Server_MoveInventoryToInventory] MoveItem"));
		MoveItem(FromInventory, FromInventorySlot, ToInventory, ToInventorySlot);
		return;
	}
}

