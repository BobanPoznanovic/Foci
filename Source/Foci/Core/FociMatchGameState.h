#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Input/FociInputTypes.h"
#include "FociMatchGameState.generated.h"

UENUM(BlueprintType)
enum class EFociMatchPhase : uint8
{
	PreMatchLobby = 0,
	KickOff = 1,
	InPlay = 2,
	GoalCelebration = 3,
	ThrowIn = 4,
	CornerKick = 5,
	GoalKick = 6,
	FreeKick = 7,
	PenaltyKick = 8,
	HalfTime = 9,
	FullTime = 10
};

UCLASS()
class FOCI_API AFociMatchGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Match")
	int32 HomeScore = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Match")
	int32 AwayScore = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Match")
	float MatchClockSeconds = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Match")
	float MatchTimeScale = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Match")
	int32 CurrentHalf = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Match")
	EFociMatchPhase CurrentPhase = EFociMatchPhase::KickOff;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Match")
	EFociTeamSide KickOffTeam = EFociTeamSide::Home;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Match")
	EFociTeamSide PossessionTeam = EFociTeamSide::Neutral;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Match")
	bool bOffsideTriggered = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Match")
	EFociTeamSide LastOffsideTeam = EFociTeamSide::Neutral;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Match")
	FVector OffsideSpot = FVector::ZeroVector;

	UFUNCTION(BlueprintPure, Category = "Foci|Match")
	int32 GetMatchMinutes() const;
};
