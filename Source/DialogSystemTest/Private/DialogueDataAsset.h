// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <Engine/DataAsset.h>

#include "DialogueDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FSpeakerData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speaker")
	FText SpeakerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speaker")
	TObjectPtr<UTexture2D> SpeakerPortrait;
};

USTRUCT(BlueprintType)
struct FDialogueData
{
	GENERATED_BODY()
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue", meta = (GetOptions = "GetSpeakerIDs"))
	FName SpeakerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FText DialogueText;
};

UCLASS()
class UDialogueDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	TArray<FDialogueData> DialogueData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	TMap<FName, FSpeakerData> SpeakersMap;

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	const TArray<FDialogueData>& GetDialogueData() const { return DialogueData; }

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	const FSpeakerData& GetSpeakerData(FName SpeakerID) const;

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	TArray<FName> GetSpeakerIDs() const;
};

inline const FSpeakerData& UDialogueDataAsset::GetSpeakerData(const FName SpeakerID) const
{
	const FSpeakerData* SpeakerDataPtr = SpeakersMap.Find(SpeakerID);
	if (!SpeakerDataPtr)
	{
		static const FSpeakerData DefaultSpeakerData;
		return DefaultSpeakerData;
	}
	return *SpeakerDataPtr;
}

inline TArray<FName> UDialogueDataAsset::GetSpeakerIDs() const
{
	TArray<FName> SpeakerIDs;
	for (const auto& SpeakerData : SpeakersMap)
	{
		SpeakerIDs.Add(SpeakerData.Key);
	}
	return SpeakerIDs;
}
