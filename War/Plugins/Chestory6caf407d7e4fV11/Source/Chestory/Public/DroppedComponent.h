// Copyright 2024 Arkai Interactive. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InventoryPluginStruct.h"
#include "InventorySubsystem.h"
#include "Components/ActorComponent.h"
#include "DroppedComponent.generated.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNewItemData, FItemData, ItemData);
PRAGMA_ENABLE_DEPRECATION_WARNINGS

UCLASS(Blueprintable, ClassGroup=(Inventory), meta=(BlueprintSpawnableComponent) )
class CHESTORY_API UDroppedComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UDroppedComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable,Category = "Dropped")
	void InitDroppedComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	bool bBeginItem = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (EditCondition = "bBeginItem"))
	FPrimaryAssetId AssetId;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (EditCondition = "bBeginItem"))
	int32 Amount = 1;
	// If = -1 then use default property

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (EditCondition = "bBeginItem"))
	float Decay = -1.f;
	// If = -1 then use default property

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (EditCondition = "bBeginItem"))
	float Durability = -1.f;
	// If empty then use default property

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (EditCondition = "bBeginItem"))
	FGameplayTagAttributeContainer Attributes;
	// If empty then use default property
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (EditCondition = "bBeginItem"))
	TArray<FSocketItem> Sockets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (EditCondition = "bBeginItem"))
	FString JsonParameter = "";
	
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Spawn",meta = (ExposeOnSpawn="true"))
	FItemData ItemData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float DecayFactor = 1.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float DecayTickTime = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bEnableAutoDestroy = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (EditCondition = "bEnableAutoDestroy"))
	float DestroyTime = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bDisablePhysics = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (EditCondition = "bDisablePhysics"))
	float PhysicsTime = 5.0f;

	
	UPROPERTY(BlueprintAssignable, Category = "InventoryManager|Event")
	FOnNewItemData OnNewItemData;

	FItemData GetItemData() const {return ItemData;};

	
	
	/** Get creation time ItemActor */
	UFUNCTION(BlueprintCallable,BlueprintPure, Category="Inventory")
	float GetCreatedTime() {return CreatedTime;};

	void SetCreatedTime(float NewCreatedTime) {CreatedTime = NewCreatedTime;};

	/** Get time remaining before destruction (Server time) */
	UFUNCTION(BlueprintCallable,BlueprintPure, Category="Inventory")
	float GetTimeRemaining();
	
	UFUNCTION(Server,Reliable)
	void Server_UpdateTimeRemaining();

	UFUNCTION(BlueprintCallable, Server,Reliable, Category = "Inventory")
	void Server_SetItemData(FItemData Item, bool Init);

	UFUNCTION(NetMulticast,Reliable)
	void NetMulticast_SendItemDataToClient(FItemData Item);
	

private:

	void SetItemData(FItemData NewItemData) {ItemData = NewItemData;};

	FTimerHandle TimerDecay;
	void DecayTick();

	FTimerHandle TimerDestroy;
	void DestroyTick();

	FTimerHandle TimerPhysics;
	void PhysicsTick();
	
	UFUNCTION(Server,Reliable)
	void Server_DisablePhysics();

	UFUNCTION(NetMulticast,Reliable)
	void NetMulticast_DisablePhysics(FTransform EndTransform);

	UPROPERTY(Replicated)
	float CreatedTime;

	UPROPERTY(Replicated)
	float TimeRemaining;

	void InitializeSubSystem();
	UInventorySubsystem* InventorySubsystem;

	void BeginWithItem();
};
