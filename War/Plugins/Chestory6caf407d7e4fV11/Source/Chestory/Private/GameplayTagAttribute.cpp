// Copyright 2024 Arkai Interactive. All Rights Reserved.


#include "GameplayTagAttribute.h"
#include "Chestory.h"



FString FGameplayTagAttribute::GetDebugString() const
{
	return FString::Printf(TEXT("Debug %s x %f"), *AttributeTag.ToString(), Value);
}

void FGameplayTagAttributeContainer::SetAttribute(FGameplayTag AttributeTag, float AttributeValue)
{
	if (!AttributeTag.IsValid())
	{
		UE_LOG(LogInventory, Warning, TEXT("[AddAttribute] Invalid AttributeTag"));
	}

	//Set Value
	if (AttributeValue > 0)
	{
		for (FGameplayTagAttribute& Attribute : Attributes)
		{
			Attribute.GetDebugString();
			if (Attribute.AttributeTag == AttributeTag)
			{
				const float NewValue = AttributeValue;
				Attribute.Value = NewValue;
				TagToValueMap[AttributeTag] = NewValue;
				MarkItemDirty(Attribute);
				return;
			}

		}
		// if not found then create a new attribute
		FGameplayTagAttribute& NewAttribute = Attributes.Emplace_GetRef(AttributeTag, AttributeValue);
		MarkItemDirty(NewAttribute);
		TagToValueMap.Add(AttributeTag, AttributeValue);
		UE_LOG(LogInventory, VeryVerbose, TEXT("No Attribute found create a new attribut %s"), *NewAttribute.GetDebugString());
	}
	else // Remove Attribute
	{
		for (auto It = Attributes.CreateIterator(); It; It++)
		{
			FGameplayTagAttribute& Attribute = *It;
			if (Attribute.AttributeTag == AttributeTag)
			{
				// Remove then is < 0 
				It.RemoveCurrent();
				TagToValueMap.Remove(AttributeTag);
				MarkArrayDirty();
			}
		}
	}
}
void FGameplayTagAttributeContainer::AddAttribute(FGameplayTag AttributeTag, float AttributeValue)
{
	if (!AttributeTag.IsValid())
	{
		UE_LOG(LogInventory, Warning, TEXT("[AddAttribute] Invalid AttributeTag"));
	}

	if (AttributeValue > 0)
	{
		//Has Attribute
		if (TagToValueMap.Contains(AttributeTag))
		{
			for (FGameplayTagAttribute& Attribute : Attributes)
			{
				Attribute.GetDebugString();
				if (Attribute.AttributeTag == AttributeTag)
				{
					const float NewValue = Attribute.Value + AttributeValue;
					Attribute.Value = NewValue;
					TagToValueMap[AttributeTag] = NewValue;
					MarkItemDirty(Attribute);
					return;
				}
			}
		}
		else
		{
			// if not found then create a new attribute
			FGameplayTagAttribute& NewAttribute = Attributes.Emplace_GetRef(AttributeTag, AttributeValue);
			MarkItemDirty(NewAttribute);
			TagToValueMap.Add(AttributeTag, AttributeValue);
			UE_LOG(LogInventory, VeryVerbose, TEXT("No Attribute found create a new attribut %s"), *NewAttribute.GetDebugString());
		}

		
		
	}

}

void FGameplayTagAttributeContainer::RemoveAttribute(FGameplayTag AttributeTag, float AttributeValue)
{
	if (!AttributeTag.IsValid())
	{
		UE_LOG(LogInventory, Warning, TEXT("[RemoveAttribute] Invalid AttributeTag"));
	}

	if (AttributeValue > 0)
	{
		for (auto It = Attributes.CreateIterator(); It; It++)
		{
			FGameplayTagAttribute& Attribute = *It;
			if (Attribute.AttributeTag == AttributeTag)
			{
				if (Attribute.Value <= AttributeValue)
				{
					// Remove then is < 0 
					It.RemoveCurrent();
					TagToValueMap.Remove(AttributeTag);
					MarkArrayDirty();
				}
				else
				{
					//Update Value Attribute
					const float NewValue = Attribute.Value - AttributeValue;
					Attribute.Value = NewValue;
					TagToValueMap[AttributeTag] = NewValue;
					MarkItemDirty(Attribute);
					return;

				}
			}

		}
	}

}

void FGameplayTagAttributeContainer::ClearAttributes()
{
	Attributes.Empty();
	TagToValueMap.Empty();
	MarkArrayDirty();
	return;

}

void FGameplayTagAttributeContainer::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	for (int32 Index : RemovedIndices)
	{
		const FGameplayTag Tag = Attributes[Index].AttributeTag;
		TagToValueMap.Remove(Tag);
	}
}

void FGameplayTagAttributeContainer::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (int32 Index : AddedIndices)
	{
		const FGameplayTagAttribute& Attribute = Attributes[Index];
		TagToValueMap.Add(Attribute.AttributeTag, Attribute.Value);
	}
}

void FGameplayTagAttributeContainer::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	for (int32 Index : ChangedIndices)
	{
		const FGameplayTagAttribute& Attribute = Attributes[Index];
		TagToValueMap[Attribute.AttributeTag] = Attribute.Value;
	}
}
