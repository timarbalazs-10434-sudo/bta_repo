// Copyright 2024 Arkai Interactive. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "ChestorySaveGame.generated.h"


USTRUCT()
struct FInventorysSaveData
{
	GENERATED_BODY()

public:
	/* Identifier for which Actor this belongs to */
	UPROPERTY(SaveGame)
	FName InventoryName;

	/* Contains all 'SaveGame' marked variables of the Actor */
	UPROPERTY(SaveGame)
	TArray<uint8> ByteData;
};

/**
 * 
 */
UCLASS()
class CHESTORY_API UChestorySaveGame : public USaveGame
{
	GENERATED_BODY()
	

public:


	UPROPERTY()
	TArray<FInventorysSaveData> SavedInventory;


	UFUNCTION()
	void ClearData() { SavedInventory.Empty(); }

	UFUNCTION()
	void AddPlayer(FInventorysSaveData AddData);


};
