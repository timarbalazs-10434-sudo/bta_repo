// Copyright 2024 Arkai Interactive. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ChestorySaveGameInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class UChestorySaveGameInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class CHESTORY_API IChestorySaveGameInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	//------ Load

	UFUNCTION(BlueprintNativeEvent)
	void OnObjectPreLoad();

	UFUNCTION(BlueprintNativeEvent)
	void OnObjectPostLoad();

	//------ Save

	UFUNCTION(BlueprintNativeEvent)
	void OnObjectPreSave();

	UFUNCTION(BlueprintNativeEvent)
	void OnObjectPostSave();

	//------ Destory
	/*
	* Trigger when Equipment Actor is Destroyed / Unequip 
	*/
	UFUNCTION(BlueprintNativeEvent)
	void OnObjectPreDestoryed();

};
