#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Input/FociInputTypes.h"
#include "FociControlsSaveGame.generated.h"

UCLASS()
class FOCI_API UFociControlsSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	static constexpr int32 MaxHumanPlayers = 4;

	UFociControlsSaveGame() = default;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Foci|SaveGame")
	FString SaveSlotName = TEXT("FociControlsSaveSlot");

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Foci|SaveGame")
	int32 UserIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|SaveGame")
	TArray<FFociPlayerInputProfile> PlayerProfiles;
};
