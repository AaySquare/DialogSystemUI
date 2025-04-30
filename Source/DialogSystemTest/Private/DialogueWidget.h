// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DialogueDataAsset.h"
#include "Blueprint/UserWidget.h"

#include "DialogueWidget.generated.h"

class UKeywordPopupWidget;
class URichTextBlock;
class UImage;
class UTextBlock;
class UInputAction;
class UInputMappingContext;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDialogueFinished);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDialogueStarted);

USTRUCT()
struct FDialogueSegment
{
	GENERATED_BODY()

	UPROPERTY()
	bool bIsTag;

	UPROPERTY()
	FString Content;
};

UCLASS()
class UDialogueWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetupInputBindings();
	void StartDialogue(UDialogueDataAsset* DialogueDataAsset);
	
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void StartTyping(const FString& Dialogue);
	
	UPROPERTY(BlueprintAssignable, Category = "Dialogue")
	FOnDialogueFinished OnDialogueFinished;

	UPROPERTY(BlueprintAssignable, Category = "Dialogue")
	FOnDialogueFinished OnDialogueStarted;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue")
	void UpdateDialogueBinding();
	
	static TArray<FDialogueSegment> ParseDialogue(const FString& FullDialogue);
	
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UTextBlock> CharacterNameText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<URichTextBlock> CharacterDialogueRichText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UImage> CharacterImage;
	
	UPROPERTY(BlueprintReadOnly, Category = "Dialogue", meta = (Notify))
	FText CurrentCharacterName;

	UPROPERTY(BlueprintReadOnly, Category = "Dialogue", meta = (Notify))
	FText CurrentCharacterDialogue;

	UPROPERTY(BlueprintReadOnly, Category = "Dialogue", meta = (Notify))
	TObjectPtr<UTexture2D> CurrentCharacterPortrait;

private:
	void ShowNextDialogue();
	void EndDialogue() const;
	void UpdateText(); 
	void SkipTyping();
	void RemoveInputMappingContext() const;
	
	bool AppendNextCharacter();
	FString BuildRenderText() const;

	TArray<FDialogueData> CurrentDialogueData;
	FText CurrentFullDialogueText;
	FString CurrentPartialDialogueText;
	int32 CurrentDialogueIndex = -1;
	int32 TypedCharacterCount = 0;
	int32 CurrentCharacterIndex = 0;
	TArray<FDialogueSegment> DialogueSegments;
	
	FTimerHandle TextTimerHandle;
	
	TArray<FString> CurrentOpenTags;
	
	UPROPERTY()
	TObjectPtr<UDialogueDataAsset> CachedDialogueDataAsset;
	
	UPROPERTY(EditAnywhere, Category = "Dialogue", meta = (ClampMin = "0.01", ClampMax = "1.0"))
	float TypeSpeed = 0.05f;  

	UPROPERTY(EditAnywhere, Category = "Dialogue", meta = (ClampMin = "0.0", ClampMax = "5.0"))
    float AutoAdvanceDelay = 1.0f; 

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputMappingContext> DialogueMappingContext;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> SkipDialogueAction;

	UPROPERTY(EditAnywhere, Category = "Dialogue")
	bool bAutoAdvance = true;
};
