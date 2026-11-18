// Copyright Foci Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FociPlayerController.generated.h"

class UInputMappingContext;

/**
 * Primary PlayerController for Foci with Enhanced Input support.
 */
UCLASS()
class FOCI_API AFociPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	/** Default Input Mapping Context applied on BeginPlay */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	virtual void BeginPlay() override;
};
