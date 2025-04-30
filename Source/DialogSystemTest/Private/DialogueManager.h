#pragma once
#include "CoreMinimal.h"
#include <GameFramework/Actor.h>

#include "DialogueDataAsset.h"
#include "DialogueWidget.h"

#include "DialogueManager.generated.h"

UCLASS(Blueprintable)
class ADialogueManager : public AActor
{
	GENERATED_BODY()

public:
	ADialogueManager();

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void StartDialogue(UDialogueDataAsset* DialogueDataAsset);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	TSubclassOf<UDialogueWidget> DialogueWidgetClass;

	UPROPERTY(VisibleInstanceOnly, Category = "Dialogue")
	TObjectPtr<UDialogueWidget> DialogueWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	TObjectPtr<UDialogueDataAsset> DefaultDialogueData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	bool bStartDialogueOnBeginPlay;

private:
	void SetUIInputMode() const;
};
