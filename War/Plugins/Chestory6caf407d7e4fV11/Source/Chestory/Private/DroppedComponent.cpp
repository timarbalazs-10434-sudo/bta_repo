// Copyright 2024 Arkai Interactive. All Rights Reserved.


#include "DroppedComponent.h"
#include "Chestory.h"
#include "Net/UnrealNetwork.h"
#include "ChestoryFunctionLibrary.h"

// Sets default values for this component's properties
UDroppedComponent::UDroppedComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
	// ...
}

// Called when the game starts or when spawned
void UDroppedComponent::BeginPlay()
{
	Super::BeginPlay();

	InitializeSubSystem();

	if(GetOwner()->HasAuthority() && bBeginItem)
	{
		BeginWithItem();
	}
	

	if (GetItemData().IsValid())
	{
		InitDroppedComponent();
	}
}

void UDroppedComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME( UDroppedComponent, TimeRemaining );
	DOREPLIFETIME( UDroppedComponent, CreatedTime );
	DOREPLIFETIME_CONDITION( UDroppedComponent, ItemData, COND_SkipOwner );
	
}

// Called every frame
void UDroppedComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UDroppedComponent::InitDroppedComponent()
{
	if (GetOwner()->HasAuthority())
	{
		SetCreatedTime(UGameplayStatics::GetTimeSeconds(this));
		
		if (ItemData.bCanDecay)
		{
			GetWorld()->GetTimerManager().SetTimer(TimerDecay,this,&UDroppedComponent::DecayTick,DecayTickTime,true,0.0f);
		}

		if (bEnableAutoDestroy)
		{
			GetWorld()->GetTimerManager().SetTimer(TimerDestroy,this,&UDroppedComponent::DestroyTick,DestroyTime,false,-1.0f);
		}
				
		if (bDisablePhysics)
		{
			GetWorld()->GetTimerManager().SetTimer(TimerPhysics,this,&UDroppedComponent::PhysicsTick,PhysicsTime,false,-1.0f);
		}
	}
}

float UDroppedComponent::GetTimeRemaining()
{
	if (GetOwner()->HasAuthority())
	{
		Server_UpdateTimeRemaining();
	}
	
	
	return TimeRemaining;
}

void UDroppedComponent::Server_SetItemData_Implementation(FItemData Item, bool Init)
{
	NetMulticast_SendItemDataToClient(Item);
	if(Init)
	{
		InitDroppedComponent();
	}
}

void UDroppedComponent::NetMulticast_SendItemDataToClient_Implementation(FItemData Item)
{
	SetItemData(Item);
	OnNewItemData.Broadcast(GetItemData());
}


void UDroppedComponent::Server_UpdateTimeRemaining_Implementation()
{
	const float CurrentTime = UGameplayStatics::GetTimeSeconds(this);
	const float TimeElapsed = (CurrentTime - GetCreatedTime());

	TimeRemaining = DestroyTime - TimeElapsed;
}



void UDroppedComponent::DecayTick()
{
	if (ItemData.bCanDecay)
	{
		ItemData.Decay -= (DecayTickTime * DecayFactor);
		if (ItemData.Decay <= 0.0f)
		{
			ItemData.Decay = ItemData.DecayTime;
			ItemData.Amount -= 1;
			
			if (ItemData.Amount == 0)
			{
				GetOwner()->Destroy();
			}
		}
	}
}

void UDroppedComponent::DestroyTick()
{
	GetOwner()->Destroy();
}

void UDroppedComponent::PhysicsTick()
{
	Server_DisablePhysics();
}

void UDroppedComponent::InitializeSubSystem()
{
	UGameInstance* GameInstance = GetOwner()->GetGameInstance();
	InventorySubsystem = GameInstance->GetSubsystem<UInventorySubsystem>();

}

void UDroppedComponent::BeginWithItem()
{
	if (IsValid(InventorySubsystem))
	{
		if (bBeginItem)
		{
			if (AssetId.IsValid())
			{
				Server_SetItemData(UChestoryFunctionLibrary::MakeItem(GetWorld(), AssetId, Amount, Decay, Durability, Attributes, Sockets, JsonParameter),true);
			}
		}
	}
}


void UDroppedComponent::Server_DisablePhysics_Implementation()
{
	NetMulticast_DisablePhysics(GetOwner()->GetTransform());
}

void UDroppedComponent::NetMulticast_DisablePhysics_Implementation(FTransform EndTransform)
{
	GetOwner()->SetActorTransform(EndTransform);
	GetOwner()->DisableComponentsSimulatePhysics();
}
