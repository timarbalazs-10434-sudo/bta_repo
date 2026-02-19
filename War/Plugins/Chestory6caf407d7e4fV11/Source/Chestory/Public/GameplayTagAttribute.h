// Copyright 2024 Arkai Interactive. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "Engine/NetSerialization.h"
#include "GameplayTagContainer.h"


#include "GameplayTagAttribute.generated.h"


struct FGameplayTagAttributeContainer;


USTRUCT(BlueprintType)
struct FGameplayTagAttribute : public FFastArraySerializerItem
{

	GENERATED_BODY()


	FGameplayTagAttribute() {}

	FGameplayTagAttribute(FGameplayTag InTag, float InValue) 
		: AttributeTag(InTag),
		Value(InValue)
	{
	}


	FString GetDebugString() const;


private:
	
	friend FGameplayTagAttributeContainer;

public:
	

	UPROPERTY(EditAnywhere, SaveGame, BlueprintReadWrite, Category = "Attribute")
	FGameplayTag AttributeTag;

	
	UPROPERTY(EditAnywhere, SaveGame, BlueprintReadWrite, Category = "Attribute")
	float Value = 0.0f;

};


USTRUCT(BlueprintType)
struct FGameplayTagAttributeContainer : public FFastArraySerializer
{
	GENERATED_BODY()

		FGameplayTagAttributeContainer()
	{
	}

public: 


	void SetAttribute(FGameplayTag AttributeTag, float AttributeValue);


	// Add value on current attribute value or add new attriube with this value
	
	void AddAttribute(FGameplayTag AttributeTag, float AttributeValue);


	void RemoveAttribute(FGameplayTag AttributeTag, float AttributeValue);

	float GetAttributeValue(FGameplayTag AttributeTag) const { return TagToValueMap.FindRef(AttributeTag); }

	bool ContainsAttributeTag(FGameplayTag AttributeTag) const { return TagToValueMap.Contains(AttributeTag); }

	TMap<FGameplayTag, float> GetAttributesToMapFormat() { return TagToValueMap; }

	int GetNum() const { return Attributes.Num(); }

	void ClearAttributes();

	//~FFastArraySerializer contract
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);
	//~End of FFastArraySerializer contract

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FGameplayTagAttribute, FGameplayTagAttributeContainer>(Attributes, DeltaParms, *this);
	}

	TArray<FGameplayTagAttribute> GetAttributes() { return Attributes; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Attributes", meta = (TitleProperty = "{AttributeTag} Value :{Value}"))
	TArray<FGameplayTagAttribute> Attributes;
private:
	
	UPROPERTY(NotReplicated,SaveGame)
	TMap<FGameplayTag, float> TagToValueMap;
};

template<>
struct TStructOpsTypeTraits<FGameplayTagAttributeContainer> : public TStructOpsTypeTraitsBase2<FGameplayTagAttributeContainer>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};