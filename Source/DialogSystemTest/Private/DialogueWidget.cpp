// Fill out your copyright notice in the Description page of Project Settings.

#include "DialogueWidget.h"

#include <EnhancedInputSubsystems.h>
#include <EnhancedInputComponent.h>

void UDialogueWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UDialogueWidget::NativeDestruct()
{
	if (GetWorld() && GetWorld()->GetTimerManager().IsTimerActive(TextTimerHandle))
	{
		GetWorld()->GetTimerManager().ClearTimer(TextTimerHandle);
	}
	
	Super::NativeDestruct();
}

TArray<FDialogueSegment> UDialogueWidget::ParseDialogue(const FString& FullDialogue)
{
	TArray<FDialogueSegment> Segments;
	int32 Index = 0;
	while (Index < FullDialogue.Len())
	{
		if (FullDialogue[Index] == '<')
		{
			int32 TagEnd = FullDialogue.Find(">", ESearchCase::IgnoreCase, ESearchDir::FromStart, Index);
			if (TagEnd != INDEX_NONE)
			{
				const FString Tag = FullDialogue.Mid(Index, TagEnd - Index + 1);
				Segments.Add(FDialogueSegment{true, Tag});
				Index = TagEnd + 1;
			}
			else
			{
				Segments.Add(FDialogueSegment{false, FString::Chr(FullDialogue[Index])});
				Index++;
			}
		}
		else
		{
			int32 NextTag = FullDialogue.Find("<", ESearchCase::IgnoreCase, ESearchDir::FromStart, Index);
			if (NextTag == INDEX_NONE) NextTag = FullDialogue.Len();
			const FString Text = FullDialogue.Mid(Index, NextTag - Index);
			Segments.Add(FDialogueSegment{false, Text});
			Index = NextTag;
		}
	}
	return Segments;
}

void UDialogueWidget::SetupInputBindings()
{
	const APlayerController* PC = GetOwningPlayer();
	if (PC == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to get Owning Player Controller. Input bindings will not be set up."));
		return;
	}

	const ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(PC->Player.Get());
	if (LocalPlayer == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("LocalPlayer is null. Input bindings will not be set up."));
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);
	if (Subsystem == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Subsystem is null. Input bindings will not be set up."));
		return;
	}

	if (DialogueMappingContext == nullptr || SkipDialogueAction == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("MappingContext or SkipAction is null. Input bindings will not be set up."));
		return;
	}
	
	Subsystem->AddMappingContext(DialogueMappingContext, 0);
	
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PC->InputComponent))
	{
		EnhancedInputComponent->BindAction(SkipDialogueAction, ETriggerEvent::Started, this, &UDialogueWidget::SkipTyping);
	}
}

void UDialogueWidget::StartTyping(const FString& Dialogue)
{
	TypedCharacterCount = 0;
	CurrentPartialDialogueText = "";
	CurrentCharacterIndex = 0;
	CurrentOpenTags.Empty();

	DialogueSegments = ParseDialogue(Dialogue);

	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TextTimerHandle);
		World->GetTimerManager().SetTimer(TextTimerHandle,this, &UDialogueWidget::UpdateText,TypeSpeed,true);
	}
}

void UDialogueWidget::StartDialogue(UDialogueDataAsset* DialogueDataAsset)
{
	OnDialogueStarted.Broadcast();
	CachedDialogueDataAsset = DialogueDataAsset;
	CurrentDialogueData = DialogueDataAsset->GetDialogueData();
	CurrentDialogueIndex = -1;
	ShowNextDialogue();
}

void UDialogueWidget::ShowNextDialogue()
{
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TextTimerHandle);
	}

	CurrentDialogueIndex++;
	if (CurrentDialogueIndex >= CurrentDialogueData.Num())
	{
		EndDialogue();
		return;
	}

	const FDialogueData& DialogueData = CurrentDialogueData[CurrentDialogueIndex];
	const FSpeakerData& SpeakerData = CachedDialogueDataAsset->GetSpeakerData(DialogueData.SpeakerID);
    
	CurrentCharacterPortrait = SpeakerData.SpeakerPortrait;
	CurrentCharacterName = SpeakerData.SpeakerName;
	CurrentFullDialogueText = DialogueData.DialogueText;
	CurrentCharacterDialogue = FText::GetEmpty();

	StartTyping(DialogueData.DialogueText.ToString());
}

void UDialogueWidget::EndDialogue() const
{
	RemoveInputMappingContext();
	OnDialogueFinished.Broadcast();
}

void UDialogueWidget::UpdateText()
{
	if (TypedCharacterCount < DialogueSegments.Num())
	{
		if (AppendNextCharacter())
		{
			CurrentCharacterDialogue = FText::FromString(BuildRenderText());
			UpdateDialogueBinding();
		}
	}
	else if (bAutoAdvance)
	{
		if (const UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(TextTimerHandle);
			World->GetTimerManager().SetTimer(TextTimerHandle,this, &UDialogueWidget::ShowNextDialogue,AutoAdvanceDelay,false);
		}
	}
}

void UDialogueWidget::SkipTyping()
{
	const UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	if (TypedCharacterCount >= DialogueSegments.Num())
	{
		World->GetTimerManager().ClearTimer(TextTimerHandle);
		ShowNextDialogue();
	}
	else if (World->GetTimerManager().IsTimerActive(TextTimerHandle))
	{
		World->GetTimerManager().ClearTimer(TextTimerHandle);
		CurrentCharacterDialogue = CurrentFullDialogueText;
		UpdateDialogueBinding();
		TypedCharacterCount = CurrentFullDialogueText.ToString().Len();

		if (bAutoAdvance)
		{
			World->GetTimerManager().SetTimer(TextTimerHandle, this, &UDialogueWidget::ShowNextDialogue, AutoAdvanceDelay, false);
		}
	}
}

void UDialogueWidget::RemoveInputMappingContext() const
{
	if (const APlayerController* PC = GetOwningPlayer())
	{
		if (const ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(PC->Player.Get()))
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
			{
				Subsystem->RemoveMappingContext(DialogueMappingContext);
			}
		}
	}
}

bool UDialogueWidget::AppendNextCharacter()
{
	if (TypedCharacterCount >= DialogueSegments.Num())
	{
		return false;
	}

	const FDialogueSegment& Segment = DialogueSegments[TypedCharacterCount];
	if (Segment.bIsTag)
	{
		CurrentPartialDialogueText += Segment.Content;
		if (Segment.Content == "</>")
		{
			if (CurrentOpenTags.Num() > 0) CurrentOpenTags.Pop();
		}
		else
		{
			CurrentOpenTags.Add(Segment.Content);
		}
		TypedCharacterCount++;
	}
	else
	{
		if (CurrentCharacterIndex < Segment.Content.Len())
		{
			CurrentPartialDialogueText.AppendChar(Segment.Content[CurrentCharacterIndex]);
			CurrentCharacterIndex++;
		}
		if (CurrentCharacterIndex >= Segment.Content.Len())
		{
			TypedCharacterCount++;
			CurrentCharacterIndex = 0;
		}
	}
	
	return true;
}

FString UDialogueWidget::BuildRenderText() const
{
	FString RenderText = CurrentPartialDialogueText;
	for (const FString& OpenTag : CurrentOpenTags)
	{
		RenderText += "</>";
	}
	return RenderText;
}
