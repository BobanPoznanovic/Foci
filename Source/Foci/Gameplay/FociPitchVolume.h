#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Input/FociInputTypes.h"
#include "FociPitchVolume.generated.h"

class UBoxComponent;

UENUM(BlueprintType)
enum class EFociPitchEvent : uint8
{
	InPlay = 0,
	GoalScoredHome = 1,
	GoalScoredAway = 2,
	ThrowInHome = 3,
	ThrowInAway = 4,
	CornerKickHome = 5,
	CornerKickAway = 6,
	GoalKickHome = 7,
	GoalKickAway = 8
};

UCLASS()
class FOCI_API AFociPitchVolume : public AActor
{
	GENERATED_BODY()

public:
	AFociPitchVolume();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foci|Pitch")
	TObjectPtr<UBoxComponent> PitchBoundsVolume;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Pitch")
	float HalfPitchLengthCm = 5250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Pitch")
	float HalfPitchWidthCm = 3400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Pitch")
	float GoalHalfWidthCm = 366.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Pitch")
	float GoalCrossbarHeightCm = 244.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Pitch")
	float PenaltyBoxLengthCm = 1650.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Pitch")
	float PenaltyBoxHalfWidthCm = 2016.0f;

	UFUNCTION(BlueprintPure, Category = "Foci|Pitch")
	FVector GetKickOffSpot() const;

	UFUNCTION(BlueprintPure, Category = "Foci|Pitch")
	FVector ComputeRestartLocation3D(EFociPitchEvent Event, const FVector& LastOutBallLocation) const;

	UFUNCTION(BlueprintPure, Category = "Foci|Pitch")
	static EFociPitchEvent EvaluateBallBoundary3D(
		const FVector& BallCenterLocation,
		float BallRadiusCm,
		EFociTeamSide LastTouchTeam,
		float HalfLengthCm = 5250.0f,
		float HalfWidthCm = 3400.0f,
		float GoalHalfWidth = 366.0f,
		float CrossbarHeight = 244.0f
	);

	UFUNCTION(BlueprintPure, Category = "Foci|Pitch")
	static bool IsPlayerInOffsidePosition3D(
		float ReceiverX,
		float BallXAtPass,
		EFociTeamSide AttackingTeam,
		const TArray<float>& DefenderXPositions
	);
};
