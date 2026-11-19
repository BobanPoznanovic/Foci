#pragma once

#include "CoreMinimal.h"
#include "Gameplay/FociFootballerCharacter.h"
#include "FociGoalkeeperCharacter.generated.h"

class AFociBallActor;

UCLASS()
class FOCI_API AFociGoalkeeperCharacter : public AFociFootballerCharacter
{
	GENERATED_BODY()

public:
	AFociGoalkeeperCharacter();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Goalkeeper")
	float DiveReachCm = 240.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Goalkeeper")
	float DiveSpeedCmPerSec = 900.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Goalkeeper")
	float CatchMaxShotSpeedCmPerSec = 1900.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Goalkeeper")
	float DiveCooldownSeconds = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Goalkeeper")
	bool bIsDiving = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Goalkeeper")
	FVector ActiveDiveTarget3D = FVector::ZeroVector;

	UFUNCTION(BlueprintCallable, Category = "Foci|Goalkeeper")
	void UpdateGoalkeeperAI(AFociBallActor* Ball, float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "Foci|Goalkeeper")
	void TriggerDiveToward(const FVector& InterceptPoint3D);

	UFUNCTION(BlueprintPure, Category = "Foci|Goalkeeper")
	static bool ComputeGoalkeeperIntercept3D(
		const FVector& BallLocation,
		const FVector& BallVelocity,
		float GoalLineX,
		float GoalHalfWidthCm,
		float CrossbarHeightCm,
		FVector& OutInterceptPoint3D,
		float& OutTimeToGoalLine
	);
};
