#include "DialogueManager.h"

#include <Engine/World.h>
#include <GameFramework/PlayerController.h>

ADialogueManager::ADialogueManager(): DialogueWidget(nullptr), DefaultDialogueData(nullptr)
{
	PrimaryActorTick.bCanEverTick = false;
	bStartDialogueOnBeginPlay = false;
}

void ADialogueManager::BeginPlay()
{
	Super::BeginPlay();

	if (bStartDialogueOnBeginPlay && DefaultDialogueData)
	{
		StartDialogue(DefaultDialogueData);
	}
}

void ADialogueManager::SetUIInputMode() const
{
	const UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}
	
	if (APlayerController* PC = World->GetFirstPlayerController())
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetWidgetToFocus(DialogueWidget->TakeWidget());
		InputMode.SetHideCursorDuringCapture(false);
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = true;
	}
}

void ADialogueManager::StartDialogue(UDialogueDataAsset* DialogueDataAsset)
{
	if (!DialogueDataAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("StartDialogue called without valid DialogueDataAsset!"));
		return;
	}

	if (!DialogueWidget && DialogueWidgetClass)
	{
		DialogueWidget = CreateWidget<UDialogueWidget>(GetWorld(), DialogueWidgetClass);
		if (DialogueWidget)
		{
			DialogueWidget->AddToViewport();
			DialogueWidget->SetupInputBindings();
		}
	}

	if (DialogueWidget)
	{
		DialogueWidget->StartDialogue(DialogueDataAsset);
		SetUIInputMode();
	}
}